/** @file sdl_emu.h

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2012.02.21

	@brief [ sdl emulation i/f ]

	@note
	This code is based on the Common Source Code Project.
	Original Author : Takeda.Toshiya
*/

#ifndef SDL_EMU_H
#define SDL_EMU_H

#include "../../common.h"
#include "../../depend.h"
#include <SDL.h>
#include <time.h>
#include "../../emu.h"
#include "../../vm/vm.h"
#include "../../logging.h"
#include "../../res/resource.h"

#ifdef USE_GTK
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo/cairo.h>
# if GTK_CHECK_VERSION(3,22,0)
#define USE_MOUSE_FLEXIBLE
# endif
# ifdef USE_OPENGL
#  if GTK_CHECK_VERSION(3,16,0)
#define USE_GTK_GLAREA
#  else
#define GtkGLArea void
#define GdkGLContext void
#  endif
# endif
#endif

// OpenGL
#ifdef USE_OPENGL
#include "../opengl.h"
#endif

#define SDL_USEREVENT_COMMAND (SDL_USEREVENT + 1)

#ifdef USE_SOCKET
class Connection;

#define SDL_USEREVENT_SOCKET (SDL_USEREVENT_COMMAND + 1)
#define SDL_USEREVENT_SOCKET_CONNECTED 1
#define SDL_USEREVENT_SOCKET_DISCONNECTED 2
#define SDL_USEREVENT_SOCKET_WRITEABLE 3

#if defined(USE_WX) || defined(USE_WX2)
#define SDL_USEREVENT_SOCKET_READABLE 4
#define SDL_USEREVENT_SOCKET_ACCEPT 5
#endif
#endif /* USE_SOCKET */

#ifdef USE_UART
class CommPorts;
#endif

class FIFO;
class FILEIO;
class GUI;
#ifdef USE_MESSAGE_BOARD
class MsgBoard;
#endif
#ifdef USE_LEDBOX
class LedBox;
#endif
class CSurface;
class CPixelFormat;
class REC_VIDEO;
class REC_AUDIO;
#if defined(USE_SDL2) || defined(USE_WX2)
class CTexture;
#endif
#ifdef USE_OPENGL
class COpenGLTexture;
#endif
#if defined(USE_WX) || defined(USE_WX2)
class wxWindow;
class wxBitmap;
class wxImage;
class wxMemoryDC;
class wxDateTime;
class wxMouseState;
class MyPanel;
class MyGLCanvas;
#if defined(USE_JOYSTICK) && !defined(USE_SDL_JOYSTICK)
class wxJoystick;
#endif
#endif

/**
	@brief emulation i/f
*/
class EMU_OSD : public EMU
{
protected:

private:
	// ----------------------------------------
	// input
	// ----------------------------------------
	/// @name input private methods
	//@{
	void EMU_INPUT();
	void initialize_input();
	void release_input();
	void update_input();

	void initialize_joystick();
	void release_joystick();

#ifdef USE_MOUSE_ABSOLUTE
//	void set_mouse_position();
#endif

#ifdef USE_GTK
	inline GdkModifierType get_pointer_state_on_window(GtkWidget *window, VmPoint *pt, GdkDevice **device = NULL);
	inline void set_pointer_on_window(GtkWidget *window, GdkDevice *ddevice, VmPoint *pt);
#endif
	//@}
	/// @name input private members
	//@{
	bool pressed_global_key;

#ifdef USE_JOYSTICK
	SDL_Joystick *joy[MAX_JOYSTICKS];
#endif

	enum en_mouse_logic_type {
		MOUSE_LOGIC_DEFAULT = 0,
		MOUSE_LOGIC_FLEXIBLE,
		MOUSE_LOGIC_PREPARE
	} mouse_logic_type;
	VmPoint mouse_position;
	//@}

private:
	// ----------------------------------------
	// screen
	// ----------------------------------------
	/// @name screen private methods
	//@{
	void EMU_SCREEN();
	void initialize_screen();
	void release_screen();
#if defined(USE_SDL2)
	bool create_sdl_texture();
#endif
	void set_screen_filter_type();
	inline void mix_screen_sub();
	inline void calc_vm_screen_size_sub();

#ifdef USE_OPENGL
	void initialize_opengl();
	void release_opengl();

	void create_opengl_texture();
	void release_opengl_texture();

	void set_opengl_attr();
	void set_opengl_poly(int width, int height);

# ifdef USE_GTK
	void realize_window(GtkWidget *);
	void realize_opengl(GtkGLArea *);
	void unrealize_opengl(GtkGLArea *);
# endif
#endif /* USE_OPENGL */
	//@}
	/// @name screen private static methods
	//@{
#ifdef USE_GTK
	static void on_realize(GtkWidget *, gpointer);
	static gboolean on_draw(GtkWidget *, cairo_t *, gpointer);
	static gboolean on_expose(GtkWidget *, GdkEvent *, gpointer);
# ifdef USE_OPENGL
	static void on_gl_realize(GtkGLArea *, gpointer);
	static void on_gl_unrealize(GtkGLArea *, gpointer);
	static gboolean on_gl_render(GtkGLArea *, GdkGLContext *, gpointer);
# endif
#endif /* USE_GTK */
	//@}
	/// @name screen private members
	//@{
	// screen settings
#if defined(USE_GTK)
	GtkWidget *screen;
	GtkWidget *window;
	cairo_surface_t *surface;
	GdkCursor *bkup_cursor;
# ifdef USE_OPENGL
	GtkWidget *glscreen;
# endif

#elif defined(USE_SDL2)
	/* SDL2 */
	SDL_Window *window;
	SDL_Renderer *renderer;
	CTexture *texMixed;
#ifdef USE_SCREEN_SDL2_MIX_ON_RENDERER
	CTexture *texLedBox;
	CTexture *texMsgBoard;
#endif
# ifdef USE_OPENGL
	SDL_GLContext glcontext;
# endif

#else
	/* SDL1 */
	SDL_Surface *screen;
	VmSize szVmDispArea;

#endif

#ifdef USE_OPENGL
	COpenGL *opengl;
#endif
	Uint32 screen_flags;

	VmSize mixed_max_size;	// for OpenGL

	// screen buffer
	scrntype* lpBmp;

	VmRectWH reMix;
	VmRectWH reSuf;

#ifdef USE_OPENGL
	COpenGLTexture *texGLMixed;
//	GLuint mix_texture_name;
	VmRect	rePyl;
	GLfloat src_tex_l, src_tex_t, src_tex_r, src_tex_b;
	GLfloat src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b;
	int use_opengl;
	uint8_t next_use_opengl;

#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	COpenGLTexture *texGLLedBox;
	COpenGLTexture *texGLMsgBoard;
#endif
#endif
#ifdef USE_LEDBOX
	LedBox *ledbox;
#endif
	//@}

	// ----------------------------------------
	// sound
	// ----------------------------------------
	/// @name sound private methods
	//@{
	void EMU_SOUND();
	void initialize_sound(int rate, int samples, int latency);
	void initialize_sound();
	void release_sound();
	void release_sound_on_emu_thread();
	void start_sound();
	void end_sound();
	//@}
	/// @name sound private static methods
	//@{
	// callback
	static void update_sound(void *userdata, uint8_t *stream, int len);
	//@}
	/// @name sound private members
	//@{
	SDL_AudioSpec audio_desired, audio_obtained;
#if defined(USE_SDL2) || defined(USE_WX2)
	SDL_AudioDeviceID audio_devid;
#endif
	// direct sound
	uint32_t sound_prev_time;
	//@}

	// ----------------------------------------
	// timer
	// ----------------------------------------
	/// @name timer private methods
	//@{
	void update_timer();
	//@}
	/// @name timer private members
	//@{
	struct tm sTime;
	//@}

	// ----------------------------------------
	// socket
	// ----------------------------------------
#ifdef USE_SOCKET
	/// @name socket private methods
	//@{
	void EMU_SOCKET();
	void initialize_socket();
	void release_socket();
	void update_socket();
	//@}
	/// @name socket private members
	//@{
	Connection *conn;
	//@}
#endif

	// ----------------------------------------
	// uart
	// ----------------------------------------
#ifdef USE_UART
	/// @name uart private methods
	//@{
	void EMU_UART();
	void initialize_uart();
	void release_uart();
	void update_uart();
	//@}
	/// @name socket private members
	//@{
	CommPorts *coms;
	//@}
#endif

//
//
//
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	/// @name initialize
	//@{
	EMU_OSD(const _TCHAR *new_app_path, const _TCHAR *new_ini_path, const _TCHAR *new_res_path);
	~EMU_OSD();
	//@}

	// ----------------------------------------
	// for windows
	// ----------------------------------------
	/// @name screen menu for ui
	//@{
	void change_screen_mode(int mode);
	void capture_screen();
	bool start_rec_video(int type, int fps_no, bool show_dialog);
	void record_rec_video();
#ifdef USE_OPENGL
	void change_screen_use_opengl(int num);
	void change_opengl_attr();
	int now_use_opengl() const {
		return use_opengl;
	}
#endif
	//@}
	/// @name input device procedures for host machine
	//@{
	int  key_down_up(uint8_t type, int code, long status);
	uint8_t translate_keysym(uint8_t type, int code, short scan_code, int *new_code, bool *new_keep_frames = NULL);

	void reset_joystick();
	void update_joystick();

	void update_mouse();
	void enable_mouse(int mode);
	void disable_mouse(int mode);
//#ifdef USE_GTK
//	void update_mouse_event(GdkEventMotion *event);
//#endif
	void mouse_enter();
	void mouse_move(int x, int y);
	void mouse_leave();
	//@}
	/// @name screen device procedures for host machine
	//@{
	void resume_window_placement();
	bool create_screen(int disp_no, int x, int y, int width, int height, uint32_t flags);
	void set_display_size(int width, int height, int power, bool now_window);
	void draw_screen();
#ifdef USE_GTK
	bool mix_screen_pa(cairo_t *);
	void update_screen_pa(cairo_t *);
# ifdef USE_OPENGL
	bool mix_screen_gl(GdkGLContext *);
	void update_screen_gl(GdkGLContext *);
# endif
#else
	bool mix_screen();
	void update_screen();
#endif
	void need_update_screen();

	void set_window(int mode, int cur_width, int cur_height);
	bool create_offlinesurface();

#ifdef USE_OPENGL
	int  get_use_opengl() const {
		return use_opengl;
	}
	void set_use_opengl(int val) {
		use_opengl = val;
	}
#endif

#ifndef USE_GTK
#ifndef USE_SDL2
	/* SDL1 */
	/// when using SDL1 on main window
	/// @return SDL_Surface * : main screen
	SDL_Surface *get_screen() {
		return screen;
	}
#else
	/* SDL2 */
	/// when using SDL2 on main window
	/// @return SDL_Window * : main window
	SDL_Window *get_window() {
		return window;
	}
#endif
#else
	/// when using GTK+ on main window
	/// @return GtkWidget * : main window
	GtkWidget *get_window() {
		return window;
	}
	/// when using GTK+ on main window
	/// @return GtkWidget * : main screen panel
	GtkWidget *get_screen() {
#ifdef USE_OPENGL
		return (use_opengl ? glscreen : screen);
#else
		return screen;
#endif
	}
#endif
	//@}
	/// @name sound device procedures for host machine
	//@{
	void mute_sound(bool mute);
	//@}
#ifdef USE_SOCKET
	/// @name socket device procedures for host machine
	//@{
//	const void *get_socket(int ch) const;
	void check_socket();
	void socket_connected(int ch);
	void socket_disconnected(int ch);
	void socket_writeable(int ch);
	void socket_readable(int ch);
	void socket_accept(int ch);
	void send_data(int ch);
	void recv_data(int ch);
	//@}
#endif
#ifdef USE_UART
	/// @name uart device procedures for host machine
	//@{
	int  enum_uarts();
	void get_uart_description(int ch, _TCHAR *buf, size_t size);
	//@}
#endif

	// ----------------------------------------
	// for virtual machine
	// ----------------------------------------
	/// @name screen for vm
	//@{
	scrntype* screen_buffer(int y);
	int screen_buffer_offset();
	void set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect);
	//@}
	/// @name sound for vm
	//@{
	void lock_sound_buffer();
	void unlock_sound_buffer();
	//@}
	/// @name timer for vm
	//@{
	void get_timer(int *time, size_t size) const;
	//@}
#ifdef USE_SOCKET
	/// @name socket for vm
	//@{
	bool init_socket_tcp(int ch, DEVICE *dev, bool server = false);
	bool init_socket_udp(int ch, DEVICE *dev, bool server = false);
	bool connect_socket(int ch, uint32_t ipaddr, int port, bool server = false);
	bool connect_socket(int ch, const _TCHAR *hostname, int port, bool server = false);
	bool is_connecting_socket(int ch);
//	bool connect_socket(int ch, IPaddress *ip, bool server = false);
//	bool get_ipaddr(const _TCHAR *hostname, int port, uint32_t *ip);
//	bool get_ipaddr_(const _TCHAR *hostname, int port, IPaddress *ip);
	void disconnect_socket(int ch);
	int  get_socket_channel();
	bool listen_socket(int ch);
	void send_data_tcp(int ch);
	void send_data_udp(int ch, uint32_t ipaddr, int port);
	void send_data_udp(int ch, const _TCHAR *hostname, int port);
	//@}
#endif
#ifdef USE_UART
	/// @name uart for vm
	//@{
	bool init_uart(int ch, DEVICE *dev);
	bool open_uart(int ch);
	bool is_opened_uart(int ch);
	void close_uart(int ch);
	int  send_uart_data(int ch, const uint8_t *data, int size);
	void send_uart_data(int ch);
	void recv_uart_data(int ch);
	//@}
#endif
#ifdef USE_EMU_INHERENT_SPEC
	/// @name send message to ui from vm
	//@{
	void update_ui(int flags);
	//@}
#endif
	/// @name for debug
	//@{
	void sleep(uint32_t ms);
	//@}
};

#endif /* SDL_EMU_H */
