/***************************************************************************
                         qgslayoutitemattributetable.cpp
                         -------------------------------
    begin                : November 2017
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

#include "qgslayoutitemattributetable.h"
#include "qgslayout.h"
#include "qgslayouttablecolumn.h"
#include "qgslayoutitemmap.h"
#include "qgslayoututils.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"
#include "qgslayoutframe.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsgeometry.h"
#include "qgsexception.h"
#include "qgsmapsettings.h"

//QgsLayoutAttributeTableCompare

///@cond PRIVATE

/**
 * Helper class for sorting tables, takes into account sorting column and ascending / descending
*/
class CORE_EXPORT QgsLayoutAttributeTableCompare
{
  public:

    /**
     * Constructor for QgsLayoutAttributeTableCompare.
     */
    QgsLayoutAttributeTableCompare() = default;
    bool operator()( const QgsLayoutTableRow &m1, const QgsLayoutTableRow &m2 )
    {
      return ( mAscending ? qgsVariantLessThan( m1[mCurrentSortColumn], m2[mCurrentSortColumn] )
               : qgsVariantGreaterThan( m1[mCurrentSortColumn], m2[mCurrentSortColumn] ) );
    }

    /**
     * Sets \a column number to sort by.
     */
    void setSortColumn( int column ) { mCurrentSortColumn = column; }

    /**
     * Sets sort order for column sorting
     * Set \a ascending to true to sort in ascending order, false to sort in descending order
     */
    void setAscending( bool ascending ) { mAscending = ascending; }

  private:
    int mCurrentSortColumn = 0;
    bool mAscending = true;
};

///@endcond

//
// QgsLayoutItemAttributeTable
//

QgsLayoutItemAttributeTable::QgsLayoutItemAttributeTable( QgsLayout *layout )
  : QgsLayoutTable( layout )
{
  if ( mLayout )
  {
    connect( mLayout->project(), static_cast < void ( QgsProject::* )( const QString & ) >( &QgsProject::layerWillBeRemoved ), this, &QgsLayoutItemAttributeTable::removeLayer );

    //coverage layer change = regenerate columns
    connect( &mLayout->reportContext(), &QgsLayoutReportContext::layerChanged, this, &QgsLayoutItemAttributeTable::atlasLayerChanged );
  }
  refreshAttributes();
}

int QgsLayoutItemAttributeTable::type() const
{
  return QgsLayoutItemRegistry::LayoutAttributeTable;
}

QIcon QgsLayoutItemAttributeTable::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemTable.svg" ) );
}

QgsLayoutItemAttributeTable *QgsLayoutItemAttributeTable::create( QgsLayout *layout )
{
  return new QgsLayoutItemAttributeTable( layout );
}

QString QgsLayoutItemAttributeTable::displayName() const
{
  return tr( "<Attribute table frame>" );
}

void QgsLayoutItemAttributeTable::setVectorLayer( QgsVectorLayer *layer )
{
  if ( layer == mVectorLayer.get() )
  {
    //no change
    return;
  }

  QgsVectorLayer *prevLayer = sourceLayer();
  mVectorLayer.setLayer( layer );

  if ( mSource == QgsLayoutItemAttributeTable::LayerAttributes && layer != prevLayer )
  {
    if ( prevLayer )
    {
      //disconnect from previous layer
      disconnect( prevLayer, &QgsVectorLayer::layerModified, this, &QgsLayoutTable::refreshAttributes );
    }

    //rebuild column list to match all columns from layer
    resetColumns();

    //listen for modifications to layer and refresh table when they occur
    connect( mVectorLayer.get(), &QgsVectorLayer::layerModified, this, &QgsLayoutTable::refreshAttributes );
  }

  refreshAttributes();
  emit changed();
}

void QgsLayoutItemAttributeTable::setRelationId( const QString &relationId )
{
  if ( relationId == mRelationId )
  {
    //no change
    return;
  }

  QgsVectorLayer *prevLayer = sourceLayer();
  mRelationId = relationId;
  QgsRelation relation = mLayout->project()->relationManager()->relation( mRelationId );
  QgsVectorLayer *newLayer = relation.referencingLayer();

  if ( mSource == QgsLayoutItemAttributeTable::RelationChildren && newLayer != prevLayer )
  {
    if ( prevLayer )
    {
      //disconnect from previous layer
      disconnect( prevLayer, &QgsVectorLayer::layerModified, this, &QgsLayoutTable::refreshAttributes );
    }

    //rebuild column list to match all columns from layer
    resetColumns();

    //listen for modifications to layer and refresh table when they occur
    connect( newLayer, &QgsVectorLayer::layerModified, this, &QgsLayoutTable::refreshAttributes );
  }

  refreshAttributes();
  emit changed();
}

void QgsLayoutItemAttributeTable::atlasLayerChanged( QgsVectorLayer *layer )
{
  if ( mSource != QgsLayoutItemAttributeTable::AtlasFeature || layer == mCurrentAtlasLayer )
  {
    //nothing to do
    return;
  }

  //atlas feature mode, atlas layer changed, so we need to reset columns
  if ( mCurrentAtlasLayer )
  {
    //disconnect from previous layer
    disconnect( mCurrentAtlasLayer, &QgsVectorLayer::layerModified, this, &QgsLayoutTable::refreshAttributes );
  }

  mCurrentAtlasLayer = layer;

  //rebuild column list to match all columns from layer
  resetColumns();
  refreshAttributes();

  //listen for modifications to layer and refresh table when they occur
  connect( layer, &QgsVectorLayer::layerModified, this, &QgsLayoutTable::refreshAttributes );
}

void QgsLayoutItemAttributeTable::resetColumns()
{
  QgsVectorLayer *source = sourceLayer();
  if ( !source )
  {
    return;
  }

  //remove existing columns
  qDeleteAll( mColumns );
  mColumns.clear();

  //rebuild columns list from vector layer fields
  int idx = 0;
  const QgsFields sourceFields = source->fields();
  for ( const auto &field : sourceFields )
  {
    QString currentAlias = source->attributeDisplayName( idx );
    std::unique_ptr< QgsLayoutTableColumn > col = qgis::make_unique< QgsLayoutTableColumn >();
    col->setAttribute( field.name() );
    col->setHeading( currentAlias );
    mColumns.append( col.release() );
    idx++;
  }
}

void QgsLayoutItemAttributeTable::setMap( QgsLayoutItemMap *map )
{
  if ( map == mMap )
  {
    //no change
    return;
  }

  if ( mMap )
  {
    //disconnect from previous map
    disconnect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutTable::refreshAttributes );
  }
  mMap = map;
  if ( mMap )
  {
    //listen out for extent changes in linked map
    connect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutTable::refreshAttributes );
  }
  refreshAttributes();
  emit changed();
}

void QgsLayoutItemAttributeTable::setMaximumNumberOfFeatures( const int features )
{
  if ( features == mMaximumNumberOfFeatures )
  {
    return;
  }

  mMaximumNumberOfFeatures = features;
  refreshAttributes();
  emit changed();
}

void QgsLayoutItemAttributeTable::setUniqueRowsOnly( const bool uniqueOnly )
{
  if ( uniqueOnly == mShowUniqueRowsOnly )
  {
    return;
  }

  mShowUniqueRowsOnly = uniqueOnly;
  refreshAttributes();
  emit changed();
}

void QgsLayoutItemAttributeTable::setDisplayOnlyVisibleFeatures( const bool visibleOnly )
{
  if ( visibleOnly == mShowOnlyVisibleFeatures )
  {
    return;
  }

  mShowOnlyVisibleFeatures = visibleOnly;
  refreshAttributes();
  emit changed();
}

void QgsLayoutItemAttributeTable::setFilterToAtlasFeature( const bool filterToAtlas )
{
  if ( filterToAtlas == mFilterToAtlasIntersection )
  {
    return;
  }

  mFilterToAtlasIntersection = filterToAtlas;
  refreshAttributes();
  emit changed();
}

void QgsLayoutItemAttributeTable::setFilterFeatures( const bool filter )
{
  if ( filter == mFilterFeatures )
  {
    return;
  }

  mFilterFeatures = filter;
  refreshAttributes();
  emit changed();
}

void QgsLayoutItemAttributeTable::setFeatureFilter( const QString &expression )
{
  if ( expression == mFeatureFilter )
  {
    return;
  }

  mFeatureFilter = expression;
  refreshAttributes();
  emit changed();
}

void QgsLayoutItemAttributeTable::setDisplayedFields( const QStringList &fields, bool refresh )
{
  QgsVectorLayer *source = sourceLayer();
  if ( !source )
  {
    return;
  }

  //rebuild columns list, taking only fields contained in supplied list
  qDeleteAll( mColumns );
  mColumns.clear();

  const QgsFields layerFields = source->fields();

  if ( !fields.isEmpty() )
  {
    for ( const QString &field : fields )
    {
      int attrIdx = layerFields.lookupField( field );
      if ( attrIdx < 0 )
        continue;

      QString currentAlias = source->attributeDisplayName( attrIdx );
      std::unique_ptr< QgsLayoutTableColumn > col = qgis::make_unique< QgsLayoutTableColumn >();
      col->setAttribute( layerFields.at( attrIdx ).name() );
      col->setHeading( currentAlias );
      mColumns.append( col.release() );
    }
  }
  else
  {
    //resetting, so add all attributes to columns
    int idx = 0;
    for ( const QgsField &field : layerFields )
    {
      QString currentAlias = source->attributeDisplayName( idx );
      std::unique_ptr< QgsLayoutTableColumn > col = qgis::make_unique< QgsLayoutTableColumn >();
      col->setAttribute( field.name() );
      col->setHeading( currentAlias );
      mColumns.append( col.release() );
      idx++;
    }
  }

  if ( refresh )
  {
    refreshAttributes();
  }
}

void QgsLayoutItemAttributeTable::restoreFieldAliasMap( const QMap<int, QString> &map )
{
  QgsVectorLayer *source = sourceLayer();
  if ( !source )
  {
    return;
  }

  for ( QgsLayoutTableColumn *column : qgis::as_const( mColumns ) )
  {
    int attrIdx = source->fields().lookupField( column->attribute() );
    if ( map.contains( attrIdx ) )
    {
      column->setHeading( map.value( attrIdx ) );
    }
    else
    {
      column->setHeading( source->attributeDisplayName( attrIdx ) );
    }
  }
}

bool QgsLayoutItemAttributeTable::getTableContents( QgsLayoutTableContents &contents )
{
  contents.clear();

  QgsVectorLayer *layer = sourceLayer();
  if ( !layer )
  {
    //no source layer
    return false;
  }

  QgsExpressionContext context = createExpressionContext();
  context.setFields( layer->fields() );

  QgsFeatureRequest req;

  //prepare filter expression
  std::unique_ptr<QgsExpression> filterExpression;
  bool activeFilter = false;
  if ( mFilterFeatures && !mFeatureFilter.isEmpty() )
  {
    filterExpression = qgis::make_unique< QgsExpression >( mFeatureFilter );
    if ( !filterExpression->hasParserError() )
    {
      activeFilter = true;
      req.setFilterExpression( mFeatureFilter );
    }
  }

  QgsRectangle selectionRect;
  if ( mMap && mShowOnlyVisibleFeatures )
  {
    selectionRect = mMap->extent();
    if ( layer )
    {
      //transform back to layer CRS
      QgsCoordinateTransform coordTransform( layer->crs(), mMap->crs(), mLayout->project() );
      try
      {
        selectionRect = coordTransform.transformBoundingBox( selectionRect, QgsCoordinateTransform::ReverseTransform );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
        return false;
      }
    }
  }

  if ( mSource == QgsLayoutItemAttributeTable::RelationChildren )
  {
    QgsRelation relation = mLayout->project()->relationManager()->relation( mRelationId );
    QgsFeature atlasFeature = mLayout->reportContext().feature();
    req = relation.getRelatedFeaturesRequest( atlasFeature );
  }

  if ( !selectionRect.isEmpty() )
    req.setFilterRect( selectionRect );

  req.setFlags( mShowOnlyVisibleFeatures ? QgsFeatureRequest::ExactIntersect : QgsFeatureRequest::NoFlags );

  if ( mSource == QgsLayoutItemAttributeTable::AtlasFeature )
  {
    //source mode is current atlas feature
    QgsFeature atlasFeature = mLayout->reportContext().feature();
    req.setFilterFid( atlasFeature.id() );
  }

  QgsFeature f;
  int counter = 0;
  QgsFeatureIterator fit = layer->getFeatures( req );

  while ( fit.nextFeature( f ) && counter < mMaximumNumberOfFeatures )
  {
    context.setFeature( f );
    //check feature against filter
    if ( activeFilter && filterExpression )
    {
      QVariant result = filterExpression->evaluate( &context );
      // skip this feature if the filter evaluation is false
      if ( !result.toBool() )
      {
        continue;
      }
    }
    //check against atlas feature intersection
    if ( mFilterToAtlasIntersection )
    {
      if ( !f.hasGeometry() )
      {
        continue;
      }
      QgsFeature atlasFeature = mLayout->reportContext().feature();
      if ( !atlasFeature.hasGeometry() ||
           !f.geometry().intersects( atlasFeature.geometry() ) )
      {
        //feature falls outside current atlas feature
        continue;
      }
    }

    QgsLayoutTableRow currentRow;

    for ( QgsLayoutTableColumn *column : qgis::as_const( mColumns ) )
    {
      int idx = layer->fields().lookupField( column->attribute() );
      if ( idx != -1 )
      {
        currentRow << replaceWrapChar( f.attributes().at( idx ) );
      }
      else
      {
        // Lets assume it's an expression
        std::unique_ptr< QgsExpression > expression = qgis::make_unique< QgsExpression >( column->attribute() );
        context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "row_number" ), counter + 1, true ) );
        expression->prepare( &context );
        QVariant value = expression->evaluate( &context );
        currentRow << value;
      }
    }

    if ( !mShowUniqueRowsOnly || !contentsContainsRow( contents, currentRow ) )
    {
      contents << currentRow;
      ++counter;
    }
  }

  //sort the list, starting with the last attribute
  QgsLayoutAttributeTableCompare c;
  QVector< QPair<int, bool> > sortColumns = sortAttributes();
  for ( int i = sortColumns.size() - 1; i >= 0; --i )
  {
    c.setSortColumn( sortColumns.at( i ).first );
    c.setAscending( sortColumns.at( i ).second );
    std::stable_sort( contents.begin(), contents.end(), c );
  }

  recalculateTableSize();
  return true;
}

QgsExpressionContext QgsLayoutItemAttributeTable::createExpressionContext() const
{
  QgsExpressionContext context = QgsLayoutTable::createExpressionContext();

  if ( mSource == LayerAttributes )
  {
    context.appendScope( QgsExpressionContextUtils::layerScope( mVectorLayer.get() ) );
  }

  return context;
}

void QgsLayoutItemAttributeTable::finalizeRestoreFromXml()
{
  QgsLayoutTable::finalizeRestoreFromXml();
  if ( !mMap && !mMapUuid.isEmpty() && mLayout )
  {
    mMap = qobject_cast< QgsLayoutItemMap *>( mLayout->itemByUuid( mMapUuid, true ) );
    if ( mMap )
    {
      //if we have found a valid map item, listen out to extent changes on it and refresh the table
      connect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutTable::refreshAttributes );
    }
  }
}

void QgsLayoutItemAttributeTable::refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property )
{
  QgsExpressionContext context = createExpressionContext();

  if ( mSource == QgsLayoutItemAttributeTable::LayerAttributes &&
       ( property == QgsLayoutObject::AttributeTableSourceLayer || property == QgsLayoutObject::AllProperties ) )
  {
    mDataDefinedVectorLayer = nullptr;

    QString currentLayerIdentifier;
    if ( QgsVectorLayer *currentLayer = mVectorLayer.get() )
      currentLayerIdentifier = currentLayer->id();

    const QString layerIdentifier = mDataDefinedProperties.valueAsString( QgsLayoutObject::AttributeTableSourceLayer, context, currentLayerIdentifier );
    QgsVectorLayer *ddLayer = qobject_cast< QgsVectorLayer * >( QgsLayoutUtils::mapLayerFromString( layerIdentifier, mLayout->project() ) );
    if ( ddLayer )
      mDataDefinedVectorLayer = ddLayer;
  }

  QgsLayoutMultiFrame::refreshDataDefinedProperty( property );
}

QVariant QgsLayoutItemAttributeTable::replaceWrapChar( const QVariant &variant ) const
{
  //avoid converting variants to string if not required (try to maintain original type for sorting)
  if ( mWrapString.isEmpty() || !variant.toString().contains( mWrapString ) )
    return variant;

  QString replaced = variant.toString();
  replaced.replace( mWrapString, QLatin1String( "\n" ) );
  return replaced;
}

QgsVectorLayer *QgsLayoutItemAttributeTable::sourceLayer() const
{
  switch ( mSource )
  {
    case QgsLayoutItemAttributeTable::AtlasFeature:
      return mLayout->reportContext().layer();
    case QgsLayoutItemAttributeTable::LayerAttributes:
    {
      if ( mDataDefinedVectorLayer )
        return mDataDefinedVectorLayer;
      else
        return mVectorLayer.get();
    }
    case QgsLayoutItemAttributeTable::RelationChildren:
    {
      QgsRelation relation = mLayout->project()->relationManager()->relation( mRelationId );
      return relation.referencingLayer();
    }
  }
  return nullptr;
}

void QgsLayoutItemAttributeTable::removeLayer( const QString &layerId )
{
  if ( mVectorLayer && mSource == QgsLayoutItemAttributeTable::LayerAttributes )
  {
    if ( layerId == mVectorLayer->id() )
    {
      mVectorLayer.setLayer( nullptr );
      //remove existing columns
      qDeleteAll( mColumns );
      mColumns.clear();
    }
  }
}

static bool columnsBySortRank( QPair<int, QgsLayoutTableColumn * > a, QPair<int, QgsLayoutTableColumn * > b )
{
  return a.second->sortByRank() < b.second->sortByRank();
}

QVector<QPair<int, bool> > QgsLayoutItemAttributeTable::sortAttributes() const
{
  //generate list of all sorted columns
  QVector< QPair<int, QgsLayoutTableColumn * > > sortedColumns;
  int idx = 0;
  for ( QgsLayoutTableColumn *column : mColumns )
  {
    if ( column->sortByRank() > 0 )
    {
      sortedColumns.append( qMakePair( idx, column ) );
    }
    idx++;
  }

  //sort columns by rank
  std::sort( sortedColumns.begin(), sortedColumns.end(), columnsBySortRank );

  //generate list of column index, bool for sort direction (to match 2.0 api)
  QVector<QPair<int, bool> > attributesBySortRank;
  for ( auto &column : qgis::as_const( sortedColumns ) )
  {
    attributesBySortRank.append( qMakePair( column.first,
                                            column.second->sortOrder() == Qt::AscendingOrder ) );
  }
  return attributesBySortRank;
}

void QgsLayoutItemAttributeTable::setWrapString( const QString &wrapString )
{
  if ( wrapString == mWrapString )
  {
    return;
  }

  mWrapString = wrapString;
  refreshAttributes();
  emit changed();
}

bool QgsLayoutItemAttributeTable::writePropertiesToElement( QDomElement &tableElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  if ( !QgsLayoutTable::writePropertiesToElement( tableElem, doc, context ) )
    return false;

  tableElem.setAttribute( QStringLiteral( "source" ), QString::number( static_cast< int >( mSource ) ) );
  tableElem.setAttribute( QStringLiteral( "relationId" ), mRelationId );
  tableElem.setAttribute( QStringLiteral( "showUniqueRowsOnly" ), mShowUniqueRowsOnly );
  tableElem.setAttribute( QStringLiteral( "showOnlyVisibleFeatures" ), mShowOnlyVisibleFeatures );
  tableElem.setAttribute( QStringLiteral( "filterToAtlasIntersection" ), mFilterToAtlasIntersection );
  tableElem.setAttribute( QStringLiteral( "maxFeatures" ), mMaximumNumberOfFeatures );
  tableElem.setAttribute( QStringLiteral( "filterFeatures" ), mFilterFeatures ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  tableElem.setAttribute( QStringLiteral( "featureFilter" ), mFeatureFilter );
  tableElem.setAttribute( QStringLiteral( "wrapString" ), mWrapString );

  if ( mMap )
  {
    tableElem.setAttribute( QStringLiteral( "mapUuid" ), mMap->uuid() );
  }

  if ( mVectorLayer )
  {
    tableElem.setAttribute( QStringLiteral( "vectorLayer" ), mVectorLayer.layerId );
    tableElem.setAttribute( QStringLiteral( "vectorLayerName" ), mVectorLayer.name );
    tableElem.setAttribute( QStringLiteral( "vectorLayerSource" ), mVectorLayer.source );
    tableElem.setAttribute( QStringLiteral( "vectorLayerProvider" ), mVectorLayer.provider );
  }
  return true;
}

bool QgsLayoutItemAttributeTable::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  QgsVectorLayer *prevLayer = sourceLayer();
  if ( prevLayer )
  {
    //disconnect from previous layer
    disconnect( prevLayer, &QgsVectorLayer::layerModified, this, &QgsLayoutTable::refreshAttributes );
  }

  if ( !QgsLayoutTable::readPropertiesFromElement( itemElem, doc, context ) )
    return false;

  mSource = QgsLayoutItemAttributeTable::ContentSource( itemElem.attribute( QStringLiteral( "source" ), QStringLiteral( "0" ) ).toInt() );
  mRelationId = itemElem.attribute( QStringLiteral( "relationId" ), QString() );

  if ( mSource == QgsLayoutItemAttributeTable::AtlasFeature )
  {
    mCurrentAtlasLayer = mLayout->reportContext().layer();
  }

  mShowUniqueRowsOnly = itemElem.attribute( QStringLiteral( "showUniqueRowsOnly" ), QStringLiteral( "0" ) ).toInt();
  mShowOnlyVisibleFeatures = itemElem.attribute( QStringLiteral( "showOnlyVisibleFeatures" ), QStringLiteral( "1" ) ).toInt();
  mFilterToAtlasIntersection = itemElem.attribute( QStringLiteral( "filterToAtlasIntersection" ), QStringLiteral( "0" ) ).toInt();
  mFilterFeatures = itemElem.attribute( QStringLiteral( "filterFeatures" ), QStringLiteral( "false" ) ) == QLatin1String( "true" );
  mFeatureFilter = itemElem.attribute( QStringLiteral( "featureFilter" ), QString() );
  mMaximumNumberOfFeatures = itemElem.attribute( QStringLiteral( "maxFeatures" ), QStringLiteral( "5" ) ).toInt();
  mWrapString = itemElem.attribute( QStringLiteral( "wrapString" ) );

  //map
  mMapUuid = itemElem.attribute( QStringLiteral( "mapUuid" ) );
  if ( mMap )
  {
    disconnect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutTable::refreshAttributes );
    mMap = nullptr;
  }
  // setting new mMap occurs in finalizeRestoreFromXml

  //vector layer
  QString layerId = itemElem.attribute( QStringLiteral( "vectorLayer" ) );
  QString layerName = itemElem.attribute( QStringLiteral( "vectorLayerName" ) );
  QString layerSource = itemElem.attribute( QStringLiteral( "vectorLayerSource" ) );
  QString layerProvider = itemElem.attribute( QStringLiteral( "vectorLayerProvider" ) );
  mVectorLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );
  mVectorLayer.resolveWeakly( mLayout->project() );

  //connect to new layer
  connect( sourceLayer(), &QgsVectorLayer::layerModified, this, &QgsLayoutTable::refreshAttributes );

  refreshAttributes();

  emit changed();
  return true;
}

void QgsLayoutItemAttributeTable::setSource( const QgsLayoutItemAttributeTable::ContentSource source )
{
  if ( source == mSource )
  {
    return;
  }

  QgsVectorLayer *prevLayer = sourceLayer();
  mSource = source;
  QgsVectorLayer *newLayer = sourceLayer();

  if ( newLayer != prevLayer )
  {
    //disconnect from previous layer
    if ( prevLayer )
    {
      disconnect( prevLayer, &QgsVectorLayer::layerModified, this, &QgsLayoutTable::refreshAttributes );
    }

    //connect to new layer
    connect( newLayer, &QgsVectorLayer::layerModified, this, &QgsLayoutTable::refreshAttributes );
    if ( mSource == QgsLayoutItemAttributeTable::AtlasFeature )
    {
      mCurrentAtlasLayer = newLayer;
    }

    //layer has changed as a result of the source change, so reset column list
    resetColumns();
  }

  refreshAttributes();
  emit changed();
}
