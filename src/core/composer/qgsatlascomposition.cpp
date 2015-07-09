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
#include <QtAlgorithms>

#include "qgsatlascomposition.h"
#include "qgsvectorlayer.h"
#include "qgscomposermap.h"
#include "qgscomposition.h"
#include "qgsvectordataprovider.h"
#include "qgsexpression.h"
#include "qgsgeometry.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"

QgsAtlasComposition::QgsAtlasComposition( QgsComposition* composition ) :
    mComposition( composition ),
    mEnabled( false ),
    mHideCoverage( false ), mFilenamePattern( "'output_'||$feature" ),
    mCoverageLayer( 0 ), mSingleFile( false ),
    mSortFeatures( false ), mSortAscending( true ), mCurrentFeatureNo( 0 ),
    mFilterFeatures( false ), mFeatureFilter( "" ),
    mFilenameParserError( QString() ),
    mFilterParserError( QString() )
{

  // declare special columns with a default value
  QgsExpression::setSpecialColumn( "$page", QVariant(( int )1 ) );
  QgsExpression::setSpecialColumn( "$feature", QVariant(( int )0 ) );
  QgsExpression::setSpecialColumn( "$numpages", QVariant(( int )1 ) );
  QgsExpression::setSpecialColumn( "$numfeatures", QVariant(( int )0 ) );
  QgsExpression::setSpecialColumn( "$atlasfeatureid", QVariant(( int )0 ) );
  QgsExpression::setSpecialColumn( "$atlasfeature", QVariant::fromValue( QgsFeature() ) );
  QgsExpression::setSpecialColumn( "$atlasgeometry", QVariant::fromValue( QgsGeometry() ) );

  //listen out for layer removal
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( removeLayers( QStringList ) ) );
}

QgsAtlasComposition::~QgsAtlasComposition()
{
}

void QgsAtlasComposition::setEnabled( bool enabled )
{
  if ( enabled == mEnabled )
  {
    return;
  }

  mEnabled = enabled;
  mComposition->setAtlasMode( QgsComposition::AtlasOff );
  emit toggled( enabled );
  emit parameterChanged();
}

void QgsAtlasComposition::removeLayers( QStringList layers )
{
  if ( !mCoverageLayer )
  {
    return;
  }

  foreach ( QString layerId, layers )
  {
    if ( layerId == mCoverageLayer->id() )
    {
      //current coverage layer removed
      mCoverageLayer = 0;
      setEnabled( false );
      return;
    }
  }
}

void QgsAtlasComposition::setCoverageLayer( QgsVectorLayer* layer )
{
  if ( layer == mCoverageLayer )
  {
    return;
  }

  mCoverageLayer = layer;

  // update the number of features
  QgsExpression::setSpecialColumn( "$numfeatures", QVariant(( int )mFeatureIds.size() ) );

  // Grab the first feature so that user can use it to test the style in rules.
  if ( layer )
  {
    QgsFeature fet;
    layer->getFeatures().nextFeature( fet );
    QgsExpression::setSpecialColumn( "$atlasfeatureid", fet.id() );
    QgsExpression::setSpecialColumn( "$atlasgeometry", QVariant::fromValue( *fet.constGeometry() ) );
    QgsExpression::setSpecialColumn( "$atlasfeature", QVariant::fromValue( fet ) );
  }

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


int QgsAtlasComposition::sortKeyAttributeIndex() const
{
  if ( !mCoverageLayer )
  {
    return -1;
  }
  return mCoverageLayer->fieldNameIndex( mSortKeyAttributeName );
}

void QgsAtlasComposition::setSortKeyAttributeIndex( int idx )
{
  if ( mCoverageLayer )
  {
    const QgsFields fields = mCoverageLayer->pendingFields();
    if ( idx >= 0 && idx < fields.count() )
    {
      mSortKeyAttributeName = fields[idx].name();
      return;
    }
  }
  mSortKeyAttributeName = "";
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

  QScopedPointer<QgsExpression> filterExpression;
  if ( mFilterFeatures && !mFeatureFilter.isEmpty() )
  {
    filterExpression.reset( new QgsExpression( mFeatureFilter ) );
    if ( filterExpression->hasParserError() )
    {
      mFilterParserError = filterExpression->parserErrorString();
      return 0;
    }
  }
  mFilterParserError = QString();

  // We cannot use nextFeature() directly since the feature pointer is rewinded by the rendering process
  // We thus store the feature ids for future extraction
  QgsFeature feat;
  mFeatureIds.clear();
  mFeatureKeys.clear();
  int sortIdx = mCoverageLayer->fieldNameIndex( mSortKeyAttributeName );
  while ( fit.nextFeature( feat ) )
  {
    if ( !filterExpression.isNull() )
    {
      QVariant result = filterExpression->evaluate( &feat, mCoverageLayer->pendingFields() );
      if ( filterExpression->hasEvalError() )
      {
        QgsMessageLog::logMessage( tr( "Atlas filter eval error: %1" ).arg( filterExpression->evalErrorString() ), tr( "Composer" ) );
      }

      // skip this feature if the filter evaluation if false
      if ( !result.toBool() )
      {
        continue;
      }
    }
    mFeatureIds.push_back( feat.id() );

    if ( mSortFeatures && sortIdx != -1 )
    {
      mFeatureKeys.insert( feat.id(), feat.attributes()[ sortIdx ] );
    }
  }

  // sort features, if asked for
  if ( mFeatureKeys.count() )
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

  emit renderBegun();

  bool featuresUpdated = updateFeatures();
  if ( !featuresUpdated )
  {
    //no matching features found
    return false;
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

  emit featureChanged( 0 );

  updateAtlasMaps();

  emit renderEnded();
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
  int newFeatureNo = mCurrentFeatureNo + 1;
  if ( newFeatureNo >= mFeatureIds.size() )
  {
    newFeatureNo = mFeatureIds.size() - 1;
  }

  prepareForFeature( newFeatureNo );
}

void QgsAtlasComposition::prevFeature()
{
  int newFeatureNo = mCurrentFeatureNo - 1;
  if ( newFeatureNo < 0 )
  {
    newFeatureNo = 0;
  }

  prepareForFeature( newFeatureNo );
}

void QgsAtlasComposition::firstFeature()
{
  prepareForFeature( 0 );
}

void QgsAtlasComposition::lastFeature()
{
  prepareForFeature( mFeatureIds.size() - 1 );
}

bool QgsAtlasComposition::prepareForFeature( const QgsFeature * feat )
{
  int featureI = mFeatureIds.indexOf( feat->id() );
  if ( featureI < 0 )
  {
    //feature not found
    return false;
  }

  return prepareForFeature( featureI );
}

void QgsAtlasComposition::refreshFeature()
{
  prepareForFeature( mCurrentFeatureNo, false );
}

bool QgsAtlasComposition::prepareForFeature( const int featureI, const bool updateMaps )
{
  if ( !mCoverageLayer )
  {
    return false;
  }

  if ( mFeatureIds.size() == 0 )
  {
    emit statusMsgChanged( tr( "No matching atlas features" ) );
    return false;
  }

  mCurrentFeatureNo = featureI;

  // retrieve the next feature, based on its id
  mCoverageLayer->getFeatures( QgsFeatureRequest().setFilterFid( mFeatureIds[ featureI ] ) ).nextFeature( mCurrentFeature );

  QgsExpression::setSpecialColumn( "$atlasfeatureid", mCurrentFeature.id() );
  QgsExpression::setSpecialColumn( "$atlasgeometry", QVariant::fromValue( *mCurrentFeature.constGeometry() ) );
  QgsExpression::setSpecialColumn( "$atlasfeature", QVariant::fromValue( mCurrentFeature ) );
  QgsExpression::setSpecialColumn( "$feature", QVariant(( int )featureI + 1 ) );

  // generate filename for current feature
  if ( !evalFeatureFilename() )
  {
    //error evaluating filename
    return false;
  }

  emit featureChanged( &mCurrentFeature );
  emit statusMsgChanged( QString( tr( "Atlas feature %1 of %2" ) ).arg( featureI + 1 ).arg( mFeatureIds.size() ) );

  if ( !mCurrentFeature.isValid() )
  {
    //bad feature
    return true;
  }

  if ( !updateMaps )
  {
    //nothing more to do
    return true;
  }

  //update composer maps

  //build a list of atlas-enabled composer maps
  QList<QgsComposerMap*> maps;
  QList<QgsComposerMap*> atlasMaps;
  mComposition->composerItems( maps );
  if ( maps.isEmpty() )
  {
    return true;
  }
  for ( QList<QgsComposerMap*>::iterator mit = maps.begin(); mit != maps.end(); ++mit )
  {
    QgsComposerMap* currentMap = ( *mit );
    if ( !currentMap->atlasDriven() )
    {
      continue;
    }
    atlasMaps << currentMap;
  }

  if ( atlasMaps.count() > 0 )
  {
    //clear the transformed bounds of the previous feature
    mTransformedFeatureBounds = QgsRectangle();

    // compute extent of current feature in the map CRS. This should be set on a per-atlas map basis,
    // but given that it's not currently possible to have maps with different CRSes we can just
    // calculate it once based on the first atlas maps' CRS.
    computeExtent( atlasMaps[0] );
  }

  for ( QList<QgsComposerMap*>::iterator mit = maps.begin(); mit != maps.end(); ++mit )
  {
    if (( *mit )->atlasDriven() )
    {
      // map is atlas driven, so update it's bounds (causes a redraw)
      prepareMap( *mit );
    }
    else
    {
      // map is not atlas driven, so manually force a redraw (to reflect possibly atlas
      // dependent symbology)
      ( *mit )->cache();
    }
  }

  return true;
}

void QgsAtlasComposition::computeExtent( QgsComposerMap* map )
{
  // compute the extent of the current feature, in the crs of the specified map

  const QgsCoordinateReferenceSystem& coverage_crs = mCoverageLayer->crs();
  // transformation needed for feature geometries
  const QgsCoordinateReferenceSystem& destination_crs = map->composition()->mapSettings().destinationCrs();
  mTransform.setSourceCrs( coverage_crs );
  mTransform.setDestCRS( destination_crs );

  // QgsGeometry::boundingBox is expressed in the geometry"s native CRS
  // We have to transform the grometry to the destination CRS and ask for the bounding box
  // Note: we cannot directly take the transformation of the bounding box, since transformations are not linear
  QgsGeometry tgeom( *mCurrentFeature.constGeometry() );
  tgeom.transform( mTransform );
  mTransformedFeatureBounds = tgeom.boundingBox();
}

void QgsAtlasComposition::prepareMap( QgsComposerMap* map )
{
  if ( !map->atlasDriven() )
  {
    return;
  }

  if ( mTransformedFeatureBounds.isEmpty() )
  {
    //transformed extent of current feature hasn't been calculated yet. This can happen if
    //a map has been set to be atlas controlled after prepare feature was called
    computeExtent( map );
  }

  double xa1 = mTransformedFeatureBounds.xMinimum();
  double xa2 = mTransformedFeatureBounds.xMaximum();
  double ya1 = mTransformedFeatureBounds.yMinimum();
  double ya2 = mTransformedFeatureBounds.yMaximum();
  QgsRectangle newExtent = mTransformedFeatureBounds;
  QgsRectangle mOrigExtent( map->extent() );

  //sanity check - only allow fixed scale mode for point layers
  bool isPointLayer = false;
  switch ( mCoverageLayer->wkbType() )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      isPointLayer = true;
      break;
    default:
      isPointLayer = false;
      break;
  }

  if ( map->atlasScalingMode() == QgsComposerMap::Fixed || map->atlasScalingMode() == QgsComposerMap::Predefined || isPointLayer )
  {
    QgsScaleCalculator calc;
    calc.setMapUnits( composition()->mapSettings().mapUnits() );
    calc.setDpi( 25.4 );
    double originalScale = calc.calculate( mOrigExtent, map->rect().width() );
    double geomCenterX = ( xa1 + xa2 ) / 2.0;
    double geomCenterY = ( ya1 + ya2 ) / 2.0;

    if ( map->atlasScalingMode() == QgsComposerMap::Fixed || isPointLayer )
    {
      // only translate, keep the original scale (i.e. width x height)
      double xMin = geomCenterX - mOrigExtent.width() / 2.0;
      double yMin = geomCenterY - mOrigExtent.height() / 2.0;
      newExtent = QgsRectangle( xMin,
                                yMin,
                                xMin + mOrigExtent.width(),
                                yMin + mOrigExtent.height() );

      //scale newExtent to match original scale of map
      //this is required for geographic coordinate systems, where the scale varies by extent
      double newScale = calc.calculate( newExtent, map->rect().width() );
      newExtent.scale( originalScale / newScale );
    }
    else if ( map->atlasScalingMode() == QgsComposerMap::Predefined )
    {
      // choose one of the predefined scales
      double newWidth = mOrigExtent.width();
      double newHeight = mOrigExtent.height();
      const QVector<qreal>& scales = mPredefinedScales;
      for ( int i = 0; i < scales.size(); i++ )
      {
        double ratio = scales[i] / originalScale;
        newWidth = mOrigExtent.width() * ratio;
        newHeight = mOrigExtent.height() * ratio;

        // compute new extent, centered on feature
        double xMin = geomCenterX - newWidth / 2.0;
        double yMin = geomCenterY - newHeight / 2.0;
        newExtent = QgsRectangle( xMin,
                                  yMin,
                                  xMin + newWidth,
                                  yMin + newHeight );

        //scale newExtent to match desired map scale
        //this is required for geographic coordinate systems, where the scale varies by extent
        double newScale = calc.calculate( newExtent, map->rect().width() );
        newExtent.scale( scales[i] / newScale );

        if (( newExtent.width() >= mTransformedFeatureBounds.width() ) && ( newExtent.height() >= mTransformedFeatureBounds.height() ) )
        {
          // this is the smallest extent that embeds the feature, stop here
          break;
        }
      }
    }
  }
  else if ( map->atlasScalingMode() == QgsComposerMap::Auto )
  {
    // auto scale

    double geomRatio = mTransformedFeatureBounds.width() / mTransformedFeatureBounds.height();
    double mapRatio = mOrigExtent.width() / mOrigExtent.height();

    // geometry height is too big
    if ( geomRatio < mapRatio )
    {
      // extent the bbox's width
      double adjWidth = ( mapRatio * mTransformedFeatureBounds.height() - mTransformedFeatureBounds.width() ) / 2.0;
      xa1 -= adjWidth;
      xa2 += adjWidth;
    }
    // geometry width is too big
    else if ( geomRatio > mapRatio )
    {
      // extent the bbox's height
      double adjHeight = ( mTransformedFeatureBounds.width() / mapRatio - mTransformedFeatureBounds.height() ) / 2.0;
      ya1 -= adjHeight;
      ya2 += adjHeight;
    }
    newExtent = QgsRectangle( xa1, ya1, xa2, ya2 );

    if ( map->atlasMargin() > 0.0 )
    {
      newExtent.scale( 1 + map->atlasMargin() );
    }
  }

  // set the new extent (and render)
  map->setNewAtlasFeatureExtent( newExtent );
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
    atlasElem.setAttribute( "sortKey", mSortKeyAttributeName );
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

  mSingleFile = atlasElem.attribute( "singleFile", "false" ) == "true" ? true : false;
  mFilenamePattern = atlasElem.attribute( "filenamePattern", "" );

  mSortFeatures = atlasElem.attribute( "sortFeatures", "false" ) == "true" ? true : false;
  if ( mSortFeatures )
  {
    mSortKeyAttributeName = atlasElem.attribute( "sortKey", "" );
    // since 2.3, the field name is saved instead of the field index
    // following code keeps compatibility with version 2.2 projects
    // to be removed in QGIS 3.0
    bool isIndex;
    int idx = mSortKeyAttributeName.toInt( &isIndex );
    if ( isIndex && mCoverageLayer )
    {
      const QgsFields fields = mCoverageLayer->pendingFields();
      if ( idx >= 0 && idx < fields.count() )
      {
        mSortKeyAttributeName = fields[idx].name();
      }
    }
    mSortAscending = atlasElem.attribute( "sortAscending", "true" ) == "true" ? true : false;
  }
  mFilterFeatures = atlasElem.attribute( "filterFeatures", "false" ) == "true" ? true : false;
  if ( mFilterFeatures )
  {
    mFeatureFilter = atlasElem.attribute( "featureFilter", "" );
  }

  mHideCoverage = atlasElem.attribute( "hideCoverage", "false" ) == "true" ? true : false;

  emit parameterChanged();
}

void QgsAtlasComposition::readXMLMapSettings( const QDomElement &elem, const QDomDocument &doc )
{
  Q_UNUSED( doc );
  //look for stored composer map, to upgrade pre 2.1 projects
  int composerMapNo = elem.attribute( "composerMap", "-1" ).toInt();
  QgsComposerMap * composerMap = 0;
  if ( composerMapNo != -1 )
  {
    QList<QgsComposerMap*> maps;
    mComposition->composerItems( maps );
    for ( QList<QgsComposerMap*>::iterator it = maps.begin(); it != maps.end(); ++it )
    {
      if (( *it )->id() == composerMapNo )
      {
        composerMap = ( *it );
        composerMap->setAtlasDriven( true );
        break;
      }
    }
  }

  //upgrade pre 2.1 projects
  double margin = elem.attribute( "margin", "0.0" ).toDouble();
  if ( composerMap && margin != 0 )
  {
    composerMap->setAtlasMargin( margin );
  }
  bool fixedScale = elem.attribute( "fixedScale", "false" ) == "true" ? true : false;
  if ( composerMap && fixedScale )
  {
    composerMap->setAtlasScalingMode( QgsComposerMap::Fixed );
  }
}

void QgsAtlasComposition::setHideCoverage( bool hide )
{
  mHideCoverage = hide;

  if ( mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    //an atlas preview is enabled, so reflect changes in coverage layer visibility immediately
    updateAtlasMaps();
    mComposition->update();
  }

}

bool QgsAtlasComposition::setFilenamePattern( const QString& pattern )
{
  mFilenamePattern = pattern;
  return updateFilenameExpression();
}

bool QgsAtlasComposition::updateFilenameExpression()
{
  if ( !mCoverageLayer )
  {
    return false;
  }

  const QgsFields& fields = mCoverageLayer->pendingFields();

  if ( mFilenamePattern.size() > 0 )
  {
    mFilenameExpr.reset( new QgsExpression( mFilenamePattern ) );
    // expression used to evaluate each filename
    // test for evaluation errors
    if ( mFilenameExpr->hasParserError() )
    {
      mFilenameParserError = mFilenameExpr->parserErrorString();
      return false;
    }

    // prepare the filename expression
    mFilenameExpr->prepare( fields );
  }

  //if atlas preview is currently enabled, regenerate filename for current feature
  if ( mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    evalFeatureFilename();
  }
  return true;
}

bool QgsAtlasComposition::evalFeatureFilename()
{
  //generate filename for current atlas feature
  if ( mFilenamePattern.size() > 0 && !mFilenameExpr.isNull() )
  {
    QVariant filenameRes = mFilenameExpr->evaluate( &mCurrentFeature, mCoverageLayer->pendingFields() );
    if ( mFilenameExpr->hasEvalError() )
    {
      QgsMessageLog::logMessage( tr( "Atlas filename evaluation error: %1" ).arg( mFilenameExpr->evalErrorString() ), tr( "Composer" ) );
      return false;
    }

    mCurrentFilename = filenameRes.toString();
  }
  return true;
}

void QgsAtlasComposition::setPredefinedScales( const QVector<qreal>& scales )
{
  mPredefinedScales = scales;
  // make sure the list is sorted
  qSort( mPredefinedScales.begin(), mPredefinedScales.end() );
}

Q_NOWARN_DEPRECATED_PUSH
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

  map->setAtlasScalingMode( fixed ? QgsComposerMap::Fixed : QgsComposerMap::Auto );
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
Q_NOWARN_DEPRECATED_POP
