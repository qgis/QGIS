/***************************************************************************
                         qgscomposerattributetable.cpp
                         -----------------------------
    begin                : April 2010
    copyright            : (C) 2010 by Marco Hugentobler
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

#include "qgscomposerattributetable.h"
#include "qgscomposertablecolumn.h"
#include "qgscomposermap.h"
#include "qgscomposerutils.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"

//QgsComposerAttributeTableCompare

QgsComposerAttributeTableCompare::QgsComposerAttributeTableCompare()
    : mCurrentSortColumn( 0 )
    , mAscending( true )
{
}


bool QgsComposerAttributeTableCompare::operator()( const QgsAttributeMap& m1, const QgsAttributeMap& m2 )
{
  return ( mAscending ? qgsVariantLessThan( m1[mCurrentSortColumn], m2[mCurrentSortColumn] )
           : qgsVariantGreaterThan( m1[mCurrentSortColumn], m2[mCurrentSortColumn] ) );
}


//QgsComposerAttributeTable

QgsComposerAttributeTable::QgsComposerAttributeTable( QgsComposition* composition )
    : QgsComposerTable( composition )
    , mVectorLayer( nullptr )
    , mComposerMap( nullptr )
    , mMaximumNumberOfFeatures( 5 )
    , mShowOnlyVisibleFeatures( false )
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
  }
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( removeLayer( const QString& ) ) );

  if ( mComposition )
  {
    //refresh table attributes when composition is refreshed
    connect( mComposition, SIGNAL( refreshItemsTriggered() ), this, SLOT( refreshAttributes() ) );

    //connect to atlas feature changes to update table rows
    connect( &mComposition->atlasComposition(), SIGNAL( featureChanged( QgsFeature* ) ), this, SLOT( refreshAttributes() ) );
  }
}

QgsComposerAttributeTable::~QgsComposerAttributeTable()
{
}

void QgsComposerAttributeTable::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  if ( mComposerMap && mComposerMap->isDrawing() )
  {
    return;
  }
  if ( !shouldDrawItem() )
  {
    return;
  }
  QgsComposerTable::paint( painter, itemStyle, pWidget );
}

void QgsComposerAttributeTable::setVectorLayer( QgsVectorLayer* layer )
{
  if ( layer == mVectorLayer )
  {
    //no change
    return;
  }

  if ( mVectorLayer )
  {
    //disconnect from previous layer
    QObject::disconnect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
  }

  mVectorLayer = layer;

  //rebuild column list to match all columns from layer
  resetColumns();
  refreshAttributes();

  //listen for modifications to layer and refresh table when they occur
  QObject::connect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
}

void QgsComposerAttributeTable::resetColumns()
{
  if ( !mVectorLayer )
  {
    return;
  }

  //remove existing columns
  qDeleteAll( mColumns );
  mColumns.clear();

  //rebuild columns list from vector layer fields
  int idx = 0;
  Q_FOREACH ( const QgsField& field, mVectorLayer->fields() )
  {
    QString currentAlias = mVectorLayer->attributeDisplayName( idx );
    QgsComposerTableColumn* col = new QgsComposerTableColumn;
    col->setAttribute( field.name() );
    col->setHeading( currentAlias );
    mColumns.append( col );
    idx++;
  }
}

void QgsComposerAttributeTable::setComposerMap( const QgsComposerMap* map )
{
  if ( map == mComposerMap )
  {
    //no change
    return;
  }

  if ( mComposerMap )
  {
    //disconnect from previous map
    QObject::disconnect( mComposerMap, SIGNAL( extentChanged() ), this, SLOT( refreshAttributes() ) );
  }
  mComposerMap = map;
  if ( mComposerMap )
  {
    //listen out for extent changes in linked map
    QObject::connect( mComposerMap, SIGNAL( extentChanged() ), this, SLOT( refreshAttributes() ) );
  }
  refreshAttributes();
}

void QgsComposerAttributeTable::setMaximumNumberOfFeatures( int features )
{
  if ( features == mMaximumNumberOfFeatures )
  {
    return;
  }

  mMaximumNumberOfFeatures = features;
  refreshAttributes();
}

void QgsComposerAttributeTable::setDisplayOnlyVisibleFeatures( bool visibleOnly )
{
  if ( visibleOnly == mShowOnlyVisibleFeatures )
  {
    return;
  }

  mShowOnlyVisibleFeatures = visibleOnly;
  refreshAttributes();
}

void QgsComposerAttributeTable::setFilterFeatures( bool filter )
{
  if ( filter == mFilterFeatures )
  {
    return;
  }

  mFilterFeatures = filter;
  refreshAttributes();
}

void QgsComposerAttributeTable::setFeatureFilter( const QString& expression )
{
  if ( expression == mFeatureFilter )
  {
    return;
  }

  mFeatureFilter = expression;
  refreshAttributes();
}

QSet<int> QgsComposerAttributeTable::displayAttributes() const
{
  return fieldsToDisplay().toSet();
}

void QgsComposerAttributeTable::setDisplayAttributes( const QSet<int>& attr, bool refresh )
{
  if ( !mVectorLayer )
  {
    return;
  }

  //rebuild columns list, taking only attributes with index in supplied QSet
  qDeleteAll( mColumns );
  mColumns.clear();

  const QgsFields& fields = mVectorLayer->fields();

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
      QString currentAlias = mVectorLayer->attributeDisplayName( attrIdx );
      QgsComposerTableColumn* col = new QgsComposerTableColumn;
      col->setAttribute( fields[attrIdx].name() );
      col->setHeading( currentAlias );
      mColumns.append( col );
    }
  }
  else
  {
    //resetting, so add all attributes to columns
    int idx = 0;
    Q_FOREACH ( const QgsField& field, fields )
    {
      QString currentAlias = mVectorLayer->attributeDisplayName( idx );
      QgsComposerTableColumn* col = new QgsComposerTableColumn;
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

QMap<int, QString> QgsComposerAttributeTable::fieldAliasMap() const
{
  QMap<int, QString> fieldAliasMap;

  QList<QgsComposerTableColumn*>::const_iterator columnIt = mColumns.constBegin();
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    int attrIdx = mVectorLayer->fieldNameIndex(( *columnIt )->attribute() );
    fieldAliasMap.insert( attrIdx, ( *columnIt )->heading() );
  }
  return fieldAliasMap;
}

void QgsComposerAttributeTable::restoreFieldAliasMap( const QMap<int, QString>& map )
{
  if ( !mVectorLayer )
  {
    return;
  }

  QList<QgsComposerTableColumn*>::const_iterator columnIt = mColumns.constBegin();
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    int attrIdx = mVectorLayer->fieldNameIndex(( *columnIt )->attribute() );
    if ( map.contains( attrIdx ) )
    {
      ( *columnIt )->setHeading( map.value( attrIdx ) );
    }
    else
    {
      ( *columnIt )->setHeading( mVectorLayer->attributeDisplayName( attrIdx ) );
    }
  }
}


void QgsComposerAttributeTable::setFieldAliasMap( const QMap<int, QString>& map )
{
  restoreFieldAliasMap( map );
  refreshAttributes();
}

QList<int> QgsComposerAttributeTable::fieldsToDisplay() const
{
  //kept for api compatibility with 2.0 only, can be removed after next api break
  QList<int> fields;

  QList<QgsComposerTableColumn*>::const_iterator columnIt = mColumns.constBegin();
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    int idx = mVectorLayer->fieldNameIndex(( *columnIt )->attribute() );
    fields.append( idx );
  }
  return fields;
}

bool QgsComposerAttributeTable::getFeatureAttributes( QList<QgsAttributeMap> &attributeMaps )
{
  if ( !mVectorLayer )
  {
    return false;
  }

  QScopedPointer< QgsExpressionContext > context( createExpressionContext() );
  context->setFields( mVectorLayer->fields() );

  attributeMaps.clear();

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
    if ( mComposition->mapSettings().hasCrsTransformEnabled() )
    {
      //transform back to layer CRS
      QgsCoordinateTransform coordTransform( mVectorLayer->crs(), mComposition->mapSettings().destinationCrs() );
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
  if ( !selectionRect.isEmpty() )
    req.setFilterRect( selectionRect );

  req.setFlags( mShowOnlyVisibleFeatures ? QgsFeatureRequest::ExactIntersect : QgsFeatureRequest::NoFlags );

  QgsFeature f;
  int counter = 0;
  QgsFeatureIterator fit = mVectorLayer->getFeatures( req );

  while ( fit.nextFeature( f ) && counter < mMaximumNumberOfFeatures )
  {
    context->setFeature( f );
    //check feature against filter
    if ( activeFilter && !filterExpression.isNull() )
    {
      QVariant result = filterExpression->evaluate( context.data() );
      // skip this feature if the filter evaluation is false
      if ( !result.toBool() )
      {
        continue;
      }
    }

    attributeMaps.push_back( QgsAttributeMap() );

    QList<QgsComposerTableColumn*>::const_iterator columnIt = mColumns.constBegin();
    int i = 0;
    for ( ; columnIt != mColumns.constEnd(); ++columnIt )
    {
      int idx = mVectorLayer->fieldNameIndex(( *columnIt )->attribute() );
      if ( idx != -1 )
      {
        attributeMaps.last().insert( i, f.attributes().at( idx ) );
      }
      else
      {
        // Lets assume it's an expression
        QgsExpression* expression = new QgsExpression(( *columnIt )->attribute() );
        context->lastScope()->setVariable( QString( "row_number" ), counter + 1 );
        expression->prepare( context.data() );
        QVariant value = expression->evaluate( context.data() );
        attributeMaps.last().insert( i, value.toString() );
      }

      i++;
    }
    ++counter;
  }

  //sort the list, starting with the last attribute
  QgsComposerAttributeTableCompare c;
  QList< QPair<int, bool> > sortColumns = sortAttributes();
  for ( int i = sortColumns.size() - 1; i >= 0; --i )
  {
    c.setSortColumn( sortColumns.at( i ).first );
    c.setAscending( sortColumns.at( i ).second );
    qStableSort( attributeMaps.begin(), attributeMaps.end(), c );
  }

  adjustFrameToSize();
  return true;
}

void QgsComposerAttributeTable::removeLayer( const QString& layerId )
{
  if ( mVectorLayer )
  {
    if ( layerId == mVectorLayer->id() )
    {
      mVectorLayer = nullptr;
      //remove existing columns
      qDeleteAll( mColumns );
      mColumns.clear();
    }
  }
}

void QgsComposerAttributeTable::setSceneRect( const QRectF& rectangle )
{
  //update rect for data defined size and position
  QRectF evaluatedRect = evalItemRect( rectangle );

  QgsComposerItem::setSceneRect( evaluatedRect );

  //refresh table attributes, since number of features has likely changed
  refreshAttributes();
}

void QgsComposerAttributeTable::setSortAttributes( const QList<QPair<int, bool> >& att )
{
  //first, clear existing sort by ranks
  Q_FOREACH ( QgsComposerTableColumn* column, mColumns )
  {
    column->setSortByRank( 0 );
  }

  //now, update sort rank of specified columns
  QList< QPair<int, bool > >::const_iterator sortedColumnIt = att.constBegin();
  int rank = 1;
  for ( ; sortedColumnIt != att.constEnd(); ++sortedColumnIt )
  {
    if (( *sortedColumnIt ).first >= mColumns.length() )
    {
      continue;
    }
    mColumns.at(( *sortedColumnIt ).first )->setSortByRank( rank );
    mColumns.at(( *sortedColumnIt ).first )->setSortOrder(( *sortedColumnIt ).second ? Qt::AscendingOrder : Qt::DescendingOrder );
    rank++;
  }

  refreshAttributes();
}

static bool columnsBySortRank( QPair<int, QgsComposerTableColumn* > a, QPair<int, QgsComposerTableColumn* > b )
{
  return a.second->sortByRank() < b.second->sortByRank();
}

QList<QPair<int, bool> > QgsComposerAttributeTable::sortAttributes() const
{
  //generate list of all sorted columns
  QVector< QPair<int, QgsComposerTableColumn* > > sortedColumns;
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
  QVector< QPair<int, QgsComposerTableColumn* > >::const_iterator sortedColumnIt = sortedColumns.constBegin();
  for ( ; sortedColumnIt != sortedColumns.constEnd(); ++sortedColumnIt )
  {

    attributesBySortRank.append( qMakePair(( *sortedColumnIt ).first,
                                           ( *sortedColumnIt ).second->sortOrder() == Qt::AscendingOrder ) );
  }
  return attributesBySortRank;
}

bool QgsComposerAttributeTable::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement composerTableElem = doc.createElement( "ComposerAttributeTable" );
  composerTableElem.setAttribute( "showOnlyVisibleFeatures", mShowOnlyVisibleFeatures );
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

  elem.appendChild( composerTableElem );
  bool ok = tableWriteXML( composerTableElem, doc );
  return ok;
}

bool QgsComposerAttributeTable::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  //read general table properties
  if ( !tableReadXML( itemElem, doc ) )
  {
    return false;
  }

  mShowOnlyVisibleFeatures = itemElem.attribute( "showOnlyVisibleFeatures", "1" ).toInt();
  mFilterFeatures = itemElem.attribute( "filterFeatures", "false" ) == "true" ? true : false;
  mFeatureFilter = itemElem.attribute( "featureFilter", "" );

  //composer map
  int composerMapId = itemElem.attribute( "composerMap", "-1" ).toInt();
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
    QObject::connect( mComposerMap, SIGNAL( extentChanged() ), this, SLOT( refreshAttributes() ) );
  }

  //vector layer
  QString layerId = itemElem.attribute( "vectorLayer", "not_existing" );
  if ( layerId == "not_existing" )
  {
    mVectorLayer = nullptr;
  }
  else
  {
    QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );
    if ( ml )
    {
      mVectorLayer = dynamic_cast<QgsVectorLayer*>( ml );
      if ( mVectorLayer )
      {
        //if we have found a valid vector layer, listen for modifications on it and refresh the table
        QObject::connect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( refreshAttributes() ) );
      }
    }
  }

  //restore display attribute map. This is required to upgrade pre 2.4 projects.
  QSet<int> displayAttributes;
  QDomNodeList displayAttributeList = itemElem.elementsByTagName( "displayAttributes" );
  if ( !displayAttributeList.isEmpty() )
  {
    QDomElement displayAttributesElem =  displayAttributeList.at( 0 ).toElement();
    QDomNodeList attributeEntryList = displayAttributesElem.elementsByTagName( "attributeEntry" );
    for ( int i = 0; i < attributeEntryList.size(); ++i )
    {
      QDomElement attributeEntryElem = attributeEntryList.at( i ).toElement();
      int index = attributeEntryElem.attribute( "index", "-1" ).toInt();
      if ( index != -1 )
      {
        displayAttributes.insert( index );
      }
    }
    setDisplayAttributes( displayAttributes, false );
  }

  //restore alias map. This is required to upgrade pre 2.4 projects.
  QMap<int, QString> fieldAliasMap;
  QDomNodeList aliasMapNodeList = itemElem.elementsByTagName( "attributeAliasMap" );
  if ( !aliasMapNodeList.isEmpty() )
  {
    QDomElement attributeAliasMapElem = aliasMapNodeList.at( 0 ).toElement();
    QDomNodeList aliasMepEntryList = attributeAliasMapElem.elementsByTagName( "aliasEntry" );
    for ( int i = 0; i < aliasMepEntryList.size(); ++i )
    {
      QDomElement aliasEntryElem = aliasMepEntryList.at( i ).toElement();
      int key = aliasEntryElem.attribute( "key", "-1" ).toInt();
      QString value = aliasEntryElem.attribute( "value", "" );
      fieldAliasMap.insert( key, value );
    }
    restoreFieldAliasMap( fieldAliasMap );
  }

  //restore sort columns. This is required to upgrade pre 2.4 projects.
  QDomElement sortColumnsElem = itemElem.firstChildElement( "sortColumns" );
  if ( !sortColumnsElem.isNull() && mVectorLayer )
  {
    QDomNodeList columns = sortColumnsElem.elementsByTagName( "column" );
    const QgsFields& fields = mVectorLayer->fields();

    for ( int i = 0; i < columns.size(); ++i )
    {
      QDomElement columnElem = columns.at( i ).toElement();
      int attribute = columnElem.attribute( "index" ).toInt();
      Qt::SortOrder order = columnElem.attribute( "ascending" ) == "true" ? Qt::AscendingOrder : Qt::DescendingOrder;
      //find corresponding column
      Q_FOREACH ( QgsComposerTableColumn* column, mColumns )
      {
        if ( column->attribute() == fields[attribute].name() )
        {
          column->setSortByRank( i + 1 );
          column->setSortOrder( order );
          break;
        }
      }
    }
  }

  //must be done here because tableReadXML->setSceneRect changes mMaximumNumberOfFeatures
  mMaximumNumberOfFeatures = itemElem.attribute( "maxFeatures", "5" ).toInt();

  refreshAttributes();

  emit itemChanged();
  return true;
}
