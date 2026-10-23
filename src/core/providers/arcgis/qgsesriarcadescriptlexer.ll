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
%option case-insensitive
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
num_bin        0[bB][01]+
num_oct        0[oO][0-7]+
num_hex        0[xX][0-9a-fA-F]+
num_float      ({dig}+\.{dig}*|\.{dig}+)([eE][-+]?{dig}+)?|{dig}+[eE][-+]?{dig}+

identifier     [A-Za-z_][A-Za-z0-9_]*
profile_var    \$[A-Za-z_][A-Za-z0-9_]*

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
"infinity" {
  yylval->numberFloat = std::numeric_limits<double>::infinity();
  return NUMBER_FLOAT;
}
"pi" {
  yylval->numberFloat = 3.14159265358979323846;
  return NUMBER_FLOAT;
}
"nan" {
  yylval->numberFloat = std::numeric_limits<double>::quiet_NaN();
  return NUMBER_FLOAT;
}

"++"             { return PLUS_PLUS; }
"--"             { return MINUS_MINUS; }
"+="             { return PLUS_ASSIGN; }
"-="             { return MINUS_ASSIGN; }
"*="             { return MUL_ASSIGN; }
"/="             { return DIV_ASSIGN; }
"="              { return ASSIGN; }
"=="             { return EQ; }
"!="             { return NE; }
"<="             { return LE; }
">="             { return GE; }
"<<"             { return LEFT_SHIFT; }
">>>"            { return UNSIGNED_RIGHT_SHIFT; }
">>"             { return SIGNED_RIGHT_SHIFT; }
"<"              { return LT; }
">"              { return GT; }
"&&"             { return LOGICAL_AND; }
"||"             { return LOGICAL_OR; }
"!"              { return NOT; }
"~"              { return BITWISE_NOT; }
"&"              { return BITWISE_AND; }
"|"              { return BITWISE_OR; }
"^"              { return BITWISE_XOR; }

"+"              { return PLUS; }
"-"              { return MINUS; }
"*"              { return MUL; }
"/"              { return DIV; }
"%"              { return MOD; }

";"              { return SEMICOLON; }
","              { return COMMA; }
":"              { return ':'; }
"?."             { return SAFE_DOT; }
"."              { return '.'; }
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

{num_bin} {
  bool ok = false;
  // yytext + 2 to skip the 0b prefix
  yylval->numberInt = QString::fromLatin1( yytext + 2 ).toInt( &ok, 2 );
  if ( ok )
    return NUMBER_INT;

  yylval->numberInt64 = QString::fromLatin1( yytext + 2 ).toLongLong( &ok, 2 );
  if ( ok )
    return NUMBER_INT64;

  return UNKNOWN_CHARACTER;
}

{num_oct} {
  bool ok = false;
  // yytext + 2 to skip the 0o prefix
  yylval->numberInt = QString::fromLatin1( yytext + 2 ).toInt( &ok, 8 );
  if ( ok )
    return NUMBER_INT;

  yylval->numberInt64 = QString::fromLatin1( yytext + 2 ).toLongLong( &ok, 8 );
  if ( ok )
    return NUMBER_INT64;

  return UNKNOWN_CHARACTER;
}

{num_hex} {
  bool ok = false;
  // yytext + 2 to skip the 0x prefix
  yylval->numberInt = QString::fromLatin1( yytext + 2 ).toInt( &ok, 16 );
  if ( ok )
    return NUMBER_INT;

  yylval->numberInt64 = QString::fromLatin1( yytext + 2 ).toLongLong( &ok, 16 );
  if ( ok )
    return NUMBER_INT64;

  return UNKNOWN_CHARACTER;
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

{profile_var} {
  TEXT;
  return PROFILE_VARIABLE;
}

. { return UNKNOWN_CHARACTER; }

%%