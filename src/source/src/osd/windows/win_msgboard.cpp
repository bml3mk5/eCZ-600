/** @file win_msgboard.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.26 -

	@brief [ message board ]
*/
#include "win_msgboard.h"
#include "../../common.h"
#include "win_emu.h"
#include "../../config.h"
#include "../../utility.h"

#undef USE_BG_TRANSPARENT

MsgBoard::MsgBoard(HWND hWnd, EMU *pEmu) : CSurface()
{
	visible = true;
	inited = false;

	info.font = new CFont();
	msg.font = new CFont();

	hPen   = NULL;
	hBrush = NULL;

	// 色
	fg.Set(0x00, 0xc0, 0x80, 0xff);
	bg.Set(0x00, 0x40, 0x00, 0xff);

	szWin.cx = 1;
	szWin.cy = 1;

	emu = pEmu;

	info.mux = new CMutex();
	msg.mux = new CMutex();
}

MsgBoard::~MsgBoard()
{
	ReleaseTexture();

	if (enable) {
		if (hBrush)     DeleteObject(hBrush);
		if (hPen)       DeleteObject(hPen);
	}
	delete info.font;
	delete msg.font;

	delete info.mux;
	delete msg.mux;
}

/// 画面初期化
void MsgBoard::InitScreen(int width, int height)
{
	// ウィンドウサイズ
	szWin.cx = width;
	szWin.cy = height;

	// Dib作成
	Create(width, 128);

	if (enable) {
#ifndef USE_BG_TRANSPARENT
		// 描画用ペンの設定
		hPen = CreatePen(PS_SOLID, 1, fg.Get());
		SelectObject(hMainDC, hPen);

		// 描画用ブラシの設定
		hBrush = CreateSolidBrush(bg.Get());
		SelectObject(hMainDC, hBrush);
#else
		// 描画用ペンの設定
		hPen = CreatePen(PS_SOLID, 1, bg);

		// 描画用ブラシの設定
		hBrush = CreateSolidBrush(fg);
#endif

		// 枠設定
		SetRect(&msg.re, 0, 0, width, 63);
		// 表示位置
		msg.pt.x = 0; msg.pt.y = 0; info.place = 2;

		// 枠設定
		SetRect(&info.re, 0, 64, width, 127);
		// 表示位置
		info.pt.x = 0; info.pt.y = 0; info.place = 1;

	}
	if (enable && SetFont()) {
		inited = true;
		pConfig->msgboard_msg_fontname.Set(msg.font->GetFontNamePtr());
		pConfig->msgboard_info_fontname.Set(info.font->GetFontNamePtr());
	}
	if (enable) {
		logging->out_log_x(LOG_INFO , CMsg::MsgBoard_OK);
	} else {
		logging->out_log_x(LOG_ERROR, CMsg::MsgBoard_Failed);
	}
}

/// フォント設定
bool MsgBoard::SetFont()
{
	_TCHAR font_name[64];

	// フォントパスの設定
	if (pConfig->font_path.Length() > 0 && !CFont::AddFontPath(pConfig->font_path.Get())) {
		logging->out_logf_x(LOG_WARN, CMsg::MsgBoard_Couldn_t_load_font_VSTR, pConfig->font_path);
	}

	msg.font->SetFont((HWND)NULL, pConfig->msgboard_msg_fontname.Get(), pConfig->msgboard_msg_fontsize, FW_NORMAL, fg.Get());
	if (msg.font->GetFont() != NULL) {
		CTchar xtitle(gMessages.Get(CMsg::message));
		msg.font->GetFontName(font_name, sizeof(font_name) / sizeof(font_name[0]));
		logging->out_logf_x(LOG_INFO, CMsg::MsgBoard_Use_VSTR_for_VSTR, font_name, xtitle.Get());
	} else {
		logging->out_logf_x(LOG_WARN, CMsg::MsgBoard_Couldn_t_load_font_VSTR_for_message, pConfig->msgboard_msg_fontname);
	}
	info.font->SetFont((HWND)NULL, pConfig->msgboard_info_fontname.Get(), pConfig->msgboard_info_fontsize, FW_BOLD, fg.Get());
	if (info.font->GetFont() != NULL) {
		CTchar xtitle(gMessages.Get(CMsg::info));
		info.font->GetFontName(font_name, sizeof(font_name) / sizeof(font_name[0]));
		logging->out_logf_x(LOG_INFO, CMsg::MsgBoard_Use_VSTR_for_VSTR, font_name, xtitle.Get());
	} else {
		logging->out_logf_x(LOG_WARN, CMsg::MsgBoard_Couldn_t_load_font_VSTR_for_info, pConfig->msgboard_info_fontname);
	}
	if (msg.font->GetFont() == NULL || info.font->GetFont() == NULL) {
		enable = false;
	} else {
		enable = true;
	}
	return enable;
}

/// 文字列出力
void MsgBoard::draw(HDC hdc, msg_data_t &data)
{
	POINT pt;

	data.mux->lock();

	if (!data.lists.empty()) {
		list_t::iterator it = data.lists.begin();

		// 基準位置の計算
		pt = data.pt;
		if (data.place & 1) {
			pt.x += szWin.cx - data.sz.cx;
		}
		if (data.place & 2) {
			pt.y += szWin.cy - data.sz.cy;
		}

		// メインコンテキストにメッセージをコピー
#ifdef USE_BG_TRANSPARENT
		draw_text(hdc, data, pt.x, pt.y);
#else
		BitBlt(hdc, pt.x, pt.y,
			data.sz.cx, data.sz.cy,
			hMainDC, data.re.left, data.re.top, SRCCOPY);
#endif
	}

	data.mux->unlock();
}
void MsgBoard::draw(LPDIRECT3DSURFACE9 suf, msg_data_t &data)
{
	RECT  reDst;

	data.mux->lock();

	if (!data.lists.empty()) {
		list_t::iterator it = data.lists.begin();

		// 基準位置の計算
		if (data.place & 1) {
			// 画面右が原点
			reDst.left = szWin.cx + data.pt.x - data.sz.cx;
			reDst.right = szWin.cx + data.pt.x;
		} else {
			// 画面左が原点
			reDst.left = data.pt.x;
			reDst.right = data.pt.x + data.sz.cx;
		}
		if (data.place & 2) {
			// 画面下が原点
			reDst.top = szWin.cy + data.pt.y - data.sz.cy;
			reDst.bottom = szWin.cy + data.pt.y;
		} else {
			// 画面上が原点
			reDst.top = data.pt.y;
			reDst.bottom = data.pt.y + data.sz.cy;
		}

		// メインコンテキストにメッセージをコピー
#ifdef USE_DIRECT3DX
		RECT  reSrc;
		SetRect(&reSrc, data.re.left, data.re.top, data.re.left + data.sz.cx, data.re.top + data.sz.cy);

		D3DXLoadSurfaceFromMemory(suf,
			NULL,
			&reDst,
			pMainBuf,
			D3DFMT_X8R8G8B8,
			szWin.cx * 4,
			NULL,
			&reSrc,
			D3DX_FILTER_NONE,
			0);
#else
		if (suf) {
			HDC hdc;
			HRESULT hre = suf->GetDC(&hdc);
			if (hre == D3D_OK) {
#ifdef USE_BG_TRANSPARENT
				draw_text(hdc, data, reDst.left, reDst.top);
#else
				BitBlt(hdc, reDst.left, reDst.top,
					data.sz.cx, data.sz.cy,
					hMainDC, data.re.left, data.re.top, SRCCOPY);
#endif
			}
			suf->ReleaseDC(hdc);
		}
#endif
	}

	data.mux->unlock();
}

void MsgBoard::draw(PDIRECT3DDEVICE9 device, msg_data_t &data, CD3DTexture &tex)
{
	RECT  reDst;

	data.mux->lock();

	if (!data.lists.empty()) {
		list_t::iterator it = data.lists.begin();

		// 基準位置の計算
		if (data.place & 1) {
			// 画面右が原点
			reDst.left = szWin.cx + data.pt.x - data.sz.cx;
			reDst.right = szWin.cx + data.pt.x;
		} else {
			// 画面左が原点
			reDst.left = data.pt.x;
			reDst.right = data.pt.x + data.sz.cx;
		}
		if (data.place & 2) {
			// 画面下が原点
			reDst.top = szWin.cy + data.pt.y - data.sz.cy;
			reDst.bottom = szWin.cy + data.pt.y;
		} else {
			// 画面上が原点
			reDst.top = data.pt.y;
			reDst.bottom = data.pt.y + data.sz.cy;
		}

		// メインコンテキストにメッセージをコピー
		tex.CopyD3DTextureFrom(this, data.re.top, 64);

		tex.SetD3DTexturePositionUv(reDst);
		tex.DrawD3DTexture(device);
	}

	data.mux->unlock();
}

void MsgBoard::Draw(HDC hdc)
{
	if (!visible || !inited || !enable) return;

	draw(hdc, msg);
	draw(hdc, info);
}

void MsgBoard::Draw(LPDIRECT3DSURFACE9 suf)
{
	if (!visible || !inited || !enable) return;

	draw(suf, msg);
	draw(suf, info);
}

void MsgBoard::Draw(PDIRECT3DDEVICE9 device)
{
#ifdef USE_SCREEN_D3D_TEXTURE
	if (!visible || !inited || !enable) return;

	draw(device, msg, tex_msg);
	draw(device, info, tex_info);
#endif
}

/// テクスチャ作成
HRESULT MsgBoard::CreateTexture(PDIRECT3DDEVICE9 device)
{
#ifdef USE_SCREEN_D3D_TEXTURE
	HRESULT hre = D3D_OK;
	hre |= tex_msg.CreateD3DTexture(device, Width(), 64);
	hre |= tex_info.CreateD3DTexture(device, Width(), 64);
	return hre;
#else
	return E_FAIL;
#endif
}

/// テクスチャ解放
void MsgBoard::ReleaseTexture()
{
#ifdef USE_SCREEN_D3D_TEXTURE
	tex_msg.ReleaseD3DTexture();
	tex_info.ReleaseD3DTexture();
#endif
}

/// 文字列をバックバッファに描画
///
/// @note ロックは呼び出し元で行うこと should be locked in caller function
void MsgBoard::draw_text(msg_data_t &data)
{
	SIZE sz;
	int len = 0;

	if (enable && !data.lists.empty()) {
		list_t::iterator it = data.lists.begin();

		// フォント設定
		SelectObject(hMainDC, data.font->GetFont());

		// 文字長さ
		len = (int)_tcslen(it->msg);

#ifndef USE_BG_TRANSPARENT
		// 背景色
		SetBkColor(hMainDC, bg.Get());
		// 文字色
		SetTextColor(hMainDC, data.font->GetFontColor());

		// 枠を描画
		FillRect(hMainDC, &data.re, hBrush);

		// 文字を描画
		TextOut(hMainDC, data.re.left + 2, data.re.top + 2, it->msg, len);
#endif

		// 文字列の幅高さの枠を計算
		GetTextExtentPoint32(hMainDC, it->msg, len, &sz);

		data.sz.cx = sz.cx + 4;
		data.sz.cy = sz.cy + 4;
	}
}

/// 文字列をバックバッファに描画
///
/// @note ロックは呼び出し元で行うこと should be locked in caller function
void MsgBoard::draw_text(HDC hdc, msg_data_t &data, int left, int top)
{
	int len = 0;

	if (enable && !data.lists.empty()) {
		list_t::iterator it = data.lists.begin();

		// 背景色は透過
		SetBkMode(hdc, TRANSPARENT);

#ifdef USE_STROKE_AND_FILLPATH
		// 背景色(縁取り)
		SelectObject(hdc, hPen);
		// 文字色
		SelectObject(hdc, hBrush);

		// フォント設定
		SelectObject(hdc, data.font->GetFont());

		// 文字を描画
		len = (int)_tcslen(it->msg);
		BeginPath(hdc);
		TextOut(hdc, left + data.re.left + 2, top + 2, it->msg, len);
		EndPath(hdc);

		// 縁取り
		StrokeAndFillPath(hdc);
#else
		// 文字色
		SetTextColor(hdc, data.font->GetFontColor());

		// 枠を描画
//		FillRect(hdc, &data.re, hBrush);

		// フォント設定
		SelectObject(hdc, data.font->GetFont());

		// 文字を描画
		len = (int)_tcslen(it->msg);
		TextOut(hdc, left + data.re.left + 2, top + 2, it->msg, len);
#endif
	}
}

/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::Set(msg_data_t &data, const _TCHAR *str, int sec)
{
	item_t itm;

	UTILITY::tcscpy(itm.msg, sizeof(itm.msg) / sizeof(itm.msg[0]), str);
	itm.cnt = (60 * sec);

	data.mux->lock();

	data.lists.push_front(itm);
	if (data.lists.size() > 10) {
		data.lists.pop_back();
	}

	if (inited && enable) {
		draw_text(data);
	}

	data.mux->unlock();
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::Set(msg_data_t &data, CMsg::Id id, int sec)
{
	Set(data, gMessages.Get(id), sec);
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::SetMessage(const _TCHAR *str, int sec)
{
	Set(msg, str, sec);
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::SetMessage(CMsg::Id id, int sec)
{
	Set(msg, id, sec);
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::SetInfo(const _TCHAR *str, int sec)
{
	Set(info, str, sec);
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::SetInfo(CMsg::Id id, int sec)
{
	Set(info, id, sec);
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::SetMessageF(const _TCHAR *format, ...)
{
	_TCHAR buf[MSGBOARD_STR_SIZE];

	va_list ap;

	va_start(ap, format);
	UTILITY::vstprintf(buf, sizeof(buf) / sizeof(buf[0]), format, ap);
	va_end(ap);

	SetMessage(buf);
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::SetInfoF(const _TCHAR *format, ...)
{
	_TCHAR buf[MSGBOARD_STR_SIZE];

	va_list ap;

	va_start(ap, format);
	UTILITY::vstprintf(buf, sizeof(buf) / sizeof(buf[0]), format, ap);
	va_end(ap);

	SetInfo(buf);
}

/// ウィンドウサイズ設定
void MsgBoard::SetSize(int width, int height)
{
	szWin.cx = width;
	szWin.cy = height;
}

/// メッセージ描画位置設定
/// @param[in] cx X
/// @param[in] cy Y
/// @param[in] place 1:画面右基準 2:画面下基準
void MsgBoard::SetMessagePos(int cx, int cy, int place)
{
	msg.pt.x = cx;
	msg.pt.y = cy;
	msg.place = place;
}
/// 情報描画位置設定
/// @param[in] cx X
/// @param[in] cy Y
/// @param[in] place 1:画面右基準 2:画面下基準
void MsgBoard::SetInfoPos(int cx, int cy, int place)
{
	info.pt.x = cx;
	info.pt.y = cy;
	info.place = place;
}

/// メッセージ削除
void MsgBoard::Delete(msg_data_t &data, const _TCHAR *str)
{
	bool redraw = false;

	data.mux->lock();

	if (!data.lists.empty()) {
		list_t::iterator it = data.lists.begin();
		list_t::iterator it_next;

		while(it != data.lists.end())	{
			it_next = it;
			it_next++;

			if (_tcscmp(str, it->msg) == 0) {
				data.lists.erase(it);
				redraw = true;
			}
			it = it_next;
		}
	}
	if (redraw) {
		draw_text(data);
	}

	data.mux->unlock();
}
void MsgBoard::Delete(msg_data_t &data, CMsg::Id id)
{
	Delete(data, gMessages.Get(id));
}
void MsgBoard::DeleteMessage(const _TCHAR *str)
{
	Delete(msg, str);
}
void MsgBoard::DeleteMessage(CMsg::Id id)
{
	Delete(msg, id);
}
void MsgBoard::DeleteInfo(const _TCHAR *str)
{
	Delete(info, str);
}
void MsgBoard::DeleteInfo(CMsg::Id id)
{
	Delete(info, id);
}

/// カウントダウン
void MsgBoard::count_down(msg_data_t &data)
{
	bool redraw = false;

	data.mux->lock();

	if (!data.lists.empty()) {
		list_t::iterator it = data.lists.begin();
		list_t::iterator it_next;

		while(it != data.lists.end())	{
			it_next = it;
			it_next++;

			if (it->cnt > 0) it->cnt--;
			if (it->cnt == 0) {
				data.lists.erase(it);
				redraw = true;
			}
			it = it_next;
		}
	}
	if (redraw) {
		draw_text(data);
	}

	data.mux->unlock();
}
void MsgBoard::CountDown(void)
{
	count_down(msg);
	count_down(info);
}
