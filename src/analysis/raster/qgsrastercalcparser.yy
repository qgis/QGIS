/***************************************************************************
                          qgsrastercalcparser.yy
             Bison file for raster calculation parser
                          --------------------
    begin                : 2010-10-25
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
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
  #include "qgsrastercalcnode.h"

  // don't redeclare malloc/free
  #define YYINCLUDED_STDLIB_H 1

  QgsRasterCalcNode* parseRasterCalcString(const QString& str, QString& parserErrorMsg);

  //! from lex.yy.c
  extern int rasterlex();
  extern char* rastertext;
  extern void set_raster_input_buffer(const char* buffer);

  //! varible where the parser error will be stored
  QString rParserErrorMsg;

  //! sets gParserErrorMsg
  void rastererror(const char* msg);

  //! temporary list for nodes without parent (if parsing fails these nodes are removed)
  QList<QgsRasterCalcNode*> gTmpNodes;
  void joinTmpNodes(QgsRasterCalcNode* parent, QgsRasterCalcNode* left, QgsRasterCalcNode* right);
  void addToTmpNodes(QgsRasterCalcNode* node);

  // we want verbose error messages
  #define YYERROR_VERBOSE 1
%}

%union { QgsRasterCalcNode* node; double number; QgsRasterCalcNode::Operator op;}

%start root

%token RASTER_BAND_REF
%token<number> NUMBER
%token<op> FUNCTION

%type <node> root
%type <node> raster_exp

%left '+' '-'
%left '*' '/'
%left '^'

%%

root: raster_exp{}
;

raster_exp:
  FUNCTION '(' raster_exp ')'   { $$ = new QgsRasterCalcNode($1, $3, 0); joinTmpNodes($$, $3, 0);}
  | raster_exp '^' raster_exp   { $$ = new QgsRasterCalcNode(QgsRasterCalcNode::opPOW, $1, $3); joinTmpNodes($$,$1,$3); }
  | raster_exp '*' raster_exp   { $$ = new QgsRasterCalcNode(QgsRasterCalcNode::opMUL, $1, $3); joinTmpNodes($$,$1,$3); }
  | raster_exp '/' raster_exp   { $$ = new QgsRasterCalcNode(QgsRasterCalcNode::opDIV, $1, $3); joinTmpNodes($$,$1,$3); }
  | raster_exp '+' raster_exp   { $$ = new QgsRasterCalcNode(QgsRasterCalcNode::opPLUS, $1, $3); joinTmpNodes($$,$1,$3); }
  | raster_exp '-' raster_exp   { $$ = new QgsRasterCalcNode(QgsRasterCalcNode::opMINUS, $1, $3); joinTmpNodes($$,$1,$3); }
  | '(' raster_exp ')'          { $$ = $2; }
  | NUMBER { $$ = new QgsRasterCalcNode($1); addToTmpNodes($$); }
  | RASTER_BAND_REF { $$ = new QgsRasterCalcNode(QString::fromUtf8(rastertext)); addToTmpNodes($$); }
;

%%

void addToTmpNodes(QgsRasterCalcNode* node)
{
  gTmpNodes.append(node);
}


void joinTmpNodes(QgsRasterCalcNode* parent, QgsRasterCalcNode* left, QgsRasterCalcNode* right)
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


QgsRasterCalcNode* localParseRasterCalcString(const QString& str, QString& parserErrorMsg)
{
  // list should be empty when starting
  Q_ASSERT(gTmpNodes.count() == 0);

  set_raster_input_buffer(str.toUtf8().constData());
  int res = rasterparse();

  // list should be empty when parsing was OK
  if (res == 0) // success?
  {
    Q_ASSERT(gTmpNodes.count() == 1);
    return gTmpNodes.takeFirst();
  }
  else // error?
  {
    parserErrorMsg = rParserErrorMsg;
    // remove nodes without parents - to prevent memory leaks
    while (gTmpNodes.size() > 0)
      delete gTmpNodes.takeFirst();
    return NULL;
  }
}

void rastererror(const char* msg)
{
  rParserErrorMsg = msg;
}



