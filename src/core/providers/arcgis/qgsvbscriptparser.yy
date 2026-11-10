/***************************************************************************
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

%{
#include "qgsvbscript.h"

#include <cstdlib>

#include <qglobal.h>
#include <QList>
#include <QStringList>

#ifdef _MSC_VER
#  pragma warning( disable: 4065 )
#  pragma warning( disable: 4702 )
#endif

#define YYINCLUDED_STDLIB_H 1

struct vbscript_parser_context;
#include "qgsvbscriptparser.hpp"

// from lexer
typedef void *yyscan_t;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern int vbscript_lex_init( yyscan_t *scanner );
extern int vbscript_lex_destroy( yyscan_t scanner );
extern int vbscript_lex( YYSTYPE *yylval_param, yyscan_t yyscanner );
extern YY_BUFFER_STATE vbscript__scan_string( const char *buffer, yyscan_t scanner );

QgsVbScriptBlockStatement *parseVbScript( const QString &scriptText, QString &parserErrorMessage );
void vbscript_error( vbscript_parser_context *parserContext, const QString &errorMessage );

struct vbscript_parser_context
{
  // lexer scanner instance
  yyscan_t flexScanner;
  // parser error message
  QString errorMessage;
  // root block node of the parsed script
  QgsVbScriptBlockStatement *rootBlock = nullptr;

  vbscript_parser_context() = default;
};

#define scanner parserContext->flexScanner
#define YYERROR_VERBOSE 1
%}

%define api.pure
%lex-param {void *scanner}
%parse-param {vbscript_parser_context *parserContext}

// suppress bison shift/reduce conflict warnings -- the VBScript language
// has inherent ambiguities because it doesn't force line ending characters!
%expect 1

%union
{
  QgsVbScriptNode *node;
  QgsVbScriptStatement *statement;
  QgsVbScriptBlockStatement *blockStatement;
  QgsVbScriptIfStatement *ifStatement;
  QgsVbScriptExpression *expression;
  QList<QgsVbScriptExpression *> *expressionList;
  QStringList *identifierList;
  QString *text;
  double numberFloat;
  int numberInt;
  qlonglong numberInt64;
  bool boolValue;
}

%start root

// keywords
%token DIM SET IF THEN ELSE ELSEIF END FOR TO STEP EACH IN NEXT WHILE WEND DO LOOP UNTIL EXIT FUNCTION SUB CALL ARRAY
%token <boolValue> BOOLEAN_LITERAL
%token NULL_LITERAL EMPTY_LITERAL NOTHING_LITERAL
%token <numberFloat> NUMBER_FLOAT
%token <numberInt> NUMBER_INT
%token <numberInt64> NUMBER_INT64
%token <text> STRING_LITERAL IDENTIFIER FIELD_REFERENCE

// operators and separators
%token NE LE GE IS AND OR XOR EQV IMP NOT MOD
%token EOL UNKNOWN_CHARACTER

// precedence tokens
%token LOWER_THAN_ELSE

// non-terminal type associations
%type <blockStatement> statement_list opt_else_clause
%type <statement> statement simple_statement var_declaration assignment_statement if_statement inline_if_statement for_statement while_statement exit_statement function_declaration expression_statement
%type <expression> expr primary_expr postfix_expr unary_expr exponent_expr multiplicative_expr integer_divide_expr modulo_expr additive_expr concat_expr relational_expr logical_not_expr logical_and_expr logical_or_expr logical_xor_expr logical_eqv_expr logical_imp_expr opt_step_clause
%type <expressionList> opt_argument_list argument_list
%type <identifierList> opt_parameter_list parameter_list identifier_list opt_param_clause
%type <text> parameter_name

%destructor { delete $$; } <statement>
%destructor { delete $$; } <blockStatement>
%destructor { delete $$; } <expression>
%destructor { delete $$; } <text>
%destructor { qDeleteAll( *$$ ); delete $$; } <expressionList>
%destructor { delete $$; } <identifierList>

// operator precedence and associativity
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%left IMP
%left EQV
%left XOR
%left OR
%left AND
%right NOT
%left '=' NE '<' '>' LE GE IS
%left '&'
%left '+' '-'
%left MOD
%left '\\'
%left '*' '/'
%right UMINUS UPLUS
%right '^'
%left '.' '(' ')'

%%

root:
    opt_eols statement_list opt_eols
    {
      parserContext->rootBlock = $2;
    }
  | opt_eols
    {
      parserContext->rootBlock = new QgsVbScriptBlockStatement();
    }
;

opt_eols:
    eols
  | /* empty */
;

eols:
    eols EOL
  | EOL
;

statement_list:
    statement_list eols statement
    {
      $$ = $1;
      if ( $3 )
        $$->appendStatement( $3 );
    }
  | statement
    {
      $$ = new QgsVbScriptBlockStatement();
      if ( $1 )
        $$->appendStatement( $1 );
    }
;

statement:
    simple_statement
    {
      $$ = $1;
    }
  | if_statement
    {
      $$ = $1;
    }
  | for_statement
    {
      $$ = $1;
    }
  | while_statement
    {
      $$ = $1;
    }
  | function_declaration
    {
      $$ = $1;
    }
;

simple_statement:
    var_declaration
    {
      $$ = $1;
    }
  | assignment_statement
    {
      $$ = $1;
    }
  | exit_statement
    {
      $$ = $1;
    }
  | expression_statement
    {
      $$ = $1;
    }
  | inline_if_statement
    {
      $$ = $1;
    }
;

var_declaration:
    DIM identifier_list
    {
      $$ = new QgsVbScriptVariableDeclarationStatement( *$2 );
      delete $2;
    }
;

identifier_list:
    identifier_list ',' IDENTIFIER
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

assignment_statement:
    postfix_expr '=' expr
    {
      $$ = new QgsVbScriptAssignmentStatement( $1, $3, false );
    }
  | SET postfix_expr '=' expr
    {
      $$ = new QgsVbScriptAssignmentStatement( $2, $4, true );
    }
;

if_statement:
    IF expr THEN eols statement_list opt_eols opt_else_clause END IF
    {
      $$ = new QgsVbScriptIfStatement( $2, $5, $7, false );
    }
  | IF expr THEN opt_eols END IF
    {
      $$ = new QgsVbScriptIfStatement( $2, new QgsVbScriptBlockStatement(), nullptr, false );
    }
;

opt_else_clause:
    ELSEIF expr THEN eols statement_list opt_eols opt_else_clause
    {
      $$ = new QgsVbScriptBlockStatement();
      $$->appendStatement( new QgsVbScriptIfStatement( $2, $5, $7, false ) );
    }
  | ELSE eols statement_list opt_eols
    {
      $$ = $3;
    }
  | /* empty */
    {
      $$ = nullptr;
    }
;

inline_if_statement:
    IF expr THEN simple_statement ELSE simple_statement
    {
      QgsVbScriptBlockStatement *trueBlock = new QgsVbScriptBlockStatement();
      trueBlock->appendStatement( $4 );
      QgsVbScriptBlockStatement *falseBlock = new QgsVbScriptBlockStatement();
      falseBlock->appendStatement( $6 );
      $$ = new QgsVbScriptIfStatement( $2, trueBlock, falseBlock, true );
    }
  | IF expr THEN simple_statement %prec LOWER_THAN_ELSE
    {
      QgsVbScriptBlockStatement *trueBlock = new QgsVbScriptBlockStatement();
      trueBlock->appendStatement( $4 );
      $$ = new QgsVbScriptIfStatement( $2, trueBlock, nullptr, true );
    }
;

for_statement:
    FOR IDENTIFIER '=' expr TO expr opt_step_clause eols statement_list opt_eols NEXT opt_next_identifier
    {
      $$ = new QgsVbScriptForStatement( *$2, $4, $6, $7, $9 );
      delete $2;
    }
  | FOR EACH IDENTIFIER IN expr eols statement_list opt_eols NEXT opt_next_identifier
    {
      $$ = new QgsVbScriptForEachStatement( *$3, $5, $7 );
      delete $3;
    }
;

opt_next_identifier:
    IDENTIFIER
    {
      delete $1;
    }
  | /* empty */
;

opt_step_clause:
    STEP expr
    {
      $$ = $2;
    }
  | /* empty */
    {
      $$ = nullptr;
    }
;

while_statement:
    WHILE expr eols statement_list opt_eols WEND
    {
      $$ = new QgsVbScriptWhileStatement( QgsVbScriptWhileStatement::LoopType::WhileWend, $2, $4 );
    }
  | DO WHILE expr eols statement_list opt_eols LOOP
    {
      $$ = new QgsVbScriptWhileStatement( QgsVbScriptWhileStatement::LoopType::DoWhile, $3, $5 );
    }
  | DO UNTIL expr eols statement_list opt_eols LOOP
    {
      $$ = new QgsVbScriptWhileStatement( QgsVbScriptWhileStatement::LoopType::DoUntil, $3, $5 );
    }
  | DO eols statement_list opt_eols LOOP WHILE expr
    {
      $$ = new QgsVbScriptWhileStatement( QgsVbScriptWhileStatement::LoopType::DoLoopWhile, $7, $3 );
    }
  | DO eols statement_list opt_eols LOOP UNTIL expr
    {
      $$ = new QgsVbScriptWhileStatement( QgsVbScriptWhileStatement::LoopType::DoLoopUntil, $7, $3 );
    }
;

exit_statement:
    EXIT DO
    {
      $$ = new QgsVbScriptExitStatement( QgsVbScriptExitStatement::ExitType::ExitDo );
    }
  | EXIT FOR
    {
      $$ = new QgsVbScriptExitStatement( QgsVbScriptExitStatement::ExitType::ExitFor );
    }
  | EXIT FUNCTION
    {
      $$ = new QgsVbScriptExitStatement( QgsVbScriptExitStatement::ExitType::ExitFunction );
    }
  | EXIT SUB
    {
      $$ = new QgsVbScriptExitStatement( QgsVbScriptExitStatement::ExitType::ExitSub );
    }
;

function_declaration:
    FUNCTION IDENTIFIER opt_param_clause eols statement_list opt_eols END FUNCTION
    {
      $$ = new QgsVbScriptFunctionDeclarationStatement( *$2, *$3, $5, false );
      delete $2;
      delete $3;
    }
  | FUNCTION IDENTIFIER opt_param_clause opt_eols END FUNCTION
    {
      $$ = new QgsVbScriptFunctionDeclarationStatement( *$2, *$3, new QgsVbScriptBlockStatement(), false );
      delete $2;
      delete $3;
    }
  | SUB IDENTIFIER opt_param_clause eols statement_list opt_eols END SUB
    {
      $$ = new QgsVbScriptFunctionDeclarationStatement( *$2, *$3, $5, true );
      delete $2;
      delete $3;
    }
  | SUB IDENTIFIER opt_param_clause opt_eols END SUB
    {
      $$ = new QgsVbScriptFunctionDeclarationStatement( *$2, *$3, new QgsVbScriptBlockStatement(), true );
      delete $2;
      delete $3;
    }
;

opt_param_clause:
    '(' opt_parameter_list ')'
    {
      $$ = $2;
    }
  | /* empty */
    {
      $$ = new QStringList();
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
    parameter_list ',' parameter_name
    {
      $$ = $1;
      $$->append( *$3 );
      delete $3;
    }
  | parameter_name
    {
      $$ = new QStringList();
      $$->append( *$1 );
      delete $1;
    }
;

parameter_name:
    IDENTIFIER
    {
      $$ = $1;
    }
  | FIELD_REFERENCE
    {
      $$ = new QString( QgsVbScript::quotedFieldReference( *$1 ) );
      delete $1;
    }
;

expression_statement:
    expr
    {
      $$ = new QgsVbScriptExpressionStatement( $1 );
    }
  | CALL postfix_expr
    {
      $$ = new QgsVbScriptExpressionStatement( $2 );
    }
;

expr:
    logical_imp_expr
    {
      $$ = $1;
    }
;

logical_imp_expr:
    logical_eqv_expr
    {
      $$ = $1;
    }
  | logical_imp_expr IMP logical_eqv_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::LogicalImp, $1, $3 );
    }
;

logical_eqv_expr:
    logical_xor_expr
    {
      $$ = $1;
    }
  | logical_eqv_expr EQV logical_xor_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::LogicalEqv, $1, $3 );
    }
;

logical_xor_expr:
    logical_or_expr
    {
      $$ = $1;
    }
  | logical_xor_expr XOR logical_or_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::LogicalXor, $1, $3 );
    }
;

logical_or_expr:
    logical_and_expr
    {
      $$ = $1;
    }
  | logical_or_expr OR logical_and_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::LogicalOr, $1, $3 );
    }
;

logical_and_expr:
    logical_not_expr
    {
      $$ = $1;
    }
  | logical_and_expr AND logical_not_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::LogicalAnd, $1, $3 );
    }
;

logical_not_expr:
    relational_expr
    {
      $$ = $1;
    }
  | NOT logical_not_expr
    {
      $$ = new QgsVbScriptUnaryOperatorExpression( QgsVbScriptUnaryOperatorExpression::UnaryOperator::Not, $2 );
    }
;

relational_expr:
    concat_expr
    {
      $$ = $1;
    }
  | relational_expr '=' concat_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::Equal, $1, $3 );
    }
  | relational_expr NE concat_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::NotEqual, $1, $3 );
    }
  | relational_expr '<' concat_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::LessThan, $1, $3 );
    }
  | relational_expr '>' concat_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::GreaterThan, $1, $3 );
    }
  | relational_expr LE concat_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::LessThanOrEqual, $1, $3 );
    }
  | relational_expr GE concat_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::GreaterThanOrEqual, $1, $3 );
    }
  | relational_expr IS concat_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::Is, $1, $3 );
    }
;

concat_expr:
    additive_expr
    {
      $$ = $1;
    }
  | concat_expr '&' additive_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::Concat, $1, $3 );
    }
;

additive_expr:
    modulo_expr
    {
      $$ = $1;
    }
  | additive_expr '+' modulo_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::Plus, $1, $3 );
    }
  | additive_expr '-' modulo_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::Minus, $1, $3 );
    }
;

modulo_expr:
    integer_divide_expr
    {
      $$ = $1;
    }
  | modulo_expr MOD integer_divide_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::Modulo, $1, $3 );
    }
;

integer_divide_expr:
    multiplicative_expr
    {
      $$ = $1;
    }
  | integer_divide_expr '\\' multiplicative_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::IntegerDivide, $1, $3 );
    }
;

multiplicative_expr:
    unary_expr
    {
      $$ = $1;
    }
  | multiplicative_expr '*' unary_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::Multiply, $1, $3 );
    }
  | multiplicative_expr '/' unary_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::Divide, $1, $3 );
    }
;

unary_expr:
    exponent_expr
    {
      $$ = $1;
    }
  | '-' unary_expr %prec UMINUS
    {
      $$ = new QgsVbScriptUnaryOperatorExpression( QgsVbScriptUnaryOperatorExpression::UnaryOperator::Minus, $2 );
    }
  | '+' unary_expr %prec UPLUS
    {
      $$ = new QgsVbScriptUnaryOperatorExpression( QgsVbScriptUnaryOperatorExpression::UnaryOperator::Plus, $2 );
    }
;

exponent_expr:
    postfix_expr
    {
      $$ = $1;
    }
  | postfix_expr '^' exponent_expr
    {
      $$ = new QgsVbScriptBinaryOperatorExpression( QgsVbScriptBinaryOperatorExpression::BinaryOperator::Exponent, $1, $3 );
    }
;

postfix_expr:
    primary_expr
    {
      $$ = $1;
    }
  | postfix_expr '.' IDENTIFIER
    {
      $$ = new QgsVbScriptMemberAccessExpression( $1, *$3 );
      delete $3;
    }
  | postfix_expr '(' opt_argument_list ')'
    {
      $$ = new QgsVbScriptFunctionCallExpression( $1, *$3 );
      delete $3;
    }
;

primary_expr:
    NUMBER_INT
    {
      $$ = new QgsVbScriptLiteralExpression( QVariant( $1 ) );
    }
  | NUMBER_INT64
    {
      $$ = new QgsVbScriptLiteralExpression( QVariant( $1 ) );
    }
  | NUMBER_FLOAT
    {
      $$ = new QgsVbScriptLiteralExpression( QVariant( $1 ) );
    }
  | STRING_LITERAL
    {
      $$ = new QgsVbScriptLiteralExpression( QVariant( *$1 ) );
      delete $1;
    }
  | BOOLEAN_LITERAL
    {
      $$ = new QgsVbScriptLiteralExpression( QVariant( $1 ) );
    }
  | NULL_LITERAL
    {
      $$ = new QgsVbScriptLiteralExpression( QVariant() );
    }
  | EMPTY_LITERAL
    {
      $$ = new QgsVbScriptLiteralExpression( QVariant() );
    }
  | NOTHING_LITERAL
    {
      $$ = new QgsVbScriptLiteralExpression( QVariant() );
    }
  | IDENTIFIER
    {
      $$ = new QgsVbScriptVariableReferenceExpression( *$1 );
      delete $1;
    }
  | FIELD_REFERENCE
    {
      $$ = new QgsVbScriptFieldReferenceExpression( *$1 );
      delete $1;
    }
  | ARRAY '(' opt_argument_list ')'
    {
      $$ = new QgsVbScriptArrayLiteralExpression( *$3 );
      delete $3;
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
      $$ = new QList<QgsVbScriptExpression *>();
    }
;

argument_list:
    argument_list ',' expr
    {
      $$ = $1;
      $$->append( $3 );
    }
  | expr
    {
      $$ = new QList<QgsVbScriptExpression *>();
      $$->append( $1 );
    }
;

%%

QgsVbScriptBlockStatement *parseVbScript( const QString &scriptText, QString &parserErrorMessage )
{
  vbscript_parser_context context;
  vbscript_lex_init( &context.flexScanner );
  vbscript__scan_string( scriptText.toUtf8().constData(), context.flexScanner );

  const int result = vbscript_parse( &context );
  vbscript_lex_destroy( context.flexScanner );

  if ( result == 0 && context.rootBlock )
  {
    return context.rootBlock;
  }

  parserErrorMessage = context.errorMessage;
  delete context.rootBlock;
  return nullptr;
}

void vbscript_error( vbscript_parser_context *parserContext, const QString &errorMessage )
{
  parserContext->errorMessage = errorMessage;
}