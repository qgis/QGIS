/***************************************************************************
                         qgssqlexpressionparser.yy
                          --------------------
    begin                : April 2016
    copyright            : (C) 2011 by Martin Dobias
    copyright            : (C) 2016 by Even Rouault
    copyright            : (C) 2010 by Frank Warmerdam (partly derived from GDAL ogr/swq_parser.y)
    email                : even.rouault at spatialys.com
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
#include "qgssqlstatement.h"

#ifdef _MSC_VER
#  pragma warning( disable: 4065 )  // switch statement contains 'default' but no 'case' labels
#  pragma warning( disable: 4702 )  // unreachable code
#endif

// don't redeclare malloc/free
#define YYINCLUDED_STDLIB_H 1

struct sqlstatement_parser_context;
#include "qgssqlstatementparser.hpp"

//! from lexer
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
extern int sqlstatement_lex_init(yyscan_t* scanner);
extern int sqlstatement_lex_destroy(yyscan_t scanner);
extern int sqlstatement_lex(YYSTYPE* yylval_param, yyscan_t yyscanner);
extern YY_BUFFER_STATE sqlstatement__scan_string(const char* buffer, yyscan_t scanner);

/** returns parsed tree, otherwise returns nullptr and sets parserErrorMsg
    (interface function to be called from QgsSQLStatement)
  */
QgsSQLStatement::Node* parse(const QString& str, QString& parserErrorMsg);

/** error handler for bison */
void sqlstatement_error(sqlstatement_parser_context* parser_ctx, const QString& errorMsg);

struct sqlstatement_parser_context
{
  // lexer context
  yyscan_t flex_scanner;

  // varible where the parser error will be stored
  QString errorMsg;
  // root node of the sqlstatement
  QgsSQLStatement::NodeSelect* rootNode;

  QgsSQLStatement::Node* whereExp;

  QList<QgsSQLStatement::NodeJoin*> joinList;

  QList<QgsSQLStatement::NodeColumnSorted*> orderByList;

  sqlstatement_parser_context() : rootNode( nullptr ), whereExp( nullptr ) {}

  void setWhere( QgsSQLStatement::Node* whereExp ) { this->whereExp = whereExp; }

  void setJoins( QList<QgsSQLStatement::NodeJoin*> joinList ) { this->joinList = joinList; }

  void setOrderBy( QList<QgsSQLStatement::NodeColumnSorted*> orderByList ) { this->orderByList = orderByList; }
};

#define scanner parser_ctx->flex_scanner

// we want verbose error messages
#define YYERROR_VERBOSE 1

#define BINOP(x, y, z)  new QgsSQLStatement::NodeBinaryOperator(x, y, z)

%}

// make the parser reentrant
%define api.pure
%lex-param {void * scanner}
%parse-param {sqlstatement_parser_context* parser_ctx}

%name-prefix "sqlstatement_"

%union
{
  QgsSQLStatement::Node* node;
  QgsSQLStatement::NodeColumnRef* nodecolumnref;
  QgsSQLStatement::NodeSelectedColumn* nodeselectedcolumn;
  QgsSQLStatement::NodeSelect* nodeselect;
  QgsSQLStatement::NodeList* nodelist;
  QgsSQLStatement::NodeJoin* nodejoin;
  QgsSQLStatement::NodeTableDef* nodetabledef;
  QgsSQLStatement::NodeColumnSorted* nodecolumnsorted;
  QList<QgsSQLStatement::NodeColumnSorted*>* columnsortedlist;
  QList<QgsSQLStatement::NodeJoin*>* joinlist;
  QList<QgsSQLStatement::NodeTableDef*>* tablelist;
  QList<QgsSQLStatement::NodeSelectedColumn*>* selectedcolumnlist;
  double numberFloat;
  int    numberInt;
  qlonglong numberInt64;
  bool   boolVal;
  QString* text;
  QgsSQLStatement::BinaryOperator b_op;
  QgsSQLStatement::UnaryOperator u_op;
  QgsSQLStatement::JoinType jointype;
  QList<QString>* usinglist;
}

%start root


//
// token definitions
//

// operator tokens
%token <b_op> OR AND EQ NE LE GE LT GT LIKE IS PLUS MINUS MUL_OR_STAR DIV INTDIV MOD CONCAT POW
%token <u_op> NOT
%token IN BETWEEN

%token SELECT ALL DISTINCT CAST AS JOIN FROM ON USING WHERE ORDER BY ASC DESC LEFT RIGHT INNER OUTER CROSS FULL NATURAL UNION

// literals
%token <numberFloat> NUMBER_FLOAT
%token <numberInt> NUMBER_INT
%token <numberInt64> NUMBER_INT64
%token <boolVal> BOOLEAN
%token NULLVALUE

%token <text> STRING IDENTIFIER

%token COMMA

%token Unknown_CHARACTER

//
// definition of non-terminal types
//

%type <node> expr
%type <node> expr_non_logical
%type <nodecolumnref> column_name
%type <nodecolumnsorted> sort_spec
%type <nodeselectedcolumn> selected_column
%type <nodeselect> select_statement
%type <nodejoin> join
%type <nodetabledef> table_def
%type <nodelist> expr_list
%type <selectedcolumnlist> selected_column_list
%type <joinlist> join_list
%type <tablelist> table_list
%type <columnsortedlist> sort_spec_list
%type <usinglist> using_list;
%type <text> as_clause
%type <jointype> join_qualifier
%type <boolVal> select_type;

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
%left BETWEEN
%left EQ NE LE GE LT GT LIKE IS IN
%left PLUS MINUS
%left MUL_OR_STAR DIV INTDIV MOD
%right POW
%left CONCAT

%right UMINUS  // fictitious symbol (for unary minus)

%left COMMA

%destructor { delete $$; } <node>
%destructor { delete $$; } <nodelist>
%destructor { delete $$; } <nodecolumnref>
%destructor { delete $$; } <text>
%destructor { delete $$; } <nodetabledef>
%destructor { delete $$; } <nodeselect>
%destructor { delete $$; } <nodeselectedcolumn>
%destructor { delete $$; } <nodecolumnsorted>
%destructor { delete $$; } <nodejoin>
%destructor { delete $$; } <usinglist>
%destructor { qDeleteAll(*$$); delete $$; } <selectedcolumnlist>
%destructor { qDeleteAll(*$$); delete $$; } <columnsortedlist>
%destructor { qDeleteAll(*$$); delete $$; } <joinlist>
%destructor { qDeleteAll(*$$); delete $$; } <tablelist>

%%

root: select_statement { parser_ctx->rootNode = $1; }
    ;

/* We have to separate expr from expr_non_logical to avoid */
/* grammar ambiguities with the AND of the "BETWEEN x AND y" and the */
/* logical binary AND */
expr:
      expr_non_logical
        {
            $$ = $1;
        }

    | expr AND expr       { $$ = BINOP($2, $1, $3); }
    | expr OR expr        { $$ = BINOP($2, $1, $3); }
    | expr EQ expr        { $$ = BINOP($2, $1, $3); }
    | expr NE expr        { $$ = BINOP($2, $1, $3); }
    | expr LE expr        { $$ = BINOP($2, $1, $3); }
    | expr GE expr        { $$ = BINOP($2, $1, $3); }
    | expr LT expr        { $$ = BINOP($2, $1, $3); }
    | expr GT expr        { $$ = BINOP($2, $1, $3); }
    | expr LIKE expr      { $$ = BINOP($2, $1, $3); }
    | expr IS expr        { $$ = BINOP($2, $1, $3); }
    | NOT expr                  { $$ = new QgsSQLStatement::NodeUnaryOperator($1, $2); }

    | expr IN '(' expr_list ')'     { $$ = new QgsSQLStatement::NodeInOperator($1, $4, false);  }
    | expr NOT IN '(' expr_list ')' { $$ = new QgsSQLStatement::NodeInOperator($1, $5, true); }

    | expr BETWEEN expr_non_logical AND expr_non_logical     { $$ = new QgsSQLStatement::NodeBetweenOperator($1, $3, $5, false);  }
    | expr NOT BETWEEN expr_non_logical AND expr_non_logical     { $$ = new QgsSQLStatement::NodeBetweenOperator($1, $4, $6, true);  }

;

column_name:
      IDENTIFIER
        {
            $$ = new QgsSQLStatement::NodeColumnRef( *$1, false );
            delete $1;
        }

    | IDENTIFIER '.' IDENTIFIER
        {
            $$ = new QgsSQLStatement::NodeColumnRef( *$1, *$3, false );
            delete $1;
            delete $3;
        }
;

expr_list:
      expr_list COMMA expr
       {
          $$ = $1; $1->append($3);
       }
    | expr              { $$ = new QgsSQLStatement::NodeList(); $$->append($1); }
;

expr_non_logical:

    //  literals
      NUMBER_FLOAT                { $$ = new QgsSQLStatement::NodeLiteral( QVariant($1) ); }
    | NUMBER_INT                  { $$ = new QgsSQLStatement::NodeLiteral( QVariant($1) ); }
    | NUMBER_INT64                { $$ = new QgsSQLStatement::NodeLiteral( QVariant($1) ); }
    | BOOLEAN                     { $$ = new QgsSQLStatement::NodeLiteral( QVariant($1) ); }
    | STRING                      { $$ = new QgsSQLStatement::NodeLiteral( QVariant(*$1) ); delete $1; }
    | NULLVALUE                   { $$ = new QgsSQLStatement::NodeLiteral( QVariant() ); }

    | column_name
        {
            $$ = $1;
        }

    | '(' expr ')'              { $$ = $2; }

    | IDENTIFIER '(' expr_list ')'
        {
          $$ = new QgsSQLStatement::NodeFunction(*$1, $3);
          delete $1;
        }

    | IDENTIFIER '(' ')'
        {
          $$ = new QgsSQLStatement::NodeFunction(*$1, new QgsSQLStatement::NodeList());
          delete $1;
        }

    | expr_non_logical PLUS expr_non_logical      { $$ = BINOP($2, $1, $3); }
    | expr_non_logical MINUS expr_non_logical     { $$ = BINOP($2, $1, $3); }
    | expr_non_logical MUL_OR_STAR expr_non_logical { $$ = BINOP($2, $1, $3); }
    | expr_non_logical INTDIV expr_non_logical    { $$ = BINOP($2, $1, $3); }
    | expr_non_logical DIV expr_non_logical       { $$ = BINOP($2, $1, $3); }
    | expr_non_logical MOD expr_non_logical       { $$ = BINOP($2, $1, $3); }
    | expr_non_logical POW expr_non_logical       { $$ = BINOP($2, $1, $3); }
    | expr_non_logical CONCAT expr_non_logical    { $$ = BINOP($2, $1, $3); }
    | PLUS expr_non_logical %prec UMINUS { $$ = $2; }
    | MINUS expr_non_logical %prec UMINUS { $$ = new QgsSQLStatement::NodeUnaryOperator( QgsSQLStatement::uoMinus, $2); }

    | CAST '(' expr AS IDENTIFIER ')'
        {
            $$ = new QgsSQLStatement::NodeCast($3, *$5);
            delete $5;
        }

;

select_type:
      SELECT
        {
            $$ = false;
        }
    | SELECT ALL
        {
            $$ = false;
        }
    | SELECT DISTINCT
        {
            $$ = true;
        }
;

select_statement:
      select_type selected_column_list FROM table_list opt_joins opt_where opt_order_by
        {
            $$ = new QgsSQLStatement::NodeSelect(*$4, *$2, $1);
            delete $2;
            delete $4;
        }
;

selected_column_list:
      selected_column_list COMMA selected_column
       {
          $$ = $1; $1->append($3);
       }
    | selected_column
      { $$ = new QList<QgsSQLStatement::NodeSelectedColumn*>(); $$->append($1); }
;

selected_column:
    expr
        {
            $$ = new QgsSQLStatement::NodeSelectedColumn($1);
        }

    | expr as_clause
        {
            $$ = new QgsSQLStatement::NodeSelectedColumn($1);
            $$->setAlias(*$2);
            delete $2;
        }

    | MUL_OR_STAR
        {
            $$ = new QgsSQLStatement::NodeSelectedColumn( new QgsSQLStatement::NodeColumnRef("*", true) );
        }

    | IDENTIFIER '.' MUL_OR_STAR
        {
            $$ = new QgsSQLStatement::NodeSelectedColumn( new QgsSQLStatement::NodeColumnRef(*$1, "*", true) );
            delete $1;
        }

    | IDENTIFIER '(' MUL_OR_STAR ')'
        {
            // special case for COUNT(*), confirm it.
            if( $1->compare("COUNT", Qt::CaseInsensitive) != 0 )
            {
                sqlstatement_error(parser_ctx, QString( QObject::tr("Syntax Error with %1(*).") ).arg(*$1));
                delete $1;
                YYERROR;
            }
            delete $1;
            QgsSQLStatement::NodeList* nodeList = new QgsSQLStatement::NodeList();
            nodeList->append( new QgsSQLStatement::NodeColumnRef("*", true) );
            $$ = new QgsSQLStatement::NodeSelectedColumn( 
                    new QgsSQLStatement::NodeFunction( "COUNT", nodeList) );
        }

    | IDENTIFIER '(' MUL_OR_STAR ')' as_clause
        {
            // special case for COUNT(*), confirm it.
            if( $1->compare("COUNT", Qt::CaseInsensitive) != 0 )
            {
                sqlstatement_error(parser_ctx, QString( QObject::tr("Syntax Error with %1(*).") ).arg(*$1));
                delete $1;
                delete $5;
                YYERROR;
            }
            delete $1;
            QgsSQLStatement::NodeList* nodeList = new QgsSQLStatement::NodeList();
            nodeList->append( new QgsSQLStatement::NodeColumnRef("*", true) );
            $$ = new QgsSQLStatement::NodeSelectedColumn( 
                    new QgsSQLStatement::NodeFunction( "COUNT", nodeList) );
            $$->setAlias(*$5);
            delete $5;
        }

    | IDENTIFIER '(' DISTINCT column_name ')'
        {
            // special case for COUNT(DISTINCT x), confirm it.
            if( $1->compare("COUNT", Qt::CaseInsensitive) != 0 )
            {
                sqlstatement_error(parser_ctx, QObject::tr(
                        "DISTINCT keyword can only be used in COUNT() operator.") );
                delete $1;
                delete $4;
                YYERROR;
            }
            delete $1;
            QgsSQLStatement::NodeList* nodeList = new QgsSQLStatement::NodeList();
            $4->setDistinct();
            nodeList->append( $4 );
            $$ = new QgsSQLStatement::NodeSelectedColumn( 
                    new QgsSQLStatement::NodeFunction( "COUNT", nodeList) );
        }

    | IDENTIFIER '(' DISTINCT column_name ')' as_clause
        {
            // special case for COUNT(DISTINCT x), confirm it.
            if( $1->compare("COUNT", Qt::CaseInsensitive) != 0 )
            {
                sqlstatement_error(parser_ctx, QObject::tr(
                        "DISTINCT keyword can only be used in COUNT() operator.") );
                delete $1;
                delete $4;
                delete $6;
                YYERROR;
            }
            delete $1;
            QgsSQLStatement::NodeList* nodeList = new QgsSQLStatement::NodeList();
            $4->setDistinct();
            nodeList->append( $4 );
            $$ = new QgsSQLStatement::NodeSelectedColumn( 
                    new QgsSQLStatement::NodeFunction( "COUNT", nodeList) );
            $$->setAlias(*$6);
            delete $6;
        }
;

as_clause:
      AS IDENTIFIER
        {
            $$ = $2;
        }

    | IDENTIFIER
;

opt_where:
    | WHERE expr
        {
            parser_ctx->setWhere($2);
        }
;

join_qualifier:
      JOIN
        {
            $$ = QgsSQLStatement::jtDefault;
        }
    | LEFT JOIN
        {
            $$ = QgsSQLStatement::jtLeft;
        }
    | LEFT OUTER JOIN
        {
            $$ = QgsSQLStatement::jtLeftOuter;
        }
    | RIGHT JOIN
        {
            $$ = QgsSQLStatement::jtRight;
        }
    | RIGHT OUTER JOIN
        {
            $$ = QgsSQLStatement::jtRightOuter;
        }
    | FULL JOIN
        {
            $$ = QgsSQLStatement::jtFull;
        }
    | CROSS JOIN
        {
            $$ = QgsSQLStatement::jtCross;
        }
    | INNER JOIN
        {
            $$ = QgsSQLStatement::jtInner;
        }
;

join:
      join_qualifier table_def ON expr
        {
            $$ = new QgsSQLStatement::NodeJoin($2, $4, $1);
        }
     | join_qualifier table_def USING '(' using_list ')'
        {
            $$ = new QgsSQLStatement::NodeJoin($2, *$5, $1);
            delete $5;
        }
;

using_list:
      IDENTIFIER
        {
          $$ = new QList<QString>(); $$->push_back(*$1);
          delete $1;
        }
    | using_list COMMA IDENTIFIER
        {
          $$ = $1; $1->push_back(*$3);
          delete $3;
        }
;

join_list:
      join
        {
          $$ = new QList<QgsSQLStatement::NodeJoin*>(); $$->push_back($1);
        }
    | join_list join
        {
          $$ = $1; $1->push_back($2);
        }
;

opt_joins:
    | join_list
        {
            parser_ctx->setJoins( *$1 );
            delete $1;
        }
;

opt_order_by:
    | ORDER BY sort_spec_list
      {
          parser_ctx->setOrderBy(*$3);
          delete $3;
      }
;

sort_spec_list:
      sort_spec_list COMMA sort_spec
       {
          $$ = $1; $1->push_back($3);
       }
    | sort_spec
      { $$ = new QList<QgsSQLStatement::NodeColumnSorted*>(); $$->push_back($1); }
;

sort_spec:
    column_name
        {
            $$ = new QgsSQLStatement::NodeColumnSorted( $1, true );
        }
    | column_name ASC
        {
            $$ = new QgsSQLStatement::NodeColumnSorted( $1, true );
        }
    | column_name DESC
        {
            $$ = new QgsSQLStatement::NodeColumnSorted( $1, false );
        }
;

table_def:
    IDENTIFIER
    {
        $$ = new QgsSQLStatement::NodeTableDef(*$1);
        delete $1;
    }

    | IDENTIFIER as_clause
    {
        $$ = new QgsSQLStatement::NodeTableDef(*$1, *$2);
        delete $1;
        delete $2;
    }
;

table_list:
      table_list COMMA table_def
       {
          $$ = $1; $1->push_back($3);
       }
    | table_def
      { $$ = new QList<QgsSQLStatement::NodeTableDef*>(); $$->push_back($1); }
;

%%


// returns parsed tree, otherwise returns nullptr and sets parserErrorMsg
QgsSQLStatement::Node* parse(const QString& str, QString& parserErrorMsg)
{
  sqlstatement_parser_context ctx;
  ctx.rootNode = 0;

  sqlstatement_lex_init(&ctx.flex_scanner);
  sqlstatement__scan_string(str.toUtf8().constData(), ctx.flex_scanner);
  int res = sqlstatement_parse(&ctx);
  sqlstatement_lex_destroy(ctx.flex_scanner);

  // list should be empty when parsing was OK
  if (res == 0) // success?
  {
    ctx.rootNode->setWhere(ctx.whereExp);
    ctx.rootNode->setJoins(ctx.joinList);
    ctx.rootNode->setOrderBy(ctx.orderByList);
    return ctx.rootNode;
  }
  else // error?
  {
    parserErrorMsg = ctx.errorMsg;
    delete ctx.rootNode;
    delete ctx.whereExp;
    qDeleteAll(ctx.joinList);
    qDeleteAll(ctx.orderByList);
    return nullptr;
  }
}


void sqlstatement_error(sqlstatement_parser_context* parser_ctx, const QString& errorMsg)
{
  parser_ctx->errorMsg = errorMsg;
}

