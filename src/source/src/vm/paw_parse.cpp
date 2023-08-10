/// @file paw_parse.cpp
///
/// @author Sasaji
/// @date   2019.08.01
///


#include "paw_parse.h"
#include <stdio.h>


namespace PARSEWAV
{

ParserBase::ParserBase()
{
	param = NULL;
	infile = NULL;
}

void ParserBase::Init()
{
}

void ParserBase::SetParameter(Parameter &param_)
{
	param = &param_;
}

void ParserBase::SetInputFile(InputFile &infile_)
{
	infile = &infile_;
}

}; /* namespace PARSEWAV */
