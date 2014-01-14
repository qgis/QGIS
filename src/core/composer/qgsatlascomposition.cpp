/***************************************************************************
                             qgsatlascomposition.cpp
                             -----------------------
    begin                : October 2012
    copyright            : (C) 2005 by Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdexcept>

#include "qgsatlascomposition.h"
#include "qgsvectorlayer.h"
#include "qgscomposermap.h"
#include "qgscomposition.h"
#include "qgsvectordataprovider.h"
#include "qgsexpression.h"
#include "qgsgeometry.h"
#include "qgscomposerlabel.h"
#include "qgscomposershape.h"
#include "qgspaperitem.h"
#include "qgsmaplayerregistry.h"

QgsAtlasComposition::QgsAtlasComposition( QgsComposition* composition ) :
    mComposition( composition ),
    mEnabled( false ),
    mHideCoverage( false ), mFilenamePattern( "'output_'||$feature" ),
    mCoverageLayer( 0 ), mSingleFile( false ),
    mSortFeatures( false ), mSortAscending( true ), mCurrentFeatureNo( 0 ),
    mFilterFeatures( false ), mFeatureFilter( "" )
{

  // declare special columns with a default value
  QgsExpression::setSpecialColumn( "$page", QVariant(( int )1 ) );
  QgsExpression::setSpecialColumn( "$feature", QVariant(( int )0 ) );
  QgsExpression::setSpecialColumn( "$numpages", QVariant(( int )1 ) );
  QgsExpression::setSpecialColumn( "$numfeatures", QVariant(( int )0 ) );
  QgsExpression::setSpecialColumn( "$atlasfeatureid", QVariant(( int )0 ) );
  QgsExpression::setSpecialColumn( "$atlasgeometry", QVariant::fromValue( QgsGeometry() ) );
}

QgsAtlasComposition::~QgsAtlasComposition()
{
}

void QgsAtlasComposition::setEnabled( bool e )
{
  mEnabled = e;
  mComposition->setAtlasMode( QgsComposition::AtlasOff );
  emit toggled( e );
}

void QgsAtlasComposition::setCoverageLayer( QgsVectorLayer* layer )
{
  mCoverageLayer = layer;

  // update the number of features
  QgsExpression::setSpecialColumn( "$numfeatures", QVariant(( int )mFeatureIds.size() ) );

  // Grab the first feature so that user can use it to test the style in rules.
  QgsFeature fet;
  layer->getFeatures().nextFeature( fet );
  QgsExpression::setSpecialColumn( "$atlasfeatureid", fet.id() );
  QgsExpression::setSpecialColumn( "$atlasgeometry", QVariant::fromValue( *fet.geometry() ) );

  emit coverageLayerChanged( layer );
}

QgsComposerMap* QgsAtlasComposition::composerMap() const
{
  //deprecated method. Until removed just return the first atlas-enabled composer map

  //build a list of composer maps
  QList<QgsComposerMap*> maps;
  mComposition->composerItems( maps );
  for ( QList<QgsComposerMap*>::iterator mit = maps.begin(); mit != maps.end(); ++mit )
  {
    QgsComposerMap* currentMap = ( *mit );
    if ( currentMap->atlasDriven() )
    {
      return currentMap;
    }
  }

  return 0;
}

void QgsAtlasComposition::setComposerMap( QgsComposerMap* map )
{
  //deprecated

  if ( !map )
  {
    return;
  }

  map->setAtlasDriven( true );
}

bool QgsAtlasComposition::fixedScale() const
{
  //deprecated method. Until removed just return the property for the first atlas-enabled composer map
  QgsComposerMap * map = composerMap();
  if ( !map )
  {
    return false;
  }

  return map->atlasFixedScale();
}

void QgsAtlasComposition::setFixedScale( bool fixed )
{
  //deprecated method. Until removed just set the property for the first atlas-enabled composer map
  QgsComposerMap * map = composerMap();
  if ( !map )
  {
    return;
  }

  map->setAtlasFixedScale( fixed );
}

float QgsAtlasComposition::margin() const
{
  //deprecated method. Until removed just return the property for the first atlas-enabled composer map
  QgsComposerMap * map = composerMap();
  if ( !map )
  {
    return 0;
  }

  return map->atlasMargin();
}

void QgsAtlasComposition::setMargin( float margin )
{
  //deprecated method. Until removed just set the property for the first atlas-enabled composer map
  QgsComposerMap * map = composerMap();
  if ( !map )
  {
    return;
  }

  map->setAtlasMargin(( double ) margin );
}

//
// Private class only used for the sorting of features
class FieldSorter
{
  public:
    FieldSorter( QgsAtlasComposition::SorterKeys& keys, bool ascending = true ) : mKeys( keys ), mAscending( ascending ) {}

    bool operator()( const QgsFeatureId& id1, const QgsFeatureId& id2 )
    {
      bool result = true;

      if ( mKeys[ id1 ].type() == QVariant::Int )
      {
        result = mKeys[ id1 ].toInt() < mKeys[ id2 ].toInt();
      }
      else if ( mKeys[ id1 ].type() == QVariant::Double )
      {
        result = mKeys[ id1 ].toDouble() < mKeys[ id2 ].toDouble();
      }
      else if ( mKeys[ id1 ].type() == QVariant::String )
      {
        result = ( QString::localeAwareCompare( mKeys[ id1 ].toString(), mKeys[ id2 ].toString() ) < 0 );
      }

      return mAscending ? result : !result;
    }
  private:
    QgsAtlasComposition::SorterKeys& mKeys;
    bool mAscending;
};

int QgsAtlasComposition::updateFeatures()
{
  //needs to be called when layer, filter, sort changes

  if ( !mCoverageLayer )
  {
    return 0;
  }

  updateFilenameExpression();

  // select all features with all attributes
  QgsFeatureIterator fit = mCoverageLayer->getFeatures();

  std::auto_ptr<QgsExpression> filterExpression;
  if ( mFilterFeatures && !mFeatureFilter.isEmpty() )
  {
    filterExpression = std::auto_ptr<QgsExpression>( new QgsExpression( mFeatureFilter ) );
    if ( filterExpression->hasParserError() )
    {
      throw std::runtime_error( tr( "Feature filter parser error: %1" ).arg( filterExpression->parserErrorString() ).toLocal8Bit().data() );
    }
  }

  // We cannot use nextFeature() directly since the feature pointer is rewinded by the rendering process
  // We thus store the feature ids for future extraction
  QgsFeature feat;
  mFeatureIds.clear();
  mFeatureKeys.clear();
  while ( fit.nextFeature( feat ) )
  {
    if ( mFilterFeatures && !mFeatureFilter.isEmpty() )
    {
      QVariant result = filterExpression->evaluate( &feat, mCoverageLayer->pendingFields() );
      if ( filterExpression->hasEvalError() )
      {
        throw std::runtime_error( tr( "Feature filter eval error: %1" ).arg( filterExpression->evalErrorString() ).toLocal8Bit().data() );
      }

      // skip this feature if the filter evaluation if false
      if ( !result.toBool() )
      {
        continue;
      }
    }
    mFeatureIds.push_back( feat.id() );

    if ( mSortFeatures )
    {
      mFeatureKeys.insert( feat.id(), feat.attributes()[ mSortKeyAttributeIdx ] );
    }
  }

  // sort features, if asked for
  if ( mSortFeatures )
  {
    FieldSorter sorter( mFeatureKeys, mSortAscending );
    qSort( mFeatureIds.begin(), mFeatureIds.end(), sorter );
  }

  QgsExpression::setSpecialColumn( "$numfeatures", QVariant(( int )mFeatureIds.size() ) );

  //jump to first feature if currently using an atlas preview
  //need to do this in case filtering/layer change has altered matching features
  if ( mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    firstFeature();
  }

  return mFeatureIds.size();
}


bool QgsAtlasComposition::beginRender()
{
  if ( !mCoverageLayer )
  {
    return false;
  }

  bool featuresUpdated = updateFeatures();
  if ( !featuresUpdated )
  {
    //no matching features found
    return false;
  }

  mRestoreLayer = false;
  QStringList& layerSet = mComposition->mapRenderer()->layerSet();
  if ( mHideCoverage )
  {
    // look for the layer in the renderer's set
    int removeAt = layerSet.indexOf( mCoverageLayer->id() );
    if ( removeAt != -1 )
    {
      mRestoreLayer = true;
      layerSet.removeAt( removeAt );
    }
  }

  // special columns for expressions
  QgsExpression::setSpecialColumn( "$numpages", QVariant( mComposition->numPages() ) );
  QgsExpression::setSpecialColumn( "$numfeatures", QVariant(( int )mFeatureIds.size() ) );

  return true;
}

void QgsAtlasComposition::endRender()
{
  if ( !mCoverageLayer )
  {
    return;
  }

  // reset label expression contexts
  QList<QgsComposerLabel*> labels;
  mComposition->composerItems( labels );
  for ( QList<QgsComposerLabel*>::iterator lit = labels.begin(); lit != labels.end(); ++lit )
  {
    ( *lit )->setExpressionContext( 0, 0 );
  }

  // restore the coverage visibility
  if ( mRestoreLayer )
  {
    QStringList& layerSet = mComposition->mapRenderer()->layerSet();

    layerSet.push_back( mCoverageLayer->id() );
  }

  updateAtlasMaps();
}

void QgsAtlasComposition::updateAtlasMaps()
{
  //update atlas-enabled composer maps
  QList<QgsComposerMap*> maps;
  mComposition->composerItems( maps );
  for ( QList<QgsComposerMap*>::iterator mit = maps.begin(); mit != maps.end(); ++mit )
  {
    QgsComposerMap* currentMap = ( *mit );
    if ( !currentMap->atlasDriven() )
    {
      continue;
    }

    currentMap->cache();
  }
}

int QgsAtlasComposition::numFeatures() const
{
  return mFeatureIds.size();
}

void QgsAtlasComposition::nextFeature()
{
  mCurrentFeatureNo++;
  if ( mCurrentFeatureNo >= mFeatureIds.size() )
  {
    mCurrentFeatureNo = mFeatureIds.size() - 1;
  }

  prepareForFeature( mCurrentFeatureNo );
}

void QgsAtlasComposition::prevFeature()
{
  mCurrentFeatureNo--;
  if ( mCurrentFeatureNo < 0 )
  {
    mCurrentFeatureNo = 0;
  }

  prepareForFeature( mCurrentFeatureNo );
}

void QgsAtlasComposition::firstFeature()
{
  mCurrentFeatureNo = 0;
  prepareForFeature( mCurrentFeatureNo );
}

void QgsAtlasComposition::lastFeature()
{
  mCurrentFeatureNo = mFeatureIds.size() - 1;
  prepareForFeature( mCurrentFeatureNo );
}

void QgsAtlasComposition::prepareForFeature( int featureI )
{
  if ( !mCoverageLayer )
  {
    return;
  }

  if ( mFeatureIds.size() == 0 )
  {
    emit statusMsgChanged( tr( "No matching atlas features" ) );
    return;
  }

  // retrieve the next feature, based on its id
  mCoverageLayer->getFeatures( QgsFeatureRequest().setFilterFid( mFeatureIds[ featureI ] ) ).nextFeature( mCurrentFeature );

  QgsExpression::setSpecialColumn( "$atlasfeatureid", mCurrentFeature.id() );
  QgsExpression::setSpecialColumn( "$atlasgeometry", QVariant::fromValue( *mCurrentFeature.geometry() ) );
  QgsExpression::setSpecialColumn( "$feature", QVariant(( int )featureI + 1 ) );

  // generate filename for current feature
  evalFeatureFilename();

  // evaluate label expressions
  QList<QgsComposerLabel*> labels;
  mComposition->composerItems( labels );
  for ( QList<QgsComposerLabel*>::iterator lit = labels.begin(); lit != labels.end(); ++lit )
  {
    ( *lit )->setExpressionContext( &mCurrentFeature, mCoverageLayer );
  }

  // update shapes (in case they use data defined symbology with atlas properties)
  QList<QgsComposerShape*> shapes;
  mComposition->composerItems( shapes );
  for ( QList<QgsComposerShape*>::iterator lit = shapes.begin(); lit != shapes.end(); ++lit )
  {
    ( *lit )->update();
  }

  // update page background (in case it uses data defined symbology with atlas properties)
  QList<QgsPaperItem*> pages;
  mComposition->composerItems( pages );
  for ( QList<QgsPaperItem*>::iterator pageIt = pages.begin(); pageIt != pages.end(); ++pageIt )
  {
    ( *pageIt )->update();
  }

  emit statusMsgChanged( QString( tr( "Atlas feature %1 of %2" ) ).arg( featureI + 1 ).arg( mFeatureIds.size() ) );

  //update composer maps

  //build a list of atlas-enabled composer maps
  QList<QgsComposerMap*> maps;
  QList<QgsComposerMap*> atlasMaps;
  mComposition->composerItems( maps );
  for ( QList<QgsComposerMap*>::iterator mit = maps.begin(); mit != maps.end(); ++mit )
  {
    QgsComposerMap* currentMap = ( *mit );
    if ( !currentMap->atlasDriven() )
    {
      continue;
    }
    atlasMaps << currentMap;
  }

  if ( atlasMaps.isEmpty() )
  {
    //no atlas enabled maps
    return;
  }

  //
  // compute the new extent
  // keep the original aspect ratio
  // and apply a margin

  const QgsCoordinateReferenceSystem& coverage_crs = mCoverageLayer->crs();
  // transformation needed for feature geometries. This should be set on a per-atlas map basis,
  // but given that it's not currently possible to have maps with different CRSes we can just
  // calculate it once based on the first atlas maps' CRS.
  const QgsCoordinateReferenceSystem& destination_crs = atlasMaps[0]->mapRenderer()->destinationCrs();
  mTransform.setSourceCrs( coverage_crs );
  mTransform.setDestCRS( destination_crs );

  // QgsGeometry::boundingBox is expressed in the geometry"s native CRS
  // We have to transform the grometry to the destination CRS and ask for the bounding box
  // Note: we cannot directly take the transformation of the bounding box, since transformations are not linear

  QgsGeometry tgeom( *mCurrentFeature.geometry() );
  tgeom.transform( mTransform );
  mTransformedFeatureBounds = tgeom.boundingBox();

  //update atlas bounds of every atlas enabled composer map
  for ( QList<QgsComposerMap*>::iterator mit = atlasMaps.begin(); mit != atlasMaps.end(); ++mit )
  {
    prepareMap( *mit );
  }
}

void QgsAtlasComposition::prepareMap( QgsComposerMap* map )
{
  if ( !map->atlasDriven() )
  {
    return;
  }

  double xa1 = mTransformedFeatureBounds.xMinimum();
  double xa2 = mTransformedFeatureBounds.xMaximum();
  double ya1 = mTransformedFeatureBounds.yMinimum();
  double ya2 = mTransformedFeatureBounds.yMaximum();
  QgsRectangle new_extent = mTransformedFeatureBounds;
  QgsRectangle mOrigExtent = map->extent();

  if ( map->atlasFixedScale() )
  {
    // only translate, keep the original scale (i.e. width x height)

    double geom_center_x = ( xa1 + xa2 ) / 2.0;
    double geom_center_y = ( ya1 + ya2 ) / 2.0;
    double xx = geom_center_x - mOrigExtent.width() / 2.0;
    double yy = geom_center_y - mOrigExtent.height() / 2.0;
    new_extent = QgsRectangle( xx,
                               yy,
                               xx + mOrigExtent.width(),
                               yy + mOrigExtent.height() );
  }
  else
  {
    // auto scale

    double geom_ratio = mTransformedFeatureBounds.width() / mTransformedFeatureBounds.height();
    double map_ratio = mOrigExtent.width() / mOrigExtent.height();

    // geometry height is too big
    if ( geom_ratio < map_ratio )
    {
      // extent the bbox's width
      double adj_width = ( map_ratio * mTransformedFeatureBounds.height() - mTransformedFeatureBounds.width() ) / 2.0;
      xa1 -= adj_width;
      xa2 += adj_width;
    }
    // geometry width is too big
    else if ( geom_ratio > map_ratio )
    {
      // extent the bbox's height
      double adj_height = ( mTransformedFeatureBounds.width() / map_ratio - mTransformedFeatureBounds.height() ) / 2.0;
      ya1 -= adj_height;
      ya2 += adj_height;
    }
    new_extent = QgsRectangle( xa1, ya1, xa2, ya2 );

    if ( map->atlasMargin() > 0.0 )
    {
      new_extent.scale( 1 + map->atlasMargin() );
    }
  }

  // set the new extent (and render)
  map->setNewAtlasFeatureExtent( new_extent );
}

const QString& QgsAtlasComposition::currentFilename() const
{
  return mCurrentFilename;
}

void QgsAtlasComposition::writeXML( QDomElement& elem, QDomDocument& doc ) const
{
  QDomElement atlasElem = doc.createElement( "Atlas" );
  atlasElem.setAttribute( "enabled", mEnabled ? "true" : "false" );
  if ( !mEnabled )
  {
    return;
  }

  if ( mCoverageLayer )
  {
    atlasElem.setAttribute( "coverageLayer", mCoverageLayer->id() );
  }
  else
  {
    atlasElem.setAttribute( "coverageLayer", "" );
  }

  atlasElem.setAttribute( "hideCoverage", mHideCoverage ? "true" : "false" );
  atlasElem.setAttribute( "singleFile", mSingleFile ? "true" : "false" );
  atlasElem.setAttribute( "filenamePattern", mFilenamePattern );

  atlasElem.setAttribute( "sortFeatures", mSortFeatures ? "true" : "false" );
  if ( mSortFeatures )
  {
    atlasElem.setAttribute( "sortKey", QString::number( mSortKeyAttributeIdx ) );
    atlasElem.setAttribute( "sortAscending", mSortAscending ? "true" : "false" );
  }
  atlasElem.setAttribute( "filterFeatures", mFilterFeatures ? "true" : "false" );
  if ( mFilterFeatures )
  {
    atlasElem.setAttribute( "featureFilter", mFeatureFilter );
  }

  elem.appendChild( atlasElem );
}

void QgsAtlasComposition::readXML( const QDomElement& atlasElem, const QDomDocument& )
{
  mEnabled = atlasElem.attribute( "enabled", "false" ) == "true" ? true : false;
  emit toggled( mEnabled );
  if ( !mEnabled )
  {
    emit parameterChanged();
    return;
  }

  // look for stored layer name
  mCoverageLayer = 0;
  QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer*>::const_iterator it = layers.begin(); it != layers.end(); ++it )
  {
    if ( it.key() == atlasElem.attribute( "coverageLayer" ) )
    {
      mCoverageLayer = dynamic_cast<QgsVectorLayer*>( it.value() );
      break;
    }
  }
  //look for stored composer map, to upgrade pre 2.1 projects
  int composerMapNo = atlasElem.attribute( "composerMap", "0" ).toInt();
  QgsComposerMap * composerMap = 0;
  if ( composerMapNo != 0 )
  {
    QList<QgsComposerMap*> maps;
    mComposition->composerItems( maps );
    for ( QList<QgsComposerMap*>::iterator it = maps.begin(); it != maps.end(); ++it )
    {
      if (( *it )->id() == composerMapNo )
      {
        ( *it )->setAtlasDriven( true );
        break;
      }
    }
  }
  mHideCoverage = atlasElem.attribute( "hideCoverage", "false" ) == "true" ? true : false;

  //upgrade pre 2.1 projects
  double margin = atlasElem.attribute( "margin", "0.0" ).toDouble();
  if ( composerMap && margin != 0 )
  {
    composerMap->setAtlasMargin( margin );
  }
  bool fixedScale = atlasElem.attribute( "fixedScale", "false" ) == "true" ? true : false;
  if ( composerMap && fixedScale )
  {
    composerMap->setAtlasFixedScale( true );
  }

  mSingleFile = atlasElem.attribute( "singleFile", "false" ) == "true" ? true : false;
  mFilenamePattern = atlasElem.attribute( "filenamePattern", "" );

  mSortFeatures = atlasElem.attribute( "sortFeatures", "false" ) == "true" ? true : false;
  if ( mSortFeatures )
  {
    mSortKeyAttributeIdx = atlasElem.attribute( "sortKey", "0" ).toInt();
    mSortAscending = atlasElem.attribute( "sortAscending", "true" ) == "true" ? true : false;
  }
  mFilterFeatures = atlasElem.attribute( "filterFeatures", "false" ) == "true" ? true : false;
  if ( mFilterFeatures )
  {
    mFeatureFilter = atlasElem.attribute( "featureFilter", "" );
  }

  emit parameterChanged();
}

void QgsAtlasComposition::setHideCoverage( bool hide )
{
  mHideCoverage = hide;

  if ( mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    //an atlas preview is enabled, so reflect changes in coverage layer visibility immediately
    QStringList& layerSet = mComposition->mapRenderer()->layerSet();
    if ( hide )
    {
      // look for the layer in the renderer's set
      int removeAt = layerSet.indexOf( mCoverageLayer->id() );
      if ( removeAt != -1 )
      {
        mRestoreLayer = true;
        layerSet.removeAt( removeAt );
      }
    }
    else
    {
      if ( mRestoreLayer )
      {
        layerSet.push_back( mCoverageLayer->id() );
        mRestoreLayer = false;
      }
    }
    updateAtlasMaps();
    mComposition->update();
  }

}

void QgsAtlasComposition::setFilenamePattern( const QString& pattern )
{
  mFilenamePattern = pattern;
  updateFilenameExpression();
}

void QgsAtlasComposition::updateFilenameExpression()
{
  const QgsFields& fields = mCoverageLayer->pendingFields();

  if ( !mSingleFile && mFilenamePattern.size() > 0 )
  {
    mFilenameExpr = std::auto_ptr<QgsExpression>( new QgsExpression( mFilenamePattern ) );
    // expression used to evaluate each filename
    // test for evaluation errors
    if ( mFilenameExpr->hasParserError() )
    {
      throw std::runtime_error( tr( "Filename parsing error: %1" ).arg( mFilenameExpr->parserErrorString() ).toLocal8Bit().data() );
    }

    // prepare the filename expression
    mFilenameExpr->prepare( fields );
  }

  //if atlas preview is currently enabled, regenerate filename for current feature
  if ( mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    evalFeatureFilename();
  }

}

void QgsAtlasComposition::evalFeatureFilename()
{
  //generate filename for current atlas feature
  if ( !mSingleFile && mFilenamePattern.size() > 0 )
  {
    QVariant filenameRes = mFilenameExpr->evaluate( &mCurrentFeature, mCoverageLayer->pendingFields() );
    if ( mFilenameExpr->hasEvalError() )
    {
      throw std::runtime_error( tr( "Filename eval error: %1" ).arg( mFilenameExpr->evalErrorString() ).toLocal8Bit().data() );
    }

    mCurrentFilename = filenameRes.toString();
  }
}


