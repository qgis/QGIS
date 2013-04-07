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

 // ensure that lexer will be 8-bit (and not just 7-bit)
%option 8bit

%{
  
#include <stdlib.h>  // atof()

#include "qgsexpression.h"
#include "qgsexpressionparser.hpp"
#include <QRegExp>
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

#define B_OP(x) exp_lval.b_op = QgsExpression::x
#define U_OP(x) exp_lval.u_op = QgsExpression::x
#define TEXT                   exp_lval.text = new QString(); *exp_lval.text = QString::fromUtf8(yytext);
#define TEXT_FILTER(filter_fn) exp_lval.text = new QString(); *exp_lval.text = filter_fn( QString::fromUtf8(yytext) );

static QString stripText(QString text)
{
  // strip single quotes on start,end
  text = text.mid( 1, text.length() - 2 );

  // make single "single quotes" from double "single quotes"
  text.replace( QRegExp( "''" ), "'" );

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

static QString stripColumnRef(QString text)
{
  // strip double quotes on start,end
  text = text.mid( 1, text.length() - 2 );

  // make single "double quotes" from double "double quotes"
  text.replace( QRegExp( "\"\"" ), "\"" );
  return text;
}

// C locale for correct parsing of numbers even if the system locale is different
static QLocale cLocale("C");

%}

white       [ \t\r\n]+

non_ascii    [\x80-\xFF]

col_first    [A-Za-z_]|{non_ascii}
col_next     [A-Za-z0-9_]|{non_ascii}
column_ref  {col_first}{col_next}*

special_col "$"{column_ref}

col_str_char  "\"\""|[^\"]
column_ref_quoted  "\""{col_str_char}*"\""

dig         [0-9]
num_int     {dig}+
num_float   {dig}*\.{dig}+([eE][-+]?{dig}+)?

str_char    ('')|(\\.)|[^'\\]
string      "'"{str_char}*"'"

%%

"NOT"   { U_OP(uoNot); return NOT; }
"AND"   { B_OP(boAnd); return AND; }
"OR"    { B_OP(boOr);  return OR;  }

"="   {  B_OP(boEQ); return EQ; }
"!="  {  B_OP(boNE); return NE; }
"<="  {  B_OP(boLE); return LE; }
">="  {  B_OP(boGE); return GE; }
"<>"  {  B_OP(boNE); return NE; }
"<"   {  B_OP(boLT); return LT; }
">"   {  B_OP(boGT); return GT; }

"~"         { B_OP(boRegexp); return REGEXP; }
"LIKE"      { B_OP(boLike); return LIKE; }
"NOT LIKE"  { B_OP(boNotLike); return LIKE; }
"ILIKE"     { B_OP(boILike); return LIKE; }
"NOT ILIKE" { B_OP(boNotILike); return LIKE; }
"IS"        { B_OP(boIs); return IS; }
"IS NOT"    { B_OP(boIsNot); return IS; }
"||"        { B_OP(boConcat); return CONCAT; }

"+"  { B_OP(boPlus); return PLUS; }
"-"  { B_OP(boMinus); return MINUS; }
"*"  { B_OP(boMul); return MUL; }
"/"  { B_OP(boDiv); return DIV; }
"%"  { B_OP(boMod); return MOD; }
"^"  { B_OP(boPow); return POW; }

"IN"  {  return IN; }

"NULL"	{ return NULLVALUE; }

"CASE" { return CASE; }
"WHEN" { return WHEN; }
"THEN" { return THEN; }
"ELSE" { return ELSE; }
"END"  { return END;  }

[()]      { return yytext[0]; }

","   { return COMMA; }

{num_float}  { exp_lval.numberFloat  = cLocale.toDouble( QString::fromAscii(yytext) ); return NUMBER_FLOAT; }
{num_int}  {
	bool ok;
	exp_lval.numberInt = cLocale.toInt( QString::fromAscii(yytext), &ok, 10 );
	if( ok )
		return NUMBER_INT;

	exp_lval.numberFloat  = cLocale.toDouble( QString::fromAscii(yytext), &ok );
	if( ok )
		return NUMBER_FLOAT;

	return Unknown_CHARACTER;
}

{string}  { TEXT_FILTER(stripText); return STRING; }

{special_col}        { TEXT; return SPECIAL_COL; }

{column_ref}         { TEXT; return QgsExpression::isFunctionName(*exp_lval.text) ? FUNCTION : COLUMN_REF; }

{column_ref_quoted}  { TEXT_FILTER(stripColumnRef); return COLUMN_REF; }

{white}    /* skip blanks and tabs */

.       { return Unknown_CHARACTER; }

%%

void exp_set_input_buffer(const char* buffer)
{
  exp__scan_string(buffer);
}

