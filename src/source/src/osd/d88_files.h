/** @file d88_files.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.09.01

	@brief [d88 files]
*/

#ifndef D88_FILES_H
#define D88_FILES_H

#include "../common.h"
#include "../cchar.h"
#include "../cptrlist.h"
#include "../vm/vm_defs.h"


// for d88 bank switch
#define MAX_D88_BANKS 50

/// @brief offset on each bank in d88 multi volume disk image
class D88Bank
{
private:
	CTchar name;
	int    offset;

	D88Bank(const D88Bank &);

public:
	D88Bank();
	~D88Bank();
	void Clear();

	void SetName(const char *val);
	const _TCHAR *GetName() const { return name.Get(); }
	int GetNameLength() const { return name.Length(); }
	int GetOffset() const { return offset; }
	void SetOffset(int val) { offset = val; }
};

/// @brief bank list on d88 multi volume disk image
class D88Banks : public CPtrList<D88Bank>
{
private:
	D88Banks(const D88Banks &);

public:
	D88Banks();
	~D88Banks();
};

/// @brief current bank on each drive 
class D88File
{
private:
	CTchar path;
	D88Banks banks;
	int curr_bank;
	int prev_bank;
	D88Bank  dummy;

	D88File(const D88File &);

public:
	D88File();
	~D88File();
	void Clear();
	void ClearBank();

	void SetBank(int bank_num);
	bool IsChangedBank() const;
	void ChangeBank();

	void SetPath(const _TCHAR *val);
	const _TCHAR *GetPath() const { return path.Get(); }
	D88Banks &GetBanks() { return banks; }
	const D88Bank *GetBank(int idx) const;
	void SetCurrentBank(int val) { curr_bank = val; }
	int GetCurrentBank() const { return curr_bank; }
	void SetPreviousBank(int val) { prev_bank = val; }
	int GetPreviousBank() const { return prev_bank; }
};

/// @brief parse d88 multi volume disk image 
class D88Files
{
private:
	D88File files[USE_FLOPPY_DISKS];

	D88Files(const D88Files &);

public:
	D88Files();
	~D88Files();

	int Parse(int drv, const _TCHAR* file_path, int bank_num);
	D88File &GetFile(int drv);
};


#endif /* D88_FILES_H */
