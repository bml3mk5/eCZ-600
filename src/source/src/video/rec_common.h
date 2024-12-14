/** @file rec_common.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.11.18 -

	@brief [ record common ]
*/

#ifndef RECORD_COMMON_H
#define RECORD_COMMON_H

#include "../common.h"
#include "../rec_video_defs.h"

/**
	@brief Record common
*/
class REC_COMMON
{
private:
	bool m_name_empty;
	_TCHAR m_file_name[64];

public:
	REC_COMMON();
	~REC_COMMON();
	void CreateFileName(bool new_name, _TCHAR *file_path, size_t maxlen, const char *extension, const _TCHAR *prefix = NULL, const _TCHAR *postfix = NULL);
};

extern REC_COMMON gRecCommon;

#endif /* RECORD_COMMON_H */
