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
#include "layout/qgslayoutitemregistry.h"
#include "layout/qgslayoutitemmap.h"
#include "layout/qgslayout.h"
#include "layout/qgsprintlayout.h"
#include "qgsprocessingoutputs.h"

///@cond PRIVATE

QString QgsLayoutMapExtentToLayerAlgorithm::name() const
{
  return QStringLiteral( "printlayoutmapextenttolayer" );
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
  return QStringLiteral( "cartography" );
}

QString QgsLayoutMapExtentToLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a polygon layer containing the extent of a print layout map item." );
}

void QgsLayoutMapExtentToLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterLayout( QStringLiteral( "LAYOUT" ), QObject::tr( "Print layout" ) ) );
  addParameter( new QgsProcessingParameterLayoutItem( QStringLiteral( "MAP" ), QObject::tr( "Map item" ), QVariant(), QStringLiteral( "LAYOUT" ), QgsLayoutItemRegistry::LayoutMap, true ) );
  auto crsParam = std::make_unique< QgsProcessingParameterCrs >( QStringLiteral( "CRS" ), QObject::tr( "Override CRS" ), QVariant(), true );
  crsParam->setFlags( crsParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( crsParam.release() );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extent" ), QgsProcessing::TypeVectorPolygon ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "WIDTH" ), QObject::tr( "Map width" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "HEIGHT" ), QObject::tr( "Map height" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "SCALE" ), QObject::tr( "Map scale" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "ROTATION" ), QObject::tr( "Map rotation" ) ) );
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
  QgsPrintLayout *layout = parameterAsLayout( parameters, QStringLiteral( "LAYOUT" ), context );
  if ( !layout )
    throw QgsProcessingException( QObject::tr( "Cannot find layout with name \"%1\"" ).arg( parameters.value( QStringLiteral( "LAYOUT" ) ).toString() ) );

  QgsLayoutItemMap *map = qobject_cast< QgsLayoutItemMap * >( parameterAsLayoutItem( parameters, QStringLiteral( "MAP" ), context, layout ) );
  if ( !map && parameters.value( QStringLiteral( "MAP" ) ).isValid() )
    throw QgsProcessingException( QObject::tr( "Cannot find matching map item with ID %1" ).arg( parameters.value( QStringLiteral( "MAP" ) ).toString() ) );

  QList< QgsLayoutItemMap *> maps;
  if ( map )
    maps << map;
  else
    layout->layoutItems( maps );

  const QgsCoordinateReferenceSystem overrideCrs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );

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
  fields.append( QgsField( QStringLiteral( "map" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "width" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "height" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "scale" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "rotation" ), QVariant::Double ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::Polygon, mCrs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  for ( QgsFeature &f : mFeatures )
  {
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  // these numeric outputs only have value for single-map layouts
  outputs.insert( QStringLiteral( "WIDTH" ), mFeatures.size() == 1 ? mWidth : QVariant() );
  outputs.insert( QStringLiteral( "HEIGHT" ), mFeatures.size() == 1 ? mHeight : QVariant() );
  outputs.insert( QStringLiteral( "SCALE" ), mFeatures.size() == 1 ? mScale : QVariant() );
  outputs.insert( QStringLiteral( "ROTATION" ), mFeatures.size() == 1 ? mRotation : QVariant() );
  return outputs;
}

///@endcond

