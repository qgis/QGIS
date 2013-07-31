/***************************************************************************
                          qgsrastercalclexer.ll
          Rules for lexical analysis of raster calc strings done by Flex
                          --------------------
    begin                : 2010-10-01
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
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
  #include "qgsrastercalcnode.h"
  #include "qgsrastercalcparser.hpp"

  // if not defined, searches for isatty()
  // which doesn't in MSVC compiler
  #define YY_NEVER_INTERACTIVE 1

  #ifdef _MSC_VER
  #define YY_NO_UNISTD_H
  #endif
%}

white       [ \t\r\n]+

dig     [0-9]
num1    {dig}+\.?([eE][-+]?{dig}+)?
num2    {dig}*\.{dig}+([eE][-+]?{dig}+)?
number  {num1}|{num2}

non_ascii    [\x80-\xFF]
raster_ref_char  [A-Za-z0-9_./:]|{non_ascii}|[-]
raster_band_ref ({raster_ref_char}+)@{dig}
raster_band_ref_quoted  \"(\\.|[^"])*\"

%%

"sqrt" { rasterlval.op = QgsRasterCalcNode::opSQRT; return FUNCTION;}
"sin"  { rasterlval.op = QgsRasterCalcNode::opSIN; return FUNCTION;}
"cos"  { rasterlval.op = QgsRasterCalcNode::opCOS; return FUNCTION;}
"tan"  { rasterlval.op = QgsRasterCalcNode::opTAN; return FUNCTION;}
"asin" { rasterlval.op = QgsRasterCalcNode::opASIN; return FUNCTION;}
"acos" { rasterlval.op = QgsRasterCalcNode::opACOS; return FUNCTION;}
"atan" { rasterlval.op = QgsRasterCalcNode::opATAN; return FUNCTION;}

"AND" { return AND; }
"OR" { return OR; }
"!=" { return NE; }
"<=" { return LE; }
">=" { return GE; }

[=><+-/*^] { return yytext[0]; }


[()] { return yytext[0]; }

{number} { rasterlval.number  = atof(rastertext); return NUMBER; }

{raster_band_ref} { return RASTER_BAND_REF; }

{raster_band_ref_quoted} { return RASTER_BAND_REF; }

{white}    /* skip blanks and tabs */
%%

void set_raster_input_buffer(const char* buffer)
{
  raster_scan_string(buffer);
}
