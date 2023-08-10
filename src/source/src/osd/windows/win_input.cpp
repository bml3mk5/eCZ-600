/** @file win_input.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified for BML3MK5/MBS1 by Sasaji at 2011.06.17

	@brief [ win32 input ]
*/

#include "win_emu.h"
#include "win_main.h"
#include "../../vm/vm.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../emu_input.h"
#include "../../utility.h"

void EMU_OSD::EMU_INPUT()
{
#ifdef USE_DIRECTINPUT
	// initialize direct input
	lpdi = NULL;
	lpdikey = NULL;
#endif
#if defined(USE_MOUSE_ABSOLUTE) || defined(USE_MOUSE_FLEXIBLE)
	mouse_position.x = mouse_position.y = 0;
#endif
}

void EMU_OSD::initialize_input()
{
#ifdef USE_DIRECTINPUT
//	if(pConfig->use_direct_input) {
#if DIRECTINPUT_VERSION >= 0x0800
		if(SUCCEEDED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&lpdi, NULL))) {
#else
		if(SUCCEEDED(DirectInputCreate(hInstance, DIRECTINPUT_VERSION, &lpdi, NULL))) {
#endif
			if(SUCCEEDED(lpdi->CreateDevice(GUID_SysKeyboard, &lpdikey, NULL))) {
				if(SUCCEEDED(lpdikey->SetDataFormat(&c_dfDIKeyboard))) {
					if(SUCCEEDED(lpdikey->SetCooperativeLevel(hMainWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
						pConfig->use_direct_input |= 4;
						memset(key_dik_prev, 0, sizeof(key_dik_prev));
					}
				}
			}
		}
//	}
#endif

	EMU::initialize_input();
}

void EMU_OSD::initialize_joystick()
{
#ifdef USE_JOYSTICK
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		memset(&joy_prm[i], 0, sizeof(joy_prm[i]));
		joy_device[i].type = JOY_DEV_DEFAULT;
		joy_device[i].id = -1;
#ifdef USE_JOYSTICK_DIRECTINPUT
		joy_device[i].lp = NULL;
#endif
	}
	memset(&dummy_key, 0, sizeof(dummy_key));
	dummy_key.type = INPUT_KEYBOARD;
	dummy_key.ki.wVk = 0x8f;
	dummy_key.ki.wScan = MapVirtualKey(0x8f, 0);
	dummy_key.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;

	send_dummy_key = 0;
#endif

	EMU::initialize_joystick();
}

#ifdef USE_JOYSTICK_DIRECTINPUT
BOOL CALLBACK EMU_OSD::attach_joy_device_callback(LPCDIDEVICEINSTANCE lpcdevi, LPVOID data)
{
	EMU_OSD *emu = (EMU_OSD *)data;
	return emu->attach_joy_device(lpcdevi);
}

BOOL EMU_OSD::attach_joy_device(LPCDIDEVICEINSTANCE lpcdevi)
{
	HRESULT hr = S_OK;
	LPDIRECTINPUTDEVICE8 lpdijoy = NULL;
	do {
		if (is_xinput_device(&lpcdevi->guidProduct)) {
			// skip xinput device
			break;
		}

		logging->out_logf(LOG_DEBUG, _T("Joypad #%d: Use DINPUT: %s"), joy_nums
			, lpcdevi->tszProductName);

		hr = lpdi->CreateDevice(lpcdevi->guidInstance, &lpdijoy, NULL);
		if (FAILED(hr)) {
			break;
		}
		hr = lpdijoy->SetDataFormat(&c_dfDIJoystick2);
		if (FAILED(hr)) {
			break;
		}

		//
		DIDEVCAPS caps;
		ZeroMemory(&caps, sizeof(caps));
		caps.dwSize = sizeof(caps);
		hr = lpdijoy->GetCapabilities(&caps);
		if (FAILED(hr)) {
			break;
		}
		if (caps.dwPOVs > 0) joy_prm[joy_nums].has_pov = 1; 

		// set absolute axes
		DIPROPDWORD prop;
		ZeroMemory(&prop, sizeof(prop));
		prop.diph.dwSize = sizeof(prop);
		prop.diph.dwHeaderSize = sizeof(prop.diph);
		prop.diph.dwHow = DIPH_DEVICE;
		prop.dwData = DIPROPAXISMODE_ABS;
		hr = lpdijoy->SetProperty(DIPROP_AXISMODE, &prop.diph);
		if (FAILED(hr)) {
			break;
		}

		// get range
		DIPROPRANGE range;
		ZeroMemory(&range, sizeof(range));
		range.diph.dwSize = sizeof(range);
		range.diph.dwHeaderSize = sizeof(range.diph);

		range.diph.dwHow = DIPH_BYOFFSET;
		range.diph.dwObj = DIJOFS_X;
		range.lMin = 0;
		range.lMax = 0;
		hr = lpdijoy->GetProperty(DIPROP_RANGE, &range.diph);
		set_joy_range(SUCCEEDED(hr), range.lMin, range.lMax, pConfig->joy_axis_threshold[joy_nums][0], joy_prm[joy_nums].x);

		range.diph.dwObj = DIJOFS_Y;
		range.lMin = 0;
		range.lMax = 0;
		hr = lpdijoy->GetProperty(DIPROP_RANGE, &range.diph);
		set_joy_range(SUCCEEDED(hr), range.lMin, range.lMax, pConfig->joy_axis_threshold[joy_nums][1], joy_prm[joy_nums].y);

		range.diph.dwObj = DIJOFS_Z;
		range.lMin = 0;
		range.lMax = 0;
		hr = lpdijoy->GetProperty(DIPROP_RANGE, &range.diph);
		set_joy_range(SUCCEEDED(hr), range.lMin, range.lMax, pConfig->joy_axis_threshold[joy_nums][2], joy_prm[joy_nums].z);

		range.diph.dwObj = DIJOFS_RZ;
		range.lMin = 0;
		range.lMax = 0;
		hr = lpdijoy->GetProperty(DIPROP_RANGE, &range.diph);
		set_joy_range(SUCCEEDED(hr), range.lMin, range.lMax, pConfig->joy_axis_threshold[joy_nums][3], joy_prm[joy_nums].r);

		range.diph.dwObj = DIJOFS_RX;
		range.lMin = 0;
		range.lMax = 0;
		hr = lpdijoy->GetProperty(DIPROP_RANGE, &range.diph);
		set_joy_range(SUCCEEDED(hr), range.lMin, range.lMax, pConfig->joy_axis_threshold[joy_nums][4], joy_prm[joy_nums].u);

		range.diph.dwObj = DIJOFS_RY;
		range.lMin = 0;
		range.lMax = 0;
		hr = lpdijoy->GetProperty(DIPROP_RANGE, &range.diph);
		set_joy_range(SUCCEEDED(hr), range.lMin, range.lMax, pConfig->joy_axis_threshold[joy_nums][5], joy_prm[joy_nums].v);

		hr = lpdijoy->SetCooperativeLevel(hMainWindow, DISCL_EXCLUSIVE | DISCL_BACKGROUND);
		if (FAILED(hr)) {
			break;
		}

		hr = lpdijoy->Acquire();
		if (FAILED(hr)) {
			break;
		}

		joy_device[joy_nums].type = JOY_DEV_DINPUT;
		joy_device[joy_nums].id = -1;
		joy_device[joy_nums].lp = lpdijoy;

		joy_enabled[joy_nums] = true;

		joy_nums++;

	} while(0);

	if (FAILED(hr)) {
		if (lpdijoy) {
			lpdijoy->Release();
		}
	}

	return joy_nums < MAX_JOYSTICKS ? DIENUM_CONTINUE : DIENUM_STOP;
}
#endif

void EMU_OSD::reset_joystick()
{
#ifdef USE_JOYSTICK
	if (!use_joystick) return;

#ifdef USE_JOYSTICK_DIRECTINPUT

	for(int i=0; i<MAX_JOYSTICKS; i++) {
		if (joy_device[i].lp) {
			joy_device[i].lp->Unacquire();
			joy_device[i].lp->Release();
			joy_device[i].lp = NULL;
		}
		joy_device[i].type = JOY_DEV_DEFAULT;
		joy_device[i].id = -1;
		joy_enabled[i] = false;
	}

	joy_nums = 0;

	// attach xinput device
	XINPUT_CAPABILITIES cap;
	for(DWORD i=0; i<XUSER_MAX_COUNT && joy_nums < MAX_JOYSTICKS; i++) {
		ZeroMemory(&cap, sizeof(cap));
		DWORD rc = XInputGetCapabilities(i, XINPUT_FLAG_GAMEPAD, &cap);
		if (rc == ERROR_DEVICE_NOT_CONNECTED) {
			continue;
		} else if (rc != ERROR_SUCCESS) {
			break;
		}

		static const _TCHAR *subtype_names[] = {
			_T("Unknown"),
			_T("Gamepad"),
			_T("Wheel"),
			_T("Arcade Stick"),
			_T("Flight Stick"),
			_T("Dance Pad"),
			_T("Guitar"),
			_T("Guitar Alt"),
			_T("Drum Kit"),
		};

		const _TCHAR *name;
		if (cap.SubType < 8) {
			name = subtype_names[cap.SubType];
		} else {
			name = subtype_names[0];
		}
		logging->out_logf(LOG_DEBUG, _T("Joypad #%d: Use XINPUT: %s"), joy_nums
			, name);

		set_joy_range(true, -32768, 32767, pConfig->joy_axis_threshold[i][0], joy_prm[joy_nums].x);
		set_joy_range(true, -32768, 32767, pConfig->joy_axis_threshold[i][1], joy_prm[joy_nums].y);
		set_joy_range(true, -32768, 32767, pConfig->joy_axis_threshold[i][2], joy_prm[joy_nums].z);
		set_joy_range(true, -32768, 32767, pConfig->joy_axis_threshold[i][3], joy_prm[joy_nums].r);
		set_joy_range(false, -32768, 32767, pConfig->joy_axis_threshold[i][4], joy_prm[joy_nums].u);
		set_joy_range(false, -32768, 32767, pConfig->joy_axis_threshold[i][5], joy_prm[joy_nums].v);
		joy_prm[joy_nums].has_pov = 0;

		joy_device[joy_nums].type = JOY_DEV_XINPUT;
		joy_device[joy_nums].id = i;
		joy_enabled[joy_nums] = true;

		joy_nums++;
	}

	if (lpdi && joy_nums < MAX_JOYSTICKS) {
		// attach direct input device
		lpdi->EnumDevices(DI8DEVCLASS_GAMECTRL, &attach_joy_device_callback, this, DIEDFL_ATTACHEDONLY);
	}
#else
	int i = 0;
	int joy_num = joyGetNumDevs();
	for(int joyid = 0; joyid < joy_num && joyid < 16 && i < MAX_JOYSTICKS; joyid++) {
		JOYINFO joyinfo;
		if(joyGetPos(joyid, &joyinfo) == JOYERR_NOERROR) {
			joy_device[i].type = JOY_DEV_DEFAULT;
			joy_device[i].id = joyid;

			JOYCAPS joycaps;
			joyGetDevCaps(joyid, &joycaps, sizeof(JOYCAPS));
			set_joy_range(joycaps.wNumAxes >= 1, joycaps.wXmin, joycaps.wXmax, pConfig->joy_axis_threshold[i][0], joy_prm[i].x);
			set_joy_range(joycaps.wNumAxes >= 2, joycaps.wYmin, joycaps.wYmax, pConfig->joy_axis_threshold[i][1], joy_prm[i].y);
			set_joy_range(joycaps.wNumAxes >= 3, joycaps.wZmin, joycaps.wZmax, pConfig->joy_axis_threshold[i][2], joy_prm[i].z);
			set_joy_range(joycaps.wNumAxes >= 4, joycaps.wRmin, joycaps.wRmax, pConfig->joy_axis_threshold[i][3], joy_prm[i].r);
			set_joy_range(joycaps.wNumAxes >= 5, joycaps.wUmin, joycaps.wUmax, pConfig->joy_axis_threshold[i][4], joy_prm[i].u);
			set_joy_range(joycaps.wNumAxes >= 6, joycaps.wVmin, joycaps.wVmax, pConfig->joy_axis_threshold[i][5], joy_prm[i].v);

			joy_prm[i].has_pov = ((joycaps.wCaps & JOYCAPS_HASPOV) ? 1 : 0);

			logging->out_logf(LOG_DEBUG, _T("Joypad #%d: MID:%04x PID:%04x"), i, joycaps.wMid, joycaps.wPid);
			joy_enabled[i] = true;

			i++;
		}
	}
#endif
#endif
}

void EMU_OSD::release_input()
{
	EMU::release_input();
}

void EMU_OSD::release_joystick()
{
	EMU::release_joystick();

#ifdef USE_JOYSTICK
	// release joystick
	for(int i=0; i<MAX_JOYSTICKS; i++) {
#ifdef USE_JOYSTICK_DIRECTINPUT
		if (joy_device[i].lp) {
			joy_device[i].lp->Unacquire();
			joy_device[i].lp->Release();
			joy_device[i].lp = NULL;
		}
#endif
		joy_device[i].type = JOY_DEV_DEFAULT;
		joy_device[i].id = -1;
	}
#endif
}

void EMU_OSD::update_input()
{
#ifdef USE_DIRECTINPUT
	if(pConfig->use_direct_input == 5) {
		// direct input
		static uint8_t key_dik[256];
		lpdikey->Acquire();
		lpdikey->GetDeviceState(256, key_dik);

#if DIRECTINPUT_VERSION < 0x0800
		// DIK_RSHIFT is not detected on Vista or later
		if(vista_or_later) {
			key_dik[DIK_RSHIFT] = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) ? 0x80 : 0;
		}
#endif
#ifdef USE_SHIFT_NUMPAD_KEY
		// XXX: don't release shift key while numpad key is pressed
		uint8_t numpad_keys;
		numpad_keys  = key_dik[DIK_NUMPAD0];
		numpad_keys |= key_dik[DIK_NUMPAD1];
		numpad_keys |= key_dik[DIK_NUMPAD2];
		numpad_keys |= key_dik[DIK_NUMPAD3];
		numpad_keys |= key_dik[DIK_NUMPAD4];
		numpad_keys |= key_dik[DIK_NUMPAD5];
		numpad_keys |= key_dik[DIK_NUMPAD6];
		numpad_keys |= key_dik[DIK_NUMPAD7];
		numpad_keys |= key_dik[DIK_NUMPAD8];
		numpad_keys |= key_dik[DIK_NUMPAD9];
		if(numpad_keys & 0x80) {
			key_dik[DIK_LSHIFT] |= key_dik_prev[DIK_LSHIFT];
			key_dik[DIK_RSHIFT] |= key_dik_prev[DIK_RSHIFT];
		}
#endif
//		key_dik[DIK_CIRCUMFLEX] |= key_dik[DIK_EQUALS     ];
//		key_dik[DIK_COLON     ] |= key_dik[DIK_APOSTROPHE ];
//		key_dik[DIK_YEN       ] |= key_dik[DIK_GRAVE      ];
//#ifndef USE_NUMPAD_ENTER
//		key_dik[DIK_RETURN    ] |= key_dik[DIK_NUMPADENTER];
//#endif

		for(int vk = 0; vk < 256; vk++) {
			if(dikey2keycode[vk] != 0 && ((key_dik[vk] ^ key_dik_prev[vk]) & 0x80) != 0) {
				key_down_up((1 - (key_dik[vk] >> 7)) | 2, vk, 0x1ff0000);
			}
		}
		memcpy(key_dik_prev, key_dik, sizeof(key_dik_prev));
	} else {
#endif
#ifdef USE_SHIFT_NUMPAD_KEY
		// update numpad key status
		if(key_shift_pressed && !key_shift_released) {
			if(key_status[VK_SHIFT] == 0) {
				// shift key is newly pressed
				key_status[VK_SHIFT] = 0x80;
#ifdef NOTIFY_KEY_DOWN
				vm->key_down(VK_SHIFT, false);
#endif
			}
		}
		else if(!key_shift_pressed && key_shift_released) {
			if(key_status[VK_SHIFT] != 0) {
				// shift key is newly released
				key_status[VK_SHIFT] = 0;
#ifdef NOTIFY_KEY_DOWN
				vm->key_up(VK_SHIFT);
#endif
				// check l/r shift
				if(!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
				if(!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
			}
		}
		key_shift_pressed = key_shift_released = false;
#endif
#ifdef USE_DIRECTINPUT
	}
#endif

	// release keys
#ifdef USE_AUTO_KEY
	if(lost_focus && !autokey_enabled) {
#else
	if(lost_focus) {
#endif
		// we lost key focus so release all pressed keys
		for(int i = 0; i < KEY_STATUS_SIZE; i++) {
			if(key_status[i] & 0x80) {
				key_status[i] &= 0x7f;
				if (!key_status[i]) {
					vm_key_up(vm_key_map[i], VM_KEY_STATUS_KEYBOARD);
				}
#ifdef NOTIFY_KEY_DOWN
				if(!key_status[i]) {
					vm->key_up(i);
				}
#endif
			}
		}
	}
	else {
		for(int i = 0; i < KEY_STATUS_SIZE; i++) {
			if(key_status[i] & 0x7f) {
				key_status[i] = (key_status[i] & 0x80) | ((key_status[i] & 0x7f) - 1);
				if (!key_status[i]) {
					vm_key_up(vm_key_map[i], VM_KEY_STATUS_KEYBOARD);
				}
#ifdef NOTIFY_KEY_DOWN
				if(!key_status[i]) {
					vm->key_up(i);
				}
#endif
			}
		}
	}
	lost_focus = false;

	// update joystick status
	update_joystick();

	// update mouse status
	update_mouse();
}

void EMU_OSD::update_mouse()
{
	memset(mouse_status, 0, sizeof(mouse_status));
#ifdef USE_MOUSE
	if(!mouse_disabled) {
		// get current status
		mouse_status[2]  = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 2 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 4 : 0;

		POINT pt;
#if defined(USE_MOUSE_ABSOLUTE)
		GetCursorPos(&pt);
		ScreenToClient(hMainWindow, &pt);
		conv_mouse_position(&pt);
		mouse_status[0] = (pt.x & 0x1fff) | (pt.x >= mouse_position.x ? 0 : 0x8000);
		mouse_status[1] = (pt.y & 0x1fff) | (pt.y >= mouse_position.y ? 0 : 0x8000);
		mouse_position.x = pt.x;
		mouse_position.y = pt.y;

#elif defined(USE_MOUSE_FLEXIBLE)
		GetCursorPos(&pt);
		mouse_status[0] = pt.x - mouse_position.x;
		mouse_status[1] = pt.y - mouse_position.y;
		mouse_position.x = pt.x;
		mouse_position.y = pt.y;

#else
		GetCursorPos(&pt);
		ScreenToClient(hMainWindow, &pt);
		mouse_status[0]  = pt.x - display_size.w / 2;
		mouse_status[1]  = pt.y - display_size.h / 2;

		// move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
			pt.x = display_size.w / 2;
			pt.y = display_size.h / 2;
			ClientToScreen(hMainWindow, &pt);
			SetCursorPos(pt.x, pt.y);
		}
#endif
	}
#endif
#ifdef USE_LIGHTPEN
	if(FLG_USELIGHTPEN) {
		// get current status
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hMainWindow, &pt);

		if (0 <= pt.x && pt.x < display_size.w && 0 <= pt.y && pt.y < display_size.h) {
			// adjust point
			pt.x = mixed_size.w * (pt.x - stretched_dest_real.x) / stretched_size.w;
			pt.y = mixed_size.h * (pt.y - stretched_dest_real.y) / stretched_size.h;

			mouse_status[0] = pt.x;
			mouse_status[1] = pt.y;
			mouse_status[2] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000 ? 1 : 0) | (GetAsyncKeyState(VK_RBUTTON) & 0x8000 ? 2 : 0);
		}
	}
#endif
}

void EMU_OSD::update_joystick()
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	memset(joy_status, 0, sizeof(joy_status));
#endif
#ifdef USE_JOYSTICK
	memset(joy2joy_status, 0, sizeof(joy2joy_status));
	if (use_joystick) {
		// update joystick status
		for(int i = 0; i < MAX_JOYSTICKS; i++) {
			uint32_t joy_stat = 0;
#ifdef USE_JOYSTICK_DIRECTINPUT
			if (joy_enabled[i]) {
			if (joy_device[i].type == JOY_DEV_DINPUT) {
				DIJOYSTATE2 dijoystate;
				if (SUCCEEDED(joy_device[i].lp->Poll()) && SUCCEEDED(joy_device[i].lp->GetDeviceState(sizeof(dijoystate), &dijoystate))) {
					// digital button on/off
					if((int)dijoystate.lY < joy_prm[i].y.mintd) joy_stat |= JOYCODE_UP;
					if((int)dijoystate.lY > joy_prm[i].y.maxtd) joy_stat |= JOYCODE_DOWN;
					if((int)dijoystate.lX < joy_prm[i].x.mintd) joy_stat |= JOYCODE_LEFT;
					if((int)dijoystate.lX > joy_prm[i].x.maxtd) joy_stat |= JOYCODE_RIGHT;

					if((int)dijoystate.lRz < joy_prm[i].r.mintd) joy_stat |= JOYCODE_R_UP;
					if((int)dijoystate.lRz > joy_prm[i].r.maxtd) joy_stat |= JOYCODE_R_DOWN;
					if((int)dijoystate.lZ < joy_prm[i].z.mintd) joy_stat |= JOYCODE_Z_LEFT;
					if((int)dijoystate.lZ > joy_prm[i].z.maxtd) joy_stat |= JOYCODE_Z_RIGHT;

					if((int)dijoystate.lRy < joy_prm[i].v.mintd) joy_stat |= JOYCODE_V_UP;
					if((int)dijoystate.lRy > joy_prm[i].v.maxtd) joy_stat |= JOYCODE_V_DOWN;
					if((int)dijoystate.lRx < joy_prm[i].u.mintd) joy_stat |= JOYCODE_U_LEFT;
					if((int)dijoystate.lRx > joy_prm[i].u.maxtd) joy_stat |= JOYCODE_U_RIGHT;

					if (joy_prm[i].has_pov) {
						if (!(dijoystate.rgdwPOV[0] & 0x70000000)) {
							switch((dijoystate.rgdwPOV[0] + 50) / 100) {
							case 0:
							case 360:
								joy_stat |= JOYCODE_POV_UP;
								break;
							case 45:
								joy_stat |= JOYCODE_POV_UPRIGHT;
								break;
							case 90:
								joy_stat |= JOYCODE_POV_RIGHT;
								break;
							case 135:
								joy_stat |= JOYCODE_POV_DOWNRIGHT;
								break;
							case 180:
								joy_stat |= JOYCODE_POV_DOWN;
								break;
							case 225:
								joy_stat |= JOYCODE_POV_DOWNLEFT;
								break;
							case 270:
								joy_stat |= JOYCODE_POV_LEFT;
								break;
							case 315:
								joy_stat |= JOYCODE_POV_UPLEFT;
								break;
							}
						}
					}

					uint32_t btn = 0x10000;
					for(int n=0; n<16; n++) {
						if (dijoystate.rgbButtons[n]) joy_stat |= btn;
						btn <<= 1;
					}
//					joy_stat |= ((joyinfoex.dwButtons & 0x0000ffff) << 16);

#ifdef USE_ANALOG_JOYSTICK
					// analog 10bits
					joy2joy_status[i][2] = ((int)dijoystate.lX - joy_prm[i].x.offset) * 1024 / joy_prm[i].x.range + 512;
					joy2joy_status[i][3] = ((int)dijoystate.lY - joy_prm[i].y.offset) * 1024 / joy_prm[i].y.range + 512;
					joy2joy_status[i][4] = ((int)dijoystate.lZ - joy_prm[i].z.offset) * 1024 / joy_prm[i].z.range + 512;
					joy2joy_status[i][5] = ((int)dijoystate.lRz - joy_prm[i].r.offset) * 1024 / joy_prm[i].r.range + 512;
					joy2joy_status[i][6] = ((int)dijoystate.lRx - joy_prm[i].u.offset) * 1024 / joy_prm[i].u.range + 512;
					joy2joy_status[i][7] = ((int)dijoystate.lRy - joy_prm[i].v.offset) * 1024 / joy_prm[i].v.range + 512;
#endif
				} else {
					joy_enabled[i] = false;
				}

			} else if (joy_device[i].type == JOY_DEV_XINPUT) {
				XINPUT_STATE state;
				if (XInputGetState(joy_device[i].id, &state) == ERROR_SUCCESS) {
					// digital button on/off
					if((int)state.Gamepad.sThumbLY < joy_prm[i].y.mintd) joy_stat |= JOYCODE_UP;
					if((int)state.Gamepad.sThumbLY > joy_prm[i].y.maxtd) joy_stat |= JOYCODE_DOWN;
					if((int)state.Gamepad.sThumbLX < joy_prm[i].x.mintd) joy_stat |= JOYCODE_LEFT;
					if((int)state.Gamepad.sThumbLX > joy_prm[i].x.maxtd) joy_stat |= JOYCODE_RIGHT;

					if((int)state.Gamepad.sThumbRY < joy_prm[i].r.mintd) joy_stat |= JOYCODE_R_UP;
					if((int)state.Gamepad.sThumbRY > joy_prm[i].r.maxtd) joy_stat |= JOYCODE_R_DOWN;
					if((int)state.Gamepad.sThumbRX < joy_prm[i].z.mintd) joy_stat |= JOYCODE_Z_LEFT;
					if((int)state.Gamepad.sThumbRX > joy_prm[i].z.maxtd) joy_stat |= JOYCODE_Z_RIGHT;

					joy_stat |= ((state.Gamepad.wButtons & (0xf)) << 8);
					joy_stat |= ((state.Gamepad.wButtons & (0xf000)) << 4);
					joy_stat |= ((state.Gamepad.wButtons & (0x3f0)) << 16);

#ifdef USE_ANALOG_JOYSTICK
					// analog 10bits
					joy2joy_status[i][2] = ((int)state.Gamepad.sThumbLX - joy_prm[i].x.offset) * 1024 / joy_prm[i].x.range + 512;
					joy2joy_status[i][3] = ((int)state.Gamepad.sThumbLY - joy_prm[i].y.offset) * 1024 / joy_prm[i].y.range + 512;
					joy2joy_status[i][4] = ((int)state.Gamepad.sThumbRX - joy_prm[i].z.offset) * 1024 / joy_prm[i].z.range + 512;
					joy2joy_status[i][5] = ((int)state.Gamepad.sThumbRY - joy_prm[i].r.offset) * 1024 / joy_prm[i].r.range + 512;
					joy2joy_status[i][6] = 512;
					joy2joy_status[i][7] = 512;
#endif
				} else {
					joy_enabled[i] = false;
				}
			}
			}
#else
			JOYINFOEX joyinfoex;
			joyinfoex.dwSize = sizeof(JOYINFOEX);
			joyinfoex.dwFlags = JOY_RETURNALL;
			if(joy_enabled[i] && joy_device[i].id != -1) {
				if (joyGetPosEx(joy_device[i].id, &joyinfoex) == JOYERR_NOERROR) {
					// digital button on/off
					if((int)joyinfoex.dwYpos < joy_prm[i].y.mintd) joy_stat |= JOYCODE_UP;
					if((int)joyinfoex.dwYpos > joy_prm[i].y.maxtd) joy_stat |= JOYCODE_DOWN;
					if((int)joyinfoex.dwXpos < joy_prm[i].x.mintd) joy_stat |= JOYCODE_LEFT;
					if((int)joyinfoex.dwXpos > joy_prm[i].x.maxtd) joy_stat |= JOYCODE_RIGHT;

					if((int)joyinfoex.dwRpos < joy_prm[i].r.mintd) joy_stat |= JOYCODE_R_UP;
					if((int)joyinfoex.dwRpos > joy_prm[i].r.maxtd) joy_stat |= JOYCODE_R_DOWN;
					if((int)joyinfoex.dwZpos < joy_prm[i].z.mintd) joy_stat |= JOYCODE_Z_LEFT;
					if((int)joyinfoex.dwZpos > joy_prm[i].z.maxtd) joy_stat |= JOYCODE_Z_RIGHT;

					if((int)joyinfoex.dwVpos < joy_prm[i].v.mintd) joy_stat |= JOYCODE_V_UP;
					if((int)joyinfoex.dwVpos > joy_prm[i].v.maxtd) joy_stat |= JOYCODE_V_DOWN;
					if((int)joyinfoex.dwUpos < joy_prm[i].u.mintd) joy_stat |= JOYCODE_U_LEFT;
					if((int)joyinfoex.dwUpos > joy_prm[i].u.maxtd) joy_stat |= JOYCODE_U_RIGHT;

					if (joy_prm[i].has_pov) {
						switch((joyinfoex.dwPOV + 50) / 100) {
						case 0:
						case 360:
							joy_stat |= JOYCODE_POV_UP;
							break;
						case 45:
							joy_stat |= JOYCODE_POV_UPRIGHT;
							break;
						case 90:
							joy_stat |= JOYCODE_POV_RIGHT;
							break;
						case 135:
							joy_stat |= JOYCODE_POV_DOWNRIGHT;
							break;
						case 180:
							joy_stat |= JOYCODE_POV_DOWN;
							break;
						case 225:
							joy_stat |= JOYCODE_POV_DOWNLEFT;
							break;
						case 270:
							joy_stat |= JOYCODE_POV_LEFT;
							break;
						case 315:
							joy_stat |= JOYCODE_POV_UPLEFT;
							break;
						}
					}

					joy_stat |= ((joyinfoex.dwButtons & 0x0000ffff) << 16);

#ifdef USE_ANALOG_JOYSTICK
					// analog 10bits
					joy2joy_status[i][2] = ((int)joyinfoex.dwXpos - joy_prm[i].x.offset) * 1024 / joy_prm[i].x.range + 512;
					joy2joy_status[i][3] = ((int)joyinfoex.dwYpos - joy_prm[i].y.offset) * 1024 / joy_prm[i].y.range + 512;
					joy2joy_status[i][4] = ((int)joyinfoex.dwZpos - joy_prm[i].z.offset) * 1024 / joy_prm[i].z.range + 512;
					joy2joy_status[i][5] = ((int)joyinfoex.dwRpos - joy_prm[i].r.offset) * 1024 / joy_prm[i].r.range + 512;
					joy2joy_status[i][6] = ((int)joyinfoex.dwUpos - joy_prm[i].u.offset) * 1024 / joy_prm[i].u.range + 512;
					joy2joy_status[i][7] = ((int)joyinfoex.dwVpos - joy_prm[i].v.offset) * 1024 / joy_prm[i].v.range + 512;
#endif
				} else {
					joy_enabled[i] = false;
				}
			}
#endif /* USE_JOYSTICK_DIRECTINPUT */

			if (joy_stat != 0 && send_dummy_key <= 0) {
				// send dummy key event, because delay start time of screen saver.
				SendInput(1, &dummy_key, sizeof(INPUT));
				send_dummy_key = 60;
			} else {
				send_dummy_key -= (send_dummy_key > 0) ? 1 : 0;
			}
			joy2joy_status[i][0] = joy_stat;

			// convert
			convert_joy_status(i);
		}
	}
#endif
#ifdef USE_KEY2JOYSTICK
	if (key2joy_enabled) {
#ifndef USE_PIAJOYSTICKBIT
		// update key 2 joystick status
		for(int i = 0; i < MAX_JOYSTICKS; i++) {
			for(int k = 0; k < 9; k++) {
				joy_status[i][0] |= key2joy_status[i][k];
			}
		}
#else
		for(int i = 0; i < MAX_JOYSTICKS; i++) {
			joy_status[i][0] |= key2joy_status[i];
		}
#endif
	}
#endif
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i = 0; i < MAX_JOYSTICKS; i++) {
		joy_status[i][0] &= joy_mashing_mask[i][joy_mashing_count];
	}
	joy_mashing_count++;
	joy_mashing_count &= 0xf;
#endif
}

/// @param[in] type bit0:0:down 1:up  bit1:0:WM_KEYDOWN/UP 1:direct_input  bit2:1:avail dinput
/// @param[in] code   : VK key code
/// @param[in] status : lParam (scan code etc.)
int EMU_OSD::key_down_up(uint8_t type, int code, long status)
{
//#ifdef USE_DIRECTINPUT
//	if (dinput_key_available && status != 0x1ff0000) {
//		return 1;
//	}
//#endif
	bool keep_frames = false;
	if (!translate_keysym(type, code, status, &code, &keep_frames)) {
		// key down
		if (key_mod & KEY_MOD_ALT_KEY) {
			// notify key down
			code = translate_global_key(code);
			system_key_down(code);
			// execute for pressed global key
			execute_global_keys(code, 0);
			return 0;
		} else {
			key_down(code, keep_frames);
		}
	} else {
		// key up
		if (key_mod & KEY_MOD_ALT_KEY) {
			// notify key up
			code = translate_global_key(code);
			system_key_up(code);
			// release global key
			if (release_global_keys(code, 0)) return 0;
		}
		key_up(code, keep_frames);
	}
	return 1;
}

#ifdef USE_BUTTON
void EMU_OSD::press_button(int num)
{
	int code = buttons[num].code;

	if(code) {
		key_down(code, false);
		key_status[code] = KEY_KEEP_FRAMES * FRAME_SPLIT_NUM;
	}
	else {
		// code=0: reset virtual machine
		vm->reset();
	}
}
#endif

#if 0
void EMU_OSD::initialize_mouse(bool enable)
{
	if (enable) enable_mouse(0);
}
#endif

void EMU_OSD::enable_mouse(int mode)
{
	// enable mouse emulation
	int pmouse_disabled = mouse_disabled;
	mouse_disabled &= (mode ? ~2 : ~1);
	if(pmouse_disabled && !mouse_disabled) {
		// hide mouse cursor
		for (int i = 0; ShowCursor(FALSE) >= 0 && i < 10; i++) {}

		POINT pt;
#if defined(USE_MOUSE_ABSOLUTE)
		GetCursorPos(&pt);
		ScreenToClient(hMainWindow, &pt);
		conv_mouse_position(&pt);
		vm->set_mouse_position(pt.x, pt.y);
		mouse_position.x = pt.x;
		mouse_position.y = pt.y;
#elif defined(USE_MOUSE_FLEXIBLE)
		GetCursorPos(&pt);
		mouse_position.x = pt.x;
		mouse_position.y = pt.y;
#else
		// move mouse cursor to the center of window
		pt.x = display_size.w / 2;
		pt.y = display_size.h / 2;
		ClientToScreen(hMainWindow, &pt);
		SetCursorPos(pt.x, pt.y);
#endif
	}
}

void EMU_OSD::disable_mouse(int mode)
{
	// disable mouse emulation
	if(!mouse_disabled) {
		// show mouse cursor
		for (int i = 0; ShowCursor(TRUE) < 0 && i < 10; i++) {}
	}
	mouse_disabled |= (mode ? 2 : 1);
}

void EMU_OSD::mouse_move(int x, int y)
{
}

void EMU_OSD::mouse_leave()
{
}

#ifdef USE_MOUSE_ABSOLUTE
void EMU_OSD::conv_mouse_position(POINT *pt)
{
	pt->x = (pt->x + 0);
	if (pt->x < 0) pt->x = 0;
	else if (pt->x >= 640) pt->x = 639;
	pt->y = (pt->y - 40);
	if (pt->y < 0) pt->y = 0;
	else if (pt->y >= 400) pt->y = 399;
}
#endif

#if 0 // def USE_MOUSE_ABSOLUTE
void EMU_OSD::set_mouse_position()
{
	POINT pt;
	GetCursorPos(&pt);
//	ScreenToClient(hMainWindow, &pt);
	//
	mouse_position.x = pt.x;
	mouse_position.y = pt.y;
	// convert to vm position
//	mouse_posx = mixed_width  * (mouse_posx + stretched_dest_real.x) / stretched_width - 64;
//	mouse_posy = mixed_height * (mouse_posy + stretched_dest_real.y) / stretched_height - 56;
#if 0
	if (mouse_posx < 0) {
		mouse_posx = 64 * stretched_width / mixed_width - stretched_dest_real.x;
	}
	if (mouse_posx > 639) {
		mouse_posx = (64 + 639) * stretched_width / mixed_width - stretched_dest_real.x;
	}
	if (mouse_posy < 0) {
		mouse_posy = 56 * stretched_height / mixed_height - stretched_dest_real.y;
	}
	if (mouse_posy > 399) {
		mouse_posy = (56 + 399) * stretched_height / mixed_height - stretched_dest_real.y;
	}
#endif
}
#endif

void EMU_OSD::enable_joystick()
{
	// reinitialize
	initialize_joystick();
}

#ifdef USE_JOYSTICK_DIRECTINPUT
// 
bool EMU_OSD::is_xinput_device(const GUID* pGuidProductFromDirectInput)
{
	RAWINPUTDEVICELIST *list = NULL;
	UINT list_count = 0;
	bool rc = false;

	do {
		// get number of raw input devices
		if (GetRawInputDeviceList(NULL, &list_count, sizeof(RAWINPUTDEVICELIST)) == -1) {
			break;
		}
		if (list_count == 0) {
			break;
		}

		list = new RAWINPUTDEVICELIST[list_count];
		ZeroMemory(list, sizeof(RAWINPUTDEVICELIST) * list_count);

		// get list
		if (GetRawInputDeviceList(list, &list_count, sizeof(RAWINPUTDEVICELIST)) == -1) {
			break;
		}

		// matching
		for (UINT i=0; i<list_count; i++) {
			if (list[i].dwType != RIM_TYPEHID) {
				continue;
			}

			RID_DEVICE_INFO rdi;
			ZeroMemory(&rdi, sizeof(rdi));
			rdi.cbSize = sizeof(rdi);
			UINT rdiSize = rdi.cbSize;

			if (GetRawInputDeviceInfoA(list[i].hDevice, RIDI_DEVICEINFO, &rdi, &rdiSize) == ((UINT)-1)) {
				continue;
			}

			if (MAKELONG(rdi.hid.dwVendorId, rdi.hid.dwProductId) != ((LONG)pGuidProductFromDirectInput->Data1)) {
				// not match VID and PID
				continue;
			}

			char devName[128];
			UINT nameSize = 128;
			
			if (GetRawInputDeviceInfoA(list[i].hDevice, RIDI_DEVICENAME, devName, &nameSize) == ((UINT)-1)) {
				continue;
			}
			if (strstr(devName, "IG_") != NULL) {
				// match device
				rc = true;
				break;
			}
		}

	} while(0);

	if (list) {
		delete [] list;
	}

	return rc;
}
#endif