/***************************************************************************
                            qgscql2parser.yy
                          --------------------
    begin                : Jun 2026
    copyright            : (C) 2026 by Juergen E. Fischer
    email                : jef at norbit dot de

    adapted from src/core/qgsexpressionparser.yy
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
#include "expression/qgsexpression.h"
#include "expression/qgsexpressionnode.h"
#include "expression/qgsexpressionnodeimpl.h"
#include "expression/qgsexpressionfunction.h"

#ifdef _MSC_VER
#  pragma warning( disable: 4065 )  // switch statement contains 'default' but no 'case' labels
#  pragma warning( disable: 4702 )  // unreachable code
#endif

// don't redeclare malloc/free
#define YYINCLUDED_STDLIB_H 1

// maximum number of errors encountered before parser aborts
#define MAX_ERRORS 10

struct expression_parser_context;
#include "qgscql2parser.hpp"

//! from lexer
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
extern int cql2_lex_init(yyscan_t* scanner);
extern int cql2_lex_destroy(yyscan_t scanner);
extern int cql2_lex(YYSTYPE* yylval_param, YYLTYPE* yyloc, yyscan_t yyscanner);
extern YY_BUFFER_STATE cql2__scan_string(const char* buffer, yyscan_t scanner);

/** returns parsed tree, otherwise returns nullptr and sets parserErrorMsg
    (interface function to be called from QgsExpression)
  */
QgsExpressionNode* parseCql2Expression(const QString &str, QString &filterCrs, QString &layerCrs, QString &parserErrorMsg, QList<QgsExpression::ParserError> &parserErrors);

/** error handler for bison */
void cql2_error(YYLTYPE* yyloc, expression_parser_context *parser_ctx, const char* msg);

struct expression_parser_context
{
  // lexer context
  yyscan_t flex_scanner;

  // List of all errors.
  QList<QgsExpression::ParserError> parserErrors;
  QString errorMsg;
  // Current parser error.
  QgsExpression::ParserError::ParserErrorType currentErrorType = QgsExpression::ParserError::Unknown;
  // root node of the expression
  QgsExpressionNode* rootNode;

  QString layerCrs;
  QString filterCrs;
};

#define scanner parser_ctx->flex_scanner

// we want verbose error messages
#define YYERROR_VERBOSE 1

#define BINOP(x, y, z)  new QgsExpressionNodeBinaryOperator(x, y, z)

void addParserLocation(YYLTYPE* yyloc, QgsExpressionNode *node)
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
%parse-param {expression_parser_context *parser_ctx}

%union
{
  QgsExpressionNode* node;
  QgsExpressionNode::NodeList* nodelist;
  double numberFloat;
  int    numberInt;
  qlonglong numberInt64;
  bool   boolVal;
  QString *text;
  QgsExpressionNodeBinaryOperator::BinaryOperator b_op;
  QgsExpressionNodeUnaryOperator::UnaryOperator u_op;
}

%start root

//
// token definitions
//

// operator tokens
%token <b_op> OR AND EQ NE LE GE LT GT LIKE IS PLUS MINUS MUL DIV INTDIV MOD CONCAT POW
%token <u_op> NOT
%token IN BETWEEN

// literals
%token <numberFloat> NUMBER_FLOAT
%token <numberInt> NUMBER_INT
%token <numberInt64> NUMBER_INT64
%token <boolVal> BOOLEAN
%token NULLVALUE

%token <text> STRING QUOTED_COLUMN_REF NAME SPATIAL_FUNCTION WKT_LITERAL DATE_LITERAL TIMESTAMP_LITERAL INTERVAL_LITERAL BBOX_LITERAL VARIABLE

%token COMMA

%token Unknown_CHARACTER

//
// definition of non-terminal types
//

%type <node> expression
%type <node> expression_non_logical
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
%left BETWEEN
%left EQ NE LE GE LT GT LIKE IS IN
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

/* We have to separate expression from expression_non_logical to avoid */
/* grammar ambiguities with the AND of the "BETWEEN x AND y" and the */
/* logical binary AND */

expression:

      expression_non_logical             { $$ = $1; }
    | expression AND expression          { $$ = BINOP($2, $1, $3); }
    | expression OR expression           { $$ = BINOP($2, $1, $3); }
    | expression EQ expression           { $$ = BINOP($2, $1, $3); }
    | expression NE expression           { $$ = BINOP($2, $1, $3); }
    | expression LE expression           { $$ = BINOP($2, $1, $3); }
    | expression GE expression           { $$ = BINOP($2, $1, $3); }
    | expression LT expression           { $$ = BINOP($2, $1, $3); }
    | expression GT expression           { $$ = BINOP($2, $1, $3); }
    | expression LIKE expression         { $$ = BINOP($2, $1, $3); }
    | expression IS expression           { $$ = BINOP($2, $1, $3); }
    | NOT expression                     { $$ = new QgsExpressionNodeUnaryOperator($1, $2); }
    | expression IN '(' exp_list ')'     { $$ = new QgsExpressionNodeInOperator($1, $4, false);  }
    | expression NOT IN '(' exp_list ')' { $$ = new QgsExpressionNodeInOperator($1, $5, true); }

    | expression BETWEEN expression_non_logical AND expression_non_logical   { $$ = new QgsExpressionNodeBetweenOperator($1, $3, $5, false ); }
    | expression NOT BETWEEN expression_non_logical AND expression_non_logical   { $$ = new QgsExpressionNodeBetweenOperator($1, $4, $6, true); }
    ;


expression_non_logical:

      expression_non_logical PLUS expression_non_logical      { $$ = BINOP($2, $1, $3); }
    | expression_non_logical MINUS expression_non_logical     { $$ = BINOP($2, $1, $3); }
    | expression_non_logical MUL expression_non_logical       { $$ = BINOP($2, $1, $3); }
    | expression_non_logical INTDIV expression_non_logical    { $$ = BINOP($2, $1, $3); }
    | expression_non_logical DIV expression_non_logical       { $$ = BINOP($2, $1, $3); }
    | expression_non_logical MOD expression_non_logical       { $$ = BINOP($2, $1, $3); }
    | expression_non_logical POW expression_non_logical       { $$ = BINOP($2, $1, $3); }
    | '(' expression ')'                                      { $$ = $2; }
    | SPATIAL_FUNCTION '(' expression COMMA expression ')'
        {
          QString functionName = *$1;
          delete $1;

          if( functionName.compare( u"S_CONTAINS"_s, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
          {
            functionName = u"contains"_s;
          }
          else if( functionName.compare( u"S_CROSSES"_s, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
          {
            functionName = u"crosses"_s;
          }
          else if( functionName.compare( u"S_DISJOINT"_s, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
          {
            functionName = u"disjoint"_s;
          }
          else if( functionName.compare( u"S_EQUALS"_s, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
          {
            functionName = u"equals"_s;
          }
          else if( functionName.compare( u"S_INTERSECTS"_s, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
          {
            functionName = u"intersects"_s;
          }
          else if( functionName.compare( u"S_OVERLAPS"_s, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
          {
            functionName = u"overlaps"_s;
          }
          else if( functionName.compare( u"S_TOUCHES"_s, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
          {
            functionName = u"touches"_s;
          }
          else if( functionName.compare( u"S_WITHIN"_s, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
          {
            functionName = u"within"_s;
          }

          const int fnIndex = QgsExpression::functionIndex(functionName);
          if (fnIndex == -1)
          {
            QgsExpression::ParserError::ParserErrorType errorType = QgsExpression::ParserError::FunctionUnknown;
            parser_ctx->currentErrorType = errorType;
            cql2_error(&yyloc, parser_ctx, QObject::tr( "Function %1 is not known" ).arg( functionName ).toUtf8().constData() );
            delete $3;
            delete $5;
            YYERROR;
          }

          QgsExpressionNode::NodeList *nodeList = new QgsExpressionNode::NodeList();
          nodeList->append($3);
          nodeList->append($5);

          QString paramError;
          if ( !QgsExpressionNodeFunction::validateParams( fnIndex, nodeList, paramError ) )
          {
             QgsExpression::ParserError::ParserErrorType errorType = QgsExpression::ParserError::FunctionInvalidParams;
             parser_ctx->currentErrorType = errorType;
             cql2_error( &yyloc, parser_ctx, paramError.toLocal8Bit().constData() );
             delete nodeList;
             YYERROR;
          }

          $$ = new QgsExpressionNodeFunction(fnIndex, nodeList);
          addParserLocation(&@1, $$);
        }
    | NAME '(' exp_list ')'
        {
          const QString expressionFunctionName = *$1;
          delete $1;

          const int fnIndex = QgsExpression::functionIndex(expressionFunctionName);
          if (fnIndex == -1)
          {
            QgsExpression::ParserError::ParserErrorType errorType = QgsExpression::ParserError::FunctionUnknown;
            parser_ctx->currentErrorType = errorType;
            cql2_error(&yyloc, parser_ctx, QObject::tr( "Function %1 is not known" ).arg( expressionFunctionName ).toUtf8().constData() );
            delete $3;
            YYERROR;
          }
          QgsExpressionFunction* func = QgsExpression::Functions()[fnIndex];
          QString paramError;
          if ( !QgsExpressionNodeFunction::validateParams( fnIndex, $3, paramError ) )
          {
            QgsExpression::ParserError::ParserErrorType errorType = QgsExpression::ParserError::FunctionInvalidParams;
            parser_ctx->currentErrorType = errorType;
            cql2_error( &yyloc, parser_ctx, paramError.toLocal8Bit().constData() );
            delete $3;
            YYERROR;
          }
          if ( func->params() != -1
               && !( func->params() >= $3->count()
               && func->minParams() <= $3->count() ) )
          {
            QgsExpression::ParserError::ParserErrorType errorType = QgsExpression::ParserError::FunctionWrongArgs;
            parser_ctx->currentErrorType = errorType;
            QString expectedMessage;
            if ( func->params() == func->minParams() )
            {
              expectedMessage = QObject::tr( "Expected %1 but got %2." ).arg( QString::number( func->params() ), QString::number( $3->count() ) );
            }
            else
            {
              expectedMessage = QObject::tr( "Expected between %1 and %2 parameters but %3 were provided." ).arg( QString::number( func->minParams() ), QString::number( func->params() ), QString::number( $3->count() ) );
            }
            cql2_error(&yyloc, parser_ctx, QObject::tr( "%1 function is called with wrong number of arguments. %2" ).arg( func->name(), expectedMessage ).toUtf8().constData() );
            delete $3;
            YYERROR;
          }

          $$ = new QgsExpressionNodeFunction(fnIndex, $3);
          addParserLocation(&@1, $$);
        }

    | NAME '(' ')'
        {
          const QString expressionFunctionName = *$1;
          const int fnIndex = QgsExpression::functionIndex(expressionFunctionName);
          delete $1;
          if (fnIndex == -1)
          {
            QgsExpression::ParserError::ParserErrorType errorType = QgsExpression::ParserError::FunctionUnknown;
            parser_ctx->currentErrorType = errorType;
            cql2_error(&yyloc, parser_ctx, QObject::tr( "Function %1 is not known" ).arg( expressionFunctionName ).toUtf8().constData() );
            YYERROR;
          }
          QgsExpressionFunction* func = QgsExpression::Functions()[fnIndex];
          // 0 parameters is expected, -1 parameters means leave it to the
          // implementation
          if ( func->minParams() > 0 )
          {
            QgsExpression::ParserError::ParserErrorType errorType = QgsExpression::ParserError::FunctionWrongArgs;
            parser_ctx->currentErrorType = errorType;
            cql2_error(&yyloc, parser_ctx, QObject::tr( "%1 function is called with wrong number of arguments" ).arg( func->name() ).toLocal8Bit().constData() );
            YYERROR;
          }
          $$ = new QgsExpressionNodeFunction(fnIndex, new QgsExpressionNode::NodeList());
          addParserLocation(&@1, $$);
        }

    | PLUS expression_non_logical %prec UMINUS { $$ = $2; }
    | MINUS expression_non_logical %prec UMINUS { $$ = new QgsExpressionNodeUnaryOperator( QgsExpressionNodeUnaryOperator::uoMinus, $2); }

    // columns
    | NAME                        { $$ = new QgsExpressionNodeColumnRef( *$1 ); delete $1; }
    | QUOTED_COLUMN_REF           { $$ = new QgsExpressionNodeColumnRef( *$1 ); delete $1; }

    //  literals
    | NUMBER_FLOAT                { $$ = new QgsExpressionNodeLiteral( QVariant($1) ); }
    | NUMBER_INT                  { $$ = new QgsExpressionNodeLiteral( QVariant($1) ); }
    | NUMBER_INT64                { $$ = new QgsExpressionNodeLiteral( QVariant($1) ); }
    | BOOLEAN                     { $$ = new QgsExpressionNodeLiteral( QVariant($1) ); }
    | STRING                      { $$ = new QgsExpressionNodeLiteral( QVariant(*$1) ); delete $1; }
    | NULLVALUE                   { $$ = new QgsExpressionNodeLiteral( QVariant() ); }
    | WKT_LITERAL                 {
        QgsExpressionNodeLiteral *arg = new QgsExpressionNodeLiteral( QVariant(*$1) );
        delete $1;

        QgsExpressionNode::NodeList *args = new QgsExpressionNode::NodeList();
        args->append( arg );

        $$ = new QgsExpressionNodeFunction( QgsExpression::functionIndex("geom_from_wkt"), args );

        if( parser_ctx->layerCrs != parser_ctx->filterCrs )
        {
           QgsExpressionNode::NodeList *args = new QgsExpressionNode::NodeList();
           args->append($$);
           args->append( new QgsExpressionNodeLiteral( QVariant(parser_ctx->filterCrs) ) );
           args->append( new QgsExpressionNodeLiteral( QVariant(parser_ctx->layerCrs) ) );
           $$ = new QgsExpressionNodeFunction( QgsExpression::functionIndex("transform"), args );
        }
    }
    ;

exp_list:
      exp_list COMMA expression   { $$ = $1; $1->append($3); }
    | expression                  { $$ = new QgsExpressionNode::NodeList(); $$->append($1); }
   ;

%%


// returns parsed tree, otherwise returns nullptr and sets parserErrorMsg
QgsExpressionNode *parseCql2Expression(const QString& str, QString &filterCrs, QString &layerCrs, QString& parserErrorMsg, QList<QgsExpression::ParserError> &parserErrors)
{
  expression_parser_context ctx;
  ctx.rootNode = 0;

  ctx.layerCrs = layerCrs;
  ctx.filterCrs = filterCrs;

  cql2_lex_init(&ctx.flex_scanner);
  cql2__scan_string(str.toUtf8().constData(), ctx.flex_scanner);
  int res = cql2_parse(&ctx);
  cql2_lex_destroy(ctx.flex_scanner);

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


void cql2_error(YYLTYPE *yyloc, expression_parser_context *parser_ctx, const char *msg)
{
  QgsExpression::ParserError error = QgsExpression::ParserError();
  error.firstColumn = yyloc->first_column;
  error.firstLine = yyloc->first_line;
  error.lastColumn = yyloc->last_column;
  error.lastLine = yyloc->last_line;
  error.errorMsg = msg;
  error.errorType = parser_ctx->currentErrorType;

  parser_ctx->parserErrors.append(error);
  // Reset the current error type for the next error.
  parser_ctx->currentErrorType = QgsExpression::ParserError::Unknown;

  parser_ctx->errorMsg = parser_ctx->errorMsg + "\n" + msg;
}
