/***************************************************************************
                         QgsComposerAttributeTableV2.cpp
                         -----------------------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerattributetablev2.h"
#include "qgscomposertablecolumn.h"
#include "qgscomposermap.h"
#include "qgscomposerutils.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"
#include "qgscomposerframe.h"
#include "qgsatlascomposition.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsgeometry.h"
#include "qgsexception.h"
#include "qgsmapsettings.h"

//QgsComposerAttributeTableCompareV2

bool QgsComposerAttributeTableCompareV2::operator()( const QgsComposerTableRow &m1, const QgsComposerTableRow &m2 )
{
  return ( mAscending ? qgsVariantLessThan( m1[mCurrentSortColumn], m2[mCurrentSortColumn] )
           : qgsVariantGreaterThan( m1[mCurrentSortColumn], m2[mCurrentSortColumn] ) );
}

//
// QgsComposerAttributeTableV2
//

QgsComposerAttributeTableV2::QgsComposerAttributeTableV2( QgsComposition *composition, bool createUndoCommands )
  : QgsComposerTableV2( composition, createUndoCommands )
  , mSource( LayerAttributes )
  , mMaximumNumberOfFeatures( 30 )
  , mShowUniqueRowsOnly( false )
  , mShowOnlyVisibleFeatures( false )
  , mFilterToAtlasIntersection( false )
  , mFilterFeatures( false )
  , mFeatureFilter( QLatin1String( "" ) )
{
  //set first vector layer from layer registry as default one
  QMap<QString, QgsMapLayer *> layerMap = mComposition->project()->mapLayers();
  QMap<QString, QgsMapLayer *>::const_iterator mapIt = layerMap.constBegin();
  for ( ; mapIt != layerMap.constEnd(); ++mapIt )
  {
    QgsVectorLayer *vl = dynamic_cast<QgsVectorLayer *>( mapIt.value() );
    if ( vl )
    {
      mVectorLayer.setLayer( vl );
      break;
    }
  }
  if ( mVectorLayer )
  {
    resetColumns();
    //listen for modifications to layer and refresh table when they occur
    connect( mVectorLayer.get(), &QgsVectorLayer::layerModified, this, &QgsComposerTableV2::refreshAttributes );
  }

  if ( mComposition )
  {
    connect( mComposition->project(), static_cast < void ( QgsProject::* )( const QString & ) >( &QgsProject::layerWillBeRemoved ), this, &QgsComposerAttributeTableV2::removeLayer );

    //refresh table attributes when composition is refreshed
    connect( mComposition, &QgsComposition::refreshItemsTriggered, this, &QgsComposerTableV2::refreshAttributes );

    //connect to atlas feature changes to update table rows
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::featureChanged, this, &QgsComposerTableV2::refreshAttributes );

    //atlas coverage layer change = regenerate columns
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::coverageLayerChanged, this, &QgsComposerAttributeTableV2::atlasLayerChanged );
  }
  refreshAttributes();
}

QString QgsComposerAttributeTableV2::displayName() const
{
  return tr( "<attribute table>" );
}

void QgsComposerAttributeTableV2::setVectorLayer( QgsVectorLayer *layer )
{
  if ( layer == mVectorLayer.get() )
  {
    //no change
    return;
  }

  QgsVectorLayer *prevLayer = sourceLayer();
  mVectorLayer.setLayer( layer );

  if ( mSource == QgsComposerAttributeTableV2::LayerAttributes && layer != prevLayer )
  {
    if ( prevLayer )
    {
      //disconnect from previous layer
      disconnect( prevLayer, &QgsVectorLayer::layerModified, this, &QgsComposerTableV2::refreshAttributes );
    }

    //rebuild column list to match all columns from layer
    resetColumns();

    //listen for modifications to layer and refresh table when they occur
    connect( mVectorLayer.get(), &QgsVectorLayer::layerModified, this, &QgsComposerTableV2::refreshAttributes );
  }

  refreshAttributes();
  emit changed();
}

void QgsComposerAttributeTableV2::setRelationId( const QString &relationId )
{
  if ( relationId == mRelationId )
  {
    //no change
    return;
  }

  QgsVectorLayer *prevLayer = sourceLayer();
  mRelationId = relationId;
  QgsRelation relation = mComposition->project()->relationManager()->relation( mRelationId );
  QgsVectorLayer *newLayer = relation.referencingLayer();

  if ( mSource == QgsComposerAttributeTableV2::RelationChildren && newLayer != prevLayer )
  {
    if ( prevLayer )
    {
      //disconnect from previous layer
      disconnect( prevLayer, &QgsVectorLayer::layerModified, this, &QgsComposerTableV2::refreshAttributes );
    }

    //rebuild column list to match all columns from layer
    resetColumns();

    //listen for modifications to layer and refresh table when they occur
    connect( newLayer, &QgsVectorLayer::layerModified, this, &QgsComposerTableV2::refreshAttributes );
  }

  refreshAttributes();
  emit changed();
}

void QgsComposerAttributeTableV2::atlasLayerChanged( QgsVectorLayer *layer )
{
  if ( mSource != QgsComposerAttributeTableV2::AtlasFeature || layer == mCurrentAtlasLayer )
  {
    //nothing to do
    return;
  }

  //atlas feature mode, atlas layer changed, so we need to reset columns
  if ( mCurrentAtlasLayer )
  {
    //disconnect from previous layer
    disconnect( mCurrentAtlasLayer, &QgsVectorLayer::layerModified, this, &QgsComposerTableV2::refreshAttributes );
  }

  mCurrentAtlasLayer = layer;

  //rebuild column list to match all columns from layer
  resetColumns();
  refreshAttributes();

  //listen for modifications to layer and refresh table when they occur
  connect( layer, &QgsVectorLayer::layerModified, this, &QgsComposerTableV2::refreshAttributes );
}

void QgsComposerAttributeTableV2::resetColumns()
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
    QgsComposerTableColumn *col = new QgsComposerTableColumn;
    col->setAttribute( field.name() );
    col->setHeading( currentAlias );
    mColumns.append( col );
    idx++;
  }
}

void QgsComposerAttributeTableV2::setComposerMap( const QgsComposerMap *map )
{
  if ( map == mComposerMap )
  {
    //no change
    return;
  }

  if ( mComposerMap )
  {
    //disconnect from previous map
    disconnect( mComposerMap, &QgsComposerMap::extentChanged, this, &QgsComposerTableV2::refreshAttributes );
  }
  mComposerMap = map;
  if ( mComposerMap )
  {
    //listen out for extent changes in linked map
    connect( mComposerMap, &QgsComposerMap::extentChanged, this, &QgsComposerTableV2::refreshAttributes );
  }
  refreshAttributes();
  emit changed();
}

void QgsComposerAttributeTableV2::setMaximumNumberOfFeatures( const int features )
{
  if ( features == mMaximumNumberOfFeatures )
  {
    return;
  }

  mMaximumNumberOfFeatures = features;
  refreshAttributes();
  emit changed();
}

void QgsComposerAttributeTableV2::setUniqueRowsOnly( const bool uniqueOnly )
{
  if ( uniqueOnly == mShowUniqueRowsOnly )
  {
    return;
  }

  mShowUniqueRowsOnly = uniqueOnly;
  refreshAttributes();
  emit changed();
}

void QgsComposerAttributeTableV2::setDisplayOnlyVisibleFeatures( const bool visibleOnly )
{
  if ( visibleOnly == mShowOnlyVisibleFeatures )
  {
    return;
  }

  mShowOnlyVisibleFeatures = visibleOnly;
  refreshAttributes();
  emit changed();
}

void QgsComposerAttributeTableV2::setFilterToAtlasFeature( const bool filterToAtlas )
{
  if ( filterToAtlas == mFilterToAtlasIntersection )
  {
    return;
  }

  mFilterToAtlasIntersection = filterToAtlas;
  refreshAttributes();
  emit changed();
}

void QgsComposerAttributeTableV2::setFilterFeatures( const bool filter )
{
  if ( filter == mFilterFeatures )
  {
    return;
  }

  mFilterFeatures = filter;
  refreshAttributes();
  emit changed();
}

void QgsComposerAttributeTableV2::setFeatureFilter( const QString &expression )
{
  if ( expression == mFeatureFilter )
  {
    return;
  }

  mFeatureFilter = expression;
  refreshAttributes();
  emit changed();
}

void QgsComposerAttributeTableV2::setDisplayedFields( const QStringList &fields, bool refresh )
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
    Q_FOREACH ( const QString &field, fields )
    {
      int attrIdx = layerFields.lookupField( field );
      if ( attrIdx < 0 )
        continue;

      QString currentAlias = source->attributeDisplayName( attrIdx );
      QgsComposerTableColumn *col = new QgsComposerTableColumn;
      col->setAttribute( layerFields.at( attrIdx ).name() );
      col->setHeading( currentAlias );
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
      QgsComposerTableColumn *col = new QgsComposerTableColumn;
      col->setAttribute( field.name() );
      col->setHeading( currentAlias );
      mColumns.append( col );
      idx++;
    }
  }

  if ( refresh )
  {
    refreshAttributes();
  }
}

void QgsComposerAttributeTableV2::restoreFieldAliasMap( const QMap<int, QString> &map )
{
  QgsVectorLayer *source = sourceLayer();
  if ( !source )
  {
    return;
  }

  QList<QgsComposerTableColumn *>::const_iterator columnIt = mColumns.constBegin();
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    int attrIdx = source->fields().lookupField( ( *columnIt )->attribute() );
    if ( map.contains( attrIdx ) )
    {
      ( *columnIt )->setHeading( map.value( attrIdx ) );
    }
    else
    {
      ( *columnIt )->setHeading( source->attributeDisplayName( attrIdx ) );
    }
  }
}

bool QgsComposerAttributeTableV2::getTableContents( QgsComposerTableContents &contents )
{
  contents.clear();

  if ( ( mSource == QgsComposerAttributeTableV2::AtlasFeature || mSource == QgsComposerAttributeTableV2::RelationChildren )
       && !mComposition->atlasComposition().enabled() )
  {
    //source mode requires atlas, but atlas disabled
    return false;
  }

  QgsVectorLayer *layer = sourceLayer();

  if ( !layer )
  {
    //no source layer
    return false;
  }

  QgsExpressionContext context = createExpressionContext();
  context.setFields( layer->fields() );

  //prepare filter expression
  std::unique_ptr<QgsExpression> filterExpression;
  bool activeFilter = false;
  if ( mFilterFeatures && !mFeatureFilter.isEmpty() )
  {
    filterExpression.reset( new QgsExpression( mFeatureFilter ) );
    if ( !filterExpression->hasParserError() )
    {
      activeFilter = true;
    }
  }

  QgsRectangle selectionRect;
  if ( mComposerMap && mShowOnlyVisibleFeatures )
  {
    selectionRect = *mComposerMap->currentMapExtent();
    if ( layer )
    {
      //transform back to layer CRS
      QgsCoordinateTransform coordTransform( layer->crs(), mComposerMap->crs() );
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

  QgsFeatureRequest req;

  if ( mSource == QgsComposerAttributeTableV2::RelationChildren )
  {
    QgsRelation relation = mComposition->project()->relationManager()->relation( mRelationId );
    QgsFeature atlasFeature = mComposition->atlasComposition().feature();
    req = relation.getRelatedFeaturesRequest( atlasFeature );
  }

  if ( !selectionRect.isEmpty() )
    req.setFilterRect( selectionRect );

  req.setFlags( mShowOnlyVisibleFeatures ? QgsFeatureRequest::ExactIntersect : QgsFeatureRequest::NoFlags );

  if ( mSource == QgsComposerAttributeTableV2::AtlasFeature
       && mComposition->atlasComposition().enabled() )
  {
    //source mode is current atlas feature
    QgsFeature atlasFeature = mComposition->atlasComposition().feature();
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
      if ( !f.hasGeometry() || ! mComposition->atlasComposition().enabled() )
      {
        continue;
      }
      QgsFeature atlasFeature = mComposition->atlasComposition().feature();
      if ( !atlasFeature.hasGeometry() ||
           !f.geometry().intersects( atlasFeature.geometry() ) )
      {
        //feature falls outside current atlas feature
        continue;
      }
    }

    QgsComposerTableRow currentRow;

    QList<QgsComposerTableColumn *>::const_iterator columnIt = mColumns.constBegin();
    for ( ; columnIt != mColumns.constEnd(); ++columnIt )
    {
      int idx = layer->fields().lookupField( ( *columnIt )->attribute() );
      if ( idx != -1 )
      {
        currentRow << replaceWrapChar( f.attributes().at( idx ) );
      }
      else
      {
        // Lets assume it's an expression
        QgsExpression *expression = new QgsExpression( ( *columnIt )->attribute() );
        context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "row_number" ), counter + 1, true ) );
        expression->prepare( &context );
        QVariant value = expression->evaluate( &context );
        currentRow << value;
        delete expression;
      }
    }

    if ( !mShowUniqueRowsOnly || !contentsContainsRow( contents, currentRow ) )
    {
      contents << currentRow;
      ++counter;
    }
  }

  //sort the list, starting with the last attribute
  QgsComposerAttributeTableCompareV2 c;
  QList< QPair<int, bool> > sortColumns = sortAttributes();
  for ( int i = sortColumns.size() - 1; i >= 0; --i )
  {
    c.setSortColumn( sortColumns.at( i ).first );
    c.setAscending( sortColumns.at( i ).second );
    std::stable_sort( contents.begin(), contents.end(), c );
  }

  recalculateTableSize();
  return true;
}

QgsExpressionContext QgsComposerAttributeTableV2::createExpressionContext() const
{
  QgsExpressionContext context = QgsComposerTableV2::createExpressionContext();

  if ( mSource == LayerAttributes )
  {
    context.appendScope( QgsExpressionContextUtils::layerScope( mVectorLayer.get() ) );
  }

  return context;
}

QVariant QgsComposerAttributeTableV2::replaceWrapChar( const QVariant &variant ) const
{
  //avoid converting variants to string if not required (try to maintain original type for sorting)
  if ( mWrapString.isEmpty() || !variant.toString().contains( mWrapString ) )
    return variant;

  QString replaced = variant.toString();
  replaced.replace( mWrapString, QLatin1String( "\n" ) );
  return replaced;
}

QgsVectorLayer *QgsComposerAttributeTableV2::sourceLayer()
{
  switch ( mSource )
  {
    case QgsComposerAttributeTableV2::AtlasFeature:
      return mComposition->atlasComposition().coverageLayer();
    case QgsComposerAttributeTableV2::LayerAttributes:
      return mVectorLayer.get();
    case QgsComposerAttributeTableV2::RelationChildren:
    {
      QgsRelation relation = mComposition->project()->relationManager()->relation( mRelationId );
      return relation.referencingLayer();
    }
  }
  return nullptr;
}

void QgsComposerAttributeTableV2::removeLayer( const QString &layerId )
{
  if ( mVectorLayer && mSource == QgsComposerAttributeTableV2::LayerAttributes )
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

static bool columnsBySortRank( QPair<int, QgsComposerTableColumn * > a, QPair<int, QgsComposerTableColumn * > b )
{
  return a.second->sortByRank() < b.second->sortByRank();
}

QList<QPair<int, bool> > QgsComposerAttributeTableV2::sortAttributes() const
{
  //generate list of all sorted columns
  QVector< QPair<int, QgsComposerTableColumn * > > sortedColumns;
  QList<QgsComposerTableColumn *>::const_iterator columnIt = mColumns.constBegin();
  int idx = 0;
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    if ( ( *columnIt )->sortByRank() > 0 )
    {
      sortedColumns.append( qMakePair( idx, *columnIt ) );
    }
    idx++;
  }

  //sort columns by rank
  std::sort( sortedColumns.begin(), sortedColumns.end(), columnsBySortRank );

  //generate list of column index, bool for sort direction (to match 2.0 api)
  QList<QPair<int, bool> > attributesBySortRank;
  QVector< QPair<int, QgsComposerTableColumn * > >::const_iterator sortedColumnIt = sortedColumns.constBegin();
  for ( ; sortedColumnIt != sortedColumns.constEnd(); ++sortedColumnIt )
  {

    attributesBySortRank.append( qMakePair( ( *sortedColumnIt ).first,
                                            ( *sortedColumnIt ).second->sortOrder() == Qt::AscendingOrder ) );
  }
  return attributesBySortRank;
}

void QgsComposerAttributeTableV2::setWrapString( const QString &wrapString )
{
  if ( wrapString == mWrapString )
  {
    return;
  }

  mWrapString = wrapString;
  refreshAttributes();
  emit changed();
}

bool QgsComposerAttributeTableV2::writeXml( QDomElement &elem, QDomDocument &doc, bool ignoreFrames ) const
{
  QDomElement composerTableElem = doc.createElement( QStringLiteral( "ComposerAttributeTableV2" ) );
  composerTableElem.setAttribute( QStringLiteral( "source" ), QString::number( static_cast< int >( mSource ) ) );
  composerTableElem.setAttribute( QStringLiteral( "relationId" ), mRelationId );
  composerTableElem.setAttribute( QStringLiteral( "showUniqueRowsOnly" ), mShowUniqueRowsOnly );
  composerTableElem.setAttribute( QStringLiteral( "showOnlyVisibleFeatures" ), mShowOnlyVisibleFeatures );
  composerTableElem.setAttribute( QStringLiteral( "filterToAtlasIntersection" ), mFilterToAtlasIntersection );
  composerTableElem.setAttribute( QStringLiteral( "maxFeatures" ), mMaximumNumberOfFeatures );
  composerTableElem.setAttribute( QStringLiteral( "filterFeatures" ), mFilterFeatures ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  composerTableElem.setAttribute( QStringLiteral( "featureFilter" ), mFeatureFilter );
  composerTableElem.setAttribute( QStringLiteral( "wrapString" ), mWrapString );

  if ( mComposerMap )
  {
    composerTableElem.setAttribute( QStringLiteral( "composerMap" ), mComposerMap->id() );
  }
  else
  {
    composerTableElem.setAttribute( QStringLiteral( "composerMap" ), -1 );
  }
  if ( mVectorLayer )
  {
    composerTableElem.setAttribute( QStringLiteral( "vectorLayer" ), mVectorLayer.layerId );
    composerTableElem.setAttribute( QStringLiteral( "vectorLayerName" ), mVectorLayer.name );
    composerTableElem.setAttribute( QStringLiteral( "vectorLayerSource" ), mVectorLayer.source );
    composerTableElem.setAttribute( QStringLiteral( "vectorLayerProvider" ), mVectorLayer.provider );
  }

  bool ok = QgsComposerTableV2::writeXml( composerTableElem, doc, ignoreFrames );

  elem.appendChild( composerTableElem );

  return ok;
}

bool QgsComposerAttributeTableV2::readXml( const QDomElement &itemElem, const QDomDocument &doc, bool ignoreFrames )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  //read general table properties
  if ( !QgsComposerTableV2::readXml( itemElem, doc, ignoreFrames ) )
  {
    return false;
  }

  QgsVectorLayer *prevLayer = sourceLayer();
  if ( prevLayer )
  {
    //disconnect from previous layer
    disconnect( prevLayer, &QgsVectorLayer::layerModified, this, &QgsComposerTableV2::refreshAttributes );
  }

  mSource = QgsComposerAttributeTableV2::ContentSource( itemElem.attribute( QStringLiteral( "source" ), QStringLiteral( "0" ) ).toInt() );
  mRelationId = itemElem.attribute( QStringLiteral( "relationId" ), QLatin1String( "" ) );

  if ( mSource == QgsComposerAttributeTableV2::AtlasFeature )
  {
    mCurrentAtlasLayer = mComposition->atlasComposition().coverageLayer();
  }

  mShowUniqueRowsOnly = itemElem.attribute( QStringLiteral( "showUniqueRowsOnly" ), QStringLiteral( "0" ) ).toInt();
  mShowOnlyVisibleFeatures = itemElem.attribute( QStringLiteral( "showOnlyVisibleFeatures" ), QStringLiteral( "1" ) ).toInt();
  mFilterToAtlasIntersection = itemElem.attribute( QStringLiteral( "filterToAtlasIntersection" ), QStringLiteral( "0" ) ).toInt();
  mFilterFeatures = itemElem.attribute( QStringLiteral( "filterFeatures" ), QStringLiteral( "false" ) ) == QLatin1String( "true" );
  mFeatureFilter = itemElem.attribute( QStringLiteral( "featureFilter" ), QLatin1String( "" ) );
  mMaximumNumberOfFeatures = itemElem.attribute( QStringLiteral( "maxFeatures" ), QStringLiteral( "5" ) ).toInt();
  mWrapString = itemElem.attribute( QStringLiteral( "wrapString" ) );

  //composer map
  int composerMapId = itemElem.attribute( QStringLiteral( "composerMap" ), QStringLiteral( "-1" ) ).toInt();
  if ( composerMapId == -1 )
  {
    mComposerMap = nullptr;
  }

  if ( composition() )
  {
    mComposerMap = composition()->getComposerMapById( composerMapId );
  }
  else
  {
    mComposerMap = nullptr;
  }

  if ( mComposerMap )
  {
    //if we have found a valid map item, listen out to extent changes on it and refresh the table
    connect( mComposerMap, &QgsComposerMap::extentChanged, this, &QgsComposerTableV2::refreshAttributes );
  }

  //vector layer
  QString layerId = itemElem.attribute( QStringLiteral( "vectorLayer" ) );
  QString layerName = itemElem.attribute( QStringLiteral( "vectorLayerName" ) );
  QString layerSource = itemElem.attribute( QStringLiteral( "vectorLayerSource" ) );
  QString layerProvider = itemElem.attribute( QStringLiteral( "vectorLayerProvider" ) );
  mVectorLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );
  mVectorLayer.resolveWeakly( mComposition->project() );

  //connect to new layer
  connect( sourceLayer(), &QgsVectorLayer::layerModified, this, &QgsComposerTableV2::refreshAttributes );

  refreshAttributes();

  emit changed();
  return true;
}

void QgsComposerAttributeTableV2::addFrame( QgsComposerFrame *frame, bool recalcFrameSizes )
{
  mFrameItems.push_back( frame );
  connect( frame, &QgsComposerItem::sizeChanged, this, &QgsComposerTableV2::recalculateFrameSizes );
  if ( mComposition )
  {
    mComposition->addComposerTableFrame( this, frame );
  }

  if ( recalcFrameSizes )
  {
    recalculateFrameSizes();
  }
}

void QgsComposerAttributeTableV2::setSource( const QgsComposerAttributeTableV2::ContentSource source )
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
      disconnect( prevLayer, &QgsVectorLayer::layerModified, this, &QgsComposerTableV2::refreshAttributes );
    }

    //connect to new layer
    connect( newLayer, &QgsVectorLayer::layerModified, this, &QgsComposerTableV2::refreshAttributes );
    if ( mSource == QgsComposerAttributeTableV2::AtlasFeature )
    {
      mCurrentAtlasLayer = newLayer;
    }

    //layer has changed as a result of the source change, so reset column list
    resetColumns();
  }

  refreshAttributes();
  emit changed();
}
