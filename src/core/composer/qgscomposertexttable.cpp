/***************************************************************************
                         qgscomposertexttable.h
                         ----------------------
    begin                : April 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposertexttable.h"
#include "qgscomposertablecolumn.h"
#include "qgscomposerframe.h"
#include "qgscomposition.h"

QgsComposerTextTableV2::QgsComposerTextTableV2( QgsComposition* c, bool createUndoCommands )
    : QgsComposerTableV2( c, createUndoCommands )
{

}

QgsComposerTextTableV2::~QgsComposerTextTableV2()
{

}

void QgsComposerTextTableV2::addRow( const QStringList& row )
{
  mRowText.append( row );
  refreshAttributes();
}

void QgsComposerTextTableV2::setContents( const QList<QStringList>& contents )
{
  mRowText = contents;
  refreshAttributes();
}

bool QgsComposerTextTableV2::getTableContents( QgsComposerTableContents& contents )
{
  contents.clear();

  QList< QStringList >::const_iterator rowIt = mRowText.constBegin();
  for ( ; rowIt != mRowText.constEnd(); ++rowIt )
  {
    QgsComposerTableRow currentRow;

    for ( int i = 0; i < mColumns.count(); ++i )
    {
      if ( i < ( *rowIt ).count() )
      {
        currentRow << ( *rowIt ).at( i );
      }
      else
      {
        currentRow << QString();
      }
    }
    contents << currentRow;
  }

  recalculateTableSize();
  return true;
}

void QgsComposerTextTableV2::addFrame( QgsComposerFrame* frame, bool recalcFrameSizes )
{
  mFrameItems.push_back( frame );
  connect( frame, SIGNAL( sizeChanged() ), this, SLOT( recalculateFrameSizes() ) );

  if ( mComposition )
  {
    //TODO - if QgsComposerTextTableV2 gains a UI, this will need a dedicated add method
    //added to QgsComposition
    mComposition->addItem( frame );
  }

  if ( recalcFrameSizes )
  {
    recalculateFrameSizes();
  }
}
