/***************************************************************************
                              qgslayout.cpp
                             -------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayout.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutguidecollection.h"

QgsLayout::QgsLayout( QgsProject *project )
  : QGraphicsScene()
  , mProject( project )
  , mSnapper( QgsLayoutSnapper( this ) )
  , mPageCollection( new QgsLayoutPageCollection( this ) )
  , mGuideCollection( new QgsLayoutGuideCollection( this, mPageCollection.get() ) )
{
  // just to make sure - this should be the default, but maybe it'll change in some future Qt version...
  setBackgroundBrush( Qt::NoBrush );
}

QgsLayout::~QgsLayout()
{
  // delete guide collection FIRST, since it depends on the page collection
  mGuideCollection.reset();
}

void QgsLayout::initializeDefaults()
{
  // default to a A4 landscape page
  QgsLayoutItemPage *page = new QgsLayoutItemPage( this );
  page->setPageSize( QgsLayoutSize( 297, 210, QgsUnitTypes::LayoutMillimeters ) );
  mPageCollection->addPage( page );
}

QgsProject *QgsLayout::project() const
{
  return mProject;
}

double QgsLayout::convertToLayoutUnits( const QgsLayoutMeasurement &measurement ) const
{
  return mContext.measurementConverter().convert( measurement, mUnits ).length();
}

QSizeF QgsLayout::convertToLayoutUnits( const QgsLayoutSize &size ) const
{
  return mContext.measurementConverter().convert( size, mUnits ).toQSizeF();
}

QPointF QgsLayout::convertToLayoutUnits( const QgsLayoutPoint &point ) const
{
  return mContext.measurementConverter().convert( point, mUnits ).toQPointF();
}

QgsLayoutMeasurement QgsLayout::convertFromLayoutUnits( const double length, const QgsUnitTypes::LayoutUnit unit ) const
{
  return mContext.measurementConverter().convert( QgsLayoutMeasurement( length, mUnits ), unit );
}

QgsLayoutSize QgsLayout::convertFromLayoutUnits( const QSizeF &size, const QgsUnitTypes::LayoutUnit unit ) const
{
  return mContext.measurementConverter().convert( QgsLayoutSize( size.width(), size.height(), mUnits ), unit );
}

QgsLayoutPoint QgsLayout::convertFromLayoutUnits( const QPointF &point, const QgsUnitTypes::LayoutUnit unit ) const
{
  return mContext.measurementConverter().convert( QgsLayoutPoint( point.x(), point.y(), mUnits ), unit );
}

QgsLayoutGuideCollection &QgsLayout::guides()
{
  return *mGuideCollection;
}

const QgsLayoutGuideCollection &QgsLayout::guides() const
{
  return *mGuideCollection;
}

QgsExpressionContext QgsLayout::createExpressionContext() const
{
  QgsExpressionContext context = QgsExpressionContext();
  context.appendScope( QgsExpressionContextUtils::globalScope() );
  context.appendScope( QgsExpressionContextUtils::projectScope( mProject ) );
  context.appendScope( QgsExpressionContextUtils::layoutScope( this ) );
#if 0 //TODO
  if ( mAtlasComposition.enabled() )
  {
    context.appendScope( QgsExpressionContextUtils::atlasScope( &mAtlasComposition ) );
  }
#endif
  return context;
}

void QgsLayout::setCustomProperty( const QString &key, const QVariant &value )
{
  mCustomProperties.setValue( key, value );

  if ( key.startsWith( QLatin1String( "variable" ) ) )
    emit variablesChanged();
}

QVariant QgsLayout::customProperty( const QString &key, const QVariant &defaultValue ) const
{
  return mCustomProperties.value( key, defaultValue );
}

void QgsLayout::removeCustomProperty( const QString &key )
{
  mCustomProperties.remove( key );
}

QStringList QgsLayout::customProperties() const
{
  return mCustomProperties.keys();
}

QgsLayoutItemMap *QgsLayout::referenceMap() const
{
  return nullptr;
}

void QgsLayout::setReferenceMap( QgsLayoutItemMap *map )
{
  Q_UNUSED( map );
}

QgsLayoutPageCollection *QgsLayout::pageCollection()
{
  return mPageCollection.get();
}

const QgsLayoutPageCollection *QgsLayout::pageCollection() const
{
  return mPageCollection.get();
}

QRectF QgsLayout::layoutBounds( bool ignorePages, double margin ) const
{
  //start with an empty rectangle
  QRectF bounds;

  //add all QgsComposerItems and QgsPaperItems which are in the composition
  Q_FOREACH ( const QGraphicsItem *item, items() )
  {
    const QgsLayoutItem *layoutItem = dynamic_cast<const QgsLayoutItem *>( item );
    if ( !layoutItem )
      continue;

    bool isPage = layoutItem->type() == QgsLayoutItemRegistry::LayoutPage;
    if ( !isPage || !ignorePages )
    {
      //expand bounds with current item's bounds
      QRectF itemBounds;
      if ( isPage )
      {
        // for pages we only consider the item's rect - not the bounding rect
        // as the bounding rect contains extra padding
        itemBounds = layoutItem->mapToScene( layoutItem->rect() ).boundingRect();
      }
      else
        itemBounds = item->sceneBoundingRect();

      if ( bounds.isValid() )
        bounds = bounds.united( itemBounds );
      else
        bounds = itemBounds;
    }
  }

  if ( bounds.isValid() && margin > 0.0 )
  {
    //finally, expand bounds out by specified margin of page size
    double maxWidth = mPageCollection->maximumPageWidth();
    bounds.adjust( -maxWidth * margin, -maxWidth * margin, maxWidth * margin, maxWidth * margin );
  }

  return bounds;

}

void QgsLayout::addLayoutItem( QgsLayoutItem *item )
{
  addItem( item );
  updateBounds();
}

void QgsLayout::updateBounds()
{
  setSceneRect( layoutBounds( false, 0.05 ) );
}
