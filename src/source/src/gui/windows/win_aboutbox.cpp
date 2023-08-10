/** @file win_aboutbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.01.21 -

	@brief [ about box ]
*/

#include "win_aboutbox.h"
#include "../gui.h"
#include "../../version.h"

namespace GUI_WIN
{

AboutBox::AboutBox(HINSTANCE hinst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: CDialogBox(hinst, IDD_ABOUTBOX, new_font, new_emu, new_gui)
{
}

AboutBox::~AboutBox()
{
}

INT_PTR AboutBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	CDialogBox::onInitDialog(message, wParam, lParam);

	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin);
	CBox *box_hall = new CBox(CBox::HorizontalBox);

	box_all->AddBox(box_hall);

	CBox *box_icon = new CBox(CBox::VerticalBox, CBox::MiddlePos, 16);
	box_hall->AddBox(box_icon);
	AdjustControl(box_icon, IDC_STATIC_1, 32, 32);

	CBox *box_info = new CBox(CBox::VerticalBox);
	box_hall->AddBox(box_info);
	AdjustStatic(box_info, IDC_STATIC_2);

	// version
	char buf[_MAX_PATH];
	sprintf(buf, "Version %s \"%s\"", APP_VERSION, PLATFORM);
#ifdef _DEBUG
	strcat(buf, " (DEBUG Version)");
#endif
	CreateStatic(box_info, IDC_STATIC_4, buf);
	// edition
	emu->get_edition_string(buf, _MAX_PATH);
	if (buf[0] != '\0') {
		CreateStatic(box_info, IDC_STATIC_5, buf);
	}
	sprintf(buf,
#ifdef _MSC_VER
		" using VisualC++ %d", _MSC_VER);
#elif __MINGW32__
		" using MinGW");
#else
		" using unknown compiler");
#endif
	if (buf[0] != '\0') {
		CreateStatic(box_info, IDC_STATIC_6, buf);
	}
#if defined(USE_SDL) || defined(USE_SDL2)
	gui->GetLibVersionString(buf, _MAX_PATH, _T(", "));
	CreateStatic(box_info, IDC_STATIC_7, buf);
#endif

	AdjustStatic(box_info, IDC_STATIC_3);

	CBox *box_btn = new CBox(CBox::HorizontalBox, CBox::RightPos, padding);
	box_all->AddBox(box_btn);
	AdjustButton(box_btn, IDOK, 8);

	box_all->Realize(*this);

	delete box_all;

	return (INT_PTR)TRUE;
}

}; /* namespace GUI_WIN */
