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
#include "qgswfsutils.h"
#include "qgsserverprojectutils.h"
#include "qgswfsdescribefeaturetypegml.h"
#include "qgswfsdescribefeaturetype.h"
#include "qgswfsparameters.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

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
  QDomElement schemaElement = doc.createElement( QStringLiteral( "schema" ) /*xsd:schema*/ );
  schemaElement.setAttribute( QStringLiteral( "xmlns" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema" ) );
  schemaElement.setAttribute( QStringLiteral( "xmlns:xsd" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema" ) );
  schemaElement.setAttribute( QStringLiteral( "xmlns:ogc" ), OGC_NAMESPACE );
  schemaElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
  schemaElement.setAttribute( QStringLiteral( "xmlns:qgs" ), QGS_NAMESPACE );
  schemaElement.setAttribute( QStringLiteral( "targetNamespace" ), QGS_NAMESPACE );
  schemaElement.setAttribute( QStringLiteral( "elementFormDefault" ), QStringLiteral( "qualified" ) );
  schemaElement.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( schemaElement );

  //xsd:import
  QDomElement importElement = doc.createElement( QStringLiteral( "import" ) /*xsd:import*/ );
  importElement.setAttribute( QStringLiteral( "namespace" ), GML_NAMESPACE );
  if ( outputFormat == QgsWfsParameters::Format::GML2 )
    importElement.setAttribute( QStringLiteral( "schemaLocation" ), QStringLiteral( "http://schemas.opengis.net/gml/2.1.2/feature.xsd" ) );
  else if ( outputFormat == QgsWfsParameters::Format::GML3 )
    importElement.setAttribute( QStringLiteral( "schemaLocation" ), QStringLiteral( "http://schemas.opengis.net/gml/3.1.1/base/gml.xsd" ) );
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

  const QString typeName = layerTypeName( layer );

  //xsd:element
  QDomElement elementElem = doc.createElement( QStringLiteral( "element" ) /*xsd:element*/ );
  elementElem.setAttribute( QStringLiteral( "name" ), typeName );
  elementElem.setAttribute( QStringLiteral( "type" ), "qgs:" + typeName + "Type" );
  elementElem.setAttribute( QStringLiteral( "substitutionGroup" ), QStringLiteral( "gml:_Feature" ) );
  parentElement.appendChild( elementElem );

  //xsd:complexType
  QDomElement complexTypeElem = doc.createElement( QStringLiteral( "complexType" ) /*xsd:complexType*/ );
  complexTypeElem.setAttribute( QStringLiteral( "name" ), typeName + "Type" );
  parentElement.appendChild( complexTypeElem );

  //xsd:complexType
  QDomElement complexContentElem = doc.createElement( QStringLiteral( "complexContent" ) /*xsd:complexContent*/ );
  complexTypeElem.appendChild( complexContentElem );

  //xsd:extension
  QDomElement extensionElem = doc.createElement( QStringLiteral( "extension" ) /*xsd:extension*/ );
  extensionElem.setAttribute( QStringLiteral( "base" ), QStringLiteral( "gml:AbstractFeatureType" ) );
  complexContentElem.appendChild( extensionElem );

  //xsd:sequence
  QDomElement sequenceElem = doc.createElement( QStringLiteral( "sequence" ) /*xsd:sequence*/ );
  extensionElem.appendChild( sequenceElem );

  //xsd:element
  if ( layer->isSpatial() )
  {
    QDomElement geomElem = doc.createElement( QStringLiteral( "element" ) /*xsd:element*/ );
    geomElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "geometry" ) );
    geomElem.setAttribute( QStringLiteral( "type" ), getGmlGeometryType( layer ) );
    geomElem.setAttribute( QStringLiteral( "minOccurs" ), QStringLiteral( "0" ) );
    geomElem.setAttribute( QStringLiteral( "maxOccurs" ), QStringLiteral( "1" ) );
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
    QDomElement attElem = doc.createElement( QStringLiteral( "element" ) /*xsd:element*/ );

    attElem.setAttribute( QStringLiteral( "name" ), attributeName );
    attElem.setAttribute( QStringLiteral( "type" ), attributeType );

    if ( !( field.constraints().constraints() & QgsFieldConstraints::Constraint::ConstraintNotNull ) )
    {
      attElem.setAttribute( QStringLiteral( "nillable" ), QStringLiteral( "true" ) );
    }

    sequenceElem.appendChild( attElem );

    const QString alias = field.alias();
    if ( !alias.isEmpty() )
    {
      attElem.setAttribute( QStringLiteral( "alias" ), alias );
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
        case Qgis::WkbType::Point25D:
        case Qgis::WkbType::Point:
          return QStringLiteral( "gml:PointPropertyType" );

        case Qgis::WkbType::LineString25D:
        case Qgis::WkbType::LineString:
          return QStringLiteral( "gml:LineStringPropertyType" );

        case Qgis::WkbType::Polygon25D:
        case Qgis::WkbType::Polygon:
          return QStringLiteral( "gml:PolygonPropertyType" );

        case Qgis::WkbType::MultiPoint25D:
        case Qgis::WkbType::MultiPoint:
          return QStringLiteral( "gml:MultiPointPropertyType" );

        case Qgis::WkbType::MultiCurve:
        case Qgis::WkbType::MultiLineString25D:
        case Qgis::WkbType::MultiLineString:
          return QStringLiteral( "gml:MultiLineStringPropertyType" );

        case Qgis::WkbType::MultiSurface:
        case Qgis::WkbType::MultiPolygon25D:
        case Qgis::WkbType::MultiPolygon:
          return QStringLiteral( "gml:MultiPolygonPropertyType" );

        default:
          return QStringLiteral( "gml:GeometryPropertyType" );
      }
    case QgsWfsParameters::Format::GML3:
      switch ( wkbType )
      {
        case Qgis::WkbType::Point25D:
        case Qgis::WkbType::Point:
          return QStringLiteral( "gml:PointPropertyType" );

        case Qgis::WkbType::LineString25D:
        case Qgis::WkbType::LineString:
          return QStringLiteral( "gml:LineStringPropertyType" );

        case Qgis::WkbType::Polygon25D:
        case Qgis::WkbType::Polygon:
          return QStringLiteral( "gml:PolygonPropertyType" );

        case Qgis::WkbType::MultiPoint25D:
        case Qgis::WkbType::MultiPoint:
          return QStringLiteral( "gml:MultiPointPropertyType" );

        case Qgis::WkbType::MultiCurve:
        case Qgis::WkbType::MultiLineString25D:
        case Qgis::WkbType::MultiLineString:
          return QStringLiteral( "gml:MultiCurvePropertyType" );

        case Qgis::WkbType::MultiSurface:
        case Qgis::WkbType::MultiPolygon25D:
        case Qgis::WkbType::MultiPolygon:
          return QStringLiteral( "gml:MultiSurfacePropertyType" );

        default:
          return QStringLiteral( "gml:GeometryPropertyType" );
      }
    default:
      return QStringLiteral( "gml:GeometryPropertyType" );
  }
}
