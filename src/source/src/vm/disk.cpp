/** @file disk.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.09.16-

	@brief [ d88 handler ]
*/

#include "disk.h"
#include "../emu.h"
#include "../fileio.h"
#include "../utility.h"
#include "../config.h"
#include "../depend.h"
#include "../labels.h"
#include "disk_parser.h"

#ifndef USE_CALC_CRC16
// crc table
static const uint16_t crc16_table[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};
#endif
#define CRC16_INIT_DATA 0xffff

#define CALC_TRACK_DATA_POS(track, side) (track * num_of_side + (side & 1))
#define GET_TRACK_DATA_TBL_PTR(buffer, pos) (buffer + 0x20 + pos * 4 + 0)

extern EMU *emu;

DISK::DISK(int drv)
{
	m_drive_num = drv;
	m_image_type = IMAGE_TYPE_D88;
	m_media_type = MEDIA_TYPE_UNK;
	m_drive_type = DRIVE_TYPE_UNK;
	inserted = ejected = write_protected = changed = false;
	m_file_size = m_file_size_orig = 0;
	m_file_offset = 0;
	track_size = TRACK_SIZE_5INCH_2D;
	sector_size = sector_nums = 0;
	density = NULL;
	sector_data = NULL;
	sector_id = NULL;
	num_of_side = 2;
	m_last_volume = false;
	m_filename_changed = false;
	m_tmp_track_size = 0;

	memset(id_c_in_track, 0, sizeof(id_c_in_track));
	memset(sector_pos, 0, sizeof(sector_pos));
}

DISK::~DISK()
{
	close();
}

uint32_t DISK::getcrc32(uint8_t data[], int size)
{
	uint32_t c, table[256];
	for(int i = 0; i < 256; i++) {
		uint32_t c = i;
		for(int j = 0; j < 8; j++) {
			if(c & 1) {
				c = (c >> 1) ^ 0xedb88320;
			}
			else {
				c >>= 1;
			}
		}
		table[i] = c;
	}
	c = ~0;
	for(int i = 0; i < size; i++) {
		c = table[(c ^ data[i]) & 0xff] ^ (c >> 8);
	}
	return ~c;
}

/// open floppy disk image
/// @param[in] path : file path
/// @param[in] offset : start position of file
/// @param[in] flags : bit0: 1 = read only  bit3: 1= last volume  bit4: 1 = multi volumed file
/// @return true = success
bool DISK::open(const _TCHAR *path, int offset, uint32_t flags)
{
	// check current disk image
	if(inserted) {
#ifdef _WIN32
		if(_tcsicmp(m_file_path, path) == 0 && m_file_offset == offset) {
#else
		if(_tcscmp(m_file_path, path) == 0 && m_file_offset == offset) {
#endif
			return true;
		}
		close();
	}

	// open disk image
	FILEIO *fio = new FILEIO();
	DISK_PARSER ps(this, fio, path, m_buffer, (int)sizeof(m_buffer)); 
	if(fio->Fopen(path, FILEIO::READ_BINARY)) {
		UTILITY::tcscpy(m_file_path, sizeof(m_file_path) / sizeof(m_file_path[0]), path);

		// check file size
		m_file_size = fio->FileLength();
		if (m_file_size == 0) {
			goto file_loaded;
		}

		// check if file protected
		write_protected = fio->IsFileProtected(path) | ((flags & OPEN_DISK_FLAGS_READ_ONLY) != 0);

		// is last volume ?
		m_last_volume = ((flags & OPEN_DISK_FLAGS_LAST_VOLUME) != 0);

		// check image file format
		if (ps.parse(offset, m_file_size_orig, m_file_offset)) {
			inserted = changed = true;
			goto file_loaded;
		}

file_loaded:
		if(inserted) {
			m_crc32 = getcrc32(m_buffer, m_file_size);
		}
		fio->Fclose();
//		ps.remove_tempfile();
		if(m_buffer[0x1a] != 0) {
			write_protected = true;
		}
		m_media_type = m_buffer[0x1b];

		check_d88_tracks();
	}
	delete fio;

	return inserted;
}

void DISK::close()
{
	// write disk image
	if(inserted) {
		if (!write_protected) {
			flash(write_protected);
		}
		ejected = true;
	}
	inserted = write_protected = false;
	m_file_size = m_file_size_orig = 0;
	m_file_offset = 0;
	sector_size = sector_nums = 0;
	sector_data = NULL;
	m_last_volume = false;
	m_filename_changed = false;
	m_tmp_track_size = 0;
	m_image_type = IMAGE_TYPE_D88;
	m_media_type = MEDIA_TYPE_UNK;
}

void DISK::flash()
{
	// write disk image
	if(inserted) {
		if (!write_protected) {
			flash(write_protected);
		}
	}
}

/// @brief flash data and write to image file 
/// @param[in] protect : whether write protect or not
bool DISK::flash(bool protect)
{
	// set write protect flag
	if (protect) {
		m_buffer[0x1a] |= 0x10;
	} else {
		m_buffer[0x1a] &= ~0x10;
	}

	// flash disk image
	bool write = true;
	if(m_file_size && getcrc32(m_buffer, m_file_size) != m_crc32) {
		// write image
//		if (!m_filename_changed && !m_last_volume && m_file_size != m_file_size_orig) {
		bool save_as_d88 = (m_image_type == IMAGE_TYPE_D88_CONVERTED || (m_image_type == IMAGE_TYPE_PLAIN && FLG_SAVE_FDPLAIN == 0));
		if (!m_filename_changed && save_as_d88) {
			// file name is converted d88 extension
			rename_file();
			m_image_type = IMAGE_TYPE_D88_CONVERTED;
		}
		save_as_d88 = save_as_d88 || (m_image_type != IMAGE_TYPE_PLAIN);
		write = write_file(m_filename_changed, !save_as_d88);
		if (write) {
			if (m_filename_changed) {
				// set file path to recent list on menu
				emu->update_floppy_disk_info(m_drive_num, m_file_path, 0);
			}
		} else {
			logging->out_logf_x(LOG_ERROR, CMsg::Floppy_image_on_drive_VDIGIT_couldn_t_be_saved, m_drive_num);
		}
	}
	return write;
}

/// @brief set write protect
/// flash disk image to file before set write protect.
void DISK::set_write_protect(bool val)
{
	if (val == false) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(m_file_path, FILEIO::READ_BINARY)) {
			val = fio->IsFileProtected(m_file_path);
			fio->Fclose();
		} else {
			val = true;
		}
		delete fio;
	}
	if (val == true && write_protected == false) {
		// when set write protect, flash disk image to file.
		flash(val);
	}
	write_protected = val;
}

/// @brief rename a file name to new name
/// new name is "PREFIX_YYYY-MM-DD_HH-MI-SS.d88"
void DISK::rename_file()
{
//	int tim[8];
	_TCHAR name[_MAX_PATH];

	UTILITY::get_dir_and_basename(m_file_path, name, 9);
	if (_tcslen(name) > 0) {
		UTILITY::tcscat(name, _MAX_PATH, _T("_"));
	}

	size_t len = _tcslen(name);
	UTILITY::create_date_file_path(NULL, &name[len], _MAX_PATH - len, LABELS::blank_floppy_disk_exts); 
//	emu->get_timer(tim, 8);
//	UTILITY::stprintf(&name[len], _MAX_PATH - len, _T("%04d-%02d-%02d_%02d-%02d-%02d.d88"), tim[0], tim[1], tim[2], tim[4], tim[5], tim[6]);

	UTILITY::tcscat(m_file_path, sizeof(m_file_path) / sizeof(m_file_path[0]), name);

	m_file_offset = 0;
	m_file_size_orig = m_file_size;
	m_filename_changed = true;

	logging->out_logf_x(LOG_INFO, CMsg::Floppy_image_on_drive_VDIGIT_is_saved_as_the_new_file_VSTR, m_drive_num, name);
	logging->out_logf_x(LOG_DEBUG, CMsg::Save_to_VSTR, m_file_path);
}

/// @brief write a disk image
/// @param[in] new_file : save as new file
/// @param[in] is_plain : save as plain image
bool DISK::write_file(bool new_file, bool is_plain)
{
	FILEIO fio;
	if(!fio.Fopen(m_file_path, new_file ? FILEIO::WRITE_BINARY : FILEIO::READ_WRITE_BINARY)) {
		return false;
	}

	if (is_plain) {
		// save as plain image
		d88_hdr_t *header = (d88_hdr_t *)m_buffer;

		for(int trkside = 0; trkside < 164; trkside++) {
			int offset = header->trkptr[trkside];
			if (offset == 0) {
				break;
			}
			uint8_t *sect = &m_buffer[offset];
			int sect_nums = ((d88_sct_t *)sect)->nsec;
			for(int s = 0; s < sect_nums; s++) {
				size_t sect_size = ((d88_sct_t *)sect)->size;
				sect += sizeof(d88_sct_t);
				fio.Fwrite(sect, sect_size, 1);
				sect += sect_size;
			}
		}

	} else {
		// save d88 image
		fio.Fseek(m_file_offset, FILEIO::SEEKSET);
		fio.Fwrite(m_buffer, m_file_size, 1);
		if (m_last_volume && m_file_size < m_file_size_orig) {
			// file size is shorter than before, so truncate file size
			trim_track_data_table();
			fio.Ftruncate(m_file_offset + m_file_size);
		}
	}

	fio.Fclose();
	return true;
}

/// get specified track offset
/// @return -1: track number is out of range.
int DISK::get_track_offset(int trk, int side)
{
	// search track
	int trkside = CALC_TRACK_DATA_POS(trk, side);
	if(!(0 <= trkside && trkside < 164)) {
		return -1;
	}
	int offset = (int)conv_to_uint32_le(GET_TRACK_DATA_TBL_PTR(m_buffer, trkside));
	return offset;
}

/// get specified track buffer
/// @param[in] trk  : track number
/// @param[in] side : side number
/// @return : buffer position of specified track in a disk
uint8_t* DISK::get_track_buffer(int trk, int side)
{
	// return start track if not insert a disk.
	if (!inserted) {
		// always ok
		sector_nums = 0;
		return &m_buffer[0x20];
	}

	// search track position
	int offset = get_track_offset(trk, side);

	if(offset <= 0) {
		// always ok
		sector_nums = 0;
		return &m_buffer[0x20];
	}

	// track found
	uint8_t *t = m_buffer + offset;
	sector_nums = conv_to_uint16_le(t + 4);

	return t;
}

/// get specified track
/// @param[in] trk  : track number
/// @param[in] side : side number
/// @return : false : not found or invalid 
bool DISK::get_track(int trk, int side)
{
	sector_size = sector_nums = 0;

	// disk not inserted or invalid media type
	// Seek command is always ok if diskette is not inserted.
//	if(!((FLG_ORIG_FDINSERT == 0 || inserted) && check_media_type())) {
//		return false;
//	}
	// ignore media check
	if(!(FLG_ORIG_FDINSERT == 0 || inserted)) {
		return false;
	}

	// search track
	uint8_t* t = get_track_buffer(trk, side);

	// track found

	if(sector_nums < 0 || sector_nums > 2048) {
		// invalid
		sector_nums = 0;
		return false;
	}

	for(int i = 0; i < sector_nums; i++) {
		id_c_in_track[i] = t[0];
		// sector position on a track
		sector_pos[t[2]] = (i & 0xff);
		t += (t[0xe] | (t[0xf] << 8)) + 0x10;
	}
	return true;
}

/// verify a track number in each sector on a track 
/// @param[in] trk  : track number
bool DISK::verify_track(int trk)
{
	for(int i = 0; i < sector_nums; i++) {
		if(id_c_in_track[i] != trk) {
			return false;
		}
	}
	return true;
}

/// make a track image
/// @param[in] trk  : track number
/// @param[in] side : side number
/// @param[in] dden : 0:single density(FM) default:double density(MFM)
bool DISK::make_track(int trk, int side, int dden)
{
	sector_size = sector_nums = 0;

	// create dummy track
	for(int i = 0; i < TRACK_BUFFER_SIZE; i++) {
		track[i] = rand();
	}

	// disk not inserted or invalid media type
	if(!(inserted && check_media_type())) {
		return false;
	}

	// search track
	uint8_t* t = get_track_buffer(trk, side);

	// get verify info

	if(sector_nums < 0 || sector_nums > 2048) {
		// invalid
		sector_nums = 0;
		return false;
	}

	// make track image
	int gap3len = 0;
	if (dden) {
//		gap3len = (sector_nums <= 5) ? 0x74 : (sector_nums <= 10) ? 0x54 : (sector_nums <= 16) ? 0x33 : 0x10;
		gap3len = (sector_nums <= 8) ? 116 : (sector_nums <= 15) ? 84 : (sector_nums <= 26) ? 54 : 16;
	} else {
		gap3len = (sector_nums <= 8) ? 58 : (sector_nums <= 15) ? 42 : (sector_nums <= 26) ? 27 : 8;
	}
	uint8_t gapmark = (dden ? 0x4e : 0xff);
	int p = 0;
	int len = 0;

	// gap0
	len = (dden ? 80 : 40);
	for(int i = 0; i < len; i++) {
		track[p++] = gapmark;
	}
	// sync
	len = (dden ? 12 : 6);
	for(int i = 0; i < len; i++) {
		track[p++] = 0;
	}
	// index mark
	if (dden) {
		track[p++] = 0xc2;
		track[p++] = 0xc2;
		track[p++] = 0xc2;
	}
	track[p++] = 0xfc;
	// gap1
	len = (dden ? 50 : 26);
	for(int i = 0; i < len; i++) {
		track[p++] = gapmark;
	}
	// sectors
	for(int i = 0; i < sector_nums; i ++) {
		// sync
		len = (dden ? 12 : 6);
		for(int j = 0; j < len; j++) {
			track[p++] = 0;
		}

		// id address mark
		uint16_t crc = CRC16_INIT_DATA;
		if (dden) {
			track[p++] = 0xa1;
			track[p++] = 0xa1;
			track[p++] = 0xa1;

			crc = set_crc16(0xa1, crc);
			crc = set_crc16(0xa1, crc);
			crc = set_crc16(0xa1, crc);
		}
		track[p++] = 0xfe;
		track[p++] = t[0];
		track[p++] = t[1];
		track[p++] = t[2];
		track[p++] = t[3];

		crc = set_crc16(0xfe, crc);
		crc = set_crc16(t[0], crc);
		crc = set_crc16(t[1], crc);
		crc = set_crc16(t[2], crc);
		crc = set_crc16(t[3], crc);

		track[p++] = crc >> 8;
		track[p++] = crc & 0xff;
		// gap2
		len = (dden ? 22 : 11);
		for(int j = 0; j < len; j++) {
			track[p++] = gapmark;
		}
		// sync
		len = (dden ? 12 : 6);
		for(int j = 0; j < len; j++) {
			track[p++] = 0;
		}
		// data mark, deleted mark
		crc = CRC16_INIT_DATA;
		if (dden) {
			track[p++] = 0xa1;
			track[p++] = 0xa1;
			track[p++] = 0xa1;

			crc = set_crc16(0xa1, crc);
			crc = set_crc16(0xa1, crc);
			crc = set_crc16(0xa1, crc);
		}
		track[p++] = (t[7]) ? 0xf8 : 0xfb;
		crc = set_crc16((t[7]) ? 0xf8 : 0xfb, crc);

		// data
		int size = t[0xe] | (t[0xf] << 8);
		for(int j = 0; j < size; j++) {
			track[p++] = t[0x10 + j];

			crc = set_crc16(t[0x10 + j], crc);
		}
		track[p++] = crc >> 8;
		track[p++] = crc & 0xff;
		t += size + 0x10;
		// gap3
		for(int j = 0; j < gap3len; j++) {
			track[p++] = gapmark;
		}
	}
	// gap4
	if (dden) {
		len = (sector_nums <= 8) ? 654 : (sector_nums <= 15) ? 400 : (sector_nums <= 26) ? 598 : 16;
	} else {
		len = (sector_nums <= 8) ? 311 : (sector_nums <= 15) ? 170 : (sector_nums <= 26) ? 247 : 8;
	}
	for(int i = 0; i < len; i++) {
		track[p++] = gapmark;
	}
	// padding
	while(p < 0x1800) {
		track[p++] = gapmark;
	}
	track_size = p;

	return true;
}

// 0:single density  1:double density
static int mark_length[2]={
	7,16
};

// index mark 0:single density  1:double density
static uint8_t index_mark[2][17]={
	{ 0,0,0,0,0,0,0xfc,0,0,0,0,0,0   ,0   ,0   ,0   ,0 },
	{ 0,0,0,0,0,0,0   ,0,0,0,0,0,0xc2,0xc2,0xc2,0xfc,0 }
};
// address mark
static uint8_t address_mark[2][17]={
	{ 0,0,0,0,0,0,0xfe,0,0,0,0,0,0   ,0   ,0   ,0   ,0 },
	{ 0,0,0,0,0,0,0   ,0,0,0,0,0,0xa1,0xa1,0xa1,0xfe,0 }
};
// data mark
static uint8_t data_mark[2][17]={
	{ 0,0,0,0,0,0,0xfb,0,0,0,0,0,0   ,0   ,0   ,0   ,0 },
	{ 0,0,0,0,0,0,0   ,0,0,0,0,0,0xa1,0xa1,0xa1,0xfb,0 }
};
// deleted data mark
static uint8_t deleted_mark[2][17]={
	{ 0,0,0,0,0,0,0xf8,0,0,0,0,0,0   ,0   ,0   ,0   ,0 },
	{ 0,0,0,0,0,0,0   ,0,0,0,0,0,0xa1,0xa1,0xa1,0xf8,0 }
};

/// parse raw track data and write to d88 track
/// @param[in] trk  : track number
/// @param[in] side : side number
/// @param[in] dden : 0:single density(FM) default:double density(MFM)
bool DISK::parse_track(int trk, int side, int dden)
{
	sector_nums = 0;

	// disk not inserted or invalid media type
	if(!(inserted && check_media_type())) {
		return false;
	}

	// search track
	get_track_buffer(trk, side);

	// get verify info

	if(sector_nums < 0 || sector_nums > 2048) {
		// invalid
		sector_nums = 0;
		return false;
	}

	// parse track image
	int p = 0;
	int phase = 0;
	int sec_size = 128;

	for(p=0; p < track_size && p < TRACK_BUFFER_SIZE; p++) {
		// search index mark
		if (phase == 0 && memcmp(&track[p], index_mark[dden], mark_length[dden]) == 0) {
			phase = 1;
		}
		// search id address mark
		if (phase == 1 && memcmp(&track[p], address_mark[dden], mark_length[dden]) == 0) {
			// search sector
			for(int i = 0; i < sector_nums; i++) {
				get_sector(trk, side, i);
				if (id[2] == track[p+mark_length[dden]+2]) {
					break;
				}
			}
			if (density != NULL) {
				switch(dden) {
					case 0:
						*density = 0x40;
						break;
					default:
						*density = 0;
						break;
				}
			}

			// calc sector size from ID N (N is 0 to 7)
			sec_size = (128 << (track[p+mark_length[dden]+3] & 7));

			if (sector_id != NULL) {
				int sz = sector_id[0xe] | (sector_id[0xf] << 8);
				if (sec_size <= sz) {
					sector_id[3] = track[p+mark_length[dden]+3];
				}
			}

			phase = 2;
		}
		// search data mark
		if (phase == 2 && memcmp(&track[p], data_mark[dden], mark_length[dden]) == 0) {

			if (sector_data != NULL) {
				for(int j=0; j < sector_size && j < sec_size; j++) {
					sector_data[j] = track[p+mark_length[dden]+j];
				}
			}
			p += sec_size;

			phase = 1;
		}
	}

	return true;
}

/// parse raw track data and replace to d88 track
/// @param[in] trk  : track number
/// @param[in] side : side number
/// @param[in] dden : 0:single density(FM) default:double density(MFM)
bool DISK::parse_track2(int trk, int side, int dden)
{
	// disk not inserted or invalid media type
	if(!(inserted && check_media_type())) {
		return false;
	}

//	// get offset
//	int offset = get_track_offset(trk, side);

	// parse track image
	int p = 0;
	int phase = 0;
	int sec_size = 128;
	int num_of_sector = 0;

	int tp = 0;	// position of tmp_track
	memset(m_tmp_track, 0, sizeof(m_tmp_track));

	for(p=0; p < track_size && p < TRACK_BUFFER_SIZE; p++) {
		// search index mark
		if (phase == 0 && memcmp(&track[p], index_mark[dden], mark_length[dden]) == 0) {
			phase = 1;
		}
		// search id address mark
		if (phase == 1 && memcmp(&track[p], address_mark[dden], mark_length[dden]) == 0) {
			// set sector
			p += mark_length[dden];

			memcpy(&m_tmp_track[tp], &track[p], 4); // C H R N
			num_of_sector = MAX(num_of_sector, track[p+2]); // sector number

			// calc sector size from ID N (N is 0 to 7)
			sec_size = (128 << (track[p+3] & 7));
			conv_from_uint16_le(&m_tmp_track[tp+14], (uint16_t)sec_size); 

			switch(dden) {
				case 0:
					m_tmp_track[tp+6] = 0x40;
					break;
				default:
					m_tmp_track[tp+6] = 0;
					break;
			}

			phase = 2;
		}
		// search data mark
		if (phase == 2
		&& (memcmp(&track[p], data_mark[dden], mark_length[dden]) == 0
		||  memcmp(&track[p], deleted_mark[dden], mark_length[dden]) == 0)) {
			p += mark_length[dden];

			m_tmp_track[tp+7] = (track[p-1] == 0xf8 ? 0x10 : 0);

			// copy sector data
			tp += 16;
			memcpy(&m_tmp_track[tp], &track[p], sec_size);

			p += sec_size;
			tp += sec_size;

			phase = 1;
		}
	}

	m_tmp_track_size = tp;

	// update number of sector
	bool valid = true;
	for(tp=0; tp < m_tmp_track_size && tp < TRACK_BUFFER_SIZE;) {
		conv_from_uint16_le(&m_tmp_track[tp+4], (uint16_t)num_of_sector);
		uint16_t next = conv_to_uint16_le(&m_tmp_track[tp+14]);
		if (next < 128 || next > 8192) {
			// invalid sector size
			valid = false;
			break;
		}
		tp += next + 16;
	}

	if (valid) {
		// replace buffer
		replace_track(trk, side);
	}

	return valid;
}

void DISK::replace_track(int trk, int side)
{
	if (m_tmp_track_size == 0) return;

	int trkside = CALC_TRACK_DATA_POS(trk, side);
	uint32_t *p = (uint32_t *)GET_TRACK_DATA_TBL_PTR(m_buffer, trkside);
	int old_track_size = 0;
	int new_track_size = 0;
	int offset = 0;
	uint8_t *buf;

	if (*p == 0) {
		// track not exists

		// upper track exist?
		int uptrkside = find_track_side(trkside, &offset);
		if (uptrkside < 0) {
			// add track
			offset = m_file_size;
			buf = m_buffer + offset;

		} else {
			// upper track exist
			buf = m_buffer + offset;

			// expand space
			expand_track_space(offset, m_tmp_track_size);
		}

		new_track_size = MIN(m_tmp_track_size, DISK_BUFFER_SIZE);
		memcpy(buf, m_tmp_track, new_track_size);

		if (uptrkside < 0) {
			// add file size
			add_file_size(new_track_size);
		}
	} else {
		// track exists
		offset = Uint32_LE(*p);
		buf = m_buffer + offset;
		old_track_size = calc_track_size(buf);

		if (old_track_size > m_tmp_track_size) {
			// shrink space
			shrink_track_space(offset + old_track_size, old_track_size - m_tmp_track_size); 
		} else if (old_track_size < m_tmp_track_size) {
			// expand space
			expand_track_space(offset + old_track_size, m_tmp_track_size - old_track_size);
		}

		// replace track
		new_track_size = MIN(m_tmp_track_size, DISK_BUFFER_SIZE);
		memcpy(buf, m_tmp_track, new_track_size);
	}

	*p = Uint32_LE(offset);
}

/// shrink one track space
void DISK::shrink_track_space(int start, int size)
{
	int offset = (start - size < 0 ? start : size);

	uint8_t *sp = &m_buffer[start];
	uint8_t *dp = sp - offset;
	uint8_t *ep = &m_buffer[m_file_size];

	for(;sp != ep; sp++, dp++) {
		*dp = *sp;
	}

	add_file_size(-offset);

	recalc_track_data_table(start, -offset);
}

/// expand one track space
void DISK::expand_track_space(int start, int size)
{
	int offset = (m_file_size + size > DISK_BUFFER_SIZE ? DISK_BUFFER_SIZE - m_file_size : size);

	uint8_t *sp = &m_buffer[m_file_size-1];
	uint8_t *dp = sp + offset;
	uint8_t *ep = &m_buffer[start-1];

	for(;sp != ep; sp--, dp--) {
		*dp = *sp;
	}

	add_file_size(offset);

	recalc_track_data_table(start, offset);
}

/// find track position
int DISK::find_track_side(int trkside, int *offset) 
{
	uint8_t *sp;
	int findtrkside = -1;

	for(int pos = trkside + 1; pos < 164; pos++) {
		sp = GET_TRACK_DATA_TBL_PTR(m_buffer, pos);
		int trkpos = (int)conv_to_uint32_le(sp);
		if (trkpos != 0) {
			findtrkside = pos;
			if (offset) {
				*offset = trkpos;
			}
			break;
		}
	}

	return findtrkside;
}

/// update track data table
void DISK::recalc_track_data_table(int start, int offset)
{
	uint8_t *sp;

	// recalc track data table
	for(int pos = 0; pos < 164; pos++) {
		sp = GET_TRACK_DATA_TBL_PTR(m_buffer, pos);
		int trkpos = (int)conv_to_uint32_le(sp);
		if (start <= trkpos) {
			trkpos += offset;
			conv_from_uint32_le(sp, (uint32_t)trkpos);
		}
	}
}

/// set zero on unused track in track data table
void DISK::trim_track_data_table()
{
	uint8_t *sp;

	for(int pos = 0; pos < 164; pos++) {
		sp = GET_TRACK_DATA_TBL_PTR(m_buffer, pos);
		int trkpos = (int)conv_to_uint32_le(sp);
		if (m_file_size <= trkpos) {
			conv_from_uint32_le(sp, 0);
		}
	}
}

/// calculate one track size
int DISK::calc_track_size(uint8_t *t)
{
	int size = 0;
	int num_of_sec = conv_to_uint16_le(t + 4);

	for(int n = 0; n < num_of_sec; n++) {
		uint16_t next = conv_to_uint16_le(t + size +14);
		size += next + 16;
	}
	return size;
}

/// add file size to buffer
void DISK::add_file_size(int offset)
{
	m_file_size += offset;
	conv_from_uint32_le(m_buffer + 0x1c, (uint32_t)m_file_size);
}

/// get sector
/// @param [in] trk : track number
/// @param [in] side : side number
/// @param [in] sect : sector number
/// @return 0:OK, 1:sector not found, 2: disk not inserted, 3:invalid media type
int DISK::get_sector(int trk, int side, int sect)
{
	return get_sector_by_index(trk, side, sector_pos[sect]);
}

/// get sector
/// @param [in] trk : track number
/// @param [in] side : side number
/// @param [in] index : sector position from top
/// @return 0:OK, 1:sector not found, 2: disk not inserted, 3:invalid media type
int DISK::get_sector_by_index(int trk, int side, int index)
{
	sector_size = sector_nums = 0;
	sector_data = NULL;
	sector_id = NULL;
	density = NULL;

	// disk inserted?
	if(!inserted) {
		return 2;
	}
	// valid media type?
	if(!check_media_type()) {
		return 3;
	}

	// search track
	uint8_t* t = get_track_buffer(trk, side);

	// track found

	if(index >= sector_nums) {
		return 1;
	}

	// skip sector
	for(int i = 0; i < index; i++) {
		t += (t[0xe] | (t[0xf] << 8)) + 0x10;
	}
	set_sector_info(t);

	return 0;
}

#if 0
bool DISK::get_sector(int trk, int side, int sect, bool cmp_side)
{
	sector_size = sector_nums = 0;
	sector_data = NULL;
	sector_id = NULL;
	density = NULL;

	// disk not inserted or invalid media type
	if(!(inserted && check_media_type())) {
		return false;
	}

	// search track
	uint8_t* t = get_track_buffer(trk, side);

	// track found

	if(sect >= sector_num) {
		return false;
	}

	// search sector
	bool match = false;
	for(int i = 0; i < sector_num; i++) {
		set_sector_info(t);

		// check sector in id field
		// check side in id field
		if(id[2] == sect && (!cmp_side || id[1] == side)) {
			match = true;
			break;
		}

		t += (t[0xe] | (t[0xf] << 8)) + 0x10;
	}

	return match;
}
#endif

void DISK::set_sector_info(uint8_t *t)
{
	// header info
	sector_id = t;
	id[0] = t[0];
	id[1] = t[1];
	id[2] = t[2];
	id[3] = t[3];

	uint16_t crc = CRC16_INIT_DATA;
	if (!(t[6] & 0x40)) {
		// double density
		crc = set_crc16(0xa1, crc);
		crc = set_crc16(0xa1, crc);
		crc = set_crc16(0xa1, crc);
	}
	crc = set_crc16(0xfe, crc);
	crc = set_crc16(t[0], crc);
	crc = set_crc16(t[1], crc);
	crc = set_crc16(t[2], crc);
	crc = set_crc16(t[3], crc);

	id[4] = crc >> 8;
	id[5] = crc & 0xff;
	density = &t[6];
	deleted = t[7];
	status = t[8];
	sector_data = t + 0x10;
//	sector_size = t[0xe] | (t[0xf] << 8);
	sector_size = (128 << id[3]);
}

bool DISK::check_media_type()
{
	if (FLG_CHECK_FDMEDIA) {
		switch(m_drive_type) {
		case DRIVE_TYPE_2D:
			return (m_media_type == MEDIA_TYPE_2D);
		case DRIVE_TYPE_2DD:
			return (m_media_type == MEDIA_TYPE_2DD);
		case DRIVE_TYPE_2HD:
			return (m_media_type == MEDIA_TYPE_2HD);
		default: // case DRIVE_TYPE_UNK:
			return false;
		}
	}
	return true;
}

bool DISK::check_d88_tracks()
{
	// disk not inserted or invalid media type
	if(!inserted) {
		return false;
	}

	// search track
	int prev_track = -1;
	int prev_side = -1;
	int max_side = -1;
	int max_trkside = -1;
	for(int trkside = 0; trkside < 164; trkside++) {
		uint32_t offset = conv_to_uint32_le(GET_TRACK_DATA_TBL_PTR(m_buffer, trkside));
		if (offset == 0) continue;

		int track = m_buffer[offset];
		int side = m_buffer[offset+1];
		if (track == prev_track && side == prev_side) {
			// track and side number are duplicate.
			side++;
		}
		if (max_side < side) max_side = side;
		if (max_trkside < trkside) max_trkside = trkside;

		prev_track = track;
		prev_side = side;
	}
	if (max_side < 0) { // no side or unformat
		// default 2 sides
		max_side = 1;
	} else if (max_side == 0) {
		// single side ?
		if (max_trkside >= 40) {
			// double side
			max_side = 1;
		}
	}
	num_of_side = max_side + 1;
	if (num_of_side > 2) num_of_side = 2;

	return true;
}

///
uint16_t DISK::set_crc16(uint8_t data, uint16_t crc)
{
#ifndef USE_CALC_CRC16
	crc = (uint16_t)((crc << 8) ^ crc16_table[(uint8_t)(crc >> 8) ^ data);
#else
	crc = calc_crc16(data, crc);
#endif
	return crc;
}

#ifdef USE_CALC_CRC16
///
uint16_t DISK::calc_crc16(uint8_t data, uint16_t crc)
{
	for (int count = 7; count >= 0; count--) {
		uint16_t bit = (((crc >> 15) ^ (data >> 7)) & 1);
		crc <<= 1;
		crc |= bit;
		data <<= 1;
		if (bit) {
			crc ^= 0x1020;
		}
	}
	return crc;
}
#endif

bool DISK::is_same_file(const _TCHAR *path, int offset)
{
	if (!inserted) return false;

#ifdef _WIN32
	if(_tcsicmp(m_file_path, path) == 0 && m_file_offset == offset) {
#else
	if(_tcscmp(m_file_path, path) == 0 && m_file_offset == offset) {
#endif
		return true;
	} else {
		return false;
	}
}
