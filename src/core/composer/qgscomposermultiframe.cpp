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
#include "qgscomposition.h"
#include <QtCore>

QgsComposerMultiFrame::QgsComposerMultiFrame( QgsComposition* c, bool createUndoCommands ):
    QgsComposerObject( c ),
    mResizeMode( UseExistingFrames ),
    mCreateUndoCommands( createUndoCommands ),
    mIsRecalculatingSize( false )
{
  mComposition->addMultiFrame( this );
  connect( mComposition, SIGNAL( nPagesChanged() ), this, SLOT( handlePageChange() ) );
}

QgsComposerMultiFrame::QgsComposerMultiFrame()
    : QgsComposerObject( 0 )
    , mResizeMode( UseExistingFrames )
    , mCreateUndoCommands( false )
    , mIsRecalculatingSize( false )
{
}

QgsComposerMultiFrame::~QgsComposerMultiFrame()
{
  deleteFrames();
}

void QgsComposerMultiFrame::render( QPainter *p, const QRectF &renderExtent )
{
  //base implementation does nothing
  Q_UNUSED( p );
  Q_UNUSED( renderExtent );
}

void QgsComposerMultiFrame::render( QPainter *painter, const QRectF &renderExtent, const int frameIndex )
{
  Q_UNUSED( frameIndex );
  //base implementation ignores frameIndex
  Q_NOWARN_DEPRECATED_PUSH
  render( painter, renderExtent );
  Q_NOWARN_DEPRECATED_POP
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
    if ( mResizeMode != RepeatOnEveryPage && currentY >= totalHeight )
    {
      if ( mResizeMode == RepeatUntilFinished || mResizeMode == ExtendToNextPage ) //remove unneeded frames in extent mode
      {
        bool removingPages = true;
        for ( int j = mFrameItems.size(); j > i; --j )
        {
          int numPagesBefore = mComposition->numPages();
          removeFrame( j - 1, removingPages );
          //if removing the frame didn't also remove the page, then stop removing pages
          removingPages = removingPages && ( mComposition->numPages() < numPagesBefore );
        }
        return;
      }
    }

    currentItem = mFrameItems.value( i );
    currentHeight = currentItem->rect().height();
    if ( mResizeMode == RepeatOnEveryPage )
    {
      currentItem->setContentSection( QRectF( 0, 0, currentItem->rect().width(), currentHeight ) );
    }
    else
    {
      currentHeight = findNearbyPageBreak( currentY + currentHeight ) - currentY;
      currentItem->setContentSection( QRectF( 0, currentY, currentItem->rect().width(), currentHeight ) );
    }
    currentItem->update();
    currentY += currentHeight;
  }

  //at end of frames but there is  still content left. Add other pages if ResizeMode ==
  if ( mResizeMode != UseExistingFrames )
  {
    while (( mResizeMode == RepeatOnEveryPage ) || currentY < totalHeight )
    {
      //find out on which page the lower left point of the last frame is
      int page = qFloor(( currentItem->pos().y() + currentItem->rect().height() ) / ( mComposition->paperHeight() + mComposition->spaceBetweenPages() ) ) + 1;

      if ( mResizeMode == RepeatOnEveryPage )
      {
        if ( page >= mComposition->numPages() )
        {
          break;
        }
      }
      else
      {
        //add an extra page if required
        if ( mComposition->numPages() < ( page + 1 ) )
        {
          mComposition->setNumPages( page + 1 );
        }
      }

      double frameHeight = 0;
      if ( mResizeMode == RepeatUntilFinished || mResizeMode == RepeatOnEveryPage )
      {
        frameHeight = currentItem->rect().height();
      }
      else //mResizeMode == ExtendToNextPage
      {
        frameHeight = ( currentY + mComposition->paperHeight() ) > totalHeight ?  totalHeight - currentY : mComposition->paperHeight();
      }

      double newFrameY = page * ( mComposition->paperHeight() + mComposition->spaceBetweenPages() );
      if ( mResizeMode == RepeatUntilFinished || mResizeMode == RepeatOnEveryPage )
      {
        newFrameY += currentItem->pos().y() - ( page - 1 ) * ( mComposition->paperHeight() + mComposition->spaceBetweenPages() );
      }

      //create new frame
      QgsComposerFrame* newFrame = createNewFrame( currentItem,
                                   QPointF( currentItem->pos().x(), newFrameY ),
                                   QSizeF( currentItem->rect().width(), frameHeight ) );

      if ( mResizeMode == RepeatOnEveryPage )
      {
        newFrame->setContentSection( QRectF( 0, 0, newFrame->rect().width(), newFrame->rect().height() ) );
        currentY += frameHeight;
      }
      else
      {
        double contentHeight = findNearbyPageBreak( currentY + newFrame->rect().height() ) - currentY;
        newFrame->setContentSection( QRectF( 0, currentY, newFrame->rect().width(), contentHeight ) );
        currentY += contentHeight;
      }

      currentItem = newFrame;
    }
  }
}

void QgsComposerMultiFrame::recalculateFrameRects()
{
  if ( mFrameItems.size() < 1 )
  {
    //no frames, nothing to do
    return;
  }

  QList<QgsComposerFrame*>::iterator frameIt = mFrameItems.begin();
  for ( ; frameIt != mFrameItems.end(); ++frameIt )
  {
    ( *frameIt )->setSceneRect( QRectF(( *frameIt )->scenePos().x(), ( *frameIt )->scenePos().y(),
                                       ( *frameIt )->rect().width(), ( *frameIt )->rect().height() ) );
  }
}

QgsComposerFrame* QgsComposerMultiFrame::createNewFrame( QgsComposerFrame* currentFrame, QPointF pos, QSizeF size )
{
  if ( !currentFrame )
  {
    return 0;
  }

  QgsComposerFrame* newFrame = new QgsComposerFrame( mComposition, this, pos.x(),
      pos.y(), size.width(), size.height() );

  //copy some settings from the parent frame
  newFrame->setBackgroundColor( currentFrame->backgroundColor() );
  newFrame->setBackgroundEnabled( currentFrame->hasBackground() );
  newFrame->setBlendMode( currentFrame->blendMode() );
  newFrame->setFrameEnabled( currentFrame->hasFrame() );
  newFrame->setFrameOutlineColor( currentFrame->frameOutlineColor() );
  newFrame->setFrameJoinStyle( currentFrame->frameJoinStyle() );
  newFrame->setFrameOutlineWidth( currentFrame->frameOutlineWidth() );
  newFrame->setTransparency( currentFrame->transparency() );
  newFrame->setHideBackgroundIfEmpty( currentFrame->hideBackgroundIfEmpty() );

  addFrame( newFrame, false );

  return newFrame;
}

QString QgsComposerMultiFrame::displayName() const
{
  return tr( "<frame>" );
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
    if ( resizeMode() != QgsComposerMultiFrame::RepeatOnEveryPage && !mIsRecalculatingSize )
    {
      //removing a frame forces the multi frame to UseExistingFrames resize mode
      //otherwise the frame may not actually be removed, leading to confusing ui behaviour
      mResizeMode = QgsComposerMultiFrame::UseExistingFrames;
      emit changed();
      recalculateFrameSizes();
    }
  }
}

void QgsComposerMultiFrame::handlePageChange()
{
  if ( mComposition->numPages() < 1 )
  {
    return;
  }

  if ( mResizeMode != RepeatOnEveryPage )
  {
    return;
  }

  //remove items beginning on non-existing pages
  for ( int i = mFrameItems.size() - 1; i >= 0; --i )
  {
    QgsComposerFrame* frame = mFrameItems[i];
    int page = frame->pos().y() / ( mComposition->paperHeight() + mComposition->spaceBetweenPages() );
    if ( page > ( mComposition->numPages() - 1 ) )
    {
      removeFrame( i );
    }
  }

  //page number of the last item
  QgsComposerFrame* lastFrame = mFrameItems.last();
  int lastItemPage = lastFrame->pos().y() / ( mComposition->paperHeight() + mComposition->spaceBetweenPages() );

  for ( int i = lastItemPage + 1; i < mComposition->numPages(); ++i )
  {
    //copy last frame to current page
    QgsComposerFrame* newFrame = new QgsComposerFrame( mComposition, this, lastFrame->pos().x(),
        lastFrame->pos().y() + mComposition->paperHeight() + mComposition->spaceBetweenPages(),
        lastFrame->rect().width(), lastFrame->rect().height() );
    addFrame( newFrame, false );
    lastFrame = newFrame;
  }

  recalculateFrameSizes();
  update();
}

void QgsComposerMultiFrame::removeFrame( int i, const bool removeEmptyPages )
{
  if ( i >= mFrameItems.count() )
  {
    return;
  }

  QgsComposerFrame* frameItem = mFrameItems[i];
  if ( mComposition )
  {
    mIsRecalculatingSize = true;
    int pageNumber = frameItem->page();
    //remove item, but don't create undo command
    mComposition->removeComposerItem( frameItem, false );
    //if frame was the only item on the page, remove the page
    if ( removeEmptyPages && mComposition->pageIsEmpty( pageNumber ) )
    {
      mComposition->setNumPages( mComposition->numPages() - 1 );
    }
    mIsRecalculatingSize = false;
  }
  mFrameItems.removeAt( i );
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
  QObject::disconnect( mComposition, SIGNAL( itemRemoved( QgsComposerItem* ) ), this, SLOT( handleFrameRemoval( QgsComposerItem* ) ) );
  QList<QgsComposerFrame*>::iterator frameIt = mFrameItems.begin();
  for ( ; frameIt != mFrameItems.end(); ++frameIt )
  {
    mComposition->removeComposerItem( *frameIt, false );
    delete *frameIt;
  }
  QObject::connect( mComposition, SIGNAL( itemRemoved( QgsComposerItem* ) ), this, SLOT( handleFrameRemoval( QgsComposerItem* ) ) );
  mFrameItems.clear();
  mResizeMode = bkResizeMode;
}

QgsComposerFrame* QgsComposerMultiFrame::frame( int i ) const
{
  if ( i >= mFrameItems.size() )
  {
    return 0;
  }
  return mFrameItems.at( i );
}

int QgsComposerMultiFrame::frameIndex( QgsComposerFrame *frame ) const
{
  return mFrameItems.indexOf( frame );
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
  QgsComposerObject::writeXML( elem, doc );
  return true;
}

bool QgsComposerMultiFrame::_readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames )
{
  QgsComposerObject::readXML( itemElem, doc );

  mResizeMode = ( ResizeMode )itemElem.attribute( "resizeMode", "0" ).toInt();
  if ( !ignoreFrames )
  {
    QDomNodeList frameList = itemElem.elementsByTagName( "ComposerFrame" );
    for ( int i = 0; i < frameList.size(); ++i )
    {
      QDomElement frameElem = frameList.at( i ).toElement();
      QgsComposerFrame* newFrame = new QgsComposerFrame( mComposition, this, 0, 0, 0, 0 );
      newFrame->readXML( frameElem, doc );
      addFrame( newFrame, false );
    }

    //TODO - think there should be a recalculateFrameSizes() call here
  }
  return true;
}
