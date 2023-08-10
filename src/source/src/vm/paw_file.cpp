/// @file paw_file.cpp
///
/// @author Sasaji
/// @date   2019.08.01
///

#include "paw_file.h"
//#include "utils.h"


namespace PARSEWAV 
{

//

SamplePosition::SamplePosition()
{
	ClearSample();
}
void SamplePosition::ClearSample()
{
	m_sample_pos = 0;
	m_sample_num = 0;
//	m_sample_usec = 0;
}
int SamplePosition::AddSamplePos(int offset)
{
	m_sample_pos += offset;
	return m_sample_pos;
}
int SamplePosition::IncreaseSamplePos()
{
	m_sample_pos++;
	return m_sample_pos;
}
int SamplePosition::DecreaseSamplePos()
{
	m_sample_pos--;
	return m_sample_pos;
}

/// 現在位置の時間を計算
uint32_t SamplePosition::CalcrateSampleUSec()
{
	return CalcrateSampleUSec(m_sample_pos, m_sample_rate);
}

/// 現在位置の時間を計算
uint32_t SamplePosition::CalcrateSampleUSec(int pos)
{
	return CalcrateSampleUSec(pos, m_sample_rate);
}

/// 現在位置の時間を計算
uint32_t SamplePosition::CalcrateSampleUSec(int pos, double rate)
{
	return (uint32_t)(1000000.0 * (double)pos / rate);
}

/// 時間から位置を計算
double SamplePosition::CalcrateSamplePos(uint32_t usec)
{
	return CalcrateSamplePos(usec, m_sample_rate);
}

/// 時間から位置を計算
double SamplePosition::CalcrateSamplePos(uint32_t usec, double rate)
{
	return (double)usec * rate / 1000000.0;
}

/// ストックしたサンプル数を戻す
void SamplePosition::RestoreSampleNum(int offset) 
{
	m_sample_num = m_sample_num_stocked + offset;
}

bool SamplePosition::IsFirstPos(int offset) const
{
	return (m_sample_pos < offset);
}
bool SamplePosition::IsEndPos(int offset) const
{
	return ((m_sample_pos + offset) >= m_sample_num);
}

void SamplePosition::SamplePos(int val)
{
	m_sample_pos = val;
}
void SamplePosition::SampleNum(int val)
{
	m_sample_num = val;
	m_sample_num_stocked = val;
}
void SamplePosition::SampleRate(double rate)
{
	m_sample_rate = rate;
}

//

InputFile::InputFile()
	: FILEIO(), SamplePosition()
{
}
/// ファイル先頭にセット
void InputFile::First()
{
	Fseek(0, FILEIO::SEEKSET);
	SamplePos(0);
}
void InputFile::Attach(FILEIO &file)
{
	SetFile(file.GetFile());
	ClearSample();
}
void InputFile::Detach()
{
	SetFile(NULL);
}

OutputFile::OutputFile()
	: FILEIO()
{
}

int OutputFile::WriteData(CSampleArray &data)
{
	int len = 0;
	for(int i=data.GetStartPos(); i<data.GetWritePos(); i++) {
		Fputc(data.At(i).Data());
		len++;
	}
	return len;
}

//

LogFile::LogFile()
	: FILEIO()
{
}
bool LogFile::Open(const _TCHAR *file_name)
{
	return Fopen(file_name, FILEIO::WRITE_ASCII);
}
void LogFile::Close()
{
	Fclose();
}

/// ログファイル実体
LogFile gLogFile;

}; /* namespace PARSEWAV */
