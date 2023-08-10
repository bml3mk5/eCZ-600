/** @file disk_parser.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.09.01

	@note Original author is Takeda.Toshiya

	@brief [ disk parser ]
*/

#include "disk_parser.h"
#include "../emu.h"
#include "../fileio.h"
#include "../utility.h"
#include "../config.h"
#include "disk.h"

#ifdef stat
#undef stat
#endif

const fd_format_t DISK_PARSER::c_fd_formats[] = {
//	{ MEDIA_TYPE_2D, 0x40, 40, 1, 16,  128, 0 },	// 1S    80KB
//	{ MEDIA_TYPE_2D, 0x40, 40, 2, 16,  128, 0 },	// 2S   160KB
//	{ MEDIA_TYPE_2D,    0, 40, 2, 16,  256, 128 },	// 2D   318KB (single density on track 0 and side 0)
//	{ MEDIA_TYPE_2D,    0, 40, 2, 16,  256, 0 },	// 2D   320KB
//	{ MEDIA_TYPE_2HD,   0, 77, 2, 26,  256, 128 },	// 2HD  997KB (like 8inch 2D)
//	{ MEDIA_TYPE_2HD,   0, 77, 2, 26,  256, 0 },	// 2HD 1001KB
//	{ MEDIA_TYPE_2DD,   0, 80, 2, 16,  256, 0 },	// 2DD  640KB (MZ-2500)
//	{ MEDIA_TYPE_2DD,   0, 80, 2,  8,  512, 0 },	// 2DD  640KB
//	{ MEDIA_TYPE_2DD,   0, 80, 2,  9,  512, 0 },	// 2DD  720KB
	{ MEDIA_TYPE_2HD,   0, 77, 2,  8, 1024, 0 },	// 2HD 1232KB
	{ MEDIA_TYPE_2HD,   0, 77, 2,  9, 1024, 0 },	// 2HD 1386KB
	{ MEDIA_TYPE_2HD,   0, 80, 2,  9, 1024, 0 },	// 2HD 1440KB
	{ -1, 0, 0, 0, 0, 0, 0 },
};

/// teledisk decoder table
const uint8_t TELEDISK_PARSER::c_teledisk_d_code[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
	0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x0e, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f,
	0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
	0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15, 0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
	0x18, 0x18, 0x19, 0x19, 0x1a, 0x1a, 0x1b, 0x1b, 0x1c, 0x1c, 0x1d, 0x1d, 0x1e, 0x1e, 0x1f, 0x1f,
	0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23, 0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
	0x28, 0x28, 0x29, 0x29, 0x2a, 0x2a, 0x2b, 0x2b, 0x2c, 0x2c, 0x2d, 0x2d, 0x2e, 0x2e, 0x2f, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};
/// teledisk decoder table length
const uint8_t TELEDISK_PARSER::c_teledisk_d_len[256] = {
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

static const int c_sector_sizes[7] = {
	128, 256, 512, 1024, 2048, 4096, 8192
};

#define COPYBUFFER(dst, dst_size, start_pos, src, src_size) { \
	if((start_pos) + (src_size) > (dst_size)) { \
		return false; \
	} \
	memcpy((dst) + (start_pos), (src), (src_size)); \
	(start_pos) += (src_size); \
}

//

TELEDISK_PARSER::TELEDISK_PARSER(FILEIO *fio_, const _TCHAR *file_path_)
{
	ptr = 0;
	bufcnt = bufndx = bufpos = 0;
	ibufcnt = ibufndx = 0;
	getbuf = 0;
	getlen = 0;

	fio = fio_;
	temporary = false;
	UTILITY::concat(tmp_path, _MAX_PATH, file_path_, _T(".$$$"), NULL);
}
TELEDISK_PARSER::~TELEDISK_PARSER()
{
	remove_tempfile();
}

int TELEDISK_PARSER::next_word()
{
	if(ibufndx >= ibufcnt) {
		ibufndx = ibufcnt = 0;
		memset(inbuf, 0, 512);
		for(int i = 0; i < 512; i++) {
			int d = fio->Fgetc();
			if(d == EOF) {
				if(i) {
					break;
				}
				return(-1);
			}
			inbuf[i] = d;
			ibufcnt = i + 1;
		}
	}
	while(getlen <= 8) {
		getbuf |= inbuf[ibufndx++] << (8 - getlen);
		getlen += 8;
	}
	return 0;
}

int TELEDISK_PARSER::get_bit()
{
	if(next_word() < 0) {
		return -1;
	}
	short i = getbuf;
	getbuf <<= 1;
	getlen--;
	return (i < 0) ? 1 : 0;
}

int TELEDISK_PARSER::get_byte()
{
	if(next_word() != 0) {
		return -1;
	}
	uint16_t i = getbuf;
	getbuf <<= 8;
	getlen -= 8;
	i >>= 8;
	return (int)i;
}

void TELEDISK_PARSER::start_huff()
{
	int i, j;
	for(i = 0; i < N_CHAR; i++) {
		freq[i] = 1;
		son[i] = i + TABLE_SIZE;
		prnt[i + TABLE_SIZE] = i;
	}
	i = 0; j = N_CHAR;
	while(j <= ROOT_POSITION) {
		freq[j] = freq[i] + freq[i + 1];
		son[j] = i;
		prnt[i] = prnt[i + 1] = j;
		i += 2; j++;
	}
	freq[TABLE_SIZE] = 0xffff;
	prnt[ROOT_POSITION] = 0;
}

void TELEDISK_PARSER::reconst()
{
	short i, j = 0, k;
	uint16_t f, l;
	for(i = 0; i < TABLE_SIZE; i++) {
		if(son[i] >= TABLE_SIZE) {
			freq[j] = (freq[i] + 1) / 2;
			son[j] = son[i];
			j++;
		}
	}
	for(i = 0, j = N_CHAR; j < TABLE_SIZE; i += 2, j++) {
		k = i + 1;
		f = freq[j] = freq[i] + freq[k];
		for(k = j - 1; f < freq[k]; k--);
		k++;
		l = (j - k) * 2;
		memmove(&freq[k + 1], &freq[k], l);
		freq[k] = f;
		memmove(&son[k + 1], &son[k], l);
		son[k] = i;
	}
	for(i = 0; i < TABLE_SIZE; i++) {
		if((k = son[i]) >= TABLE_SIZE) {
			prnt[k] = i;
		}
		else {
			prnt[k] = prnt[k + 1] = i;
		}
	}
}

void TELEDISK_PARSER::update(int c)
{
	int i, j, k, l;
	if(freq[ROOT_POSITION] == MAX_FREQ) {
		reconst();
	}
	c = prnt[c + TABLE_SIZE];
	do {
		k = ++freq[c];
		if(k > freq[l = c + 1]) {
			while(k > freq[++l]);
			l--;
			freq[c] = freq[l];
			freq[l] = k;
			i = son[c];
			prnt[i] = l;
			if(i < TABLE_SIZE) {
				prnt[i + 1] = l;
			}
			j = son[l];
			son[l] = i;
			prnt[j] = c;
			if(j < TABLE_SIZE) {
				prnt[j + 1] = c;
			}
			son[c] = j;
			c = l;
		}
	}
	while((c = prnt[c]) != 0);
}

short TELEDISK_PARSER::decode_char()
{
	int ret;
	uint16_t c = son[ROOT_POSITION];
	while(c < TABLE_SIZE) {
		if((ret = get_bit()) < 0) {
			return -1;
		}
		c += (unsigned)ret;
		c = son[c];
	}
	c -= TABLE_SIZE;
	update(c);
	return c;
}

short TELEDISK_PARSER::decode_position()
{
	short bit;
	uint16_t i, j, c;
	if((bit = get_byte()) < 0) {
		return -1;
	}
	i = (uint16_t)bit;
	c = (uint16_t)c_teledisk_d_code[i] << 6;
	j = c_teledisk_d_len[i] - 2;
	while(j--) {
		if((bit = get_bit()) < 0) {
			 return -1;
		}
		i = (i << 1) + bit;
	}
	return (c | (i & 0x3f));
}

void TELEDISK_PARSER::init_decode()
{
	ibufcnt= ibufndx = bufcnt = getbuf = 0;
	getlen = 0;
	start_huff();
	for(int i = 0; i < STRING_BUFFER_SIZE - LOOKAHEAD_BUFFER_SIZE; i++) {
		text_buf[i] = ' ';
	}
	ptr = STRING_BUFFER_SIZE - LOOKAHEAD_BUFFER_SIZE;
}

int TELEDISK_PARSER::decode(uint8_t *buf, int len)
{
	short c, pos;
	int  count;
	for(count = 0; count < len;) {
		if(bufcnt == 0) {
			if((c = decode_char()) < 0) {
				return count;
			}
			if(c < 256) {
				*(buf++) = (uint8_t)c;
				text_buf[ptr++] = (uint8_t)c;
				ptr &= (STRING_BUFFER_SIZE - 1);
				count++;
			}
			else {
				if((pos = decode_position()) < 0) {
					return count;
				}
				bufpos = (ptr - pos - 1) & (STRING_BUFFER_SIZE - 1);
				bufcnt = c - 255 + THRESHOLD;
				bufndx = 0;
			}
		}
		else {
			while(bufndx < bufcnt && count < len) {
				c = text_buf[(bufpos + bufndx) & (STRING_BUFFER_SIZE - 1)];
				*(buf++) = (uint8_t)c;
				bufndx++;
				text_buf[ptr++] = (uint8_t)c;
				ptr &= (STRING_BUFFER_SIZE - 1);
				count++;
			}
			if(bufndx >= bufcnt) {
				bufndx = bufcnt = 0;
			}
		}
	}
	return count;
}

/// @brief parse and decode teledisk, and convert to d88 disk image
///
/// @param[out] buffer        : d88 disk image
/// @param[in]  buffer_size
/// @param[out] d88_file_size : d88 disk image size in buffer
bool TELEDISK_PARSER::convert_to_d88(uint8_t *buffer, size_t buffer_size, int &d88_file_size)
{
	struct td_hdr_t hdr;
	struct td_cmt_t cmt;
	struct td_trk_t trk;
	struct td_sct_t sct;
	d88_hdr_t d88_hdr;
	d88_sct_t d88_sct;
	uint8_t obuf[512];

	// check teledisk header
	fio->Fseek(0, FILEIO::SEEKSET);
	fio->Fread(&hdr, sizeof(td_hdr_t), 1);
	if(hdr.sig[0] == 't' && hdr.sig[1] == 'd') {
		// decompress to the temporary file
		FILEIO* fo = new FILEIO();
		if(!fo->Fopen(tmp_path, FILEIO::WRITE_BINARY)) {
			delete fo;
			return false;
		}
		int rd = 1;
		init_decode();
		do {
			if((rd = decode(obuf, 512)) > 0) {
				fo->Fwrite(obuf, rd, 1);
			}
		}
		while(rd > 0);
		fo->Fclose();
		delete fo;
		temporary = true;

		// reopen the temporary file
		fio->Fclose();
		if(!fio->Fopen(tmp_path, FILEIO::READ_BINARY)) {
			return false;
		}
	}
	if(hdr.flag & 0x80) {
		// skip comment
		fio->Fread(&cmt, sizeof(td_cmt_t), 1);
		fio->Fseek(cmt.len, FILEIO::SEEKCUR);
	}

	// create d88 image
	d88_file_size = 0;

	// create d88 header
	static const int media_types[4] = {
		MEDIA_TYPE_2D, MEDIA_TYPE_2HD, MEDIA_TYPE_2DD, MEDIA_TYPE_2HD
	};
	memset(&d88_hdr, 0, sizeof(d88_hdr_t));
	strcpy(d88_hdr.title, "TELEDISK");
	d88_hdr.protect = 0; // non-protected
	d88_hdr.media_type = (hdr.type >= 1 && hdr.type <= 4) ? media_types[hdr.type - 1] : MEDIA_TYPE_UNK;
	COPYBUFFER(buffer, buffer_size, d88_file_size, &d88_hdr, sizeof(d88_hdr_t));

	// create tracks
	int trkcnt = 0, trkptr = sizeof(d88_hdr_t);
	fio->Fread(&trk, sizeof(td_trk_t), 1);
	while(trk.nsec != 0xff) {
		d88_hdr.trkptr[trkcnt++] = trkptr;

		// read sectors in this track
		for(int i = 0; i < trk.nsec; i++) {
			uint8_t buf[2048], dst[2048];
			memset(buf, 0, sizeof(buf));
			memset(dst, 0, sizeof(dst));

			// read sector header
			fio->Fread(&sct, sizeof(td_sct_t), 1);

			// create d88 sector header
			memset(&d88_sct, 0, sizeof(d88_sct_t));
			d88_sct.c = sct.c;
			d88_sct.h = sct.h;
			d88_sct.r = sct.r;
			d88_sct.n = sct.n;
			d88_sct.nsec = trk.nsec;
			d88_sct.dens = 0; // double density
			d88_sct.del = (sct.ctrl & 4) ? 0x10 : 0;
			d88_sct.stat = (sct.ctrl & 2) ? 0x10 : 0; // crc?
			d88_sct.size = c_sector_sizes[sct.n & 3];

			// create sector image
			if(sct.ctrl != 0x10) {
				// read sector source
				int len = fio->Fgetc();
				len += fio->Fgetc() * 256 - 1;
				int flag = fio->Fgetc(), d = 0;
				fio->Fread(buf, len, 1);

				// convert
				if(flag == 0) {
					memcpy(dst, buf, len);
				}
				else if(flag == 1) {
					int len2 = buf[0] | (buf[1] << 8);
					while(len2--) {
						dst[d++] = buf[2];
						dst[d++] = buf[3];
					}
				}
				else if(flag == 2) {
					for(int s = 0; s < len;) {
						int type = buf[s++];
						int len2 = buf[s++];
						if(type == 0) {
							while(len2--) {
								dst[d++] = buf[s++];
							}
						}
						else if(type < 5) {
							uint8_t pat[256];
							int n = 2;
							while(type-- > 1) {
								n *= 2;
							}
							for(int j = 0; j < n; j++) {
								pat[j] = buf[s++];
							}
							while(len2--) {
								for(int j = 0; j < n; j++) {
									dst[d++] = pat[j];
								}
							}
						}
						else {
							break; // unknown type
						}
					}
				}
				else {
					break; // unknown flag
				}
			}
			else {
				d88_sct.size = 0;
			}

			// copy to d88
			COPYBUFFER(buffer, buffer_size, d88_file_size, &d88_sct, sizeof(d88_sct_t));
			COPYBUFFER(buffer, buffer_size, d88_file_size, dst, (size_t)d88_sct.size);
			trkptr += sizeof(d88_sct_t) + d88_sct.size;
		}
		// read next track
		fio->Fread(&trk, sizeof(td_trk_t), 1);
	}
	d88_hdr.size = trkptr;
	memcpy(buffer, &d88_hdr, sizeof(d88_hdr_t));
	return true;
}

void TELEDISK_PARSER::remove_tempfile()
{
	if(temporary) {
		FILEIO::RemoveFile(tmp_path);
	}
}

// ======================================================================

DISK_PARSER::DISK_PARSER(DISK *disk, FILEIO *fio, const _TCHAR *file_path, uint8_t *buffer, size_t buffer_size)
{
	this->p_disk = disk;
	this->fio = fio;
	this->p_file_path = file_path;
	this->p_buffer = buffer;
	this->m_buffer_size = buffer_size;
	this->p_tmp_buffer = NULL;
}

DISK_PARSER::~DISK_PARSER()
{
	free_tmp_buffer();
}

void DISK_PARSER::alloc_tmp_buffer()
{
	if (!p_tmp_buffer) {
		p_tmp_buffer = new uint8_t[m_buffer_size];
	}
}

void DISK_PARSER::free_tmp_buffer()
{
	delete [] p_tmp_buffer;
	p_tmp_buffer = NULL;
}

// teledisk image decoder

bool DISK_PARSER::teledisk_to_d88()
{
	TELEDISK_PARSER tdp(fio, p_file_path);
	int d88_file_size = p_disk->get_file_size();
	bool rc = tdp.convert_to_d88(p_buffer, m_buffer_size, d88_file_size);
	p_disk->set_file_size(d88_file_size);
	p_disk->set_image_type(DISK::IMAGE_TYPE_D88_CONVERTED);
	return rc;
}

// imagedisk image decoder

bool DISK_PARSER::imagedisk_to_d88()
{
	struct imd_trk_t trk;
	d88_hdr_t d88_hdr;
	d88_sct_t d88_sct;

	// skip comment
	fio->Fseek(0, FILEIO::SEEKSET);
	int tmp;
	while((tmp = fio->Fgetc()) != 0x1a) {
		if(tmp == EOF) {
			return false;
		}
	}

	// create d88 image
	int file_size = 0;
	p_disk->set_file_size(0);

	// create d88 header
	memset(&d88_hdr, 0, sizeof(d88_hdr_t));
	strcpy(d88_hdr.title, "IMAGEDISK");
	d88_hdr.protect = 0; // non-protected
	d88_hdr.media_type = MEDIA_TYPE_UNK; // TODO
	COPYBUFFER(p_buffer, m_buffer_size, file_size, &d88_hdr, sizeof(d88_hdr_t));

	// create tracks
	int trkptr = sizeof(d88_hdr_t);
	for(int t = 0; t < 164; t++) {
		// check end of file
		if(fio->Fread(&trk, sizeof(imd_trk_t), 1) != 1) {
			break;
		}
		// check track header
		if(trk.mode > 5 || trk.size > 6) {
			return false;
		}
		if(!trk.nsec) {
			continue;
		}
		d88_hdr.trkptr[t] = trkptr;

		// setup sector id
		uint8_t c[64], h[64], r[64];
		fio->Fread(r, trk.nsec, 1);
		if(trk.head & 0x80) {
			fio->Fread(c, trk.nsec, 1);
		}
		else {
			memset(c, trk.cyl, sizeof(c));
		}
		if(trk.head & 0x40) {
			fio->Fread(h, trk.nsec, 1);
		}
		else {
			memset(h, trk.head & 1, sizeof(h));
		}

		// read sectors in this track
		for(int i = 0; i < trk.nsec; i++) {
			// create d88 sector header
			static uint8_t del[] = {0, 0, 0, 0x10, 0x10, 0, 0, 0x10, 0x10};
			static uint8_t err[] = {0, 0, 0, 0, 0, 0x10, 0x10, 0x10, 0x10};
			int sectype = fio->Fgetc();
			if(sectype > 8) {
				return false;
			}
			memset(&d88_sct, 0, sizeof(d88_sct_t));
			d88_sct.c = c[i];
			d88_sct.h = h[i];
			d88_sct.r = r[i];
			d88_sct.n = trk.size;
			d88_sct.nsec = trk.nsec;
			d88_sct.dens = (trk.mode < 3) ? 0x40 : 0;
			d88_sct.del = del[sectype];
			d88_sct.stat = err[sectype];
			d88_sct.size = c_sector_sizes[trk.size];

			// create sector image
			uint8_t dst[8192];
			if(sectype == 1 || sectype == 3 || sectype == 5 || sectype == 7) {
				// uncompressed
				fio->Fread(dst, d88_sct.size, 1);
			}
			else if(sectype == 2 || sectype == 4 || sectype == 6 || sectype == 8) {
				// compressed
				int tmp = fio->Fgetc();
				memset(dst, tmp, d88_sct.size);
			}
			else {
				d88_sct.size = 0;
			}

			// copy to d88
			COPYBUFFER(p_buffer, m_buffer_size, file_size, &d88_sct, sizeof(d88_sct_t));
			COPYBUFFER(p_buffer, m_buffer_size, file_size, dst, (size_t)d88_sct.size);
			trkptr += sizeof(d88_sct_t) + d88_sct.size;
		}
	}
	d88_hdr.size = trkptr;
	memcpy(p_buffer, &d88_hdr, sizeof(d88_hdr_t));
	p_disk->set_file_size(file_size);
	p_disk->set_image_type(DISK::IMAGE_TYPE_D88_CONVERTED);
	return true;
}

// cpdread image decoder (from MESS formats/dsk_dsk.c)

bool DISK_PARSER::cpdread_to_d88(int extended)
{
	d88_hdr_t d88_hdr;
	d88_sct_t d88_sct;
	int t = 0;

	// allocate tmp buffer
	alloc_tmp_buffer();

	// get cylinder number and side number
	int file_size = p_disk->get_file_size();
	memcpy(p_tmp_buffer, p_buffer, file_size);
	int ncyl = p_tmp_buffer[0x30];
	int nside = p_tmp_buffer[0x31];

	// create d88 image
	file_size = 0;
	p_disk->set_file_size(0);

	// create d88 header
	memset(&d88_hdr, 0, sizeof(d88_hdr_t));
	strcpy(d88_hdr.title, "CPDRead");
	d88_hdr.protect = 0; // non-protected
	d88_hdr.media_type = MEDIA_TYPE_UNK; // TODO
	COPYBUFFER(p_buffer, m_buffer_size, file_size, &d88_hdr, sizeof(d88_hdr_t));

	// create tracks
	int trkofs = 0x100, trkofs_ptr = 0x34;
	int trkptr = sizeof(d88_hdr_t);

	for(int c = 0; c < ncyl; c++) {
		for(int h = 0; h < nside; h++) {
			d88_hdr.trkptr[t++] = trkptr;
			if(nside == 1) {
				// double side
				d88_hdr.trkptr[t++] = trkptr;
			}

			// read sectors in this track
			uint8_t *track_info = p_tmp_buffer + trkofs;
			int nsec = track_info[0x15];
			int size = 1 << (track_info[0x14] + 7); // standard
			int sctofs = trkofs + 0x100;

			for(int s = 0; s < nsec; s++) {
				// get sector size
				uint8_t *sector_info = p_tmp_buffer + trkofs + 0x18 + s * 8;
				if(extended) {
					size = sector_info[6] + sector_info[7] * 256;
				}

				// create d88 sector header
				memset(&d88_sct, 0, sizeof(d88_sct_t));
				d88_sct.c = sector_info[0];
				d88_sct.h = sector_info[1];
				d88_sct.r = sector_info[2];
				d88_sct.n = sector_info[3];
				d88_sct.nsec = nsec;
				d88_sct.dens = 0;
				d88_sct.del = 0;
				d88_sct.stat = 0;
				d88_sct.size = size;

				// copy to d88
				COPYBUFFER(p_buffer, m_buffer_size, file_size, &d88_sct, sizeof(d88_sct_t));
				COPYBUFFER(p_buffer, m_buffer_size, file_size, p_tmp_buffer + sctofs, (size_t)size);
				trkptr += sizeof(d88_sct_t) + size;
				sctofs += size;
			}

			if(extended) {
				trkofs += p_tmp_buffer[trkofs_ptr++] * 256;
			}
			else {
				trkofs += p_tmp_buffer[0x32] + p_tmp_buffer[0x33] * 256;
			}
		}
	}
	d88_hdr.size = trkptr;
	memcpy(p_buffer, &d88_hdr, sizeof(d88_hdr_t));
	p_disk->set_file_size(file_size);
	p_disk->set_image_type(DISK::IMAGE_TYPE_D88_CONVERTED);
	return true;
}

/// @brief standard (plain) image decoder (fdi/tfd/2d/sf7)
/// @param[in] param : disk parameter
/// @return true / false
bool DISK_PARSER::standard_to_d88(const fd_format_t *param)
{
	d88_hdr_t d88_hdr;
	d88_sct_t d88_sct;
	int n = 0, sn = 0, t = 0;

	int file_size = 0;
	p_disk->set_file_size(0);

	// create d88 header
	memset(&d88_hdr, 0, sizeof(d88_hdr_t));
	strcpy(d88_hdr.title, "STANDARD");
	d88_hdr.protect = 0; // non-protected
	d88_hdr.media_type = param->media_type;
	COPYBUFFER(p_buffer, m_buffer_size, file_size, &d88_hdr, sizeof(d88_hdr_t));

	// sector length
	for(int i = 0; i < 8; i++) {
		if(param->size == (uint32_t)(128 << i)) {
			n = i;
			break;
		}
	}
	if (param->ssize > 0) {
		for(int i = 0; i < 8; i++) {
			if(param->ssize == (uint32_t)(128 << i)) {
				sn = i;
				break;
			}
		}
	}

	// create tracks
	int trkptr = sizeof(d88_hdr_t);
	for(uint32_t c = 0; c < param->ncyl; c++) {
		for(uint32_t h = 0; h < param->nside; h++) {
			// set pointer to a track data
			d88_hdr.trkptr[t++] = trkptr;
			// read sectors in this track
			for(uint32_t s = 0; s < param->nsec; s++) {
				// create d88 sector header
				memset(&d88_sct, 0, sizeof(d88_sct_t));
				d88_sct.c = c;
				d88_sct.h = h;
				d88_sct.r = s + 1;
				d88_sct.nsec = param->nsec;
				d88_sct.del = 0;
				d88_sct.stat = 0;
				if (c == 0 && h == 0 && param->ssize > 0) {
					// single density
					d88_sct.n = sn;
					d88_sct.dens = 0x40;
					d88_sct.size = param->ssize;
				} else {
					// standard
					d88_sct.n = n;
					d88_sct.dens = param->dens;
					d88_sct.size = param->size;
				}

				// create sector image
				uint8_t dst[16384];
				fio->Fread(dst, d88_sct.size, 1);

				// copy to d88
				COPYBUFFER(p_buffer, m_buffer_size, file_size, &d88_sct, sizeof(d88_sct_t));
				COPYBUFFER(p_buffer, m_buffer_size, file_size, dst, (size_t)d88_sct.size);
				trkptr += sizeof(d88_sct_t) + d88_sct.size;
			}
		}
	}
	d88_hdr.size = trkptr;
	memcpy(p_buffer, &d88_hdr, sizeof(d88_hdr_t));
	p_disk->set_file_size(file_size);
	return true;
}

/// @brief standard (plain) image parser (fdi/tfd/2d/sf7)
bool DISK_PARSER::parse_standard()
{
	bool converted = false;
	int file_size = p_disk->get_file_size();

	for(int i = 0;; i++) {
		const fd_format_t *param = &c_fd_formats[i];
		if(param->media_type == -1) {
			break;
		}
		int len = param->ncyl * param->nside * param->nsec * param->size;
		if (param->ssize > 0) {
			// single density on track 0 and side 0
			len += param->nsec * param->ssize;	// single density size
			len -= param->nsec * param->size;
		}

		if (file_size == (len + 4096)) {
			// 4096 bytes: FDI header ???
			struct fdi_hdr_t fdi_hdr;
			fio->Fseek(0, FILEIO::SEEKSET);
			fio->Fread(&fdi_hdr, sizeof(struct fdi_hdr_t), 1);
			if (param->ncyl == fdi_hdr.n_cyls && param->nside == fdi_hdr.n_heads && param->nsec == fdi_hdr.n_secs) {
				fio->Fseek(file_size - len, FILEIO::SEEKSET);
				if(standard_to_d88(param)) {
					converted = true;
					p_disk->set_image_type(DISK::IMAGE_TYPE_D88_CONVERTED);
					break;
				}
			}
		} else if(file_size == len) {
			// beta
			fio->Fseek(file_size - len, FILEIO::SEEKSET);
			if(standard_to_d88(param)) {
				converted = true;
				p_disk->set_image_type(DISK::IMAGE_TYPE_PLAIN);
				break;
			}
		}
	}
//	if (converted) {
//		UTILITY::stprintf(n_file_path, _MAX_PATH, _T("%s.D88"), p_file_path);
//	}
	return converted;
}

/// @brief parse a floppy disk image
/// @param[in] offset       : offset for one disk in d88 multi volume image
/// @param[out] file_size_orig : original file size on disk image
/// @param[out] file_offset    : offset for one disk
bool DISK_PARSER::parse(int offset, int &file_size_orig, int &file_offset)
{
	bool rc = false;
	int file_size = p_disk->get_file_size();
	p_disk->set_image_type(DISK::IMAGE_TYPE_UNK);

	// is this d88 format ?
	if(UTILITY::check_file_extensions(p_file_path, _T(".d88"), _T(".d77"), NULL)) {
		if (file_size < 0x20) {
			// too small
			return false;
		}
		fio->Fseek(offset + 0x1c, FILEIO::SEEKSET);
		file_size = fio->FgetUint32_LE();
		if (file_size > 0) {
			file_size_orig = file_size;
			fio->Fseek(offset, FILEIO::SEEKSET);
			fio->Fread(p_buffer, file_size, 1);
			file_offset = offset;

			p_disk->set_file_size(file_size);
			p_disk->set_image_type(DISK::IMAGE_TYPE_D88);
			rc = true;
		}
		return rc;
	}

	if(file_size <= 0 || (int)m_buffer_size < file_size) {
		return false;
	}

	// parse standard image
	if (parse_standard()) {
		return true;
	}

	memset(p_buffer, 0, m_buffer_size);
	fio->Fseek(0, FILEIO::SEEKSET);
	fio->Fread(p_buffer, file_size, 1);

	// check d88 format (temporary)
	uint32_t *buffer_file_size = (uint32_t *)(&p_buffer[0x1c]);
	if(Uint32_LE(*buffer_file_size) == (uint32_t)file_size) {
		return true;
	}
//	UTILITY::stprintf(n_file_path, _MAX_PATH, _T("%s.D88"), p_file_path);

	// check file header
	try {
		if(memcmp(p_buffer, "TD", 2) == 0 || memcmp(p_buffer, "td", 2) == 0) {
			// teledisk image file
			rc = teledisk_to_d88();
		}
		else if(memcmp(p_buffer, "IMD", 3) == 0) {
			// imagedisk image file
			rc = imagedisk_to_d88();
		}
		else if(memcmp(p_buffer, "MV - CPC", 8) == 0) {
			// standard cpdread image file
			rc = cpdread_to_d88(0);
		}
		else if(memcmp(p_buffer, "EXTENDED", 8) == 0) {
			// extended cpdread image file
			rc = cpdread_to_d88(1);
		}
	}
	catch(...) {
		// failed to convert the disk image
	}

	return rc;
}
