/** @file sdl_msgboard.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.26 -

	@brief [ message board ]
*/
#include "sdl_msgboard.h"
#include "../../cpixfmt.h"
#include "sdl_csurface.h"
#include "sdl_emu.h"
#include "../../config.h"
#include "../../utility.h"
#if !defined(_WIN32)
#include <dirent.h>
#endif

#undef USE_BG_TRANSPARENT

MsgBoard::MsgBoard(EMU *pEmu)
{
	visible = true;
	enable  = false;

	sMainSuf = NULL;
	emu = pEmu;

	msg.font = NULL;
	info.font = NULL;

	msg.mux = new CMutex();
	info.mux = new CMutex();

	inited = false;
}

MsgBoard::~MsgBoard()
{
	if (inited) {
		if (msg.font != NULL) TTF_CloseFont(msg.font);
		if (info.font != NULL) TTF_CloseFont(info.font);
		TTF_Quit();
	}
	delete sMainSuf;

	delete msg.mux;
	delete info.mux;
}

bool MsgBoard::CreateSurface(CPixelFormat *format, int width, int height)
{
	if (sMainSuf != NULL) {
		delete sMainSuf;
	}
	sMainSuf = new CSurface(width, height, *format);
	return (sMainSuf != NULL);
}

/// 画面初期化
void MsgBoard::InitScreen(CPixelFormat *format, int width, int height)
{
	// 色
	fg.Set(*format, 0x00, 0xc0, 0x80, 0xff);
	bg.Set(*format, 0x00, 0x40, 0x00, 0xff);

	// ウィンドウサイズ
	szWin.cx = width;
	szWin.cy = height;

	if (CreateSurface(format, width, 128)) {
		// SDL_ttf 初期化
		if (!TTF_WasInit()) {
			if (TTF_Init() != -1) {
				inited = true;
			} else {
				enable = false;
			}
		}
	}
	// メッセージ用フォントの設定
	// 枠設定
	RECT_IN(msg.re, 0, 0, width, 63);
	// 表示位置
	msg.pt.x = 0; msg.pt.y = 0; info.place = 2;
	// 情報用フォントの設定
	// 枠設定
	RECT_IN(info.re, 0, 64, width, 127);
	// 表示位置
	info.pt.x = 0; info.pt.y = 0; info.place = 1;

	SetFont();

	if (enable) {
		logging->out_log_x(LOG_INFO , CMsg::MsgBoard_OK);
	} else {
		logging->out_log_x(LOG_ERROR, CMsg::MsgBoard_Failed);
	}
}

/// フォント設定
bool MsgBoard::SetFont()
{
	if (!inited) return false;

	// メッセージ用フォントの設定
	enable = set_sys_font(CMsg::message, pConfig->msgboard_msg_fontname.Get(), pConfig->msgboard_msg_fontsize, &msg.font, msg.font_name);
	if (enable) {
		TTF_SetFontStyle(msg.font, TTF_STYLE_NORMAL);
		pConfig->msgboard_msg_fontname.SetN(msg.font_name);
		// 情報用フォントの設定
		enable = set_sys_font(CMsg::info, pConfig->msgboard_info_fontname.Get(), pConfig->msgboard_info_fontsize, &info.font, info.font_name);
		if (enable) {
			TTF_SetFontStyle(info.font, TTF_STYLE_NORMAL);
			pConfig->msgboard_info_fontname.SetN(info.font_name);
		}
	}
	return enable;
}

/// 基準位置の計算
void MsgBoard::calc_place(msg_data_t &data, SDL_Rect &reDst)
{
	if (data.place & 1) {
		reDst.x = szWin.cx + data.pt.x - data.sz.cx;
		reDst.w = data.sz.cx;
	} else {
		reDst.x = data.pt.x;
		reDst.w = data.sz.cx;
	}
	if (data.place & 2) {
		reDst.y = szWin.cy + data.pt.y - data.sz.cy;
		reDst.h = data.sz.cy;
	} else {
		reDst.y = data.pt.y;
		reDst.h = data.sz.cy;
	}
}

/// 文字列出力
void MsgBoard::draw(CSurface &screen, msg_data_t &data)
{
	SDL_Rect reDst;

	data.mux->lock();

	if (!data.lists.empty()) {
		// 基準位置の計算
		calc_place(data, reDst);

		// メインコンテキストにメッセージをコピー
#ifdef USE_BG_TRANSPARENT
		draw_text(screen, data, reDst.x, reDst.y);
#else
		sMainSuf->Blit(data.re, screen, reDst);
#endif
	}

	data.mux->unlock();
}

/// 文字列出力
void MsgBoard::draw(SDL_Surface &screen, msg_data_t &data)
{
	SDL_Rect reDst;

	data.mux->lock();

	if (!data.lists.empty()) {
		// 基準位置の計算
		calc_place(data, reDst);

		// メインコンテキストにメッセージをコピー
#ifdef USE_BG_TRANSPARENT
		draw_text(screen, data, reDst.x, reDst.y);
#else
		sMainSuf->Blit(data.re, screen, reDst);
#endif
	}

	data.mux->unlock();
}

#ifdef USE_GTK
/// 文字列出力
void MsgBoard::draw(cairo_t *screen, msg_data_t &data)
{
	SDL_Rect reDst;

	data.mux->lock();

	if (!data.lists.empty()) {
		// 基準位置の計算
		calc_place(data, reDst);

		// メインコンテキストにメッセージをコピー
#ifdef USE_BG_TRANSPARENT
//		draw_text(screen, data, reDst.x, reDst.y);
#else
		CCairoSurface cas(*sMainSuf, data.re.w, data.re.y + data.re.h);
		cas.BlitC(data.re, screen, reDst);
		cairo_paint(screen);
#endif
	}

	data.mux->unlock();
}
#endif

#if defined(USE_SDL2)
/// 文字列出力
void MsgBoard::draw(CTexture &texture, msg_data_t &data)
{
	SDL_Rect reDst;

	data.mux->lock();

	if (!data.lists.empty()) {
		// 基準位置の計算
		calc_place(data, reDst);

		// メインコンテキストにメッセージをコピー
#ifdef USE_BG_TRANSPARENT
		draw_text(screen, data, reDst.x, reDst.y);
#else
		scrntype *buf = sMainSuf->GetBuffer();
		buf += data.re.y * sMainSuf->Width();
		SDL_Rect texture_re;
		RECT_IN(texture_re, 0, data.re.y, reDst.w, reDst.h);
		SDL_UpdateTexture(texture.Get(), &texture_re, buf, sMainSuf->BytesPerLine());
		SDL_RenderCopy(texture.Renderer(), texture.Get(), &texture_re, &reDst);
#endif
	}

	data.mux->unlock();
}
#endif

#if defined(USE_OPENGL)
void MsgBoard::draw(COpenGLTexture &texture, msg_data_t &data)
{
	SDL_Rect reDst;

	data.mux->lock();

	if (!data.lists.empty()) {
		// 基準位置の計算
		calc_place(data, reDst);

		// メインコンテキストにメッセージをコピー
#ifdef USE_BG_TRANSPARENT
		draw_text(screen, data, reDst.x, reDst.y);
#else
		scrntype *buf = sMainSuf->GetBuffer();
		buf += data.re.y * sMainSuf->Width();

		float pyl_l = (float)reDst.x * 2.0f / (float)(szWin.cx) - 1.0f;
		float pyl_r = (float)(reDst.x + reDst.w) * 2.0f / (float)(szWin.cx) - 1.0f;
		float pyl_t = 1.0f - (float)reDst.y * 2.0f / (float)(szWin.cy);
		float pyl_b = 1.0f - (float)(reDst.y + reDst.h) * 2.0f / (float)(szWin.cy);

		float tex_r = (float)(data.sz.cx) / (float)sMainSuf->Width();
		float tex_b = (float)(data.sz.cy) / 64.0f;

		texture.SetPos(pyl_l, pyl_t, pyl_r, pyl_b, 0.0f, 0.0f, tex_r, tex_b);
		texture.Render(sMainSuf->Width(), 64, buf);
#endif
	}

	data.mux->unlock();
}
#endif

void MsgBoard::Draw(CSurface &screen)
{
	if (!enable || !visible) return;

	draw(screen, msg);
	draw(screen, info);
}

void MsgBoard::Draw(SDL_Surface &screen)
{
	if (!enable || !visible) return;

	draw(screen, msg);
	draw(screen, info);
}

#ifdef USE_GTK
void MsgBoard::Draw(cairo_t *screen)
{
	if (!enable || !visible) return;

	draw(screen, msg);
	draw(screen, info);
}
#endif

#if defined(USE_SDL2)
void MsgBoard::Draw(CTexture &texture)
{
	if (!enable || !visible) return;

	draw(texture, msg);
	draw(texture, info);
}
#endif

#if defined(USE_OPENGL)
void MsgBoard::Draw(COpenGLTexture &texture)
{
	if (!enable || !visible) return;

	draw(texture, msg);
	draw(texture, info);
}
#endif

/// 文字列をバックバッファに描画
///
/// @note ロックは呼び出し元で行うこと should be locked in caller function
void MsgBoard::draw_text(msg_data_t &data)
{
	SDL_Surface *sTextSuf = NULL;

	int cx = 0;
	int cy = 0;
//	int len = 0;

	if (enable && !data.lists.empty()) {
		list_t::iterator it = data.lists.begin();

#ifndef USE_BG_TRANSPARENT
		// 枠を描画
		SDL_FillRect(sMainSuf->Get(), &data.re, bg.Map(sMainSuf->GetPixelFormat()));

		// 文字を描画
//		len = (int)strlen(it->cmsg);
		sTextSuf = TTF_RenderUTF8_Shaded(data.font, it->cmsg, fg.Get(), bg.Get());
		if (sTextSuf != NULL) {
			SDL_BlitSurface(sTextSuf, NULL, sMainSuf->Get(), &data.re);

			// 文字列の幅高さの枠を計算
			TTF_SizeUTF8(data.font, it->cmsg, &cx, &cy);
			data.sz.cx = cx + 4;
			data.sz.cy = cy + 4;

			SDL_FreeSurface(sTextSuf);

		} else {
			// エラー
			data.sz.cx = 0;
			data.sz.cy = 0;
		}
#else
		// 文字列の幅高さの枠を計算
		TTF_SizeUTF8(data.font, it->cmsg, &cx, &cy);
		data.sz.cx = cx + 4;
		data.sz.cy = cy + 4;
#endif
	}
}

/// 文字列をバックバッファに描画
///
/// @note ロックは呼び出し元で行うこと should be locked in caller function
void MsgBoard::draw_text(CSurface *suf, msg_data_t &data, int left, int top)
{
	SDL_Surface *sTextSuf = NULL;
	SDL_Rect re;
//	int len = 0;

	if (enable && !data.lists.empty()) {
		list_t::iterator it = data.lists.begin();

		// 文字を描画
//		len = (int)strlen(it->cmsg);
		sTextSuf = TTF_RenderUTF8_Blended(data.font, it->cmsg, fg.Get());
		if (sTextSuf != NULL) {
			re.x = data.re.x + left; re.y = top;
			SDL_BlitSurface(sTextSuf, NULL, suf->Get(), &re);
			SDL_FreeSurface(sTextSuf);
		}
	}
}

/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::Set(msg_data_t &data, const _TCHAR *str, int sec)
{
	item_t itm;

	UTILITY::tcs_to_mbs(itm.cmsg, str, MSGBOARD_STR_SIZE);
	itm.cnt = (60 * sec);

	data.mux->lock();

	data.lists.push_front(itm);
	if (data.lists.size() > 10) {
		data.lists.pop_back();
	}

	if (enable) {
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
	char cmsg[MSGBOARD_STR_SIZE];

	data.mux->lock();

	if (!data.lists.empty()) {
		UTILITY::tcs_to_mbs(cmsg, str, MSGBOARD_STR_SIZE);
		list_t::iterator it = data.lists.begin();
		list_t::iterator it_next;

		while(it != data.lists.end())	{
			it_next = it;
			it_next++;

			if (strcmp(cmsg, it->cmsg) == 0) {
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

/// 画面表示用システムフォントの設定
bool MsgBoard::set_sys_font(CMsg::Id title, const _TCHAR *name, int pt, TTF_Font **font, char *font_file)
{
	sbuf_t sbuf;
	CPtrList<CNchar> fpath;
	CPtrList<CNchar> fname;
//	TTF_Font *f;

#if defined(_WIN32)
	pConfig->font_path.GetN(sbuf.buf, _MAX_PATH);
	fpath.Add(new CNchar(sbuf.buf));
	emu->resource_path(sbuf.buf, _MAX_PATH);
	fpath.Add(new CNchar(sbuf.buf));
	emu->application_path(sbuf.buf, _MAX_PATH);
	fpath.Add(new CNchar(sbuf.buf));
	UTILITY::concat(sbuf.buf, sizeof(sbuf.buf), getenv("SystemRoot"), "\\fonts\\", NULL);
	fpath.Add(new CNchar(sbuf.buf));

	UTILITY::tcs_to_mbs(sbuf.buf, name, _MAX_PATH);
	fname.Add(new CNchar(sbuf.buf));
	fname.Add(new CNchar("msgothic.ttc"));
	fname.Add(new CNchar("arial.ttf"));

#elif defined(linux)
	pConfig->font_path.GetN(sbuf.buf, _MAX_PATH);
	fpath.Add(new CNchar(sbuf.buf));
	emu->resource_path(sbuf.buf, _MAX_PATH);
	fpath.Add(new CNchar(sbuf.buf));
	emu->application_path(sbuf.buf, _MAX_PATH);
	fpath.Add(new CNchar(sbuf.buf));
	UTILITY::sprintf(sbuf.buf, _MAX_PATH, "%s/.fonts/", getenv("HOME"));
	fpath.Add(new CNchar(sbuf.buf));
	fpath.Add(new CNchar("/usr/share/fonts/*"));
	fpath.Add(new CNchar("/usr/X11R6/lib/X11/fonts/*"));

	UTILITY::tcs_to_mbs(sbuf.buf, name, _MAX_PATH);
	fname.Add(new CNchar(sbuf.buf));
	fname.Add(new CNchar("ttf-japanese-gothic.ttf"));
	fname.Add(new CNchar("fonts-japanese-gothic.ttf"));
	fname.Add(new CNchar("NotoSansCJK-Regular.ttc"));
	fname.Add(new CNchar("NotoSansCJK-VF.ttc"));
	fname.Add(new CNchar("ipag.ttf"));
	fname.Add(new CNchar("OpenSans-Regular.ttf"));
	fname.Add(new CNchar("FreeSans.ttf"));

#elif defined(__APPLE__) && defined(__MACH__)
	pConfig->font_path.GetN(sbuf.buf, _MAX_PATH);
	fpath.Add(new CNchar(sbuf.buf));
	emu->resource_path(sbuf.buf, _MAX_PATH);
	fpath.Add(new CNchar(sbuf.buf));
	emu->application_path(sbuf.buf, _MAX_PATH);
	fpath.Add(new CNchar(sbuf.buf));
	UTILITY::sprintf(sbuf.buf, _MAX_PATH, "%s/Library/Fonts/", getenv("HOME"));
	fpath.Add(new CNchar(sbuf.buf));
	fpath.Add(new CNchar("/System/Library/Fonts/"));
	fpath.Add(new CNchar("/Library/Fonts/"));

	UTILITY::tcs_to_mbs(sbuf.buf, name, _MAX_PATH);
	fname.Add(new CNchar(sbuf.buf));
	fname.Add(new CNchar("Hiragino Sans GB W3.ttc"));
	fname.Add(new CNchar("Hiragino Sans GB.ttc"));
	fname.Add(new CNchar("Osaka.ttf"));
	fname.Add(new CNchar("AppleSDGothicNeo.ttf"));
	fname.Add(new CNchar("AppleGothic.ttf"));

#elif defined(__FreeBSD__)
	pConfig->font_path.GetN(sbuf.buf, _MAX_PATH);
	fpath.Add(new CNchar(sbuf.buf));
	emu->resource_path(sbuf.buf, _MAX_PATH);
	fpath.Add(new CNchar(sbuf.buf));
	emu->application_path(sbuf.buf, _MAX_PATH);
	fpath.Add(new CNchar(sbuf.buf));
	UTILITY::sprintf(sbuf.buf, _MAX_PATH, "%s/.fonts/", getenv("HOME"));
	fpath.Add(new CNchar(sbuf.buf));
	fpath.Add(new CNchar("/usr/share/fonts/*"));
	fpath.Add(new CNchar("/usr/local/lib/X11/fonts/*"));

	UTILITY::tcs_to_mbs(sbuf.buf, name, _MAX_PATH);
	fname.Add(new CNchar(sbuf.buf));
	fname.Add(new CNchar("ipagp-mona.ttf"));
	fname.Add(new CNchar("luxisr.ttf"));

#endif
	CTchar xtitle(gMessages.Get(title));

	int ncount = fname.Count();
	int pcount = fpath.Count();
	for (int ni=0; ni<ncount; ni++) {
		for (int pi=0; pi<pcount; pi++) {
			int fpath_len = fpath[pi]->Length();
			const _TCHAR *fpath_buf = fpath[pi]->Get();
			if (fpath_len >= 2 && fpath_buf[fpath_len - 2] == _T('/') && fpath_buf[fpath_len - 1] == _T('*')) {
				// search with sub directory
				char fpath_sbuf[1024];
				UTILITY::strcpy(fpath_sbuf, 1024, fpath_buf);
				fpath_sbuf[fpath_len - 1] = '\0';
				if (open_font_subdir(font, fpath_sbuf, fname[ni]->Get(), pt, xtitle.GetM(), font_file, 0)) {
					return true;
				}
			} else {
				if (open_font(font, fpath_buf, fname[ni]->Get(), pt, xtitle.GetM(), font_file)) {
					return true;
				}
			}
		}
	}
	logging->out_logf_x(LOG_WARN, CMsg::MsgBoard_Couldn_t_find_fonts_for_VSTR, xtitle.GetM());
	return false;
}

/// フォントを開く
bool MsgBoard::open_font_subdir(TTF_Font **font, const char *path, const char *file_name, int pt, const _TCHAR *title, char *font_file, int depth)
{
	if (depth > 4) {
		return false;
	}
	if (open_font(font, path, file_name, pt, title, font_file)) {
		return true;
	}

#if !defined(_WIN32)
	// search sub directory
	char spath[1024];
	struct dirent *dp;

	DIR *dir = opendir(path);
	if (!dir) {
//		logging->out_debugf(_T("cannot open: %s"), path);
		return false;
	}

	while((dp = readdir(dir)) != NULL) {
		if (dp->d_name[0] == '.') {
			continue;
		}
		UTILITY::strcpy(spath, 1024, path);
		UTILITY::strcat(spath, 1024, dp->d_name);
		UTILITY::strcat(spath, 1024, "/");
//		logging->out_debugf(_T("%s : %d"), spath, dp->d_type);
		if (dp->d_type == DT_DIR) {
			if (open_font_subdir(font, spath, file_name, pt, title, font_file, depth + 1)) {
				return true;
			}
		}
	}
	closedir(dir);
#endif
	return false;
}

/// フォントを開く
bool MsgBoard::open_font(TTF_Font **font, const char *path, const char *file_name, int pt, const _TCHAR *title, char *font_file)
{
	char font_path[1024];
	UTILITY::concat(font_path, 1024, path, file_name, NULL);
	TTF_Font *f = TTF_OpenFont(font_path, pt);
	logging->out_debugf(_T("%s: %s"), font_path, f ? _T("OK") : _T("NG"));
	if (f != NULL) {
		if (*font != NULL) {
			TTF_CloseFont(*font);
		}
		*font = f;
		CTchar tbuf(font_path);
		logging->out_logf_x(LOG_INFO, CMsg::MsgBoard_Use_VSTR_for_VSTR, tbuf.Get(), title);
		if (font_file) {
			UTILITY::strcpy(font_file, 128, file_name);
		}
		return true;
	}
	// font not found
	return false;
}
