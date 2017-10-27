/***************************************************************************
                              qgslayoutmultiframe.cpp
                              -----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutmultiframe.h"
#include "qgslayoutframe.h"
#include "qgslayout.h"
#include <QtCore>

QgsLayoutMultiFrame::QgsLayoutMultiFrame( QgsLayout *layout )
  : QgsLayoutObject( layout )
{
  mLayout->addMultiFrame( this );

#if 0 //TODO
  connect( mLayout, &QgsLayout::nPagesChanged, this, &QgsLayoutMultiFrame::handlePageChange );
#endif
}

QgsLayoutMultiFrame::~QgsLayoutMultiFrame()
{
  deleteFrames();
}

QSizeF QgsLayoutMultiFrame::fixedFrameSize( const int frameIndex ) const
{
  Q_UNUSED( frameIndex );
  return QSizeF( 0, 0 );
}

QSizeF QgsLayoutMultiFrame::minFrameSize( const int frameIndex ) const
{
  Q_UNUSED( frameIndex );
  return QSizeF( 0, 0 );
}

double QgsLayoutMultiFrame::findNearbyPageBreak( double yPos )
{
  return yPos;
}

void QgsLayoutMultiFrame::addFrame( QgsLayoutFrame *frame, bool recalcFrameSizes )
{
  if ( !frame )
    return;

  mFrameItems.push_back( frame );
  frame->mMultiFrame = this;
  connect( frame, &QgsLayoutFrame::destroyed, this, &QgsLayoutMultiFrame::handleFrameRemoval );
  if ( mLayout )
  {
    mLayout->addLayoutItem( frame );
  }

  if ( recalcFrameSizes )
  {
    recalculateFrameSizes();
  }
}

void QgsLayoutMultiFrame::setResizeMode( ResizeMode mode )
{
  if ( mode != mResizeMode )
  {
    mResizeMode = mode;
    recalculateFrameSizes();
    emit changed();
  }
}

QList<QgsLayoutFrame *> QgsLayoutMultiFrame::frames() const
{
  return mFrameItems;
}

void QgsLayoutMultiFrame::recalculateFrameSizes()
{
#if 0 //TODO
  if ( mFrameItems.empty() )
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
  QgsLayoutFrame *currentItem = nullptr;

  for ( int i = 0; i < mFrameItems.size(); ++i )
  {
    if ( mResizeMode != RepeatOnEveryPage && currentY >= totalHeight )
    {
      if ( mResizeMode == RepeatUntilFinished || mResizeMode == ExtendToNextPage ) //remove unneeded frames in extent mode
      {
        bool removingPages = true;
        for ( int j = mFrameItems.size(); j > i; --j )
        {
          int numPagesBefore = mLayout->pageCollection()->pageCount();
          removeFrame( j - 1, removingPages );
          //if removing the frame didn't also remove the page, then stop removing pages
          removingPages = removingPages && ( mLayout->pageCollection()->pageCount() < numPagesBefore );
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
    while ( ( mResizeMode == RepeatOnEveryPage ) || currentY < totalHeight )
    {
      //find out on which page the lower left point of the last frame is
      int page = std::floor( ( currentItem->pos().y() + currentItem->rect().height() ) / ( mLayout->paperHeight() + mComposition->spaceBetweenPages() ) ) + 1;

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
      QgsLayoutFrame *newFrame = createNewFrame( currentItem,
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
#endif
}

void QgsLayoutMultiFrame::recalculateFrameRects()
{
#if 0 //TODO
  if ( mFrameItems.empty() )
  {
    //no frames, nothing to do
    return;
  }

  const QList< QgsLayoutFrame * > frames = mFrameItems;
  for ( QgsLayoutFrame *frame : frames )
  {
    frame->setSceneRect( QRectF( frame->scenePos().x(), frame->scenePos().y(),
                                 frame->rect().width(), frame->rect().height() ) );
  }
#endif
}

QgsLayoutFrame *QgsLayoutMultiFrame::createNewFrame( QgsLayoutFrame *currentFrame, QPointF pos, QSizeF size )
{
  if ( !currentFrame )
  {
    return nullptr;
  }

  QgsLayoutFrame *newFrame = new QgsLayoutFrame( mLayout, this );
  newFrame->attemptSetSceneRect( QRectF( pos.x(), pos.y(), size.width(), size.height() ) );

  //copy some settings from the parent frame
  newFrame->setBackgroundColor( currentFrame->backgroundColor() );
  newFrame->setBackgroundEnabled( currentFrame->hasBackground() );
  newFrame->setBlendMode( currentFrame->blendMode() );
  newFrame->setFrameEnabled( currentFrame->hasFrame() );
  newFrame->setFrameStrokeColor( currentFrame->frameStrokeColor() );
  newFrame->setFrameJoinStyle( currentFrame->frameJoinStyle() );
  newFrame->setFrameStrokeWidth( currentFrame->frameStrokeWidth() );
  newFrame->setItemOpacity( currentFrame->itemOpacity() );
  newFrame->setHideBackgroundIfEmpty( currentFrame->hideBackgroundIfEmpty() );

  addFrame( newFrame, false );

  return newFrame;
}

QString QgsLayoutMultiFrame::displayName() const
{
  return tr( "<Multiframe>" );
}

void QgsLayoutMultiFrame::handleFrameRemoval()
{
  if ( mBlockUpdates )
    return;

  QgsLayoutFrame *frame = qobject_cast<QgsLayoutFrame *>( sender() );
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
  if ( !mFrameItems.isEmpty() )
  {
    if ( resizeMode() != QgsLayoutMultiFrame::RepeatOnEveryPage && !mIsRecalculatingSize )
    {
      //removing a frame forces the multi frame to UseExistingFrames resize mode
      //otherwise the frame may not actually be removed, leading to confusing ui behavior
      mResizeMode = QgsLayoutMultiFrame::UseExistingFrames;
      emit changed();
      recalculateFrameSizes();
    }
  }
}

void QgsLayoutMultiFrame::handlePageChange()
{
#if 0 //TODO
  if ( mLayout->pageCollection()->pageCount() < 1 )
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
    QgsLayoutFrame *frame = mFrameItems.at( i );
    int page = frame->pos().y() / ( mComposition->paperHeight() + mComposition->spaceBetweenPages() );
    if ( page > ( mComposition->numPages() - 1 ) )
    {
      removeFrame( i );
    }
  }

  //page number of the last item
  QgsLayoutFrame *lastFrame = mFrameItems.last();
  int lastItemPage = lastFrame->pos().y() / ( mLayout->paperHeight() + mLayout->spaceBetweenPages() );

  for ( int i = lastItemPage + 1; i < mLayout->pageCollection()->pageCount(); ++i )
  {
    //copy last frame to current page
    QgsLayoutFrame *newFrame = new QgsLayoutFrame( mLayout, this, lastFrame->pos().x(),
        lastFrame->pos().y() + mLayout->paperHeight() + mLayout->spaceBetweenPages(),
        lastFrame->rect().width(), lastFrame->rect().height() );
    addFrame( newFrame, false );
    lastFrame = newFrame;
  }

  recalculateFrameSizes();
  update();
#endif
}

void QgsLayoutMultiFrame::removeFrame( int i, const bool removeEmptyPages )
{
  if ( i >= mFrameItems.count() )
  {
    return;
  }

  QgsLayoutFrame *frameItem = mFrameItems.at( i );
  if ( mLayout )
  {
    mIsRecalculatingSize = true;
#if 0 //TODO
    int pageNumber = frameItem->page();
    //remove item, but don't create undo command
#if 0 //TODO - block undo commands
#endif
    mLayout->removeLayoutItem( frameItem );
    //if frame was the only item on the page, remove the page
    if ( removeEmptyPages && mComposition->pageIsEmpty( pageNumber ) )
    {
      mComposition->setNumPages( mComposition->numPages() - 1 );
    }
#endif
    mIsRecalculatingSize = false;
  }
  mFrameItems.removeAt( i );
}

void QgsLayoutMultiFrame::update()
{
  for ( QgsLayoutFrame *frame : qgis::as_const( mFrameItems ) )
  {
    frame->update();
  }
}

void QgsLayoutMultiFrame::deleteFrames()
{
  mBlockUpdates = true;
  ResizeMode bkResizeMode = mResizeMode;
  mResizeMode = UseExistingFrames;
  for ( QgsLayoutFrame *frame : qgis::as_const( mFrameItems ) )
  {
#if 0 //TODO -block undo commands
#endif
    mLayout->removeLayoutItem( frame );
  }
  mFrameItems.clear();
  mResizeMode = bkResizeMode;
  mBlockUpdates = false;
}

QgsLayoutFrame *QgsLayoutMultiFrame::frame( int i ) const
{
  if ( i < 0 || i >= mFrameItems.size() )
  {
    return nullptr;
  }
  return mFrameItems.at( i );
}

int QgsLayoutMultiFrame::frameIndex( QgsLayoutFrame *frame ) const
{
  return mFrameItems.indexOf( frame );
}

bool QgsLayoutMultiFrame::_writeXml( QDomElement &elem, QDomDocument &doc, bool ignoreFrames ) const
{
#if 0 //TODO
  elem.setAttribute( QStringLiteral( "resizeMode" ), mResizeMode );
  if ( !ignoreFrames )
  {
    QList<QgsComposerFrame *>::const_iterator frameIt = mFrameItems.constBegin();
    for ( ; frameIt != mFrameItems.constEnd(); ++frameIt )
    {
      ( *frameIt )->writeXml( elem, doc );
    }
  }
  QgsComposerObject::writeXml( elem, doc );
#endif
  return true;
}

bool QgsLayoutMultiFrame::_readXml( const QDomElement &itemElem, const QDomDocument &doc, bool ignoreFrames )
{
#if 0 //TODO
  QgsComposerObject::readXml( itemElem, doc );

  mResizeMode = static_cast< ResizeMode >( itemElem.attribute( QStringLiteral( "resizeMode" ), QStringLiteral( "0" ) ).toInt() );
  if ( !ignoreFrames )
  {
    QDomNodeList frameList = itemElem.elementsByTagName( QStringLiteral( "ComposerFrame" ) );
    for ( int i = 0; i < frameList.size(); ++i )
    {
      QDomElement frameElem = frameList.at( i ).toElement();
      QgsComposerFrame *newFrame = new QgsComposerFrame( mComposition, this, 0, 0, 0, 0 );
      newFrame->readXml( frameElem, doc );
      addFrame( newFrame, false );
    }

    //TODO - think there should be a recalculateFrameSizes() call here
  }
#endif
  return true;
}


