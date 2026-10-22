/***************************************************************************
    qgsesriarcadescriptlexer.ll
    ---------------------------
    begin                : October 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

%option noyywrap
%option never-interactive
%option nounput
%option prefix="arcade_"
%option reentrant
%option bison-bridge
%option 8bit

%{
#include "qgsesriarcadescript.h"

struct arcadescript_parser_context;
#include "qgsesriarcadeparser.hpp"

#include <QLocale>

#define YY_NEVER_INTERACTIVE 1

#ifndef YY_NO_UNPUT
#define YY_NO_UNPUT
#endif

#ifdef _MSC_VER
#define YY_NO_UNISTD_H
#endif

#define TEXT yylval->text = new QString( QString::fromUtf8( yytext ) );

static QString stripText( const QString &text )
{
  // strip surrounding quotes
  QString result = text.mid( 1, text.length() - 2 );

  const QChar quoteChar = text[0];
  if ( quoteChar == '\'' )
  {
    result.replace( "''"_L1, "'"_L1 );
  }

  int index = 0;
  while ( ( index = result.indexOf( '\\', index ) ) != -1 )
  {
    result.remove( index, 1 );
    if ( index >= result.length() )
      break;

    QChar character;
    switch ( result[index].toLatin1() )
    {
      case 'n':
        character = '\n';
        break;
      case 't':
        character = '\t';
        break;
      case 'r':
        character = '\r';
        break;
      case '\\':
        character = '\\';
        break;
      case '\'':
        character = '\'';
        break;
      case '\"':
        character = '\"';
        break;
      default:
        character = result[index];
        break;
    }
    result[index++] = character;
  }
  return result;
}

// standard c locale for numeric conversions
Q_GLOBAL_STATIC_WITH_ARGS( QLocale, cLocale, ( "C" ) )
%}

white          [ \t\r\n]+
dig            [0-9]
num_int        {dig}+
num_float      ({dig}+\.{dig}*|\.{dig}+)([eE][-+]?{dig}+)?|{dig}+[eE][-+]?{dig}+

identifier     [A-Za-z_][A-Za-z0-9_]*

string_double  \"([^\"\\]|\\.)*\"
string_single  \'([^\'\\]|\\.)*\'

line_comment   \/\/[^\r\n]*
block_comment  \/\*([^*]|\*+[^/*])*\*+\/

%%

{white}          /* skip whitespace */
{line_comment}   /* skip line comment */
{block_comment}  /* skip block comment */

"var"            { return VAR; }
"function"       { return FUNCTION; }
"return"         { return RETURN; }
"true"           { yylval->boolValue = true; return BOOLEAN_LITERAL; }
"false"          { yylval->boolValue = false; return BOOLEAN_LITERAL; }
"null"           { return NULL_LITERAL; }

"="              { return ASSIGN; }
"=="             { return EQ; }
"!="             { return NE; }
"<="             { return LE; }
">="             { return GE; }
"<"              { return LT; }
">"              { return GT; }
"&&"             { return LOGICAL_AND; }
"||"             { return LOGICAL_OR; }
"!"              { return NOT; }

"+"              { return PLUS; }
"-"              { return MINUS; }
"*"              { return MUL; }
"/"              { return DIV; }
"%"              { return MOD; }

";"              { return SEMICOLON; }
","              { return COMMA; }
"("              { return '('; }
")"              { return ')'; }
"{"              { return '{'; }
"}"              { return '}'; }
"["              { return '['; }
"]"              { return ']'; }

{num_float} {
  yylval->numberFloat = cLocale()->toDouble( QString::fromLatin1( yytext ) );
  return NUMBER_FLOAT;
}

{num_int} {
  bool ok = false;
  yylval->numberInt = cLocale()->toInt( QString::fromLatin1( yytext ), &ok );
  if ( ok )
    return NUMBER_INT;

  yylval->numberInt64 = cLocale()->toLongLong( QString::fromLatin1( yytext ), &ok );
  if ( ok )
    return NUMBER_INT64;

  yylval->numberFloat = cLocale()->toDouble( QString::fromLatin1( yytext ), &ok );
  return NUMBER_FLOAT;
}

{string_double} {
  yylval->text = new QString( stripText( QString::fromUtf8( yytext ) ) );
  return STRING_LITERAL;
}

{string_single} {
  yylval->text = new QString( stripText( QString::fromUtf8( yytext ) ) );
  return STRING_LITERAL;
}

{identifier} {
  TEXT;
  return IDENTIFIER;
}

. { return UNKNOWN_CHARACTER; }

%%