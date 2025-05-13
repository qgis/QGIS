/***************************************************************************
                         qgsalgorithmexportgeometryattributes.cpp
                         ---------------------
    begin                : February 2025
    copyright            : (C) 2025 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmexportgeometryattributes.h"
#include "qgsunittypes.h"
#include "qgsgeometrycollection.h"
#include "qgscurve.h"

///@cond PRIVATE

QString QgsExportGeometryAttributesAlgorithm::name() const
{
  return QStringLiteral( "exportaddgeometrycolumns" );
}

QString QgsExportGeometryAttributesAlgorithm::displayName() const
{
  return QObject::tr( "Add geometry attributes" );
}

QStringList QgsExportGeometryAttributesAlgorithm::tags() const
{
  return QObject::tr( "export,add,information,measurements,areas,lengths,perimeters,latitudes,longitudes,x,y,z,extract,points,lines,polygons,sinuosity,fields" ).split( ',' );
}

QString QgsExportGeometryAttributesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsExportGeometryAttributesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsExportGeometryAttributesAlgorithm::shortHelpString() const
{
  return QObject::tr( "Computes geometric properties of the features in a vector layer. Algorithm generates a new "
                      "vector layer with the same content as the input one, but with additional attributes in its "
                      "attributes table, containing geometric measurements.\n\n"
                      "Depending on the geometry type of the vector layer, the attributes added to the table will "
                      "be different." );
}

QgsExportGeometryAttributesAlgorithm *QgsExportGeometryAttributesAlgorithm::createInstance() const
{
  return new QgsExportGeometryAttributesAlgorithm();
}

void QgsExportGeometryAttributesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );

  const QStringList options = QStringList()
                              << QObject::tr( "Cartesian Calculations in Layer's CRS" )
                              << QObject::tr( "Cartesian Calculations in Project's CRS" )
                              << QObject::tr( "Ellipsoidal Calculations" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ), QObject::tr( "Calculate using" ), options, false, 0 ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Added geometry info" ) ) );
}

bool QgsExportGeometryAttributesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  Q_UNUSED( parameters );

  mProjectCrs = context.project()->crs();
  return true;
}

QVariantMap QgsExportGeometryAttributesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const int method = parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context );

  const Qgis::WkbType wkbType = source->wkbType();
  QgsFields fields = source->fields();
  QgsFields newFields;

  bool exportZ = false;
  bool exportM = false;
  if ( QgsWkbTypes::geometryType( wkbType ) == Qgis::GeometryType::Polygon )
  {
    newFields.append( QgsField( QStringLiteral( "area" ), QMetaType::Type::Double ) );
    newFields.append( QgsField( QStringLiteral( "perimeter" ), QMetaType::Type::Double ) );
  }
  else if ( QgsWkbTypes::geometryType( wkbType ) == Qgis::GeometryType::Line )
  {
    newFields.append( QgsField( QStringLiteral( "length" ), QMetaType::Type::Double ) );
    if ( !QgsWkbTypes::isMultiType( wkbType ) )
    {
      newFields.append( QgsField( QStringLiteral( "straightdis" ), QMetaType::Type::Double ) );
      newFields.append( QgsField( QStringLiteral( "sinuosity" ), QMetaType::Type::Double ) );
    }
  }
  else
  {
    if ( QgsWkbTypes::isMultiType( wkbType ) )
    {
      newFields.append( QgsField( QStringLiteral( "numparts" ), QMetaType::Type::Int ) );
    }
    else
    {
      newFields.append( QgsField( QStringLiteral( "xcoord" ), QMetaType::Type::Double ) );
      newFields.append( QgsField( QStringLiteral( "ycoord" ), QMetaType::Type::Double ) );
      if ( QgsWkbTypes::hasZ( wkbType ) )
      {
        newFields.append( QgsField( QStringLiteral( "zcoord" ), QMetaType::Type::Double ) );
        exportZ = true;
      }
      if ( QgsWkbTypes::hasM( wkbType ) )
      {
        newFields.append( QgsField( QStringLiteral( "mvalue" ), QMetaType::Type::Double ) );
        exportM = true;
      }
    }
  }

  fields = QgsProcessingUtils::combineFields( fields, newFields );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, wkbType, source->sourceCrs() ) );
  if ( !sink )
  {
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  QgsCoordinateTransform transform;
  mDa = QgsDistanceArea();

  if ( method == 2 )
  {
    mDa.setSourceCrs( source->sourceCrs(), context.transformContext() );
    mDa.setEllipsoid( context.ellipsoid() );
    mDistanceConversionFactor = QgsUnitTypes::fromUnitToUnitFactor( mDa.lengthUnits(), context.distanceUnit() );
    mAreaConversionFactor = QgsUnitTypes::fromUnitToUnitFactor( mDa.areaUnits(), context.areaUnit() );
  }
  else if ( method == 1 )
  {
    if ( !context.project() )
    {
      throw QgsProcessingException( QObject::tr( "No project is available in this context" ) );
    }
    transform = QgsCoordinateTransform( source->sourceCrs(), mProjectCrs, context.transformContext() );
  }

  QgsFeatureIterator it = source->getFeatures();
  const double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 0;
  long i = 0;
  QgsFeature f;

  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsFeature outputFeature( f );
    QgsAttributes attrs = f.attributes();
    QgsGeometry geom = f.geometry();

    if ( !geom.isNull() )
    {
      if ( transform.isValid() )
      {
        try
        {
          geom.transform( transform );
        }
        catch ( QgsCsException &e )
        {
          throw QgsProcessingException( QObject::tr( "Could not transform feature to project's CRS: %1" ).arg( e.what() ) );
        }
      }

      if ( geom.type() == Qgis::GeometryType::Point )
      {
        attrs << pointAttributes( geom, exportZ, exportM );
      }
      else if ( geom.type() == Qgis::GeometryType::Polygon )
      {
        attrs << polygonAttributes( geom );
      }
      else
      {
        attrs << lineAttributes( geom );
      }
    }

    // ensure consistent count of attributes - otherwise null geometry features will have incorrect attribute
    // length and provider may reject them
    while ( attrs.size() < fields.size() )
    {
      attrs.append( QVariant() );
    }

    outputFeature.setAttributes( attrs );
    if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
    {
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }

    i++;
    feedback->setProgress( i * step );
  }

  sink->finalize();

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), dest );
  return results;
}

QgsAttributes QgsExportGeometryAttributesAlgorithm::pointAttributes( const QgsGeometry &geom, const bool exportZ, const bool exportM )
{
  QgsAttributes attrs;

  if ( !geom.isMultipart() )
  {
    auto point = qgsgeometry_cast<const QgsPoint *>( geom.constGet() );
    attrs.append( point->x() );
    attrs.append( point->y() );
    // add point Z/M
    if ( exportZ )
    {
      attrs.append( point->z() );
    }
    if ( exportM )
    {
      attrs.append( point->m() );
    }
  }
  else
  {
    attrs.append( qgsgeometry_cast<const QgsGeometryCollection *>( geom.constGet() )->numGeometries() );
  }
  return attrs;
}

QgsAttributes QgsExportGeometryAttributesAlgorithm::lineAttributes( const QgsGeometry &geom )
{
  QgsAttributes attrs;

  if ( geom.isMultipart() )
  {
    attrs.append( mDistanceConversionFactor * mDa.measureLength( geom ) );
  }
  else
  {
    auto curve = qgsgeometry_cast<const QgsCurve *>( geom.constGet() );
    const QgsPoint p1 = curve->startPoint();
    const QgsPoint p2 = curve->endPoint();
    const double straightDistance = mDistanceConversionFactor * mDa.measureLine( QgsPointXY( p1 ), QgsPointXY( p2 ) );
    const double sinuosity = curve->sinuosity();
    attrs.append( mDistanceConversionFactor * mDa.measureLength( geom ) );
    attrs.append( straightDistance );
    attrs.append( std::isnan( sinuosity ) ? QVariant() : sinuosity );
  }

  return attrs;
}

QgsAttributes QgsExportGeometryAttributesAlgorithm::polygonAttributes( const QgsGeometry &geom )
{
  const double area = mAreaConversionFactor * mDa.measureArea( geom );
  const double perimeter = mDistanceConversionFactor * mDa.measurePerimeter( geom );

  return QgsAttributes() << area << perimeter;
}

///@endcond
