/***************************************************************************
                           qgssearchstring.cpp
          interface for parsing and evaluation of search strings
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

#include "qgssearchstring.h"
#include "qgssearchtreenode.h"


//! global function from parser.y that interfaces parser
extern QgsSearchTreeNode* parseSearchString( const QString& str, QString& parserErrorMsg );

QgsSearchString::QgsSearchString()
{
  mTree = NULL;
}

QgsSearchString::QgsSearchString( const QString & str )
{
  mTree = NULL;
  setString( str );
}

QgsSearchString::QgsSearchString( const QgsSearchString& str )
{
  if ( str.mTree )
    mTree = new QgsSearchTreeNode( *str.mTree );
  else
    mTree = NULL;
  mString = str.mString;
}

QgsSearchString& QgsSearchString::operator=( const QgsSearchString & str )
{
  clear();

  if ( str.mTree )
    mTree = new QgsSearchTreeNode( *str.mTree );
  else
    mTree = NULL;
  mString = str.mString;

  return *this;
}


QgsSearchString::~QgsSearchString()
{
  delete mTree; // deletes complete tree
}


bool QgsSearchString::setString( QString str )
{
  mParserErrorMsg.clear();

  // empty string
  if ( str.isEmpty() )
  {
    clear();
    return true;
  }

  // calls external C function that does all parsing
  QgsSearchTreeNode* tree = parseSearchString( str, mParserErrorMsg );
  if ( tree )
  {
    delete mTree;
    mTree = tree;
    mString = str;
    return true;
  }

  return false;
}


bool QgsSearchString::setTree( QgsSearchTreeNode* tree )
{
  if ( tree == NULL )
  {
    clear();
  }
  else
  {
    delete mTree;
    mTree = new QgsSearchTreeNode( *tree );
    mString = mTree->makeSearchString();
  }
  return true;
}

bool QgsSearchString::isEmpty()
{
  return ( mTree == NULL );
}

void QgsSearchString::clear()
{
  delete mTree;
  mTree = NULL;
  mString.clear();
}
