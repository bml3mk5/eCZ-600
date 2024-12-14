/** @file rec_common.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.11.18 -

	@brief [ record common ]
*/

#include "rec_common.h"
#include "../config.h"
#include "../utility.h"

REC_COMMON::REC_COMMON()
{
	m_name_empty = true;
	memset(m_file_name, 0, sizeof(m_file_name));
}

REC_COMMON::~REC_COMMON()
{
}

void REC_COMMON::CreateFileName(bool new_name, _TCHAR *file_path, size_t maxlen, const char *extension, const _TCHAR *prefix, const _TCHAR *postfix)
{
	const _TCHAR *app_path;
	app_path = pConfig->snapshot_path.Length() > 0 ? pConfig->snapshot_path.Get() : emu->application_path();

	if (new_name || m_name_empty) {
		UTILITY::create_date_file_name(m_file_name, sizeof(m_file_name) / sizeof(m_file_name[0]));
		m_name_empty = false;
	} else {
		m_name_empty = true;
	}
	UTILITY::create_file_path(app_path, file_path, _MAX_PATH, m_file_name, extension, prefix, postfix);
}

REC_COMMON gRecCommon;
