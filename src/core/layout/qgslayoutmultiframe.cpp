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
#include "qgslayoutmultiframeundocommand.h"
#include "qgslayoutframe.h"
#include "qgslayout.h"
#include <QtCore>

QgsLayoutMultiFrame::QgsLayoutMultiFrame( QgsLayout *layout )
  : QgsLayoutObject( layout )
  , mUuid( QUuid::createUuid().toString() )
{
  mLayout->addMultiFrame( this );

  connect( mLayout->pageCollection(), &QgsLayoutPageCollection::changed, this, &QgsLayoutMultiFrame::handlePageChange );
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
  connect( frame, &QgsLayoutItem::sizePositionChanged, this, &QgsLayoutMultiFrame::recalculateFrameSizes );
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
    mLayout->undoStack()->beginMacro( tr( "Change Resize Mode" ) );
    mResizeMode = mode;
    recalculateFrameSizes();
    mLayout->undoStack()->endMacro();
    emit changed();
  }
}

QList<QgsLayoutFrame *> QgsLayoutMultiFrame::frames() const
{
  return mFrameItems;
}

void QgsLayoutMultiFrame::recalculateFrameSizes()
{
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

  if ( mBlockUndoCommands )
    mLayout->undoStack()->blockCommands( true );

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
  if ( currentItem && mResizeMode != UseExistingFrames )
  {
    while ( ( mResizeMode == RepeatOnEveryPage ) || currentY < totalHeight )
    {
      //find out on which page the lower left point of the last frame is
      int page = mLayout->pageCollection()->predictPageNumberForPoint( QPointF( 0, currentItem->pos().y() + currentItem->rect().height() ) ) + 1;

      if ( mResizeMode == RepeatOnEveryPage )
      {
        if ( page >= mLayout->pageCollection()->pageCount() )
        {
          break;
        }
      }
      else
      {
        //add new pages if required
        for ( int p = mLayout->pageCollection()->pageCount() - 1 ; p < page; ++p )
        {
          mLayout->pageCollection()->extendByNewPage();
        }
      }

      double currentPageHeight = mLayout->pageCollection()->page( page )->rect().height();

      double frameHeight = 0;
      switch ( mResizeMode )
      {
        case RepeatUntilFinished:
        case RepeatOnEveryPage:
        {
          frameHeight = currentItem->rect().height();
          break;
        }
        case ExtendToNextPage:
        {
          frameHeight = ( currentY + currentPageHeight ) > totalHeight ?  totalHeight - currentY : currentPageHeight;
          break;
        }

        case UseExistingFrames:
          break;
      }

      double newFrameY = mLayout->pageCollection()->page( page )->pos().y();
      if ( mResizeMode == RepeatUntilFinished || mResizeMode == RepeatOnEveryPage )
      {
        newFrameY += currentItem->pagePos().y();
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

  if ( mBlockUndoCommands )
    mLayout->undoStack()->blockCommands( false );
}

void QgsLayoutMultiFrame::recalculateFrameRects()
{
  if ( mFrameItems.empty() )
  {
    //no frames, nothing to do
    return;
  }

  const QList< QgsLayoutFrame * > frames = mFrameItems;
  for ( QgsLayoutFrame *frame : frames )
  {
    frame->refreshItemSize();
  }
}

void QgsLayoutMultiFrame::refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty )
{

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

QgsAbstractLayoutUndoCommand *QgsLayoutMultiFrame::createCommand( const QString &text, int id, QUndoCommand *parent )
{
  return new QgsLayoutMultiFrameUndoCommand( this, text, id, parent );
}

void QgsLayoutMultiFrame::beginCommand( const QString &commandText, QgsLayoutMultiFrame::UndoCommand command )
{
  if ( !mLayout )
    return;

  mLayout->undoStack()->beginCommand( this, commandText, command );
}

void QgsLayoutMultiFrame::endCommand()
{
  if ( mLayout )
    mLayout->undoStack()->endCommand();
}

void QgsLayoutMultiFrame::cancelCommand()
{
  if ( mLayout )
    mLayout->undoStack()->cancelCommand();
}

void QgsLayoutMultiFrame::finalizeRestoreFromXml()
{
}

void QgsLayoutMultiFrame::refresh()
{
  QgsLayoutObject::refresh();
  refreshDataDefinedProperty();
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
    int page = mLayout->pageCollection()->predictPageNumberForPoint( frame->pos() );
    if ( page >= mLayout->pageCollection()->pageCount() )
    {
      removeFrame( i );
    }
  }

  //page number of the last item
  QgsLayoutFrame *lastFrame = mFrameItems.last();
  int lastItemPage = mLayout->pageCollection()->predictPageNumberForPoint( lastFrame->pos() );

  for ( int i = lastItemPage + 1; i < mLayout->pageCollection()->pageCount(); ++i )
  {
    //copy last frame to current page
    std::unique_ptr< QgsLayoutFrame > newFrame = qgis::make_unique< QgsLayoutFrame >( mLayout, this );

    newFrame->attemptSetSceneRect( QRectF( lastFrame->pos().x(),
                                           mLayout->pageCollection()->page( i )->pos().y() + lastFrame->pagePos().y(),
                                           lastFrame->rect().width(), lastFrame->rect().height() ) );
    lastFrame = newFrame.get();
    addFrame( newFrame.release(), false );
  }

  recalculateFrameSizes();
  update();
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
    int pageNumber = frameItem->page();
    //remove item, but don't create undo command
    mLayout->undoStack()->blockCommands( true );
    mLayout->removeLayoutItem( frameItem );
    //if frame was the only item on the page, remove the page
    if ( removeEmptyPages && mLayout->pageCollection()->pageIsEmpty( pageNumber ) )
    {
      mLayout->pageCollection()->deletePage( pageNumber );
    }
    mLayout->undoStack()->blockCommands( false );
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
  mLayout->undoStack()->blockCommands( true );
  for ( QgsLayoutFrame *frame : qgis::as_const( mFrameItems ) )
  {
    mLayout->removeLayoutItem( frame );
  }
  mLayout->undoStack()->blockCommands( false );
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

bool QgsLayoutMultiFrame::writeXml( QDomElement &parentElement, QDomDocument &doc, const QgsReadWriteContext &context, bool ignoreFrames ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "LayoutMultiFrame" ) );
  element.setAttribute( QStringLiteral( "resizeMode" ), mResizeMode );
  element.setAttribute( QStringLiteral( "uuid" ), mUuid );
  element.setAttribute( QStringLiteral( "type" ), type() );

#if 0 //TODO

  if ( !ignoreFrames )
  {
    QList<QgsComposerFrame *>::const_iterator frameIt = mFrameItems.constBegin();
    for ( ; frameIt != mFrameItems.constEnd(); ++frameIt )
    {
      ( *frameIt )->writeXml( elem, doc );
    }
  }
#endif

  writeObjectPropertiesToElement( element, doc, context );
  writePropertiesToElement( element, doc, context );
  parentElement.appendChild( element );
  return true;
}

bool QgsLayoutMultiFrame::readXml( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context, bool ignoreFrames )
{
  if ( element.nodeName() != QStringLiteral( "LayoutMultiFrame" ) )
  {
    return false;
  }

  mBlockUndoCommands = true;
  mLayout->undoStack()->blockCommands( true );

  readObjectPropertiesFromElement( element, doc, context );

  mUuid = element.attribute( QStringLiteral( "uuid" ), QUuid::createUuid().toString() );
  mResizeMode = static_cast< ResizeMode >( element.attribute( QStringLiteral( "resizeMode" ), QStringLiteral( "0" ) ).toInt() );
#if 0 //TODO
  if ( !ignoreFrames )
  {
    deleteFrames();
  }


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


  bool result = readPropertiesFromElement( element, doc, context );

  mBlockUndoCommands = false;
  mLayout->undoStack()->blockCommands( false );
  return result;
}

bool QgsLayoutMultiFrame::writePropertiesToElement( QDomElement &, QDomDocument &, const QgsReadWriteContext & ) const
{
  return true;
}

bool QgsLayoutMultiFrame::readPropertiesFromElement( const QDomElement &, const QDomDocument &, const QgsReadWriteContext & )
{

  return true;
}

