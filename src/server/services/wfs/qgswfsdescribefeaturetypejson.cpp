/***************************************************************************
                              qgswfsdescribefeaturetypegeojson.cpp
                              ------------------------------------
  begin                : December 09 , 2022
  copyright            : (C) 2022 by David Marteau
  email                : david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswfsutils.h"
#include "qgsserverprojectutils.h"
#include "qgswfsdescribefeaturetypejson.h"
#include "qgswfsdescribefeaturetype.h"
#include "qgswfsparameters.h"
#include "qgsjsonutils.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsdatetimefieldformatter.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

using namespace QgsWfs;

QgsWfsDescribeFeatureTypeJson::QgsWfsDescribeFeatureTypeJson( const QgsWfsParameters wfsParams )
  : wfsParameters( wfsParams )
{}

void QgsWfsDescribeFeatureTypeJson::writeDescribeFeatureType( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request, QgsServerResponse &response ) const
{
  const QJsonDocument doc( createDescribeFeatureTypeDocument( serverIface, project, version, request ) );

  response.setHeader( "Content-Type", "application/vnd.geo+json; charset=utf-8" );
  response.write( doc.toJson( QJsonDocument::Compact ) );
}


QJsonObject QgsWfsDescribeFeatureTypeJson::createDescribeFeatureTypeDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request ) const
{
  Q_UNUSED( version )


#ifdef HAVE_SERVER_PYTHON_PLUGINS
  QgsAccessControl *accessControl = serverIface->accessControls();
#else
  ( void ) serverIface;
#endif

  QJsonObject json;
  json[QStringLiteral( "elementFormDefault" )] = QStringLiteral( "qualified" );
  json[QStringLiteral( "targetNamespace" )] = QGS_NAMESPACE;
  json[QStringLiteral( "targetPrefix" )] = QStringLiteral( "qgs" );

  QJsonArray featureTypes;

  QStringList typeNameList = getRequestTypeNames( request, wfsParameters );

  const QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
  for ( int i = 0; i < wfsLayerIds.size(); ++i )
  {
    QgsMapLayer *layer = project->mapLayer( wfsLayerIds.at( i ) );
    if ( !layer )
    {
      continue;
    }

    const QString name = layerTypeName( layer );

    if ( !typeNameList.isEmpty() && !typeNameList.contains( name ) )
    {
      continue;
    }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    if ( accessControl && !accessControl->layerReadPermission( layer ) )
    {
      if ( !typeNameList.isEmpty() )
      {
        throw QgsSecurityAccessException( QStringLiteral( "Feature access permission denied" ) );
      }
      else
      {
        continue;
      }
    }
#endif
    QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );
    QgsVectorDataProvider *provider = vLayer->dataProvider();
    if ( !provider )
    {
      continue;
    }

    featureTypes.append( schemaLayerToJson( const_cast<QgsVectorLayer *>( vLayer ) ) );
  }

  json[QStringLiteral( "featureTypes" )] = featureTypes;
  return json;
}

QJsonObject QgsWfsDescribeFeatureTypeJson::schemaLayerToJson( const QgsVectorLayer *layer ) const
{
  const QString typeName = layerTypeName( layer );

  QJsonObject json;
  QJsonArray properties;

  json[QStringLiteral( "typeName" )] = typeName;

  if ( layer->isSpatial() )
  {
    QString geomType, geomLocalType;
    getGeometryType( layer, geomType, geomLocalType );

    QJsonObject property;
    property[QStringLiteral( "name" )] = QStringLiteral( "geometry" );
    property[QStringLiteral( "minOccurs" )] = QStringLiteral( "0" );
    property[QStringLiteral( "maxOccurs" )] = QStringLiteral( "1" );
    property[QStringLiteral( "type" )] = geomType;


    if ( !geomLocalType.isEmpty() )
    {
      property[QStringLiteral( "localType" )] = geomLocalType;
    }
    properties.append( property );
  }

  //Attributes
  const QgsFields fields = layer->fields();
  //hidden attributes for this layer
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    const QgsField field = fields.at( idx );
    //skip attribute if excluded from WFS publication
    if ( field.configurationFlags().testFlag( Qgis::FieldConfigurationFlag::HideFromWfs ) )
    {
      continue;
    }

    QString attributeName, attributeType;

    // Defined in qgswfsdescribefeaturetype.h
    getFieldAttributes( field, attributeName, attributeType );

    QJsonObject property;
    property[QStringLiteral( "name" )] = attributeName;
    property[QStringLiteral( "type" )] = attributeType;
    property[QStringLiteral( "localType" )] = attributeType;

    if ( !( field.constraints().constraints() & QgsFieldConstraints::Constraint::ConstraintNotNull ) )
    {
      property[QStringLiteral( "nillable" )] = "true";
    }

    const QString alias = field.alias();
    if ( !alias.isEmpty() )
    {
      property[QStringLiteral( "alias" )] = alias;
    }

    properties.append( property );
  }

  json[QStringLiteral( "properties" )] = properties;
  return json;
}

void QgsWfsDescribeFeatureTypeJson::getGeometryType( const QgsVectorLayer *layer, QString &geomType, QString &geomLocalType ) const
{
  const Qgis::WkbType wkbType = layer->wkbType();
  switch ( wkbType )
  {
    case Qgis::WkbType::Point25D:
    case Qgis::WkbType::Point:
      geomType = QStringLiteral( "gml:PointPropertyType" );
      geomLocalType = QStringLiteral( "Point" );
      break;

    case Qgis::WkbType::LineString25D:
    case Qgis::WkbType::LineString:
      geomType = QStringLiteral( "gml:LineStringPropertyType" );
      geomLocalType = QStringLiteral( "LineString" );
      break;

    case Qgis::WkbType::Polygon25D:
    case Qgis::WkbType::Polygon:
      geomType = QStringLiteral( "gml:PolygonPropertyType" );
      geomLocalType = QStringLiteral( "Polygon" );
      break;

    case Qgis::WkbType::MultiPoint25D:
    case Qgis::WkbType::MultiPoint:
      geomType = QStringLiteral( "gml:MultiPointPropertyType" );
      geomLocalType = QStringLiteral( "MultiPoint" );
      break;

    case Qgis::WkbType::MultiCurve:
    case Qgis::WkbType::MultiLineString25D:
    case Qgis::WkbType::MultiLineString:
      geomType = QStringLiteral( "gml:MultiCurvePropertyType" );
      geomLocalType = QStringLiteral( "MultiCurve" );
      break;

    case Qgis::WkbType::MultiSurface:
    case Qgis::WkbType::MultiPolygon25D:
    case Qgis::WkbType::MultiPolygon:
      geomType = QStringLiteral( "gml:MultiSurfacePropertyType" );
      geomLocalType = QStringLiteral( "MultiSurface" );
      break;

    default:
      geomType = QStringLiteral( "gml:GeometryPropertyType" );
  }
}
