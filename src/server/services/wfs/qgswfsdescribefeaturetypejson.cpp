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
#include "qgswfsdescribefeaturetypejson.h"

#include "qgsdatetimefieldformatter.h"
#include "qgsjsonutils.h"
#include "qgsproject.h"
#include "qgsserverprojectutils.h"
#include "qgsvectorlayer.h"
#include "qgswfsdescribefeaturetype.h"
#include "qgswfsparameters.h"
#include "qgswfsutils.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

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
  json[u"elementFormDefault"_s] = u"qualified"_s;
  json[u"targetNamespace"_s] = QGS_NAMESPACE;
  json[u"targetPrefix"_s] = u"qgs"_s;

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

    const QString name = layer->serverProperties()->wfsTypeName();

    if ( !typeNameList.isEmpty() && !typeNameList.contains( name ) )
    {
      continue;
    }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    if ( accessControl && !accessControl->layerReadPermission( layer ) )
    {
      if ( !typeNameList.isEmpty() )
      {
        throw QgsSecurityAccessException( u"Feature access permission denied"_s );
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

  json[u"featureTypes"_s] = featureTypes;
  return json;
}

QJsonObject QgsWfsDescribeFeatureTypeJson::schemaLayerToJson( const QgsVectorLayer *layer ) const
{
  const QString typeName = layer->serverProperties()->wfsTypeName();

  QJsonObject json;
  QJsonArray properties;

  json[u"typeName"_s] = typeName;

  if ( layer->isSpatial() )
  {
    QString geomType, geomLocalType;
    getGeometryType( layer, geomType, geomLocalType );

    QJsonObject property;
    property[u"name"_s] = u"geometry"_s;
    property[u"minOccurs"_s] = u"0"_s;
    property[u"maxOccurs"_s] = u"1"_s;
    property[u"type"_s] = geomType;


    if ( !geomLocalType.isEmpty() )
    {
      property[u"localType"_s] = geomLocalType;
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
    property[u"name"_s] = attributeName;
    property[u"type"_s] = attributeType;
    property[u"localType"_s] = attributeType;

    if ( !( field.constraints().constraints() & QgsFieldConstraints::Constraint::ConstraintNotNull ) )
    {
      property[u"nillable"_s] = "true";
    }

    const QString alias = field.alias();
    if ( !alias.isEmpty() )
    {
      property[u"alias"_s] = alias;
    }

    properties.append( property );
  }

  json[u"properties"_s] = properties;
  return json;
}

void QgsWfsDescribeFeatureTypeJson::getGeometryType( const QgsVectorLayer *layer, QString &geomType, QString &geomLocalType ) const
{
  const Qgis::WkbType wkbType = layer->wkbType();
  switch ( wkbType )
  {
    case Qgis::WkbType::Point25D:
    case Qgis::WkbType::Point:
      geomType = u"gml:PointPropertyType"_s;
      geomLocalType = u"Point"_s;
      break;

    case Qgis::WkbType::LineString25D:
    case Qgis::WkbType::LineString:
      geomType = u"gml:LineStringPropertyType"_s;
      geomLocalType = u"LineString"_s;
      break;

    case Qgis::WkbType::Polygon25D:
    case Qgis::WkbType::Polygon:
      geomType = u"gml:PolygonPropertyType"_s;
      geomLocalType = u"Polygon"_s;
      break;

    case Qgis::WkbType::MultiPoint25D:
    case Qgis::WkbType::MultiPoint:
      geomType = u"gml:MultiPointPropertyType"_s;
      geomLocalType = u"MultiPoint"_s;
      break;

    case Qgis::WkbType::MultiCurve:
    case Qgis::WkbType::MultiLineString25D:
    case Qgis::WkbType::MultiLineString:
      geomType = u"gml:MultiCurvePropertyType"_s;
      geomLocalType = u"MultiCurve"_s;
      break;

    case Qgis::WkbType::MultiSurface:
    case Qgis::WkbType::MultiPolygon25D:
    case Qgis::WkbType::MultiPolygon:
      geomType = u"gml:MultiSurfacePropertyType"_s;
      geomLocalType = u"MultiSurface"_s;
      break;

    default:
      geomType = u"gml:GeometryPropertyType"_s;
  }
}
