/** @file sdl_emu.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2012.02.21

	@brief [ sdl emulation i/f ]

	@note
	This code is based on the Common Source Code Project.
	Original Author : Takeda.Toshiya
*/

#include "sdl_emu.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../gui/gui.h"
//#include "ledbox.h"
#ifdef USE_MESSAGE_BOARD
#include "../../msgboard.h"
#endif
#include "sdl_csurface.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

EMU_OSD::EMU_OSD(const _TCHAR *new_app_path, const _TCHAR *new_ini_path, const _TCHAR *new_res_path)
	: EMU(new_app_path, new_ini_path, new_res_path)
{
	EMU_INPUT();
	EMU_SCREEN();
	EMU_SOUND();
#ifdef USE_SOCKET
	EMU_SOCKET();
#endif
#ifdef USE_UART
	EMU_UART();
#endif
}

EMU_OSD::~EMU_OSD()
{
}

void EMU_OSD::sleep(uint32_t ms)
{
	CDelay(ms);
}

#ifdef USE_OPENGL
void EMU_OSD::change_screen_use_opengl(int num)
{
	CMsg::Id need_restart = CMsg::Null;
	const CMsg::Id list[] = {
		CMsg::OpenGL_OFF,
		CMsg::OpenGL_ON_Sync,
		CMsg::OpenGL_ON_Async,
		CMsg::End
	};

	if (num >= 0) {
		pConfig->use_opengl = (pConfig->use_opengl == num) ? 0 : num;
	} else {
		pConfig->use_opengl = (pConfig->use_opengl + 1) % 3;
	}

	if (pConfig->use_opengl != next_use_opengl) {
		need_restart = CMsg::LB_Need_restart_program_RB;
	}
	out_infoc_x(list[pConfig->use_opengl], need_restart, 0);

#ifdef OPENGL_IMMCHANGE
// warning: cannot change immediate under agar gui
	use_opengl = pConfig->use_opengl;
	if (!create_screen(pConfig->disp_device_no, 0, 0, pConfig->screen_width, pConfig->screen_height, screen_flags)) {
		exit(1);
	}
	lock_screen();
	update_config();
	unlock_screen();
#endif
}
#endif
