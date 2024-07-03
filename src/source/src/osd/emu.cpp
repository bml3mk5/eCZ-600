/** @file emu.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.12.01

	@brief [ emulation i/f ]
*/

#include "../common.h"
//#if defined(USE_WX) || defined(USE_WX2)
//#include <wx/wx.h>
//#endif
#include "../emu.h"
#include "../depend.h"
#include "../config.h"
#include "../fileio.h"
#include "../gui/gui.h"
#include "../utility.h"
#include "../clocale.h"
//#include "ledbox.h"
#ifdef USE_MESSAGE_BOARD
#include "../msgboard.h"
#endif
//#include "../csurface.h"

#define D88_BLANK_TITLE "BLANK"

#ifdef USE_PERFORMANCE_METER
int gdPMvalue = 0;
#endif

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

EMU::EMU(const _TCHAR *new_app_path, const _TCHAR *new_ini_path, const _TCHAR *new_res_path)
	: LogMessageReceiver()
{
	initialized = false;

	// get module path
	app_path.Set(new_app_path);
	ini_path.Set(new_ini_path);
	res_path.Set(new_res_path);

	vm = NULL;
	gui = NULL;
#ifdef USE_MESSAGE_BOARD
	msgboard = NULL;
#endif

#ifdef USE_EMU_INHERENT_SPEC
	memset(parami, 0, sizeof(parami));
	memset(paramv, 0, sizeof(paramv));
#endif

#ifdef USE_DEBUGGER
	EMU_DEBUGGER();
#endif
	EMU_INPUT();
	EMU_SCREEN();
	EMU_SOUND();
#ifdef USE_SOCKET
	EMU_SOCKET();
#endif
#ifdef USE_UART
	EMU_UART();
#endif
#ifdef USE_MIDI
	EMU_MIDI();
#endif

	// open logfile
//	open_logfile(ini_path);
	logging->out_logf(LOG_INFO, _T("START [%s]"), app_path.Get());

	//
	vm_pause = 0;

	// d88 bank switch
//	memset(d88_file, 0, sizeof(d88_file));
//	for(int drv=0; drv < USE_FLOPPY_DISKS; drv++) d88_file[drv].prev_bank = -2;
}

void EMU::initialize()
{
#ifdef USE_CPU_CLOCK_LOW
	cpu_clock_low = pConfig->cpu_clock_low;
#endif

	// initialize
	vm = new VM(this);
#ifdef USE_DEBUGGER
	initialize_debugger();
#endif
	initialize_input();
	initialize_screen();
	initialize_sound();
#ifdef USE_FD1
	initialize_disk_insert();
#endif
#ifdef USE_HD1
	initialize_hard_disk_insert();
#endif
#ifdef USE_MEDIA
	initialize_media();
#endif
#ifdef USE_SOCKET
	initialize_socket();
#endif
#ifdef USE_UART
	initialize_uart();
#endif
#ifdef USE_MIDI
	initialize_midi();
#endif
//	vm->reset();
	start_sound();

	if (gui) gui->InitializedEmu();

	initialized = true;
}

EMU::~EMU()
{
	logging->out_log(LOG_INFO, _T("EXIT"));
//	close_logfile();

#ifdef USE_MESSAGE_BOARD
	delete msgboard;
#endif
}

void EMU::release()
{
	end_sound();

	update_params();

#ifdef USE_DEBUGGER
	release_debugger();
#endif
	if(vm) {
		delete vm;
	}
	release_input();
	release_screen();
	release_sound();
#ifdef USE_MEDIA
	release_media();
#endif
#ifdef USE_SOCKET
	release_socket();
#endif
#ifdef USE_UART
	release_uart();
#endif
#ifdef USE_MIDI
	release_midi();
#endif
}

/// @note processed by emu thread
void EMU::release_on_emu_thread()
{
	release_sound_on_emu_thread();
	release_screen_on_emu_thread();
}

// ----------------------------------------------------------------------------
// drive machine
// ----------------------------------------------------------------------------

int EMU::frame_interval()
{
#ifdef SUPPORT_VARIABLE_TIMING
	return (int)(1024. * 1000. / vm->get_frame_rate() + 0.5);

#else
	return (int)(1024. * 1000. / FRAMES_PER_SEC + 0.5);
#endif
}

double EMU::get_frame_rate()
{
	return vm->get_frame_rate();
}

int EMU::run(int split_num)
{
	update_input();
	update_timer();
#ifdef USE_FD1
	update_disk_insert();
#endif
#ifdef USE_SOCKET
	update_socket();
#endif
#ifdef USE_UART
	update_uart();
#endif
#ifdef USE_MIDI
	update_midi();
#endif

	int extra_frames = 0;
//	update_sound(&extra_frames);

	// drive virtual machine
	if(extra_frames == 0) {
		vm->run(split_num);
		extra_frames = 1;
	}
#ifdef USE_MESSAGE_BOARD
	if (msgboard && split_num == 0) {
		msgboard->CountDown();
	}
#endif
	return extra_frames;
}

// update status if need
void EMU::update()
{
	update_timer();
#ifdef USE_FD1
	update_disk_insert();
#endif
#ifdef USE_SOCKET
	update_socket();
#endif
#ifdef USE_UART
	update_uart();
#endif

	return;
}

void EMU::reset()
{
	if (pConfig->use_power_off && pConfig->now_power_off) {
		// power off
		set_pause(3, true);
	} else {
		// power on
		set_pause(3, false);
	}

	// update config
	update_params();

#ifdef USE_CPU_CLOCK_LOW
	if(cpu_clock_low != pConfig->cpu_clock_low) {
		// stop sound
		if(sound_ok && sound_started) {
			lpdsb->Stop();
			sound_started = false;
		}
		// reinitialize virtual machine
		delete vm;
		vm = new VM(this);
		vm->initialize_sound(sound_rate, sound_samples);
		vm->reset();

		// restore inserted floppy disks
#ifdef USE_FD1
		for(int drv = 0; drv < USE_FLOPPY_DISKS; drv++) {
			if(disk_insert[drv].path[0] != _T('\0')) {
				vm->open_disk(drv, disk_insert[drv].path, disk_insert[drv].offset);
			}
		}
#endif
		cpu_clock_low = pConfig->cpu_clock_low;
	}
	else {
#endif
		// reinit sound
		vm->reset_sound(sound_rate, sound_samples);

		// reset virtual machine
		vm->reset();
#ifdef USE_CPU_CLOCK_LOW
	}
#endif

#ifdef USE_MEDIA
	stop_media();
#endif
#ifdef USE_MIDI
	reset_midi(!(pConfig->use_power_off && pConfig->now_power_off));
#endif
	// restart recording
//	restart_rec_video();
//	restart_rec_sound();
}

#ifdef USE_SPECIAL_RESET
void EMU::force_reset()
{
	vm->force_reset();
}

void EMU::special_reset()
{
	vm->special_reset();
}

bool EMU::now_special_reset()
{
	return vm->now_special_reset();
}

void EMU::warm_reset(int onoff)
{
	// reset virtual machine
	vm->warm_reset(onoff);
#ifdef USE_MEDIA
	stop_media();
#endif
}
#endif

void EMU::change_dipswitch(int num)
{
#ifdef USE_DIPSWITCH
	pConfig->dipswitch ^= (1 << num);
	vm->change_dipswitch(num);
#endif
}

#ifdef USE_POWER_OFF
void EMU::notify_power_off()
{
	vm->notify_power_off();
}
#endif

bool EMU::get_pause(int idx) const
{
	switch(idx) {
		case 1:
			return ((vm_pause & VM_SYSPAUSE_MASK) != 0);
			break;
		case 2:
			return ((vm_pause & VM_USRPAUSE_MASK) != 0);
			break;
		case 3:
			return ((vm_pause & (VM_SYSPAUSE_MASK | VM_USRPAUSE_MASK)) != 0);
			break;
	}
	return (vm_pause != 0);
}

void EMU::set_pause(int idx, bool val)
{
	switch(idx) {
		case 1:
			vm_pause = val ? (vm_pause | VM_SYSPAUSE_MASK) : (vm_pause & ~VM_SYSPAUSE_MASK);
			break;
		case 2:
			vm_pause = val ? (vm_pause | VM_USRPAUSE_MASK) : (vm_pause & ~VM_USRPAUSE_MASK);
			out_info_x(CMsg::Pause, val, -1);
			break;
		case 3:
			vm_pause = val ? (vm_pause | VM_POWEROFF_MASK) : (vm_pause & ~VM_POWEROFF_MASK);
			break;
	}

	// mute sound
	mute_sound(vm_pause != 0);

	// send to vm
	vm->pause(vm_pause);

#ifdef USE_MOUSE
	// control mouse cursor on main thread, so post message
	gui->PostMtEnableMouseTemp(!vm_pause);
#endif
#ifdef USE_MIDI
	pause_midi();
#endif
}

int *EMU::get_pause_ptr(void) {
	return &vm_pause;
}

void EMU::application_path(char *path, int len) const
{
	app_path.GetN(path, len);
}
const _TCHAR *EMU::application_path() const
{
	return app_path.Get();
}
void EMU::initialize_path(char *path, int len) const
{
	ini_path.GetN(path, len);
}
const _TCHAR *EMU::initialize_path() const
{
	return ini_path.Get();
}
void EMU::resource_path(char *path, int len) const
{
	res_path.GetN(path, len);
}
const _TCHAR *EMU::resource_path() const
{
	return res_path.Get();
}
#if defined(_UNICODE)
void EMU::application_path(wchar_t *path, int len) const
{
	app_path.GetW(path, len);
}
void EMU::initialize_path(wchar_t *path, int len) const
{
	ini_path.GetW(path, len);
}
void EMU::resource_path(wchar_t *path, int len) const
{
	res_path.GetW(path, len);
}
#endif

void EMU::set_gui(GUI *new_gui)
{
	gui = new_gui;
}
GUI *EMU::get_gui()
{
	return gui;
}

// ----------------------------------------------------------------------------

void EMU::send_log_message(int level, const _TCHAR *levelstr, const _TCHAR *msg)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard && level != LOG_DEBUG) {
		msgboard->SetMessage(UTILITY::concat(_T("["), levelstr, _T("]"), msg, NULL));
	}
#endif
}

// ----------------------------------------------------------------------------

void EMU::out_msg(const _TCHAR* msg, bool set, int sec)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		if (set) {
			msgboard->SetMessage(msg, sec);
		} else {
			msgboard->DeleteMessage(msg);
		}
	}
#endif
}
void EMU::out_msg(const _TCHAR* msg, bool set)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		if (set) {
			msgboard->SetMessage(msg);
		} else {
			msgboard->DeleteMessage(msg);
		}
	}
#endif
}

void EMU::out_info(const _TCHAR* msg, bool set, int sec)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		if (set) {
			msgboard->SetInfo(msg, sec);
		} else {
			msgboard->DeleteInfo(msg);
		}
	}
#endif
}

void EMU::out_info(const _TCHAR* msg, bool set)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		if (set) {
			msgboard->SetInfo(msg);
		} else {
			msgboard->DeleteInfo(msg);
		}
	}
#endif
}

void EMU::out_infof(const _TCHAR* format, ...)
{
#ifdef USE_MESSAGE_BOARD
	va_list ap;

	va_start(ap, format);
	out_infov(format, ap);
	va_end(ap);
#endif
}

void EMU::out_infov(const _TCHAR* format, va_list ap)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		_TCHAR buffer[1024];
#if !defined(USE_WIN) && defined(_WIN32) && !defined(_UNICODE) && !defined(__MINGW32__)
		UTILITY::vsprintf_utf8(buffer, 1024, format, ap);
#else
		UTILITY::vstprintf(buffer, 1024, format, ap);
#endif
		msgboard->SetInfo(buffer);
	}
#endif
}

void EMU::out_msg_x(const _TCHAR* msg, bool set, int sec)
{
#ifdef USE_MESSAGE_BOARD
	out_msg(_tgettext(msg), set, sec);
#endif
}
void EMU::out_msg_x(const _TCHAR* msg, bool set)
{
#ifdef USE_MESSAGE_BOARD
	out_msg(_tgettext(msg), set);
#endif
}

void EMU::out_info_x(const _TCHAR* msg, bool set, int sec)
{
#ifdef USE_MESSAGE_BOARD
	out_info(_tgettext(msg), set, sec);
#endif
}

void EMU::out_info_x(const _TCHAR* msg, bool set)
{
#ifdef USE_MESSAGE_BOARD
	out_info(_tgettext(msg), set);
#endif
}

void EMU::out_infof_x(const _TCHAR* format, ...)
{
#ifdef USE_MESSAGE_BOARD
	va_list ap;

	va_start(ap, format);
	out_infov(_tgettext(format), ap);
	va_end(ap);
#endif
}

void EMU::out_infoc_x(const _TCHAR* msg1, ...)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		_TCHAR buffer[1024];
		size_t max_len = 1024;
		const _TCHAR *src2;

		va_list ap;

		va_start(ap, msg1);
#if defined(USE_QT)
		UTILITY::tcscpy(buffer, max_len, QCoreApplication::translate(NULL, msg1).toUtf8().data());
		for(size_t i=1; (src2 = va_arg(ap, const _TCHAR *)) != NULL; i++) {
			if (src2[0] != _T('\0')) UTILITY::tcscat(buffer, max_len, QCoreApplication::translate(NULL, src2).toUtf8().data());
		}
#else
		UTILITY::tcscpy(buffer, max_len, _tgettext(msg1));
		for(size_t i=1; (src2 = va_arg(ap, const _TCHAR *)) != NULL; i++) {
			if (src2[0] != _T('\0')) UTILITY::tcscat(buffer, max_len, _tgettext(src2));
		}
#endif
		va_end(ap);
		msgboard->SetInfo(buffer);
	}
#endif
}

void EMU::out_infov(CMsg::Id msg_id, va_list ap)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		_TCHAR buffer[1024];
		const _TCHAR *format = gMessages.Get(msg_id);
#if !defined(USE_WIN) && defined(_WIN32) && !defined(_UNICODE) && !defined(__MINGW32__)
		UTILITY::vsprintf_utf8(buffer, 1024, format, ap);
#else
		UTILITY::vstprintf(buffer, 1024, format, ap);
#endif
		msgboard->SetInfo(buffer);
	}
#endif
}

void EMU::out_msg_x(CMsg::Id msg_id, bool set, int sec)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		if (set) {
			msgboard->SetMessage(msg_id, sec);
		} else {
			msgboard->DeleteMessage(msg_id);
		}
	}
#endif
}
void EMU::out_msg_x(CMsg::Id msg_id, bool set)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		if (set) {
			msgboard->SetMessage(msg_id);
		} else {
			msgboard->DeleteMessage(msg_id);
		}
	}
#endif
}

void EMU::out_info_x(CMsg::Id msg_id, bool set, int sec)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		if (set) {
			msgboard->SetInfo(msg_id, sec);
		} else {
			msgboard->DeleteInfo(msg_id);
		}
	}
#endif
}

void EMU::out_info_x(CMsg::Id msg_id, bool set)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		if (set) {
			msgboard->SetInfo(msg_id);
		} else {
			msgboard->DeleteInfo(msg_id);
		}
	}
#endif
}

void EMU::out_infof_x(CMsg::Id msg_id, ...)
{
#ifdef USE_MESSAGE_BOARD
	va_list ap;

	va_start(ap, msg_id);
	out_infov(msg_id, ap);
	va_end(ap);
#endif
}

void EMU::out_infoc_x(CMsg::Id msg_id, ...)
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		_TCHAR buffer[1024];
		size_t max_len = 1024;
		int id2;

		va_list ap;

		va_start(ap, msg_id);
		UTILITY::tcscpy(buffer, max_len, CMSGV(msg_id));
		for(size_t i=1; (id2 = va_arg(ap, int)) != 0; i++) {
			if (id2 > 0 && id2 < CMsg::End) UTILITY::tcscat(buffer, max_len, CMSGV((CMsg::Id)id2));
		}
		va_end(ap);
		msgboard->SetInfo(buffer);
	}
#endif
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void EMU::change_cpu_power(int num)
{
	pConfig->cpu_power = num;

	if (num == 0) {
		out_info_x(CMsg::CPU_x0_5);
	} else {
		out_infof_x(CMsg::CPU_xVDIGIT, (1 << (num - 1)));
	}
	update_config();

#ifdef USE_MIDI
	set_midi_cpu_power();
#endif
}
#ifdef USE_EMU_INHERENT_SPEC
void EMU::change_sync_irq()
{
	pConfig->sync_irq = !pConfig->sync_irq;

	if (pConfig->sync_irq) {
		out_info_x(CMsg::Synchronize_Device_Speed_With_CPU_Speed);
	} else {
		out_info_x(CMsg::Asynchronize_Device_Speed_With_CPU_Speed);
	}
	update_config();
}

void EMU::assert_interrupt(int num)
{
	vm->assert_interrupt(num);
}
#endif

#ifdef USE_FD1
/// @brief open a new floppy disk image
/// @param[in] drv : drive number
/// @param[in] file_path : file path
/// @param[in] offset : position of a file image
/// @param[in] flags : bit0: 1 = read only
/// @return 0:opened successfully / -1:cannot open a new image / -2:failed closing the current image before open a new image
int EMU::open_floppy_disk_main(int drv, const _TCHAR* file_path, int offset, uint32_t flags)
{
	if(floppy_disk_inserted(drv) || disk_insert[drv].wait_count != 0) {
		if(floppy_disk_inserted(drv)) {
			if (!vm->close_floppy_disk(drv, flags)) return -2;
#ifdef USE_EMU_INHERENT_SPEC
		}
		return vm->open_floppy_disk(drv, file_path, offset, flags) ? 0 : -1;
#else
			// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
			disk_insert[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#else
			disk_insert[drv].wait_count = (int)(FRAMES_PER_SEC / 2);
#endif
		}
		UTILITY::tcscpy(disk_insert[drv].path, sizeof(disk_insert[drv].path) / sizeof(disk_insert[drv].path[0]), file_path);
		disk_insert[drv].offset = offset;
		disk_insert[drv].flags = flags;

		return 0;
#endif
	}
	else {
		return vm->open_floppy_disk(drv, file_path, offset, flags) ? 0 : -1;
	}
}
bool EMU::open_floppy_disk(int drv, const _TCHAR* file_path, int bank_num, int offset, uint32_t flags)
{
	int rc = -1;
	if (vm && file_path && file_path[0] != 0) {
		rc = open_floppy_disk_main(drv, file_path, offset, flags);
		if (!rc) {
			pConfig->UpdateRecentFloppyDiskPath(drv, file_path, bank_num);
			pConfig->SetNewOpenedFloppyDiskPath(drv, file_path, bank_num);
		} else {
			if (rc == -1) {
				// open error message
				logging->out_logf_x(LOG_ERROR, CMsg::Floppy_image_on_drive_VDIGIT_couldn_t_be_opened, drv);
			}
		}
		pConfig->SetInitialFloppyDiskPathFrom(file_path);
	}
	return (rc == 0);
}

void EMU::update_floppy_disk_info(int drv, const _TCHAR* file_path, int bank_num)
{
	pConfig->UpdateRecentFloppyDiskPath(drv, file_path, bank_num);
	pConfig->SetOpenedFloppyDiskPath(drv, file_path, bank_num);
	pConfig->SetInitialFloppyDiskPathFrom(file_path);
}

/// @brief open floppy disk with parsing multi volume
/// @param[in] drv : drive number
/// @param[in] file_path : file path
/// @param[in] bank_num : set index when multi volume
/// @param[in] flags : bit0: 1 = read only
/// @param[in] multiopen : open 2 drives when multi volume
/// @return true = success
bool EMU::open_floppy_disk_by_bank_num(int drv, const _TCHAR* file_path, int bank_num, uint32_t flags, bool multiopen)
{
	bool rc = false;

	bank_num = d88_files.Parse(drv, file_path, bank_num);

	int bank_nums = d88_files.GetFile(drv).GetBanks().Count();

	rc = open_floppy_disk_with_sel_bank(drv, bank_num, flags);
	if (!rc || !multiopen) return rc;
#ifdef USE_FD2
	if(drv == 0 && bank_num + 1 < bank_nums) {
		rc = open_floppy_disk_by_bank_num(drv + 1, file_path, bank_num + 1, flags, multiopen);
		if (!rc) return rc;
	}
#endif
#ifdef USE_FD4
	if(drv == 2 && bank_num + 1 < bank_nums) {
		rc = open_floppy_disk_by_bank_num(drv + 1, file_path, bank_num + 1, flags, multiopen);
		if (!rc) return rc;
	}
#endif
#ifdef USE_FD6
	if(drv == 4 && bank_num + 1 < bank_nums) {
		rc = open_floppy_disk_by_bank_num(drv + 1, file_path, bank_num + 1, flags, multiopen);
		if (!rc) return rc;
	}
#endif
	return rc;
}
bool EMU::open_floppy_disk_with_sel_bank(int drv, int bank_num, uint32_t flags)
{
	bool rc = false;
	D88File *d88_file = &d88_files.GetFile(drv); 
	int bank_nums = d88_file->GetBanks().Count();
	if (bank_nums > 1) {
		// set flags as multi volume
		flags |= OPEN_DISK_FLAGS_MULTI_VOLUME;
	}
	if (bank_nums == (bank_num + 1)) {
		// set flags as last volume
		flags |= OPEN_DISK_FLAGS_LAST_VOLUME;
	}
	if(d88_file->GetCurrentBank() != bank_num) {
		if ((rc = open_floppy_disk(drv, d88_file->GetPath(), bank_num, d88_file->GetBank(bank_num)->GetOffset(), flags)) == true) {
			d88_file->SetBank(bank_num);
		}
	}
	return rc;
}
void EMU::close_floppy_disk(int drv, uint32_t flags)
{
	if (!vm) return;

	if (!vm->close_floppy_disk(drv, flags)) return;

	pConfig->ClearOpenedFloppyDiskPath(drv);
	d88_files.GetFile(drv).Clear();
}
void EMU::initialize_disk_insert()
{
	memset(disk_insert, 0, sizeof(disk_insert));
}
void EMU::update_disk_insert()
{
	if (!vm) return;

	for(int drv = 0; drv < USE_FLOPPY_DISKS; drv++) {
		if(disk_insert[drv].wait_count != 0 && --disk_insert[drv].wait_count == 0) {
			vm->open_floppy_disk(drv, disk_insert[drv].path, disk_insert[drv].offset, disk_insert[drv].flags);
		}
	}
}
int EMU::change_floppy_disk(int drv)
{
	return (vm ? vm->change_floppy_disk(drv) : 0);
}
int EMU::get_floppy_disk_side(int drv)
{
	return (vm ? vm->get_floppy_disk_side(drv) : 0);
}
void EMU::toggle_floppy_disk_write_protect(int drv)
{
	if (!vm) return;

	vm->toggle_floppy_disk_write_protect(drv);
}
bool EMU::floppy_disk_write_protected(int drv)
{
	return (vm ? vm->floppy_disk_write_protected(drv) : false);
}
bool EMU::floppy_disk_inserted(int drv)
{
	return (vm ? vm->floppy_disk_inserted(drv) : false);
}
bool EMU::changed_cur_bank(int drv)
{
	bool upd = d88_files.GetFile(drv).IsChangedBank();
	d88_files.GetFile(drv).ChangeBank();
	return upd;
}
/// @brief create blank floppy disk image
/// @param[in] file_path: path
/// @param[in] type: 0x00 = 2D, 0x10 = 2DD, 0x20 = 2HD
bool EMU::create_blank_floppy_disk(const _TCHAR* file_path, uint8_t type)
{
	struct {
		char title[17];
		uint8_t rsrv[9];
		uint8_t protect;
		uint8_t type;
		uint32_t size;
		uint32_t trkptr[164];
	} d88_hdr;

	memset(&d88_hdr, 0, sizeof(d88_hdr));
	memcpy(d88_hdr.title, D88_BLANK_TITLE, strlen(D88_BLANK_TITLE));
	d88_hdr.type = type;
	d88_hdr.size = sizeof(d88_hdr);

	bool valid = false;
	FILEIO *fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO::WRITE_BINARY)) {
		fio->Fwrite(&d88_hdr, sizeof(d88_hdr), 1);
		fio->Fclose();
		valid = true;
	}
	delete fio;

	return valid;
}
bool EMU::is_same_floppy_disk(int drv, const _TCHAR *file_path, int bank_num)
{
	bank_num = d88_files.Parse(drv, file_path, bank_num);
	int offset = d88_files.GetFile(drv).GetBank(bank_num)->GetOffset();
	return (vm ? vm->is_same_floppy_disk(drv, file_path, offset) : false);
}
#endif

#ifdef USE_HD1
/// @brief open a new hard disk image
/// @param[in] drv : drive number
/// @param[in] file_path : file path
/// @param[in] flags : bit0: 1 = read only
/// @return true / false
bool EMU::open_hard_disk(int drv, const _TCHAR* file_path, uint32_t flags)
{
	bool rc = false;
	if (vm && file_path && file_path[0] != 0) {
		rc = vm->open_hard_disk(drv, file_path, flags);
		if (rc) {
			pConfig->UpdateRecentHardDiskPath(drv, file_path, 0);
			pConfig->SetNewOpenedHardDiskPath(drv, file_path, 0);
		}
		pConfig->SetInitialHardDiskPathFrom(file_path);
	}
	return rc;
}

void EMU::update_hard_disk_info(int drv, const _TCHAR* file_path, int bank_num)
{
	pConfig->UpdateRecentHardDiskPath(drv, file_path, bank_num);
	pConfig->SetOpenedHardDiskPath(drv, file_path, bank_num);
	pConfig->SetInitialHardDiskPathFrom(file_path);
}

void EMU::close_hard_disk(int drv, uint32_t flags)
{
	if (!vm) return;

	if (!vm->close_hard_disk(drv, flags)) return;

	pConfig->ClearOpenedHardDiskPath(drv);
}

void EMU::initialize_hard_disk_insert()
{
}

bool EMU::hard_disk_mounted(int drv)
{
	return (vm ? vm->hard_disk_mounted(drv) : false);
}

void EMU::toggle_hard_disk_write_protect(int drv)
{
	if (!vm) return;

	vm->toggle_hard_disk_write_protect(drv);
}

bool EMU::hard_disk_write_protected(int drv)
{
	return (vm ? vm->hard_disk_write_protected(drv) : false);
}

int EMU::get_hard_disk_device_type(int drv)
{
	if (!vm) return 0;

	return vm->get_hard_disk_device_type(drv);
}

void EMU::change_hard_disk_device_type(int drv, int num)
{
	if (!vm) return;

	vm->change_hard_disk_device_type(drv, num);
}

int EMU::get_current_hard_disk_device_type(int drv)
{
	if (!vm) return 0;

	return vm->get_current_hard_disk_device_type(drv);
}

/// @brief create blank hard disk image
/// @param[in] file_path : file path
/// @param[in] type: 0 = 10M, 1 = 20M, 2 = 40M
bool EMU::create_blank_hard_disk(const _TCHAR* file_path, uint8_t type)
{
	// sectors = 33/17, surfaces = 4, cylinders = 309, sector_size = 256/512	// 10MB
	// sectors = 33/17, surfaces = 4, cylinders = 614, sector_size = 256/512	// 20MB
	// sectors = 33/17, surfaces = 8, cylinders = 614, sector_size = 256/512	// 40MB
	static const int csize[] = {309, 614, 1228, 0};
	if (type < 0) type = 0;
	else if (type > 2) type = 2;

	int file_size = 33 * 4 * csize[type] * 256;
	bool valid = false;
	FILEIO *fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO::WRITE_BINARY)) {
		fio->Fsets(0, file_size);
		fio->Fclose();
		valid = true;
	}
	delete fio;

	return valid;
}
bool EMU::is_same_hard_disk(int drv, const _TCHAR *file_path)
{
	return (vm ? vm->is_same_hard_disk(drv, file_path) : false);
}
#endif

#ifdef USE_DATAREC
bool EMU::play_datarec(const _TCHAR* file_path)
{
	bool rc = false;
	pConfig->ClearOpenedDataRecPath();
	if (vm && file_path && file_path[0] != 0) {
		if (vm->play_datarec(file_path)) {
			pConfig->UpdateRecentDataRecPath(file_path, 0);
			pConfig->SetOpenedDataRecPath(file_path, 0);
			rc = true;
		} else {
			logging->out_log_x(LOG_ERROR, CMsg::Tape_image_couldn_t_be_opened);
		}
		pConfig->SetInitialDataRecPathFrom(file_path);
	}

	return rc;
}
bool EMU::rec_datarec(const _TCHAR* file_path)
{
	bool rc = false;
	pConfig->ClearOpenedDataRecPath();
	if (vm && file_path && file_path[0] != 0) {
		if (vm->rec_datarec(file_path)) {
			pConfig->UpdateRecentDataRecPath(file_path, 0);
			pConfig->SetOpenedDataRecPath(file_path, 0);
			rc = true;
		} else {
			logging->out_log_x(LOG_ERROR, CMsg::Tape_image_couldn_t_be_saved);
		}
		pConfig->SetInitialDataRecPathFrom(file_path);
	}

	return rc;
}
void EMU::close_datarec()
{
	if (!vm) return;

	pConfig->ClearOpenedDataRecPath();

	vm->close_datarec();
}
void EMU::rewind_datarec()
{
	if (!vm) return;

	vm->rewind_datarec();
}
void EMU::fast_forward_datarec()
{
	if (!vm) return;

	vm->fast_forward_datarec();
}
void EMU::stop_datarec()
{
	if (!vm) return;

	vm->stop_datarec();
}
void EMU::realmode_datarec()
{
	if (!vm) return;

	vm->realmode_datarec();
}
bool EMU::datarec_opened(bool play_mode)
{
	return (vm ? vm->datarec_opened(play_mode) : false);
}
#endif

#ifdef USE_DATAREC_BUTTON
void EMU::push_play()
{
	if (!vm) return;

	vm->push_play();
}
void EMU::push_stop()
{
	if (!vm) return;

	vm->push_stop();
}
#endif

#ifdef USE_CART1
void EMU::open_cart(int drv, const _TCHAR* file_path)
{
	if (!vm) return;

	vm->open_cart(file_path);

	// restart recording
	restart_rec_video();
	restart_rec_sound();
}
void EMU::close_cart(int drv)
{
	if (!vm) return;

	vm->close_cart();

	// stop recording
	stop_rec_video();
	stop_rec_sound();
}
#endif

#ifdef USE_QD1
void EMU::open_quickdisk(int drv, const _TCHAR* file_path)
{
	if (!vm) return;

	vm->open_quickdisk(drv, file_path);
}
void EMU::close_quickdisk(int drv)
{
	if (!vm) return;

	vm->close_quickdisk(drv);
}
void EMU::toggle_quickdisk_write_protect(int drv)
{
	if (!vm) return;

	vm->toggle_quickdisk_write_protect(drv);
}
#endif

#ifdef USE_BINARY_FILE1
void EMU::load_binary(int drv, const _TCHAR* file_path)
{
	if (!vm) return;

	vm->load_binary(drv, file_path);
}
void EMU::save_binary(int drv, const _TCHAR* file_path)
{
	if (!vm) return;

	vm->save_binary(drv, file_path);
}
void EMU::close_binary(int drv)
{
	if (!vm) return;

	vm->close_binary(drv);
}
#endif

#ifdef USE_SCANLINE
void EMU::change_screen_scanline(int num)
{
	if (num >= 0) {
		pConfig->scan_line = num;
	} else {
		pConfig->scan_line = (pConfig->scan_line + 1) % 4;
	}

	CMsg::Id msg_id;
	switch(pConfig->scan_line) {
	case 3:
		msg_id = CMsg::Stripe_Strongly_Drawing;
		break;
	case 2:
		msg_id = CMsg::Stripe_Drawing;
		break;
	case 1:
		msg_id = CMsg::Scanline_Drawing;
		break;
	default:
		msg_id = CMsg::Full_Drawing;
		break;
	}
	out_info_x(msg_id);
	update_config();
}
#endif
#ifdef USE_AFTERIMAGE
void EMU::change_screen_afterimage(int num)
{
	if (num >= 0) {
		pConfig->afterimage = (pConfig->afterimage == num) ? 0 : num;
	} else {
		pConfig->afterimage = (pConfig->afterimage + 1) % 3;
	}

	if (pConfig->afterimage != 0) {
		out_infof_x(CMsg::AfterimageVDIGIT_ON, pConfig->afterimage);
	} else {
		out_info_x(CMsg::Afterimage_OFF);
	}
	update_config();
}
#endif
#ifdef USE_KEEPIMAGE
void EMU::change_screen_keepimage(int num)
{
	if (num >= 0) {
		pConfig->keepimage = (pConfig->keepimage == num) ? 0 : num;
	} else {
		pConfig->keepimage = (pConfig->keepimage + 1) % 3;
	}

	if (pConfig->keepimage != 0) {
		out_infof_x(CMsg::Keepimage_d_ON, pConfig->keepimage);
	} else {
		out_info_x(CMsg::Keepimage_OFF);
	}
	update_config();
}
#endif

void EMU::power_on()
{
	// power on
	pConfig->now_power_off = false;
	set_pause(3, false);
}

void EMU::power_off()
{
	// power off
	pConfig->now_power_off = true;
	set_pause(3, true);
}

void EMU::toggle_power_on_off()
{
	// power on / off ?
	if (pConfig->use_power_off && !pConfig->now_power_off) {
		power_off();
	} else {
		power_on();
	}
}

bool EMU::now_skip()
{
	return (vm ? vm->now_skip() : false);
}

void EMU::update_config()
{
	if (!vm) return;

	vm->update_config();
}

#ifdef USE_PRINTER
bool EMU::save_printer(int dev, const _TCHAR *file_path)
{
	bool rc = false;
	if (vm && file_path && file_path[0] != 0) {
		if (vm->save_printer(dev, file_path)) {
			rc = true;
		} else {
			logging->out_log_x(LOG_ERROR, CMsg::Print_image_couldn_t_be_saved);
		}
		pConfig->SetInitialPrinterPathFrom(file_path);
	}
	return rc;
}
void EMU::clear_printer(int dev)
{
	if (!vm) return;

	vm->clear_printer(dev);
}
int  EMU::get_printer_buffer_size(int dev)
{
	return (vm ? vm->get_printer_buffer_size(dev) : 0);
}
uint8_t* EMU::get_printer_buffer(int dev)
{
	return (vm ? vm->get_printer_buffer(dev) : NULL);
}
void EMU::enable_printer_direct(int dev)
{
	if (!vm) return;

	vm->enable_printer_direct(dev);
}
bool EMU::print_printer(int dev)
{
	return (vm ? vm->print_printer(dev) : false);
}
void EMU::toggle_printer_online(int dev)
{
	if (!vm) return;

	vm->toggle_printer_online(dev);
}
#endif

#ifdef USE_EMU_INHERENT_SPEC
void EMU::enable_comm_server(int dev)
{
	if (!vm) return;

	vm->enable_comm_server(dev);
}
void EMU::enable_comm_connect(int dev, int num)
{
	if (!vm) return;

	vm->enable_comm_connect(dev, num);
}
bool EMU::now_comm_connecting(int dev, int num)
{
	if (!vm) return false;

	return vm->now_comm_connecting(dev, num);
}
void EMU::send_comm_telnet_command(int dev, int num)
{
	if (!vm) return;

	vm->send_comm_telnet_command(dev, num);
}

uint64_t EMU::update_led()
{
#ifndef USE_PERFORMANCE_METER
	return (vm ? vm->update_led() : 0);
#else
	return (vm ? vm->update_led() : 0) | ((uint64_t)gdPMvalue << 48);
#endif
}

#if 0
void EMU::show_led()
{
	if (ledbox) {
		pConfig->misc_flags ^= MSK_SHOWLEDBOX;
		ledbox->Show(FLG_LEDBOX_ALL);
	}
}

void EMU::inside_led()
{
	if (ledbox) {
		pConfig->misc_flags ^= MSK_INSIDELEDBOX;
		ledbox->Show(FLG_LEDBOX_ALL);
	}
}

void EMU::move_led()
{
	if (ledbox) {
		ledbox->Move();
	}
}

void EMU::change_pos_led(int num)
{
	if (ledbox) {
		if (num == -1) {
			pConfig->led_pos = (pConfig->led_pos + 1) % 4;
		} else {
			pConfig->led_pos = num;
		}
		ledbox->SetPos(pConfig->led_pos);
	}
}

/// @param[in] id
/// @return -1:ledbox is disable 0:false 1:true
int EMU::is_shown_ledbox(int id)
{
	if (ledbox && ledbox->IsEnable()) {
		switch(id) {
			case 0:
				return FLG_SHOWLEDBOX ? 1 : 0;
				break;
			case 1:
				return FLG_INSIDELEDBOX ? 1 : 0;
				break;
		}
	}
	return -1;
}
#endif

void EMU::show_message_board()
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		pConfig->misc_flags ^= MSK_SHOWMSGBOARD;
		msgboard->SetVisible(FLG_SHOWMSGBOARD ? true : false);
	}
#endif
}

/// @return -1:ledbox is disable 0:false 1:true
int EMU::is_shown_message_board()
{
#ifdef USE_MESSAGE_BOARD
	return ((msgboard && msgboard->IsEnable()) ? (FLG_SHOWMSGBOARD ? 1 : 0) : -1);
#else
	return -1;
#endif
}

void EMU::change_use_joypad(int num)
{
#if defined(USE_JOYSTICK)
	if (num < 0) {
#ifdef USE_PIAJOYSTICK
		num = (FLG_USEPIAJOYSTICK ? 2 : 0);
		num = 2 - num;
//		num = (FLG_USEJOYSTICK ? 1 : (FLG_USEPIAJOYSTICK ? 2 : 0));
//		num = (num + 1) % 3;
#else
		num = (FLG_USEJOYSTICK ? 1 : 0);
		num = (num + 1) % 2;
#endif
	}

	switch(num) {
	case 1:
		pConfig->misc_flags = ((pConfig->misc_flags & ~MSK_USEJOYSTICK_ALL) | MSK_USEJOYSTICK);
		use_joystick = true;
		reset_joystick();
		break;
	case 2:
		pConfig->misc_flags = ((pConfig->misc_flags & ~MSK_USEJOYSTICK_ALL) | MSK_USEPIAJOYSTICK);
		use_joystick = true;
		reset_joystick();
		break;
	default:
		pConfig->misc_flags = (pConfig->misc_flags & ~MSK_USEJOYSTICK_ALL);
		release_joystick();
		use_joystick = false;
		break;
	}
#else
	num;
#endif
}

bool EMU::is_enable_joypad(int num)
{
	switch(num) {
	case 1:
		return (FLG_USEJOYSTICK != 0);
	case 2:
		return (FLG_USEPIAJOYSTICK != 0);
	default:
		return (FLG_USEJOYSTICK_ALL != 0);
	}
}

void EMU::toggle_enable_key2joy()
{
#if defined(USE_KEY2JOYSTICK)
	if (FLG_USEKEY2JOYSTICK) {
		pConfig->misc_flags &= ~MSK_USEKEY2JOYSTICK;
		release_key2joy();
		key2joy_enabled = false;
	} else {
		pConfig->misc_flags |= MSK_USEKEY2JOYSTICK;
		key2joy_enabled = true;
		reset_key2joy();
	}
#endif
}

bool EMU::is_enable_key2joy()
{
#if defined(USE_KEY2JOYSTICK)
	return key2joy_enabled;
#else
	return false;
#endif
}

void EMU::modify_joytype()
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	vm->modify_joytype();
#endif
}

/// @brief save keybind data
void EMU::save_keybind()
{
	if (!vm) return;

	vm->save_keybind();
}

/// @brief change archtecture in vm
/// @param[in] id
/// @param[in] num
/// @param[in] reset
void EMU::change_archtecture(int id, int num, bool reset)
{
	if (vm) vm->change_archtecture(id, num, reset);
}

/// @brief save a state file
/// @param[in] file_path
/// @return true / false:file cannot open
bool EMU::save_state(const _TCHAR* file_path)
{
	bool rc = false;
#ifdef USE_STATE
	pConfig->ClearSavedStatePath();
	if (vm && file_path && file_path[0] != 0) {
		if (vm->save_state(file_path)) {
			pConfig->UpdateRecentStatePath(file_path, 0);
			pConfig->SetSavedStatePath(file_path, 0);
			rc = true;
		} else {
			logging->out_log_x(LOG_ERROR, CMsg::Status_image_couldn_t_be_saved);
		}
		pConfig->SetInitialStatePathFrom(file_path);
	}
#endif
	return rc;
}

/// @brief load a state file
/// @param[in] file_path
/// @return true / false:file cannot open
bool EMU::load_state(const _TCHAR* file_path)
{
	bool rc = false;
#ifdef USE_STATE
	if (vm && file_path && file_path[0] != 0) {
		if (vm->load_state(file_path)) {
			pConfig->UpdateRecentStatePath(file_path, 0);
			rc = true;
		} else {
			logging->out_log_x(LOG_ERROR, CMsg::Status_image_couldn_t_be_loaded);
		}
		pConfig->SetInitialStatePathFrom(file_path);
	}
#endif
	return rc;
}

#ifdef USE_KEY_RECORD
/// @brief play recording keys
/// @param[in] file_path
/// @return true / false:file cannot open
bool EMU::play_reckey(const _TCHAR* file_path)
{
	bool rc = false;
	if (vm && file_path && file_path[0] != 0) {
		if (vm->play_reckey(file_path)) {
			rc = true;
		} else {
			logging->out_log_x(LOG_ERROR, CMsg::Auto_key_file_couldn_t_be_opened);
		}
		pConfig->SetInitialStatePathFrom(file_path);
	}
	return rc;
}

/// @brief record key press/release action
/// @param[in] file_path
/// @return true / false:file cannot open
bool EMU::record_reckey(const _TCHAR* file_path)
{
	bool rc = false;
	if (vm && file_path && file_path[0] != 0) {
		if (vm->record_reckey(file_path)) {
			rc = true;
		} else {
			logging->out_log_x(LOG_ERROR, CMsg::Record_key_file_couldn_t_be_saved);
		}
		pConfig->SetInitialStatePathFrom(file_path);
	}
	return rc;
}

void EMU::stop_reckey(bool stop_play, bool stop_record)
{
	vm->stop_reckey(stop_play, stop_record);
}
#endif

void EMU::update_ui(int flags)
{
}

void EMU::get_edition_string(char *buffer, size_t buffer_size) const
{
	vm->get_edition_string(buffer, buffer_size);
}

/// @brief get VM specific parameter
///
/// @param[in] id
/// @return parameter
int EMU::get_parami(int id) const
{
	if (id < VM::ParamiUnknown) {
		return parami[id];
	}
	return 0;
}

/// @brief set VM specific parameter
///
/// @param[in] id
/// @param[in] val : parameter
void EMU::set_parami(int id, int val)
{
	if (id < VM::ParamiUnknown) {
		parami[id] = val;
	}
}

/// @brief get VM specific object
///
/// @param[in] id
/// @return object
void *EMU::get_paramv(int id) const
{
	if (id < VM::ParamvUnknown) {
		return paramv[id];
	}
	return NULL;
}

/// @brief set VM specific object
///
/// @param[in] id
/// @param[in] val : object
void EMU::set_paramv(int id, void *val)
{
	if (id < VM::ParamvUnknown) {
		paramv[id] = val;
	}
}

void EMU::update_params()
{
	if (vm) vm->update_params();
}

#endif	/* USE_EMU_INHERENT_SPEC */

/// @brief load a data from file
///
/// @param[in]  file_path : directory
/// @param[in]  file_name : data file
/// @param[out] data      : loaded data
/// @param[in]  size      : buffer size of data
/// @param[in]  first_data      : (nullable) first pattern to compare to loaded data
/// @param[in]  first_data_size : size of first_data
/// @param[in]  first_data_pos  : comparing position in loaded data
/// @param[in]  last_data       : (nullable) last pattern to compare to loaded data
/// @param[in]  last_data_size  : size of last_data
/// @param[in]  last_data_pos   : comparing position in loaded data
/// @return 1:successfully loaded  2:data loaded but unmatch pattern or size  0:unloaded
int EMU::load_data_from_file(const _TCHAR *file_path, const _TCHAR *file_name
	, uint8_t *data, size_t size
	, const uint8_t *first_data, size_t first_data_size, size_t first_data_pos
	, const uint8_t *last_data,  size_t last_data_size, size_t last_data_pos)
{
	_TCHAR path[_MAX_PATH];
	FILEIO fio;
	int loaded = 0;

	UTILITY::concat(path, _MAX_PATH, file_path, file_name, NULL);
	if(fio.Fopen(path, FILEIO::READ_BINARY)) {
		size_t len = (size_t)fio.FileLength();
		fio.Fread(data, size, 1);
		fio.Fclose();
		loaded = 1;
		logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_loaded, file_name);
		if (size > len) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_is_VDIGIT_bytes_smaller_than_assumed_one, file_name, (int)size - len);
			loaded = 2;
		}
		if (first_data != NULL && first_data_size > 0
		 && memcmp(&data[first_data_pos], first_data, first_data_size) != 0) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_is_different_image_from_assumed_one, file_name);
			loaded = 2;
		}
		if (last_data != NULL && last_data_size > 0
		 && memcmp(&data[last_data_pos], last_data, last_data_size) != 0) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_is_different_image_from_assumed_one, file_name);
			loaded = 2;
		}
	}
	return loaded;
}

/// @brief load a data from file (filename is case-insensifile)
///
/// @param[in]  file_path : directory
/// @param[in]  file_name : data file
/// @param[out] data      : loaded data
/// @param[in]  size      : buffer size of data
/// @param[in]  first_data      : (nullable) first pattern to compare to loaded data
/// @param[in]  first_data_size : size of first_data
/// @param[in]  first_data_pos  : comparing position in loaded data
/// @param[in]  last_data       : (nullable) last pattern to compare to loaded data
/// @param[in]  last_data_size  : size of last_data
/// @param[in]  last_data_pos   : comparing position in loaded data
/// @return 1:successfully loaded  2:data loaded but unmatch pattern or size  0:unloaded
int EMU::load_data_from_file_i(const _TCHAR *file_path, const _TCHAR *file_name
	, uint8_t *data, size_t size
	, const uint8_t *first_data, size_t first_data_size, size_t first_data_pos
	, const uint8_t *last_data,  size_t last_data_size, size_t last_data_pos)
{
#ifdef _WIN32
	return load_data_from_file(file_path, file_name, data, size
			,first_data, first_data_size, first_data_pos
			,last_data, last_data_size, last_data_pos);
#else
	CTchar n_file_name;
	int rc = 0;
	for(int i=0; i<3 && !rc; i++) {
		switch(i) {
		case 2:
			// lower case
			n_file_name.ToLower();
			break;
		case 1:
			// upper case
			n_file_name.ToUpper();
			break;
		default:
			n_file_name.Set(file_name);
			break;
		}
		rc = load_data_from_file(file_path, n_file_name.Get(), data, size
			,first_data, first_data_size, first_data_pos
			,last_data, last_data_size, last_data_pos);
	}
	return rc;
#endif
}

/// @return current clock on vm
uint64_t EMU::get_current_clock()
{
	if (vm) return vm->get_current_clock();
	else return 0;
}

int EMU::get_current_power()
{
	if (vm) return vm->get_current_power();
	else return 1;
}

void EMU::sleep(uint32_t ms)
{
}

