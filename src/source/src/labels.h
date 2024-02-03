/** @file labels.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2021.04.18 -

	@brief [ labels ]
*/

#ifndef LABELS_H
#define LABELS_H

#include "common.h"
#include "msgs.h"

namespace LABELS {

extern const CMsg::Id tabs[];

extern const CMsg::Id power_state[];

extern const CMsg::Id io_port[];
extern const uint8_t io_port_pos[];

extern const CMsg::Id sys_mode[];
extern const CMsg::Id fdd_type[];

extern const CMsg::Id correct[];
extern const _TCHAR *correct_amp[];

extern const _TCHAR *wav_sampling_rate[];
extern const _TCHAR *wav_sampling_bits[];

#ifdef USE_DIRECT3D
extern const CMsg::Id d3d_use[];
extern const CMsg::Id d3d_filter[];
#endif

#ifdef USE_OPENGL
extern const CMsg::Id opengl_use[];
extern const CMsg::Id opengl_filter[];
#endif

extern const CMsg::Id led_show[];
extern const CMsg::Id led_pos[];

extern const _TCHAR *capture_fmt[];

extern const CMsg::Id sound_samples[];
extern const CMsg::Id sound_late[];

extern const CMsg::Id comm_baud[];

extern const _TCHAR *comm_uart_baudrate[];
extern const _TCHAR *comm_uart_databit[];
extern const CMsg::Id comm_uart_parity[];
extern const _TCHAR *comm_uart_stopbit[];
extern const CMsg::Id comm_uart_flowctrl[];

extern const CMsg::Id main_ram_size[];

extern const char *datarec_exts;

extern const char *floppy_disk_exts;
extern const char *blank_floppy_disk_exts;

extern const char *hard_disk_exts;
extern const char *blank_hard_disk_exts;

extern const char *state_file_exts;
extern const char *key_rec_file_exts;

extern const char *autokey_file_exts;
extern const char *printing_file_exts;

extern const CMsg::Id volume[];

extern const CMsg::Id keybind_col[][2];
extern const CMsg::Id keybind_tab[];
extern const CMsg::Id keybind_btn[];
extern const CMsg::Id keybind_combi[];

extern const CMsg::Id joypad_axis[];
extern const CMsg::Id joypad_type[];

#if defined(_X68000)
extern const _TCHAR *boot_devices[];
extern const _TCHAR *rs232c_baudrate[];
extern const _TCHAR *rs232c_databit[];
extern const _TCHAR *rs232c_stopbit[];
extern const CMsg::Id rs232c_flowctrl[];
extern const CMsg::Id sram_purpose[];
extern const _TCHAR *key_repeat_delay[];
extern const _TCHAR *key_repeat_rate[];
#endif

}; /* namespace LABELS */

#endif /* LABELS_H */

