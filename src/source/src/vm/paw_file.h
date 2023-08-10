/// @file paw_file.h
///
/// @author Sasaji
/// @date   2019.08.01
///


#ifndef _PARSEWAV_FILE_H_
#define _PARSEWAV_FILE_H_

#include "../common.h"
#include "../fileio.h"
//#include "paw_defs.h"
#include "paw_datas.h"


namespace PARSEWAV 
{

/// サンプル位置保持用
class SamplePosition
{
protected:
	int      m_sample_pos;
	int      m_sample_num;

	int      m_sample_num_stocked;

	double   m_sample_rate;	///< サンプルレート

public:
	SamplePosition();
	void ClearSample();

	int      AddSamplePos(int offset);
	int      IncreaseSamplePos();
	int      DecreaseSamplePos();

	uint32_t CalcrateSampleUSec();
	uint32_t CalcrateSampleUSec(int pos);
	static uint32_t CalcrateSampleUSec(int pos, double rate);
	double CalcrateSamplePos(uint32_t usec);
	static double CalcrateSamplePos(uint32_t usec, double rate);

	void RestoreSampleNum(int offset = 0);

	bool     IsFirstPos(int offset = 0) const;
	bool     IsEndPos(int offset = 0) const;

	int      SamplePos() const { return m_sample_pos; }
	int      SampleNum() const { return m_sample_num; }
	double   SampleRate() const { return m_sample_rate; }
	void SamplePos(int val);
	void SampleNum(int val);
	void SampleRate(double rate);
};

/// 入力ファイルクラス
class InputFile : public FILEIO, public SamplePosition
{
public:
	InputFile();

	void First();
	void Attach(FILEIO &file);
	void Detach();
};

/// 出力ファイルクラス
class OutputFile : public FILEIO
{
public:
	OutputFile();

	int WriteData(CSampleArray &data);
};

/// ログファイルクラス
class LogFile : public FILEIO
{
private:

public:
	LogFile();

	bool Open(const _TCHAR *file_name);
	void Close();
};

/// ログファイル
extern LogFile gLogFile;

}; /* namespace PARSEWAV */

#endif /* _PARSEWAV_FILE_H_ */
