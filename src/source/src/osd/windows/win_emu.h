/** @file win_emu.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified by Sasaji at 2011.06.17

	@brief [ win32 emulation i/f ]

*/

#ifndef WIN_EMU_H
#define WIN_EMU_H

#include <windows.h>

#include "../../common.h"
#include "../../emu.h"
#include "../../vm/vm.h"
#include "../../logging.h"
#include "win_main.h"
#include "../../res/resource.h"
#include "../windowmode.h"
#include "win_screenmode.h"
#include "win_d3d.h"

// DirectX
#define DIRECTSOUND_VERSION 0x900

#ifdef USE_DIRECTINPUT
#define DIRECTINPUT_VERSION	0x800
#include <dinput.h>
#if DIRECTINPUT_VERSION >= 0x0800
#pragma comment(lib, "dinput8.lib")
#else
#pragma comment(lib, "dinput.lib")
#endif
#ifdef USE_JOYSTICK
#include <xinput.h>
#define USE_JOYSTICK_DIRECTINPUT
#define XUSER_MAX_COUNT 4
#pragma comment(lib, "XInput.lib")
#endif
#endif

#define WM_RESIZE  (WM_USER + 1)

#include <dsound.h>
#pragma comment(lib, "dxguid.lib")

#ifdef USE_SOCKET
#include <winsock.h>

class Connection;

#define WM_SOCKET0 (WM_USER + 2)
#define WM_SOCKET1 (WM_USER + 3)
#define WM_SOCKET2 (WM_USER + 4)
#define WM_SOCKET3 (WM_USER + 5)
#define WM_SOCKET4 (WM_USER + 6)
#define WM_SOCKET5 (WM_USER + 7)
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
	void conv_mouse_position(POINT *pt);
#endif
	//@}
	/// @name input private members
	//@{
#ifdef USE_JOYSTICK
	INPUT dummy_key;
	int send_dummy_key;
#endif

#ifdef USE_DIRECTINPUT
#if DIRECTINPUT_VERSION >= 0x0800
	LPDIRECTINPUT8 lpdi;
	LPDIRECTINPUTDEVICE8 lpdikey;
//	LPDIRECTINPUTDEVICE8 lpdijoy;
#else
	LPDIRECTINPUT lpdi;
	LPDIRECTINPUTDEVICE lpdikey;
//	LPDIRECTINPUTDEVICE lpdijoy;
#endif
	uint8_t key_dik_prev[256];
#endif

#ifdef USE_JOYSTICK_DIRECTINPUT
	int joy_nums;
	static BOOL CALLBACK attach_joy_device_callback(LPCDIDEVICEINSTANCE, LPVOID);
	BOOL attach_joy_device(LPCDIDEVICEINSTANCE);
	static bool is_xinput_device(const GUID *);
#endif

#ifdef USE_JOYSTICK
	enum en_joy_dev_type {
		JOY_DEV_DEFAULT = 0,
		JOY_DEV_DINPUT,
		JOY_DEV_XINPUT
	};
	struct st_joy_device {
		en_joy_dev_type type;
		int id;
#ifdef USE_JOYSTICK_DIRECTINPUT
		LPDIRECTINPUTDEVICE8 lp;	///< direct input device
#endif
	} joy_device[MAX_JOYSTICKS];
#endif

#if defined(USE_MOUSE_ABSOLUTE) || defined(USE_MOUSE_FLEXIBLE)
	VmPoint mouse_position;
#endif
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
	void update_screen_dc(HDC hdc);
	inline void calc_vm_screen_size_sub();

#ifdef USE_DIRECT3D
	void initialize_d3device(HWND hWnd);
	HRESULT create_d3device(HWND hWnd);
	void release_d3device();
	void terminate_d3device();

	void update_screen_d3d();
#ifdef USE_SCREEN_D3D_TEXTURE
	inline void copy_d3dtex_dib(PDIRECT3DTEXTURE9 tex, scrntype *buf, bool to_dib);
#else
	inline void copy_d3dsuf_dib(PDIRECT3DSURFACE9 suf, scrntype *buf, bool to_dib);
#endif
	void create_d3dofflinesurface();
	HRESULT reset_d3device(HWND hWnd);
	void set_d3dpresent_interval();
#ifdef USE_SCREEN_D3D_TEXTURE
	void set_d3d_viewport();
#endif
#endif
	//@}
	/// @name screen private members
	//@{
	// screen settings
	DWORD dwStyle;

#if defined(USE_WIN)
	HWND hWindow;
#endif

#ifdef USE_LEDBOX
	LedBox *ledbox;
#endif

	// screen buffer

#ifdef USE_DIRECT3D
	PDIRECT3D9			pD3D;
	D3DPRESENT_PARAMETERS	d3dpp;
	D3DDISPLAYMODE			d3ddm;
	PDIRECT3DDEVICE9	pD3Device;
#ifdef USE_SCREEN_D3D_TEXTURE
	CD3DTexture			*pD3Dsource;
	CD3DTexture			*pD3Dorigin;
#ifdef USE_SCREEN_ROTATE
	CD3DTexture			*pD3Drotate;
#endif
#else
	CD3DSurface			*pD3Dsource;
	CD3DSurface			*pD3Dorigin;
#ifdef USE_SCREEN_ROTATE
	CD3DSurface			*pD3Drotate;
#endif
#ifdef USE_SCREEN_D3D_MIX_SURFACE
	CD3DSurface			*pD3Dmixsuf;
#endif
#endif

	scrntype*			lpD3DBmp;
	bool                lost_d3device;
	bool                enable_direct3d;

	RECT				reD3Dmix;
	RECT				reD3Dsuf;
#ifdef USE_SCREEN_D3D_TEXTURE
	RECT				reD3Dply;
#endif
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
	void start_sound();
	void end_sound();
	DWORD update_sound_th();
	//@}
	/// @name sound private static methods
	//@{
	// callback event
	static DWORD WINAPI sound_proc(LPVOID lpParameter);
	//@}
	/// @name sound private members
	//@{
	// direct sound
	LPDIRECTSOUND lpds;
	LPDIRECTSOUNDBUFFER lpdsb, lpdsp;
	DWORD sound_prev_time;
	// thread and event
	HANDLE sound_nt_event;
	DWORD  sound_threadid;
	HANDLE sound_thread;
	LPDIRECTSOUNDNOTIFY lpdsnt;
	CMutex *mux_sound_nt;
	//@}

	// ----------------------------------------
	// floppy disk
	// ----------------------------------------
#ifdef USE_FD1
	/// @name floppy disk private methods
	//@{
	void initialize_disk_insert();
	void update_disk_insert();
	//@}
#endif

	// ----------------------------------------
	// media
	// ----------------------------------------
#ifdef USE_MEDIA
	/// @name media private methods
	//@{
	void initialize_media();
	void release_media();
	//@}
#endif

	// ----------------------------------------
	// timer
	// ----------------------------------------
	/// @name timer private methods
	//@{
	void update_timer();
	//@}
	/// @name timer private members
	//@{
	SYSTEMTIME sTime;
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
	void initialize();
	void release();
	void release_on_emu_thread();
	//@}

	// ----------------------------------------
	// for windows
	// ----------------------------------------
	/// @name screen menu for ui
	//@{
	void capture_screen();
	void record_rec_video();
#ifdef USE_DIRECT3D
	void change_screen_use_direct3d(int num);
#endif
#ifdef USE_DIRECT3D
	bool enabled_direct3d() const { return enable_direct3d; }
#endif
	//@}

	/// @name input device procedures for host machine
	//@{
	int  key_down_up(uint8_t type, int code, long status);
//	void post_command_message(int id);
	uint8_t translate_keysym(uint8_t type, int code, long status, int *new_code, bool *new_keep_frames = NULL);
	uint8_t translate_keysym(uint8_t type, int code, short scan_code, int *new_code, bool *new_keep_frames = NULL);

	void reset_joystick();
	void update_joystick();

	void update_mouse();
	void enable_mouse(int mode);
	void disable_mouse(int mode);

	void mouse_move(int x, int y);
	void mouse_leave();

	void enable_joystick();
	//@}
	/// @name screen device procedures for host machine
	//@{
	void resume_window_placement();
	bool create_screen(int disp_no, int x, int y, int width, int height, uint32_t flags);
	void set_display_size(int width, int height, int power, bool now_window);
	void draw_screen();
	bool mix_screen();
	void update_screen(HDC hdc);
	void skip_screen(bool val) {
		skip_frame = val;
	}
	void need_update_screen();

	void set_client_pos(int dest_x, int dest_y, int client_width, int client_height, UINT flags);
	void set_window(int mode, int cur_width, int cur_height);
	bool create_offlinesurface();

#ifdef USE_DIRECT3D
	PDIRECT3DDEVICE9 get_d3device() { return pD3Device; }
#endif
	/// @return HWND : handle of main window
	HWND get_window() {
		return hWindow;
	}
	//@}
	/// @name sound device procedures for host machine
	//@{
	void mute_sound(bool mute);
	uint32_t adjust_sound_pos(uint32_t msec);
	//@}
#ifdef USE_SOCKET
	/// @name socket device procedures for host machine
	//@{
	const void *get_socket(int ch) const;
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
//	bool get_ipaddr(const _TCHAR *hostname, int port, uint32_t *ipaddr);
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
	/// @name for debug
	//@{
	void sleep(uint32_t ms);
	//@}
};

#endif /* WIN_EMU_H */
