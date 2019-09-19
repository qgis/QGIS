/***************************************************************************
                          qgsmeshcalclexer.ll
                          -------------------
    begin                : December 19th, 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

%option noyywrap
%option nounput
%option case-insensitive
%option never-interactive

 // ensure that lexer will be 8-bit (and not just 7-bit)
%option 8bit

%{
  //directly included in the output program
  #include "qgsmeshcalcnode.h"
  #include "qgsmeshcalcparser.hpp"

  // if not defined, searches for isatty()
  // which doesn't in MSVC compiler
  #define YY_NEVER_INTERACTIVE 1

  #ifdef _MSC_VER
  #define YY_NO_UNISTD_H
  #endif
  
  #ifndef _MSC_VER
  #pragma GCC diagnostic ignored "-Wsign-compare"
  #endif
%}

white       [ \t\r\n]+

dig     [0-9]
num1    {dig}+\.?([eE][-+]?{dig}+)?
num2    {dig}*\.{dig}+([eE][-+]?{dig}+)?
number  {num1}|{num2}

non_ascii    [\x80-\xFF]
dataset_ref_char  [A-Za-z0-9_./:]|{non_ascii}|[-]
dataset_ref ({dataset_ref_char}+)
dataset_ref_quoted  \"(\\.|[^"])*\"

%%

"sum_aggr"      { meshlval.op = QgsMeshCalcNode::opSUM_AGGR; return FUNCTION; }
"max_aggr"      { meshlval.op = QgsMeshCalcNode::opMAX_AGGR; return FUNCTION; }
"min_aggr"      { meshlval.op = QgsMeshCalcNode::opMIN_AGGR; return FUNCTION; }
"average_aggr"  { meshlval.op = QgsMeshCalcNode::opAVG_AGGR; return FUNCTION; }
"abs"           { meshlval.op = QgsMeshCalcNode::opABS; return FUNCTION; }

"max"           { meshlval.op = QgsMeshCalcNode::opMAX; return FUNCTION2; }
"min"           { meshlval.op = QgsMeshCalcNode::opMIN; return FUNCTION2; }

"IF"     { return IF; }
"AND"    { return AND; }
"OR"     { return OR; }
"NOT"    { return NOT; }
"!="     { return NE; }
"<="     { return LE; }
">="     { return GE; }
"NODATA" {return NODATA;}

[=><+-/*^] { return yytext[0]; }

[()] { return yytext[0]; }

{number} { meshlval.number  = atof(meshtext); return NUMBER; }

{dataset_ref} { return DATASET_REF; }

{dataset_ref_quoted} { return DATASET_REF; }

{white}    /* skip blanks and tabs */
%%

void set_mesh_input_buffer(const char* buffer)
{
  mesh_scan_string(buffer);
}
