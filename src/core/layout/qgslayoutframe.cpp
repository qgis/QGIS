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
#include "qgsexpressioncontextutils.h"

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
  const int frameIndex = mMultiFrame->frameIndex( const_cast< QgsLayoutFrame * >( this ) );
  //check minimum size
  return QgsLayoutSize( mMultiFrame->minFrameSize( frameIndex ), QgsUnitTypes::LayoutMillimeters );
}

QgsLayoutSize QgsLayoutFrame::fixedSize() const
{
  if ( !mMultiFrame )
    return QgsLayoutSize();

  //calculate index of frame
  const int frameIndex = mMultiFrame->frameIndex( const_cast< QgsLayoutFrame * >( this ) );
  //check fixed size
  return QgsLayoutSize( mMultiFrame->fixedFrameSize( frameIndex ), QgsUnitTypes::LayoutMillimeters );
}

int QgsLayoutFrame::type() const
{
  return QgsLayoutItemRegistry::LayoutFrame;
}

QIcon QgsLayoutFrame::icon() const
{
  if ( mMultiFrame )
    return mMultiFrame->icon();
  else
    return QIcon();
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

  const double multiFrameHeight = mMultiFrame->totalSize().height();
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

  //add frame's individual context
  context.appendScope( QgsExpressionContextUtils::layoutItemScope( this ) );

  return context;
}

QgsLayoutItem::ExportLayerBehavior QgsLayoutFrame::exportLayerBehavior() const
{
  return CanGroupWithItemsOfSameType;
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

void QgsLayoutFrame::cleanup()
{
  if ( mMultiFrame )
    mMultiFrame->handleFrameRemoval( this );

  QgsLayoutItem::cleanup();
}

void QgsLayoutFrame::draw( QgsLayoutItemRenderContext &context )
{
  if ( mMultiFrame )
  {
    //calculate index of frame
    const int frameIndex = mMultiFrame->frameIndex( this );
    Q_ASSERT_X( frameIndex >= 0, "QgsLayoutFrame::draw", "Invalid frame index for frame" );
    mMultiFrame->render( context, mSection, frameIndex );
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

bool QgsLayoutFrame::writePropertiesToElement( QDomElement &parentElement, QDomDocument &, const QgsReadWriteContext & ) const
{
  parentElement.setAttribute( QStringLiteral( "multiFrame" ), mMultiFrameUuid );
  parentElement.setAttribute( QStringLiteral( "multiFrameTemplateUuid" ), mMultiFrameUuid );
  parentElement.setAttribute( QStringLiteral( "sectionX" ), QString::number( mSection.x() ) );
  parentElement.setAttribute( QStringLiteral( "sectionY" ), QString::number( mSection.y() ) );
  parentElement.setAttribute( QStringLiteral( "sectionWidth" ), QString::number( mSection.width() ) );
  parentElement.setAttribute( QStringLiteral( "sectionHeight" ), QString::number( mSection.height() ) );
  parentElement.setAttribute( QStringLiteral( "hidePageIfEmpty" ), mHidePageIfEmpty );
  parentElement.setAttribute( QStringLiteral( "hideBackgroundIfEmpty" ), mHideBackgroundIfEmpty );
  return true;
}

bool QgsLayoutFrame::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext & )
{
  const double x = itemElem.attribute( QStringLiteral( "sectionX" ) ).toDouble();
  const double y = itemElem.attribute( QStringLiteral( "sectionY" ) ).toDouble();
  const double width = itemElem.attribute( QStringLiteral( "sectionWidth" ) ).toDouble();
  const double height = itemElem.attribute( QStringLiteral( "sectionHeight" ) ).toDouble();
  mSection = QRectF( x, y, width, height );
  mHidePageIfEmpty = itemElem.attribute( QStringLiteral( "hidePageIfEmpty" ), QStringLiteral( "0" ) ).toInt();
  mHideBackgroundIfEmpty = itemElem.attribute( QStringLiteral( "hideBackgroundIfEmpty" ), QStringLiteral( "0" ) ).toInt();

  mMultiFrameUuid = itemElem.attribute( QStringLiteral( "multiFrame" ) );
  if ( mMultiFrameUuid.isEmpty( ) )
  {
    mMultiFrameUuid = itemElem.attribute( QStringLiteral( "multiFrameTemplateUuid" ) );
  }
  mMultiFrame = mLayout->multiFrameByUuid( mMultiFrameUuid );
  return true;
}
