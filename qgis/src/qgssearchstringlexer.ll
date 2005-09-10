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

%{
  
#include <stdlib.h>  // atof()
  
#include "qgssearchtreenode.h"
#include "qgssearchstringparser.h"

%}

white       [ \t\n]+

col_first    [A-Za-z_]
col_next     [A-Za-z0-9_]
column_ref  {col_first}{col_next}*

dig     [0-9]
num1    {dig}+\.?([eE][-+]?{dig}+)?
num2    {dig}*\.{dig}+([eE][-+]?{dig}+)?
number  {num1}|{num2}

str_char    ('')|(\\.)|[^'\\]
string      "'"{str_char}*"'"


comparison    (">="|"<="|"<"|">"|"="|"!="|"<>"|"~"|"LIKE")

arithmetic    [+-/*]

%%

{white}    /* skip blanks and tabs */

[()]      { return yytext[0]; }

{number}  { yylval.number  = atof(yytext); return NUMBER; }

"NOT "    { return NOT; }
" AND "   { return AND;  }
" OR "    { return OR; }

"="   {  yylval.op = QgsSearchTreeNode::opEQ; return COMPARISON; }
"!="  {  yylval.op = QgsSearchTreeNode::opNE; return COMPARISON; }
"<="  {  yylval.op = QgsSearchTreeNode::opLE; return COMPARISON; }
">="  {  yylval.op = QgsSearchTreeNode::opGE; return COMPARISON; }
"<>"  {  yylval.op = QgsSearchTreeNode::opNE; return COMPARISON; }
"<"   {  yylval.op = QgsSearchTreeNode::opLT; return COMPARISON; }
">"   {  yylval.op = QgsSearchTreeNode::opGT; return COMPARISON; }
"~"   {  yylval.op = QgsSearchTreeNode::opRegexp; return COMPARISON; }
"LIKE" { yylval.op = QgsSearchTreeNode::opLike; return COMPARISON; }

{arithmetic}    { return yytext[0]; }

{string}  { return STRING; }

{column_ref}   { return COLUMN_REF; }

.       { return UNKNOWN_CHARACTER; }

%%

void set_input_buffer(const char* buffer)
{
  yy_scan_string(buffer);
}

