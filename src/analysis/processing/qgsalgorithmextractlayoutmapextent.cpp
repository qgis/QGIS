/***************************************************************************
                         qgsalgorithmextractlayoutmapextent.cpp
                         ---------------------
    begin                : March 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsalgorithmextractlayoutmapextent.h"

#include "layout/qgslayout.h"
#include "layout/qgslayoutitemmap.h"
#include "layout/qgslayoutitemregistry.h"
#include "layout/qgsprintlayout.h"
#include "qgsprocessingoutputs.h"

///@cond PRIVATE

QString QgsLayoutMapExtentToLayerAlgorithm::name() const
{
  return u"printlayoutmapextenttolayer"_s;
}

QString QgsLayoutMapExtentToLayerAlgorithm::displayName() const
{
  return QObject::tr( "Print layout map extent to layer" );
}

QStringList QgsLayoutMapExtentToLayerAlgorithm::tags() const
{
  return QObject::tr( "layout,composer,composition,visible" ).split( ',' );
}

QString QgsLayoutMapExtentToLayerAlgorithm::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsLayoutMapExtentToLayerAlgorithm::groupId() const
{
  return u"cartography"_s;
}

QString QgsLayoutMapExtentToLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a polygon layer containing the extent of a print layout map item." );
}

void QgsLayoutMapExtentToLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterLayout( u"LAYOUT"_s, QObject::tr( "Print layout" ) ) );
  addParameter( new QgsProcessingParameterLayoutItem( u"MAP"_s, QObject::tr( "Map item" ), QVariant(), u"LAYOUT"_s, QgsLayoutItemRegistry::LayoutMap, true ) );
  auto crsParam = std::make_unique<QgsProcessingParameterCrs>( u"CRS"_s, QObject::tr( "Override CRS" ), QVariant(), true );
  crsParam->setFlags( crsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( crsParam.release() );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Extent" ), Qgis::ProcessingSourceType::VectorPolygon ) );
  addOutput( new QgsProcessingOutputNumber( u"WIDTH"_s, QObject::tr( "Map width" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"HEIGHT"_s, QObject::tr( "Map height" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"SCALE"_s, QObject::tr( "Map scale" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"ROTATION"_s, QObject::tr( "Map rotation" ) ) );
}

Qgis::ProcessingAlgorithmFlags QgsLayoutMapExtentToLayerAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

QString QgsLayoutMapExtentToLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a polygon layer containing the extent of a print layout map item (or items), "
                      "with attributes specifying the map size (in layout units), scale and rotation.\n\n"
                      "If the map item parameter is specified, then only the matching map extent will be exported. If it "
                      "is not specified, all map extents from the layout will be exported.\n\n"
                      "Optionally, a specific output CRS can be specified. If it is not specified, the original map "
                      "item CRS will be used." );
}

QgsLayoutMapExtentToLayerAlgorithm *QgsLayoutMapExtentToLayerAlgorithm::createInstance() const
{
  return new QgsLayoutMapExtentToLayerAlgorithm();
}

bool QgsLayoutMapExtentToLayerAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  // this needs to be done in main thread, layouts are not thread safe
  QgsPrintLayout *layout = parameterAsLayout( parameters, u"LAYOUT"_s, context );
  if ( !layout )
    throw QgsProcessingException( QObject::tr( "Cannot find layout with name \"%1\"" ).arg( parameters.value( u"LAYOUT"_s ).toString() ) );

  QgsLayoutItemMap *map = qobject_cast<QgsLayoutItemMap *>( parameterAsLayoutItem( parameters, u"MAP"_s, context, layout ) );
  if ( !map && parameters.value( u"MAP"_s ).isValid() )
    throw QgsProcessingException( QObject::tr( "Cannot find matching map item with ID %1" ).arg( parameters.value( u"MAP"_s ).toString() ) );

  QList<QgsLayoutItemMap *> maps;
  if ( map )
    maps << map;
  else
    layout->layoutItems( maps );

  const QgsCoordinateReferenceSystem overrideCrs = parameterAsCrs( parameters, u"CRS"_s, context );

  mFeatures.reserve( maps.size() );
  for ( QgsLayoutItemMap *map : maps )
  {
    if ( !mCrs.isValid() )
      mCrs = !overrideCrs.isValid() ? map->crs() : overrideCrs;

    QgsGeometry extent = QgsGeometry::fromQPolygonF( map->visibleExtentPolygon() );
    if ( map->crs() != mCrs )
    {
      try
      {
        extent.transform( QgsCoordinateTransform( map->crs(), mCrs, context.transformContext() ) );
      }
      catch ( QgsCsException & )
      {
        feedback->reportError( QObject::tr( "Error reprojecting map to destination CRS" ) );
        continue;
      }
    }

    mWidth = map->rect().width();
    mHeight = map->rect().height();
    mScale = map->scale();
    mRotation = map->mapRotation();

    QgsFeature f;
    f.setAttributes( QgsAttributes() << map->displayName() << mWidth << mHeight << mScale << mRotation );
    f.setGeometry( extent );

    mFeatures << f;
  }
  return true;
}

QVariantMap QgsLayoutMapExtentToLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFields fields;
  fields.append( QgsField( u"map"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"width"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"height"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"scale"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"rotation"_s, QMetaType::Type::Double ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::Polygon, mCrs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  for ( QgsFeature &f : mFeatures )
  {
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
  }

  sink->finalize();

  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  // these numeric outputs only have value for single-map layouts
  outputs.insert( u"WIDTH"_s, mFeatures.size() == 1 ? mWidth : QVariant() );
  outputs.insert( u"HEIGHT"_s, mFeatures.size() == 1 ? mHeight : QVariant() );
  outputs.insert( u"SCALE"_s, mFeatures.size() == 1 ? mScale : QVariant() );
  outputs.insert( u"ROTATION"_s, mFeatures.size() == 1 ? mRotation : QVariant() );
  return outputs;
}

///@endcond
