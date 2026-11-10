/***************************************************************************
    qgsvbscriptlexer.ll
    ---------------------------
    begin                : November 2026
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
%option prefix="vbscript_"
%option reentrant
%option bison-bridge
%option 8bit

%{
#include "qgsvbscript.h"

struct vbscript_parser_context;
#include "qgsvbscriptparser.hpp"

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
  // character escaping is very basic in VBscripts...!
  result.replace( "\"\""_L1, "\""_L1 );
  return result;
}

// standard c locale for numeric conversions
Q_GLOBAL_STATIC_WITH_ARGS( QLocale, cLocale, ( "C" ) )
%}

white          [ \t]+
non_ascii      [\x80-\xFF]
continuation   _[ \t]*\r?\n
newline        \r?\n|:

dig            [0-9]
num_int        {dig}+
num_hex        &[hH][0-9a-fA-F]+
num_oct        &[oO][0-7]+
num_float      ({dig}+\.{dig}*|\.{dig}+)([eE][-+]?{dig}+)?|{dig}+[eE][-+]?{dig}+

field_ref      \[([^\]]|\]\])*\]
field_char     [A-Za-z0-9_.]|{non_ascii}
field_ref_unbracketed ({field_char}|"\]\]")+([ \t]+({field_char}|"\]\]")+)*[ \t]*\]
identifier     ([A-Za-z_]|{non_ascii})([A-Za-z0-9_]|{non_ascii})*

string_literal  \"([^\"]|\"\")*\"
date_literal    \#[^#\r\n]+\#

line_comment     (\'|(?i:rem)[ \t])[^\r\n]*

%%

{white}          /* skip whitespace */
{line_comment}   /* skip line comment */
{continuation}   /* skip line continuation (handles multi-line statement concatemnation to single-line in vbscript */

"dim"            { return DIM; }
"set"            { return SET; }
"if"             { return IF; }
"then"           { return THEN; }
"else"           { return ELSE; }
"elseif"         { return ELSEIF; }
"end"            { return END; }
"for"            { return FOR; }
"to"             { return TO; }
"step"           { return STEP; }
"each"           { return EACH; }
"in"             { return IN; }
"next"           { return NEXT; }
"while"          { return WHILE; }
"wend"           { return WEND; }
"do"             { return DO; }
"loop"           { return LOOP; }
"until"          { return UNTIL; }
"exit"           { return EXIT; }
"function"       { return FUNCTION; }
"sub"            { return SUB; }
"call"           { return CALL; }
"array"          { return ARRAY; }

"true"           { yylval->boolValue = true; return BOOLEAN_LITERAL; }
"false"          { yylval->boolValue = false; return BOOLEAN_LITERAL; }
"null"           { return NULL_LITERAL; }
"empty"          { return EMPTY_LITERAL; }
"nothing"        { return NOTHING_LITERAL; }
"vbNewLine" {
  yylval->text = new QString("\n");
  return STRING_LITERAL;
}
"vbCrLf" {
  yylval->text = new QString("\n");
  return STRING_LITERAL;
}
"vbTab" {
  yylval->text = new QString("\t");
  return STRING_LITERAL;
}
"="              { return '='; }
"<>"             { return NE; }
"<="             { return LE; }
">="             { return GE; }
"<"              { return '<'; }
">"              { return '>'; }
"is"             { return IS; }
"and"            { return AND; }
"or"             { return OR; }
"xor"            { return XOR; }
"eqv"            { return EQV; }
"imp"            { return IMP; }
"not"            { return NOT; }
"&"              { return '&'; }

"+"              { return '+'; }
"-"              { return '-'; }
"*"              { return '*'; }
"/"              { return '/'; }
"\\"             { return '\\'; }
"mod"            { return MOD; }
"^"              { return '^'; }

","              { return ','; }
"."              { return '.'; }
"("              { return '('; }
")"              { return ')'; }

{num_float} {
  yylval->numberFloat = cLocale()->toDouble( QString::fromLatin1( yytext ) );
  return NUMBER_FLOAT;
}

{num_oct} {
  bool ok = false;
  // yytext + 2 to skip the &o prefix
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
  // yytext + 2 to skip the &h prefix
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

{string_literal} {
  yylval->text = new QString( stripText( QString::fromUtf8( yytext ) ) );
  return STRING_LITERAL;
}

{date_literal} {
  QString content = QString::fromUtf8( yytext );
  content = content.mid( 1, content.length() - 2 );
  yylval->text = new QString( content );
  return STRING_LITERAL;
}

{field_ref} {
  yylval->text = new QString( QgsVbScript::extractFieldReference( QString::fromUtf8( yytext ) ) );
  return FIELD_REFERENCE;
}

{field_ref_unbracketed} {
  yylval->text = new QString( QgsVbScript::extractFieldReference( QString::fromUtf8( yytext ) ) );
  return FIELD_REFERENCE;
}

{identifier} {
  TEXT;
  return IDENTIFIER;
}

{newline}        { return EOL; }

.                { return UNKNOWN_CHARACTER; }

%%