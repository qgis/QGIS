/***************************************************************************
                              qgswfssutils.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  ( parts from qgswmshandler)
                         (C) 2012 by René-Luc D'Hont    ( parts from qgswmshandler)
                         (C) 2014 by Alessandro Pasotti ( parts from qgswmshandler)
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
 *   (at your option) any later version.                                  *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsutils.h"
#include "qgsogcutils.h"
#include "qgsserverprojectutils.h"
#include "qgswfsparameters.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"

namespace QgsWfs
{
  QString implementationVersion()
  {
    return QStringLiteral( "1.1.0" );
  }

  QString serviceUrl( const QgsServerRequest &request, const QgsProject *project )
  {
    QUrl href;
    if ( project )
    {
      href.setUrl( QgsServerProjectUtils::wfsServiceUrl( *project ) );
    }

    // Build default url
    if ( href.isEmpty() )
    {

      static QSet<QString> sFilter
      {
        QStringLiteral( "REQUEST" ),
        QStringLiteral( "VERSION" ),
        QStringLiteral( "SERVICE" ),
      };

      href = request.originalUrl();
      QUrlQuery q( href );

      for ( auto param : q.queryItems() )
      {
        if ( sFilter.contains( param.first.toUpper() ) )
          q.removeAllQueryItems( param.first );
      }

      href.setQuery( q );
    }

    return  href.toString();
  }

  QString layerTypeName( const QgsMapLayer *layer )
  {
    QString name = layer->name();
    if ( !layer->shortName().isEmpty() )
      name = layer->shortName();
    name = name.replace( ' ', '_' );
    return name;
  }

  QgsVectorLayer *layerByTypeName( const QgsProject *project, const QString &typeName )
  {
    QStringList layerIds = QgsServerProjectUtils::wfsLayerIds( *project );
    for ( const QString &layerId : layerIds )
    {
      QgsMapLayer *layer = project->mapLayer( layerId );
      if ( !layer )
      {
        continue;
      }
      if ( layer->type() != QgsMapLayerType::VectorLayer )
      {
        continue;
      }

      if ( layerTypeName( layer ) == typeName )
      {
        return qobject_cast<QgsVectorLayer *>( layer );
      }
    }
    return nullptr;
  }

  QgsFeatureRequest parseFilterElement( const QString &typeName, QDomElement &filterElem, const QgsProject *project )
  {
    QgsFeatureRequest request;

    QDomNodeList fidNodes = filterElem.elementsByTagName( QStringLiteral( "FeatureId" ) );
    QDomNodeList goidNodes = filterElem.elementsByTagName( QStringLiteral( "GmlObjectId" ) );
    if ( !fidNodes.isEmpty() )
    {
      QgsFeatureIds fids;
      QDomElement fidElem;
      for ( int f = 0; f < fidNodes.size(); f++ )
      {
        fidElem = fidNodes.at( f ).toElement();
        if ( !fidElem.hasAttribute( QStringLiteral( "fid" ) ) )
        {
          throw QgsRequestNotWellFormedException( "FeatureId element without fid attribute" );
        }

        QString fid = fidElem.attribute( QStringLiteral( "fid" ) );
        if ( fid.contains( QLatin1String( "." ) ) )
        {
          if ( fid.section( QStringLiteral( "." ), 0, 0 ) != typeName )
            continue;
          fid = fid.section( QStringLiteral( "." ), 1, 1 );
        }
        fids.insert( fid.toInt() );
      }

      if ( !fids.isEmpty() )
      {
        request.setFilterFids( fids );
      }
      else
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "No FeatureId element correctly parse against typeName '%1'" ).arg( typeName ) );
      }
      request.setFlags( QgsFeatureRequest::NoFlags );
      return request;
    }
    else if ( !goidNodes.isEmpty() )
    {
      QgsFeatureIds fids;
      QDomElement goidElem;
      for ( int f = 0; f < goidNodes.size(); f++ )
      {
        goidElem = goidNodes.at( f ).toElement();
        if ( !goidElem.hasAttribute( QStringLiteral( "id" ) ) && !goidElem.hasAttribute( QStringLiteral( "gml:id" ) ) )
        {
          throw QgsRequestNotWellFormedException( "GmlObjectId element without gml:id attribute" );
        }

        QString fid = goidElem.attribute( QStringLiteral( "id" ) );
        if ( fid.isEmpty() )
          fid = goidElem.attribute( QStringLiteral( "gml:id" ) );
        if ( fid.contains( QLatin1String( "." ) ) )
        {
          if ( fid.section( QStringLiteral( "." ), 0, 0 ) != typeName )
            continue;
          fid = fid.section( QStringLiteral( "." ), 1, 1 );
        }
        fids.insert( fid.toInt() );
      }

      if ( !fids.isEmpty() )
      {
        request.setFilterFids( fids );
      }
      else
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "No GmlObjectId element correctly parse against typeName '%1'" ).arg( typeName ) );
      }
      request.setFlags( QgsFeatureRequest::NoFlags );
      return request;
    }
    else if ( filterElem.firstChildElement().tagName() == QLatin1String( "BBOX" ) )
    {
      QDomElement bboxElem = filterElem.firstChildElement();
      QDomElement childElem = bboxElem.firstChildElement();

      while ( !childElem.isNull() )
      {
        if ( childElem.tagName() == QLatin1String( "Box" ) )
        {
          request.setFilterRect( QgsOgcUtils::rectangleFromGMLBox( childElem ) );
        }
        else if ( childElem.tagName() != QLatin1String( "PropertyName" ) )
        {
          QgsGeometry geom = QgsOgcUtils::geometryFromGML( childElem );
          request.setFilterRect( geom.boundingBox() );
        }
        childElem = childElem.nextSiblingElement();
      }
      request.setFlags( QgsFeatureRequest::ExactIntersect | QgsFeatureRequest::NoFlags );
      return request;
    }
    // Apply BBOX through filterRect even inside an And to use spatial index
    else if ( filterElem.firstChildElement().tagName() == QLatin1String( "And" ) &&
              !filterElem.firstChildElement().firstChildElement( QLatin1String( "BBOX" ) ).isNull() )
    {
      QDomElement childElem = filterElem.firstChildElement().firstChildElement();
      while ( !childElem.isNull() )
      {
        QDomElement childFilterElement = filterElem.ownerDocument().createElement( QLatin1String( "Filter" ) );
        childFilterElement.appendChild( childElem.cloneNode( true ) );
        QgsFeatureRequest childRequest = parseFilterElement( typeName, childFilterElement );
        if ( childElem.tagName() == QLatin1String( "BBOX" ) )
        {
          if ( request.filterRect().isEmpty() )
          {
            request.setFilterRect( childRequest.filterRect() );
          }
          else
          {
            request.setFilterRect( request.filterRect().intersect( childRequest.filterRect() ) );
          }
        }
        else
        {
          if ( !request.filterExpression() )
          {
            request.setFilterExpression( childRequest.filterExpression()->expression() );
          }
          else
          {
            QgsExpressionNode *opLeft = request.filterExpression()->rootNode()->clone();
            QgsExpressionNode *opRight = childRequest.filterExpression()->rootNode()->clone();
            std::unique_ptr<QgsExpressionNodeBinaryOperator> node = qgis::make_unique<QgsExpressionNodeBinaryOperator>( QgsExpressionNodeBinaryOperator::boAnd, opLeft, opRight );
            QgsExpression expr( node->dump() );
            request.setFilterExpression( expr );
          }
        }
        childElem = childElem.nextSiblingElement();
      }
      request.setFlags( QgsFeatureRequest::ExactIntersect | QgsFeatureRequest::NoFlags );
      return request;
    }
    else
    {
      QgsVectorLayer *layer = nullptr;
      if ( project != nullptr )
      {
        layer = layerByTypeName( project, typeName );
      }
      std::shared_ptr<QgsExpression> filter( QgsOgcUtils::expressionFromOgcFilter( filterElem, layer ) );
      if ( filter )
      {
        if ( filter->hasParserError() )
        {
          throw QgsRequestNotWellFormedException( filter->parserErrorString() );
        }

        if ( filter->needsGeometry() )
        {
          request.setFlags( QgsFeatureRequest::NoFlags );
        }
        request.setFilterExpression( filter->expression() );
        return request;
      }
    }
    return request;
  }

} // namespace QgsWfs


