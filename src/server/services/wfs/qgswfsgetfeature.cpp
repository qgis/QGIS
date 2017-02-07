/***************************************************************************
                              qgswfsgetfeature.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
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

    QString createFeatureGeoJSON( QgsFeature* feat, int prec, QgsCoordinateReferenceSystem& crs, const QgsAttributeList& attrIndexes,
                                  const QSet<QString>& excludedAttributes, const QString& typeName, bool withGeom,
                                  const QString& geometryName );

    QDomElement createFeatureGML2( QgsFeature* feat, QDomDocument& doc, int prec, QgsCoordinateReferenceSystem& crs,
                                   const QgsAttributeList& attrIndexes, const QSet<QString>& excludedAttributes, const QString& typeName,
                                   bool withGeom, const QString& geometryName );

    QDomElement createFeatureGML3( QgsFeature* feat, QDomDocument& doc, int prec, QgsCoordinateReferenceSystem& crs,
                                   const QgsAttributeList& attrIndexes, const QSet<QString>& excludedAttributes, const QString& typeName,
                                   bool withGeom, const QString geometryName );

    void startGetFeature( const QgsServerRequest& request, QgsServerResponse& response, const QgsProject* project, const QString& format,
                          int prec, QgsCoordinateReferenceSystem& crs, QgsRectangle* rect, const QStringList& typeNames );

    void setGetFeature( QgsServerResponse& response, const QString& format, QgsFeature* feat, int featIdx, int prec,
                        QgsCoordinateReferenceSystem& crs, const QgsAttributeList& attrIndexes, const QSet<QString>& excludedAttributes,
                        const QString& typeName, bool withGeom, const QString& geometryName );

    void endGetFeature( QgsServerResponse& response, const QString& format );

  }

  /** Output WFS  GetCapabilities response
   */
  void writeGetFeature( QgsServerInterface* serverIface, const QgsProject* project,
                        const QString& version, const QgsServerRequest& request,
                        QgsServerResponse& response )
  {
    Q_UNUSED( version );

    QgsWfsProjectParser* configParser = getConfigParser( serverIface );

    QgsServerRequest::Parameters parameters = request.parameters();
    QgsAccessControl* accessControl = serverIface->accessControls();

    QStringList wfsLayersId = configParser->wfsLayers();

    QList<QgsMapLayer*> layerList;
    QgsMapLayer* currentLayer = nullptr;
    QgsCoordinateReferenceSystem layerCrs;
    QgsRectangle searchRect( 0, 0, 0, 0 );

    QStringList errors;
    QStringList typeNames;
    QString typeName;
    QString propertyName;
    QString geometryName;

    QString format = parameters.value( QStringLiteral( "OUTPUTFORMAT" ) );

    bool withGeom = true;

    long maxFeatures = 0;
    bool hasFeatureLimit = false;
    long startIndex = 0;
    long featureCounter = 0;
    int layerPrec = 8;
    long featCounter = 0;

    QgsExpressionContext expressionContext;
    expressionContext << QgsExpressionContextUtils::globalScope()
    << QgsExpressionContextUtils::projectScope( QgsProject::instance() );

    QDomDocument doc;
    QString errorMsg;

    //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
    //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
    std::unique_ptr< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer( accessControl ) );

    if ( doc.setContent( parameters.value( QStringLiteral( "REQUEST_BODY" ) ), true, &errorMsg ) )
    {
      QDomElement docElem = doc.documentElement();
      if ( docElem.hasAttribute( QStringLiteral( "maxFeatures" ) ) )
      {
        hasFeatureLimit = true;
        maxFeatures = docElem.attribute( QStringLiteral( "maxFeatures" ) ).toLong();
      }
      if ( docElem.hasAttribute( QStringLiteral( "startIndex" ) ) )
      {
        startIndex = docElem.attribute( QStringLiteral( "startIndex" ) ).toLong();
      }

      QDomNodeList queryNodes = docElem.elementsByTagName( QStringLiteral( "Query" ) );
      QDomElement queryElem;
      for ( int i = 0; i < queryNodes.size(); i++ )
      {
        queryElem = queryNodes.at( 0 ).toElement();
        typeName = queryElem.attribute( QStringLiteral( "typeName" ), QLatin1String( "" ) );
        if ( typeName.contains( QLatin1String( ":" ) ) )
        {
          typeName = typeName.section( QStringLiteral( ":" ), 1, 1 );
        }
        typeNames << typeName;
      }
      for ( int i = 0; i < queryNodes.size(); i++ )
      {
        queryElem = queryNodes.at( 0 ).toElement();
        typeName = queryElem.attribute( QStringLiteral( "typeName" ), QLatin1String( "" ) );
        if ( typeName.contains( QLatin1String( ":" ) ) )
        {
          typeName = typeName.section( QStringLiteral( ":" ), 1, 1 );
        }

        layerList = configParser->mapLayerFromTypeName( typeName );
        if ( layerList.size() < 1 )
        {
          errors << QStringLiteral( "The layer for the TypeName '%1' is not found" ).arg( typeName );
          continue;
        }

        currentLayer = layerList.at( 0 );
        QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( currentLayer );
        if ( layer && wfsLayersId.contains( layer->id() ) )
        {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
          if ( !accessControl->layerReadPermission( currentLayer ) )
          {
            throw QgsSecurityAccessException( QStringLiteral( "Feature access permission denied" ) );
          }
          QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, currentLayer, filterRestorer->originalFilters() );
#endif
          expressionContext << QgsExpressionContextUtils::layerScope( layer );

          //is there alias info for this vector layer?
          QMap< int, QString > layerAliasInfo;
          QgsStringMap aliasMap = layer->attributeAliases();
          QgsStringMap::const_iterator aliasIt = aliasMap.constBegin();
          for ( ; aliasIt != aliasMap.constEnd(); ++aliasIt )
          {
            int attrIndex = layer->fields().lookupField( aliasIt.key() );
            if ( attrIndex != -1 )
            {
              layerAliasInfo.insert( attrIndex, aliasIt.value() );
            }
          }

          //excluded attributes for this layer
          const QSet<QString>& layerExcludedAttributes = layer->excludeAttributesWfs();

          //get layer precision
          layerPrec = configParser->wfsLayerPrecision( layer->id() );

          //do a select with searchRect and go through all the features
          QgsVectorDataProvider* provider = layer->dataProvider();
          if ( !provider )
          {
            errors << QStringLiteral( "The layer's provider for the TypeName '%1' is not found" ).arg( typeName );
            continue;
          }

          QgsFeature feature;

          withGeom = true;

          //Using pending attributes and pending fields
          QgsAttributeList attrIndexes = layer->pendingAllAttributesList();

          QDomNodeList queryChildNodes = queryElem.childNodes();
          if ( queryChildNodes.size() )
          {
            QStringList::const_iterator alstIt;
            QList<int> idxList;
            QgsFields fields = layer->pendingFields();
            QString fieldName;
            QDomElement propertyElem;
            for ( int q = 0; q < queryChildNodes.size(); q++ )
            {
              QDomElement queryChildElem = queryChildNodes.at( q ).toElement();
              if ( queryChildElem.tagName() == QLatin1String( "PropertyName" ) )
              {
                fieldName = queryChildElem.text();
                if ( fieldName.contains( QLatin1String( ":" ) ) )
                {
                  fieldName = fieldName.section( QStringLiteral( ":" ), 1, 1 );
                }
                int fieldNameIdx = fields.lookupField( fieldName );
                if ( fieldNameIdx > -1 )
                {
                  idxList.append( fieldNameIdx );
                }
              }
            }
            if ( !idxList.isEmpty() )
            {
              attrIndexes = idxList;
            }
          }

          //map extent
          searchRect = layer->extent();
          searchRect.set( searchRect.xMinimum() - 1. / pow( 10., layerPrec )
                          , searchRect.yMinimum() - 1. / pow( 10., layerPrec )
                          , searchRect.xMaximum() + 1. / pow( 10., layerPrec )
                          , searchRect.yMaximum() + 1. / pow( 10., layerPrec ) );
          layerCrs = layer->crs();

          QgsFeatureRequest fReq;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
          fReq.setFlags( QgsFeatureRequest::ExactIntersect | ( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
          accessControl->filterFeatures( layer, fReq );

          QStringList attributes = QStringList();
          Q_FOREACH ( int idx, attrIndexes )
          {
            attributes.append( layer->pendingFields().field( idx ).name() );
          }
          fReq.setSubsetOfAttributes(
            accessControl->layerAttributes( layer, attributes ),
            layer->pendingFields() );
#endif

          QgsFeatureIterator fit = layer->getFeatures( fReq );

          QDomNodeList filterNodes = queryElem.elementsByTagName( QStringLiteral( "Filter" ) );
          if ( !filterNodes.isEmpty() )
          {
            QDomElement filterElem = filterNodes.at( 0 ).toElement();
            QDomNodeList fidNodes = filterElem.elementsByTagName( QStringLiteral( "FeatureId" ) );
            if ( !fidNodes.isEmpty() )
            {
              QDomElement fidElem;
              QString fid = QLatin1String( "" );
              for ( int f = 0; f < fidNodes.size(); f++ )
              {
                fidElem = fidNodes.at( f ).toElement();
                fid = fidElem.attribute( QStringLiteral( "fid" ) );
                if ( fid.contains( QLatin1String( "." ) ) )
                {
                  if ( fid.section( QStringLiteral( "." ), 0, 0 ) != typeName )
                    continue;
                  fid = fid.section( QStringLiteral( "." ), 1, 1 );
                }

                //Need to be test for propertyname
                layer->getFeatures( QgsFeatureRequest()
                                    .setFilterFid( fid.toInt() )
                                    .setFlags( QgsFeatureRequest::ExactIntersect | ( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) )
                                    .setSubsetOfAttributes( attrIndexes )
                                  ).nextFeature( feature );

                if ( featureCounter == 0 )
                  startGetFeature( request, response, project, format, layerPrec, layerCrs, &searchRect, typeNames );

                setGetFeature( response, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes,
                               typeName, withGeom, geometryName );

                fid = QLatin1String( "" );
                ++featCounter;
                ++featureCounter;
              }
            }
            else if ( filterElem.firstChildElement().tagName() == QLatin1String( "BBOX" ) )
            {
              QDomElement bboxElem = filterElem.firstChildElement();
              QDomElement childElem = bboxElem.firstChildElement();

              QgsFeatureRequest req;
              req.setFlags( QgsFeatureRequest::ExactIntersect | ( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );

              while ( !childElem.isNull() )
              {
                if ( childElem.tagName() == QLatin1String( "Box" ) )
                {
                  req.setFilterRect( QgsOgcUtils::rectangleFromGMLBox( childElem ) );
                }
                else if ( childElem.tagName() != QLatin1String( "PropertyName" ) )
                {
                  QgsGeometry geom = QgsOgcUtils::geometryFromGML( childElem );
                  req.setFilterRect( geom.boundingBox() );
                }
                childElem = childElem.nextSiblingElement();
              }
              req.setSubsetOfAttributes( attrIndexes );

              QgsFeatureIterator fit = layer->getFeatures( req );
              while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
              {
                if ( featureCounter == startIndex )
                  startGetFeature( request, response, project, format, layerPrec, layerCrs, &searchRect, typeNames );

                if ( featureCounter >= startIndex )
                {
                  setGetFeature( response, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes,
                                 typeName, withGeom, geometryName );
                  ++featCounter;
                }
                ++featureCounter;
              }
            }
            else
            {
              QSharedPointer<QgsExpression> filter( QgsOgcUtils::expressionFromOgcFilter( filterElem ) );
              if ( filter )
              {
                if ( filter->hasParserError() )
                {
                  throw QgsRequestNotWellFormedException( filter->parserErrorString() );
                }
                QgsFeatureRequest req;
                if ( filter->needsGeometry() )
                {
                  req.setFlags( QgsFeatureRequest::NoFlags );
                }
                else
                {
                  req.setFlags( QgsFeatureRequest::ExactIntersect | ( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
                }
                req.setFilterExpression( filter->expression() );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
                accessControl->filterFeatures( layer, req );

                QStringList attributes = QStringList();
                Q_FOREACH ( int idx, attrIndexes )
                {
                  attributes.append( layer->pendingFields().field( idx ).name() );
                }
                req.setSubsetOfAttributes(
                  accessControl->layerAttributes( layer, attributes ),
                  layer->pendingFields() );
#endif
                QgsFeatureIterator fit = layer->getFeatures( req );
                while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
                {
                  expressionContext.setFeature( feature );

                  QVariant res = filter->evaluate( &expressionContext );
                  if ( filter->hasEvalError() )
                  {
                    // XXX It is useless to throw an error at this point if startGetFeature has been called
                    // because request content has already been flushed to client
                    throw QgsRequestNotWellFormedException( filter->evalErrorString() );
                  }
                  if ( res.toInt() != 0 )
                  {
                    if ( featureCounter == startIndex )
                      startGetFeature( request, response, project, format, layerPrec, layerCrs, &searchRect, typeNames );

                    if ( featureCounter >= startIndex )
                    {
                      setGetFeature( response, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes,
                                     typeName, withGeom, geometryName );
                      ++featCounter;
                    }
                    ++featureCounter;
                  }
                }
              }
            }
          }
          else
          {
            while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
            {
              if ( featureCounter == startIndex )
                startGetFeature( request, response, project, format, layerPrec, layerCrs, &searchRect, typeNames );

              if ( featureCounter >= startIndex )
              {
                setGetFeature( response, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes,
                               typeName, withGeom, geometryName );
                ++featCounter;
              }
              ++featureCounter;
            }
          }
        }
        else
        {
          errors << QStringLiteral( "The layer for the TypeName '%1' is not a WFS layer" ).arg( typeName );
        }

      }

      //force restoration of original layer filters
      filterRestorer.reset();

      QgsMessageLog::logMessage( errors.join( QStringLiteral( "\n" ) ) );

      QgsProject::instance()->removeAllMapLayers();
      if ( featureCounter <= startIndex )
        startGetFeature( request, response, project, format, layerPrec, layerCrs, &searchRect, typeNames );
      endGetFeature( response, format );
      return;
    }

    // Information about parameters
    // FILTER
    bool filterOk = false;
    QDomDocument filter;
    // EXP_FILTER
    bool expFilterOk = false;
    QString expFilter;
    // BBOX
    bool bboxOk = false;
    double minx = 0.0, miny = 0.0, maxx = 0.0, maxy = 0.0;

    //read FEATUREDID
    bool featureIdOk = false;
    QStringList featureIdList;
    QMap<QString, QString>::const_iterator feature_id_it = parameters.constFind( QStringLiteral( "FEATUREID" ) );
    if ( feature_id_it != parameters.constEnd() )
    {
      featureIdOk = true;
      featureIdList = feature_id_it.value().split( QStringLiteral( "," ) );
      QStringList typeNameList;
      Q_FOREACH ( const QString &fidStr, featureIdList )
      {
        // testing typename in the WFS featureID
        if ( !fidStr.contains( QLatin1String( "." ) ) )
          throw QgsRequestNotWellFormedException( QStringLiteral( "FEATUREID has to have  TYPENAME in the values" ) );

        QString typeName = fidStr.section( QStringLiteral( "." ), 0, 0 );
        if ( !typeNameList.contains( typeName ) )
          typeNameList << typeName;
      }

      typeName = typeNameList.join( QStringLiteral( "," ) );
    }

    if ( !featureIdOk )
    {
      //read TYPENAME
      QMap<QString, QString>::const_iterator type_name_it = parameters.constFind( QStringLiteral( "TYPENAME" ) );
      if ( type_name_it != parameters.constEnd() )
      {
        typeName = type_name_it.value();
      }
      else
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "TYPENAME is MANDATORY" ) );
      }

      //read FILTER
      QMap<QString, QString>::const_iterator filterIt = parameters.constFind( QStringLiteral( "FILTER" ) );
      if ( filterIt != parameters.constEnd() )
      {
        QString errorMsg;
        if ( !filter.setContent( filterIt.value(), true, &errorMsg ) )
        {
          throw QgsRequestNotWellFormedException( QStringLiteral( "error message: %1. The XML string was: %2" ).arg( errorMsg, filterIt.value() ) );
        }
        else
        {
          filterOk = true;
        }
      }

      //read EXP_FILTER
      if ( !filterOk )
      {
        QMap<QString, QString>::const_iterator expFilterIt = parameters.constFind( QStringLiteral( "EXP_FILTER" ) );
        if ( expFilterIt != parameters.constEnd() )
        {
          expFilterOk = true;
          expFilter = expFilterIt.value();
        }
      }

      //read BBOX
      if ( !filterOk )
      {
        QMap<QString, QString>::const_iterator bbIt = parameters.constFind( QStringLiteral( "BBOX" ) );
        if ( bbIt == parameters.constEnd() )
        {
          minx = 0;
          miny = 0;
          maxx = 0;
          maxy = 0;
        }
        else
        {
          bool conversionSuccess;
          bboxOk = true;
          QString bbString = bbIt.value();
          minx = bbString.section( QStringLiteral( "," ), 0, 0 ).toDouble( &conversionSuccess );
          bboxOk &= conversionSuccess;
          miny = bbString.section( QStringLiteral( "," ), 1, 1 ).toDouble( &conversionSuccess );
          bboxOk &= conversionSuccess;
          maxx = bbString.section( QStringLiteral( "," ), 2, 2 ).toDouble( &conversionSuccess );
          bboxOk &= conversionSuccess;
          maxy = bbString.section( QStringLiteral( "," ), 3, 3 ).toDouble( &conversionSuccess );
          bboxOk &= conversionSuccess;
        }
      }
    }

    //read MAXFEATURES
    QMap<QString, QString>::const_iterator mfIt = parameters.constFind( QStringLiteral( "MAXFEATURES" ) );
    if ( mfIt != parameters.constEnd() )
    {
      QString mfString = mfIt.value();
      bool mfOk;
      hasFeatureLimit = true;
      maxFeatures = mfString.toLong( &mfOk, 10 );
    }

    //read STARTINDEX
    QMap<QString, QString>::const_iterator siIt = parameters.constFind( QStringLiteral( "STARTINDEX" ) );
    if ( siIt != parameters.constEnd() )
    {
      QString siString = siIt.value();
      bool siOk;
      startIndex = siString.toLong( &siOk, 10 );
    }

    //read PROPERTYNAME
    withGeom = true;
    propertyName = QStringLiteral( "*" );
    QMap<QString, QString>::const_iterator pnIt = parameters.constFind( QStringLiteral( "PROPERTYNAME" ) );
    if ( pnIt != parameters.constEnd() )
    {
      propertyName = pnIt.value();
    }
    geometryName = QLatin1String( "" );
    QMap<QString, QString>::const_iterator gnIt = parameters.constFind( QStringLiteral( "GEOMETRYNAME" ) );
    if ( gnIt != parameters.constEnd() )
    {
      geometryName = gnIt.value().toUpper();
    }

    typeNames = typeName.split( QStringLiteral( "," ) );
    Q_FOREACH ( const QString &tnStr, typeNames )
    {
      typeName = tnStr;
      layerList = configParser->mapLayerFromTypeName( tnStr );
      if ( layerList.size() < 1 )
      {
        errors << QStringLiteral( "The layer for the TypeName '%1' is not found" ).arg( tnStr );
        continue;
      }

      currentLayer = layerList.at( 0 );

      QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( currentLayer );
      if ( layer && wfsLayersId.contains( layer->id() ) )
      {
        expressionContext << QgsExpressionContextUtils::layerScope( layer );

        //is there alias info for this vector layer?
        QMap< int, QString > layerAliasInfo;
        QgsStringMap aliasMap = layer->attributeAliases();
        QgsStringMap::const_iterator aliasIt = aliasMap.constBegin();
        for ( ; aliasIt != aliasMap.constEnd(); ++aliasIt )
        {
          int attrIndex = layer->fields().lookupField( aliasIt.key() );
          if ( attrIndex != -1 )
          {
            layerAliasInfo.insert( attrIndex, aliasIt.value() );
          }
        }

        //excluded attributes for this layer
        const QSet<QString>& layerExcludedAttributes = layer->excludeAttributesWfs();

        //get layer precision
        int layerPrec = configParser->wfsLayerPrecision( layer->id() );

        //do a select with searchRect and go through all the features
        QgsVectorDataProvider* provider = layer->dataProvider();
        if ( !provider )
        {
          errors << QStringLiteral( "The layer's provider for the TypeName '%1' is not found" ).arg( tnStr );
          continue;
        }

        QgsFeature feature;

        //map extent
        searchRect = layer->extent();

        //Using pending attributes and pending fields
        QgsAttributeList attrIndexes = layer->pendingAllAttributesList();
        if ( propertyName != QLatin1String( "*" ) )
        {
          QStringList attrList = propertyName.split( QStringLiteral( "," ) );
          if ( !attrList.isEmpty() )
          {
            QStringList::const_iterator alstIt;
            QList<int> idxList;
            QgsFields fields = layer->pendingFields();
            QString fieldName;
            for ( alstIt = attrList.begin(); alstIt != attrList.end(); ++alstIt )
            {
              fieldName = *alstIt;
              int fieldNameIdx = fields.lookupField( fieldName );
              if ( fieldNameIdx > -1 )
              {
                idxList.append( fieldNameIdx );
              }
            }
            if ( !idxList.isEmpty() )
            {
              attrIndexes = idxList;
            }
          }
        }

        if ( bboxOk )
          searchRect.set( minx, miny, maxx, maxy );
        else
          searchRect.set( searchRect.xMinimum() - 1. / pow( 10., layerPrec ),
                          searchRect.yMinimum() - 1. / pow( 10., layerPrec ),
                          searchRect.xMaximum() + 1. / pow( 10., layerPrec ),
                          searchRect.yMaximum() + 1. / pow( 10., layerPrec ) );
        layerCrs = layer->crs();

        if ( featureIdOk )
        {
          Q_FOREACH ( const QString &fidStr, featureIdList )
          {
            if ( !fidStr.startsWith( tnStr ) )
              continue;
            //Need to be test for propertyname
            layer->getFeatures( QgsFeatureRequest()
                                .setFilterFid( fidStr.section( QStringLiteral( "." ), 1, 1 ).toInt() )
                                .setFlags( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry )
                                .setSubsetOfAttributes( attrIndexes )
                              ).nextFeature( feature );

            if ( featureCounter == 0 )
              startGetFeature( request, response, project, format, layerPrec, layerCrs, &searchRect, typeNames );

            setGetFeature( response, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes,
                           typeName, withGeom, geometryName );
            ++featCounter;
            ++featureCounter;
          }
        }
        else if ( expFilterOk )
        {
          QgsFeatureRequest req;
          if ( layer->wkbType() != QgsWkbTypes::NoGeometry )
          {
            if ( bboxOk )
            {
              req.setFilterRect( searchRect );
              req.setFlags( QgsFeatureRequest::ExactIntersect | ( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
            }
            else
            {
              req.setFlags( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry );
            }
          }
          else
          {
            req.setFlags( QgsFeatureRequest::NoGeometry );
            withGeom = false;
          }
          req.setSubsetOfAttributes( attrIndexes );
          QgsFeatureIterator fit = layer->getFeatures( req );
          QSharedPointer<QgsExpression> filter( new QgsExpression( expFilter ) );
          if ( filter )
          {
            if ( filter->hasParserError() )
            {
              throw QgsRequestNotWellFormedException( QStringLiteral( "Expression filter error message: %1." ).arg( filter->parserErrorString() ) );
            }
            while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
            {
              expressionContext.setFeature( feature );
              QVariant res = filter->evaluate( &expressionContext );
              if ( filter->hasEvalError() )
              {
                throw QgsRequestNotWellFormedException( QStringLiteral( "Expression filter eval error message: %1." ).arg( filter->evalErrorString() ) );
              }
              if ( res.toInt() != 0 )
              {
                if ( featureCounter == startIndex )
                  startGetFeature( request, response, project, format, layerPrec, layerCrs, &searchRect, typeNames );

                if ( featureCounter >= startIndex )
                {
                  setGetFeature( response, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes,
                                 typeName, withGeom, geometryName );
                  ++featCounter;
                }
                ++featureCounter;
              }
            }
          }
        }
        else if ( filterOk )
        {
          QDomElement filterElem = filter.firstChildElement();
          QDomNodeList fidNodes = filterElem.elementsByTagName( QStringLiteral( "FeatureId" ) );
          if ( !fidNodes.isEmpty() )
          {
            QDomElement fidElem;
            QString fid = QLatin1String( "" );
            for ( int f = 0; f < fidNodes.size(); f++ )
            {
              fidElem = fidNodes.at( f ).toElement();
              fid = fidElem.attribute( QStringLiteral( "fid" ) );
              if ( fid.contains( QLatin1String( "." ) ) )
              {
                if ( fid.section( QStringLiteral( "." ), 0, 0 ) != typeName )
                  continue;
                fid = fid.section( QStringLiteral( "." ), 1, 1 );
              }

              //Need to be test for propertyname
              layer->getFeatures( QgsFeatureRequest()
                                  .setFilterFid( fid.toInt() )
                                  .setFlags( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry )
                                  .setSubsetOfAttributes( attrIndexes )
                                ).nextFeature( feature );

              if ( featureCounter == 0 )
                startGetFeature( request, response, project, format, layerPrec, layerCrs, &searchRect, typeNames );

              setGetFeature( response, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes,
                             typeName, withGeom, geometryName );

              fid = QLatin1String( "" );
              ++featCounter;
              ++featureCounter;
            }
          }
          else if ( filterElem.firstChildElement().tagName() == QLatin1String( "BBOX" ) )
          {
            QDomElement bboxElem = filterElem.firstChildElement();
            QDomElement childElem = bboxElem.firstChildElement();

            QgsFeatureRequest req;
            req.setFlags( QgsFeatureRequest::ExactIntersect | ( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );

            while ( !childElem.isNull() )
            {
              if ( childElem.tagName() == QLatin1String( "Box" ) )
              {
                req.setFilterRect( QgsOgcUtils::rectangleFromGMLBox( childElem ) );
              }
              else if ( childElem.tagName() != QLatin1String( "PropertyName" ) )
              {
                QgsGeometry geom = QgsOgcUtils::geometryFromGML( childElem );
                req.setFilterRect( geom.boundingBox() );
              }
              childElem = childElem.nextSiblingElement();
            }
            req.setSubsetOfAttributes( attrIndexes );

            QgsFeatureIterator fit = layer->getFeatures( req );
            while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
            {
              if ( featureCounter == startIndex )
                startGetFeature( request, response, project, format, layerPrec, layerCrs, &searchRect, typeNames );

              if ( featureCounter >= startIndex )
              {
                setGetFeature( response, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes,
                               typeName, withGeom, geometryName );
                ++featCounter;
              }
              ++featureCounter;
            }
          }
          else
          {
            QSharedPointer<QgsExpression> filter( QgsOgcUtils::expressionFromOgcFilter( filterElem ) );
            if ( filter )
            {
              if ( filter->hasParserError() )
              {
                throw QgsRequestNotWellFormedException( QStringLiteral( "OGC expression filter error message: %1." ).arg( filter->parserErrorString() ) );
              }
              QgsFeatureRequest req;
              if ( layer->wkbType() != QgsWkbTypes::NoGeometry )
              {
                if ( bboxOk )
                {
                  req.setFilterRect( searchRect ).setFlags( QgsFeatureRequest::ExactIntersect | ( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
                }
                else
                {
                  req.setFlags( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry );
                }
              }
              else
              {
                req.setFlags( QgsFeatureRequest::NoGeometry );
                withGeom = false;
              }
              req.setSubsetOfAttributes( attrIndexes );
              QgsFeatureIterator fit = layer->getFeatures( req );
              while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
              {
                expressionContext.setFeature( feature );
                QVariant res = filter->evaluate( &expressionContext );
                if ( filter->hasEvalError() )
                {
                  throw QgsRequestNotWellFormedException( QStringLiteral( "OGC expression filter eval error message: %1." ).arg( filter->evalErrorString() ) );
                }
                if ( res.toInt() != 0 )
                {
                  if ( featureCounter == startIndex )
                    startGetFeature( request, response, project, format, layerPrec, layerCrs, &searchRect, typeNames );

                  if ( featureCounter >= startIndex )
                  {
                    setGetFeature( response, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes,
                                   typeName, withGeom, geometryName );
                    ++featCounter;
                  }
                  ++featureCounter;
                }
              }
            }
          }
        }
        else
        {
          //throw QgsMapServiceException( "RequestNotWellFormed", QString( "attrIndexes length: %1." ).arg( attrIndexes.count() ) );
          QgsFeatureRequest req;
          if ( layer->wkbType() != QgsWkbTypes::NoGeometry )
          {
            if ( bboxOk )
            {
              req.setFilterRect( searchRect ).setFlags( QgsFeatureRequest::ExactIntersect | ( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
            }
            else
            {
              req.setFlags( withGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry );
            }
          }
          else
          {
            req.setFlags( QgsFeatureRequest::NoGeometry );
            withGeom = false;
          }
          req.setSubsetOfAttributes( attrIndexes );
          QgsFeatureIterator fit = layer->getFeatures( req );
          while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
          {
            errors << QStringLiteral( "The feature %2 of layer for the TypeName '%1'" ).arg( tnStr ).arg( featureCounter );
            if ( featureCounter == startIndex )
              startGetFeature( request, response, project, format, layerPrec, layerCrs, &searchRect, typeNames );

            if ( featureCounter >= startIndex )
            {
              setGetFeature( response, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes,
                             typeName, withGeom, geometryName );
              ++featCounter;
            }
            ++featureCounter;
          }
        }

      }
      else
      {
        errors << QStringLiteral( "The layer for the TypeName '%1' is not a WFS layer" ).arg( tnStr );
      }

    }

    QgsProject::instance()->removeAllMapLayers();
    if ( featureCounter <= startIndex )
      startGetFeature( request, response, project, format, layerPrec, layerCrs, &searchRect, typeNames );
    endGetFeature( response, format );

  }

  namespace
  {

    void startGetFeature( const QgsServerRequest& request, QgsServerResponse& response, const QgsProject* project, const QString& format,
                          int prec, QgsCoordinateReferenceSystem& crs, QgsRectangle* rect, const QStringList& typeNames )
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
        query.addQueryItem( QStringLiteral( "TYPENAME" ), typeNames.join( QStringLiteral( "," ) ) );
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

    void setGetFeature( QgsServerResponse& response, const QString& format, QgsFeature* feat, int featIdx, int prec,
                        QgsCoordinateReferenceSystem& crs, const QgsAttributeList& attrIndexes, const QSet<QString>& excludedAttributes,
                        const QString& typeName, bool withGeom, const QString& geometryName )
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

    void endGetFeature( QgsServerResponse& response, const QString& format )
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


    QString createFeatureGeoJSON( QgsFeature* feat, int prec, QgsCoordinateReferenceSystem& crs, const QgsAttributeList& attrIndexes, const QSet<QString>& excludedAttributes, const QString& typeName, bool withGeom, const QString& geometryName )
    {
      QString id = QStringLiteral( "%1.%2" ).arg( typeName, FID_TO_STRING( feat->id() ) );

      QgsJSONExporter exporter;
      exporter.setSourceCrs( crs );
      exporter.setPrecision( prec );

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


    QDomElement createFeatureGML2( QgsFeature* feat, QDomDocument& doc, int prec, QgsCoordinateReferenceSystem& crs, const QgsAttributeList& attrIndexes, const QSet<QString>& excludedAttributes, const QString& typeName, bool withGeom, const QString& geometryName )
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
          gmlElem = QgsOgcUtils::geometryToGML( &bbox , doc, prec );
        }
        else if ( geometryName == QLatin1String( "CENTROID" ) )
        {
          QgsGeometry centroid = geom.centroid();
          gmlElem = QgsOgcUtils::geometryToGML( &centroid, doc, prec );
        }
        else
          gmlElem = QgsOgcUtils::geometryToGML( &geom, doc, prec );
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

    QDomElement createFeatureGML3( QgsFeature* feat, QDomDocument& doc, int prec, QgsCoordinateReferenceSystem& crs, const QgsAttributeList& attrIndexes, const QSet<QString>& excludedAttributes, const QString& typeName, bool withGeom, const QString geometryName )
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
          gmlElem = QgsOgcUtils::geometryToGML( &bbox, doc, QStringLiteral( "GML3" ), prec );
        }
        else if ( geometryName == QLatin1String( "CENTROID" ) )
        {
          QgsGeometry centroid = geom.centroid();
          gmlElem = QgsOgcUtils::geometryToGML( &centroid, doc, QStringLiteral( "GML3" ), prec );
        }
        else
          gmlElem = QgsOgcUtils::geometryToGML( &geom, doc, QStringLiteral( "GML3" ), prec );
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



