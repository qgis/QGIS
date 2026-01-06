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

#include "qgsexpressioncontextutils.h"
#include "qgslayout.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutmultiframe.h"

#include "moc_qgslayoutframe.cpp"

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
    connect( multiFrame, &QgsLayoutMultiFrame::contentsChanged, this, [this]
    {
      update();
    } );

    //force recalculation of rect, so that multiframe specified sizes can be applied
    refreshItemSize();
  }
}

QgsLayoutFrame::~QgsLayoutFrame()
{
  QgsLayoutFrame::cleanup();
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
  return QgsLayoutSize( mMultiFrame->minFrameSize( frameIndex ), Qgis::LayoutUnit::Millimeters );
}

QgsLayoutSize QgsLayoutFrame::fixedSize() const
{
  if ( !mMultiFrame )
    return QgsLayoutSize();

  //calculate index of frame
  const int frameIndex = mMultiFrame->frameIndex( const_cast< QgsLayoutFrame * >( this ) );
  //check fixed size
  return QgsLayoutSize( mMultiFrame->fixedFrameSize( frameIndex ), Qgis::LayoutUnit::Millimeters );
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
  mMultiFrame = nullptr;

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
  parentElement.setAttribute( u"multiFrame"_s, mMultiFrameUuid );
  parentElement.setAttribute( u"multiFrameTemplateUuid"_s, mMultiFrameUuid );
  parentElement.setAttribute( u"sectionX"_s, QString::number( mSection.x() ) );
  parentElement.setAttribute( u"sectionY"_s, QString::number( mSection.y() ) );
  parentElement.setAttribute( u"sectionWidth"_s, QString::number( mSection.width() ) );
  parentElement.setAttribute( u"sectionHeight"_s, QString::number( mSection.height() ) );
  parentElement.setAttribute( u"hidePageIfEmpty"_s, mHidePageIfEmpty );
  parentElement.setAttribute( u"hideBackgroundIfEmpty"_s, mHideBackgroundIfEmpty );
  return true;
}

bool QgsLayoutFrame::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext & )
{
  const double x = itemElem.attribute( u"sectionX"_s ).toDouble();
  const double y = itemElem.attribute( u"sectionY"_s ).toDouble();
  const double width = itemElem.attribute( u"sectionWidth"_s ).toDouble();
  const double height = itemElem.attribute( u"sectionHeight"_s ).toDouble();
  mSection = QRectF( x, y, width, height );
  mHidePageIfEmpty = itemElem.attribute( u"hidePageIfEmpty"_s, u"0"_s ).toInt();
  mHideBackgroundIfEmpty = itemElem.attribute( u"hideBackgroundIfEmpty"_s, u"0"_s ).toInt();

  mMultiFrameUuid = itemElem.attribute( u"multiFrame"_s );
  if ( mMultiFrameUuid.isEmpty( ) )
  {
    mMultiFrameUuid = itemElem.attribute( u"multiFrameTemplateUuid"_s );
  }
  mMultiFrame = mLayout->multiFrameByUuid( mMultiFrameUuid );
  return true;
}
