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
#include "qgsmaplayerregistry.h"

QgsAtlasComposition::QgsAtlasComposition( QgsComposition* composition ) :
    mComposition( composition ),
    mEnabled( false ),
    mComposerMap( 0 ),
    mHideCoverage( false ), mFixedScale( false ), mMargin( 0.10 ), mFilenamePattern( "'output_'||$feature" ),
    mCoverageLayer( 0 ), mSingleFile( false )
{

  // declare special columns with a default value
  QgsExpression::setSpecialColumn( "$page", QVariant(( int )1 ) );
  QgsExpression::setSpecialColumn( "$feature", QVariant(( int )0 ) );
  QgsExpression::setSpecialColumn( "$numpages", QVariant(( int )1 ) );
  QgsExpression::setSpecialColumn( "$numfeatures", QVariant(( int )0 ) );
}

QgsAtlasComposition::~QgsAtlasComposition()
{
}

void QgsAtlasComposition::setCoverageLayer( QgsVectorLayer* layer )
{
  mCoverageLayer = layer;

  // update the number of features
  QgsExpression::setSpecialColumn( "$numfeatures", QVariant(( int )mFeatureIds.size() ) );
}

void QgsAtlasComposition::beginRender()
{
  if ( !mComposerMap || !mCoverageLayer )
  {
    return;
  }

  const QgsCoordinateReferenceSystem& coverage_crs = mCoverageLayer->crs();
  const QgsCoordinateReferenceSystem& destination_crs = mComposerMap->mapRenderer()->destinationCrs();
  // transformation needed for feature geometries
  mTransform.setSourceCrs( coverage_crs );
  mTransform.setDestCRS( destination_crs );

  const QgsFields& fields = mCoverageLayer->pendingFields();

  if ( mFilenamePattern.size() > 0 )
  {
    mFilenameExpr = std::auto_ptr<QgsExpression>( new QgsExpression( mFilenamePattern ) );
    // expression used to evaluate each filename
    // test for evaluation errors
    if ( mFilenameExpr->hasParserError() )
    {
      throw std::runtime_error( "Filename parsing error: " + mFilenameExpr->parserErrorString().toStdString() );
    }

    // prepare the filename expression
    mFilenameExpr->prepare( fields );
  }

  // select all features with all attributes
  mCoverageLayer->select( mCoverageLayer->pendingAllAttributesList() );

  // We cannot use nextFeature() directly since the feature pointer is rewinded by the rendering process
  // We thus store the feature ids for future extraction
  QgsFeature feat;
  mFeatureIds.clear();
  while ( mCoverageLayer->nextFeature( feat ) )
  {
    mFeatureIds.push_back( feat.id() );
  }

  mOrigExtent = mComposerMap->extent();

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
}

void QgsAtlasComposition::endRender()
{
  if ( !mComposerMap || !mCoverageLayer )
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
    mComposerMap->cache();
    mComposerMap->update();
  }
  mComposerMap->setNewExtent( mOrigExtent );
}

size_t QgsAtlasComposition::numFeatures() const
{
  return mFeatureIds.size();
}

void QgsAtlasComposition::prepareForFeature( size_t featureI )
{
  if ( !mComposerMap || !mCoverageLayer )
  {
    return;
  }

  // retrieve the next feature, based on its id
  mCoverageLayer->featureAtId( mFeatureIds[ featureI ], mCurrentFeature, /* fetchGeometry = */ true, /* fetchAttributes = */ true );

  if ( mFilenamePattern.size() > 0 )
  {
    QgsExpression::setSpecialColumn( "$feature", QVariant(( int )featureI + 1 ) );
    QVariant filenameRes = mFilenameExpr->evaluate( &mCurrentFeature );
    if ( mFilenameExpr->hasEvalError() )
    {
      throw std::runtime_error( "Filename eval error: " + mFilenameExpr->evalErrorString().toStdString() );
    }

    mCurrentFilename = filenameRes.toString();
  }

  //
  // compute the new extent
  // keep the original aspect ratio
  // and apply a margin

  // QgsGeometry::boundingBox is expressed in the geometry"s native CRS
  // We have to transform the grometry to the destination CRS and ask for the bounding box
  // Note: we cannot directly take the transformation of the bounding box, since transformations are not linear

  QgsGeometry tgeom( *mCurrentFeature.geometry() );
  tgeom.transform( mTransform );
  QgsRectangle geom_rect = tgeom.boundingBox();

  double xa1 = geom_rect.xMinimum();
  double xa2 = geom_rect.xMaximum();
  double ya1 = geom_rect.yMinimum();
  double ya2 = geom_rect.yMaximum();
  QgsRectangle new_extent = geom_rect;

  // restore the original extent
  // (successive calls to setNewExtent tend to deform the original rectangle)
  mComposerMap->setNewExtent( mOrigExtent );

  if ( mFixedScale )
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

    double geom_ratio = geom_rect.width() / geom_rect.height();
    double map_ratio = mOrigExtent.width() / mOrigExtent.height();

    // geometry height is too big
    if ( geom_ratio < map_ratio )
    {
      // extent the bbox's width
      double adj_width = ( map_ratio * geom_rect.height() - geom_rect.width() ) / 2.0;
      xa1 -= adj_width;
      xa2 += adj_width;
    }
    // geometry width is too big
    else if ( geom_ratio > map_ratio )
    {
      // extent the bbox's height
      double adj_height = ( geom_rect.width() / map_ratio - geom_rect.height() ) / 2.0;
      ya1 -= adj_height;
      ya2 += adj_height;
    }
    new_extent = QgsRectangle( xa1, ya1, xa2, ya2 );

    if ( mMargin > 0.0 )
    {
      new_extent.scale( 1 + mMargin );
    }
  }

  // evaluate label expressions
  QList<QgsComposerLabel*> labels;
  mComposition->composerItems( labels );
  QgsExpression::setSpecialColumn( "$feature", QVariant(( int )featureI + 1 ) );

  for ( QList<QgsComposerLabel*>::iterator lit = labels.begin(); lit != labels.end(); ++lit )
  {
    ( *lit )->setExpressionContext( &mCurrentFeature, mCoverageLayer );
  }

  // set the new extent (and render)
  mComposerMap->setNewExtent( new_extent );
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
  if ( mComposerMap )
  {
    atlasElem.setAttribute( "composerMap", mComposerMap->id() );
  }
  else
  {
    atlasElem.setAttribute( "composerMap", "" );
  }
  atlasElem.setAttribute( "hideCoverage", mHideCoverage ? "true" : "false" );
  atlasElem.setAttribute( "fixedScale", mFixedScale ? "true" : "false" );
  atlasElem.setAttribute( "singleFile", mSingleFile ? "true" : "false" );
  atlasElem.setAttribute( "margin", QString::number( mMargin ) );
  atlasElem.setAttribute( "filenamePattern", mFilenamePattern );

  elem.appendChild( atlasElem );
}

void QgsAtlasComposition::readXML( const QDomElement& atlasElem, const QDomDocument& )
{
  mEnabled = atlasElem.attribute( "enabled", "false" ) == "true" ? true : false;
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
  // look for stored composer map
  mComposerMap = 0;
  QList<const QgsComposerMap*> maps = mComposition->composerMapItems();
  for ( QList<const QgsComposerMap*>::const_iterator it = maps.begin(); it != maps.end(); ++it )
  {
    if (( *it )->id() == atlasElem.attribute( "composerMap" ).toInt() )
    {
      mComposerMap = const_cast<QgsComposerMap*>( *it );
      break;
    }
  }
  mMargin = atlasElem.attribute( "margin", "0.0" ).toDouble();
  mHideCoverage = atlasElem.attribute( "hideCoverage", "false" ) == "true" ? true : false;
  mFixedScale = atlasElem.attribute( "fixedScale", "false" ) == "true" ? true : false;
  mSingleFile = atlasElem.attribute( "singleFile", "false" ) == "true" ? true : false;
  mFilenamePattern = atlasElem.attribute( "filenamePattern", "" );

  std::cout << "emit parameter changed this = " << this << std::endl;
  emit parameterChanged();
}
