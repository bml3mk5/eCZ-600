/// @file paw_parse.h
///
/// @author Sasaji
/// @date   2019.08.01
///


#ifndef _PARSEWAV_PARSE_H_
#define _PARSEWAV_PARSE_H_

#include "../common.h"
#include "paw_defs.h"
#include "paw_param.h"
#include "paw_file.h"


namespace PARSEWAV 
{

/// データ解析用基底クラス
class ParserBase
{
protected:
	/// ダイアログパラメータ
	Parameter *param;
	/// 入力ファイル
	InputFile *infile;

public:
	ParserBase();
	void Init();

	void SetParameter(Parameter &param_);
	void SetInputFile(InputFile &infile_);

};

}; /* namespace PARSEWAV */

#endif /* _PARSEWAV_PARSE_H_ */
