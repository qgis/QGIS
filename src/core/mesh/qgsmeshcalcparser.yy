/***************************************************************************
                          qgsmeshcalcparser.yy
                          --------------------
    begin                : December 19th, 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

%{
  #include "qgsmeshcalcnode.h"

#ifdef _MSC_VER
#  pragma warning( disable: 4065 )  // switch statement contains 'default' but no 'case' labels
#  pragma warning( disable: 4701 )  // Potentially uninitialized local variable 'name' used
#endif

  // don't redeclare malloc/free
  #define YYINCLUDED_STDLIB_H 1

  QgsMeshCalcNode* parseMeshCalcString(const QString& str, QString& parserErrorMsg);

  //! from lex.yy.c
  extern int meshlex();
  extern char* meshtext;
  extern void set_mesh_input_buffer(const char* buffer);

  //! variable where the parser error will be stored
  QString rMeshParserErrorMsg;

  //! sets gParserErrorMsg
  void mesherror(const char* msg);

  //! temporary list for nodes without parent (if parsing fails these nodes are removed)
  QList<QgsMeshCalcNode*> gMeshTmpNodes;
  void joinTmpNodes(QgsMeshCalcNode* parent, QgsMeshCalcNode* left, QgsMeshCalcNode* right, QgsMeshCalcNode* condition);
  void addToTmpNodes(QgsMeshCalcNode* node);

  // we want verbose error messages
  #define YYERROR_VERBOSE 1
%}

%union { QgsMeshCalcNode* node; double number; QgsMeshCalcNode::Operator op;}

%start root

%token NODATA
%token DATASET_REF
%token<number> NUMBER
%token<op> FUNCTION
%token<op> FUNCTION2

%type <node> root
%type <node> mesh_exp

%left AND
%left OR
%left NOT
%left NE
%left GE
%left LE
%left IF

%left '=' '<' '>'
%left '+' '-'
%left '*' '/'
%left '^'
%left UMINUS  // fictitious symbol (for unary minus)

%%

root: mesh_exp{}
;

mesh_exp:
  FUNCTION '(' mesh_exp ')'   { $$ = new QgsMeshCalcNode($1, $3, 0); joinTmpNodes($$, $3, 0, 0);}
  | FUNCTION2 '(' mesh_exp ',' mesh_exp ')'   { $$ = new QgsMeshCalcNode($1, $3, $5); joinTmpNodes($$, $3, $5, 0);}
  | IF '(' mesh_exp ',' mesh_exp ',' mesh_exp ')'   { $$ = new QgsMeshCalcNode($3, $5, $7); joinTmpNodes($$, $3, $5, $7);}
  | NOT '(' mesh_exp ')'      { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opNOT, $3, 0 ); joinTmpNodes($$,$3, 0, 0); }
  | mesh_exp AND mesh_exp     { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opAND, $1, $3 ); joinTmpNodes($$,$1,$3, 0); }
  | mesh_exp OR mesh_exp      { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opOR, $1, $3 ); joinTmpNodes($$,$1,$3, 0); }
  | mesh_exp '=' mesh_exp     { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opEQ, $1, $3 ); joinTmpNodes($$,$1,$3, 0); }
  | mesh_exp NE mesh_exp      { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opNE, $1, $3 ); joinTmpNodes($$,$1,$3, 0); }
  | mesh_exp '>' mesh_exp     { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opGT, $1, $3 ); joinTmpNodes($$, $1, $3, 0); }
  | mesh_exp '<' mesh_exp     { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opLT, $1, $3 ); joinTmpNodes($$, $1, $3, 0); }
  | mesh_exp GE mesh_exp      { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opGE, $1, $3 ); joinTmpNodes($$, $1, $3, 0); }
  | mesh_exp LE mesh_exp      { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opLE, $1, $3 ); joinTmpNodes($$, $1, $3, 0); }
  | mesh_exp '^' mesh_exp     { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opPOW, $1, $3 ); joinTmpNodes($$,$1,$3, 0); }
  | mesh_exp '*' mesh_exp     { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opMUL, $1, $3 ); joinTmpNodes($$,$1,$3, 0); }
  | mesh_exp '/' mesh_exp     { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opDIV, $1, $3 ); joinTmpNodes($$,$1,$3, 0); }
  | mesh_exp '+' mesh_exp     { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opPLUS, $1, $3 ); joinTmpNodes($$,$1,$3, 0); }
  | mesh_exp '-' mesh_exp     { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opMINUS, $1, $3 ); joinTmpNodes($$,$1,$3, 0); }
  | '(' mesh_exp ')'          { $$ = $2; }
  | '+' mesh_exp %prec UMINUS { $$ = $2; }
  | '-' mesh_exp %prec UMINUS { $$ = new QgsMeshCalcNode( QgsMeshCalcNode::opSIGN, $2, 0 ); joinTmpNodes($$, $2, 0, 0); }
  | NUMBER { $$ = new QgsMeshCalcNode($1); addToTmpNodes($$); }
  | DATASET_REF { $$ = new QgsMeshCalcNode(QString::fromUtf8(meshtext)); addToTmpNodes($$); }
  | NODATA { $$ = new QgsMeshCalcNode(); addToTmpNodes($$); }
;

%%

void addToTmpNodes(QgsMeshCalcNode* node)
{
  gMeshTmpNodes.append(node);
}


void removeTmpNode(QgsMeshCalcNode* node)
{
  bool res;
  Q_UNUSED(res)

  if (node)
  {
    res = gMeshTmpNodes.removeAll(node) != 0;
    Q_ASSERT(res);
  }
}

void joinTmpNodes(QgsMeshCalcNode* parent, QgsMeshCalcNode* left, QgsMeshCalcNode* right, QgsMeshCalcNode* condition)
{
  removeTmpNode(right);
  removeTmpNode(left);
  removeTmpNode(condition);
  gMeshTmpNodes.append(parent);
}


QgsMeshCalcNode* localParseMeshCalcString(const QString& str, QString& parserErrorMsg)
{
  // list should be empty when starting
  Q_ASSERT(gMeshTmpNodes.count() == 0);

  set_mesh_input_buffer(str.toUtf8().constData());
  int res = meshparse();

  // list should be empty when parsing was OK
  if (res == 0) // success?
  {
    Q_ASSERT(gMeshTmpNodes.count() == 1);
    return gMeshTmpNodes.takeFirst();
  }
  else // error?
  {
    parserErrorMsg = rMeshParserErrorMsg;
    // remove nodes without parents - to prevent memory leaks
    while (gMeshTmpNodes.size() > 0)
      delete gMeshTmpNodes.takeFirst();
    return nullptr;
  }
}

void mesherror(const char* msg)
{
  rMeshParserErrorMsg = msg;
}



