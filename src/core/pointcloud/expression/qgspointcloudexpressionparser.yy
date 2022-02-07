/***************************************************************************
                      qgspointcouldexpressionparser.yy
                      --------------------------------
    begin                : January 2022
    copyright            : (C) 2022 Stefanos Natsis
    email                : uclaros at gmail dot com
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
#include "qgslogger.h"
#include "qgspointcloudexpression.h"
#include "qgspointcloudexpressionnode.h"
#include "qgspointcloudexpressionnodeimpl.h"

#ifdef _MSC_VER
#  pragma warning( disable: 4065 )  // switch statement contains 'default' but no 'case' labels
#  pragma warning( disable: 4702 )  // unreachable code
#endif

// don't redeclare malloc/free
#define YYINCLUDED_STDLIB_H 1

// maximum number of errors encountered before parser aborts
#define MAX_ERRORS 10

struct expression_parser_context;
#include "qgspointcloudexpressionparser.hpp"

//! from lexer
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
extern int pointcloud_lex_init(yyscan_t* scanner);
extern int pointcloud_lex_destroy(yyscan_t scanner);
extern int pointcloud_lex(YYSTYPE* yylval_param, YYLTYPE* yyloc, yyscan_t yyscanner);
extern YY_BUFFER_STATE pointcloud__scan_string(const char* buffer, yyscan_t scanner);

/** returns parsed tree, otherwise returns nullptr and sets parserErrorMsg
    (interface function to be called from QgsPointCloudExpression)
  */
QgsPointCloudExpressionNode* parseExpression(const QString& str, QString& parserErrorMsg, QList<QgsPointCloudExpression::ParserError>& parserError);

/** error handler for bison */
void pointcloud_error(YYLTYPE* yyloc, expression_parser_context* parser_ctx, const char* msg);

struct expression_parser_context
{
  // lexer context
  yyscan_t flex_scanner;

  // List of all errors.
  QList<QgsPointCloudExpression::ParserError> parserErrors;
  QString errorMsg;
  // Current parser error.
  QgsPointCloudExpression::ParserError::ParserErrorType currentErrorType = QgsPointCloudExpression::ParserError::Unknown;
  // root node of the expression
  QgsPointCloudExpressionNode* rootNode;
};

#define scanner parser_ctx->flex_scanner

// we want verbose error messages
#define YYERROR_VERBOSE 1

#define BINOP(x, y, z)  new QgsPointCloudExpressionNodeBinaryOperator(x, y, z)

void addParserLocation(YYLTYPE* yyloc, QgsPointCloudExpressionNode *node)
{
  node->parserFirstLine = yyloc->first_line;
  node->parserFirstColumn = yyloc->first_column;
  node->parserLastLine = yyloc->last_line;
  node->parserLastColumn = yyloc->last_column;
}

%}

// make the parser reentrant
%locations
%define api.pure
%lex-param {void * scanner}
%parse-param {expression_parser_context* parser_ctx}

%union
{
  QgsPointCloudExpressionNode* node;
  QgsPointCloudExpressionNode::NodeList* nodelist;
  double numberFloat;
  int    numberInt;
  qlonglong numberInt64;
  bool   boolVal;
  QString* text;
  QgsPointCloudExpressionNodeBinaryOperator::BinaryOperator b_op;
  QgsPointCloudExpressionNodeUnaryOperator::UnaryOperator u_op;
}

%start root


//
// token definitions
//

// operator tokens
%token <b_op> OR AND EQ NE LE GE LT GT PLUS MINUS MUL DIV INTDIV MOD POW
%token <u_op> NOT
%token IN

// literals
%token <numberFloat> NUMBER_FLOAT
%token <numberInt> NUMBER_INT
%token <numberInt64> NUMBER_INT64
%token <boolVal> BOOLEAN
%token NULLVALUE


%token <text> STRING QUOTED_ATTRIBUTE_REF NAME SPECIAL_COL

%token COMMA

%token Unknown_CHARACTER

//
// definition of non-terminal types
//

%type <node> expression
%type <nodelist> exp_list

// debugging
%define parse.error verbose

//
// operator precedence
//

// left associativity means that 1+2+3 translates to (1+2)+3
// the order of operators here determines their precedence

%left OR
%left AND
%right NOT
%left EQ NE LE GE LT GT IN
%left PLUS MINUS
%left MUL DIV INTDIV MOD
%right POW

%right UMINUS  // fictitious symbol (for unary minus)

%left COMMA

%destructor { delete $$; } <node>
%destructor { delete $$; } <nodelist>
%destructor { delete $$; } <text>

%%

root: expression { parser_ctx->rootNode = $1; }
    | error expression
        {
            delete $2;
            if ( parser_ctx->parserErrors.count() < MAX_ERRORS )
              yyerrok;
            else
              YYABORT;
        }
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
    | expression PLUS expression      { $$ = BINOP($2, $1, $3); }
    | expression MINUS expression     { $$ = BINOP($2, $1, $3); }
    | expression MUL expression       { $$ = BINOP($2, $1, $3); }
    | expression INTDIV expression    { $$ = BINOP($2, $1, $3); }
    | expression DIV expression       { $$ = BINOP($2, $1, $3); }
    | expression MOD expression       { $$ = BINOP($2, $1, $3); }
    | expression POW expression       { $$ = BINOP($2, $1, $3); }
    | NOT expression                  { $$ = new QgsPointCloudExpressionNodeUnaryOperator($1, $2); }
    | '(' expression ')'              { $$ = $2; }
    | expression IN '(' exp_list ')'     { $$ = new QgsPointCloudExpressionNodeInOperator($1, $4, false);  }
    | expression NOT IN '(' exp_list ')' { $$ = new QgsPointCloudExpressionNodeInOperator($1, $5, true); }

    | PLUS expression %prec UMINUS { $$ = $2; }
    | MINUS expression %prec UMINUS { $$ = new QgsPointCloudExpressionNodeUnaryOperator( QgsPointCloudExpressionNodeUnaryOperator::uoMinus, $2); }

    // columns
    | NAME                  { $$ = new QgsPointCloudExpressionNodeAttributeRef( *$1 ); delete $1; }
    | QUOTED_ATTRIBUTE_REF                  { $$ = new QgsPointCloudExpressionNodeAttributeRef( *$1 ); delete $1; }

    //  literals
    | NUMBER_FLOAT                { $$ = new QgsPointCloudExpressionNodeLiteral( $1 ); }
    | NUMBER_INT                  { $$ = new QgsPointCloudExpressionNodeLiteral( $1 ); }
    | NUMBER_INT64                { $$ = new QgsPointCloudExpressionNodeLiteral( $1 ); }
;

exp_list:
      exp_list COMMA expression
       {
         $$ = $1; $1->append($3);
       }
    | expression              { $$ = new QgsPointCloudExpressionNode::NodeList(); $$->append($1); }
   ;

%%


// returns parsed tree, otherwise returns nullptr and sets parserErrorMsg
QgsPointCloudExpressionNode* parseExpression(const QString& str, QString& parserErrorMsg, QList<QgsPointCloudExpression::ParserError> &parserErrors)
{
  expression_parser_context ctx;
  ctx.rootNode = 0;

  pointcloud_lex_init(&ctx.flex_scanner);
  pointcloud__scan_string(str.toUtf8().constData(), ctx.flex_scanner);
  int res = pointcloud_parse(&ctx);
  pointcloud_lex_destroy(ctx.flex_scanner);

  // list should be empty when parsing was OK
  if (res == 0 && ctx.parserErrors.count() == 0) // success?
  {
    return ctx.rootNode;
  }
  else // error?
  {
    parserErrorMsg = ctx.errorMsg;
    parserErrors = ctx.parserErrors;
    delete ctx.rootNode;
    return nullptr;
  }
}


void pointcloud_error(YYLTYPE* yyloc,expression_parser_context* parser_ctx, const char* msg)
{
  QgsPointCloudExpression::ParserError error = QgsPointCloudExpression::ParserError();
  error.firstColumn = yyloc->first_column;
  error.firstLine = yyloc->first_line;
  error.lastColumn = yyloc->last_column;
  error.lastLine = yyloc->last_line;
  error.errorMsg = msg;
  error.errorType = parser_ctx->currentErrorType;
  parser_ctx->parserErrors.append(error);
  // Reset the current error type for the next error.
  parser_ctx->currentErrorType = QgsPointCloudExpression::ParserError::Unknown;

  parser_ctx->errorMsg = parser_ctx->errorMsg + "\n" + msg;
}

