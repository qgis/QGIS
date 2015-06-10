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
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgscomposerframe.h"
#include "qgsatlascomposition.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsgeometry.h"

//QgsComposerAttributeTableCompareV2

QgsComposerAttributeTableCompareV2::QgsComposerAttributeTableCompareV2()
    : mCurrentSortColumn( 0 ), mAscending( true )
{
}


bool QgsComposerAttributeTableCompareV2::operator()( const QgsComposerTableRow& m1, const QgsComposerTableRow& m2 )
{
  QVariant v1 = m1[mCurrentSortColumn];
  QVariant v2 = m2[mCurrentSortColumn];

  bool less = false;

  //sort null values first
  if ( v1.isNull() && v2.isNull() )
  {
    less = false;
  }
  else if ( v1.isNull() )
  {
    less = true;
  }
  else if ( v2.isNull() )
  {
    less = false;
  }
  else
  {
    //otherwise sort by converting to corresponding type and comparing
    switch ( v1.type() )
    {
      case QVariant::Int:
      case QVariant::UInt:
      case QVariant::LongLong:
      case QVariant::ULongLong:
        less = v1.toLongLong() < v2.toLongLong();
        break;

      case QVariant::Double:
        less = v1.toDouble() < v2.toDouble();
        break;

      case QVariant::Date:
        less = v1.toDate() < v2.toDate();
        break;

      case QVariant::DateTime:
        less = v1.toDateTime() < v2.toDateTime();
        break;

      case QVariant::Time:
        less = v1.toTime() < v2.toTime();
        break;

      default:
        //use locale aware compare for strings
        less = v1.toString().localeAwareCompare( v2.toString() ) < 0;
        break;
    }
  }

  return ( mAscending ? less : !less );
}

//
// QgsComposerAttributeTableV2
//

QgsComposerAttributeTableV2::QgsComposerAttributeTableV2( QgsComposition* composition, bool createUndoCommands )
    : QgsComposerTableV2( composition, createUndoCommands )
    , mSource( LayerAttributes )
    , mVectorLayer( 0 )
    , mCurrentAtlasLayer( 0 )
    , mComposerMap( 0 )
    , mMaximumNumberOfFeatures( 30 )
    , mShowUniqueRowsOnly( false )
    , mShowOnlyVisibleFeatures( false )
    , mFilterToAtlasIntersection( false )
    , mFilterFeatures( false )
    , mFeatureFilter( "" )
{
  //set first vector layer from layer registry as default one
  QMap<QString, QgsMapLayer*> layerMap =  QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::const_iterator mapIt = layerMap.constBegin();
  for ( ; mapIt != layerMap.constEnd(); ++mapIt )
  {
    QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( mapIt.value() );
    if ( vl )
    {
      mVectorLayer = vl;
      break;
    }
  }
  if ( mVectorLayer )
  {
    resetColumns();
    //listen for modifications to layer and refresh table when they occur
    connect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
  }
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( removeLayer( const QString& ) ) );

  if ( mComposition )
  {
    //refresh table attributes when composition is refreshed
    connect( mComposition, SIGNAL( refreshItemsTriggered() ), this, SLOT( refreshAttributes() ) );

    //connect to atlas feature changes to update table rows
    connect( &mComposition->atlasComposition(), SIGNAL( featureChanged( QgsFeature* ) ), this, SLOT( refreshAttributes() ) );

    //atlas coverage layer change = regenerate columns
    connect( &mComposition->atlasComposition(), SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ), this, SLOT( atlasLayerChanged( QgsVectorLayer* ) ) );
  }
  refreshAttributes();
}

QgsComposerAttributeTableV2::~QgsComposerAttributeTableV2()
{
}

QString QgsComposerAttributeTableV2::displayName() const
{
  return tr( "<attribute table>" );
}

void QgsComposerAttributeTableV2::setVectorLayer( QgsVectorLayer* layer )
{
  if ( layer == mVectorLayer )
  {
    //no change
    return;
  }

  QgsVectorLayer* prevLayer = sourceLayer();
  mVectorLayer = layer;

  if ( mSource == QgsComposerAttributeTableV2::LayerAttributes && layer != prevLayer )
  {
    if ( prevLayer )
    {
      //disconnect from previous layer
      disconnect( prevLayer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
    }

    //rebuild column list to match all columns from layer
    resetColumns();

    //listen for modifications to layer and refresh table when they occur
    connect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
  }

  refreshAttributes();
  emit changed();
}

void QgsComposerAttributeTableV2::setRelationId( const QString relationId )
{
  if ( relationId == mRelationId )
  {
    //no change
    return;
  }

  QgsVectorLayer* prevLayer = sourceLayer();
  mRelationId = relationId;
  QgsRelation relation = QgsProject::instance()->relationManager()->relation( mRelationId );
  QgsVectorLayer* newLayer = relation.referencingLayer();

  if ( mSource == QgsComposerAttributeTableV2::RelationChildren && newLayer != prevLayer )
  {
    if ( prevLayer )
    {
      //disconnect from previous layer
      disconnect( prevLayer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
    }

    //rebuild column list to match all columns from layer
    resetColumns();

    //listen for modifications to layer and refresh table when they occur
    connect( newLayer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
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
    disconnect( mCurrentAtlasLayer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
  }

  mCurrentAtlasLayer = layer;

  //rebuild column list to match all columns from layer
  resetColumns();
  refreshAttributes();

  //listen for modifications to layer and refresh table when they occur
  connect( layer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
}

void QgsComposerAttributeTableV2::resetColumns()
{
  QgsVectorLayer* source = sourceLayer();
  if ( !source )
  {
    return;
  }

  //remove existing columns
  qDeleteAll( mColumns );
  mColumns.clear();

  //rebuild columns list from vector layer fields
  const QgsFields& fields = source->pendingFields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    QString currentAlias = source->attributeDisplayName( idx );
    QgsComposerTableColumn* col = new QgsComposerTableColumn;
    col->setAttribute( fields[idx].name() );
    col->setHeading( currentAlias );
    mColumns.append( col );
  }
}

void QgsComposerAttributeTableV2::setComposerMap( const QgsComposerMap* map )
{
  if ( map == mComposerMap )
  {
    //no change
    return;
  }

  if ( mComposerMap )
  {
    //disconnect from previous map
    disconnect( mComposerMap, SIGNAL( extentChanged() ), this, SLOT( refreshAttributes() ) );
  }
  mComposerMap = map;
  if ( mComposerMap )
  {
    //listen out for extent changes in linked map
    connect( mComposerMap, SIGNAL( extentChanged() ), this, SLOT( refreshAttributes() ) );
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

void QgsComposerAttributeTableV2::setFeatureFilter( const QString& expression )
{
  if ( expression == mFeatureFilter )
  {
    return;
  }

  mFeatureFilter = expression;
  refreshAttributes();
  emit changed();
}

void QgsComposerAttributeTableV2::setDisplayAttributes( const QSet<int>& attr, bool refresh )
{
  QgsVectorLayer* source = sourceLayer();
  if ( !source )
  {
    return;
  }

  //rebuild columns list, taking only attributes with index in supplied QSet
  qDeleteAll( mColumns );
  mColumns.clear();

  const QgsFields& fields = source->pendingFields();

  if ( !attr.empty() )
  {
    QSet<int>::const_iterator attIt = attr.constBegin();
    for ( ; attIt != attr.constEnd(); ++attIt )
    {
      int attrIdx = ( *attIt );
      if ( !fields.exists( attrIdx ) )
      {
        continue;
      }
      QString currentAlias = source->attributeDisplayName( attrIdx );
      QgsComposerTableColumn* col = new QgsComposerTableColumn;
      col->setAttribute( fields[attrIdx].name() );
      col->setHeading( currentAlias );
      mColumns.append( col );
    }
  }
  else
  {
    //resetting, so add all attributes to columns
    for ( int idx = 0; idx < fields.count(); ++idx )
    {
      QString currentAlias = source->attributeDisplayName( idx );
      QgsComposerTableColumn* col = new QgsComposerTableColumn;
      col->setAttribute( fields[idx].name() );
      col->setHeading( currentAlias );
      mColumns.append( col );
    }
  }

  if ( refresh )
  {
    refreshAttributes();
  }
}

void QgsComposerAttributeTableV2::restoreFieldAliasMap( const QMap<int, QString>& map )
{
  QgsVectorLayer* source = sourceLayer();
  if ( !source )
  {
    return;
  }

  QList<QgsComposerTableColumn*>::const_iterator columnIt = mColumns.constBegin();
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    int attrIdx = source->fieldNameIndex(( *columnIt )->attribute() );
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

  if (( mSource == QgsComposerAttributeTableV2::AtlasFeature || mSource == QgsComposerAttributeTableV2::RelationChildren )
      && !mComposition->atlasComposition().enabled() )
  {
    //source mode requires atlas, but atlas disabled
    return false;
  }

  QgsVectorLayer* layer = sourceLayer();

  if ( !layer )
  {
    //no source layer
    return false;
  }

  //prepare filter expression
  QScopedPointer<QgsExpression> filterExpression;
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
    if ( layer && mComposition->mapSettings().hasCrsTransformEnabled() )
    {
      //transform back to layer CRS
      QgsCoordinateTransform coordTransform( layer->crs(), mComposition->mapSettings().destinationCrs() );
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
    QgsRelation relation = QgsProject::instance()->relationManager()->relation( mRelationId );
    QgsFeature* atlasFeature = mComposition->atlasComposition().currentFeature();
    if ( atlasFeature )
    {
      req = relation.getRelatedFeaturesRequest( *atlasFeature );
    }
    else
    {
      //no atlas feature, so empty table
      return true;
    }
  }

  if ( !selectionRect.isEmpty() )
    req.setFilterRect( selectionRect );

  req.setFlags( mShowOnlyVisibleFeatures ? QgsFeatureRequest::ExactIntersect : QgsFeatureRequest::NoFlags );

  if ( mSource == QgsComposerAttributeTableV2::AtlasFeature
       && mComposition->atlasComposition().enabled() )
  {
    //source mode is current atlas feature
    QgsFeature* atlasFeature = mComposition->atlasComposition().currentFeature();
    if ( atlasFeature )
    {
      req.setFilterFid( atlasFeature->id() );
    }
    else
    {
      //no atlas feature, so empty table
      return true;
    }
  }

  QgsFeature f;
  int counter = 0;
  QgsFeatureIterator fit = layer->getFeatures( req );

  while ( fit.nextFeature( f ) && counter < mMaximumNumberOfFeatures )
  {
    //check feature against filter
    if ( activeFilter && !filterExpression.isNull() )
    {
      QVariant result = filterExpression->evaluate( &f, layer->pendingFields() );
      // skip this feature if the filter evaluation is false
      if ( !result.toBool() )
      {
        continue;
      }
    }
    //check against atlas feature intersection
    if ( mFilterToAtlasIntersection )
    {
      if ( !f.constGeometry() || ! mComposition->atlasComposition().enabled() )
      {
        continue;
      }
      QgsFeature* atlasFeature = mComposition->atlasComposition().currentFeature();
      if ( !atlasFeature || !atlasFeature->constGeometry() ||
           !f.constGeometry()->intersects( atlasFeature->constGeometry() ) )
      {
        //feature falls outside current atlas feature
        continue;
      }
    }

    QgsComposerTableRow currentRow;

    QList<QgsComposerTableColumn*>::const_iterator columnIt = mColumns.constBegin();
    for ( ; columnIt != mColumns.constEnd(); ++columnIt )
    {
      int idx = layer->fieldNameIndex(( *columnIt )->attribute() );
      if ( idx != -1 )
      {
        currentRow << f.attributes()[idx];
      }
      else
      {
        // Lets assume it's an expression
        QgsExpression* expression = new QgsExpression(( *columnIt )->attribute() );
        expression->setCurrentRowNumber( counter + 1 );
        expression->prepare( layer->pendingFields() );
        QVariant value = expression->evaluate( f );
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
  QgsComposerAttributeTableCompareV2 c;
  QList< QPair<int, bool> > sortColumns = sortAttributes();
  for ( int i = sortColumns.size() - 1; i >= 0; --i )
  {
    c.setSortColumn( sortColumns.at( i ).first );
    c.setAscending( sortColumns.at( i ).second );
    qStableSort( contents.begin(), contents.end(), c );
  }

  recalculateTableSize();
  return true;
}

QgsVectorLayer *QgsComposerAttributeTableV2::sourceLayer()
{
  switch ( mSource )
  {
    case QgsComposerAttributeTableV2::AtlasFeature:
      return mComposition->atlasComposition().coverageLayer();
    case QgsComposerAttributeTableV2::LayerAttributes:
      return mVectorLayer;
    case QgsComposerAttributeTableV2::RelationChildren:
    {
      QgsRelation relation = QgsProject::instance()->relationManager()->relation( mRelationId );
      return relation.referencingLayer();
    }
  }
  return 0;
}

void QgsComposerAttributeTableV2::removeLayer( QString layerId )
{
  if ( mVectorLayer && mSource == QgsComposerAttributeTableV2::LayerAttributes )
  {
    if ( layerId == mVectorLayer->id() )
    {
      mVectorLayer = 0;
      //remove existing columns
      qDeleteAll( mColumns );
      mColumns.clear();
    }
  }
}

static bool columnsBySortRank( QPair<int, QgsComposerTableColumn* > a, QPair<int, QgsComposerTableColumn* > b )
{
  return a.second->sortByRank() < b.second->sortByRank();
}

QList<QPair<int, bool> > QgsComposerAttributeTableV2::sortAttributes() const
{
  //generate list of all sorted columns
  QList< QPair<int, QgsComposerTableColumn* > > sortedColumns;
  QList<QgsComposerTableColumn*>::const_iterator columnIt = mColumns.constBegin();
  int idx = 0;
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    if (( *columnIt )->sortByRank() > 0 )
    {
      sortedColumns.append( qMakePair( idx, *columnIt ) );
    }
    idx++;
  }

  //sort columns by rank
  qSort( sortedColumns.begin(), sortedColumns.end(), columnsBySortRank );

  //generate list of column index, bool for sort direction (to match 2.0 api)
  QList<QPair<int, bool> > attributesBySortRank;
  QList< QPair<int, QgsComposerTableColumn* > >::const_iterator sortedColumnIt = sortedColumns.constBegin();
  for ( ; sortedColumnIt != sortedColumns.constEnd(); ++sortedColumnIt )
  {

    attributesBySortRank.append( qMakePair(( *sortedColumnIt ).first,
                                           ( *sortedColumnIt ).second->sortOrder() == Qt::AscendingOrder ) );
  }
  return attributesBySortRank;
}

bool QgsComposerAttributeTableV2::writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames ) const
{
  QDomElement composerTableElem = doc.createElement( "ComposerAttributeTableV2" );
  composerTableElem.setAttribute( "source", QString::number(( int )mSource ) );
  composerTableElem.setAttribute( "relationId", mRelationId );
  composerTableElem.setAttribute( "showUniqueRowsOnly", mShowUniqueRowsOnly );
  composerTableElem.setAttribute( "showOnlyVisibleFeatures", mShowOnlyVisibleFeatures );
  composerTableElem.setAttribute( "filterToAtlasIntersection", mFilterToAtlasIntersection );
  composerTableElem.setAttribute( "maxFeatures", mMaximumNumberOfFeatures );
  composerTableElem.setAttribute( "filterFeatures", mFilterFeatures ? "true" : "false" );
  composerTableElem.setAttribute( "featureFilter", mFeatureFilter );

  if ( mComposerMap )
  {
    composerTableElem.setAttribute( "composerMap", mComposerMap->id() );
  }
  else
  {
    composerTableElem.setAttribute( "composerMap", -1 );
  }
  if ( mVectorLayer )
  {
    composerTableElem.setAttribute( "vectorLayer", mVectorLayer->id() );
  }

  bool ok = QgsComposerTableV2::writeXML( composerTableElem, doc, ignoreFrames );

  elem.appendChild( composerTableElem );

  return ok;
}

bool QgsComposerAttributeTableV2::readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  //read general table properties
  if ( !QgsComposerTableV2::readXML( itemElem, doc, ignoreFrames ) )
  {
    return false;
  }

  QgsVectorLayer* prevLayer = sourceLayer();
  if ( prevLayer )
  {
    //disconnect from previous layer
    disconnect( prevLayer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
  }

  mSource = QgsComposerAttributeTableV2::ContentSource( itemElem.attribute( "source", "0" ).toInt() );
  mRelationId = itemElem.attribute( "relationId", "" );

  if ( mSource == QgsComposerAttributeTableV2::AtlasFeature )
  {
    mCurrentAtlasLayer = mComposition->atlasComposition().coverageLayer();
  }

  mShowUniqueRowsOnly = itemElem.attribute( "showUniqueRowsOnly", "0" ).toInt();
  mShowOnlyVisibleFeatures = itemElem.attribute( "showOnlyVisibleFeatures", "1" ).toInt();
  mFilterToAtlasIntersection = itemElem.attribute( "filterToAtlasIntersection", "0" ).toInt();
  mFilterFeatures = itemElem.attribute( "filterFeatures", "false" ) == "true" ? true : false;
  mFeatureFilter = itemElem.attribute( "featureFilter", "" );
  mMaximumNumberOfFeatures = itemElem.attribute( "maxFeatures", "5" ).toInt();

  //composer map
  int composerMapId = itemElem.attribute( "composerMap", "-1" ).toInt();
  if ( composerMapId == -1 )
  {
    mComposerMap = 0;
  }

  if ( composition() )
  {
    mComposerMap = composition()->getComposerMapById( composerMapId );
  }
  else
  {
    mComposerMap = 0;
  }

  if ( mComposerMap )
  {
    //if we have found a valid map item, listen out to extent changes on it and refresh the table
    connect( mComposerMap, SIGNAL( extentChanged() ), this, SLOT( refreshAttributes() ) );
  }

  //vector layer
  QString layerId = itemElem.attribute( "vectorLayer", "not_existing" );
  if ( layerId == "not_existing" )
  {
    mVectorLayer = 0;
  }
  else
  {
    QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );
    if ( ml )
    {
      mVectorLayer = dynamic_cast<QgsVectorLayer*>( ml );
    }
  }

  //connect to new layer
  connect( sourceLayer(), SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );

  refreshAttributes();

  emit changed();
  return true;
}

void QgsComposerAttributeTableV2::addFrame( QgsComposerFrame *frame, bool recalcFrameSizes )
{
  mFrameItems.push_back( frame );
  connect( frame, SIGNAL( sizeChanged() ), this, SLOT( recalculateFrameSizes() ) );
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

  QgsVectorLayer* prevLayer = sourceLayer();
  mSource = source;
  QgsVectorLayer* newLayer = sourceLayer();

  if ( newLayer != prevLayer )
  {
    //disconnect from previous layer
    if ( prevLayer )
    {
      disconnect( prevLayer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
    }

    //connect to new layer
    connect( newLayer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
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
