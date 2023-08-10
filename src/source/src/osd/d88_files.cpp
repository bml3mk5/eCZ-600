/** @file d88_files.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.09.01

	@brief [d88 files]
*/


#include "d88_files.h"
#include "../fileio.h"
#include "../utility.h"

//

D88Bank::D88Bank()
{
	Clear();
}
D88Bank::D88Bank(const D88Bank &)
{
	Clear();
}
D88Bank::~D88Bank()
{
}
void D88Bank::Clear()
{
	name.Clear();
	offset = 0;
}
void D88Bank::SetName(const char *val)
{
	name.SetN(val);
}

//

D88Banks::D88Banks()
	: CPtrList<D88Bank>()
{
}
D88Banks::D88Banks(const D88Banks &src)
	: CPtrList<D88Bank>(src)
{
}
D88Banks::~D88Banks()
{
}

//

D88File::D88File()
{
	Clear();
}
D88File::D88File(const D88File &)
{
	Clear();
}
D88File::~D88File()
{
}
void D88File::Clear()
{
	path.Clear();
	ClearBank();
}
void D88File::ClearBank()
{
	banks.Clear();
	curr_bank = -1;
	prev_bank = -2;
}
void D88File::SetBank(int bank_num)
{
	curr_bank = bank_num;
	prev_bank = -2;
}
bool D88File::IsChangedBank() const
{
	return (curr_bank != prev_bank);
}
void D88File::ChangeBank()
{
	prev_bank = curr_bank;
}
void D88File::SetPath(const _TCHAR *val)
{
	path.Set(val);
}
const D88Bank *D88File::GetBank(int idx) const
{
	if (0 <= idx && idx < banks.Count()) {
		return banks[idx];
	} else {
		return &dummy;
	}
}

//

D88Files::D88Files()
{
}
D88Files::D88Files(const D88Files &)
{
}
D88Files::~D88Files()
{
}
/// @brief check multi volumes and get name in each volume on d88 disk image
///
/// @param[in] drv       : drive number
/// @param[in] file_path : disk image file path
/// @param[in] bank_num  : bank (volume) number
/// @return bank number
int D88Files::Parse(int drv, const _TCHAR* file_path, int bank_num)
{
	files[drv].ClearBank();
	files[drv].SetPath(file_path);

	if (drv < 0 || MAX_DRIVE <= drv) {
		return 0;
	}

	if(!(UTILITY::check_file_extensions(file_path, _T(".d88"), _T(".d77"), NULL))) {
		return 0;
	}

	FILEIO fio;
	if(!fio.Fopen(file_path, FILEIO::READ_BINARY)) {
		return 0;
	}

	try {
		fio.Fseek(0, FILEIO::SEEKEND);
		int file_size = (int)fio.Ftell();
		int file_offset = 0;
		while(file_offset + 0x2b0 <= file_size && files[drv].GetBanks().Count() < MAX_D88_BANKS) {
			D88Bank *new_bank = new D88Bank();

			files[drv].GetBanks().Add(new_bank);

			new_bank->SetOffset(file_offset);

			fio.Fseek(file_offset, FILEIO::SEEKSET);
			char name[20];
			fio.Fread(name, sizeof(char), 17);
			name[17] = '\0';

			new_bank->SetName(name);

			fio.Fseek(file_offset + 0x1c, FILEIO::SEEKSET);

			file_offset += fio.FgetUint32_LE();
		}
	}
	catch(...) {
		files[drv].ClearBank();
		bank_num = 0;
	}
	return bank_num;
}

D88File &D88Files::GetFile(int drv)
{
	if (drv < 0 || MAX_DRIVE <= drv) {
		drv = 0;
	}
	return files[drv];
}
