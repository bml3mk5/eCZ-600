/** @file harddisk.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.11.03 -

	@note Original author is Takeda.Toshiya

	@brief [ hard disk ]
*/

#ifndef HARDDISK_H
#define HARDDISK_H

#include "vm.h"
//#include "../emu.h"
#include "../cchar.h"

class FILEIO;

/**
	@brief hard disk image handler
*/
class HARDDISK
{
public:
	enum en_device_types {
		DEVICE_TYPE_SASI_HDD = 0,
		DEVICE_TYPE_SCSI_HDD,
		DEVICE_TYPE_SCSI_MO,
		DEVICE_TYPE_MAX
	};

private:
	FILEIO *fio;
	CTchar m_file_path;

	en_device_types m_device_type;
	int m_drive_num;
//	int m_default_sector_size;
	int m_header_size;

	int m_cylinders;
	int m_surfaces;
	int m_sectors;
	int m_sector_size;
	int m_sector_total;

	int m_curr_block;

	bool m_access;
	int m_write_protected;
	enum en_write_protect_flags {
		WP_HOST = 0x01,
		WP_USER = 0x02,
	};

	/// usec
	enum en_access_times {
		TIME_MOTOR_ON = 8000,		///< time to ready 
		TIME_TRACK_TO_TRACK = 5000,	///< time to move track to track
		TIME_SECTOR_TO_SECTOR = 50,	///< time to move sector
	};

	inline bool is_valid_block(int block) const;
	inline int get_cylinder(int block);
	inline int get_cylinder_diff(int block);

public:
	HARDDISK(int drv);
	~HARDDISK();
	
	void clear_param();
	bool open(const _TCHAR* file_path, uint32_t flags, int sector_size = 0);
	void close();
	bool mounted() const;
	bool is_valid_disk(int device_type) const;
	bool accessed();
	bool is_same_file(const _TCHAR *file_path);
	void set_write_protect(bool val);
	bool is_write_protected() const;
	bool read_buffer(int block, int length, uint8_t *buffer, int *cylinder_diff);
	bool write_buffer(int block, int length, const uint8_t *buffer, int *cylinder_diff);
	bool verify_buffer(int block, int length, const uint8_t *buffer, int *cylinder_diff);
	bool format_disk();
	bool format_track(int block, int *cylinder_diff);
	bool seek(int block, int *cylinder_diff);

	int get_current_block() const { return m_curr_block; }
//	int get_cylinder_diff(int block);
	int calc_access_time(int diff);
	int get_access_time(int block);
	static int get_cylinder_to_cylinder_delay();

	int get_sector_total() const { return m_sector_total; }
	int get_sector_size() const { return m_sector_size; }

#if 0
	// device name
	void set_device_name(const _TCHAR* format, ...)
	{
		if(format != NULL) {
			va_list ap;
			_TCHAR buffer[1024];
			
			va_start(ap, format);
			my_vstprintf_s(buffer, 1024, format, ap);
			va_end(ap);
			
			my_tcscpy_s(this_device_name, 128, buffer);
		}
	}
	const _TCHAR *get_device_name()
	{
		return (const _TCHAR *)this_device_name;
	}
	_TCHAR this_device_name[128];
#endif
};

#endif /* HARDDISK_H */

