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

class HARDDISK
{
private:
	FILEIO *fio;
	CTchar m_file_path;

	int m_drive_num;
	int m_header_size;

	int m_cylinders;
	int m_surfaces;
	int m_sectors;
	int m_sector_size;
	int m_sector_total;

	int m_curr_block;

	bool m_access;
	bool m_write_protected;

	/// usec
	enum en_access_times {
		TIME_MOTOR_ON = 8000,		///< time to ready 
		TIME_TRACK_TO_TRACK = 5000,	///< time to move track to track
		TIME_SECTOR_TO_SECTOR = 50,	///< time to move sector
	};

	inline bool is_valid_block(int block) const;
	inline int get_cylinder(int block);

public:
	HARDDISK(int drv);
	~HARDDISK();
	
	bool open(const _TCHAR* file_path, int default_sector_size, uint32_t flags);
	void close();
	bool mounted();
	bool accessed();
	bool is_same_file(const _TCHAR *file_path);
	bool is_write_protected();
	bool read_buffer(int block, int length, uint8_t *buffer);
	bool write_buffer(int block, int length, const uint8_t *buffer);
	bool format_disk();
	bool format_track(int block);
	bool seek(int block);

	int get_current_block() const { return m_curr_block; }
	int get_cylinder_diff(int block);
	int calc_access_time(int diff);
	int get_access_time(int block);
	static int get_cylinder_to_cylinder_delay();

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

