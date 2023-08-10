/** @file win_emu.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified by Sasaji at 2011.06.17

	@brief [ win32 emulation i/f ]
*/

#include "win_emu.h"
#include "win_main.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../gui/gui.h"
//#include "ledbox.h"
#ifdef USE_MESSAGE_BOARD
#include "../../msgboard.h"
#endif
#include "win_csurface.h"

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

#ifdef USE_DIRECT3D
void EMU_OSD::change_screen_use_direct3d(int num)
{
	const CMsg::Id list[] = {
		CMsg::Direct3D_OFF,
		CMsg::Direct3D_ON_Sync,
		CMsg::Direct3D_ON_Async,
		CMsg::End
	};

	if(enable_direct3d) {
		uint8_t pre_num = pConfig->use_direct3d;

		if (num >= 0) {
			pConfig->use_direct3d = (pre_num == num) ? 0 : num;
		} else {
			pConfig->use_direct3d = (pre_num + 1) % 3;
		}

		lock_screen();

		if (pConfig->use_direct3d != 0) {
			set_d3dpresent_interval();
			reset_d3device(hMainWindow);
		}

		if (pD3Device != NULL && pre_num == 0 && pConfig->use_direct3d != 0) {
#ifdef USE_SCREEN_D3D_TEXTURE
			copy_d3dtex_dib(pD3Dorigin->GetD3DTexture(), sufOrigin->GetBuffer(), false);
#else
			copy_d3dsuf_dib(pD3Dorigin->GetD3DSurface(), sufOrigin->GetBuffer(), false);
#endif
		} else if (pre_num != 0 && pConfig->use_direct3d == 0) {
#ifdef USE_SCREEN_D3D_TEXTURE
			copy_d3dtex_dib(pD3Dorigin->GetD3DTexture(), sufOrigin->GetBuffer(), true);
#else
			copy_d3dsuf_dib(pD3Dorigin->GetD3DSurface(), sufOrigin->GetBuffer(), true);
#endif
		}

		unlock_screen();

		out_infof_x(list[pConfig->use_direct3d]);
//		update_config();
	}
}
#endif
