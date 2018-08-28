/***************************************************************************
                              qgswfsdescribefeaturetype.cpp
                              -------------------------
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
#include "qgswfsdescribefeaturetype.h"
#include "qgswfsparameters.h"

#include "qgsproject.h"
#include "qgsexception.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsmapserviceexception.h"
#include "qgscoordinatereferencesystem.h"

#include <QStringList>

namespace QgsWfs
{

  void writeDescribeFeatureType( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                                 const QgsServerRequest &request, QgsServerResponse &response )
  {
    QDomDocument doc = createDescribeFeatureTypeDocument( serverIface, project, version, request );

    response.setHeader( "Content-Type", "text/xml; charset=utf-8" );
    response.write( doc.toByteArray() );
  }


  QDomDocument createDescribeFeatureTypeDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
      const QgsServerRequest &request )
  {
    Q_UNUSED( version );

    QDomDocument doc;

    QgsServerRequest::Parameters parameters = request.parameters();
    QgsWfsParameters wfsParameters( QUrlQuery( request.url() ) );
    QgsWfsParameters::Format oFormat = wfsParameters.outputFormat();

    // test oFormat
    if ( oFormat == QgsWfsParameters::Format::NONE )
      throw QgsBadRequestException( QStringLiteral( "Invalid WFS Parameter" ),
                                    "OUTPUTFORMAT " + wfsParameters.outputFormatAsString() + "is not supported" );

    QgsAccessControl *accessControl = serverIface->accessControls();

    //xsd:schema
    QDomElement schemaElement = doc.createElement( QStringLiteral( "schema" )/*xsd:schema*/ );
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
    QDomElement importElement = doc.createElement( QStringLiteral( "import" )/*xsd:import*/ );
    importElement.setAttribute( QStringLiteral( "namespace" ),  GML_NAMESPACE );
    if ( oFormat == QgsWfsParameters::Format::GML2 )
      importElement.setAttribute( QStringLiteral( "schemaLocation" ), QStringLiteral( "http://schemas.opengis.net/gml/2.1.2/feature.xsd" ) );
    else if ( oFormat == QgsWfsParameters::Format::GML3 )
      importElement.setAttribute( QStringLiteral( "schemaLocation" ), QStringLiteral( "http://schemas.opengis.net/gml/3.1.1/base/gml.xsd" ) );
    schemaElement.appendChild( importElement );

    QStringList typeNameList;
    QDomDocument queryDoc;
    QString errorMsg;
    if ( queryDoc.setContent( parameters.value( QStringLiteral( "REQUEST_BODY" ) ), true, &errorMsg ) )
    {
      //read doc
      QDomElement queryDocElem = queryDoc.documentElement();
      QDomNodeList docChildNodes = queryDocElem.childNodes();
      if ( docChildNodes.size() )
      {
        for ( int i = 0; i < docChildNodes.size(); i++ )
        {
          QDomElement docChildElem = docChildNodes.at( i ).toElement();
          if ( docChildElem.tagName() == QLatin1String( "TypeName" ) )
          {
            QString typeName = docChildElem.text().trimmed();
            if ( typeName.contains( ':' ) )
              typeNameList << typeName.section( ':', 1, 1 );
            else
              typeNameList << typeName;
          }
        }
      }
    }
    else
    {
      QString typeNames = request.parameter( QStringLiteral( "TYPENAME" ) );
      if ( !typeNames.isEmpty() )
      {
        QStringList typeNameSplit = typeNames.split( ',' );
        for ( int i = 0; i < typeNameSplit.size(); ++i )
        {
          QString typeName = typeNameSplit.at( i ).trimmed();
          if ( typeName.contains( ':' ) )
            typeNameList << typeName.section( ':', 1, 1 );
          else
            typeNameList << typeName;
        }
      }
    }

    QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
    for ( int i = 0; i < wfsLayerIds.size(); ++i )
    {
      QgsMapLayer *layer = project->mapLayer( wfsLayerIds.at( i ) );
      if ( !layer )
      {
        continue;
      }
      if ( layer->type() != QgsMapLayer::LayerType::VectorLayer )
      {
        continue;
      }

      QString name = layerTypeName( layer );

      if ( !typeNameList.isEmpty() && !typeNameList.contains( name ) )
      {
        continue;
      }

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

  void setSchemaLayer( QDomElement &parentElement, QDomDocument &doc, const QgsVectorLayer *layer )
  {
    const QgsVectorDataProvider *provider = layer->dataProvider();
    if ( !provider )
    {
      return;
    }

    QString typeName = layerTypeName( layer );

    //xsd:element
    QDomElement elementElem = doc.createElement( QStringLiteral( "element" )/*xsd:element*/ );
    elementElem.setAttribute( QStringLiteral( "name" ), typeName );
    elementElem.setAttribute( QStringLiteral( "type" ), "qgs:" + typeName + "Type" );
    elementElem.setAttribute( QStringLiteral( "substitutionGroup" ), QStringLiteral( "gml:_Feature" ) );
    parentElement.appendChild( elementElem );

    //xsd:complexType
    QDomElement complexTypeElem = doc.createElement( QStringLiteral( "complexType" )/*xsd:complexType*/ );
    complexTypeElem.setAttribute( QStringLiteral( "name" ), typeName + "Type" );
    parentElement.appendChild( complexTypeElem );

    //xsd:complexType
    QDomElement complexContentElem = doc.createElement( QStringLiteral( "complexContent" )/*xsd:complexContent*/ );
    complexTypeElem.appendChild( complexContentElem );

    //xsd:extension
    QDomElement extensionElem = doc.createElement( QStringLiteral( "extension" )/*xsd:extension*/ );
    extensionElem.setAttribute( QStringLiteral( "base" ), QStringLiteral( "gml:AbstractFeatureType" ) );
    complexContentElem.appendChild( extensionElem );

    //xsd:sequence
    QDomElement sequenceElem = doc.createElement( QStringLiteral( "sequence" )/*xsd:sequence*/ );
    extensionElem.appendChild( sequenceElem );

    //xsd:element
    if ( layer->isSpatial() )
    {
      QDomElement geomElem = doc.createElement( QStringLiteral( "element" )/*xsd:element*/ );
      geomElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "geometry" ) );

      QgsWkbTypes::Type wkbType = layer->wkbType();
      switch ( wkbType )
      {
        case QgsWkbTypes::Point25D:
        case QgsWkbTypes::Point:
          geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:PointPropertyType" ) );
          break;
        case QgsWkbTypes::LineString25D:
        case QgsWkbTypes::LineString:
          geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:LineStringPropertyType" ) );
          break;
        case QgsWkbTypes::Polygon25D:
        case QgsWkbTypes::Polygon:
          geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:PolygonPropertyType" ) );
          break;
        case QgsWkbTypes::MultiPoint25D:
        case QgsWkbTypes::MultiPoint:
          geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:MultiPointPropertyType" ) );
          break;
        case QgsWkbTypes::MultiLineString25D:
        case QgsWkbTypes::MultiLineString:
          geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:MultiLineStringPropertyType" ) );
          break;
        case QgsWkbTypes::MultiPolygon25D:
        case QgsWkbTypes::MultiPolygon:
          geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:MultiPolygonPropertyType" ) );
          break;
        default:
          geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:GeometryPropertyType" ) );
          break;
      }
      geomElem.setAttribute( QStringLiteral( "minOccurs" ), QStringLiteral( "0" ) );
      geomElem.setAttribute( QStringLiteral( "maxOccurs" ), QStringLiteral( "1" ) );
      sequenceElem.appendChild( geomElem );

      //Attributes
      QgsFields fields = layer->fields();
      //hidden attributes for this layer
      const QSet<QString> &layerExcludedAttributes = layer->excludeAttributesWfs();
      for ( int idx = 0; idx < fields.count(); ++idx )
      {

        QString attributeName = fields.at( idx ).name();
        //skip attribute if excluded from WFS publication
        if ( layerExcludedAttributes.contains( attributeName ) )
        {
          continue;
        }

        //xsd:element
        QDomElement attElem = doc.createElement( QStringLiteral( "element" )/*xsd:element*/ );
        attElem.setAttribute( QStringLiteral( "name" ), attributeName.replace( ' ', '_' ).replace( cleanTagNameRegExp, QString() ) );
        QVariant::Type attributeType = fields.at( idx ).type();
        if ( attributeType == QVariant::Int )
          attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "integer" ) );
        else if ( attributeType == QVariant::LongLong )
          attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "long" ) );
        else if ( attributeType == QVariant::Double )
          attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "double" ) );
        else if ( attributeType == QVariant::Bool )
          attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "boolean" ) );
        else if ( attributeType == QVariant::Date )
          attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "date" ) );
        else if ( attributeType == QVariant::Time )
          attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "time" ) );
        else if ( attributeType == QVariant::DateTime )
          attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "dateTime" ) );
        else
          attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "string" ) );

        sequenceElem.appendChild( attElem );

        QString alias = fields.at( idx ).alias();
        if ( !alias.isEmpty() )
        {
          attElem.setAttribute( QStringLiteral( "alias" ), alias );
        }
      }
    }
  }

} // namespace QgsWfs
