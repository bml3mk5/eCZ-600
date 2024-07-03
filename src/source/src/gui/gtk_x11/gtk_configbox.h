/** @file gtk_configbox.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ config box ]
*/

#ifndef GUI_GTK_CONFIGBOX_H
#define GUI_GTK_CONFIGBOX_H

#include "../../common.h"
#include <gtk/gtk.h>
#include "gtk_dialogbox.h"
#include "../../vm/vm_defs.h"
#include "../../config.h"
#include "../../cchar.h"
#include "../../cptrlist.h"

namespace GUI_GTK_X11
{

/**
	@brief Configure dialog box
*/
class ConfigBox : public DialogBox
{
private:

	GtkWidget *txtROMPath;
#if defined(_X68000)
	GtkWidget *comRamSize;
	GtkWidget *chkOwSizeSRAM;
	GtkWidget *chkSRAMClear;
	GtkWidget *chkSRAMSave;
	GtkWidget *chkSRAMChanged;
	GtkWidget *chkAddrErr;
	GtkWidget *txtSramRomAddr;
	GtkWidget *txtSramSramAddr;
	GtkWidget *comSramBootDevice;
	GtkWidget *comSramRS232CBaud;
	GtkWidget *comSramRS232CDataBit;
	GtkWidget *comSramRS232CParity;
	GtkWidget *comSramRS232CStopBit;
	GtkWidget *comSramRS232CFlowCtrl;
	GtkWidget *spnSramContrast;
	GtkWidget *chkSramFdEject;
	GtkWidget *comSramPurpose;
	GtkWidget *comSramKRDelay;
	GtkWidget *comSramKRRate;
	GtkWidget *chkSramAlarm;
	GtkWidget *chkSramKLEDkana;
	GtkWidget *chkSramKLEDromaji;
	GtkWidget *chkSramKLEDcinput;
	GtkWidget *chkSramKLEDcaps;
	GtkWidget *chkSramKLEDins;
	GtkWidget *chkSramKLEDhira;
	GtkWidget *chkSramKLEDzen;
#endif
	GtkWidget *chkPowerOff;
	GtkWidget *comPowerState;

	GtkWidget *comUseOpenGL;
	GtkWidget *comGLFilter;

#if defined(_X68000)
	GtkWidget *txtRasterSkewV;
	GtkWidget *txtRasterSkewH;
	GtkWidget *txtVertSkew;
#endif

	GtkWidget *comLEDShow;
	GtkWidget *comLEDPos;

	GtkWidget *comCapType;

	GtkWidget *txtSnapPath;
	GtkWidget *txtFontPath;
	GtkWidget *txtMsgFontName;
	GtkWidget *txtMsgFontSize;
	GtkWidget *txtInfoFontName;
	GtkWidget *txtInfoFontSize;

#ifdef USE_SCREEN_MIX_SURFACE
	GtkWidget *chkDoubleBuffer;
#endif
	GtkWidget *chkBorderColor;

#ifdef USE_DATAREC
	GtkWidget *chkReverseWave;
	GtkWidget *chkHalfWave;
	GtkWidget *radCorrectType[3];
	GtkWidget *txtCorrectAmp[2];

	GtkWidget *comSampleRate;
	GtkWidget *comSampleBits;
#endif

#ifdef USE_FD1
	GtkWidget *chkFDMount[USE_FLOPPY_DISKS];

	GtkWidget *chkDelayFd1;
	GtkWidget *chkDelayFd2;
	GtkWidget *chkFdDensity;
	GtkWidget *chkFdMedia;
	GtkWidget *chkFdSavePlain;
#endif

#ifdef USE_HD1
	GtkWidget *chkHDMount[USE_HARD_DISKS];

	GtkWidget *chkDelayHd2;
	GtkWidget *radSCSIType[SCSI_TYPE_END];

	GtkWidget *spnSramNumHdds;
	GtkWidget *chkSramScsiEn;
	GtkWidget *txtSramScsiId;
	GtkWidget *txtSramSasiOnScsi;
#endif

	GtkWidget *chkEnableMIDI;
#ifdef USE_MIDI
	GtkWidget *comMIDIOut;
	GtkWidget *spnMIDIOutDelay;
	GtkWidget *comMIDIResetType;
	GtkWidget *chkMIDIResPowerOn;
	GtkWidget *chkMIDIResPowerOff;
	GtkWidget *chkMIDIResHardRes;
	GtkWidget *chkMIDIResEndApp;
	GtkWidget *chkMIDINoRTMsg;
#endif

#ifdef MAX_PRINTER
	GtkWidget *txtLPTHost[MAX_PRINTER];
	GtkWidget *txtLPTPort[MAX_PRINTER];
	GtkWidget *txtLPTDelay[MAX_PRINTER];
#endif
#ifdef MAX_COMM
	GtkWidget *txtCOMHost[MAX_COMM];
	GtkWidget *txtCOMPort[MAX_COMM];
	GtkWidget *comCOMBaud[MAX_COMM];
#endif
#ifdef USE_DEBUGGER
	GtkWidget *txtDbgrHost;
	GtkWidget *txtDbgrPort;
#endif
	GtkWidget *comCOMUartBaud;
	GtkWidget *comCOMUartDataBit;
	GtkWidget *comCOMUartParity;
	GtkWidget *comCOMUartStopBit;
	GtkWidget *comCOMUartFlowCtrl;

	GtkWidget *comLanguage;

	CPtrList<CTchar> lang_list;

	bool SetData();
	void ShowFolderBox(const char *title, GtkWidget *entry);
	void ShowFontFileBox(const char *title, GtkWidget *entry);
	void ChangeFDDType(int index);
	void ChangeIOPort(int index);
	void ChangeFmOpn();
	void ChangeExPsg();
#ifdef USE_MIDI
	void SendMIDIResetMessage();
#endif

	static void OnChangedFDD(GtkWidget *widget, gpointer user_data);
	static void OnChangedIO(GtkWidget *widget, gpointer user_data);
	static void OnChangedFmOpn(GtkWidget *widget, gpointer user_data);
	static void OnChangedExPsg(GtkWidget *widget, gpointer user_data);

	static void OnSelectSnapPath(GtkWidget *widget, gpointer user_data);
	static void OnSelectFontPath(GtkWidget *widget, gpointer user_data);
	static void OnSelectROMPath(GtkWidget *widget, gpointer user_data);
	static void OnSelectMessageFont(GtkWidget *widget, gpointer user_data);
	static void OnSelectInfoFont(GtkWidget *widget, gpointer user_data);
	
#ifdef USE_MIDI
	static void OnSelectSendMIDIResetMessage(GtkWidget *widget, gpointer user_data);
#endif

	static void OnResponse(GtkWidget *widget, gint response_id, gpointer user_data);

public:
	ConfigBox(GUI *new_gui);
	~ConfigBox();
	bool Show(GtkWidget *parent_window);
	void Hide();
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_CONFIGBOX_H */
