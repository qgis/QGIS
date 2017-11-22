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
  , mMultiFrameUuid( multiFrame ? multiFrame->uuid() : QString() )
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

    //force recalculation of rect, so that multiframe specified sizes can be applied
    refreshItemSize();
  }
}

QgsLayoutFrame *QgsLayoutFrame::create( QgsLayout *layout )
{
  return new QgsLayoutFrame( layout, nullptr );
}

QgsLayoutMultiFrame *QgsLayoutFrame::multiFrame() const
{
  return mMultiFrame;
}

QgsLayoutSize QgsLayoutFrame::minimumSize() const
{
  if ( !mMultiFrame )
    return QgsLayoutSize();

  //calculate index of frame
  int frameIndex = mMultiFrame->frameIndex( const_cast< QgsLayoutFrame * >( this ) );
  //check minimum size
  return QgsLayoutSize( mMultiFrame->minFrameSize( frameIndex ), QgsUnitTypes::LayoutMillimeters );
}

QgsLayoutSize QgsLayoutFrame::fixedSize() const
{
  if ( !mMultiFrame )
    return QgsLayoutSize();

  //calculate index of frame
  int frameIndex = mMultiFrame->frameIndex( const_cast< QgsLayoutFrame * >( this ) );
  //check fixed size
  return QgsLayoutSize( mMultiFrame->fixedFrameSize( frameIndex ), QgsUnitTypes::LayoutMillimeters );
}

#if 0// TODO - save/restore multiframe uuid!
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

QString QgsLayoutFrame::uuid() const
{
  if ( mMultiFrame )
    return mMultiFrame->uuid() + ':' + mMultiFrame->frameIndex( const_cast< QgsLayoutFrame * >( this ) );
  else
    return QgsLayoutItem::uuid();
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
