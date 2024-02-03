/** @file harddisk.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.11.03 -

	@note Original author is Takeda.Toshiya

	@brief [ hard disk ]
*/

#include "harddisk.h"
#include "../emu.h"
#include "../fileio.h"
#include "../utility.h"
#include <math.h>
#include <stdlib.h>

extern EMU *emu;

HARDDISK::HARDDISK(int drv)
{
	m_drive_num = drv;
	m_header_size = 0;

	fio = new FILEIO();

	m_cylinders = 1;
	m_surfaces = 1;
	m_sectors = 1;
	m_sector_size = 256;
	m_sector_total = 1;

	m_curr_block = 0;

	m_access = false;
	m_write_protected = false;

//	set_device_name(_T("Hard Disk Drive #%d"), drv + 1);
}
HARDDISK::~HARDDISK()
{
	close();
	delete fio;
}

/// @return true = success
bool HARDDISK::open(const _TCHAR* file_path, int default_sector_size, uint32_t flags)
{
	uint8_t header[512];
//	pair32_t tmp;
	
	close();
	
	if(!FILEIO::IsFileExists(file_path)) {
		return false;
	}
	if(!fio->Fopen(file_path, FILEIO::READ_WRITE_BINARY)) {
		return false;
	}

	m_file_path.Set(file_path);

	m_write_protected = fio->IsFileProtected(file_path);
	if (flags & OPEN_DISK_FLAGS_READ_ONLY) {
		m_write_protected = true;
	}


	// from NP2 sxsihdd.c
	const char sig_vhd[8] = "VHD1.00";
	const char sig_nhd[15] = "T98HDDIMAGE.R0";
			
	fio->Fread(header, 256, 1);
			
	if(UTILITY::check_file_extension(file_path, _T(".thd"))) {
		// T98
/*
		typedef struct thd_header_s {
			int16_t cylinders;
		} thd_header_s;
*/
		m_header_size = 256;

		m_cylinders = conv_to_int16_le(header + 0);
		m_surfaces = 8;
		m_sectors = 33;
		m_sector_size = 256;
		m_sector_total = m_cylinders * m_surfaces * m_sectors;

	} else if(UTILITY::check_file_extension(file_path, _T(".nhd")) && memcmp(header, sig_nhd, 15) == 0) {
		// T98-Next
/*
		typedef struct nhd_header_s {
			char sig[16];
			char comment[256];
			int32_t header_size;	// +272
			int32_t cylinders;	// +276
			int16_t surfaces;	// +280
			int16_t sectors;	// +282
			int16_t sector_size;	// +284
			uint8_t reserved[0xe2];
		} nhd_header_t;
*/
		m_header_size = conv_to_int32_le(header + 272);
		m_cylinders = conv_to_int32_le(header + 276);
		m_surfaces = conv_to_int16_le(header + 280);
		m_sectors = conv_to_int16_le(header + 282);
		m_sector_size = conv_to_int32_le(header + 284);
		m_sector_total = m_cylinders * m_surfaces * m_sectors;

	} else if(UTILITY::check_file_extension(file_path, _T(".hdi"))) {
		// ANEX86
/*
		typedef struct hdi_header_s {
			int32_t dummy;		// + 0
			int32_t hdd_type;	// + 4
			int32_t header_size;	// + 8
			int32_t hdd_size;	// +12
			int32_t sector_size;	// +16
			int32_t sectors;	// +20
			int32_t surfaces;	// +24
			int32_t cylinders;	// +28
		} hdi_header_t;
*/
		m_header_size = conv_to_int32_le(header + 8);
		m_sector_size = conv_to_int32_le(header + 16);
		m_sectors = conv_to_int32_le(header + 20);
		m_surfaces = conv_to_int32_le(header + 24);
		m_cylinders = conv_to_int32_le(header + 28);
		m_sector_total = m_cylinders * m_surfaces * m_sectors;

	} else if(UTILITY::check_file_extension(file_path, _T(".hdd")) && memcmp(header, sig_vhd, 5) == 0) {
		// Virtual98
/*
		typedef struct {
			char    sig[3];		// +  0
			char    ver[4];		// +  3
			char    delimita;	// +  7
			char    comment[128];	// +  8
			uint8_t pad1[4];	// +136
			int16_t mbsize;		// +140
			int16_t sectorsize;	// +142
			uint8_t sectors;	// +144
			uint8_t surfaces;	// +145
			int16_t cylinders;	// +146
			int32_t totals;		// +148
			uint8_t pad2[0x44];	// +152
		} virtual98_header_t;
*/
		m_header_size = 288;
		m_sector_size = conv_to_int16_le(header + 142);
		m_sectors = header[144];
		m_surfaces = header[145];
//		tmp.read_2bytes_le_from(header + 146);
//		int cylinders = tmp.sd;
		m_sector_total = conv_to_int32_le(header + 148);
		m_cylinders = m_sector_total / m_surfaces / m_sectors;
//		m_sector_num = cylinders * m_surfaces * m_sectors;

	} else {
		// solid
		m_header_size = 0;
		// sectors = 33/17, surfaces = 4, cylinders = 153, sector_size = 256/512	// 5MB
		// sectors = 33/17, surfaces = 4, cylinders = 309, sector_size = 256/512	// 10MB
		// sectors = 33/17, surfaces = 4, cylinders = 614, sector_size = 256/512	// 20MB
		// sectors = 33/17, surfaces = 8, cylinders = 614, sector_size = 256/512	// 40MB
#if 0
		m_surfaces = (fio->FileLength() <= 17 * 4 * 615 * 512) ? 4 : 8;
#else
		m_surfaces = ((int)(fio->FileLength() / 1024 / 1024) < 24) ? 4 : 8;
#endif
		m_sectors = (default_sector_size == 1024) ? 8 : (default_sector_size == 512) ? 17 : 33;
		m_sector_size = default_sector_size;
		m_sector_total = fio->FileLength() / m_sector_size;
		m_cylinders = m_sector_total / m_surfaces / m_sectors;

	}

	return true;
}

void HARDDISK::close()
{
	// write disk image
	if(fio->IsOpened()) {
		fio->Fclose();
		m_file_path.Clear();
	}
}

bool HARDDISK::mounted()
{
	return fio->IsOpened();
}

bool HARDDISK::accessed()
{
	bool value = m_access;
	m_access = false;
	return value;
}

bool HARDDISK::is_same_file(const _TCHAR *file_path)
{
	return (m_file_path.Compare(file_path, (int)_tcslen(file_path)) == 0);
}

bool HARDDISK::is_write_protected()
{
	return m_write_protected;
}

bool HARDDISK::read_buffer(int block, int length, uint8_t *buffer)
{
	if(!mounted()) return false;
	if (!is_valid_block(block)) return false;
	m_curr_block = block;
	if(fio->Fseek(m_header_size + block * m_sector_size, FILEIO::SEEKSET) == 0) {
		m_access = true;
		return (fio->Fread(buffer, length, 1) == 1);
	}
	return false;
}

bool HARDDISK::write_buffer(int block, int length, const uint8_t *buffer)
{
	if (!mounted()) return false;
	if (!is_valid_block(block)) return false;
	m_curr_block = block;
	if(fio->Fseek(m_header_size + block * m_sector_size, FILEIO::SEEKSET) == 0) {
		m_access = true;
		return (fio->Fwrite(buffer, length, 1) == 1);
	}
	return false;
}

bool HARDDISK::format_disk()
{
	if (!mounted()) return false;
	int data_size = fio->FileLength() - m_header_size;
	if(fio->Fseek(m_header_size, FILEIO::SEEKSET) == 0) {
		fio->Fsets(0, data_size);
		return true;
	}
	return false;
}

bool HARDDISK::format_track(int block)
{
	if (!mounted()) return false;
	if (!is_valid_block(block)) return false;
	m_curr_block = block;
	if(fio->Fseek(m_header_size + block * m_sector_size, FILEIO::SEEKSET) == 0) {
		int track_size = m_sectors * m_sector_size;
		fio->Fsets(0, track_size);
		return true;
	}
	return false;
}

bool HARDDISK::seek(int block)
{
	if (!mounted()) return false;
	if (!is_valid_block(block)) return false;
	m_curr_block = block;
	return true;
}

bool HARDDISK::is_valid_block(int block) const
{
	return (block < m_sector_total);
}

int HARDDISK::get_cylinder(int block)
{
	int track = (block / m_sectors / m_surfaces);
	return track % m_cylinders;
}

/// @brief difference from current cylinder position
/// @note difference is 10 max
int HARDDISK::get_cylinder_diff(int block)
{
	int curr_cylinder = get_cylinder(m_curr_block);
	int next_cylinder = get_cylinder(block);
	int diff = abs(curr_cylinder - next_cylinder);
	if (diff > 10) diff = 10;
	return diff;
}

int HARDDISK::calc_access_time(int diff)
{
	if (diff <= 0) {
		diff = TIME_SECTOR_TO_SECTOR;
	} else {
		diff *= TIME_TRACK_TO_TRACK;
	}
	return diff;
}

int HARDDISK::get_access_time(int block)
{
	int diff = get_cylinder_diff(block);
	return calc_access_time(diff);
}

int HARDDISK::get_cylinder_to_cylinder_delay()
{
	return TIME_TRACK_TO_TRACK;
}
