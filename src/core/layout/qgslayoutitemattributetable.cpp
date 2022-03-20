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
#include "qgsfieldformatter.h"
#include "qgsfieldformatterregistry.h"
#include "qgsgeometry.h"
#include "qgsexception.h"
#include "qgsmapsettings.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsgeometryengine.h"
#include "qgsconditionalstyle.h"

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

  const bool mustRebuildColumns = static_cast< bool >( mCurrentAtlasLayer ) || mColumns.empty();
  mCurrentAtlasLayer = layer;

  if ( mustRebuildColumns )
  {
    //rebuild column list to match all columns from layer
    resetColumns();
  }

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
  mColumns.clear();
  mSortColumns.clear();

  //rebuild columns list from vector layer fields
  int idx = 0;
  const QgsFields sourceFields = source->fields();

  for ( const auto &field : sourceFields )
  {
    QString currentAlias = source->attributeDisplayName( idx );
    QgsLayoutTableColumn col;
    col.setAttribute( field.name() );
    col.setHeading( currentAlias );
    mColumns.append( col );
    idx++;
  }
}

void QgsLayoutItemAttributeTable::disconnectCurrentMap()
{
  if ( !mMap )
  {
    return;
  }

  disconnect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutTable::refreshAttributes );
  disconnect( mMap, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutTable::refreshAttributes );
  disconnect( mMap, &QObject::destroyed, this, &QgsLayoutItemAttributeTable::disconnectCurrentMap );
  mMap = nullptr;
}

bool QgsLayoutItemAttributeTable::useConditionalStyling() const
{
  return mUseConditionalStyling;
}

void QgsLayoutItemAttributeTable::setUseConditionalStyling( bool useConditionalStyling )
{
  if ( useConditionalStyling == mUseConditionalStyling )
  {
    return;
  }

  mUseConditionalStyling = useConditionalStyling;
  refreshAttributes();
  emit changed();
}

void QgsLayoutItemAttributeTable::setMap( QgsLayoutItemMap *map )
{
  if ( map == mMap )
  {
    //no change
    return;
  }
  disconnectCurrentMap();

  mMap = map;
  if ( mMap )
  {
    //listen out for extent changes in linked map
    connect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutTable::refreshAttributes );
    connect( mMap, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutTable::refreshAttributes );
    connect( mMap, &QObject::destroyed, this, &QgsLayoutItemAttributeTable::disconnectCurrentMap );
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
  mColumns.clear();

  const QgsFields layerFields = source->fields();

  if ( !fields.isEmpty() )
  {
    for ( const QString &field : fields )
    {
      int attrIdx = layerFields.lookupField( field );
      if ( attrIdx < 0 )
      {
        continue;
      }
      QString currentAlias = source->attributeDisplayName( attrIdx );
      QgsLayoutTableColumn col;
      col.setAttribute( layerFields.at( attrIdx ).name() );
      col.setHeading( currentAlias );
      mColumns.append( col );
    }
  }
  else
  {
    //resetting, so add all attributes to columns
    int idx = 0;
    for ( const QgsField &field : layerFields )
    {
      QString currentAlias = source->attributeDisplayName( idx );
      QgsLayoutTableColumn col;
      col.setAttribute( field.name() );
      col.setHeading( currentAlias );
      mColumns.append( col );
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

  for ( int i = 0; i < mColumns.count(); i++ )
  {
    int attrIdx = source->fields().lookupField( mColumns[i].attribute() );
    if ( map.contains( attrIdx ) )
    {
      mColumns[i].setHeading( map.value( attrIdx ) );
    }
    else
    {
      mColumns[i].setHeading( source->attributeDisplayName( attrIdx ) );
    }
  }
}

bool QgsLayoutItemAttributeTable::getTableContents( QgsLayoutTableContents &contents )
{
  contents.clear();
  mLayerCache.clear();

  QgsVectorLayer *layer = sourceLayer();
  if ( !layer )
  {
    //no source layer
    return false;
  }

  const QgsConditionalLayerStyles *conditionalStyles = layer->conditionalStyles();

  QgsExpressionContext context = createExpressionContext();
  context.setFields( layer->fields() );

  QgsFeatureRequest req;
  req.setExpressionContext( context );

  //prepare filter expression
  std::unique_ptr<QgsExpression> filterExpression;
  bool activeFilter = false;
  if ( mFilterFeatures && !mFeatureFilter.isEmpty() )
  {
    filterExpression = std::make_unique< QgsExpression >( mFeatureFilter );
    if ( !filterExpression->hasParserError() )
    {
      activeFilter = true;
      req.setFilterExpression( mFeatureFilter );
    }
  }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  if ( mLayout->renderContext().featureFilterProvider() )
  {
    mLayout->renderContext().featureFilterProvider()->filterFeatures( layer, req );
  }
#endif

  QgsRectangle selectionRect;
  QgsGeometry visibleRegion;
  std::unique_ptr< QgsGeometryEngine > visibleMapEngine;
  if ( mMap && mShowOnlyVisibleFeatures )
  {
    visibleRegion = QgsGeometry::fromQPolygonF( mMap->visibleExtentPolygon() );
    selectionRect = visibleRegion.boundingBox();
    //transform back to layer CRS
    const QgsCoordinateTransform coordTransform( layer->crs(), mMap->crs(), mLayout->project() );
    QgsCoordinateTransform extentTransform = coordTransform;
    extentTransform.setBallparkTransformsAreAppropriate( true );
    try
    {
      selectionRect = extentTransform.transformBoundingBox( selectionRect, Qgis::TransformDirection::Reverse );
      visibleRegion.transform( coordTransform, Qgis::TransformDirection::Reverse );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse )
      return false;
    }
    visibleMapEngine.reset( QgsGeometry::createGeometryEngine( visibleRegion.constGet() ) );
    visibleMapEngine->prepareGeometry();
  }

  QgsGeometry atlasGeometry;
  std::unique_ptr< QgsGeometryEngine > atlasGeometryEngine;
  if ( mFilterToAtlasIntersection )
  {
    atlasGeometry = mLayout->reportContext().currentGeometry( layer->crs() );
    if ( !atlasGeometry.isNull() )
    {
      if ( selectionRect.isNull() )
      {
        selectionRect = atlasGeometry.boundingBox();
      }
      else
      {
        selectionRect = selectionRect.intersect( atlasGeometry.boundingBox() );
      }

      atlasGeometryEngine.reset( QgsGeometry::createGeometryEngine( atlasGeometry.constGet() ) );
      atlasGeometryEngine->prepareGeometry();
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

  for ( const QgsLayoutTableColumn &column : std::as_const( mSortColumns ) )
  {
    req.addOrderBy( column.attribute(), column.sortOrder() == Qt::AscendingOrder );
  }

  QgsFeature f;
  int counter = 0;
  QgsFeatureIterator fit = layer->getFeatures( req );

  mConditionalStyles.clear();
  mFeatures.clear();

  QVector< QVector< Cell > > tempContents;
  QgsLayoutTableContents existingContents;

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

    // check against exact map bounds
    if ( visibleMapEngine )
    {
      if ( !f.hasGeometry() )
        continue;

      if ( !visibleMapEngine->intersects( f.geometry().constGet() ) )
        continue;
    }

    //check against atlas feature intersection
    if ( mFilterToAtlasIntersection )
    {
      if ( !f.hasGeometry() || !atlasGeometryEngine )
      {
        continue;
      }

      if ( !atlasGeometryEngine->intersects( f.geometry().constGet() ) )
        continue;
    }

    QgsConditionalStyle rowStyle;

    if ( mUseConditionalStyling )
    {
      const QList<QgsConditionalStyle> styles = QgsConditionalStyle::matchingConditionalStyles( conditionalStyles->rowStyles(), QVariant(),  context );
      rowStyle = QgsConditionalStyle::compressStyles( styles );
    }

    // We need to build up two different lists here -- one is a pair of the cell contents along with the cell style.
    // We need this one because we do a sorting step later, and we need to ensure that the cell styling is attached to the right row and sorted
    // correctly when this occurs
    // We also need a list of just the cell contents, so that we can do a quick check for row uniqueness (when the
    // corresponding option is enabled)
    QVector< Cell > currentRow;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    mColumns = filteredColumns();
#endif
    currentRow.reserve( mColumns.count() );
    QgsLayoutTableRow rowContents;
    rowContents.reserve( mColumns.count() );

    for ( const QgsLayoutTableColumn &column : std::as_const( mColumns ) )
    {
      int idx = layer->fields().lookupField( column.attribute() );

      QgsConditionalStyle style;

      if ( idx != -1 )
      {

        QVariant val = f.attributes().at( idx );

        if ( mUseConditionalStyling )
        {
          QList<QgsConditionalStyle> styles = conditionalStyles->fieldStyles( layer->fields().at( idx ).name() );
          styles = QgsConditionalStyle::matchingConditionalStyles( styles, val, context );
          styles.insert( 0, rowStyle );
          style = QgsConditionalStyle::compressStyles( styles );
        }

        const QgsEditorWidgetSetup setup = layer->fields().at( idx ).editorWidgetSetup();

        if ( ! setup.isNull() )
        {
          QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
          QVariant cache;

          auto it = mLayerCache.constFind( column.attribute() );
          if ( it != mLayerCache.constEnd() )
          {
            cache = it.value();
          }
          else
          {
            cache = fieldFormatter->createCache( mVectorLayer.get(), idx, setup.config() );
            mLayerCache.insert( column.attribute(), cache );
          }

          val = fieldFormatter->representValue( mVectorLayer.get(), idx, setup.config(), cache, val );
        }

        QVariant v = val.isNull() ? QString() : replaceWrapChar( val );
        currentRow << Cell( v, style, f );
        rowContents << v;
      }
      else
      {
        // Lets assume it's an expression
        std::unique_ptr< QgsExpression > expression = std::make_unique< QgsExpression >( column.attribute() );
        context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "row_number" ), counter + 1, true ) );
        expression->prepare( &context );
        QVariant value = expression->evaluate( &context );

        currentRow << Cell( value, rowStyle, f );
        rowContents << value;
      }
    }

    if ( mShowUniqueRowsOnly )
    {
      if ( contentsContainsRow( existingContents, rowContents ) )
        continue;
    }

    tempContents << currentRow;
    existingContents << rowContents;
    ++counter;
  }

  // build final table contents
  contents.reserve( tempContents.size() );
  mConditionalStyles.reserve( tempContents.size() );
  mFeatures.reserve( tempContents.size() );
  for ( auto it = tempContents.constBegin(); it != tempContents.constEnd(); ++it )
  {
    QgsLayoutTableRow row;
    QList< QgsConditionalStyle > rowStyles;
    row.reserve( it->size() );
    rowStyles.reserve( it->size() );

    for ( auto cellIt = it->constBegin(); cellIt != it->constEnd(); ++cellIt )
    {
      row << cellIt->content;
      rowStyles << cellIt->style;
      if ( cellIt == it->constBegin() )
        mFeatures << cellIt->feature;
    }
    contents << row;
    mConditionalStyles << rowStyles;
  }

  recalculateTableSize();
  return true;
}

QgsConditionalStyle QgsLayoutItemAttributeTable::conditionalCellStyle( int row, int column ) const
{
  if ( row >= mConditionalStyles.size() )
    return QgsConditionalStyle();

  return mConditionalStyles.at( row ).at( column );
}

QgsExpressionContextScope *QgsLayoutItemAttributeTable::scopeForCell( int row, int column ) const
{
  std::unique_ptr< QgsExpressionContextScope >scope( QgsLayoutTable::scopeForCell( row, column ) );
  scope->setFeature( mFeatures.value( row ) );
  scope->setFields( scope->feature().fields() );
  return scope.release();
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
      connect( mMap, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutTable::refreshAttributes );
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

#ifdef HAVE_SERVER_PYTHON_PLUGINS
QgsLayoutTableColumns QgsLayoutItemAttributeTable::filteredColumns()
{

  QgsLayoutTableColumns allowedColumns { mColumns };

  // Filter columns
  if ( mLayout->renderContext().featureFilterProvider() )
  {

    QgsVectorLayer *source { sourceLayer() };

    if ( ! source )
    {
      return allowedColumns;
    }

    QHash<const QString, QSet<QString>> columnAttributesMap;
    QSet<QString> allowedAttributes;

    for ( const auto &c : std::as_const( allowedColumns ) )
    {
      if ( ! c.attribute().isEmpty() && ! columnAttributesMap.contains( c.attribute() ) )
      {
        columnAttributesMap[ c.attribute() ] = QSet<QString>();
        const QgsExpression columnExp { c.attribute() };
        const auto constRefs { columnExp.findNodes<QgsExpressionNodeColumnRef>() };
        for ( const auto &cref : constRefs )
        {
          columnAttributesMap[ c.attribute() ].insert( cref->name() );
          allowedAttributes.insert( cref->name() );
        }
      }
    }

    const QStringList filteredAttributes { layout()->renderContext().featureFilterProvider()->layerAttributes( source, allowedAttributes.values() ) };
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    const QSet<QString> filteredAttributesSet( filteredAttributes.constBegin(), filteredAttributes.constEnd() );
#else
    const QSet<QString> filteredAttributesSet { filteredAttributes.toSet() };
#endif
    if ( filteredAttributesSet != allowedAttributes )
    {
      const auto forbidden { allowedAttributes.subtract( filteredAttributesSet ) };
      allowedColumns.erase( std::remove_if( allowedColumns.begin(), allowedColumns.end(), [ &columnAttributesMap, &forbidden ]( QgsLayoutTableColumn & c ) -> bool
      {
        for ( const auto &f : std::as_const( forbidden ) )
        {
          if ( columnAttributesMap[ c.attribute() ].contains( f ) )
          {
            return true;
          }
        }
        return false;
      } ), allowedColumns.end() );

    }
  }

  return allowedColumns;
}
#endif

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
      mColumns.clear();
    }
  }
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
  tableElem.setAttribute( QStringLiteral( "useConditionalStyling" ), mUseConditionalStyling );

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
  if ( QgsVectorLayer *prevLayer = sourceLayer() )
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
  mUseConditionalStyling = itemElem.attribute( QStringLiteral( "useConditionalStyling" ), QStringLiteral( "0" ) ).toInt();

  //map
  mMapUuid = itemElem.attribute( QStringLiteral( "mapUuid" ) );
  if ( mMap )
  {
    disconnect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutTable::refreshAttributes );
    disconnect( mMap, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutTable::refreshAttributes );
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
  if ( QgsVectorLayer *newLayer = sourceLayer() )
    connect( newLayer, &QgsVectorLayer::layerModified, this, &QgsLayoutTable::refreshAttributes );

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
