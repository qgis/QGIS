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
  QgsEsriArcadeDictionaryElement *dictionaryElement;
  QList<QgsEsriArcadeDictionaryElement *> *dictionaryElementList;
  QgsEsriArcadeAssignmentStatement::AssignmentOperator assignmentOperator;
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
%token <text> STRING_LITERAL IDENTIFIER PROFILE_VARIABLE

// operators and separators
%token ASSIGN PLUS_ASSIGN MINUS_ASSIGN MUL_ASSIGN DIV_ASSIGN
%token PLUS_PLUS MINUS_MINUS
%token LEFT_SHIFT SIGNED_RIGHT_SHIFT UNSIGNED_RIGHT_SHIFT
%token BITWISE_AND BITWISE_OR BITWISE_XOR BITWISE_NOT
%token EQ NE LT GT LE GE
%token LOGICAL_AND LOGICAL_OR NOT
%token PLUS MINUS MUL DIV MOD
%token SEMICOLON COMMA
%token SAFE_DOT UNKNOWN_CHARACTER

// non-terminal type associations
%type <blockStatement> statement_list block_statement
%type <statement> statement var_declaration assignment_statement return_statement function_declaration expression_statement return_without_semicolon
%type <expression> expr primary_expr postfix_expr unary_expr multiplicative_expr additive_expr shift_expr relational_expr equality_expr bitwise_and_expr bitwise_xor_expr bitwise_or_expr logical_and_expr logical_or_expr opt_initializer opt_expr
%type <expressionList> opt_argument_list argument_list opt_element_list element_list
%type <identifierList> opt_parameter_list parameter_list
%type <assignmentOperator> assign_op
%type <dictionaryElement> property_entry
%type <dictionaryElementList> opt_property_list property_list
%type <text> property_name

%destructor { delete $$; } <statement>
%destructor { delete $$; } <blockStatement>
%destructor { delete $$; } <expression>
%destructor { delete $$; } <text>
%destructor { qDeleteAll( *$$ ); delete $$; } <expressionList>
%destructor { delete $$; } <identifierList>
%destructor { delete $$; } <dictionaryElement>
%destructor { qDeleteAll( *$$ ); delete $$; } <dictionaryElementList>

// operator precedence and associativity
%left LOGICAL_OR
%left LOGICAL_AND
%left BITWISE_OR
%left BITWISE_XOR
%left BITWISE_AND
%left EQ NE
%left LT GT LE GE
%left LEFT_SHIFT SIGNED_RIGHT_SHIFT UNSIGNED_RIGHT_SHIFT
%left PLUS MINUS
%left MUL DIV MOD
%right NOT BITWISE_NOT UMINUS UPLUS PREFIX_INC PREFIX_DEC
%left '[' ']' '(' ')' '.' SAFE_DOT
%left POSTFIX_INC POSTFIX_DEC

%%

root:
    statement_list
    {
      parserContext->rootBlock = $1;
    }
  | statement_list expr
    {
      $1->appendStatement( new QgsEsriArcadeExpressionStatement( $2 ) );
      parserContext->rootBlock = $1;
    }
  | statement_list postfix_expr assign_op expr
    {
      $1->appendStatement( new QgsEsriArcadeAssignmentStatement( $3, $2, $4 ) );
      parserContext->rootBlock = $1;
    }
  | statement_list return_without_semicolon
    {
      $1->appendStatement( $2 );
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
  | assignment_statement
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
  | '{' statement_list expr '}'
    {
      $2->appendStatement( new QgsEsriArcadeExpressionStatement( $3 ) );
      $$ = $2;
    }
  | '{' statement_list postfix_expr assign_op expr '}'
    {
      $2->appendStatement( new QgsEsriArcadeAssignmentStatement( $4, $3, $5 ) );
      $$ = $2;
    }
  | '{' statement_list return_without_semicolon '}'
    {
      $2->appendStatement( $3 );
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

assign_op:
    ASSIGN
    {
      $$ = QgsEsriArcadeAssignmentStatement::AssignmentOperator::aoAssign;
    }
  | PLUS_ASSIGN
    {
      $$ = QgsEsriArcadeAssignmentStatement::AssignmentOperator::aoPlusAssign;
    }
  | MINUS_ASSIGN
    {
      $$ = QgsEsriArcadeAssignmentStatement::AssignmentOperator::aoMinusAssign;
    }
  | MUL_ASSIGN
    {
      $$ = QgsEsriArcadeAssignmentStatement::AssignmentOperator::aoMultiplyAssign;
    }
  | DIV_ASSIGN
    {
      $$ = QgsEsriArcadeAssignmentStatement::AssignmentOperator::aoDivideAssign;
    }
;

assignment_statement:
    postfix_expr assign_op expr SEMICOLON
    {
      $$ = new QgsEsriArcadeAssignmentStatement( $2, $1, $3 );
    }
;

return_statement:
    RETURN opt_expr SEMICOLON
    {
      $$ = new QgsEsriArcadeReturnStatement( $2 );
    }
;

return_without_semicolon:
    RETURN opt_expr
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
    logical_or_expr
    {
      $$ = $1;
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
    bitwise_or_expr
    {
      $$ = $1;
    }
  | logical_and_expr LOGICAL_AND bitwise_or_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boLogicalAnd, $1, $3 );
    }
;

bitwise_or_expr:
    bitwise_xor_expr
    {
      $$ = $1;
    }
  | bitwise_or_expr BITWISE_OR bitwise_xor_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boBitwiseOr, $1, $3 );
    }
;

bitwise_xor_expr:
    bitwise_and_expr
    {
      $$ = $1;
    }
  | bitwise_xor_expr BITWISE_XOR bitwise_and_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boBitwiseXor, $1, $3 );
    }
;

bitwise_and_expr:
    equality_expr
    {
      $$ = $1;
    }
  | bitwise_and_expr BITWISE_AND equality_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boBitwiseAnd, $1, $3 );
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
    shift_expr
    {
      $$ = $1;
    }
  | relational_expr LT shift_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boLessThan, $1, $3 );
    }
  | relational_expr GT shift_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boGreaterThan, $1, $3 );
    }
  | relational_expr LE shift_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boLessThanOrEqual, $1, $3 );
    }
  | relational_expr GE shift_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boGreaterThanOrEqual, $1, $3 );
    }
;

shift_expr:
    additive_expr
    {
      $$ = $1;
    }
  | shift_expr LEFT_SHIFT additive_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boBitwiseLeftShift, $1, $3 );
    }
  | shift_expr SIGNED_RIGHT_SHIFT additive_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boBitwiseSignedRightShift, $1, $3 );
    }
  | shift_expr UNSIGNED_RIGHT_SHIFT additive_expr
    {
      $$ = new QgsEsriArcadeBinaryOperatorExpression( QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::boBitwiseUnsignedRightShift, $1, $3 );
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
  | PLUS unary_expr %prec UPLUS
    {
      $$ = new QgsEsriArcadeUnaryOperatorExpression( QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::uoPlus, $2 );
    }
  | NOT unary_expr
    {
      $$ = new QgsEsriArcadeUnaryOperatorExpression( QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::uoNot, $2 );
    }
  | BITWISE_NOT unary_expr
    {
      $$ = new QgsEsriArcadeUnaryOperatorExpression( QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::uoBitwiseNot, $2 );
    }
  | PLUS_PLUS unary_expr %prec PREFIX_INC
    {
      $$ = new QgsEsriArcadeUnaryOperatorExpression( QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::uoPreIncrement, $2 );
    }
  | MINUS_MINUS unary_expr %prec PREFIX_DEC
    {
      $$ = new QgsEsriArcadeUnaryOperatorExpression( QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::uoPreDecrement, $2 );
    }
;

postfix_expr:
    primary_expr
    {
      $$ = $1;
    }
  | postfix_expr PLUS_PLUS %prec POSTFIX_INC
    {
      $$ = new QgsEsriArcadeUnaryOperatorExpression( QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::uoPostIncrement, $1 );
    }
  | postfix_expr MINUS_MINUS %prec POSTFIX_DEC
    {
      $$ = new QgsEsriArcadeUnaryOperatorExpression( QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::uoPostDecrement, $1 );
    }
  | postfix_expr '.' IDENTIFIER
    {
      $$ = new QgsEsriArcadeMemberAccessExpression( $1, *$3 );
      delete $3;
    }
  | postfix_expr SAFE_DOT IDENTIFIER
    {
      $$ = new QgsEsriArcadeMemberAccessExpression( $1, *$3, true );
      delete $3;
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
  | PROFILE_VARIABLE
    {
      $$ = new QgsEsriArcadeProfileVariableReferenceExpression( *$1 );
      delete $1;
    }
  | '(' expr ')'
    {
      $$ = $2;
    }
  | '[' opt_element_list ']'
    {
      $$ = new QgsEsriArcadeArrayLiteralExpression( *$2 );
      delete $2;
    }
  | '{' opt_property_list '}'
    {
      $$ = new QgsEsriArcadeDictionaryLiteralExpression( *$2 );
      delete $2;
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

opt_element_list:
    element_list
    {
      $$ = $1;
    }
  | /* empty element list */
    {
      $$ = new QList<QgsEsriArcadeExpression *>();
    }
;

element_list:
    element_list COMMA expr
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

opt_property_list:
    property_list
    {
      $$ = $1;
    }
  | property_list COMMA
    {
      $$ = $1;
    }
  | /* empty property list */
    {
      $$ = new QList<QgsEsriArcadeDictionaryElement *>();
    }
;

property_list:
    property_list COMMA property_entry
    {
      $$ = $1;
      $$->append( $3 );
    }
  | property_entry
    {
      $$ = new QList<QgsEsriArcadeDictionaryElement *>();
      $$->append( $1 );
    }
;

property_entry:
    property_name ':' expr
    {
      $$ = new QgsEsriArcadeDictionaryElement( *$1, $3, false );
      delete $1;
    }
  | IDENTIFIER
    {
      $$ = new QgsEsriArcadeDictionaryElement( *$1, new QgsEsriArcadeVariableReferenceExpression( *$1 ), true );
      delete $1;
    }
;

property_name:
    IDENTIFIER
    {
      $$ = $1;
    }
  | STRING_LITERAL
    {
      $$ = $1;
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
