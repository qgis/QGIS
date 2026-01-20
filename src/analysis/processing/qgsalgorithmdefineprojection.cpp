/***************************************************************************
                         qgsalgorithmdefineprojection.cpp
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

#include "qgsalgorithmdefineprojection.h"

#include "qgsproviderregistry.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsDefineProjectionAlgorithm::name() const
{
  return u"definecurrentprojection"_s;
}

QString QgsDefineProjectionAlgorithm::displayName() const
{
  return QObject::tr( "Define projection" );
}

QStringList QgsDefineProjectionAlgorithm::tags() const
{
  return QObject::tr( "layer,shp,prj,qpj,change,alter" ).split( ',' );
}

QString QgsDefineProjectionAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsDefineProjectionAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

QString QgsDefineProjectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm sets an existing layer's projection to the provided CRS without reprojecting features. "
                      "Contrary to the \"Assign projection\" algorithm, it will not output a new layer.\n\n"
                      "If the input layer is a shapefile, the .prj file will be overwritten — or created if "
                      "missing — to match the provided CRS." );
}

QString QgsDefineProjectionAlgorithm::shortDescription() const
{
  return QObject::tr( "Sets an existing layer's projection to the provided CRS without reprojecting features." );
}

QgsDefineProjectionAlgorithm *QgsDefineProjectionAlgorithm::createInstance() const
{
  return new QgsDefineProjectionAlgorithm();
}

void QgsDefineProjectionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( u"INPUT"_s, QObject::tr( "Input shapefile" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );
  addParameter( new QgsProcessingParameterCrs( u"CRS"_s, QObject::tr( "CRS" ), QgsCoordinateReferenceSystem( "EPSG:4326" ) ) );
  addOutput( new QgsProcessingOutputVectorLayer( u"OUTPUT"_s, QObject::tr( "Layer with projection" ) ) );
}

bool QgsDefineProjectionAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, u"INPUT"_s, context );
  const QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, u"CRS"_s, context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  mLayerId = layer->id();

  if ( layer->providerType().compare( u"ogr"_s, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
  {
    const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->dataProvider()->dataSourceUri() );
    QString layerPath;
    if ( parts.size() > 0 )
    {
      layerPath = parts.value( u"path"_s ).toString();
    }

    if ( !layerPath.isEmpty() && layerPath.endsWith( u".shp"_s, Qt::CaseSensitivity::CaseInsensitive ) )
    {
      const QString filePath = layerPath.chopped( 4 );
      const QString wkt = crs.toWkt( Qgis::CrsWktVariant::Wkt1Esri );

      QFile prjFile( filePath + ".prj"_L1 );
      if ( prjFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
      {
        QTextStream stream( &prjFile );
        stream << wkt << Qt::endl;
      }
      else
      {
        feedback->pushWarning( QObject::tr( "Failed to open .prj file for writing." ) );
      }

      QFile qpjFile( filePath + ".qpj"_L1 );
      if ( qpjFile.exists() )
      {
        qpjFile.remove();
      }
    }
    else
    {
      feedback->pushWarning( QObject::tr( "Data source isn't a shapefile, skipping .prj creation" ) );
    }
  }
  else
  {
    feedback->pushInfo( QObject::tr( "Data source isn't a shapefile, skipping .prj creation" ) );
  }

  layer->setCrs( crs );
  layer->triggerRepaint();

  return true;
}

QVariantMap QgsDefineProjectionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );
  Q_UNUSED( feedback );

  QVariantMap results;
  results.insert( u"OUTPUT"_s, mLayerId );
  return results;
}

///@endcond
