/** @file sdl_msgboard.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.26 -

	@brief [ message board ]
*/

#ifndef SDL_MSGBOARD_H
#define SDL_MSGBOARD_H

#include "../../common.h"
#include <list>
#include <SDL.h>
#include <SDL_ttf.h>
#include "sdl_ccolor.h"
#include "../../cmutex.h"
#include "../../msgs.h"
#ifdef USE_GTK
#include <cairo/cairo.h>
#endif
#ifdef USE_OPENGL
#include "../opengl.h"
#endif

class EMU;
class CSurface;
class CTexture;
class CPixelFormat;

#define MSGBOARD_STR_SIZE	512

/**
	@brief MsgBoard is the class that display various messages on the screen.
*/
class MsgBoard
{
private:
	EMU *emu;
	CSurface *sMainSuf;

	CColor fg;
	CColor bg;

	struct {
		int cx;
		int cy;
	} szWin;

	typedef struct msg_item_st {
		char cmsg[MSGBOARD_STR_SIZE];
		int cnt;
	} item_t;

	typedef std::list<item_t> list_t;

	typedef struct msg_data_st {
		SDL_Rect re;	// 描画枠
		struct {
			int cx;
			int cy;
		} sz;			// 描画した文字の幅、高さ
		struct {
			int x;
			int y;
		} pt;			// 描画先の位置
		int place;		// ptの基準 0:左上 1:右上 2:左下 3:右下
		TTF_Font *font;
		char font_name[128];
		CMutex *mux;
		list_t lists;
	} msg_data_t;

	msg_data_t msg;
	msg_data_t info;

	bool visible;
	bool enable;

	bool inited;

	typedef struct sbuf_st {
		char buf[1024];
	} sbuf_t;

	// 基準位置の計算
	inline void calc_place(msg_data_t &data, SDL_Rect &reDst);
	// 文字列出力
	void draw(CSurface &screen, msg_data_t &data);
	void draw(SDL_Surface &screen, msg_data_t &data);
#ifdef USE_GTK
	void draw(cairo_t *screen, msg_data_t &data);
#endif
#if defined(USE_SDL2)
	void draw(CTexture &texture, msg_data_t &data);
#endif
#ifdef USE_OPENGL
	void draw(COpenGLTexture &texture, msg_data_t &data);
#endif
	// 文字列をバックバッファに描画
	void draw_text(msg_data_t &data);
	void draw_text(CSurface *suf, msg_data_t &data, int left, int top);
	// カウントダウン
	void count_down(msg_data_t &data);

	// 画面表示用システムフォントの設定
	bool set_sys_font(CMsg::Id title, const _TCHAR *name, int pt, TTF_Font **font, char *font_file);
	// フォントを開く
	bool open_font_subdir(TTF_Font **font, const char *path, const char *file_name, int pt, const _TCHAR *title, char *font_file, int depth);
	// フォントを開く
	bool open_font(TTF_Font **font, const char *path, const char *file_name, int pt, const _TCHAR *title, char *font_file);
public:
	MsgBoard(EMU *pEmu);
	~MsgBoard();

	// 初期化
	bool CreateSurface(CPixelFormat *format, int width, int height);
	void InitScreen(CPixelFormat *format, int width, int height);

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
	void Draw(CSurface &screen);
	void Draw(SDL_Surface &screen);
#ifdef USE_GTK
	void Draw(cairo_t *screen);
#endif
#if defined(USE_SDL2)
	void Draw(CTexture &texture);
#endif
#ifdef USE_OPENGL
	void Draw(COpenGLTexture &texture);
#endif

	// フォント設定
	bool SetFont();

	// 有効？
	bool IsEnable() { return enable; }
};

#endif /* SDL_MSGBOARD_H */
