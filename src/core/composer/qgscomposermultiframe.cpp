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

QgsComposerMultiFrame::QgsComposerMultiFrame( QgsComposition* c, bool createUndoCommands ): mComposition( c ), mResizeMode( UseExistingFrames ), mCreateUndoCommands( createUndoCommands )
{
  mComposition->addMultiFrame( this );
}

QgsComposerMultiFrame::QgsComposerMultiFrame(): mComposition( 0 ), mResizeMode( UseExistingFrames )
{
}

QgsComposerMultiFrame::~QgsComposerMultiFrame()
{
  deleteFrames();
}

void QgsComposerMultiFrame::setResizeMode( ResizeMode mode )
{
  if ( mode != mResizeMode )
  {
    mResizeMode = mode;
    recalculateFrameSizes();
    emit changed();
  }
}

void QgsComposerMultiFrame::recalculateFrameSizes()
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
      if ( mResizeMode == RepeatUntilFinished || mResizeMode == ExtendToNextPage ) //remove unneeded frames in extent mode
      {
        for ( int j = mFrameItems.size(); j > i; --j )
        {
          removeFrame( j - 1 );
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
  if ( mResizeMode == RepeatUntilFinished || mResizeMode == ExtendToNextPage )
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

      double frameHeight = 0;
      if ( mResizeMode == RepeatUntilFinished )
      {
        frameHeight = currentItem->rect().height();
      }
      else //mResizeMode == ExtendToNextPage
      {
        frameHeight = ( currentY + mComposition->paperHeight() ) > totalHeight ?  totalHeight - currentY : mComposition->paperHeight();
      }
      QgsComposerFrame* newFrame = new QgsComposerFrame( mComposition, this, currentItem->transform().dx(),
          ( mComposition->numPages() - 1 ) * ( mComposition->paperHeight() + mComposition->spaceBetweenPages() ),
          currentItem->rect().width(), frameHeight );
      newFrame->setContentSection( QRectF( 0, currentY, newFrame->rect().width(), newFrame->rect().height() ) );
      currentY += frameHeight;
      currentItem = newFrame;
      addFrame( newFrame, false );
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
  if ( mFrameItems.size() > 0 )
  {
    recalculateFrameSizes();
  }
}

void QgsComposerMultiFrame::removeFrame( int i )
{
  QgsComposerFrame* frameItem = mFrameItems[i];
  if ( mComposition )
  {
    mComposition->removeComposerItem( frameItem );
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

void QgsComposerMultiFrame::deleteFrames()
{
  ResizeMode bkResizeMode = mResizeMode;
  mResizeMode = UseExistingFrames;
  mComposition->blockSignals( true );
  QList<QgsComposerFrame*>::iterator frameIt = mFrameItems.begin();
  for ( ; frameIt != mFrameItems.end(); ++frameIt )
  {
    mComposition->removeComposerItem( *frameIt, false );
    delete *frameIt;
  }
  mComposition->blockSignals( false );
  mFrameItems.clear();
  mResizeMode = bkResizeMode;
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
