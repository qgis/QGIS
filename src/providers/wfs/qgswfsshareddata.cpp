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
#include "moc_qgswfsshareddata.cpp"
#include "qgswfsprovider.h"
#include "qgsogcutils.h"
#include "qgsexpression.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"
#include <QUrlQuery>

QgsWFSSharedData::QgsWFSSharedData( const QString &uri )
  : QgsBackgroundCachedSharedData( "wfs", tr( "WFS" ) )
  , mURI( uri )
{
  mHideProgressDialog = mURI.hideDownloadProgressDialog();
  mServerPrefersCoordinatesForTransactions_1_1 = mURI.preferCoordinatesForWfst11();
}

QgsWFSSharedData::~QgsWFSSharedData()
{
  QgsDebugMsgLevel( QStringLiteral( "~QgsWFSSharedData()" ), 4 );

  cleanup();
}

std::unique_ptr<QgsFeatureDownloaderImpl> QgsWFSSharedData::newFeatureDownloaderImpl( QgsFeatureDownloader *downloader, bool requestMadeFromMainThread )
{
  return std::unique_ptr<QgsFeatureDownloaderImpl>( new QgsWFSFeatureDownloaderImpl( this, downloader, requestMadeFromMainThread ) );
}

bool QgsWFSSharedData::isRestrictedToRequestBBOX() const
{
  return mURI.isRestrictedToRequestBBOX();
}

QgsWFSSharedData *QgsWFSSharedData::clone() const
{
  QgsWFSSharedData *copy = new QgsWFSSharedData( mURI.uri( true ) );
  copy->mURI = mURI;
  copy->mWFSVersion = mWFSVersion;
  copy->mGeometryAttribute = mGeometryAttribute;
  copy->mLayerPropertiesList = mLayerPropertiesList;
  copy->mMapFieldNameToSrcLayerNameFieldName = mMapFieldNameToSrcLayerNameFieldName;
  copy->mFieldNameToXPathAndIsNestedContentMap = mFieldNameToXPathAndIsNestedContentMap;
  copy->mNamespacePrefixToURIMap = mNamespacePrefixToURIMap;
  copy->mPageSize = mPageSize;
  copy->mCaps = mCaps;
  copy->mHasWarnedAboutMissingFeatureId = mHasWarnedAboutMissingFeatureId;
  copy->mGetFeatureEPSGDotHonoursEPSGOrder = mGetFeatureEPSGDotHonoursEPSGOrder;
  copy->mServerPrefersCoordinatesForTransactions_1_1 = mServerPrefersCoordinatesForTransactions_1_1;
  copy->mWKBType = mWKBType;
  copy->mWFSFilter = mWFSFilter;
  copy->mWFSGeometryTypeFilter = mWFSGeometryTypeFilter;
  copy->mSortBy = mSortBy;
  QgsBackgroundCachedSharedData::copyStateToClone( copy );

  return copy;
}

void QgsWFSSharedData::invalidateCacheBaseUnderLock()
{
}


QString QgsWFSSharedData::srsName() const
{
  QString srsName;
  if ( !mSourceCrs.authid().isEmpty() )
  {
    if ( mWFSVersion.startsWith( QLatin1String( "1.0" ) ) || !mSourceCrs.authid().startsWith( QLatin1String( "EPSG:" ) ) ||
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

QString QgsWFSSharedData::computedExpression( const QgsExpression &expression ) const
{
  if ( expression.isValid() )
  {
    QgsOgcUtils::GMLVersion gmlVersion;
    QgsOgcUtils::FilterVersion filterVersion;
    bool honourAxisOrientation = false;
    getVersionValues( gmlVersion, filterVersion, honourAxisOrientation );

    QMap<QString, QString> fieldNameToXPathMap;
    if ( !mFieldNameToXPathAndIsNestedContentMap.isEmpty() )
    {
      for ( auto iterFieldName = mFieldNameToXPathAndIsNestedContentMap.constBegin(); iterFieldName != mFieldNameToXPathAndIsNestedContentMap.constEnd(); ++iterFieldName )
      {
        const QString &fieldName = iterFieldName.key();
        const auto &value = iterFieldName.value();
        fieldNameToXPathMap[fieldName] = value.first;
      }
    }

    QDomDocument expressionDoc;
    QDomElement expressionElem = QgsOgcUtils::expressionToOgcExpression(
      expression, expressionDoc, gmlVersion, filterVersion, mGeometryAttribute,
      srsName(), honourAxisOrientation, mURI.invertAxisOrientation(), nullptr,
      true,
      fieldNameToXPathMap, mNamespacePrefixToURIMap
    );

    if ( !expressionElem.isNull() )
    {
      expressionDoc.appendChild( expressionElem );
      return expressionDoc.toString();
    }
  }
  return QString();
}

bool QgsWFSSharedData::computeFilter( QString &errorMsg )
{
  errorMsg.clear();
  mWFSFilter.clear();
  mSortBy.clear();

  QgsOgcUtils::GMLVersion gmlVersion;
  QgsOgcUtils::FilterVersion filterVersion;
  bool honourAxisOrientation = false;
  getVersionValues( gmlVersion, filterVersion, honourAxisOrientation );

  if ( !mURI.sql().isEmpty() )
  {
    const QgsSQLStatement sql( mURI.sql() );

    const QgsSQLStatement::NodeSelect *select = dynamic_cast<const QgsSQLStatement::NodeSelect *>( sql.rootNode() );
    if ( !select )
    {
      // Makes Coverity happy, but cannot happen in practice
      QgsDebugError( QStringLiteral( "should not happen" ) );
      return false;
    }
    const QList<QgsSQLStatement::NodeColumnSorted *> orderBy = select->orderBy();
    const auto constOrderBy = orderBy;
    for ( QgsSQLStatement::NodeColumnSorted *columnSorted : constOrderBy )
    {
      if ( !mSortBy.isEmpty() )
        mSortBy += QLatin1Char( ',' );
      mSortBy += columnSorted->column()->name();
      if ( !columnSorted->ascending() )
      {
        if ( mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
          mSortBy += QLatin1String( " DESC" );
        else
          mSortBy += QLatin1String( " D" );
      }
    }

    QMap<QString, QString> fieldNameToXPathMap;
    if ( !mFieldNameToXPathAndIsNestedContentMap.isEmpty() )
    {
      for ( auto iterFieldName = mFieldNameToXPathAndIsNestedContentMap.constBegin(); iterFieldName != mFieldNameToXPathAndIsNestedContentMap.constEnd(); ++iterFieldName )
      {
        const QString &fieldName = iterFieldName.key();
        const auto &value = iterFieldName.value();
        fieldNameToXPathMap[fieldName] = value.first;
      }
    }

    QDomDocument filterDoc;
    const QDomElement filterElem = QgsOgcUtils::SQLStatementToOgcFilter(
      sql, filterDoc, gmlVersion, filterVersion, mLayerPropertiesList,
      honourAxisOrientation, mURI.invertAxisOrientation(),
      mCaps.mapUnprefixedTypenameToPrefixedTypename,
      &errorMsg, fieldNameToXPathMap, mNamespacePrefixToURIMap
    );
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
    const QString filter( mURI.filter() );
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
        const QgsExpression filterExpression( filter );

        QMap<QString, QString> fieldNameToXPathMap;
        if ( !mFieldNameToXPathAndIsNestedContentMap.isEmpty() )
        {
          for ( auto iterFieldName = mFieldNameToXPathAndIsNestedContentMap.constBegin(); iterFieldName != mFieldNameToXPathAndIsNestedContentMap.constEnd(); ++iterFieldName )
          {
            const QString &fieldName = iterFieldName.key();
            const auto &value = iterFieldName.value();
            fieldNameToXPathMap[fieldName] = value.first;
          }
        }

        const QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter(
          filterExpression, filterDoc, gmlVersion, filterVersion,
          mLayerPropertiesList.size() == 1 ? mLayerPropertiesList[0].mNamespacePrefix : QString(),
          mLayerPropertiesList.size() == 1 ? mLayerPropertiesList[0].mNamespaceURI : QString(),
          mGeometryAttribute,
          srsName(), honourAxisOrientation, mURI.invertAxisOrientation(),
          &errorMsg, fieldNameToXPathMap, mNamespacePrefixToURIMap
        );

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

void QgsWFSSharedData::computeGeometryTypeFilter()
{
  if ( mWKBType == Qgis::WkbType::NoGeometry )
    mWFSGeometryTypeFilter = QgsWFSProvider::buildIsNullGeometryFilter( mCaps, mGeometryAttribute );
  else if ( mWKBType == Qgis::WkbType::MultiPoint )
    mWFSGeometryTypeFilter = QgsWFSProvider::buildFilterByGeometryType( mCaps, mGeometryAttribute, "IsPoint" );
  else if ( mWKBType == Qgis::WkbType::MultiCurve )
    mWFSGeometryTypeFilter = QgsWFSProvider::buildFilterByGeometryType( mCaps, mGeometryAttribute, "IsCurve" );
  else if ( mWKBType == Qgis::WkbType::MultiSurface )
    mWFSGeometryTypeFilter = QgsWFSProvider::buildFilterByGeometryType( mCaps, mGeometryAttribute, "IsSurface" );
  else if ( mWKBType == Qgis::WkbType::GeometryCollection )
    mWFSGeometryTypeFilter = QgsWFSProvider::buildGeometryCollectionFilter( mCaps, mGeometryAttribute );
}

void QgsWFSSharedData::pushError( const QString &errorMsg ) const
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

  if ( mLayerPropertiesList.size() > 1 )
  {
    QList<QgsGmlStreamingParser::LayerProperties> layerPropertiesList;
    const auto constMLayerPropertiesList = mLayerPropertiesList;
    for ( const QgsOgcUtils::LayerProperties &layerProperties : constMLayerPropertiesList )
    {
      QgsGmlStreamingParser::LayerProperties layerPropertiesOut;
      layerPropertiesOut.mName = layerProperties.mName;
      layerPropertiesOut.mGeometryAttribute = layerProperties.mGeometryAttribute;
      layerPropertiesList << layerPropertiesOut;
    }

    return new QgsGmlStreamingParser( layerPropertiesList, mFields, mMapFieldNameToSrcLayerNameFieldName, axisOrientationLogic, mURI.invertAxisOrientation() );
  }
  else
  {
    auto parser = new QgsGmlStreamingParser( mURI.typeName(), mGeometryAttribute, mFields, axisOrientationLogic, mURI.invertAxisOrientation() );
    if ( !mFieldNameToXPathAndIsNestedContentMap.isEmpty() )
    {
      parser->setFieldsXPath( mFieldNameToXPathAndIsNestedContentMap, mNamespacePrefixToURIMap );
    }
    return parser;
  }
}


QgsRectangle QgsWFSSharedData::getExtentFromSingleFeatureRequest() const
{
  QgsWFSSingleFeatureRequest request( this );
  return request.getExtent();
}

long long QgsWFSSharedData::getFeatureCountFromServer() const
{
  QgsWFSFeatureHitsRequest request( mURI );
  return request.getFeatureCount( mWFSVersion, combineWFSFilters( { mWFSFilter, mWFSGeometryTypeFilter } ), mCaps );
}

QString QgsWFSSharedData::combineWFSFilters( const std::vector<QString> &filters ) const
{
  int countNonEmpty = 0;
  QString nonEmptyFilter;
  for ( const QString &filter : filters )
  {
    if ( !filter.isEmpty() )
    {
      countNonEmpty++;
      nonEmptyFilter = filter;
    }
  }
  if ( countNonEmpty == 0 )
    return QString();
  if ( countNonEmpty == 1 )
    return nonEmptyFilter;

  std::vector<QDomNode> nodes;

  bool envelopeFilterDocSet = false;
  QDomDocument envelopeFilterDoc;
  for ( const QString &filter : filters )
  {
    if ( filter.isEmpty() )
      continue;
    QDomDocument doc;
    QDomNode node;

    ( void ) doc.setContent( filter, !filter.contains( QStringLiteral( "BBOX" ) ) );
    node = doc.firstChildElement().firstChildElement();
    node = doc.firstChildElement().removeChild( node );
    if ( !envelopeFilterDocSet || filter.contains( QStringLiteral( "BBOX" ) ) )
    {
      envelopeFilterDocSet = true;
      envelopeFilterDoc = doc;
    }

    nodes.push_back( node );
  }

  QDomElement andElem = envelopeFilterDoc.createElement( mWFSVersion.startsWith( QLatin1String( "2.0" ) ) ? "fes:And" : "ogc:And" );
  for ( const auto &node : nodes )
  {
    andElem.appendChild( node );
  }
  envelopeFilterDoc.firstChildElement().appendChild( andElem );

  QSet<QString> setNamespaceURI;
  for ( auto iterFieldName = mFieldNameToXPathAndIsNestedContentMap.constBegin(); iterFieldName != mFieldNameToXPathAndIsNestedContentMap.constEnd(); ++iterFieldName )
  {
    const auto &value = iterFieldName.value();
    const QStringList parts = value.first.split( '/' );
    for ( const QString &part : parts )
    {
      const QStringList subparts = part.split( ':' );
      if ( subparts.size() == 2 && subparts[0] != QLatin1String( "gml" ) )
      {
        const auto iter = mNamespacePrefixToURIMap.constFind( subparts[0] );
        if ( iter != mNamespacePrefixToURIMap.constEnd() && !setNamespaceURI.contains( *iter ) )
        {
          setNamespaceURI.insert( *iter );
          QDomAttr attr = envelopeFilterDoc.createAttribute( QStringLiteral( "xmlns:" ) + subparts[0] );
          attr.setValue( *iter );
          envelopeFilterDoc.firstChildElement().setAttributeNode( attr );
        }
      }
    }
  }

  if ( mLayerPropertiesList.size() == 1 && envelopeFilterDoc.firstChildElement().hasAttribute( QStringLiteral( "xmlns:" ) + mLayerPropertiesList[0].mNamespacePrefix ) )
  {
    // nothing to do
  }
  else
  {
    // add xmls:PREFIX=URI attributes to top element
    for ( const QgsOgcUtils::LayerProperties &props : std::as_const( mLayerPropertiesList ) )
    {
      if ( !props.mNamespacePrefix.isEmpty() && !props.mNamespaceURI.isEmpty() && !setNamespaceURI.contains( props.mNamespaceURI ) )
      {
        setNamespaceURI.insert( props.mNamespaceURI );
        QDomAttr attr = envelopeFilterDoc.createAttribute( QStringLiteral( "xmlns:" ) + props.mNamespacePrefix );
        attr.setValue( props.mNamespaceURI );
        envelopeFilterDoc.firstChildElement().setAttributeNode( attr );
      }
    }
  }

  // fprintf(stderr, "%s\n", envelopeFilterDoc.toString().toStdString().c_str());
  return envelopeFilterDoc.toString();
}

void QgsWFSSharedData::getVersionValues( QgsOgcUtils::GMLVersion &gmlVersion, QgsOgcUtils::FilterVersion &filterVersion, bool &honourAxisOrientation ) const
{
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
}

bool QgsWFSSharedData::detectPotentialServerAxisOrderIssueFromSingleFeatureExtent() const
{
  Q_ASSERT( !mComputedExtent.isNull() );
  if ( mWFSVersion.startsWith( QLatin1String( "1.1" ) ) && !mURI.ignoreAxisOrientation() && !mURI.invertAxisOrientation() && mSourceCrs.hasAxisInverted() && mCapabilityExtent.contains( mComputedExtent ) )
  {
    pushError( QObject::tr( "It is likely that there is an issue with coordinate axis order of geometries when interacting with the server. You may want to enable the Ignore axis orientation and/or Invert axis orientation settings of the WFS connection." ) );
    return true;
  }
  return false;
}

// -------------------------


QgsWFSFeatureHitsRequest::QgsWFSFeatureHitsRequest( const QgsWFSDataSourceURI &uri )
  : QgsWfsRequest( uri )
{
}

long long QgsWFSFeatureHitsRequest::getFeatureCount( const QString &WFSVersion, const QString &filter, const QgsWfsCapabilities::Capabilities &caps )
{
  const QString typeName = mUri.typeName();

  QUrl getFeatureUrl( mUri.requestUrl( QStringLiteral( "GetFeature" ) ) );
  QUrlQuery query( getFeatureUrl );
  query.addQueryItem( QStringLiteral( "VERSION" ), WFSVersion );
  if ( WFSVersion.startsWith( QLatin1String( "2.0" ) ) )
  {
    query.addQueryItem( QStringLiteral( "TYPENAMES" ), typeName );
  }
  else
  {
    query.addQueryItem( QStringLiteral( "TYPENAME" ), typeName );
  }

  const QString namespaceValue( caps.getNamespaceParameterValue( WFSVersion, typeName ) );
  if ( !namespaceValue.isEmpty() )
  {
    if ( WFSVersion.startsWith( QLatin1String( "2.0" ) ) )
    {
      query.addQueryItem( QStringLiteral( "NAMESPACES" ), namespaceValue );
    }
    else
    {
      query.addQueryItem( QStringLiteral( "NAMESPACE" ), namespaceValue );
    }
  }

  if ( !filter.isEmpty() )
  {
    query.addQueryItem( QStringLiteral( "FILTER" ), filter );
  }
  query.addQueryItem( QStringLiteral( "RESULTTYPE" ), QStringLiteral( "hits" ) );

  getFeatureUrl.setQuery( query );
  if ( !sendGET( getFeatureUrl, QString(), true ) )
    return -1;

  const QByteArray &buffer = response();

  QgsDebugMsgLevel( QStringLiteral( "parsing QgsWFSFeatureHitsRequest: " ) + buffer, 4 );

  // parse XML
  QString error;
  QDomDocument domDoc;
  if ( !domDoc.setContent( buffer, true, &error ) )
  {
    QgsDebugError( QStringLiteral( "parsing failed: " ) + error );
    return -1;
  }

  const QDomElement doc = domDoc.documentElement();
  const QString numberOfFeatures = ( WFSVersion.startsWith( QLatin1String( "1.1" ) ) ) ? doc.attribute( QStringLiteral( "numberOfFeatures" ) ) :
                                                                                       /* 2.0 */ doc.attribute( QStringLiteral( "numberMatched" ) );
  if ( !numberOfFeatures.isEmpty() )
  {
    bool isValid;
    const long long ret = numberOfFeatures.toLongLong( &isValid );
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
  QUrlQuery query( getFeatureUrl );
  query.addQueryItem( QStringLiteral( "VERSION" ), mShared->mWFSVersion );
  if ( mShared->mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
    query.addQueryItem( QStringLiteral( "TYPENAMES" ), mUri.typeName() );
  else
    query.addQueryItem( QStringLiteral( "TYPENAME" ), mUri.typeName() );

  const QString namespaceValue( mShared->mCaps.getNamespaceParameterValue( mShared->mWFSVersion, mUri.typeName() ) );
  if ( !namespaceValue.isEmpty() )
  {
    if ( mShared->mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
      query.addQueryItem( QStringLiteral( "NAMESPACES" ), namespaceValue );
    else
      query.addQueryItem( QStringLiteral( "NAMESPACE" ), namespaceValue );
  }

  if ( mShared->mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
    query.addQueryItem( QStringLiteral( "COUNT" ), QString::number( 1 ) );
  else
    query.addQueryItem( QStringLiteral( "MAXFEATURES" ), QString::number( 1 ) );

  getFeatureUrl.setQuery( query );
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
    QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> featurePtrList = parser->getAndStealReadyFeatures();
    for ( int i = 0; i < featurePtrList.size(); i++ )
    {
      const QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair &featPair = featurePtrList[i];
      const QgsFeature f( *( featPair.first ) );
      const QgsGeometry geometry = f.geometry();
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
