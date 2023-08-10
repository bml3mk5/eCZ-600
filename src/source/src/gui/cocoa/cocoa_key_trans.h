/** @file cocoa_key_trans.h

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.04.30 -

 @note based on SDL_mackeys.h

 @brief [ key translate ]
 */

#ifndef COCOA_KEY_TRANS_H
#define COCOA_KEY_TRANS_H

#include <Cocoa/Cocoa.h>
#include <SDL.h>

void SDL_QZ_InitOSKeymap();
int  SDL_QZ_HandleKeyEvents(NSEvent *event);

#endif /* COCOA_KEY_TRANS_H */
