/** @file disk.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.09.16-

	@brief [ d88 image handler ]
*/

#ifndef DISK_H
#define DISK_H

#include "../common.h"
#include "../d88_defs.h"
//#include "disk_parser.h"
//#include <stdlib.h>

// use crc calcrate function
#define USE_CALC_CRC16

// d88 constant
#define DISK_BUFFER_SIZE	0x180000	// 1.5MB
#define TRACK_BUFFER_SIZE	0x40000		// 256KB

// track size
// preamble 146 + 16sectors 5936 + postamble (598)
#define TRACK_SIZE_5INCH_2D	0x1800
// preamble 146 + 26sectors 9646 + postamble (598)
#define TRACK_SIZE_8INCH_2D	0x2700

class FILEIO;

/**
	@brief d88 image handler
*/
class DISK
{
public:
	enum en_image_types {
		IMAGE_TYPE_D88 = 0,
		IMAGE_TYPE_D88_CONVERTED,
		IMAGE_TYPE_PLAIN,
		IMAGE_TYPE_UNK = 0xff
	};
private:
	int m_drive_num;
	uint8_t m_image_type;
	uint8_t m_media_type;
	uint8_t m_drive_type;
	uint8_t m_buffer[DISK_BUFFER_SIZE];
	uint8_t m_tmp_track[TRACK_BUFFER_SIZE];
	int m_tmp_track_size;

	_TCHAR m_file_path[_MAX_PATH];
	int m_file_size, m_file_size_orig;
	int m_file_offset;
	uint32_t m_crc32;
	bool m_last_volume;
	bool m_filename_changed;

	bool flash(bool protect);

	bool check_media_type();
	bool check_d88_tracks();

	int get_track_offset(int trk, int side);
	uint8_t* get_track_buffer(int trk, int side);
	void set_sector_info(uint8_t *t);

	void replace_track(int trk, int side);
	void shrink_track_space(int start, int size);
	void expand_track_space(int start, int size);
	int find_track_side(int trkside, int *offset);
	void recalc_track_data_table(int start, int offset);
	void trim_track_data_table();
	int calc_track_size(uint8_t *t);
	void add_file_size(int offset);

	void rename_file();
	bool write_file(bool new_file, bool is_plain);

	static uint32_t getcrc32(uint8_t data[], int size);

public:
	DISK(int drv);
	~DISK();

	bool open(const _TCHAR *path, int offset, uint32_t flags = 0);
	void close();
	void flash();
	void set_write_protect(bool val);
	bool get_track(int trk, int side);
	bool verify_track(int trk);
	bool make_track(int trk, int side, int dden);
	bool parse_track(int trk, int side, int dden);
	bool parse_track2(int trk, int side, int dden);
	int  get_sector(int trk, int side, int sect);
	int  get_sector_by_index(int trk, int side, int index);
//	bool get_sector(int trk, int side, int sect, bool cmp_side);

	static uint16_t set_crc16(uint8_t data, uint16_t crc);
#ifdef USE_CALC_CRC16
	static uint16_t calc_crc16(uint8_t data, uint16_t crc);
#endif

	bool is_same_file(const _TCHAR *path, int offset);

	void set_image_type(uint8_t val) { m_image_type = val; }
	uint8_t get_image_type() const { return m_image_type; }
	void set_drive_type(uint8_t val) { m_drive_type = val; }
	uint8_t get_drive_type() const { return m_drive_type; }

	void set_file_size(int val) { m_file_size = val; }
	int get_file_size() const { return m_file_size; }

	bool inserted;
	bool ejected;
	bool write_protected;
	bool changed;

	//
	int num_of_side;

	// track (use on reading/writing a track operation)
	uint8_t track[TRACK_BUFFER_SIZE];
	int track_size;
	int sector_nums;

	// sector
	uint8_t* sector_id;
	uint8_t* sector_data;
	int sector_size;
	uint8_t id[6];
	uint8_t* density;
	uint8_t deleted;
	uint8_t status;
	uint8_t id_c_in_track[128];		///< C number in current track
	uint8_t sector_pos[256];	///< sector position in current track
};

#endif /* DISK_H */

