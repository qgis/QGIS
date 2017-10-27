/***************************************************************************
                              qgslayoutframe.cpp
                              ------------------
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

#include "qgslayoutframe.h"
#include "qgslayoutmultiframe.h"
#include "qgslayoutitemregistry.h"
#include "qgslayout.h"

QgsLayoutFrame::QgsLayoutFrame( QgsLayout *layout, QgsLayoutMultiFrame *multiFrame )
  : QgsLayoutItem( layout )
  , mMultiFrame( multiFrame )
{

  //default to no background
  setBackgroundEnabled( false );

  if ( multiFrame )
  {
    //repaint frame when multiframe content changes
    connect( multiFrame, &QgsLayoutMultiFrame::contentsChanged, this, [ = ]
    {
      update();
    } );

#if 0 //TODO
    //force recalculation of rect, so that multiframe specified sizes can be applied
    setSceneRect( QRectF( pos().x(), pos().y(), rect().width(), rect().height() ) );
#endif
  }
}

QgsLayoutMultiFrame *QgsLayoutFrame::multiFrame() const
{
  return mMultiFrame;
}
#if 0// TODO
bool QgsLayoutFrame::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  QDomElement frameElem = doc.createElement( QStringLiteral( "ComposerFrame" ) );
  frameElem.setAttribute( QStringLiteral( "sectionX" ), QString::number( mSection.x() ) );
  frameElem.setAttribute( QStringLiteral( "sectionY" ), QString::number( mSection.y() ) );
  frameElem.setAttribute( QStringLiteral( "sectionWidth" ), QString::number( mSection.width() ) );
  frameElem.setAttribute( QStringLiteral( "sectionHeight" ), QString::number( mSection.height() ) );
  frameElem.setAttribute( QStringLiteral( "hidePageIfEmpty" ), mHidePageIfEmpty );
  frameElem.setAttribute( QStringLiteral( "hideBackgroundIfEmpty" ), mHideBackgroundIfEmpty );
  elem.appendChild( frameElem );

  return _writeXml( frameElem, doc );
}

bool QgsLayoutFrame::readXml( const QDomElement &itemElem, const QDomDocument &doc )
{
  double x = itemElem.attribute( QStringLiteral( "sectionX" ) ).toDouble();
  double y = itemElem.attribute( QStringLiteral( "sectionY" ) ).toDouble();
  double width = itemElem.attribute( QStringLiteral( "sectionWidth" ) ).toDouble();
  double height = itemElem.attribute( QStringLiteral( "sectionHeight" ) ).toDouble();
  mSection = QRectF( x, y, width, height );
  mHidePageIfEmpty = itemElem.attribute( QStringLiteral( "hidePageIfEmpty" ), QStringLiteral( "0" ) ).toInt();
  mHideBackgroundIfEmpty = itemElem.attribute( QStringLiteral( "hideBackgroundIfEmpty" ), QStringLiteral( "0" ) ).toInt();
  QDomElement composerItem = itemElem.firstChildElement( QStringLiteral( "ComposerItem" ) );
  if ( composerItem.isNull() )
  {
    return false;
  }
  return _readXml( composerItem, doc );
}
#endif

int QgsLayoutFrame::type() const
{
  return QgsLayoutItemRegistry::LayoutFrame;
}

QString QgsLayoutFrame::stringType() const
{
  return QStringLiteral( "ItemFrame" );
}

void QgsLayoutFrame::setHidePageIfEmpty( const bool hidePageIfEmpty )
{
  mHidePageIfEmpty = hidePageIfEmpty;
}

void QgsLayoutFrame::setHideBackgroundIfEmpty( const bool hideBackgroundIfEmpty )
{
  if ( hideBackgroundIfEmpty == mHideBackgroundIfEmpty )
  {
    return;
  }

  mHideBackgroundIfEmpty = hideBackgroundIfEmpty;
  update();
}

bool QgsLayoutFrame::isEmpty() const
{
  if ( !mMultiFrame )
  {
    return true;
  }

  double multiFrameHeight = mMultiFrame->totalSize().height();
  if ( multiFrameHeight <= mSection.top() )
  {
    //multiframe height is less than top of this frame's visible portion
    return true;
  }

  return false;

}

QgsExpressionContext QgsLayoutFrame::createExpressionContext() const
{
  if ( !mMultiFrame )
    return QgsLayoutItem::createExpressionContext();

  //start with multiframe's context
  QgsExpressionContext context = mMultiFrame->createExpressionContext();

#if 0 //TODO
  //add frame's individual context
  context.appendScope( QgsExpressionContextUtils::layoutItemScope( this ) );
#endif

  return context;
}


QString QgsLayoutFrame::displayName() const
{
  if ( !id().isEmpty() )
  {
    return id();
  }

  if ( mMultiFrame )
  {
    return mMultiFrame->displayName();
  }

  return tr( "<Frame>" );
}

#if 0 //TODO
void QgsLayoutFrame::setSceneRect( const QRectF &rectangle )
{
  QRectF fixedRect = rectangle;

  if ( mMultiFrame )
  {
    //calculate index of frame
    int frameIndex = mMultiFrame->frameIndex( this );

    QSizeF fixedSize = mMultiFrame->fixedFrameSize( frameIndex );
    if ( fixedSize.width() > 0 )
    {
      fixedRect.setWidth( fixedSize.width() );
    }
    if ( fixedSize.height() > 0 )
    {
      fixedRect.setHeight( fixedSize.height() );
    }

    //check minimum size
    QSizeF minSize = mMultiFrame->minFrameSize( frameIndex );
    fixedRect.setWidth( std::max( minSize.width(), fixedRect.width() ) );
    fixedRect.setHeight( std::max( minSize.height(), fixedRect.height() ) );
  }

  QgsComposerItem::setSceneRect( fixedRect );
}
#endif


void QgsLayoutFrame::draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle )
{
  if ( mMultiFrame )
  {
    //calculate index of frame
    int frameIndex = mMultiFrame->frameIndex( this );
    mMultiFrame->render( context, mSection, frameIndex, itemStyle );
  }
}

void QgsLayoutFrame::drawFrame( QgsRenderContext &context )
{
  if ( !isEmpty() || !mHideBackgroundIfEmpty )
  {
    QgsLayoutItem::drawFrame( context );
  }
}

void QgsLayoutFrame::drawBackground( QgsRenderContext &context )
{
  if ( !isEmpty() || !mHideBackgroundIfEmpty )
  {
    QgsLayoutItem::drawBackground( context );
  }
}

#if 0 //TODO
void QgsLayoutFrame::beginItemCommand( const QString &text )
{
  if ( mComposition )
  {
    mComposition->beginMultiFrameCommand( multiFrame(), text );
  }
}

void QgsLayoutFrame::endItemCommand()
{
  if ( mComposition )
  {
    mComposition->endMultiFrameCommand();
  }
}
#endif
