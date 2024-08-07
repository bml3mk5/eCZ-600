/** @file harddisk.cpp

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

	fio = new FILEIO();

	clear_param();

	m_write_protected = 0;

//	set_device_name(_T("Hard Disk Drive #%d"), drv + 1);
}
HARDDISK::~HARDDISK()
{
	close();
	delete fio;
}

/// @brief Clear parameters for initialize
void HARDDISK::clear_param()
{
	m_device_type = DEVICE_TYPE_SASI_HDD;
	m_header_size = 0;
	m_cylinders = 1;
	m_surfaces = 1;
	m_sectors = 1;
	m_sector_size = 512;
	m_sector_total = 1;

	m_curr_block = 0;

	m_access = false;
}

/// @brief Open a hard disk image
///
/// @param[in] file_path   : path of image file
/// @param[in] flags       : bit0: read only
/// @param[in] sector_size : sector size
/// @return true = success
bool HARDDISK::open(const _TCHAR* file_path, uint32_t flags, int sector_size)
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

	m_write_protected = fio->IsFileProtected(file_path) ? WP_HOST : 0;
	if (flags & OPEN_DISK_FLAGS_READ_ONLY) {
		m_write_protected = WP_HOST;
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

	} else if(UTILITY::check_file_extension(file_path, _T(".hdf"))) {
		// solid
		// SASI disk
		m_device_type = DEVICE_TYPE_SASI_HDD;
		m_header_size = 0;
		// sectors = 33, surfaces = 4, cylinders = 153, sector_size = 256	// 5MB
		// sectors = 33, surfaces = 4, cylinders = 309, sector_size = 256	// 10MB
		// sectors = 33, surfaces = 4, cylinders = 614, sector_size = 256	// 20MB
		// sectors = 33, surfaces = 8, cylinders = 614, sector_size = 256	// 40MB
		m_sector_size = sector_size ? sector_size : 256;
		m_sectors = (m_sector_size == 1024) ? 8 : (m_sector_size == 512) ? 17 : 33;
		m_surfaces = ((int)(fio->FileLength() / 1024 / 1024) < 24) ? 4 : 8;
		m_sector_total = fio->FileLength() / m_sector_size;
		m_cylinders = m_sector_total / m_surfaces / m_sectors;

	} else if(UTILITY::check_file_extension(file_path, _T(".mos"))) {
		// solid
		// SCSI MO 3.5inch disk
		m_device_type = DEVICE_TYPE_SCSI_MO;
		m_header_size = 0;
		m_sector_size = fio->FileLength() <= 540000000 ? 512 : 2048;
		m_sector_total = fio->FileLength() / m_sector_size;
		// MO has not following parameters, so values are incorrect
		m_cylinders = 1;
		m_surfaces = 2;
		m_sectors = m_sector_total / m_surfaces / m_cylinders;

	} else {
		// solid
		// SCSI disk
		m_device_type = DEVICE_TYPE_SCSI_HDD;
		m_header_size = 0;
		m_sector_size = sector_size ? sector_size : 512;
		m_sector_total = fio->FileLength() / m_sector_size;
		// following calcration is so unreliable
		int remain = m_sector_total;
		for(int n=5; n>=0; n--) {
			int m = (1 << n);	// 32,16,8,4,2,1
			if ((remain % m) == 0) {
				m_sectors = m;
				remain /= m;
				break;
			}
		}
		for(int n=3; n>=0; n--) {
			int m = (1 << n);	// 8,4,2,1
			if ((remain % m) == 0) {
				m_surfaces = m;
				remain /= m;
				break;
			}
		}
		m_cylinders = remain;

	}

	return true;
}

/// @brief Close the hard disk image
void HARDDISK::close()
{
	// write disk image
	if(fio->IsOpened()) {
		fio->Fclose();
		m_file_path.Clear();
	}
	clear_param();
}

/// @brief Is file already opened?
///
/// @return true if open
bool HARDDISK::mounted() const
{
	return fio->IsOpened();
}

/// @brief Is file already opened and the same device?
///
/// @param[in] device_type : device type
/// @return true if the same
bool HARDDISK::is_valid_disk(int device_type) const
{
	return mounted() && (m_device_type == device_type);
}

/// @brief Is file accessed for reading or writing?
///
/// @return true if access
bool HARDDISK::accessed()
{
	bool value = m_access;
	m_access = false;
	return value;
}

/// @brief Is file the same?
///
/// @param[in] file_path : path of image file
/// @return true if the same
bool HARDDISK::is_same_file(const _TCHAR *file_path)
{
	return (m_file_path.Compare(file_path, (int)_tcslen(file_path)) == 0);
}

/// @brief Set or clear the write protection flag
///
/// When set write protect, flush disk image to file.
/// @param[in] val : true if set
void HARDDISK::set_write_protect(bool val)
{
	if (val == true && m_write_protected == 0) {
		// when set write protect, flush disk image to file.
		fio->Flush();
	}
	BIT_ONOFF(m_write_protected, WP_USER, val);
}

/// @brief Is file write protected?
///
/// @return true if yes
bool HARDDISK::is_write_protected() const
{
	return (m_write_protected != 0);
}

/// @brief Read data in the image file
///
/// @param[in] block          : block number
/// @param[in] length         : data size
/// @param[out] buffer        : read data
/// @param[out] cylinder_diff : moved distance on cylinder (optional)
/// @return true if success
bool HARDDISK::read_buffer(int block, int length, uint8_t *buffer, int *cylinder_diff)
{
	if(!mounted()) return false;
	if (!is_valid_block(block)) return false;
	if (cylinder_diff) {
		*cylinder_diff = get_cylinder_diff(block);
	}
	m_curr_block = block;
	if(fio->Fseek(m_header_size + block * m_sector_size, FILEIO::SEEKSET) == 0) {
		m_access = true;
		return (fio->Fread(buffer, length, 1) == 1);
	}
	return false;
}

/// @brief Write data to the image file
///
/// @param[in] block          : block number
/// @param[in] length         : data size
/// @param[in] buffer         : data to write
/// @param[out] cylinder_diff : moved distance on cylinder (optional)
/// @return true if success
bool HARDDISK::write_buffer(int block, int length, const uint8_t *buffer, int *cylinder_diff)
{
	if (!mounted()) return false;
	if (!is_valid_block(block)) return false;
	if (cylinder_diff) {
		*cylinder_diff = get_cylinder_diff(block);
	}
	m_curr_block = block;
	if(fio->Fseek(m_header_size + block * m_sector_size, FILEIO::SEEKSET) == 0) {
		m_access = true;
		return (fio->Fwrite(buffer, length, 1) == 1);
	}
	return false;
}

/// @brief Verify data in the image file
///
/// @param[in] block          : block number
/// @param[in] length         : data size
/// @param[in] buffer         : data to verify
/// @param[out] cylinder_diff : moved distance on cylinder (optional)
/// @return true if success
bool HARDDISK::verify_buffer(int block, int length, const uint8_t *buffer, int *cylinder_diff)
{
	if (!mounted()) return false;
	if (!is_valid_block(block)) return false;
	if (cylinder_diff) {
		*cylinder_diff = get_cylinder_diff(block);
	}
	m_curr_block = block;
	if(fio->Fseek(m_header_size + block * m_sector_size, FILEIO::SEEKSET) == 0) {
		m_access = true;
		uint8_t *rbuffer = new uint8_t[length + 1];
		int n = (int)fio->Fread(rbuffer, length, 1);
		if (n != 1) return false;
		n = memcmp(buffer, rbuffer, length);
		delete [] rbuffer;
		return (n == 0);
	}
	return false;
}

/// @brief Format the image file (zero padding)
///
/// @return true if success
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

/// @brief Format a track (zero padding)
///
/// @param[in] block          : block number
/// @param[out] cylinder_diff : moved distance on cylinder (optional)
/// @return true if success
bool HARDDISK::format_track(int block, int *cylinder_diff)
{
	if (!mounted()) return false;
	if (!is_valid_block(block)) return false;
	if (cylinder_diff) {
		*cylinder_diff = get_cylinder_diff(block);
	}
	m_curr_block = block;
	if(fio->Fseek(m_header_size + block * m_sector_size, FILEIO::SEEKSET) == 0) {
		int track_size = m_sectors * m_sector_size;
		fio->Fsets(0, track_size);
		return true;
	}
	return false;
}

/// @brief Seek
///
/// @param[in] block          : block number
/// @param[out] cylinder_diff : moved distance on cylinder (optional)
/// @return true if success
bool HARDDISK::seek(int block, int *cylinder_diff)
{
	if (!mounted()) return false;
	if (!is_valid_block(block)) return false;
	if (cylinder_diff) {
		*cylinder_diff = get_cylinder_diff(block);
	}
	m_curr_block = block;
	return true;
}

/// @brief Does the block exists?
///
/// @param[in] block          : block number
/// @return true if yes
bool HARDDISK::is_valid_block(int block) const
{
	return (block < m_sector_total);
}

/// @brief Get cylinder number from block number
///
/// @param[in] block          : block number
/// @return number
int HARDDISK::get_cylinder(int block)
{
	int track = (block / m_sectors / m_surfaces);
	return track % m_cylinders;
}

/// @brief Calculate distance from current cylinder position
///
/// @param[in] block          : block number
/// @return distance
/// @note difference is 10 max
int HARDDISK::get_cylinder_diff(int block)
{
	int curr_cylinder = get_cylinder(m_curr_block);
	int next_cylinder = get_cylinder(block);
	int diff = abs(curr_cylinder - next_cylinder);
	if (diff > 10) diff = 10;
	return diff;
}

/// @brief Calculate access time
///
/// @param[in] diff : difference
/// @return time
int HARDDISK::calc_access_time(int diff)
{
	if (diff <= 0) {
		diff = TIME_SECTOR_TO_SECTOR;
	} else {
		diff *= TIME_TRACK_TO_TRACK;
	}
	return diff;
}

/// @brief Get the access time to reach specified block
///
/// @param[in] block          : block number
/// @return time
int HARDDISK::get_access_time(int block)
{
	int diff = get_cylinder_diff(block);
	return calc_access_time(diff);
}

/// @brief The time to move to the next cylinder
///
/// @return time
int HARDDISK::get_cylinder_to_cylinder_delay()
{
	return TIME_TRACK_TO_TRACK;
}
