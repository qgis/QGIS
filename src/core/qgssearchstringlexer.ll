/***************************************************************************
                          qgssearchstringparser.ll
          Rules for lexical analysis of search strings done by Flex
                          --------------------
    begin                : 2005-07-26
    copyright            : (C) 2005 by Martin Dobias
    email                : won.der at centrum.sk
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */
 
%option noyywrap
%option case-insensitive
%option never-interactive

 // ensure that lexer will be 8-bit (and not just 7-bit)
%option 8bit

%{
  
#include <stdlib.h>  // atof()
  
#include "qgssearchtreenode.h"
#include "qgssearchstringparser.hpp"

// if not defined, searches for isatty()
// which doesn't in MSVC compiler
#define YY_NEVER_INTERACTIVE 1

#ifdef _MSC_VER
#define YY_NO_UNISTD_H
#endif

%}

white       [ \t\r\n]+

non_ascii    [\x80-\xFF]

col_first    [A-Za-z_]|{non_ascii}
col_next     [A-Za-z0-9_]|{non_ascii}
column_ref  {col_first}{col_next}*

col_str_char  "\"\""|[^\"]
column_ref_quoted  "\""{col_str_char}*"\""

dig     [0-9]
num1    {dig}+\.?([eE][-+]?{dig}+)?
num2    {dig}*\.{dig}+([eE][-+]?{dig}+)?
number  {num1}|{num2}

str_char    ('')|(\\.)|[^'\\]
string      "'"{str_char}*"'"


%%

"NOT"    { return NOT; }
"AND"   { return AND;  }
"OR"    { return OR; }

"NULL"	{ return NULLVALUE; }

"IS"  {  return IS; }
"IN"  {  return IN; }

"="   {  yylval.op = QgsSearchTreeNode::opEQ; return COMPARISON; }
"!="  {  yylval.op = QgsSearchTreeNode::opNE; return COMPARISON; }
"<="  {  yylval.op = QgsSearchTreeNode::opLE; return COMPARISON; }
">="  {  yylval.op = QgsSearchTreeNode::opGE; return COMPARISON; }
"<>"  {  yylval.op = QgsSearchTreeNode::opNE; return COMPARISON; }
"<"   {  yylval.op = QgsSearchTreeNode::opLT; return COMPARISON; }
">"   {  yylval.op = QgsSearchTreeNode::opGT; return COMPARISON; }
"~"   {  yylval.op = QgsSearchTreeNode::opRegexp; return COMPARISON; }
"LIKE" { yylval.op = QgsSearchTreeNode::opLike; return COMPARISON; }
"ILIKE" { yylval.op = QgsSearchTreeNode::opILike; return COMPARISON; }

"sqrt" { yylval.op = QgsSearchTreeNode::opSQRT; return FUNCTION1;}
"sin"  { yylval.op = QgsSearchTreeNode::opSIN; return FUNCTION1;}
"cos"  { yylval.op = QgsSearchTreeNode::opCOS; return FUNCTION1;}
"tan"  { yylval.op = QgsSearchTreeNode::opTAN; return FUNCTION1;}
"asin" { yylval.op = QgsSearchTreeNode::opASIN; return FUNCTION1;}
"acos" { yylval.op = QgsSearchTreeNode::opACOS; return FUNCTION1;}
"atan" { yylval.op = QgsSearchTreeNode::opATAN; return FUNCTION1;}
"to int" { yylval.op = QgsSearchTreeNode::opTOINT; return FUNCTION1;}
"to real" { yylval.op = QgsSearchTreeNode::opTOREAL; return FUNCTION1;}
"to string" { yylval.op = QgsSearchTreeNode::opTOSTRING; return FUNCTION1;}
"lower" { yylval.op = QgsSearchTreeNode::opLOWER; return FUNCTION1;}
"upper" { yylval.op = QgsSearchTreeNode::opUPPER; return FUNCTION1;}
"length" { yylval.op = QgsSearchTreeNode::opSTRLEN; return FUNCTION1;}

"atan2" { yylval.op = QgsSearchTreeNode::opATAN2; return FUNCTION2;}

"replace" { yylval.op = QgsSearchTreeNode::opREPLACE; return FUNCTION3;}
"substr" { yylval.op = QgsSearchTreeNode::opSUBSTR; return FUNCTION3;}

"||"   { return CONCAT; }

[+-/*^]    { return yytext[0]; }

[()]      { return yytext[0]; }

{number}  { yylval.number  = atof(yytext); return NUMBER; }

{string}  { return STRING; }

"$rownum" { return ROWNUM; }

"$area" { return AREA; }
"$length" { return LENGTH; }
"$id" { return ID; }

{column_ref}   { return COLUMN_REF; }
{column_ref_quoted}  { return COLUMN_REF; }

{white}    /* skip blanks and tabs */

.       { return Unknown_CHARACTER; }

%%

void set_input_buffer(const char* buffer)
{
  yy_scan_string(buffer);
}

