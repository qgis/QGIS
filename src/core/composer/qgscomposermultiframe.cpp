/***************************************************************************
                              qgscomposermultiframe.cpp
    ------------------------------------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposermultiframe.h"
#include "qgscomposerframe.h"
#include "qgsaddremovemultiframecommand.h"

QgsComposerMultiFrame::QgsComposerMultiFrame( QgsComposition* c ): mComposition( c ), mResizeMode( UseExistingFrames )
{
  mComposition->addMultiFrame( this );
}

QgsComposerMultiFrame::QgsComposerMultiFrame(): mComposition( 0 ), mResizeMode( UseExistingFrames )
{
}

QgsComposerMultiFrame::~QgsComposerMultiFrame()
{
  QList<QgsComposerFrame*>::iterator frameIt = mFrameItems.begin();
  for ( ; frameIt != mFrameItems.begin(); ++frameIt )
  {
    mComposition->removeComposerItem( *frameIt );
    delete( *frameIt );
  }
}

void QgsComposerMultiFrame::setResizeMode( ResizeMode mode )
{
  if ( mode != mResizeMode )
  {
    mResizeMode = mode;
    recalculateFrameSizes( true );
    emit changed();
  }
}

void QgsComposerMultiFrame::recalculateFrameSizes( bool addCommands )
{
  if ( mFrameItems.size() < 1 )
  {
    return;
  }

  QSizeF size = totalSize();
  double totalHeight = size.height();

  if ( totalHeight < 1 )
  {
    return;
  }

  double currentY = 0;
  double currentHeight = 0;
  QgsComposerFrame* currentItem = 0;

  for ( int i = 0; i < mFrameItems.size(); ++i )
  {
    if ( currentY >= totalHeight )
    {
      if ( mResizeMode == ExtendToNextPage ) //remove unneeded frames in extent mode
      {
        for ( int j = mFrameItems.size(); j > i; --j )
        {
          removeFrame( j - 1, addCommands );
        }
      }
      return;
    }

    currentItem = mFrameItems.value( i );
    currentHeight = currentItem->rect().height();
    currentItem->setContentSection( QRectF( 0, currentY, currentItem->rect().width(), currentHeight ) );
    currentItem->update();
    currentY += currentHeight;
  }

  //at end of frames but there is  still content left. Add other pages if ResizeMode ==
  if ( mResizeMode == ExtendToNextPage )
  {
    while ( currentY < totalHeight )
    {
      //find out on which page the lower left point of the last frame is
      int page = currentItem->transform().dy() / ( mComposition->paperHeight() + mComposition->spaceBetweenPages() );

      //add new pages if necessary
      if ( mComposition->numPages() < ( page + 2 ) )
      {
        mComposition->setNumPages( page + 2 );
      }

      //copy last frame
      QgsComposerFrame* newFrame = new QgsComposerFrame( mComposition, this, currentItem->transform().dx(), currentItem->transform().dy() + mComposition->paperHeight() + mComposition->spaceBetweenPages(),
          currentItem->rect().width(), currentItem->rect().height() );
      newFrame->setContentSection( QRectF( 0, currentY, newFrame->rect().width(), newFrame->rect().height() ) );
      currentY += newFrame->rect().height();
      currentItem = newFrame;
      addFrame( newFrame, addCommands );
    }
  }
}

void QgsComposerMultiFrame::handleFrameRemoval( QgsComposerItem* item )
{
  QgsComposerFrame* frame = dynamic_cast<QgsComposerFrame*>( item );
  if ( !frame )
  {
    return;
  }
  int index = mFrameItems.indexOf( frame );
  if ( index == -1 )
  {
    return;
  }
  mFrameItems.removeAt( index );

  if ( mFrameItems.size() < 1 )
  {
    if ( mComposition )
    {
      //schedule this composer multi frame for deletion
      mComposition->removeMultiFrame( this );
    }
  }
  else
  {
    recalculateFrameSizes();
  }
}

void QgsComposerMultiFrame::removeFrame( int i, bool addCommand )
{
  QgsComposerFrame* frameItem = mFrameItems[i];
  if ( mComposition )
  {
    mComposition->removeComposerItem( frameItem );
    if ( addCommand )
    {
      mComposition->pushAddRemoveCommand( frameItem, tr( "Frame removed" ), QgsAddRemoveItemCommand::Removed );
    }
  }
}

void QgsComposerMultiFrame::update()
{
  QList<QgsComposerFrame*>::iterator frameIt = mFrameItems.begin();
  for ( ; frameIt != mFrameItems.end(); ++frameIt )
  {
    ( *frameIt )->update();
  }
}

bool QgsComposerMultiFrame::_writeXML( QDomElement& elem, QDomDocument& doc, bool ignoreFrames ) const
{
  elem.setAttribute( "resizeMode", mResizeMode );
  if ( !ignoreFrames )
  {
    QList<QgsComposerFrame*>::const_iterator frameIt = mFrameItems.constBegin();
    for ( ; frameIt != mFrameItems.constEnd(); ++frameIt )
    {
      ( *frameIt )->writeXML( elem, doc );
    }
  }
  return true;
}

bool QgsComposerMultiFrame::_readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames )
{
  mResizeMode = ( ResizeMode )itemElem.attribute( "resizeMode", "0" ).toInt();
  if ( !ignoreFrames )
  {
    QDomNodeList frameList = itemElem.elementsByTagName( "ComposerFrame" );
    for ( int i = 0; i < frameList.size(); ++i )
    {
      QDomElement frameElem = frameList.at( i ).toElement();
      QgsComposerFrame* newFrame = new QgsComposerFrame( mComposition, this, 0, 0, 0, 0 );
      newFrame->readXML( frameElem, doc );
      addFrame( newFrame );
    }
  }
  return true;
}
