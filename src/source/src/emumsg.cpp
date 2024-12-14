/** @file emumsg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.04.01 -

	@brief [ emu message ]
*/

#if defined(USE_WX) || defined(USE_WX2)
#include <wx/wx.h>
#endif
#include "emumsg.h"
#include "emu.h"
#include "gui/gui.h"
#include "main.h"
#include "utility.h"
#include "config.h"

extern EMU *emu;
extern GUI *gui;

EmuMsgItem::EmuMsgItem()
{
	Initialize(EMUMSG_MSG_NONE, EMUMSG_ID_NONE);
}
EmuMsgItem::EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id)
{
	Initialize(new_msg, new_id);
}
EmuMsgItem::EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id, const _TCHAR *new_file)
{
	Set(new_msg, new_id, new_file);
}
EmuMsgItem::EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_num)
{
	Set(new_msg, new_id, new_num);
}
EmuMsgItem::EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id, const _TCHAR *new_file, bool new_sys_pause)
{
	Set(new_msg, new_id, new_file, new_sys_pause);
}
EmuMsgItem::EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_drv, const _TCHAR *new_file, int new_bank, uint32_t new_flags, bool new_multi)
{
	Set(new_msg, new_id, new_drv, new_file, new_bank, new_flags, new_multi);
}
EmuMsgItem::EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_drv, int new_num)
{
	Set(new_msg, new_id, new_drv, new_num);
}
EmuMsgItem::EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_drv, const _TCHAR *new_file)
{
	Set(new_msg, new_id, new_drv, new_file);
}
EmuMsgItem::~EmuMsgItem()
{
	Terminate();
}
void EmuMsgItem::Initialize(en_emumsg_msg new_msg, en_emumsg_id new_id)
{
	msg = new_msg;
	id = new_id;

	drv = 0;
	num = 0;
	flags = 0;
	multi = false;
	sys_pause = false;
	file1 = NULL;
}
void EmuMsgItem::Terminate()
{
	delete [] file1;
}
void EmuMsgItem::SetFile(const _TCHAR *new_file)
{
	size_t len = _tcslen(new_file);
	file1 = new _TCHAR[len + 1];
	memcpy(file1, new_file, len * sizeof(_TCHAR));
	file1[len] = _T('\0');
}
void EmuMsgItem::Set(en_emumsg_msg new_msg, en_emumsg_id new_id, const _TCHAR *new_file)
{
	Initialize(new_msg, new_id);
	SetFile(new_file);
}
void EmuMsgItem::Set(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_num)
{
	Initialize(new_msg, new_id);
	num = new_num;
}
void EmuMsgItem::Set(en_emumsg_msg new_msg, en_emumsg_id new_id, const _TCHAR *new_file, bool new_sys_pause)
{
	Initialize(new_msg, new_id);
	sys_pause = new_sys_pause;
	SetFile(new_file);
}
void EmuMsgItem::Set(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_drv, const _TCHAR *new_file, int new_bank, uint32_t new_flags, bool new_multi)
{
	Initialize(new_msg, new_id);
	drv = new_drv;
	num = new_bank;
	flags = new_flags;
	multi = new_multi;
	SetFile(new_file);
}
void EmuMsgItem::Set(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_drv, int new_num)
{
	Initialize(new_msg, new_id);
	drv = new_drv;
	num = new_num;
}
void EmuMsgItem::Set(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_drv, const _TCHAR *new_file)
{
	Initialize(new_msg, new_id);
	drv = new_drv;
	SetFile(new_file);
}




EmuMsg::EmuMsg()
{
	emumsg_push = 0;
	emumsg_pop  = 0;
	emumsg_count = 0;
	for(int i=0; i<EMUMSG_MSG_BUFFER_SIZE; i++) {
		stack[i] = NULL;
	}
}

EmuMsg::~EmuMsg()
{
	for(int i=0; i<EMUMSG_MSG_BUFFER_SIZE; i++) {
		delete stack[i];
	}
}

EmuMsgItem *EmuMsg::Get()
{
	if (emumsg_count <= 0) return NULL;
	return stack[emumsg_pop];
}
void EmuMsg::Pop()
{
	delete stack[emumsg_pop];
	stack[emumsg_pop] = NULL;
	emumsg_count--;
	emumsg_pop = (emumsg_pop + 1) % EMUMSG_MSG_BUFFER_SIZE;
}
void EmuMsg::Push(EmuMsgItem *new_item)
{
	delete stack[emumsg_push];
	stack[emumsg_push] = new_item;
	emumsg_count++;
	emumsg_push = (emumsg_push + 1) % EMUMSG_MSG_BUFFER_SIZE;
}
void EmuMsg::Send(en_emumsg_id id)
{
	EmuMsgItem *item = new EmuMsgItem(EMUMSG_MSG_COMMAND, id);
	Push(item);
}
void EmuMsg::Send(en_emumsg_id id, int new_num)
{
	EmuMsgItem *item = new EmuMsgItem(EMUMSG_MSG_COMMAND, id, new_num);
	Push(item);
}
void EmuMsg::Send(en_emumsg_id id, const _TCHAR *new_file)
{
	EmuMsgItem *item = new EmuMsgItem(EMUMSG_MSG_COMMAND, id, new_file);
	Push(item);
}
void EmuMsg::Send(en_emumsg_id id, const _TCHAR *new_file, bool new_sys_pause)
{
	EmuMsgItem *item = new EmuMsgItem(EMUMSG_MSG_COMMAND, id, new_file, new_sys_pause);
	Push(item);
}
void EmuMsg::Send(en_emumsg_id id, int new_drv, const _TCHAR *new_file, int new_bank, uint32_t new_flags, bool new_multi)
{
	EmuMsgItem *item = new EmuMsgItem(EMUMSG_MSG_COMMAND, id, new_drv, new_file, new_bank, new_flags, new_multi);
	Push(item);
}
void EmuMsg::Send(en_emumsg_id id, int new_drv, int new_num)
{
	EmuMsgItem *item = new EmuMsgItem(EMUMSG_MSG_COMMAND, id, new_drv, new_num);
	Push(item);
}
void EmuMsg::Send(en_emumsg_id id, int new_drv, const _TCHAR *new_file)
{
	EmuMsgItem *item = new EmuMsgItem(EMUMSG_MSG_COMMAND, id, new_drv, new_file);
	Push(item);
}

int EmuMsg::Process()
{
	EmuMsgItem *item;

	if ((item = Get()) == NULL) return 0;
	switch(item->GetMsg()) {
	case EMUMSG_MSG_COMMAND:
		switch(item->GetId()) {
		case EMUMSG_ID_POWER_ON:
			if (emu) {
				emu->power_on();
				emu->reset();
			}
			break;
		case EMUMSG_ID_RESET:
			if (emu) {
				emu->toggle_power_on_off();
				emu->reset();
			}
			break;
#ifdef USE_SPECIAL_RESET
		case EMUMSG_ID_FORCE_RESET:
			if (emu) {
				emu->force_reset();
			}
			break;
		case EMUMSG_ID_SPECIAL_RESET:
			if (emu) {
				emu->special_reset();
			}
			break;
		case EMUMSG_ID_WARM_RESET:
			if (emu) {
				emu->warm_reset(item->GetNum());
			}
			break;
#endif
		case EMUMSG_ID_CPU_POWER:
			if (emu) {
				emu->change_cpu_power(item->GetNum());
			}
			break;
#ifdef USE_EMU_INHERENT_SPEC
		case EMUMSG_ID_INTERRUPT:
			if (emu) {
				emu->assert_interrupt(item->GetNum());
			}
			break;
		case EMUMSG_ID_SYSTEM_PAUSE:
			if (gui) {
				gui->SystemPause(item->GetNum() ? true : false);
			}
			break;
		case EMUMSG_ID_PAUSE:
			if (gui) {
				gui->TogglePause();
			}
			break;
		case EMUMSG_ID_SYNC_IRQ:
			if (emu) {
				emu->change_sync_irq();
			}
			break;
#endif
#ifdef USE_AUTO_KEY
		case EMUMSG_ID_AUTOKEY_START:
			if(gui) {
				gui->StartAutoKey();
			}
			break;
		case EMUMSG_ID_AUTOKEY_OPEN:
			if (emu) {
				emu->open_auto_key(item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_AUTOKEY_STOP:
			if (emu) {
				emu->stop_auto_key();
			}
			break;
#endif
#ifdef USE_KEY_RECORD
		case EMUMSG_ID_RECKEY_PLAY:
			if (emu) {
				emu->play_reckey(item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_RECKEY_REC:
			if (emu) {
				emu->record_reckey(item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_RECKEY_STOP_PLAY:
			if(emu) {
				emu->stop_reckey(true, false);
			}
			break;
		case EMUMSG_ID_RECKEY_STOP_REC:
			if(emu) {
				emu->stop_reckey(false, true);
			}
			break;
#endif
#ifdef USE_STATE
		case EMUMSG_ID_LOAD_STATE:
			if (emu) {
				emu->load_state(item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_SAVE_STATE:
			if (emu) {
				emu->save_state(item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_RECENT_STATE:
			if (emu) {
				emu->load_state(item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
#endif
#ifdef USE_FD1
		case EMUMSG_ID_OPEN_FD:
			if (emu) {
				emu->open_floppy_disk_by_bank_num(item->GetDrv(), item->GetFile(), item->GetBankNum(), item->GetFlags(), item->GetMulti());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_CLOSE_FD:
			gui->CloseFloppyDisk(item->GetDrv());
			break;
		case EMUMSG_ID_CHANGE_FD:
			if (emu) {
				emu->change_floppy_disk(item->GetDrv());
			}
			break;
		case EMUMSG_ID_WRITEPROTECT_FD:
			if (emu) {
				emu->toggle_floppy_disk_write_protect(item->GetDrv());
			}
			break;
		case EMUMSG_ID_RECENT_FD:
			if (emu) {
				emu->open_floppy_disk_by_bank_num(item->GetDrv(), item->GetFile(), item->GetBankNum(), item->GetFlags(), item->GetMulti());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_SELECT_D88_BANK:
			{
				emu->open_floppy_disk_with_sel_bank(item->GetDrv(), item->GetNum());
			}
			break;
#endif
#ifdef USE_HD1
		case EMUMSG_ID_OPEN_HD:
			if (emu) {
				emu->open_hard_disk(item->GetDrv(), item->GetFile(), item->GetFlags());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_CLOSE_HD:
			if (emu) {
				emu->close_hard_disk(item->GetDrv());
			}
			break;
		case EMUMSG_ID_WRITEPROTECT_HD:
			if (emu) {
				emu->toggle_hard_disk_write_protect(item->GetDrv());
			}
			break;
		case EMUMSG_ID_RECENT_HD:
			if (emu) {
				emu->open_hard_disk(item->GetDrv(), item->GetFile(), item->GetFlags());
				gui->SystemPause(item->GetSysPause());
			}
			break;
#endif
#ifdef USE_DATAREC
		case EMUMSG_ID_PLAY_DATAREC:
			if (emu) {
				emu->play_datarec(item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_REC_DATAREC:
			if (emu) {
				emu->rec_datarec(item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_CLOSE_DATAREC:
			if (emu) {
				emu->close_datarec();
			}
			break;
		case EMUMSG_ID_REWIND_DATAREC:
			if(emu) {
				emu->rewind_datarec();
			}
			break;
		case EMUMSG_ID_FAST_FORWARD_DATAREC:
			if(emu) {
				emu->fast_forward_datarec();
			}
			break;
		case EMUMSG_ID_STOP_DATAREC:
			if(emu) {
				emu->stop_datarec();
			}
			break;
		case EMUMSG_ID_REAL_DATAREC:
			if(emu) {
				emu->realmode_datarec();
			}
			break;
		case EMUMSG_ID_RECENT_DATAREC:
			if (emu) {
				emu->play_datarec(item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
#endif
#ifdef USE_CART1
		case EMUMSG_ID_OPEN_CART:
			if (emu) {
				emu->open_cart(item->GetDrv(), item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_CLOSE_CART:
			if (emu) {
				emu->close_cart(item->GetDrv());
			}
			break;
		case EMUMSG_ID_RECENT_CART:
			if (emu) {
				emu->open_cart(item->GetDrv(), item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
#endif
#ifdef USE_QD1
		case EMUMSG_ID_OPEN_QD:
			if (emu) {
				emu->open_quickdisk(item->GetDrv(), item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_CLOSE_QD:
			if (emu) {
				emu->close_quickdisk(item->GetDrv());
			}
			break;
		case EMUMSG_ID_WRITEPROTECT_QD:
			if (emu) {
				emu->toggle_quickdisk_write_protect(item->GetDrv());
			}
			break;
		case EMUMSG_ID_RECENT_QD:
			if (emu) {
				emu->open_quickdisk(item->GetDrv(), item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
#endif
#ifdef USE_MEDIA
		case EMUMSG_ID_OPEN_MEDIA:
			if (emu) {
				emu->open_media(item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_CLOSE_MEDIA:
			if (emu) {
				emu->close_media();
			}
			break;
		case EMUMSG_ID_RECENT_MEDIA:
			if (emu) {
				emu->open_media(item->GetDrv(), item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
#endif
#ifdef USE_BINARY_FILE1
		case EMUMSG_ID_LOAD_BINARY:
			if (emu) {
				emu->load_binary(item->GetDrv(), item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_SAVE_BINARY:
			if (emu) {
				emu->save_binary(item->GetDrv(), item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_CLOSE_BINARY:
			if (emu) {
				emu->close_binary(item->GetDrv());
			}
			break;
		case EMUMSG_ID_RECENT_BINARY:
			if (emu) {
				emu->load_binary(item->GetDrv(), item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
#endif
#ifdef USE_SCANLINE
		case EMUMSG_ID_SCREEN_SCANLINE1:
			if (emu) {
				emu->change_screen_scanline(item->GetNum());
			}
			break;
		case EMUMSG_ID_SCREEN_SCANLINE_A:
			if (emu) {
				emu->change_screen_scanline(-1);
			}
			break;
#endif
#ifdef USE_AFTERIMAGE
		case EMUMSG_ID_SCREEN_AFTERIMAGE1:
			if (emu) {
				emu->change_screen_afterimage(item->GetNum());
			}
			break;
		case EMUMSG_ID_SCREEN_AFTERIMAGE_A:
			if (emu) {
				emu->change_screen_afterimage(-1);
			}
			break;
#endif
#ifdef USE_KEEPIMAGE
		case EMUMSG_ID_SCREEN_KEEPIMAGE1:
			if (emu) {
				emu->change_screen_keepimage(item->GetNum());
			}
			break;
		case EMUMSG_ID_SCREEN_KEEPIMAGE_A:
			if (emu) {
				emu->change_screen_keepimage(-1);
			}
			break;
#endif
		case EMUMSG_ID_SCREEN_START_RECORD:
			if (emu) {
				gui->SystemPause(true);
				if (emu->start_rec_video(emu->get_parami(VM::ParamRecVideoType), item->GetNum(), true)) {
					emu->start_rec_sound(emu->get_parami(VM::ParamRecAudioType), true);
				}
				gui->SystemPause(false);
			}
			break;
		case EMUMSG_ID_SCREEN_STOP_RECORD:
			if (emu) {
				emu->stop_rec_video();
				emu->stop_rec_sound();
			}
			break;
		case EMUMSG_ID_SCREEN_RESIZE_RECORD:
			if (emu) {
				emu->resize_rec_video(item->GetNum());
			}
			break;
		case EMUMSG_ID_SCREEN_CAPTURE:
			if (emu) {
				emu->capture_screen();
			}
			break;
		case EMUMSG_ID_SOUND_START_RECORD:
			if (emu) {
				gui->SystemPause(true);
				emu->start_rec_sound(emu->get_parami(VM::ParamRecAudioType), false);
				gui->SystemPause(false);
			}
			break;
		case EMUMSG_ID_SOUND_STOP_RECORD:
			if (emu) {
				emu->stop_rec_sound();
			}
			break;
#ifdef USE_PRINTER
		case EMUMSG_ID_PRINTER_SAVE:
			if (emu) {
				emu->save_printer(item->GetDrv(), item->GetFile());
				gui->SystemPause(item->GetSysPause());
			}
			break;
		case EMUMSG_ID_PRINTER_PRINT:
			if (emu) {
				emu->print_printer(item->GetDrv());
			}
			break;
		case EMUMSG_ID_PRINTER_CLEAR:
			if (emu) {
				emu->clear_printer(item->GetDrv());
			}
			break;
		case EMUMSG_ID_PRINTER_DIRECT:
			if (emu) {
				emu->enable_printer_direct(item->GetDrv());
			}
			break;
		case EMUMSG_ID_PRINTER_ONLINE:
			if (emu) {
				emu->toggle_printer_online(item->GetDrv());
			}
			break;
#endif
#ifdef MAX_COMM
		case EMUMSG_ID_COMM_SERVER:
			if(emu) {
				emu->enable_comm_server(item->GetDrv());
			}
			break;
		case EMUMSG_ID_COMM_CONNECT:
			if(emu) {
				emu->enable_comm_connect(item->GetDrv(), item->GetNum());
			}
			break;
#endif
#ifdef USE_PIAJOYSTICK
		case EMUMSG_ID_MODIFY_JOYTYPE:
			if (emu) {
				emu->modify_joytype();
			}
			break;
#endif
		default:
			break;
		}
		break;
	default:
		break;
	}
	Pop();
	return 1;
}

EmuMsg emumsg;
