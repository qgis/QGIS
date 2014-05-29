/***************************************************************************
                         qgsexpressionparser.yy
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

%{
#include <qglobal.h>
#include <QList>
#include <cstdlib>
#include "qgsexpression.h"

#ifdef _MSC_VER
#  pragma warning( disable: 4065 )  // switch statement contains 'default' but no 'case' labels
#  pragma warning( disable: 4702 )  // unreachable code
#endif

// don't redeclare malloc/free
#define YYINCLUDED_STDLIB_H 1

//! from lexer
extern int exp_lex();
extern void exp_set_input_buffer(const char* buffer);

/** returns parsed tree, otherwise returns NULL and sets parserErrorMsg
    (interface function to be called from QgsExpression)
  */
QgsExpression::Node* parseExpression(const QString& str, QString& parserErrorMsg);

/** error handler for bison */
void exp_error(const char* msg);

//! varible where the parser error will be stored
QString gExpParserErrorMsg;
QgsExpression::Node* gExpParserRootNode;


// we want verbose error messages
#define YYERROR_VERBOSE 1

#define BINOP(x, y, z)  new QgsExpression::NodeBinaryOperator(x, y, z)

%}

%name-prefix "exp_"

%union
{
  QgsExpression::Node* node;
  QgsExpression::NodeList* nodelist;
  double numberFloat;
  int    numberInt;
  QString* text;
  QgsExpression::BinaryOperator b_op;
  QgsExpression::UnaryOperator u_op;
  QgsExpression::WhenThen* whenthen;
  QgsExpression::WhenThenList* whenthenlist;
}

%start root


//
// token definitions
//

// operator tokens
%token <b_op> OR AND EQ NE LE GE LT GT REGEXP LIKE IS PLUS MINUS MUL DIV MOD CONCAT POW
%token <u_op> NOT
%token IN

// literals
%token <numberFloat> NUMBER_FLOAT
%token <numberInt> NUMBER_INT
%token NULLVALUE

// tokens for conditional expressions
%token CASE WHEN THEN ELSE END

%token <text> STRING COLUMN_REF FUNCTION SPECIAL_COL

%token COMMA

%token Unknown_CHARACTER

//
// definition of non-terminal types
//

%type <node> expression
%type <nodelist> exp_list
%type <whenthen> when_then_clause
%type <whenthenlist> when_then_clauses

// debugging
%error-verbose

//
// operator precedence
//

// left associativity means that 1+2+3 translates to (1+2)+3
// the order of operators here determines their precedence

%left OR
%left AND
%right NOT
%left EQ NE LE GE LT GT REGEXP LIKE IS IN
%left PLUS MINUS
%left MUL DIV MOD
%right POW
%left CONCAT

%right UMINUS  // fictitious symbol (for unary minus)

%left COMMA

%destructor { delete $$; } <node>
%destructor { delete $$; } <nodelist>
%destructor { delete $$; } <text>

%%

root: expression { gExpParserRootNode = $1; }
    ;

expression:
      expression AND expression       { $$ = BINOP($2, $1, $3); }
    | expression OR expression        { $$ = BINOP($2, $1, $3); }
    | expression EQ expression        { $$ = BINOP($2, $1, $3); }
    | expression NE expression        { $$ = BINOP($2, $1, $3); }
    | expression LE expression        { $$ = BINOP($2, $1, $3); }
    | expression GE expression        { $$ = BINOP($2, $1, $3); }
    | expression LT expression        { $$ = BINOP($2, $1, $3); }
    | expression GT expression        { $$ = BINOP($2, $1, $3); }
    | expression REGEXP expression    { $$ = BINOP($2, $1, $3); }
    | expression LIKE expression      { $$ = BINOP($2, $1, $3); }
    | expression IS expression        { $$ = BINOP($2, $1, $3); }
    | expression PLUS expression      { $$ = BINOP($2, $1, $3); }
    | expression MINUS expression     { $$ = BINOP($2, $1, $3); }
    | expression MUL expression       { $$ = BINOP($2, $1, $3); }
    | expression DIV expression       { $$ = BINOP($2, $1, $3); }
    | expression MOD expression       { $$ = BINOP($2, $1, $3); }
    | expression POW expression       { $$ = BINOP($2, $1, $3); }
    | expression CONCAT expression    { $$ = BINOP($2, $1, $3); }
    | NOT expression                  { $$ = new QgsExpression::NodeUnaryOperator($1, $2); }
    | '(' expression ')'              { $$ = $2; }

    | FUNCTION '(' exp_list ')'
        {
          int fnIndex = QgsExpression::functionIndex(*$1);
          if (fnIndex == -1)
          {
            // this should not actually happen because already in lexer we check whether an identifier is a known function
            // (if the name is not known the token is parsed as a column)
            exp_error("Function is not known");
            YYERROR;
          }
          if ( QgsExpression::Functions()[fnIndex]->params() != -1
               && QgsExpression::Functions()[fnIndex]->params() != $3->count() )
          {
            exp_error("Function is called with wrong number of arguments");
            YYERROR;
          }
          $$ = new QgsExpression::NodeFunction(fnIndex, $3);
          delete $1;
        }

    | expression IN '(' exp_list ')'     { $$ = new QgsExpression::NodeInOperator($1, $4, false);  }
    | expression NOT IN '(' exp_list ')' { $$ = new QgsExpression::NodeInOperator($1, $5, true); }

    | PLUS expression %prec UMINUS { $$ = $2; }
    | MINUS expression %prec UMINUS { $$ = new QgsExpression::NodeUnaryOperator( QgsExpression::uoMinus, $2); }

    | CASE when_then_clauses END      { $$ = new QgsExpression::NodeCondition($2); }
    | CASE when_then_clauses ELSE expression END  { $$ = new QgsExpression::NodeCondition($2,$4); }

    // columns
    | COLUMN_REF                  { $$ = new QgsExpression::NodeColumnRef( *$1 ); delete $1; }

    // special columns (actually functions with no arguments)
    | SPECIAL_COL
        {
          int fnIndex = QgsExpression::functionIndex(*$1);
          if (fnIndex == -1)
          {
      if ( !QgsExpression::hasSpecialColumn( *$1 ) )
	    {
	      exp_error("Special column is not known");
	      YYERROR;
	    }
	    // $var is equivalent to _specialcol_( "$var" )
	    QgsExpression::NodeList* args = new QgsExpression::NodeList();
	    QgsExpression::NodeLiteral* literal = new QgsExpression::NodeLiteral( *$1 );
	    args->append( literal );
	    $$ = new QgsExpression::NodeFunction( QgsExpression::functionIndex( "_specialcol_" ), args );
          }
	  else
	  {
	    $$ = new QgsExpression::NodeFunction( fnIndex, NULL );
	    delete $1;
	  }
        }

    //  literals
    | NUMBER_FLOAT                { $$ = new QgsExpression::NodeLiteral( QVariant($1) ); }
    | NUMBER_INT                  { $$ = new QgsExpression::NodeLiteral( QVariant($1) ); }
    | STRING                      { $$ = new QgsExpression::NodeLiteral( QVariant(*$1) ); delete $1; }
    | NULLVALUE                   { $$ = new QgsExpression::NodeLiteral( QVariant() ); }
;

exp_list:
      exp_list COMMA expression { $$ = $1; $1->append($3); }
    | expression              { $$ = new QgsExpression::NodeList(); $$->append($1); }
    ;

when_then_clauses:
      when_then_clauses when_then_clause  { $$ = $1; $1->append($2); }
    | when_then_clause                    { $$ = new QgsExpression::WhenThenList(); $$->append($1); }
    ;

when_then_clause:
      WHEN expression THEN expression     { $$ = new QgsExpression::WhenThen($2,$4); }
    ;

%%

// returns parsed tree, otherwise returns NULL and sets parserErrorMsg
QgsExpression::Node* parseExpression(const QString& str, QString& parserErrorMsg)
{
  gExpParserRootNode = NULL;
  exp_set_input_buffer(str.toUtf8().constData());
  int res = exp_parse();

  // list should be empty when parsing was OK
  if (res == 0) // success?
  {
    return gExpParserRootNode;
  }
  else // error?
  {
    parserErrorMsg = gExpParserErrorMsg;
    return NULL;
  }
}


void exp_error(const char* msg)
{
  gExpParserErrorMsg = msg;
}

