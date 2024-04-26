/** @file win_hdtypebox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.03.20 -

	@brief [ select hd device type box ]
*/

#ifndef WIN_HDTYPEBOX_H
#define WIN_HDTYPEBOX_H

#include <windows.h>
#include "win_dialogbox.h"
#include "../../config.h"

namespace GUI_WIN
{

/**
	@brief Select hd device type box
*/
class HDTypeBox : public CDialogBox
{
private:
	int m_drive;
	int m_device_type;
	int m_count;

	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	INT_PTR onCommand(UINT message, WPARAM wParam, LPARAM lParam);
	INT_PTR onOK(UINT message, WPARAM wParam, LPARAM lParam);

public:
	HDTypeBox(HINSTANCE, CFont *, EMU *, GUI *, int drv, int device_type);
	~HDTypeBox();
//	void SetDeviceType(int);
	int  GetDeviceType() const;
};

}; /* namespace GUI_WIN */

#endif /* WIN_HDTYPEBOX_H */
