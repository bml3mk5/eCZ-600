/** @file win_msgboard.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.26 -

	@brief [ message board ]
*/

#ifndef _WIN_MSGBOARD_H_
#define _WIN_MSGBOARD_H_

#include <windows.h>
#include <tchar.h>
#include <list>
#include "win_csurface.h"
#include "win_ccolor.h"
#include "win_d3d.h"
#include "../../gui/windows/winfont.h"
#include "../../cmutex.h"
#include "../../msgs.h"

class EMU;

#define MSGBOARD_STR_SIZE	512

/**
	@brief MsgBoard is the class that display various messages on the screen.
*/
class MsgBoard : public CSurface
{
private:
	HPEN   hPen;
	HBRUSH hBrush;

	CColor fg;
	CColor bg;

	SIZE szWin;

	typedef struct msg_item_st {
		_TCHAR msg[MSGBOARD_STR_SIZE];
		int cnt;
	} item_t;

	typedef std::list<item_t> list_t;

	typedef struct msg_data_st {
		RECT re;	// 描画枠
		SIZE sz;	// 描画した文字の幅、高さ
		POINT pt;	// 描画先の位置
		int place;	// ptの基準 0:左上 1:右上 2:左下 3:右下
		CFont *font;
		CMutex *mux;
		list_t lists;
	} msg_data_t;

	msg_data_t msg;
	msg_data_t info;

#ifdef USE_SCREEN_D3D_TEXTURE
	CD3DTexture tex_msg;
	CD3DTexture tex_info;
#endif

	bool inited;
	bool visible;

	EMU *emu;

	// 文字列出力
	void draw(HDC hdc, msg_data_t &data);
	void draw(LPDIRECT3DSURFACE9 suf, msg_data_t &data);
	void draw(PDIRECT3DDEVICE9 device, msg_data_t &data, CD3DTexture &tex);
	// 文字列をバックバッファに描画
	void draw_text(msg_data_t &data);
	void draw_text(HDC hdc, msg_data_t &data, int left, int top);
	// カウントダウン
	void count_down(msg_data_t &data);

public:
	MsgBoard(HWND, EMU *);
	~MsgBoard();

	// 初期化
	void InitScreen(int width, int height);

	// 表示
	void SetVisible(bool val) {
		visible = val;
	}
	// カウントダウン
	void CountDown(void);

	// メッセージ設定
	void Set(msg_data_t &data, const _TCHAR *str, int sec);
	void Set(msg_data_t &data, CMsg::Id id, int sec);
	// メッセージ削除
	void Delete(msg_data_t &data, const _TCHAR *str);
	void Delete(msg_data_t &data, CMsg::Id id);
	// ウィンドウサイズ設定
	void SetSize(int width, int height);

	// メッセージ位置設定
	void SetMessagePos(int cx, int cy, int place);
	// メッセージ設定
	void SetMessage(const _TCHAR *str, int sec = 5);
	void SetMessage(CMsg::Id id, int sec = 5);
	void SetMessageF(const _TCHAR *format, ...);
	// メッセージ削除
	void DeleteMessage(const _TCHAR *str);
	void DeleteMessage(CMsg::Id id);

	// 情報位置設定
	void SetInfoPos(int cx, int cy, int place);
	// 情報設定
	void SetInfo(const _TCHAR *str, int sec = 2);
	void SetInfo(CMsg::Id id, int sec = 2);
	void SetInfoF(const _TCHAR *format, ...);
	// 情報削除
	void DeleteInfo(const _TCHAR *str);
	void DeleteInfo(CMsg::Id id);

	// 文字列出力
	void Draw(HDC hdc);
	void Draw(LPDIRECT3DSURFACE9 suf);
	void Draw(PDIRECT3DDEVICE9 device);

	// テクスチャ作成
	HRESULT CreateTexture(PDIRECT3DDEVICE9 device);
	void ReleaseTexture();

	// フォント取得
	bool SetFont();
	CFont *GetMsgFont() { return msg.font; }
	CFont *GetInfoFont() { return info.font; }
};

#endif
