/***************************************************************************
                          qgsexpressionlexer.ll
                          --------------------
    begin                : August 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

%option noyywrap
%option case-insensitive
%option never-interactive
%option nounput
%option prefix="exp_"
 // this makes flex generate lexer with context + init/destroy functions
%option reentrant
%option yylineno
 // this makes Bison send yylex another argument to use instead of using the global variable yylval
%option bison-bridge
%option bison-locations


 // ensure that lexer will be 8-bit (and not just 7-bit)
%option 8bit

%{

#include <stdlib.h>  // atof()

#include "qgsexpression.h"
#include "expression/qgsexpressionnodeimpl.h"
#include "qgsexpressionfunction.h"
        struct expression_parser_context;
#include "qgsexpressionparser.hpp"
#include <QLocale>

// if not defined, searches for isatty()
// which doesn't in MSVC compiler
#define YY_NEVER_INTERACTIVE 1

#ifndef YY_NO_UNPUT
#define YY_NO_UNPUT	// unused
#endif

#ifdef _MSC_VER
#define YY_NO_UNISTD_H
#endif

#define B_OP(x) yylval->b_op = QgsExpressionNodeBinaryOperator::x
#define U_OP(x) yylval->u_op = QgsExpressionNodeUnaryOperator::x
#define TEXT                   yylval->text = new QString( QString::fromUtf8(yytext) );
#define TEXT_FILTER(filter_fn) yylval->text = new QString( filter_fn( QString::fromUtf8(yytext) ) );

#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
    if(yytext[i] == '\n') { \
        yylloc->last_line++; \
        yylloc->last_column = 0; \
    } \
    else { \
        yylloc->last_column++; \
    } \
}

static QString stripText(QString text)
{
  // strip single quotes on start,end
  text = text.mid( 1, text.length() - 2 );

  // make single "single quotes" from double "single quotes"
  text.replace( "''", "'" );

  // strip \n \' etc.
  int index = 0;
  while (( index = text.indexOf( '\\', index ) ) != -1 )
  {
    text.remove( index, 1 ); // delete backslash
    QChar chr;
    switch ( text[index].toLatin1() ) // evaluate backslashed character
    {
      case 'n':  chr = '\n'; break;
      case 't':  chr = '\t'; break;
      case '\\': chr = '\\'; break;
      case '\'': chr = '\''; break;
      default: chr = '?'; break;
    }
    text[index++] = chr; // set new character and push index +1
  }
  return text;
}

static QString stripNamedText(QString text)
{
  text.remove(":=");
  return text.trimmed();
}

static QString stripColumnRef(QString text)
{
  // strip double quotes on start,end
  text = text.mid( 1, text.length() - 2 );

  // make single "double quotes" from double "double quotes"
  text.replace( "\"\"", "\"" );
  return text;
}

// C locale for correct parsing of numbers even if the system locale is different
Q_GLOBAL_STATIC_WITH_ARGS(QLocale, cLocale, ("C") )

%}

%s BLOCK_COMMENT

line_comment \-\-[^\r\n]*[\r\n]?

white       [ \t\r\n]+

non_ascii    [\x80-\xFF]

col_first    [A-Za-z_]|{non_ascii}
col_next     [A-Za-z0-9_]|{non_ascii}
identifier  {col_first}{col_next}*

deprecated_function "$"[xXyY]_?[aA][tT]
special_col "$"{identifier}
variable "@"{identifier}

named_node {identifier}{white}*":="{white}*

col_str_char  "\"\""|[^\"]
identifier_quoted  "\""{col_str_char}*"\""

dig         [0-9]
num_int     {dig}+
num_float   {dig}*(\.{dig}+([eE][-+]?{dig}+)?|[eE][-+]?{dig}+)
boolean     "TRUE"|"FALSE"

str_char    ('')|(\\.)|[^'\\]
string      "'"{str_char}*"'"

%%

<INITIAL>{
  "/*" BEGIN(BLOCK_COMMENT);
}
<BLOCK_COMMENT>{
  "*/" BEGIN(INITIAL);
  [^*\n]+   // eat comment in chunks
  "*"       // eat the lone star
  \n        yylineno++;
}

"NOT"               { U_OP(uoNot); return NOT; }
"AND"               { B_OP(boAnd); return AND; }
"OR"                { B_OP(boOr);  return OR;  }

"="                 { B_OP(boEQ); return EQ; }
"!="                { B_OP(boNE); return NE; }
"<="                { B_OP(boLE); return LE; }
">="                { B_OP(boGE); return GE; }
"<>"                { B_OP(boNE); return NE; }
"<"                 { B_OP(boLT); return LT; }
">"                 { B_OP(boGT); return GT; }

"~"                 { B_OP(boRegexp); return REGEXP; }
"LIKE"              { B_OP(boLike); return LIKE; }
"NOT"{white}"LIKE"  { B_OP(boNotLike); return LIKE; }
"ILIKE"             { B_OP(boILike); return LIKE; }
"NOT"{white}"ILIKE" { B_OP(boNotILike); return LIKE; }
"IS"                { B_OP(boIs); return IS; }
"IS"{white}"NOT"    { B_OP(boIsNot); return IS; }
"||"                { B_OP(boConcat); return CONCAT; }

"+"                 { B_OP(boPlus); return PLUS; }
"-"                 { B_OP(boMinus); return MINUS; }
"*"                 { B_OP(boMul); return MUL; }
"//"                { B_OP(boIntDiv); return INTDIV; }
"/"                 { B_OP(boDiv); return DIV; }
"%"                 { B_OP(boMod); return MOD; }
"^"                 { B_OP(boPow); return POW; }

"IN"                { return IN; }
"BETWEEN"           { return BETWEEN; }

"NULL"              { return NULLVALUE; }

"CASE"              { return CASE; }
"WHEN"              { return WHEN; }
"THEN"              { return THEN; }
"ELSE"              { return ELSE; }
"END"               { return END;  }

[()\[\]]            { return yytext[0]; }

","                 { return COMMA; }

{num_float}  { yylval->numberFloat = cLocale()->toDouble( QString::fromLatin1(yytext) ); return NUMBER_FLOAT; }
{num_int}  {
	bool ok;
	yylval->numberInt = cLocale()->toInt( QString::fromLatin1(yytext), &ok );
	if( ok )
		return NUMBER_INT;

  yylval->numberInt64 = cLocale()->toLongLong( QString::fromLatin1(yytext), &ok );
  if( ok )
    return NUMBER_INT64;

	yylval->numberFloat = cLocale()->toDouble( QString::fromLatin1(yytext), &ok );
	if( ok )
		return NUMBER_FLOAT;

	return Unknown_CHARACTER;
}

{boolean} { yylval->boolVal = QString( yytext ).compare( "true", Qt::CaseInsensitive ) == 0; return BOOLEAN; }

{string}  { TEXT_FILTER(stripText); return STRING; }

{deprecated_function} { TEXT; return NAME; }

{named_node} { TEXT_FILTER(stripNamedText); return NAMED_NODE; }

{special_col}        { TEXT; return SPECIAL_COL; }

{variable}        { TEXT; return VARIABLE; }

{identifier}         { TEXT; return NAME; }

{identifier_quoted}  { TEXT_FILTER(stripColumnRef); return QUOTED_COLUMN_REF; }

{white}    /* skip blanks and tabs */

{line_comment} /* skip line comments */

.       { return Unknown_CHARACTER; }


%%
