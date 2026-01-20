/***************************************************************************
                              qgswfsdescribefeaturetypegml.cpp
                              --------------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2012 by René-Luc D'Hont    (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2017 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswfsdescribefeaturetypegml.h"

#include "qgsproject.h"
#include "qgsserverprojectutils.h"
#include "qgsvectorlayer.h"
#include "qgswfsdescribefeaturetype.h"
#include "qgswfsparameters.h"
#include "qgswfsutils.h"

using namespace QgsWfs;

QgsWfsDescribeFeatureTypeGml::QgsWfsDescribeFeatureTypeGml( const QgsWfsParameters wfsParams )
  : wfsParameters( wfsParams )
{}

void QgsWfsDescribeFeatureTypeGml::writeDescribeFeatureType( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request, QgsServerResponse &response ) const
{
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  QgsAccessControl *accessControl = serverIface->accessControls();
#endif
  QDomDocument doc;
  const QDomDocument *describeDocument = nullptr;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  QgsServerCacheManager *cacheManager = serverIface->cacheManager();
  if ( cacheManager && cacheManager->getCachedDocument( &doc, project, request, accessControl ) )
  {
    describeDocument = &doc;
  }
  else //describe feature xml not in cache. Create a new one
  {
    doc = createDescribeFeatureTypeDocument( serverIface, project, version, request );

    if ( cacheManager )
    {
      cacheManager->setCachedDocument( &doc, project, request, accessControl );
    }
    describeDocument = &doc;
  }
#else
  doc = createDescribeFeatureTypeDocument( serverIface, project, version, request );
  describeDocument = &doc;
#endif
  response.setHeader( "Content-Type", "text/xml; charset=utf-8" );
  response.write( describeDocument->toByteArray() );
}


QDomDocument QgsWfsDescribeFeatureTypeGml::createDescribeFeatureTypeDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request ) const
{
  Q_UNUSED( version )

  QDomDocument doc;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  QgsAccessControl *accessControl = serverIface->accessControls();
#else
  ( void ) serverIface;
#endif

  auto outputFormat = wfsParameters.outputFormat();

  //xsd:schema
  QDomElement schemaElement = doc.createElement( u"schema"_s /*xsd:schema*/ );
  schemaElement.setAttribute( u"xmlns"_s, u"http://www.w3.org/2001/XMLSchema"_s );
  schemaElement.setAttribute( u"xmlns:xsd"_s, u"http://www.w3.org/2001/XMLSchema"_s );
  schemaElement.setAttribute( u"xmlns:ogc"_s, OGC_NAMESPACE );
  schemaElement.setAttribute( u"xmlns:gml"_s, GML_NAMESPACE );
  schemaElement.setAttribute( u"xmlns:qgs"_s, QGS_NAMESPACE );
  schemaElement.setAttribute( u"targetNamespace"_s, QGS_NAMESPACE );
  schemaElement.setAttribute( u"elementFormDefault"_s, u"qualified"_s );
  schemaElement.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( schemaElement );

  //xsd:import
  QDomElement importElement = doc.createElement( u"import"_s /*xsd:import*/ );
  importElement.setAttribute( u"namespace"_s, GML_NAMESPACE );
  if ( outputFormat == QgsWfsParameters::Format::GML2 )
    importElement.setAttribute( u"schemaLocation"_s, u"http://schemas.opengis.net/gml/2.1.2/feature.xsd"_s );
  else if ( outputFormat == QgsWfsParameters::Format::GML3 )
    importElement.setAttribute( u"schemaLocation"_s, u"http://schemas.opengis.net/gml/3.1.1/base/gml.xsd"_s );
  schemaElement.appendChild( importElement );

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
    setSchemaLayer( schemaElement, doc, const_cast<QgsVectorLayer *>( vLayer ) );
  }
  return doc;
}

void QgsWfsDescribeFeatureTypeGml::setSchemaLayer( QDomElement &parentElement, QDomDocument &doc, const QgsVectorLayer *layer ) const
{
  const QgsVectorDataProvider *provider = layer->dataProvider();
  if ( !provider )
  {
    return;
  }

  const QString typeName = layer->serverProperties()->wfsTypeName();

  //xsd:element
  QDomElement elementElem = doc.createElement( u"element"_s /*xsd:element*/ );
  elementElem.setAttribute( u"name"_s, typeName );
  elementElem.setAttribute( u"type"_s, "qgs:" + typeName + "Type" );
  elementElem.setAttribute( u"substitutionGroup"_s, u"gml:_Feature"_s );
  parentElement.appendChild( elementElem );

  //xsd:complexType
  QDomElement complexTypeElem = doc.createElement( u"complexType"_s /*xsd:complexType*/ );
  complexTypeElem.setAttribute( u"name"_s, typeName + "Type" );
  parentElement.appendChild( complexTypeElem );

  //xsd:complexType
  QDomElement complexContentElem = doc.createElement( u"complexContent"_s /*xsd:complexContent*/ );
  complexTypeElem.appendChild( complexContentElem );

  //xsd:extension
  QDomElement extensionElem = doc.createElement( u"extension"_s /*xsd:extension*/ );
  extensionElem.setAttribute( u"base"_s, u"gml:AbstractFeatureType"_s );
  complexContentElem.appendChild( extensionElem );

  //xsd:sequence
  QDomElement sequenceElem = doc.createElement( u"sequence"_s /*xsd:sequence*/ );
  extensionElem.appendChild( sequenceElem );

  //xsd:element
  if ( layer->isSpatial() )
  {
    QDomElement geomElem = doc.createElement( u"element"_s /*xsd:element*/ );
    geomElem.setAttribute( u"name"_s, u"geometry"_s );
    geomElem.setAttribute( u"type"_s, getGmlGeometryType( layer ) );
    geomElem.setAttribute( u"minOccurs"_s, u"0"_s );
    geomElem.setAttribute( u"maxOccurs"_s, u"1"_s );
    sequenceElem.appendChild( geomElem );
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

    //xsd:element
    QDomElement attElem = doc.createElement( u"element"_s /*xsd:element*/ );

    attElem.setAttribute( u"name"_s, attributeName );
    attElem.setAttribute( u"type"_s, attributeType );

    if ( !( field.constraints().constraints() & QgsFieldConstraints::Constraint::ConstraintNotNull ) )
    {
      attElem.setAttribute( u"nillable"_s, u"true"_s );
    }

    sequenceElem.appendChild( attElem );

    const QString alias = field.alias();
    if ( !alias.isEmpty() )
    {
      attElem.setAttribute( u"alias"_s, alias );
    }
  }
}

QString QgsWfsDescribeFeatureTypeGml::getGmlGeometryType( const QgsVectorLayer *layer ) const
{
  const Qgis::WkbType wkbType = layer->wkbType();
  switch ( wfsParameters.outputFormat() )
  {
    case QgsWfsParameters::Format::GML2:
      switch ( wkbType )
      {
        case Qgis::WkbType::PointZ:
        case Qgis::WkbType::Point25D:
        case Qgis::WkbType::Point:
          return u"gml:PointPropertyType"_s;

        case Qgis::WkbType::LineStringZ:
        case Qgis::WkbType::LineString25D:
        case Qgis::WkbType::LineString:
          return u"gml:LineStringPropertyType"_s;

        case Qgis::WkbType::PolygonZ:
        case Qgis::WkbType::Polygon25D:
        case Qgis::WkbType::Polygon:
          return u"gml:PolygonPropertyType"_s;

        case Qgis::WkbType::MultiPointZ:
        case Qgis::WkbType::MultiPoint25D:
        case Qgis::WkbType::MultiPoint:
          return u"gml:MultiPointPropertyType"_s;

        case Qgis::WkbType::MultiCurveZ:
        case Qgis::WkbType::MultiCurve:
        case Qgis::WkbType::MultiLineString25D:
        case Qgis::WkbType::MultiLineStringZ:
        case Qgis::WkbType::MultiLineString:
          return u"gml:MultiLineStringPropertyType"_s;

        case Qgis::WkbType::MultiSurfaceZ:
        case Qgis::WkbType::MultiSurface:
        case Qgis::WkbType::MultiPolygon25D:
        case Qgis::WkbType::MultiPolygon:
        case Qgis::WkbType::MultiPolygonZ:
          return u"gml:MultiPolygonPropertyType"_s;

        default:
          return u"gml:GeometryPropertyType"_s;
      }
    case QgsWfsParameters::Format::GML3:
      switch ( wkbType )
      {
        case Qgis::WkbType::PointZ:
        case Qgis::WkbType::Point25D:
        case Qgis::WkbType::Point:
          return u"gml:PointPropertyType"_s;

        case Qgis::WkbType::LineString25D:
        case Qgis::WkbType::LineString:
        case Qgis::WkbType::LineStringZ:
          return u"gml:LineStringPropertyType"_s;

        case Qgis::WkbType::Polygon25D:
        case Qgis::WkbType::Polygon:
        case Qgis::WkbType::PolygonZ:
          return u"gml:PolygonPropertyType"_s;

        case Qgis::WkbType::MultiPoint25D:
        case Qgis::WkbType::MultiPoint:
          return u"gml:MultiPointPropertyType"_s;

        case Qgis::WkbType::MultiCurve:
        case Qgis::WkbType::MultiCurveZ:
        case Qgis::WkbType::MultiLineString25D:
        case Qgis::WkbType::MultiLineString:
        case Qgis::WkbType::MultiLineStringZ:
          return u"gml:MultiCurvePropertyType"_s;

        case Qgis::WkbType::MultiSurface:
        case Qgis::WkbType::MultiPolygon25D:
        case Qgis::WkbType::MultiPolygon:
        case Qgis::WkbType::MultiPolygonZ:
          return u"gml:MultiSurfacePropertyType"_s;

        default:
          return u"gml:GeometryPropertyType"_s;
      }
    default:
      return u"gml:GeometryPropertyType"_s;
  }
}
