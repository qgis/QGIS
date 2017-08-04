/***************************************************************************
                              qgswfsgetfeature.cpp
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
#include "qgsfields.h"
#include "qgsexpression.h"
#include "qgsgeometry.h"
#include "qgsmaplayer.h"
#include "qgsfeatureiterator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsfilterrestorer.h"
#include "qgsproject.h"
#include "qgsogcutils.h"
#include "qgsjsonutils.h"

#include "qgswfsgetfeature.h"

#include <QStringList>

namespace QgsWfs
{

  namespace
  {

    QString createFeatureGeoJSON( QgsFeature *feat, int prec, QgsCoordinateReferenceSystem &crs, const QgsAttributeList &attrIndexes,
                                  const QSet<QString> &excludedAttributes, const QString &typeName, bool withGeom,
                                  const QString &geometryName );

    QDomElement createFeatureGML2( QgsFeature *feat, QDomDocument &doc, int prec, QgsCoordinateReferenceSystem &crs,
                                   const QgsAttributeList &attrIndexes, const QSet<QString> &excludedAttributes, const QString &typeName,
                                   bool withGeom, const QString &geometryName );

    QDomElement createFeatureGML3( QgsFeature *feat, QDomDocument &doc, int prec, QgsCoordinateReferenceSystem &crs,
                                   const QgsAttributeList &attrIndexes, const QSet<QString> &excludedAttributes, const QString &typeName,
                                   bool withGeom, const QString geometryName );

    void startGetFeature( const QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project, const QString &format,
                          int prec, QgsCoordinateReferenceSystem &crs, QgsRectangle *rect, const QStringList &typeNames );

    void setGetFeature( QgsServerResponse &response, const QString &format, QgsFeature *feat, int featIdx, int prec,
                        QgsCoordinateReferenceSystem &crs, const QgsAttributeList &attrIndexes, const QSet<QString> &excludedAttributes,
                        const QString &typeName, bool withGeom, const QString &geometryName );

    void endGetFeature( QgsServerResponse &response, const QString &format );

  }

  void writeGetFeature( QgsServerInterface *serverIface, const QgsProject *project,
                        const QString &version, const QgsServerRequest &request,
                        QgsServerResponse &response )
  {
    Q_UNUSED( version );

    QgsServerRequest::Parameters parameters = request.parameters();
    getFeatureRequest aRequest;

    QDomDocument doc;
    QString errorMsg;

    if ( doc.setContent( parameters.value( QStringLiteral( "REQUEST_BODY" ) ), true, &errorMsg ) )
    {
      QDomElement docElem = doc.documentElement();
      aRequest = parseGetFeatureRequestBody( docElem );
    }
    else
    {
      aRequest = parseGetFeatureParameters( parameters );
    }

    // store typeName
    QStringList typeNameList;

    // Request metadata
    bool onlyOneLayer = ( aRequest.queries.size() == 1 );
    QgsRectangle requestRect;
    QgsCoordinateReferenceSystem requestCrs;
    int requestPrecision = 6;
    if ( !onlyOneLayer )
      requestCrs = QgsCoordinateReferenceSystem( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );

    QList<getFeatureQuery>::iterator qIt = aRequest.queries.begin();
    for ( ; qIt != aRequest.queries.end(); ++qIt )
    {
      typeNameList << ( *qIt ).typeName;
    }

    // get layers and
    // update the request metadata
    QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
    QMap<QString, QgsMapLayer *> mapLayerMap;
    for ( int i = 0; i < wfsLayerIds.size(); ++i )
    {
      QgsMapLayer *layer = project->mapLayer( wfsLayerIds.at( i ) );
      if ( layer->type() != QgsMapLayer::LayerType::VectorLayer )
      {
        continue;
      }

      QString name = layer->name();
      if ( !layer->shortName().isEmpty() )
        name = layer->shortName();
      name = name.replace( ' ', '_' );

      if ( typeNameList.contains( name ) )
      {
        // store layers
        mapLayerMap[name] = layer;
        // update request metadata
        if ( onlyOneLayer )
        {
          requestRect = layer->extent();
          requestCrs = layer->crs();
        }
        else
        {
          QgsCoordinateTransform transform( layer->crs(), requestCrs );
          try
          {
            if ( requestRect.isEmpty() )
            {
              requestRect = transform.transform( layer->extent() );
            }
            else
            {
              requestRect.combineExtentWith( transform.transform( layer->extent() ) );
            }
          }
          catch ( QgsException &cse )
          {
            Q_UNUSED( cse );
            requestRect = QgsRectangle( -180.0, -90.0, 180.0, 90.0 );
          }
        }
      }
    }

    QgsAccessControl *accessControl = serverIface->accessControls();

    //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
    //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
    std::unique_ptr< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer( accessControl ) );

    // features counters
    long sentFeatures = 0;
    long iteratedFeatures = 0;
    // sent features
    QgsFeature feature;
    qIt = aRequest.queries.begin();
    for ( ; qIt != aRequest.queries.end(); ++qIt )
    {
      getFeatureQuery &query = *qIt;
      QString typeName = query.typeName;

      if ( !mapLayerMap.keys().contains( typeName ) )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "TypeName '%1' unknown" ).arg( typeName ) );
      }

      QgsMapLayer *layer = mapLayerMap[typeName];
      if ( accessControl && !accessControl->layerReadPermission( layer ) )
      {
        throw QgsSecurityAccessException( QStringLiteral( "Feature access permission denied" ) );
      }

      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      if ( !vlayer )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "TypeName '%1' layer error" ).arg( typeName ) );
      }

      //test provider
      QgsVectorDataProvider *provider = vlayer->dataProvider();
      if ( !provider )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "TypeName '%1' layer's provider error" ).arg( typeName ) );
      }

      if ( accessControl )
      {
        QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, vlayer, filterRestorer->originalFilters() );
      }

      //is there alias info for this vector layer?
      QMap< int, QString > layerAliasInfo;
      QgsStringMap aliasMap = vlayer->attributeAliases();
      QgsStringMap::const_iterator aliasIt = aliasMap.constBegin();
      for ( ; aliasIt != aliasMap.constEnd(); ++aliasIt )
      {
        int attrIndex = vlayer->fields().lookupField( aliasIt.key() );
        if ( attrIndex != -1 )
        {
          layerAliasInfo.insert( attrIndex, aliasIt.value() );
        }
      }

      // get propertyList from query
      QStringList propertyList = query.propertyList;

      //Using pending attributes and pending fields
      QgsAttributeList attrIndexes = vlayer->pendingAllAttributesList();
      bool withGeom = true;
      if ( !propertyList.isEmpty() && propertyList.first() != QStringLiteral( "*" ) )
      {
        withGeom = false;
        QStringList::const_iterator plstIt;
        QList<int> idxList;
        QgsFields fields = vlayer->pendingFields();
        QString fieldName;
        for ( plstIt = propertyList.begin(); plstIt != propertyList.end(); ++plstIt )
        {
          fieldName = *plstIt;
          int fieldNameIdx = fields.lookupField( fieldName );
          if ( fieldNameIdx > -1 )
          {
            idxList.append( fieldNameIdx );
          }
          else if ( fieldName == QStringLiteral( "geometry" ) )
          {
            withGeom = true;
          }
        }
        if ( !idxList.isEmpty() )
        {
          attrIndexes = idxList;
        }
      }


      // update request
      QgsFeatureRequest featureRequest = query.featureRequest;

      // expression context
      QgsExpressionContext expressionContext;
      expressionContext << QgsExpressionContextUtils::globalScope()
                        << QgsExpressionContextUtils::projectScope( project )
                        << QgsExpressionContextUtils::layerScope( vlayer );
      featureRequest.setExpressionContext( expressionContext );

      // geometry flags
      if ( vlayer->wkbType() == QgsWkbTypes::NoGeometry )
        featureRequest.setFlags( featureRequest.flags() | QgsFeatureRequest::NoGeometry );
      else
        featureRequest.setFlags( featureRequest.flags() | ( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
      // subset of attributes
      featureRequest.setSubsetOfAttributes( attrIndexes );

      if ( accessControl )
      {
        accessControl->filterFeatures( vlayer, featureRequest );

        QStringList attributes = QStringList();
        Q_FOREACH ( int idx, attrIndexes )
        {
          attributes.append( vlayer->pendingFields().field( idx ).name() );
        }
        featureRequest.setSubsetOfAttributes(
          accessControl->layerAttributes( vlayer, attributes ),
          vlayer->pendingFields() );
      }

      if ( onlyOneLayer )
      {
        requestPrecision = QgsServerProjectUtils::wfsLayerPrecision( *project, vlayer->id() );
      }

      if ( onlyOneLayer && !featureRequest.filterRect().isEmpty() )
      {
        requestRect = featureRequest.filterRect();
      }

      if ( aRequest.maxFeatures > 0 )
      {
        featureRequest.setLimit( aRequest.maxFeatures + aRequest.startIndex - sentFeatures );
      }
      // specific layer precision
      int layerPrecision = QgsServerProjectUtils::wfsLayerPrecision( *project, vlayer->id() );
      // specific layer crs
      QgsCoordinateReferenceSystem layerCrs = vlayer->crs();
      //excluded attributes for this layer
      const QSet<QString> &layerExcludedAttributes = vlayer->excludeAttributesWfs();
      // Geometry name
      QString geometryName = aRequest.geometryName;
      if ( !withGeom )
      {
        geometryName = QLatin1String( "NONE" );
      }

      // Iterate through features
      QgsFeatureIterator fit = vlayer->getFeatures( featureRequest );
      while ( fit.nextFeature( feature ) && ( aRequest.maxFeatures == -1 || sentFeatures < aRequest.maxFeatures ) )
      {
        if ( iteratedFeatures == aRequest.startIndex )
          startGetFeature( request, response, project, aRequest.outputFormat, requestPrecision, requestCrs, &requestRect, typeNameList );

        if ( iteratedFeatures >= aRequest.startIndex )
        {
          setGetFeature( response, aRequest.outputFormat, &feature, sentFeatures, layerPrecision, layerCrs, attrIndexes, layerExcludedAttributes,
                         typeName, withGeom, geometryName );
          ++sentFeatures;
        }
        ++iteratedFeatures;
      }
    }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    //force restoration of original layer filters
    filterRestorer.reset();
#endif

    // End of GetFeature
    if ( iteratedFeatures <= aRequest.startIndex )
      startGetFeature( request, response, project, aRequest.outputFormat, requestPrecision, requestCrs, &requestRect, typeNameList );
    endGetFeature( response, aRequest.outputFormat );

  }

  getFeatureRequest parseGetFeatureParameters( QgsServerRequest::Parameters parameters )
  {
    getFeatureRequest request;
    request.maxFeatures = -1;
    request.startIndex = 0;
    request.outputFormat = parameters.value( QStringLiteral( "OUTPUTFORMAT" ), QStringLiteral( "GML2" ) );

    // Verifying parameters mutually exclusive
    if ( ( parameters.contains( QStringLiteral( "FEATUREID" ) )
           && ( parameters.contains( QStringLiteral( "FILTER" ) ) || parameters.contains( QStringLiteral( "BBOX" ) ) ) )
         || ( parameters.contains( QStringLiteral( "FILTER" ) )
              && ( parameters.contains( QStringLiteral( "FEATUREID" ) ) || parameters.contains( QStringLiteral( "BBOX" ) ) ) )
         || ( parameters.contains( QStringLiteral( "BBOX" ) )
              && ( parameters.contains( QStringLiteral( "FEATUREID" ) ) || parameters.contains( QStringLiteral( "FILTER" ) ) ) )
       )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "FEATUREID FILTER and BBOX parameters are mutually exclusive" ) );
    }

    // Get and split PROPERTYNAME parameter
    QStringList propertyNameList;
    if ( parameters.contains( QStringLiteral( "PROPERTYNAME" ) ) )
    {
      QString propertyName = parameters.value( QStringLiteral( "PROPERTYNAME" ) );
      QRegExp rx( "\\(([^()]+)\\)" );
      if ( rx.indexIn( propertyName, 0 ) == -1 )
      {
        propertyNameList << propertyName;
      }
      else
      {
        int pos = 0;
        while ( ( pos = rx.indexIn( propertyName, pos ) ) != -1 )
        {
          propertyNameList << rx.cap( 1 );
          pos += rx.matchedLength();
        }
      }
    }

    // Manage extra parameter GeometryName
    request.geometryName = QLatin1String( "" );
    if ( parameters.contains( QStringLiteral( "GEOMETRYNAME" ) ) )
    {
      request.geometryName = parameters.value( QStringLiteral( "GEOMETRYNAME" ) ).toUpper();
    }

    QStringList typeNameList;
    // parse FEATUREID
    if ( parameters.contains( QStringLiteral( "FEATUREID" ) ) )
    {
      QStringList fidList = parameters.value( QStringLiteral( "FEATUREID" ) ).split( ',' );
      // Verifying the 1:1 mapping between FEATUREID and PROPERTYNAME
      if ( !propertyNameList.isEmpty() && propertyNameList.size() != fidList.size() )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "There has to be a 1:1 mapping between each element in a FEATUREID and the PROPERTYNAME list" ) );
      }
      if ( propertyNameList.isEmpty() )
      {
        for ( int i = 0; i < fidList.size(); ++i )
        {
          propertyNameList << QStringLiteral( "*" );
        }
      }

      QMap<QString, QgsFeatureIds> fidsMap;

      QStringList::const_iterator fidIt = fidList.constBegin();
      QStringList::const_iterator propertyNameIt = propertyNameList.constBegin();
      for ( ; fidIt != fidList.constEnd(); ++fidIt )
      {
        // Get FeatureID
        QString fid = *fidIt;
        fid = fid.trimmed();
        // Get PropertyName for this FeatureID
        QString propertyName;
        if ( propertyNameIt != propertyNameList.constEnd() )
        {
          propertyName = *propertyNameIt;
        }
        // testing typename in the WFS featureID
        if ( !fid.contains( '.' ) )
        {
          throw QgsRequestNotWellFormedException( QStringLiteral( "FEATUREID has to have TYPENAME in the values" ) );
        }

        QString typeName = fid.section( '.', 0, 0 );
        fid = fid.section( '.', 1, 1 );
        if ( !typeNameList.contains( typeName ) )
        {
          typeNameList << typeName;
        }

        // each Feature requested by FEATUREID can have each own property list
        QString key = QStringLiteral( "%1(%2)" ).arg( typeName ).arg( propertyName );
        QgsFeatureIds fids;
        if ( fidsMap.contains( key ) )
        {
          fids = fidsMap.value( key );
        }
        fids.insert( fid.toInt() );
        fidsMap.insert( key, fids );

        if ( propertyNameIt != propertyNameList.constEnd() )
        {
          ++propertyNameIt;
        }
      }

      QMap<QString, QgsFeatureIds>::const_iterator fidsMapIt = fidsMap.constBegin();
      while ( fidsMapIt != fidsMap.constEnd() )
      {
        QString key = fidsMapIt.key();

        //Extract TypeName and PropertyName from key
        QRegExp rx( "([^()]+)\\(([^()]+)\\)" );
        if ( rx.indexIn( key, 0 ) == -1 )
        {
          throw QgsRequestNotWellFormedException( QStringLiteral( "Error getting properties for FEATUREID" ) );
        }
        QString typeName = rx.cap( 1 );
        QString propertyName = rx.cap( 2 );

        getFeatureQuery query;
        query.typeName = typeName;

        // Parse PropertyName
        if ( propertyName != QStringLiteral( "*" ) )
        {
          QStringList propertyList;

          QStringList attrList = propertyName.split( ',' );
          QStringList::const_iterator alstIt;
          for ( alstIt = attrList.begin(); alstIt != attrList.end(); ++alstIt )
          {
            QString fieldName = *alstIt;
            fieldName = fieldName.trimmed();
            if ( fieldName.contains( ':' ) )
            {
              fieldName = fieldName.section( ':', 1, 1 );
            }
            if ( fieldName.contains( '/' ) )
            {
              if ( fieldName.section( '/', 0, 0 ) != typeName )
              {
                throw QgsRequestNotWellFormedException( QStringLiteral( "PropertyName text '%1' has to contain TypeName '%2'" ).arg( fieldName ).arg( typeName ) );
              }
              fieldName = fieldName.section( '/', 1, 1 );
            }
            propertyList.append( fieldName );
          }
          query.propertyList = propertyList;
        }

        QgsFeatureIds fids = fidsMapIt.value();
        QgsFeatureRequest featureRequest( fids );

        query.featureRequest = featureRequest;
        request.queries.append( query );
      }
      return request;
    }

    if ( parameters.contains( QStringLiteral( "MAXFEATURES" ) ) )
    {
      request.maxFeatures = parameters.value( QStringLiteral( "MAXFEATURES" ) ).toLong();
    }
    if ( parameters.contains( QStringLiteral( "STARTINDEX" ) ) )
    {
      request.startIndex = parameters.value( QStringLiteral( "STARTINDEX" ) ).toLong();
    }

    if ( !parameters.contains( QStringLiteral( "TYPENAME" ) ) )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TYPENAME is mandatory except if FEATUREID is used" ) );
    }

    typeNameList = parameters.value( QStringLiteral( "TYPENAME" ) ).split( ',' );
    // Verifying the 1:1 mapping between TYPENAME and PROPERTYNAME
    if ( !propertyNameList.isEmpty() && typeNameList.size() != propertyNameList.size() )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "There has to be a 1:1 mapping between each element in a TYPENAME and the PROPERTYNAME list" ) );
    }
    if ( propertyNameList.isEmpty() )
    {
      for ( int i = 0; i < typeNameList.size(); ++i )
      {
        propertyNameList << QStringLiteral( "*" );
      }
    }

    // Create queries based on TypeName and propertyName
    QStringList::const_iterator typeNameIt = typeNameList.constBegin();
    QStringList::const_iterator propertyNameIt = propertyNameList.constBegin();
    for ( ; typeNameIt != typeNameList.constEnd(); ++typeNameIt )
    {
      QString typeName = *typeNameIt;
      typeName = typeName.trimmed();
      // Get PropertyName for this typeName
      QString propertyName;
      if ( propertyNameIt != propertyNameList.constEnd() )
      {
        propertyName = *propertyNameIt;
      }

      getFeatureQuery query;
      query.typeName = typeName;

      // Parse PropertyName
      if ( propertyName != QStringLiteral( "*" ) )
      {
        QStringList propertyList;

        QStringList attrList = propertyName.split( ',' );
        QStringList::const_iterator alstIt;
        for ( alstIt = attrList.begin(); alstIt != attrList.end(); ++alstIt )
        {
          QString fieldName = *alstIt;
          fieldName = fieldName.trimmed();
          if ( fieldName.contains( ':' ) )
          {
            fieldName = fieldName.section( ':', 1, 1 );
          }
          if ( fieldName.contains( '/' ) )
          {
            if ( fieldName.section( '/', 0, 0 ) != typeName )
            {
              throw QgsRequestNotWellFormedException( QStringLiteral( "PropertyName text '%1' has to contain TypeName '%2'" ).arg( fieldName ).arg( typeName ) );
            }
            fieldName = fieldName.section( '/', 1, 1 );
          }
          propertyList.append( fieldName );
        }
        query.propertyList = propertyList;
      }

      request.queries.append( query );

      if ( propertyNameIt != propertyNameList.constEnd() )
      {
        ++propertyNameIt;
      }
    }

    // Manage extra parameter exp_filter
    if ( parameters.contains( QStringLiteral( "EXP_FILTER" ) ) )
    {
      QString expFilterName = parameters.value( QStringLiteral( "EXP_FILTER" ) );
      QStringList expFilterList;
      QRegExp rx( "\\(([^()]+)\\)" );
      if ( rx.indexIn( expFilterName, 0 ) == -1 )
      {
        expFilterList << expFilterName;
      }
      else
      {
        int pos = 0;
        while ( ( pos = rx.indexIn( expFilterName, pos ) ) != -1 )
        {
          expFilterList << rx.cap( 1 );
          pos += rx.matchedLength();
        }
      }

      // Verifying the 1:1 mapping between TYPENAME and EXP_FILTER but without exception
      if ( request.queries.size() == expFilterList.size() )
      {
        // set feature request filter expression based on filter element
        QList<getFeatureQuery>::iterator qIt = request.queries.begin();
        QStringList::const_iterator expFilterIt = expFilterList.constBegin();
        for ( ; qIt != request.queries.end(); ++qIt )
        {
          getFeatureQuery &query = *qIt;
          // Get Filter for this typeName
          QString expFilter;
          if ( expFilterIt != expFilterList.constEnd() )
          {
            expFilter = *expFilterIt;
          }
          std::shared_ptr<QgsExpression> filter( new QgsExpression( expFilter ) );
          if ( filter )
          {
            if ( filter->hasParserError() )
            {
              QgsMessageLog::logMessage( filter->parserErrorString() );
            }
            else
            {
              if ( filter->needsGeometry() )
              {
                query.featureRequest.setFlags( QgsFeatureRequest::NoFlags );
              }
              query.featureRequest.setFilterExpression( filter->expression() );
            }
          }
        }
      }
      else
      {
        QgsMessageLog::logMessage( "There has to be a 1:1 mapping between each element in a TYPENAME and the EXP_FILTER list" );
      }
    }

    if ( parameters.contains( QStringLiteral( "BBOX" ) ) )
    {
      // get bbox value
      QString bbox = parameters.value( QStringLiteral( "BBOX" ) );
      if ( bbox.isEmpty() )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "BBOX parameter is empty" ) );
      }

      // get bbox corners
      QStringList corners = bbox.split( "," );
      if ( corners.size() != 4 )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "BBOX has to be composed of 4 elements: '%1'" ).arg( bbox ) );
      }

      // convert corners to double
      double d[4];
      bool ok;
      for ( int i = 0; i < 4; i++ )
      {
        corners[i].replace( ' ', '+' );
        d[i] = corners[i].toDouble( &ok );
        if ( !ok )
        {
          throw QgsRequestNotWellFormedException( QStringLiteral( "BBOX has to be composed of 4 double: '%1'" ).arg( bbox ) );
        }
      }
      // create extent
      QgsRectangle extent( d[0], d[1], d[2], d[3] );

      // set feature request filter rectangle
      QList<getFeatureQuery>::iterator qIt = request.queries.begin();
      for ( ; qIt != request.queries.end(); ++qIt )
      {
        getFeatureQuery &query = *qIt;
        query.featureRequest.setFilterRect( extent );
      }
      return request;
    }
    else if ( parameters.contains( QStringLiteral( "FILTER" ) ) )
    {
      QString filterName = parameters.value( QStringLiteral( "FILTER" ) );
      QStringList filterList;
      QRegExp rx( "\\(([^()]+)\\)" );
      if ( rx.indexIn( filterName, 0 ) == -1 )
      {
        filterList << filterName;
      }
      else
      {
        int pos = 0;
        while ( ( pos = rx.indexIn( filterName, pos ) ) != -1 )
        {
          filterList << rx.cap( 1 );
          pos += rx.matchedLength();
        }
      }

      // Verifying the 1:1 mapping between TYPENAME and FILTER
      if ( request.queries.size() != filterList.size() )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "There has to be a 1:1 mapping between each element in a TYPENAME and the FILTER list" ) );
      }

      // set feature request filter expression based on filter element
      QList<getFeatureQuery>::iterator qIt = request.queries.begin();
      QStringList::const_iterator filterIt = filterList.constBegin();
      for ( ; qIt != request.queries.end(); ++qIt )
      {
        getFeatureQuery &query = *qIt;
        // Get Filter for this typeName
        QDomDocument filter;
        if ( filterIt != filterList.constEnd() )
        {
          QString errorMsg;
          if ( !filter.setContent( *filterIt, true, &errorMsg ) )
          {
            throw QgsRequestNotWellFormedException( QStringLiteral( "error message: %1. The XML string was: %2" ).arg( errorMsg, *filterIt ) );
          }
        }

        QDomElement filterElem = filter.firstChildElement();
        query.featureRequest = parseFilterElement( query.typeName, filterElem );

        if ( filterIt != filterList.constEnd() )
        {
          ++filterIt;
        }
      }
      return request;
    }

    return request;
  }

  getFeatureRequest parseGetFeatureRequestBody( QDomElement &docElem )
  {
    getFeatureRequest request;
    request.maxFeatures = -1;
    request.startIndex = 0;
    request.outputFormat = "GML2";

    if ( docElem.hasAttribute( QStringLiteral( "maxFeatures" ) ) )
    {
      request.maxFeatures = docElem.attribute( QStringLiteral( "maxFeatures" ) ).toLong();
    }
    if ( docElem.hasAttribute( QStringLiteral( "startIndex" ) ) )
    {
      request.startIndex = docElem.attribute( QStringLiteral( "startIndex" ) ).toLong();
    }
    if ( docElem.hasAttribute( QStringLiteral( "outputFormat" ) ) )
    {
      request.outputFormat = docElem.attribute( QStringLiteral( "outputFormat" ) );
    }

    QDomNodeList queryNodes = docElem.elementsByTagName( QStringLiteral( "Query" ) );
    QDomElement queryElem;
    for ( int i = 0; i < queryNodes.size(); i++ )
    {
      queryElem = queryNodes.at( i ).toElement();
      getFeatureQuery query = parseQueryElement( queryElem );
      request.queries.append( query );
    }
    return request;
  }

  getFeatureQuery parseQueryElement( QDomElement &queryElem )
  {
    QString typeName = queryElem.attribute( QStringLiteral( "typeName" ), QLatin1String( "" ) );
    if ( typeName.contains( ':' ) )
    {
      typeName = typeName.section( ':', 1, 1 );
    }

    QgsFeatureRequest featureRequest;
    QStringList propertyList;
    QDomNodeList queryChildNodes = queryElem.childNodes();
    if ( queryChildNodes.size() )
    {
      for ( int q = 0; q < queryChildNodes.size(); q++ )
      {
        QDomElement queryChildElem = queryChildNodes.at( q ).toElement();
        if ( queryChildElem.tagName() == QLatin1String( "PropertyName" ) )
        {
          QString fieldName = queryChildElem.text().trimmed();
          if ( fieldName.contains( ':' ) )
          {
            fieldName = fieldName.section( ':', 1, 1 );
          }
          if ( fieldName.contains( '/' ) )
          {
            if ( fieldName.section( '/', 0, 0 ) != typeName )
            {
              throw QgsRequestNotWellFormedException( QStringLiteral( "PropertyName text '%1' has to contain TypeName '%2'" ).arg( fieldName ).arg( typeName ) );
            }
            fieldName = fieldName.section( '/', 1, 1 );
          }
          propertyList.append( fieldName );
        }
        else if ( queryChildElem.tagName() == QLatin1String( "Filter" ) )
        {
          featureRequest = parseFilterElement( typeName, queryChildElem );
        }
      }
    }

    getFeatureQuery query;
    query.typeName = typeName;
    query.featureRequest = featureRequest;
    query.propertyList = propertyList;
    return query;
  }

  namespace
  {

    void startGetFeature( const QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project, const QString &format,
                          int prec, QgsCoordinateReferenceSystem &crs, QgsRectangle *rect, const QStringList &typeNames )
    {
      QString fcString;

      std::unique_ptr< QgsRectangle > transformedRect;

      if ( format == QLatin1String( "GeoJSON" ) )
      {
        response.setHeader( "Content-Type", "application/json; charset=utf-8" );

        if ( crs.isValid() )
        {
          QgsGeometry exportGeom = QgsGeometry::fromRect( *rect );
          QgsCoordinateTransform transform;
          transform.setSourceCrs( crs );
          transform.setDestinationCrs( QgsCoordinateReferenceSystem( 4326, QgsCoordinateReferenceSystem::EpsgCrsId ) );
          try
          {
            if ( exportGeom.transform( transform ) == 0 )
            {
              transformedRect.reset( new QgsRectangle( exportGeom.boundingBox() ) );
              rect = transformedRect.get();
            }
          }
          catch ( QgsException &cse )
          {
            Q_UNUSED( cse );
          }
        }
        fcString = QStringLiteral( "{\"type\": \"FeatureCollection\",\n" );
        fcString += " \"bbox\": [ " + qgsDoubleToString( rect->xMinimum(), prec ) + ", " + qgsDoubleToString( rect->yMinimum(), prec ) + ", " + qgsDoubleToString( rect->xMaximum(), prec ) + ", " + qgsDoubleToString( rect->yMaximum(), prec ) + "],\n";
        fcString += QLatin1String( " \"features\": [\n" );
        response.write( fcString.toUtf8() );
      }
      else
      {
        response.setHeader( "Content-Type", "text/xml; charset=utf-8" );

        //Prepare url
        QString hrefString = serviceUrl( request, project );

        QUrl mapUrl( hrefString );

        QUrlQuery query( mapUrl );
        query.addQueryItem( QStringLiteral( "SERVICE" ), QStringLiteral( "WFS" ) );
        query.addQueryItem( QStringLiteral( "VERSION" ), implementationVersion() );

        query.removeAllQueryItems( QStringLiteral( "REQUEST" ) );
        query.removeAllQueryItems( QStringLiteral( "FORMAT" ) );
        query.removeAllQueryItems( QStringLiteral( "OUTPUTFORMAT" ) );
        query.removeAllQueryItems( QStringLiteral( "BBOX" ) );
        query.removeAllQueryItems( QStringLiteral( "FEATUREID" ) );
        query.removeAllQueryItems( QStringLiteral( "TYPENAME" ) );
        query.removeAllQueryItems( QStringLiteral( "FILTER" ) );
        query.removeAllQueryItems( QStringLiteral( "EXP_FILTER" ) );
        query.removeAllQueryItems( QStringLiteral( "MAXFEATURES" ) );
        query.removeAllQueryItems( QStringLiteral( "STARTINDEX" ) );
        query.removeAllQueryItems( QStringLiteral( "PROPERTYNAME" ) );
        query.removeAllQueryItems( QStringLiteral( "_DC" ) );

        query.addQueryItem( QStringLiteral( "REQUEST" ), QStringLiteral( "DescribeFeatureType" ) );
        query.addQueryItem( QStringLiteral( "TYPENAME" ), typeNames.join( ',' ) );
        query.addQueryItem( QStringLiteral( "OUTPUTFORMAT" ), QStringLiteral( "XMLSCHEMA" ) );

        mapUrl.setQuery( query );

        hrefString = mapUrl.toString();

        //wfs:FeatureCollection valid
        fcString = QStringLiteral( "<wfs:FeatureCollection" );
        fcString += " xmlns:wfs=\"" + WFS_NAMESPACE + "\"";
        fcString += " xmlns:ogc=\"" + OGC_NAMESPACE + "\"";
        fcString += " xmlns:gml=\"" + GML_NAMESPACE + "\"";
        fcString += QLatin1String( " xmlns:ows=\"http://www.opengis.net/ows\"" );
        fcString += QLatin1String( " xmlns:xlink=\"http://www.w3.org/1999/xlink\"" );
        fcString += " xmlns:qgs=\"" + QGS_NAMESPACE + "\"";
        fcString += QLatin1String( " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" );
        fcString += " xsi:schemaLocation=\"" + WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.0.0/wfs.xsd " + QGS_NAMESPACE + " " + hrefString.replace( QLatin1String( "&" ), QLatin1String( "&amp;" ) ) + "\"";
        fcString += QLatin1String( ">" );

        response.write( fcString.toUtf8() );
        response.flush();

        QDomDocument doc;
        QDomElement bbElem = doc.createElement( QStringLiteral( "gml:boundedBy" ) );
        if ( format == QLatin1String( "GML3" ) )
        {
          QDomElement envElem = QgsOgcUtils::rectangleToGMLEnvelope( rect, doc, prec );
          if ( !envElem.isNull() )
          {
            if ( crs.isValid() )
            {
              envElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
            }
            bbElem.appendChild( envElem );
            doc.appendChild( bbElem );
          }
        }
        else
        {
          QDomElement boxElem = QgsOgcUtils::rectangleToGMLBox( rect, doc, prec );
          if ( !boxElem.isNull() )
          {
            if ( crs.isValid() )
            {
              boxElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
            }
            bbElem.appendChild( boxElem );
            doc.appendChild( bbElem );
          }
        }
        response.write( doc.toByteArray() );
        response.flush();
      }
    }

    void setGetFeature( QgsServerResponse &response, const QString &format, QgsFeature *feat, int featIdx, int prec,
                        QgsCoordinateReferenceSystem &crs, const QgsAttributeList &attrIndexes, const QSet<QString> &excludedAttributes,
                        const QString &typeName, bool withGeom, const QString &geometryName )
    {
      if ( !feat->isValid() )
        return;

      if ( format == QLatin1String( "GeoJSON" ) )
      {
        QString fcString;
        if ( featIdx == 0 )
          fcString += QLatin1String( "  " );
        else
          fcString += QLatin1String( " ," );
        fcString += createFeatureGeoJSON( feat, prec, crs, attrIndexes, excludedAttributes, typeName, withGeom, geometryName );
        fcString += QLatin1String( "\n" );

        response.write( fcString.toUtf8() );
      }
      else
      {
        QDomDocument gmlDoc;
        QDomElement featureElement;
        if ( format == QLatin1String( "GML3" ) )
        {
          featureElement = createFeatureGML3( feat, gmlDoc, prec, crs, attrIndexes, excludedAttributes, typeName, withGeom, geometryName );
          gmlDoc.appendChild( featureElement );
        }
        else
        {
          featureElement = createFeatureGML2( feat, gmlDoc, prec, crs, attrIndexes, excludedAttributes, typeName, withGeom, geometryName );
          gmlDoc.appendChild( featureElement );
        }
        response.write( gmlDoc.toByteArray() );
      }

      // Stream partial content
      response.flush();
    }

    void endGetFeature( QgsServerResponse &response, const QString &format )
    {
      QString fcString;
      if ( format == QLatin1String( "GeoJSON" ) )
      {
        fcString += QLatin1String( " ]\n" );
        fcString += QLatin1String( "}" );
      }
      else
      {
        fcString = QStringLiteral( "</wfs:FeatureCollection>\n" );
      }
      response.write( fcString.toUtf8() );
    }


    QString createFeatureGeoJSON( QgsFeature *feat, int prec, QgsCoordinateReferenceSystem &crs, const QgsAttributeList &attrIndexes, const QSet<QString> &excludedAttributes, const QString &typeName, bool withGeom, const QString &geometryName )
    {
      QString id = QStringLiteral( "%1.%2" ).arg( typeName, FID_TO_STRING( feat->id() ) );

      QgsJsonExporter exporter;
      exporter.setSourceCrs( crs );
      //QgsJsonExporter force transform geometry to ESPG:4326
      //and the RFC 7946 GeoJSON specification recommends limiting coordinate precision to 6
      Q_UNUSED( prec );
      //exporter.setPrecision( prec );

      //copy feature so we can modify its geometry as required
      QgsFeature f( *feat );
      QgsGeometry geom = feat->geometry();
      exporter.setIncludeGeometry( false );
      if ( !geom.isNull() && withGeom && geometryName != QLatin1String( "NONE" ) )
      {
        exporter.setIncludeGeometry( true );
        if ( geometryName == QLatin1String( "EXTENT" ) )
        {
          QgsRectangle box = geom.boundingBox();
          f.setGeometry( QgsGeometry::fromRect( box ) );
        }
        else if ( geometryName == QLatin1String( "CENTROID" ) )
        {
          f.setGeometry( geom.centroid() );
        }
      }

      QgsFields fields = feat->fields();
      QgsAttributeList attrsToExport;
      for ( int i = 0; i < attrIndexes.count(); ++i )
      {
        int idx = attrIndexes[i];
        if ( idx >= fields.count() )
        {
          continue;
        }
        QString attributeName = fields.at( idx ).name();
        //skip attribute if it is excluded from WFS publication
        if ( excludedAttributes.contains( attributeName ) )
        {
          continue;
        }

        attrsToExport << idx;
      }

      exporter.setIncludeAttributes( !attrsToExport.isEmpty() );
      exporter.setAttributes( attrsToExport );

      return exporter.exportFeature( f, QVariantMap(), id );
    }


    QDomElement createFeatureGML2( QgsFeature *feat, QDomDocument &doc, int prec, QgsCoordinateReferenceSystem &crs, const QgsAttributeList &attrIndexes, const QSet<QString> &excludedAttributes, const QString &typeName, bool withGeom, const QString &geometryName )
    {
      //gml:FeatureMember
      QDomElement featureElement = doc.createElement( QStringLiteral( "gml:featureMember" )/*wfs:FeatureMember*/ );

      //qgs:%TYPENAME%
      QDomElement typeNameElement = doc.createElement( "qgs:" + typeName /*qgs:%TYPENAME%*/ );
      typeNameElement.setAttribute( QStringLiteral( "fid" ), typeName + "." + QString::number( feat->id() ) );
      featureElement.appendChild( typeNameElement );

      if ( withGeom && geometryName != QLatin1String( "NONE" ) )
      {
        //add geometry column (as gml)
        QgsGeometry geom = feat->geometry();

        QDomElement geomElem = doc.createElement( QStringLiteral( "qgs:geometry" ) );
        QDomElement gmlElem;
        if ( geometryName == QLatin1String( "EXTENT" ) )
        {
          QgsGeometry bbox = QgsGeometry::fromRect( geom.boundingBox() );
          gmlElem = QgsOgcUtils::geometryToGML( bbox, doc, prec );
        }
        else if ( geometryName == QLatin1String( "CENTROID" ) )
        {
          QgsGeometry centroid = geom.centroid();
          gmlElem = QgsOgcUtils::geometryToGML( centroid, doc, prec );
        }
        else
        {
          QgsAbstractGeometry *abstractGeom = geom.geometry();
          if ( abstractGeom )
          {
            gmlElem = abstractGeom->asGML2( doc, prec, "http://www.opengis.net/gml" );
          }
        }

        if ( !gmlElem.isNull() )
        {
          QgsRectangle box = geom.boundingBox();
          QDomElement bbElem = doc.createElement( QStringLiteral( "gml:boundedBy" ) );
          QDomElement boxElem = QgsOgcUtils::rectangleToGMLBox( &box, doc, prec );

          if ( crs.isValid() )
          {
            boxElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
            gmlElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
          }

          bbElem.appendChild( boxElem );
          typeNameElement.appendChild( bbElem );

          geomElem.appendChild( gmlElem );
          typeNameElement.appendChild( geomElem );
        }
      }

      //read all attribute values from the feature
      QgsAttributes featureAttributes = feat->attributes();
      QgsFields fields = feat->fields();
      for ( int i = 0; i < attrIndexes.count(); ++i )
      {
        int idx = attrIndexes[i];
        if ( idx >= fields.count() )
        {
          continue;
        }
        QString attributeName = fields.at( idx ).name();
        //skip attribute if it is excluded from WFS publication
        if ( excludedAttributes.contains( attributeName ) )
        {
          continue;
        }

        QDomElement fieldElem = doc.createElement( "qgs:" + attributeName.replace( QStringLiteral( " " ), QStringLiteral( "_" ) ) );
        QDomText fieldText = doc.createTextNode( featureAttributes[idx].toString() );
        fieldElem.appendChild( fieldText );
        typeNameElement.appendChild( fieldElem );
      }

      return featureElement;
    }

    QDomElement createFeatureGML3( QgsFeature *feat, QDomDocument &doc, int prec, QgsCoordinateReferenceSystem &crs, const QgsAttributeList &attrIndexes, const QSet<QString> &excludedAttributes, const QString &typeName, bool withGeom, const QString geometryName )
    {
      //gml:FeatureMember
      QDomElement featureElement = doc.createElement( QStringLiteral( "gml:featureMember" )/*wfs:FeatureMember*/ );

      //qgs:%TYPENAME%
      QDomElement typeNameElement = doc.createElement( "qgs:" + typeName /*qgs:%TYPENAME%*/ );
      typeNameElement.setAttribute( QStringLiteral( "gml:id" ), typeName + "." + QString::number( feat->id() ) );
      featureElement.appendChild( typeNameElement );

      if ( withGeom && geometryName != QLatin1String( "NONE" ) )
      {
        //add geometry column (as gml)
        QgsGeometry geom = feat->geometry();

        QDomElement geomElem = doc.createElement( QStringLiteral( "qgs:geometry" ) );
        QDomElement gmlElem;
        if ( geometryName == QLatin1String( "EXTENT" ) )
        {
          QgsGeometry bbox = QgsGeometry::fromRect( geom.boundingBox() );
          gmlElem = QgsOgcUtils::geometryToGML( bbox, doc, QStringLiteral( "GML3" ), prec );
        }
        else if ( geometryName == QLatin1String( "CENTROID" ) )
        {
          QgsGeometry centroid = geom.centroid();
          gmlElem = QgsOgcUtils::geometryToGML( centroid, doc, QStringLiteral( "GML3" ), prec );
        }
        else
        {
          QgsAbstractGeometry *abstractGeom = geom.geometry();
          if ( abstractGeom )
          {
            gmlElem = abstractGeom->asGML3( doc, prec, "http://www.opengis.net/gml" );
          }
        }

        if ( !gmlElem.isNull() )
        {
          QgsRectangle box = geom.boundingBox();
          QDomElement bbElem = doc.createElement( QStringLiteral( "gml:boundedBy" ) );
          QDomElement boxElem = QgsOgcUtils::rectangleToGMLEnvelope( &box, doc, prec );

          if ( crs.isValid() )
          {
            boxElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
            gmlElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
          }

          bbElem.appendChild( boxElem );
          typeNameElement.appendChild( bbElem );

          geomElem.appendChild( gmlElem );
          typeNameElement.appendChild( geomElem );
        }
      }

      //read all attribute values from the feature
      QgsAttributes featureAttributes = feat->attributes();
      QgsFields fields = feat->fields();
      for ( int i = 0; i < attrIndexes.count(); ++i )
      {
        int idx = attrIndexes[i];
        if ( idx >= fields.count() )
        {
          continue;
        }
        QString attributeName = fields.at( idx ).name();
        //skip attribute if it is excluded from WFS publication
        if ( excludedAttributes.contains( attributeName ) )
        {
          continue;
        }

        QDomElement fieldElem = doc.createElement( "qgs:" + attributeName.replace( QStringLiteral( " " ), QStringLiteral( "_" ) ) );
        QDomText fieldText = doc.createTextNode( featureAttributes[idx].toString() );
        fieldElem.appendChild( fieldText );
        typeNameElement.appendChild( fieldElem );
      }

      return featureElement;
    }




  } // namespace

} // samespace QgsWfs



