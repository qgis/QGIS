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
#include "qgsconfigcache.h"
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
      if ( layer->type() != QgsMapLayer::LayerType::VectorLayer )
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

  QgsFeatureRequest parseFilterElement( const QString &typeName, QDomElement &filterElem, const QgsProject *project, QStringList *serverFids )
  {
    QgsFeatureRequest request;

    QDomNodeList fidNodes = filterElem.elementsByTagName( QStringLiteral( "FeatureId" ) );
    QDomNodeList goidNodes = filterElem.elementsByTagName( QStringLiteral( "GmlObjectId" ) );
    if ( !fidNodes.isEmpty() )
    {
      // Get the server feature ids in filter element
      QStringList collectedServerFids;
      QDomElement fidElem;
      for ( int f = 0; f < fidNodes.size(); f++ )
      {
        fidElem = fidNodes.at( f ).toElement();
        if ( !fidElem.hasAttribute( QStringLiteral( "fid" ) ) )
        {
          throw QgsRequestNotWellFormedException( "FeatureId element without fid attribute" );
        }

        QString serverFid = fidElem.attribute( QStringLiteral( "fid" ) );
        if ( serverFid.contains( QLatin1String( "." ) ) )
        {
          if ( serverFid.section( QStringLiteral( "." ), 0, 0 ) != typeName )
            continue;
          serverFid = serverFid.section( QStringLiteral( "." ), 1, 1 );
        }
        collectedServerFids << serverFid;
      }
      // No server feature ids found
      if ( collectedServerFids.isEmpty() )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "No FeatureId element correctly parse against typeName '%1'" ).arg( typeName ) );
      }
      // update server feature ids
      serverFids->append( collectedServerFids );
      request.setFlags( QgsFeatureRequest::NoFlags );
      return request;
    }
    else if ( !goidNodes.isEmpty() )
    {
      // Get the server feature idsin filter element
      QStringList collectedServerFids;
      QDomElement goidElem;
      for ( int f = 0; f < goidNodes.size(); f++ )
      {
        goidElem = goidNodes.at( f ).toElement();
        if ( !goidElem.hasAttribute( QStringLiteral( "id" ) ) && !goidElem.hasAttribute( QStringLiteral( "gml:id" ) ) )
        {
          throw QgsRequestNotWellFormedException( "GmlObjectId element without gml:id attribute" );
        }

        QString serverFid = goidElem.attribute( QStringLiteral( "id" ) );
        if ( serverFid.isEmpty() )
          serverFid = goidElem.attribute( QStringLiteral( "gml:id" ) );
        if ( serverFid.contains( QLatin1String( "." ) ) )
        {
          if ( serverFid.section( QStringLiteral( "." ), 0, 0 ) != typeName )
            continue;
          serverFid = serverFid.section( QStringLiteral( "." ), 1, 1 );
        }
        collectedServerFids << serverFid;
      }
      // No server feature ids found
      if ( collectedServerFids.isEmpty() )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "No GmlObjectId element correctly parse against typeName '%1'" ).arg( typeName ) );
      }
      // update server feature ids
      serverFids->append( collectedServerFids );
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
        // Create a filter element to parse And child
        QDomElement childFilterElement = filterElem.ownerDocument().createElement( QLatin1String( "Filter" ) );
        // Clone And child
        childFilterElement.appendChild( childElem.cloneNode( true ) );

        // Parse the filter element with the cloned And child
        QStringList collectedServerFids;
        QgsFeatureRequest childRequest = parseFilterElement( typeName, childFilterElement, project, &collectedServerFids );

        // Update server feature ids
        if ( !collectedServerFids.isEmpty() )
        {
          serverFids->append( collectedServerFids );
        }

        // Update request based on BBOX
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
        // Update expression
        else
        {
          if ( !request.filterExpression() )
          {
            request.setFilterExpression( childRequest.filterExpression()->expression() );
          }
          else
          {
            request.setFilterExpression( QStringLiteral( "( %1 ) AND ( %2 )" ).arg( request.filterExpression()->expression(), childRequest.filterExpression()->expression() ) );
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


