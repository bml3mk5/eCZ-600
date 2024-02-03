/** @file cocoa_configpanel.h

 SHARP X68000 Emulator 'eCZ-600'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2022.09.22 -

 @brief [ config panel ]
 */

#ifndef COCOA_CONFIGPANEL_H
#define COCOA_CONFIGPANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"
#import "../../vm/vm.h"
#import "../../config.h"
#import "../../cchar.h"
#import "../../cptrlist.h"

/**
	@brief Config dialog box
*/
@interface CocoaConfigPanel : CocoaBasePanel
{

	CocoaTextField *txtROMPath;

#if defined(_X68000)
	CocoaPopUpButton *popMainRam;
	CocoaCheckBox *chkOwSizeSRAM;

	CocoaCheckBox *chkSRAMClear;
	CocoaCheckBox *chkSRAMSave;
	CocoaCheckBox *chkSRAMChanged;

	CocoaCheckBox *chkAddrErr;

	CocoaTextField *txtSramRomStartAddr;
	CocoaTextField *txtSramRamStartAddr;
	CocoaPopUpButton *popSramBootDevice;
	CocoaPopUpButton *popSramRS232CBaud;
	CocoaPopUpButton *popSramRS232CDataBit;
	CocoaPopUpButton *popSramRS232CParity;
	CocoaPopUpButton *popSramRS232CStopBit;
	CocoaPopUpButton *popSramRS232CFlowCtrl;
	CocoaStepper *stpSramContrast;
	CocoaCheckBox *chkSramFdEject;
	CocoaPopUpButton *popSramPurpose;
	CocoaPopUpButton *popSramKRDelay;
	CocoaPopUpButton *popSramKRRate;
	CocoaCheckBox *chkSramAlarm;
	CocoaStepper *stpSramNumHdds;
	CocoaCheckBox *chkSramKLEDkana;
	CocoaCheckBox *chkSramKLEDromaji;
	CocoaCheckBox *chkSramKLEDcinput;
	CocoaCheckBox *chkSramKLEDcaps;
	CocoaCheckBox *chkSramKLEDins;
	CocoaCheckBox *chkSramKLEDhira;
	CocoaCheckBox *chkSramKLEDzen;
#endif
	CocoaCheckBox *chkPowerOff;
	CocoaPopUpButton *popPowerState;

	CocoaPopUpButton *popUseOpenGL;
	CocoaPopUpButton *popGLFilter;
	CocoaPopUpButton *popLEDShow;
	CocoaPopUpButton *popLEDPosition;
	CocoaTextField *txtRasterSkew;
	CocoaTextField *txtVertSkew;
	CocoaPopUpButton *popCaptureType;

	CocoaTextField *txtSnapPath;
	CocoaTextField *txtFontPath;
	CocoaTextField *txtMsgFontName;
	CocoaTextField *txtMsgFontSize;
	CocoaTextField *txtInfoFontName;
	CocoaTextField *txtInfoFontSize;

	CocoaCheckBox *chkBorderColor;

#ifdef USE_DATAREC
	CocoaCheckBox *chkReverseWave;
	CocoaCheckBox *chkHalfWave;
//	CocoaCheckBox *chkCorrectWave;
	CocoaRadioGroup *radCorrect;
	CocoaTextField *txtCorrectAmp[2];
	CocoaPopUpButton *popSampleRate;
	CocoaPopUpButton *popSampleBits;
#endif

#ifdef USE_FD1
	CocoaCheckBox *chkFddMount[MAX_DRIVE];

	CocoaCheckBox *chkDelayFd1;
	CocoaCheckBox *chkDelayFd2;
	CocoaCheckBox *chkFdDensity;
	CocoaCheckBox *chkFdMedia;
	CocoaCheckBox *chkFdSavePlain;
#endif

#ifdef USE_FD1
	CocoaCheckBox *chkHddMount[MAX_HARD_DISKS];

	CocoaCheckBox *chkDelayHd2;
#endif

#ifdef MAX_PRINTER
	CocoaTextField *txtLPTHost[MAX_PRINTER];
	CocoaTextField *txtLPTPort[MAX_PRINTER];
	CocoaTextField *txtLPTDelay[MAX_PRINTER];
#endif
#ifdef MAX_COMM
	CocoaTextField *txtCOMHost[MAX_COMM];
	CocoaTextField *txtCOMPort[MAX_COMM];
//	CocoaPopUpButton *popCOMDipswitch[MAX_COMM];
#endif
#ifdef USE_DEBUGGER
	CocoaTextField *txtDbgrHost;
	CocoaTextField *txtDbgrPort;
#endif
	CocoaPopUpButton *popCOMUartBaud;
	CocoaPopUpButton *popCOMUartDataBit;
	CocoaPopUpButton *popCOMUartParity;
	CocoaPopUpButton *popCOMUartStopBit;
	CocoaPopUpButton *popCOMUartFlowCtrl;

	CocoaPopUpButton *popLanguage;
	CPtrList<CTchar> lang_list;
}
- (id)init;
- (NSInteger)runModal;
- (void)close;
- (void)dialogCancel:(id)sender;
- (void)dialogOk:(id)sender;
- (void)showFolderPanel:(id)sender;
- (void)showFilePanel:(id)sender;
- (void)showFontPanel:(id)sender;
- (void)changeFont:(id)sender;

#if defined(_MBS1)
- (void)selectSysMode:(CocoaRadioButton *)sender;
- (void)selectFmOpn:(CocoaCheckBox *)sender;
- (void)selectExPsg:(CocoaCheckBox *)sender;
#endif
- (void)selectFddType:(CocoaRadioButton *)sender;
- (void)selectIO:(CocoaCheckBox *)sender;
- (void)selectCorrect:(CocoaCheckBox *)sender;
@end

#endif /* COCOA_CONFIGPANEL_H */
