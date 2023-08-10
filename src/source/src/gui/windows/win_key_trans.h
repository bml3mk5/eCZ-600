/** @file win_key_trans.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.04.30 -

	@brief [ win to SDL key translate ]
*/

#ifndef WIN_KEY_TRANS_H
#define WIN_KEY_TRANS_H

#include <Windows.h>
#include "../../common.h"
#include <SDL.h>

namespace GUI_WIN
{

//void SDL_DIB_InitOSKeymap();
UINT32 translate_to_sdlkey(WPARAM wParam, LPARAM lParam);

}; /* namespace GUI_WIN */

#endif /* WIN_KEY_TRANS_H */
