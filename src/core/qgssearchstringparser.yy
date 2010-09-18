/***************************************************************************
                          qgssearchstringparser.yy 
                Grammar rules for search string used by Bison
                          --------------------
    begin                : 2005-07-26
    copyright            : (C) 2005 by Martin Dobias
    email                : won.der at centrum.sk
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */
 
%{
#include <qglobal.h>
#include <QList>
#include <cstdlib>
#include "qgssearchtreenode.h"

// don't redeclare malloc/free
#define YYINCLUDED_STDLIB_H 1

/** returns parsed tree, otherwise returns NULL and sets parserErrorMsg
    (interface function to be called from QgsSearchString) 
  */
QgsSearchTreeNode* parseSearchString(const QString& str, QString& parserErrorMsg);


//! from lex.yy.c
extern int yylex();
extern char* yytext;
extern void set_input_buffer(const char* buffer);

//! varible where the parser error will be stored
QString gParserErrorMsg;

//! sets gParserErrorMsg
void yyerror(const char* msg);

//! temporary list for nodes without parent (if parsing fails these nodes are removed)
QList<QgsSearchTreeNode*> gTmpNodes;
void joinTmpNodes(QgsSearchTreeNode* parent, QgsSearchTreeNode* left, QgsSearchTreeNode* right);
void addToTmpNodes(QgsSearchTreeNode* node);

// we want verbose error messages
#define YYERROR_VERBOSE 1

%}

%union { QgsSearchTreeNode* node; double number; QgsSearchTreeNode::Operator op;}

%start root

/* http://docs.openlinksw.com/virtuoso/GRAMMAR.html */

%token <number> NUMBER
%token <op> COMPARISON
%token <op> FUNCTION
%token CONCAT
%token IS
%token IN
%token ROWNUM
%token AREA
%token LENGTH
%token ID
%token NULLVALUE

%token STRING
%token COLUMN_REF

%token Unknown_CHARACTER

%token NOT
%token AND
%token OR

%type <node> root
%type <node> search_cond
%type <node> predicate
%type <node> comp_predicate
%type <node> scalar_exp
%type <node> scalar_exp_list

// debugging
//%error-verbose

// operator precedence
// all operators have left associativity, i.e. 1+2+3 translates to (1+2)+3
// the order of operators here determines their precedence

%left OR
%left AND
%left NOT

%left COMPARISON
%left CONCAT
%left IN

%left '+' '-'
%left '*' '/'
%left '^'
%left UMINUS  // fictitious symbol (for unary minus)


%%

root: search_cond { /*gParserRootNode = $1;*/ }
      | scalar_exp {}
    ;

search_cond:
      search_cond OR search_cond  { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opOR,  $1, $3); joinTmpNodes($$,$1,$3); }
    | search_cond AND search_cond { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opAND, $1, $3); joinTmpNodes($$,$1,$3); }
    | NOT search_cond             { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opNOT, $2,  0); joinTmpNodes($$,$2, 0); }
    | '(' search_cond ')'         { $$ = $2; }
    | predicate
    ;

    // more predicates to come
predicate:
    comp_predicate 
    ;

comp_predicate:
      scalar_exp IS NULLVALUE           { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opISNULL, $1, 0); joinTmpNodes($$,$1,0); }
    | scalar_exp IS NOT NULLVALUE       { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opISNOTNULL, $1, 0); joinTmpNodes($$,$1,0); }
    | scalar_exp COMPARISON scalar_exp  { $$ = new QgsSearchTreeNode($2, $1, $3); joinTmpNodes($$,$1,$3); }
    ;

scalar_exp_list:
      scalar_exp_list ',' scalar_exp { $$ = $1; $1->append($3); joinTmpNodes($1,$1,$3); }
    | scalar_exp                     { $$ = new QgsSearchTreeNode( QgsSearchTreeNode::tNodeList ); $$->append($1); joinTmpNodes($$,$1,0); }
    ;

scalar_exp:
      FUNCTION '(' scalar_exp ')' { $$ = new QgsSearchTreeNode($1, $3, 0); joinTmpNodes($$, $3, 0);}
    | scalar_exp '^' scalar_exp   { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opPOW,  $1, $3); joinTmpNodes($$,$1,$3); }
    | scalar_exp '*' scalar_exp   { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opMUL,  $1, $3); joinTmpNodes($$,$1,$3); }
    | scalar_exp '/' scalar_exp   { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opDIV,  $1, $3); joinTmpNodes($$,$1,$3); }
    | scalar_exp '+' scalar_exp   { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opPLUS, $1, $3); joinTmpNodes($$,$1,$3); }
    | scalar_exp '-' scalar_exp   { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opMINUS,$1, $3); joinTmpNodes($$,$1,$3); }
    | '(' scalar_exp ')'          { $$ = $2; }
    | '+' scalar_exp %prec UMINUS { $$ = $2; }
    | '-' scalar_exp %prec UMINUS { $$ = $2; if ($$->type() == QgsSearchTreeNode::tNumber) $$->setNumber(- $$->number()); }
    | scalar_exp CONCAT scalar_exp { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opCONCAT, $1, $3); joinTmpNodes($$, $1, $3); }
    | ROWNUM                      { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opROWNUM, 0, 0); addToTmpNodes($$); }
    | AREA                        { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opAREA, 0, 0); addToTmpNodes($$); }
    | LENGTH                      { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opLENGTH, 0, 0); addToTmpNodes($$); }
    | ID                          { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opID, 0, 0); addToTmpNodes($$); }
    | NUMBER                      { $$ = new QgsSearchTreeNode($1); addToTmpNodes($$); }
    | STRING                      { $$ = new QgsSearchTreeNode(QString::fromUtf8(yytext), 0); addToTmpNodes($$); }
    | COLUMN_REF                  { $$ = new QgsSearchTreeNode(QString::fromUtf8(yytext), 1); addToTmpNodes($$); }
    | scalar_exp IN '(' scalar_exp_list ')' { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opIN, $1, $4); joinTmpNodes($$,$1,$4); }
    | scalar_exp NOT IN '(' scalar_exp_list ')' { $$ = new QgsSearchTreeNode(QgsSearchTreeNode::opNOTIN, $1, $5); joinTmpNodes($$,$1,$5); }
;

%%

void addToTmpNodes(QgsSearchTreeNode* node)
{
  gTmpNodes.append(node);
}


void joinTmpNodes(QgsSearchTreeNode* parent, QgsSearchTreeNode* left, QgsSearchTreeNode* right)
{
  bool res;

  if (left)
  {
    res = gTmpNodes.removeAll(left);
    Q_ASSERT(res);
  }

  if (right)
  {
    res = gTmpNodes.removeAll(right);
    Q_ASSERT(res);
  }

  gTmpNodes.append(parent);
}

// returns parsed tree, otherwise returns NULL and sets parserErrorMsg
QgsSearchTreeNode* parseSearchString(const QString& str, QString& parserErrorMsg)
{
  // list should be empty when starting
  Q_ASSERT(gTmpNodes.count() == 0);

  set_input_buffer(str.toUtf8().constData());
  int res = yyparse();
 
  // list should be empty when parsing was OK
  if (res == 0) // success?
  {
    Q_ASSERT(gTmpNodes.count() == 1);
    return gTmpNodes.takeFirst();
  }
  else // error?
  {
    parserErrorMsg = gParserErrorMsg;
    // remove nodes without parents - to prevent memory leaks
    while (gTmpNodes.size() > 0)
      delete gTmpNodes.takeFirst();
    return NULL;
  }
}


void yyerror(const char* msg)
{
  gParserErrorMsg = msg;
}


