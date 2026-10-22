/***************************************************************************
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

%{
#include "qgsesriarcadescript.h"

#include <cstdlib>

#include <qglobal.h>
#include <QList>
#include <QStringList>

#ifdef _MSC_VER
#  pragma warning( disable: 4065 )
#  pragma warning( disable: 4702 )
#endif

#define YYINCLUDED_STDLIB_H 1

struct arcadescript_parser_context;
#include "qgsesriarcadeparser.hpp"

// from lexer
typedef void *yyscan_t;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern int arcade_lex_init( yyscan_t *scanner );
extern int arcade_lex_destroy( yyscan_t scanner );
extern int arcade_lex( YYSTYPE *yylval_param, yyscan_t yyscanner );
extern YY_BUFFER_STATE arcade__scan_string( const char *buffer, yyscan_t scanner );

QgsEsriArcadeBlockStatement *parseArcadeScript( const QString &scriptText, QString &parserErrorMessage );
void arcade_error( arcadescript_parser_context *parserContext, const QString &errorMessage );

struct arcadescript_parser_context
{
  // lexer scanner instance
  yyscan_t flexScanner;
  // parser error message
  QString errorMessage;
  // root block node of the parsed script
  QgsEsriArcadeBlockStatement *rootBlock = nullptr;

  arcadescript_parser_context() = default;
};

#define scanner parserContext->flexScanner
#define YYERROR_VERBOSE 1
%}

%define api.pure
%lex-param {void *scanner}
%parse-param {arcadescript_parser_context *parserContext}

%union
{
  QgsEsriArcadeStatement *statement;
  QgsEsriArcadeBlockStatement *blockStatement;
  QgsEsriArcadeExpression *expression;
  QList<QgsEsriArcadeExpression *> *expressionList;
  QStringList *identifierList;
  QString *text;
  double numberFloat;
  int numberInt;
  qlonglong numberInt64;
  bool boolValue;
}

%start root

// keywords
%token VAR FUNCTION RETURN
%token <boolValue> BOOLEAN_LITERAL
%token NULL_LITERAL
%token <numberFloat> NUMBER_FLOAT
%token <numberInt> NUMBER_INT
%token <numberInt64> NUMBER_INT64
%token <text> STRING_LITERAL IDENTIFIER

// operators and separators
%token ASSIGN
%token EQ NE LT GT LE GE
%token LOGICAL_AND LOGICAL_OR NOT
%token PLUS MINUS MUL DIV MOD
%token SEMICOLON COMMA
%token UNKNOWN_CHARACTER

// non-terminal type associations
%type <blockStatement> statement_list block_statement
%type <statement> statement var_declaration return_statement function_declaration expression_statement
%type <expression> expr primary_expr postfix_expr unary_expr multiplicative_expr additive_expr relational_expr equality_expr logical_and_expr logical_or_expr assignment_expr opt_initializer opt_expr
%type <expressionList> opt_argument_list argument_list
%type <identifierList> opt_parameter_list parameter_list

%destructor { delete $$; } <statement>
%destructor { delete $$; } <blockStatement>
%destructor { delete $$; } <expression>
%destructor { delete $$; } <text>
%destructor { qDeleteAll( *$$ ); delete $$; } <expressionList>
%destructor { delete $$; } <identifierList>

// precedence declarations
%right ASSIGN
%left LOGICAL_OR
%left LOGICAL_AND
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left MUL DIV MOD
%right NOT UMINUS
%left '[' ']' '(' ')'

%%

root:
    statement_list
    {
      parserContext->rootBlock = $1;
    }
;

statement_list:
    statement_list statement
    {
      $$ = $1;
      if ( $2 )
        $$->appendStatement( $2 );
    }
  | /* empty */
    {
      $$ = new QgsEsriArcadeBlockStatement();
    }
;

statement:
    var_declaration
    {
      $$ = $1;
    }
  | return_statement
    {
      $$ = $1;
    }
  | function_declaration
    {
      $$ = $1;
    }
  | expression_statement
    {
      $$ = $1;
    }
  | block_statement
    {
      $$ = $1;
    }
  | SEMICOLON
    {
      $$ = nullptr;
    }
;

block_statement:
    '{' statement_list '}'
    {
      $$ = $2;
    }
;

var_declaration:
    VAR IDENTIFIER opt_initializer SEMICOLON
    {
      $$ = new QgsEsriArcadeVariableDeclarationStatement( *$2, $3 );
      delete $2;
    }
;

opt_initializer:
    ASSIGN expr
    {
      $$ = $2;
    }
  | /* empty */
    {
      $$ = nullptr;
    }
;

return_statement:
    RETURN opt_expr SEMICOLON
    {
      $$ = new QgsEsriArcadeReturnStatement( $2 );
    }
;

opt_expr:
    expr
    {
      $$ = $1;
    }
  | /* empty */
    {
      $$ = nullptr;
    }
;

function_declaration:
    FUNCTION IDENTIFIER '(' opt_parameter_list ')' block_statement
    {
      $$ = new QgsEsriArcadeFunctionDeclarationStatement( *$2, *$4, $6 );
      delete $2;
      delete $4;
    }
;

opt_parameter_list:
    parameter_list
    {
      $$ = $1;
    }
  | /* empty */
    {
      $$ = new QStringList();
    }
;

parameter_list:
    parameter_list COMMA IDENTIFIER
    {
      $$ = $1;
      $$->append( *$3 );
      delete $3;
    }
  | IDENTIFIER
    {
      $$ = new QStringList();
      $$->append( *$1 );
      delete $1;
    }
;

expression_statement:
    expr SEMICOLON
    {
      $$ = new QgsEsriArcadeExpressionStatement( $1 );
    }
;

expr:
    assignment_expr
    {
      $$ = $1;
    }
;

assignment_expr:
    logical_or_expr
    {
      $$ = $1;
    }
  | postfix_expr ASSIGN assignment_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boAssign, $1, $3 );
    }
;

logical_or_expr:
    logical_and_expr
    {
      $$ = $1;
    }
  | logical_or_expr LOGICAL_OR logical_and_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boLogicalOr, $1, $3 );
    }
;

logical_and_expr:
    equality_expr
    {
      $$ = $1;
    }
  | logical_and_expr LOGICAL_AND equality_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boLogicalAnd, $1, $3 );
    }
;

equality_expr:
    relational_expr
    {
      $$ = $1;
    }
  | equality_expr EQ relational_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boEqual, $1, $3 );
    }
  | equality_expr NE relational_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boNotEqual, $1, $3 );
    }
;

relational_expr:
    additive_expr
    {
      $$ = $1;
    }
  | relational_expr LT additive_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boLessThan, $1, $3 );
    }
  | relational_expr GT additive_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boGreaterThan, $1, $3 );
    }
  | relational_expr LE additive_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boLessThanOrEqual, $1, $3 );
    }
  | relational_expr GE additive_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boGreaterThanOrEqual, $1, $3 );
    }
;

additive_expr:
    multiplicative_expr
    {
      $$ = $1;
    }
  | additive_expr PLUS multiplicative_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boPlus, $1, $3 );
    }
  | additive_expr MINUS multiplicative_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boMinus, $1, $3 );
    }
;

multiplicative_expr:
    unary_expr
    {
      $$ = $1;
    }
  | multiplicative_expr MUL unary_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boMultiply, $1, $3 );
    }
  | multiplicative_expr DIV unary_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boDivide, $1, $3 );
    }
  | multiplicative_expr MOD unary_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boModulo, $1, $3 );
    }
;

unary_expr:
    postfix_expr
    {
      $$ = $1;
    }
  | MINUS unary_expr %prec UMINUS
    {
      $$ = new QgsEsriArcadeUnaryOperatorExpression( QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::uoMinus, $2 );
    }
  | PLUS unary_expr %prec UMINUS
    {
      $$ = new QgsEsriArcadeUnaryOperatorExpression( QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::uoPlus, $2 );
    }
  | NOT unary_expr
    {
      $$ = new QgsEsriArcadeUnaryOperatorExpression( QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::uoNot, $2 );
    }
;

postfix_expr:
    primary_expr
    {
      $$ = $1;
    }
  | postfix_expr '[' expr ']'
    {
      $$ = new QgsEsriArcadeIndexExpression( $1, $3 );
    }
  | IDENTIFIER '(' opt_argument_list ')'
    {
      $$ = new QgsEsriArcadeFunctionCallExpression( *$1, *$3 );
      delete $1;
      delete $3;
    }
;

primary_expr:
    NUMBER_INT
    {
      $$ = new QgsEsriArcadeLiteralExpression( QVariant( $1 ) );
    }
  | NUMBER_INT64
    {
      $$ = new QgsEsriArcadeLiteralExpression( QVariant( $1 ) );
    }
  | NUMBER_FLOAT
    {
      $$ = new QgsEsriArcadeLiteralExpression( QVariant( $1 ) );
    }
  | STRING_LITERAL
    {
      $$ = new QgsEsriArcadeLiteralExpression( QVariant( *$1 ) );
      delete $1;
    }
  | BOOLEAN_LITERAL
    {
      $$ = new QgsEsriArcadeLiteralExpression( QVariant( $1 ) );
    }
  | NULL_LITERAL
    {
      $$ = new QgsEsriArcadeLiteralExpression( QVariant() );
    }
  | IDENTIFIER
    {
      $$ = new QgsEsriArcadeVariableReferenceExpression( *$1 );
      delete $1;
    }
  | '(' expr ')'
    {
      $$ = $2;
    }
;

opt_argument_list:
    argument_list
    {
      $$ = $1;
    }
  | /* empty */
    {
      $$ = new QList<QgsEsriArcadeExpression *>();
    }
;

argument_list:
    argument_list COMMA expr
    {
      $$ = $1;
      $$->append( $3 );
    }
  | expr
    {
      $$ = new QList<QgsEsriArcadeExpression *>();
      $$->append( $1 );
    }
;

%%

QgsEsriArcadeBlockStatement *parseArcadeScript( const QString &scriptText, QString &parserErrorMessage )
{
  arcadescript_parser_context context;
  arcade_lex_init( &context.flexScanner );
  arcade__scan_string( scriptText.toUtf8().constData(), context.flexScanner );

  const int result = arcade_parse( &context );
  arcade_lex_destroy( context.flexScanner );

  if ( result == 0 && context.rootBlock )
  {
    return context.rootBlock;
  }

  parserErrorMessage = context.errorMessage;
  delete context.rootBlock;
  return nullptr;
}

void arcade_error( arcadescript_parser_context *parserContext, const QString &errorMessage )
{
  parserContext->errorMessage = errorMessage;
}