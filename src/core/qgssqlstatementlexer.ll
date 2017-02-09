/***************************************************************************
                          qgssqlstatementlexer.ll
                          --------------------
    begin                : April 2016
    copyright            : (C) 2011 by Martin Dobias
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
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
%option prefix="sqlstatement_"
 // this makes flex generate lexer with context + init/destroy functions
%option reentrant
 // this makes Bison send yylex another argument to use instead of using the global variable yylval
%option bison-bridge

 // ensure that lexer will be 8-bit (and not just 7-bit)
%option 8bit

%{

#include "qgssqlstatement.h"
struct sqlstatement_parser_context;
#include "qgssqlstatementparser.hpp"
#include <QLocale>

// if not defined, searches for isatty()
// which doesn't in MSVC compiler
#define YY_NEVER_INTERACTIVE 1

#ifndef YY_NO_UNPUT
#define YY_NO_UNPUT     // unused
#endif

#ifdef _MSC_VER
#define YY_NO_UNISTD_H
#endif

#define B_OP(x) yylval->b_op = QgsSQLStatement::x
#define U_OP(x) yylval->u_op = QgsSQLStatement::x
#define TEXT                   yylval->text = new QString( QString::fromUtf8(yytext) );
#define TEXT_FILTER(filter_fn) yylval->text = new QString( filter_fn( QString::fromUtf8(yytext) ) );

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

// C locale for correct parsing of numbers even if the system locale is different
static QLocale cLocale("C");

%}

white       [ \t\r\n]+

non_ascii    [\x80-\xFF]

identifier_first    [A-Za-z_]|{non_ascii}
identifier_next     [A-Za-z0-9_]|{non_ascii}
identifier  {identifier_first}{identifier_next}*

identifier_str_char  "\"\""|[^\"]
identifier_quoted  "\""{identifier_str_char}*"\""

dig         [0-9]
num_int     [-]?{dig}+{identifier_first}*
num_float   [-]?{dig}*(\.{dig}+([eE][-+]?{dig}+)?|[eE][-+]?{dig}+)
boolean     "TRUE"|"FALSE"

str_char    ('')|(\\.)|[^'\\]
string      "'"{str_char}*"'"

%%

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

"LIKE"              { B_OP(boLike); return LIKE; }
"NOT"{white}"LIKE"  { B_OP(boNotLike); return LIKE; }
"ILIKE"             { B_OP(boILike); return LIKE; }
"NOT"{white}"ILIKE" { B_OP(boNotILike); return LIKE; }
"IS"                { B_OP(boIs); return IS; }
"IS"{white}"NOT"    { B_OP(boIsNot); return IS; }
"||"                { B_OP(boConcat); return CONCAT; }

"+"                 { B_OP(boPlus); return PLUS; }
"-"                 { B_OP(boMinus); return MINUS; }
"*"                 { B_OP(boMul); return MUL_OR_STAR; }
"//"                { B_OP(boIntDiv); return INTDIV; }
"/"                 { B_OP(boDiv); return DIV; }
"%"                 { B_OP(boMod); return MOD; }
"^"                 { B_OP(boPow); return POW; }

"IN"                { return IN; }
"BETWEEN"           { return BETWEEN; }

"NULL"              { return NULLVALUE; }

"SELECT"            { return SELECT; }
"ALL"               { return ALL; }
"DISTINCT"          { return DISTINCT; }
"CAST"              { return CAST; }
"AS"                { return AS; }
"FROM"              { return FROM; }
"JOIN"              { return JOIN; }
"ON"                { return ON; }
"USING"             { return USING; }
"WHERE"             { return WHERE; }
"ORDER"             { return ORDER; }
"BY"                { return BY; }
"ASC"               { return ASC; }
"DESC"              { return DESC; }
"LEFT"              { return LEFT; }
"RIGHT"             { return RIGHT; }
"INNER"             { return INNER; }
"OUTER"             { return OUTER; }
"CROSS"             { return CROSS; }
"FULL"              { return FULL; }
"NATURAL"           { return NATURAL; }
"UNION"             { return UNION; }

[().]                { return yytext[0]; }

","                 { return COMMA; }

{num_float}  { yylval->numberFloat = cLocale.toDouble( QString::fromAscii(yytext) ); return NUMBER_FLOAT; }
{num_int}  {
        bool ok;
        yylval->numberInt = cLocale.toInt( QString::fromAscii(yytext), &ok );
        if( ok )
                return NUMBER_INT;

        yylval->numberInt64 = cLocale.toLongLong( QString::fromAscii(yytext), &ok );
        if( ok )
                return NUMBER_INT64;

        yylval->numberFloat = cLocale.toDouble( QString::fromAscii(yytext), &ok );
        if( ok )
                return NUMBER_FLOAT;

        return Unknown_CHARACTER;
}

{boolean} { yylval->boolVal = QString( yytext ).compare( "true", Qt::CaseInsensitive ) == 0; return BOOLEAN; }

{string}  { TEXT_FILTER(stripText); return STRING; }

{identifier}         { TEXT; return IDENTIFIER; }

{identifier_quoted}  { TEXT_FILTER(QgsSQLStatement::stripQuotedIdentifier); return IDENTIFIER; }

{white}    /* skip blanks and tabs */

.       { return Unknown_CHARACTER; }


%%
