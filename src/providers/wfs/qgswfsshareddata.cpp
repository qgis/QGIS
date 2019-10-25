/***************************************************************************
    qgswfsshareddata.cpp
    ---------------------
    begin                : March 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsshareddata.h"
#include "qgswfsutils.h"
#include "qgscachedirectorymanager.h"
#include "qgsogcutils.h"
#include "qgsexpression.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"

QgsWFSSharedData::QgsWFSSharedData( const QString &uri )
  : QgsBackgroundCachedSharedData( "wfs", tr( "WFS" ) )
  , mURI( uri )
{
  mHideProgressDialog = mURI.hideDownloadProgressDialog();
}

QgsWFSSharedData::~QgsWFSSharedData()
{
  QgsDebugMsgLevel( QStringLiteral( "~QgsWFSSharedData()" ), 4 );

  cleanup();
}

std::unique_ptr<QgsFeatureDownloaderImpl> QgsWFSSharedData::newFeatureDownloaderImpl( QgsFeatureDownloader *downloader )
{
  return std::unique_ptr<QgsFeatureDownloaderImpl>( new QgsWFSFeatureDownloaderImpl( this, downloader ) );
}

bool QgsWFSSharedData::isRestrictedToRequestBBOX() const
{
  return mURI.isRestrictedToRequestBBOX();
}

void QgsWFSSharedData::invalidateCacheBaseUnderLock()
{
}


QString QgsWFSSharedData::srsName() const
{
  QString srsName;
  if ( !mSourceCrs.authid().isEmpty() )
  {
    if ( mWFSVersion.startsWith( QLatin1String( "1.0" ) ) ||
         !mSourceCrs.authid().startsWith( QLatin1String( "EPSG:" ) ) ||
         // For servers like Geomedia that advertise EPSG:XXXX in capabilities even in WFS 1.1 or 2.0
         mCaps.useEPSGColumnFormat )
    {
      srsName = mSourceCrs.authid();
    }
    else
    {
      QStringList list = mSourceCrs.authid().split( ':' );
      srsName = QStringLiteral( "urn:ogc:def:crs:EPSG::%1" ).arg( list.last() );
    }
  }
  return srsName;
}

bool QgsWFSSharedData::computeFilter( QString &errorMsg )
{
  errorMsg.clear();
  mWFSFilter.clear();
  mSortBy.clear();

  QgsOgcUtils::GMLVersion gmlVersion;
  QgsOgcUtils::FilterVersion filterVersion;
  bool honourAxisOrientation = false;
  if ( mWFSVersion.startsWith( QLatin1String( "1.0" ) ) )
  {
    gmlVersion = QgsOgcUtils::GML_2_1_2;
    filterVersion = QgsOgcUtils::FILTER_OGC_1_0;
  }
  else if ( mWFSVersion.startsWith( QLatin1String( "1.1" ) ) )
  {
    honourAxisOrientation = !mURI.ignoreAxisOrientation();
    gmlVersion = QgsOgcUtils::GML_3_1_0;
    filterVersion = QgsOgcUtils::FILTER_OGC_1_1;
  }
  else
  {
    honourAxisOrientation = !mURI.ignoreAxisOrientation();
    gmlVersion = QgsOgcUtils::GML_3_2_1;
    filterVersion = QgsOgcUtils::FILTER_FES_2_0;
  }

  if ( !mURI.sql().isEmpty() )
  {
    QgsSQLStatement sql( mURI.sql() );

    const QgsSQLStatement::NodeSelect *select = dynamic_cast<const QgsSQLStatement::NodeSelect *>( sql.rootNode() );
    if ( !select )
    {
      // Makes Coverity happy, but cannot happen in practice
      QgsDebugMsg( QStringLiteral( "should not happen" ) );
      return false;
    }
    QList<QgsSQLStatement::NodeColumnSorted *> orderBy = select->orderBy();
    const auto constOrderBy = orderBy;
    for ( QgsSQLStatement::NodeColumnSorted *columnSorted : constOrderBy )
    {
      if ( !mSortBy.isEmpty() )
        mSortBy += QLatin1String( "," );
      mSortBy += columnSorted->column()->name();
      if ( !columnSorted->ascending() )
      {
        if ( mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
          mSortBy += QLatin1String( " DESC" );
        else
          mSortBy += QLatin1String( " D" );
      }
    }

    QDomDocument filterDoc;
    QDomElement filterElem = QgsOgcUtils::SQLStatementToOgcFilter(
                               sql, filterDoc, gmlVersion, filterVersion, mLayerPropertiesList,
                               honourAxisOrientation, mURI.invertAxisOrientation(),
                               mCaps.mapUnprefixedTypenameToPrefixedTypename,
                               &errorMsg );
    if ( !errorMsg.isEmpty() )
    {
      errorMsg = tr( "SQL statement to OGC Filter error: " ) + errorMsg;
      return false;
    }
    if ( !filterElem.isNull() )
    {
      filterDoc.appendChild( filterElem );
      mWFSFilter = filterDoc.toString();
    }
  }
  else
  {
    QString filter( mURI.filter() );
    if ( !filter.isEmpty() )
    {
      //test if filterString is already an OGC filter xml
      QDomDocument filterDoc;
      if ( filterDoc.setContent( filter ) )
      {
        mWFSFilter = filter;
      }
      else
      {
        //if not, if must be a QGIS expression
        QgsExpression filterExpression( filter );

        QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter(
                                   filterExpression, filterDoc, gmlVersion, filterVersion, mGeometryAttribute,
                                   srsName(), honourAxisOrientation, mURI.invertAxisOrientation(),
                                   &errorMsg );

        if ( !errorMsg.isEmpty() )
        {
          errorMsg = tr( "Expression to OGC Filter error: " ) + errorMsg;
          return false;
        }
        if ( !filterElem.isNull() )
        {
          filterDoc.appendChild( filterElem );
          mWFSFilter = filterDoc.toString();
        }
      }
    }
  }

  return true;
}

void QgsWFSSharedData::pushError( const QString &errorMsg )
{
  QgsMessageLog::logMessage( errorMsg, tr( "WFS" ) );
  emit raiseError( errorMsg );
}

QgsGmlStreamingParser *QgsWFSSharedData::createParser() const
{
  QgsGmlStreamingParser::AxisOrientationLogic axisOrientationLogic( QgsGmlStreamingParser::Honour_EPSG_if_urn );
  if ( mURI.ignoreAxisOrientation() )
  {
    axisOrientationLogic = QgsGmlStreamingParser::Ignore_EPSG;
  }

  if ( !mLayerPropertiesList.isEmpty() )
  {
    QList< QgsGmlStreamingParser::LayerProperties > layerPropertiesList;
    const auto constMLayerPropertiesList = mLayerPropertiesList;
    for ( QgsOgcUtils::LayerProperties layerProperties : constMLayerPropertiesList )
    {
      QgsGmlStreamingParser::LayerProperties layerPropertiesOut;
      layerPropertiesOut.mName = layerProperties.mName;
      layerPropertiesOut.mGeometryAttribute = layerProperties.mGeometryAttribute;
      layerPropertiesList << layerPropertiesOut;
    }

    return new QgsGmlStreamingParser( layerPropertiesList,
                                      mFields,
                                      mMapFieldNameToSrcLayerNameFieldName,
                                      axisOrientationLogic,
                                      mURI.invertAxisOrientation() );
  }
  else
  {
    return new QgsGmlStreamingParser( mURI.typeName(),
                                      mGeometryAttribute,
                                      mFields,
                                      axisOrientationLogic,
                                      mURI.invertAxisOrientation() );
  }
}


QgsRectangle QgsWFSSharedData::getExtentFromSingleFeatureRequest() const
{
  QgsWFSSingleFeatureRequest request( this );
  return request.getExtent();
}

int QgsWFSSharedData::getFeatureCountFromServer() const
{
  QgsWFSFeatureHitsRequest request( mURI );
  return request.getFeatureCount( mWFSVersion, mWFSFilter, mCaps );
}

// -------------------------


QgsWFSFeatureHitsRequest::QgsWFSFeatureHitsRequest( const QgsWFSDataSourceURI &uri )
  : QgsWfsRequest( uri )
{
}

int QgsWFSFeatureHitsRequest::getFeatureCount( const QString &WFSVersion,
    const QString &filter, const QgsWfsCapabilities::Capabilities &caps )
{
  QString typeName = mUri.typeName();

  QUrl getFeatureUrl( mUri.requestUrl( QStringLiteral( "GetFeature" ) ) );
  getFeatureUrl.addQueryItem( QStringLiteral( "VERSION" ),  WFSVersion );
  if ( WFSVersion.startsWith( QLatin1String( "2.0" ) ) )
  {
    getFeatureUrl.addQueryItem( QStringLiteral( "TYPENAMES" ), typeName );
  }
  getFeatureUrl.addQueryItem( QStringLiteral( "TYPENAME" ), typeName );

  QString namespaceValue( caps.getNamespaceParameterValue( WFSVersion, typeName ) );
  if ( !namespaceValue.isEmpty() )
  {
    if ( WFSVersion.startsWith( QLatin1String( "2.0" ) ) )
      getFeatureUrl.addQueryItem( QStringLiteral( "NAMESPACES" ), namespaceValue );
    getFeatureUrl.addQueryItem( QStringLiteral( "NAMESPACE" ), namespaceValue );
  }

  if ( !filter.isEmpty() )
  {
    getFeatureUrl.addQueryItem( QStringLiteral( "FILTER" ), filter );
  }
  getFeatureUrl.addQueryItem( QStringLiteral( "RESULTTYPE" ), QStringLiteral( "hits" ) );

  if ( !sendGET( getFeatureUrl, QString(), true ) )
    return -1;

  const QByteArray &buffer = response();

  QgsDebugMsgLevel( QStringLiteral( "parsing QgsWFSFeatureHitsRequest: " ) + buffer, 4 );

  // parse XML
  QString error;
  QDomDocument domDoc;
  if ( !domDoc.setContent( buffer, true, &error ) )
  {
    QgsDebugMsg( QStringLiteral( "parsing failed: " ) + error );
    return -1;
  }

  QDomElement doc = domDoc.documentElement();
  QString numberOfFeatures =
    ( WFSVersion.startsWith( QLatin1String( "1.1" ) ) ) ? doc.attribute( QStringLiteral( "numberOfFeatures" ) ) :
    /* 2.0 */                         doc.attribute( QStringLiteral( "numberMatched" ) );
  if ( !numberOfFeatures.isEmpty() )
  {
    bool isValid;
    int ret = numberOfFeatures.toInt( &isValid );
    if ( !isValid )
      return -1;
    return ret;
  }

  return -1;
}

QString QgsWFSFeatureHitsRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of feature count failed: %1" ).arg( reason );
}


// -------------------------


QgsWFSSingleFeatureRequest::QgsWFSSingleFeatureRequest( const QgsWFSSharedData *shared )
  : QgsWfsRequest( shared->mURI ), mShared( shared )
{
}

QgsRectangle QgsWFSSingleFeatureRequest::getExtent()
{
  QUrl getFeatureUrl( mUri.requestUrl( QStringLiteral( "GetFeature" ) ) );
  getFeatureUrl.addQueryItem( QStringLiteral( "VERSION" ),  mShared->mWFSVersion );
  if ( mShared->mWFSVersion .startsWith( QLatin1String( "2.0" ) ) )
    getFeatureUrl.addQueryItem( QStringLiteral( "TYPENAMES" ), mUri.typeName() );
  getFeatureUrl.addQueryItem( QStringLiteral( "TYPENAME" ), mUri.typeName() );

  QString namespaceValue( mShared->mCaps.getNamespaceParameterValue( mShared->mWFSVersion, mUri.typeName() ) );
  if ( !namespaceValue.isEmpty() )
  {
    if ( mShared->mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
      getFeatureUrl.addQueryItem( QStringLiteral( "NAMESPACES" ), namespaceValue );
    getFeatureUrl.addQueryItem( QStringLiteral( "NAMESPACE" ), namespaceValue );
  }

  if ( mShared->mWFSVersion .startsWith( QLatin1String( "2.0" ) ) )
    getFeatureUrl.addQueryItem( QStringLiteral( "COUNT" ), QString::number( 1 ) );
  else
    getFeatureUrl.addQueryItem( QStringLiteral( "MAXFEATURES" ), QString::number( 1 ) );

  if ( !sendGET( getFeatureUrl, QString(), true ) )
    return QgsRectangle();

  const QByteArray &buffer = response();

  QgsDebugMsgLevel( QStringLiteral( "parsing QgsWFSSingleFeatureRequest: " ) + buffer, 4 );

  // parse XML
  QgsGmlStreamingParser *parser = mShared->createParser();
  QString gmlProcessErrorMsg;
  QgsRectangle extent;
  if ( parser->processData( buffer, true, gmlProcessErrorMsg ) )
  {
    QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> featurePtrList =
      parser->getAndStealReadyFeatures();
    for ( int i = 0; i < featurePtrList.size(); i++ )
    {
      QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair &featPair = featurePtrList[i];
      QgsFeature f( *( featPair.first ) );
      QgsGeometry geometry = f.geometry();
      if ( !geometry.isNull() )
      {
        extent = geometry.boundingBox();
      }
      delete featPair.first;
    }
  }
  delete parser;
  return extent;
}

QString QgsWFSSingleFeatureRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of feature failed: %1" ).arg( reason );
}

