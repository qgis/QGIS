/***************************************************************************
    qgsgml.cpp
    ---------------------
    begin                : February 2013
    copyright            : (C) 2013 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgml.h"

#include <limits>
#include <ogr_api.h>

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsogcutils.h"
#include "qgsogrutils.h"
#include "qgsrectangle.h"
#include "qgssetrequestinitiator_p.h"
#include "qgswkbptr.h"

#include <QBuffer>
#include <QList>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QTextCodec>
#include <QUrl>

#include "moc_qgsgml.cpp"

using namespace nlohmann;

#ifndef NS_SEPARATOR_DEFINED
#define NS_SEPARATOR_DEFINED
static const char NS_SEPARATOR = '?';
#endif

static const char *GML_NAMESPACE = "http://www.opengis.net/gml";
static const char *GML32_NAMESPACE = "http://www.opengis.net/gml/3.2";

QgsGml::QgsGml(
  const QString &typeName,
  const QString &geometryAttribute,
  const QgsFields &fields )
  : mParser( typeName, geometryAttribute, fields )
  , mTypeName( typeName )
{
  const int index = mTypeName.indexOf( ':' );
  if ( index != -1 && index < mTypeName.length() )
  {
    mTypeName = mTypeName.mid( index + 1 );
  }
}

int QgsGml::getFeatures( const QString &uri, Qgis::WkbType *wkbType, QgsRectangle *extent, const QString &userName, const QString &password, const QString &authcfg )
{
  //start with empty extent
  mExtent.setNull();

  QNetworkRequest request( uri );
  QgsSetRequestInitiatorClass( request, u"QgsGml"_s );

  if ( !authcfg.isEmpty() )
  {
    if ( !QgsApplication::authManager()->updateNetworkRequest( request, authcfg ) )
    {
      QgsMessageLog::logMessage(
        tr( "GML Getfeature network request update failed for authcfg %1" ).arg( authcfg ),
        tr( "Network" ),
        Qgis::MessageLevel::Critical
      );
      return 1;
    }
  }
  else if ( !userName.isNull() || !password.isNull() )
  {
    request.setRawHeader( "Authorization", "Basic " + u"%1:%2"_s.arg( userName, password ).toLatin1().toBase64() );
  }
  QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );

  if ( !authcfg.isEmpty() )
  {
    if ( !QgsApplication::authManager()->updateNetworkReply( reply, authcfg ) )
    {
      reply->deleteLater();
      QgsMessageLog::logMessage(
        tr( "GML Getfeature network reply update failed for authcfg %1" ).arg( authcfg ),
        tr( "Network" ),
        Qgis::MessageLevel::Critical
      );
      return 1;
    }
  }

  connect( reply, &QNetworkReply::finished, this, &QgsGml::setFinished );
  connect( reply, &QNetworkReply::downloadProgress, this, &QgsGml::handleProgressEvent );

  //find out if there is a QGIS main window. If yes, display a progress dialog
  QProgressDialog *progressDialog = nullptr;
  QWidget *mainWindow = nullptr;
  const QWidgetList topLevelWidgets = qApp->topLevelWidgets();
  for ( QWidgetList::const_iterator it = topLevelWidgets.constBegin(); it != topLevelWidgets.constEnd(); ++it )
  {
    if ( ( *it )->objectName() == "QgisApp"_L1 )
    {
      mainWindow = *it;
      break;
    }
  }
  if ( mainWindow )
  {
    progressDialog = new QProgressDialog( tr( "Loading GML data\n%1" ).arg( mTypeName ), tr( "Abort" ), 0, 0, mainWindow );
    progressDialog->setWindowModality( Qt::ApplicationModal );
    connect( this, &QgsGml::dataReadProgress, progressDialog, &QProgressDialog::setValue );
    connect( this, &QgsGml::totalStepsUpdate, progressDialog, &QProgressDialog::setMaximum );
    connect( progressDialog, &QProgressDialog::canceled, this, &QgsGml::setFinished );
    progressDialog->show();
  }

  int atEnd = 0;
  while ( !atEnd )
  {
    if ( mFinished )
    {
      atEnd = 1;
    }
    const QByteArray readData = reply->readAll();
    if ( !readData.isEmpty() )
    {
      QString errorMsg;
      if ( !mParser.processData( readData, atEnd, errorMsg ) )
        QgsMessageLog::logMessage( errorMsg, QObject::tr( "WFS" ) );

    }
    QCoreApplication::processEvents();
  }

  fillMapsFromParser();

  const QNetworkReply::NetworkError replyError = reply->error();
  const QString replyErrorString = reply->errorString();

  delete reply;
  delete progressDialog;

  if ( replyError )
  {
    QgsMessageLog::logMessage(
      tr( "GML Getfeature network request failed with error: %1" ).arg( replyErrorString ),
      tr( "Network" ),
      Qgis::MessageLevel::Critical
    );
    return 1;
  }

  *wkbType = mParser.wkbType();

  if ( *wkbType != Qgis::WkbType::Unknown )
  {
    if ( mExtent.isEmpty() )
    {
      //reading of bbox from the server failed, so we calculate it less efficiently by evaluating the features
      calculateExtentFromFeatures();
    }
  }

  if ( extent )
    *extent = mExtent;

  return 0;
}

int QgsGml::getFeatures( const QByteArray &data, Qgis::WkbType *wkbType, QgsRectangle *extent )
{
  mExtent.setNull();

  QString errorMsg;
  if ( !mParser.processData( data, true /* atEnd */, errorMsg ) )
    QgsMessageLog::logMessage( errorMsg, QObject::tr( "WFS" ) );

  fillMapsFromParser();

  *wkbType = mParser.wkbType();

  if ( extent )
    *extent = mExtent;

  return 0;
}

void QgsGml::fillMapsFromParser()
{
  const QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = mParser.getAndStealReadyFeatures();
  const auto constFeatures = features;
  for ( const QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair &featPair : constFeatures )
  {
    QgsFeature *feat = featPair.first;
    const QString &gmlId = featPair.second;
    mFeatures.insert( feat->id(), feat );
    if ( !gmlId.isEmpty() )
    {
      mIdMap.insert( feat->id(), gmlId );
    }
  }
}

void QgsGml::setFinished()
{
  mFinished = true;
}

void QgsGml::handleProgressEvent( qint64 progress, qint64 totalSteps )
{
  if ( totalSteps < 0 )
  {
    totalSteps = 0;
    progress = 0;
  }
  emit totalStepsUpdate( totalSteps );
  emit dataReadProgress( progress );
  emit dataProgressAndSteps( progress, totalSteps );
}

void QgsGml::calculateExtentFromFeatures()
{
  if ( mFeatures.empty() )
  {
    return;
  }

  QgsFeature *currentFeature = nullptr;
  QgsGeometry currentGeometry;
  bool bboxInitialized = false; //gets true once bbox has been set to the first geometry

  for ( int i = 0; i < mFeatures.size(); ++i )
  {
    currentFeature = mFeatures[i];
    if ( !currentFeature )
    {
      continue;
    }
    currentGeometry = currentFeature->geometry();
    if ( !currentGeometry.isNull() )
    {
      if ( !bboxInitialized )
      {
        mExtent = currentGeometry.boundingBox();
        bboxInitialized = true;
      }
      else
      {
        mExtent.combineExtentWith( currentGeometry.boundingBox() );
      }
    }
  }
}

QgsCoordinateReferenceSystem QgsGml::crs() const
{
  QgsCoordinateReferenceSystem crs;
  if ( !mParser.srsName().isEmpty() )
  {
    crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mParser.srsName() );
  }
  return crs;
}





QgsGmlStreamingParser::QgsGmlStreamingParser( const QString &typeName,
    const QString &geometryAttribute,
    const QgsFields &fields,
    AxisOrientationLogic axisOrientationLogic,
    bool invertAxisOrientation )
  : mTypeName( typeName )
  , mTypeNameBA( mTypeName.toUtf8() )
  , mTypeNamePtr( mTypeNameBA.constData() )
  , mTypeNameUTF8Len( strlen( mTypeNamePtr ) )
  , mWkbType( Qgis::WkbType::Unknown )
  , mGeometryAttribute( geometryAttribute )
  , mGeometryAttributeBA( geometryAttribute.toUtf8() )
  , mGeometryAttributePtr( mGeometryAttributeBA.constData() )
  , mGeometryAttributeUTF8Len( strlen( mGeometryAttributePtr ) )
  , mFields( fields )
  , mIsException( false )
  , mTruncatedResponse( false )
  , mParseDepth( 0 )
  , mFeatureTupleDepth( 0 )
  , mFeatureCount( 0 )
  , mCurrentWKB( nullptr, 0 )
  , mBoundedByNullFound( false )
  , mDimension( 0 )
  , mCoorMode( Coordinate )
  , mAxisOrientationLogic( axisOrientationLogic )
  , mInvertAxisOrientationRequest( invertAxisOrientation )
  , mInvertAxisOrientation( invertAxisOrientation )
  , mNumberReturned( -1 )
  , mNumberMatched( -1 )
  , mFoundUnhandledGeometryElement( false )
{
  mThematicAttributes.clear();
  for ( int i = 0; i < fields.size(); i++ )
  {
    mThematicAttributes.insert( fields.at( i ).name(), qMakePair( i, fields.at( i ) ) );
  }

  mEndian = QgsApplication::endian();

  const int index = mTypeName.indexOf( ':' );
  if ( index != -1 && index < mTypeName.length() )
  {
    mTypeName = mTypeName.mid( index + 1 );
    mTypeNameBA = mTypeName.toUtf8();
    mTypeNamePtr = mTypeNameBA.constData();
    mTypeNameUTF8Len = strlen( mTypeNamePtr );
  }

  createParser();
}

static QString stripNS( const QString &string )
{
  const int index = string.indexOf( ':' );
  if ( index != -1 && index < string.length() )
  {
    return string.mid( index + 1 );
  }
  return string;
}

QgsGmlStreamingParser::QgsGmlStreamingParser( const QList<LayerProperties> &layerProperties,
    const QgsFields &fields,
    const QMap< QString, QPair<QString, QString> > &fieldNameToSrcLayerNameFieldNameMap,
    AxisOrientationLogic axisOrientationLogic,
    bool invertAxisOrientation )
  : mLayerProperties( layerProperties )
  , mTypeNameUTF8Len( 0 )
  , mWkbType( Qgis::WkbType::Unknown )
  , mGeometryAttributeUTF8Len( 0 )
  , mFields( fields )
  , mIsException( false )
  , mTruncatedResponse( false )
  , mParseDepth( 0 )
  , mFeatureTupleDepth( 0 )
  , mFeatureCount( 0 )
  , mCurrentWKB( nullptr, 0 )
  , mBoundedByNullFound( false )
  , mDimension( 0 )
  , mCoorMode( Coordinate )
  , mAxisOrientationLogic( axisOrientationLogic )
  , mInvertAxisOrientationRequest( invertAxisOrientation )
  , mInvertAxisOrientation( invertAxisOrientation )
  , mNumberReturned( -1 )
  , mNumberMatched( -1 )
  , mFoundUnhandledGeometryElement( false )
{
  mThematicAttributes.clear();
  for ( int i = 0; i < fields.size(); i++ )
  {
    const QMap< QString, QPair<QString, QString> >::const_iterator att_it = fieldNameToSrcLayerNameFieldNameMap.constFind( fields.at( i ).name() );
    if ( att_it != fieldNameToSrcLayerNameFieldNameMap.constEnd() )
    {
      if ( mLayerProperties.size() == 1 )
        mThematicAttributes.insert( att_it.value().second, qMakePair( i, fields.at( i ) ) );
      else
        mThematicAttributes.insert( stripNS( att_it.value().first ) + "|" + att_it.value().second, qMakePair( i, fields.at( i ) ) );
    }
  }
  bool alreadyFoundGeometry = false;
  for ( int i = 0; i < mLayerProperties.size(); i++ )
  {
    // We only support one geometry field per feature
    if ( !mLayerProperties[i].mGeometryAttribute.isEmpty() )
    {
      if ( alreadyFoundGeometry )
      {
        QgsDebugMsgLevel( u"Will ignore geometry field %1 from typename %2"_s.
                          arg( mLayerProperties[i].mGeometryAttribute, mLayerProperties[i].mName ), 2 );
        mLayerProperties[i].mGeometryAttribute.clear();
      }
      alreadyFoundGeometry = true;
    }
    mMapTypeNameToProperties.insert( stripNS( mLayerProperties[i].mName ), mLayerProperties[i] );
  }

  if ( mLayerProperties.size() == 1 )
  {
    mTypeName = mLayerProperties[0].mName;
    mGeometryAttribute = mLayerProperties[0].mGeometryAttribute;
    mGeometryAttributeBA = mGeometryAttribute.toUtf8();
    mGeometryAttributePtr = mGeometryAttributeBA.constData();
    mGeometryAttributeUTF8Len = strlen( mGeometryAttributePtr );
    const int index = mTypeName.indexOf( ':' );
    if ( index != -1 && index < mTypeName.length() )
    {
      mTypeName = mTypeName.mid( index + 1 );
    }
    mTypeNameBA = mTypeName.toUtf8();
    mTypeNamePtr = mTypeNameBA.constData();
    mTypeNameUTF8Len = strlen( mTypeNamePtr );
  }

  mEndian = QgsApplication::endian();

  createParser();
}


void QgsGmlStreamingParser::setFieldsXPath(
  const QMap<QString, QPair<QString, bool>> &fieldNameToXPathMapAndIsNestedContent,
  const QMap<QString, QString> &mapNamespacePrefixToURI )
{
  for ( auto iter = fieldNameToXPathMapAndIsNestedContent.constBegin(); iter != fieldNameToXPathMapAndIsNestedContent.constEnd(); ++iter )
  {
    mMapXPathToFieldNameAndIsNestedContent[iter.value().first] = QPair<QString, bool>( iter.key(), iter.value().second );
  }
  for ( auto iter = mapNamespacePrefixToURI.constBegin(); iter != mapNamespacePrefixToURI.constEnd(); ++iter )
    mMapNamespaceURIToNamespacePrefix[iter.value()] = iter.key();
}


QgsGmlStreamingParser::~QgsGmlStreamingParser()
{
  XML_ParserFree( mParser );

  // Normally a sane user of this class should have consumed everything...
  const auto constMFeatureList = mFeatureList;
  for ( const QgsGmlFeaturePtrGmlIdPair &featPair : constMFeatureList )
  {
    delete featPair.first;
  }

  delete mCurrentFeature;
}

bool QgsGmlStreamingParser::processData( const QByteArray &data, bool atEnd )
{
  QString errorMsg;
  if ( !processData( data, atEnd, errorMsg ) )
  {
    QgsMessageLog::logMessage( errorMsg, QObject::tr( "WFS" ) );
    return false;
  }
  return true;
}

bool QgsGmlStreamingParser::processData( const QByteArray &pdata, bool atEnd, QString &errorMsg )
{
  QByteArray data = pdata;

  if ( mCodec )
  {
    // convert data to UTF-8
    QString strData = mCodec->toUnicode( pdata );
    data = strData.toUtf8();
  }

  if ( XML_Parse( mParser, data, data.size(), atEnd ) == XML_STATUS_ERROR )
  {
    const XML_Error errorCode = XML_GetErrorCode( mParser );
    if ( !mCodec && errorCode == XML_ERROR_UNKNOWN_ENCODING )
    {
      // Specified encoding is unknown, Expat only accepts UTF-8, UTF-16, ISO-8859-1
      // Try to get encoding string and convert data to utf-8
      const thread_local QRegularExpression reEncoding( u"<?xml.*encoding=['\"]([^'\"]*)['\"].*?>"_s,
          QRegularExpression::CaseInsensitiveOption );
      QRegularExpressionMatch match = reEncoding.match( pdata );
      const QString encoding = match.hasMatch() ? match.captured( 1 ) : QString();
      mCodec = !encoding.isEmpty() ? QTextCodec::codecForName( encoding.toLatin1() ) : nullptr;
      if ( mCodec )
      {
        // recreate parser with UTF-8 encoding
        XML_ParserFree( mParser );
        mParser = nullptr;
        createParser( QByteArrayLiteral( "UTF-8" ) );

        return processData( data, atEnd, errorMsg );
      }
    }

    errorMsg = QObject::tr( "Error: %1 on line %2, column %3" )
               .arg( XML_ErrorString( errorCode ) )
               .arg( XML_GetCurrentLineNumber( mParser ) )
               .arg( XML_GetCurrentColumnNumber( mParser ) );

    return false;
  }

  return true;
}

QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> QgsGmlStreamingParser::getAndStealReadyFeatures()
{
  QVector<QgsGmlFeaturePtrGmlIdPair> ret = mFeatureList;
  mFeatureList.clear();
  return ret;
}

/**
 * Returns a json string or number from the provided string. When a string
 * looks like a number, a json number is returned.
 */
static json jsonFromString( const QString &s )
{
  bool conversionOk;

  // Does it look like a floating-point value ?
  if ( s.indexOf( '.' ) >= 0 || s.indexOf( 'e' ) >= 0 )
  {
    const auto doubleVal = s.toDouble( &conversionOk );
    if ( conversionOk )
    {
      return json( doubleVal );
    }
  }
  // Does it look like an integer? (but don't recognize strings starting with
  // 0)
  else if ( !s.isEmpty() && s[0] != '0' )
  {
    const auto longlongVal = s.toLongLong( &conversionOk );
    if ( conversionOk )
    {
      return json( longlongVal );
    }
  }

  return json( s.toStdString() );
}

#define LOCALNAME_EQUALS(string_constant) \
  ( localNameLen == static_cast<int>(strlen( string_constant )) && memcmp(pszLocalName, string_constant, localNameLen) == 0 )

void QgsGmlStreamingParser::startElement( const XML_Char *el, const XML_Char **attr )
{
  const int elLen = static_cast<int>( strlen( el ) );
  const char *pszSep = strchr( el, NS_SEPARATOR );
  const char *pszLocalName = ( pszSep ) ? pszSep + 1 : el;
  const int nsLen = ( pszSep ) ? ( int )( pszSep - el ) : 0;
  const int localNameLen = ( pszSep ) ? ( int )( elLen - nsLen ) - 1 : elLen;
  const ParseMode parseMode( mParseModeStack.isEmpty() ? None : mParseModeStack.top() );
  int elDimension = 0;

  // Figure out if the GML namespace is GML_NAMESPACE or GML32_NAMESPACE
  if ( !mGMLNameSpaceURIPtr && pszSep )
  {
    if ( nsLen == static_cast<int>( strlen( GML_NAMESPACE ) ) && memcmp( el, GML_NAMESPACE, nsLen ) == 0 )
    {
      mGMLNameSpaceURI = GML_NAMESPACE;
      mGMLNameSpaceURIPtr = GML_NAMESPACE;
    }
    else if ( nsLen == static_cast<int>( strlen( GML32_NAMESPACE ) ) && memcmp( el, GML32_NAMESPACE, nsLen ) == 0 )
    {
      mGMLNameSpaceURI = GML32_NAMESPACE;
      mGMLNameSpaceURIPtr = GML32_NAMESPACE;
    }
  }

  const bool isGMLNS = ( nsLen == mGMLNameSpaceURI.size() && mGMLNameSpaceURIPtr && memcmp( el, mGMLNameSpaceURIPtr, nsLen ) == 0 );
  bool isGeom = false;

  if ( parseMode == Geometry || parseMode == Coordinate || parseMode == PosList ||
       parseMode == MultiPoint || parseMode == MultiLine || parseMode == MultiPolygon )
  {
    mGeometryString.append( "<", 1 );
    mGeometryString.append( pszLocalName, localNameLen );
    mGeometryString.append( " ", 1 );
    for ( const XML_Char **attrIter = attr; attrIter && *attrIter; attrIter += 2 )
    {
      const size_t nAttrLen = strlen( attrIter[0] );
      const size_t GML32_NAMESPACE_LEN = strlen( GML32_NAMESPACE );
      const size_t GML_NAMESPACE_LEN = strlen( GML_NAMESPACE );
      if ( nAttrLen > GML32_NAMESPACE_LEN &&
           attrIter[0][GML32_NAMESPACE_LEN] == '?' &&
           memcmp( attrIter[0], GML32_NAMESPACE, GML32_NAMESPACE_LEN ) == 0 )
      {
        mGeometryString.append( "gml:" );
        mGeometryString.append( attrIter[0] + GML32_NAMESPACE_LEN + 1 );
      }
      else if ( nAttrLen > GML_NAMESPACE_LEN &&
                attrIter[0][GML_NAMESPACE_LEN] == '?' &&
                memcmp( attrIter[0], GML_NAMESPACE, GML_NAMESPACE_LEN ) == 0 )
      {
        mGeometryString.append( "gml:" );
        mGeometryString.append( attrIter[0] + GML_NAMESPACE_LEN + 1 );
      }
      else
      {
        mGeometryString.append( attrIter[0] );
      }
      mGeometryString.append( "=\"", 2 );
      mGeometryString.append( attrIter[1] );
      mGeometryString.append( "\" ", 2 );

    }
    mGeometryString.append( ">", 1 );
  }

  if ( !mAttributeValIsNested && isGMLNS && LOCALNAME_EQUALS( "coordinates" ) )
  {
    mParseModeStack.push( Coordinate );
    mCoorMode = QgsGmlStreamingParser::Coordinate;
    mStringCash.clear();
    mCoordinateSeparator = readAttribute( u"cs"_s, attr );
    if ( mCoordinateSeparator.isEmpty() )
    {
      mCoordinateSeparator = ',';
    }
    mTupleSeparator = readAttribute( u"ts"_s, attr );
    if ( mTupleSeparator.isEmpty() )
    {
      mTupleSeparator = ' ';
    }
  }
  else if ( !mAttributeValIsNested && isGMLNS &&
            ( LOCALNAME_EQUALS( "pos" ) || LOCALNAME_EQUALS( "posList" ) ) )
  {
    mParseModeStack.push( QgsGmlStreamingParser::PosList );
    if ( mCoorMode == QgsGmlStreamingParser::PosList )
    {
      if ( !mStringCash.isEmpty() )
      {
        mStringCash.append( ' '_L1 );
      }
    }
    else
    {
      mStringCash.clear();
    }
    mCoorMode = QgsGmlStreamingParser::PosList;
    elDimension = readSrsNameAndDimensionAttributes( attr );
  }
  else if ( ( parseMode == Feature || parseMode == FeatureTuple ) &&
            mCurrentFeature &&
            localNameLen == static_cast<int>( mGeometryAttributeUTF8Len ) &&
            memcmp( pszLocalName, mGeometryAttributePtr, localNameLen ) == 0 )
  {
    mParseModeStack.push( QgsGmlStreamingParser::Geometry );
    mFoundUnhandledGeometryElement = false;
    mGeometryString.clear();
    mStringCash.clear();
  }
  //else if ( mParseModeStack.size() == 0 && elementName == mGMLNameSpaceURI + NS_SEPARATOR + "boundedBy" )
  else if ( !mAttributeValIsNested && isGMLNS && LOCALNAME_EQUALS( "boundedBy" ) )
  {
    mParseModeStack.push( QgsGmlStreamingParser::BoundingBox );
    mCurrentExtent = QgsRectangle();
    mBoundedByNullFound = false;
  }
  else if ( parseMode == BoundingBox &&
            isGMLNS && LOCALNAME_EQUALS( "null" ) )
  {
    mParseModeStack.push( QgsGmlStreamingParser::Null );
    mBoundedByNullFound = true;
  }
  else if ( parseMode == BoundingBox &&
            isGMLNS && LOCALNAME_EQUALS( "Envelope" ) )
  {
    isGeom = true;
    mParseModeStack.push( QgsGmlStreamingParser::Envelope );
  }
  else if ( parseMode == Envelope &&
            isGMLNS && LOCALNAME_EQUALS( "lowerCorner" ) )
  {
    mParseModeStack.push( QgsGmlStreamingParser::LowerCorner );
    mStringCash.clear();
  }
  else if ( parseMode == Envelope &&
            isGMLNS && LOCALNAME_EQUALS( "upperCorner" ) )
  {
    mParseModeStack.push( QgsGmlStreamingParser::UpperCorner );
    mStringCash.clear();
  }
  else if ( parseMode == None && !mTypeNamePtr &&
            LOCALNAME_EQUALS( "Tuple" ) )
  {
    Q_ASSERT( !mCurrentFeature );
    mCurrentFeature = new QgsFeature( mFeatureCount );
    mCurrentFeature->setFields( mFields ); // allow name-based attribute lookups
    const QgsAttributes attributes( mThematicAttributes.size() ); //add empty attributes
    mCurrentFeature->setAttributes( attributes );
    mParseModeStack.push( QgsGmlStreamingParser::Tuple );
    mCurrentFeatureId.clear();
  }
  else if ( parseMode == Tuple )
  {
    const QString currentTypename( QString::fromUtf8( pszLocalName, localNameLen ) );
    const QMap< QString, LayerProperties >::const_iterator iter = mMapTypeNameToProperties.constFind( currentTypename );
    if ( iter != mMapTypeNameToProperties.constEnd() )
    {
      mFeatureTupleDepth = mParseDepth;
      mCurrentTypename = currentTypename;
      mGeometryAttribute.clear();
      if ( mCurrentWKB.size() == 0 )
      {
        mGeometryAttribute = iter.value().mGeometryAttribute;
      }
      mGeometryAttributeBA = mGeometryAttribute.toUtf8();
      mGeometryAttributePtr = mGeometryAttributeBA.constData();
      mGeometryAttributeUTF8Len = strlen( mGeometryAttributePtr );
      mParseModeStack.push( QgsGmlStreamingParser::FeatureTuple );
      QString id;
      if ( mGMLNameSpaceURI.isEmpty() )
      {
        id = readAttribute( QString( GML_NAMESPACE ) + NS_SEPARATOR + "id", attr );
        if ( !id.isEmpty() )
        {
          mGMLNameSpaceURI = GML_NAMESPACE;
          mGMLNameSpaceURIPtr = GML_NAMESPACE;
        }
        else
        {
          id = readAttribute( QString( GML32_NAMESPACE ) + NS_SEPARATOR + "id", attr );
          if ( !id.isEmpty() )
          {
            mGMLNameSpaceURI = GML32_NAMESPACE;
            mGMLNameSpaceURIPtr = GML32_NAMESPACE;
          }
        }
      }
      else
        id = readAttribute( mGMLNameSpaceURI + NS_SEPARATOR + "id", attr );
      if ( !mCurrentFeatureId.isEmpty() )
        mCurrentFeatureId += '|';
      mCurrentFeatureId += id;
    }
  }
  else if ( parseMode == None &&
            localNameLen == static_cast<int>( mTypeNameUTF8Len ) &&
            mTypeNamePtr &&
            memcmp( pszLocalName, mTypeNamePtr, mTypeNameUTF8Len ) == 0 )
  {
    Q_ASSERT( !mCurrentFeature );
    mCurrentFeature = new QgsFeature( mFeatureCount );
    mCurrentFeature->setFields( mFields ); // allow name-based attribute lookups
    const QgsAttributes attributes( mThematicAttributes.size() ); //add empty attributes
    mCurrentFeature->setAttributes( attributes );
    mParseModeStack.push( QgsGmlStreamingParser::Feature );
    mCurrentXPathWithinFeature.clear();
    mCurrentFeatureId = readAttribute( u"fid"_s, attr );
    if ( mCurrentFeatureId.isEmpty() )
    {
      // Figure out if the GML namespace is GML_NAMESPACE or GML32_NAMESPACE
      // (should happen only for the first features if there's no gml: element
      // encountered before
      if ( mGMLNameSpaceURI.isEmpty() )
      {
        mCurrentFeatureId = readAttribute( QString( GML_NAMESPACE ) + NS_SEPARATOR + "id", attr );
        if ( !mCurrentFeatureId.isEmpty() )
        {
          mGMLNameSpaceURI = GML_NAMESPACE;
          mGMLNameSpaceURIPtr = GML_NAMESPACE;
        }
        else
        {
          mCurrentFeatureId = readAttribute( QString( GML32_NAMESPACE ) + NS_SEPARATOR + "id", attr );
          if ( !mCurrentFeatureId.isEmpty() )
          {
            mGMLNameSpaceURI = GML32_NAMESPACE;
            mGMLNameSpaceURIPtr = GML32_NAMESPACE;
          }
        }
      }
      else
        mCurrentFeatureId = readAttribute( mGMLNameSpaceURI + NS_SEPARATOR + "id", attr );
    }
  }

  else if ( parseMode == BoundingBox && isGMLNS && LOCALNAME_EQUALS( "Box" ) )
  {
    isGeom = true;
  }
  else if ( !mAttributeValIsNested && isGMLNS && LOCALNAME_EQUALS( "Point" ) )
  {
    isGeom = true;
  }
  else if ( !mAttributeValIsNested && isGMLNS && LOCALNAME_EQUALS( "LineString" ) )
  {
    isGeom = true;
  }
  else if ( !mAttributeValIsNested && isGMLNS &&
            localNameLen == static_cast<int>( strlen( "Polygon" ) ) && memcmp( pszLocalName, "Polygon", localNameLen ) == 0 )
  {
    isGeom = true;
    mCurrentWKBFragments.push_back( QList<QByteArray>() );
  }
  else if ( !mAttributeValIsNested && isGMLNS && LOCALNAME_EQUALS( "MultiPoint" ) )
  {
    isGeom = true;
    mParseModeStack.push( QgsGmlStreamingParser::MultiPoint );
    //we need one nested list for intermediate WKB
    mCurrentWKBFragments.push_back( QList<QByteArray>() );
  }
  else if ( !mAttributeValIsNested && isGMLNS && ( LOCALNAME_EQUALS( "MultiLineString" ) || LOCALNAME_EQUALS( "MultiCurve" ) ) )
  {
    isGeom = true;
    mParseModeStack.push( QgsGmlStreamingParser::MultiLine );
    //we need one nested list for intermediate WKB
    mCurrentWKBFragments.push_back( QList<QByteArray>() );
  }
  else if ( !mAttributeValIsNested && isGMLNS && ( LOCALNAME_EQUALS( "MultiPolygon" ) || LOCALNAME_EQUALS( "MultiSurface" ) ) )
  {
    isGeom = true;
    mParseModeStack.push( QgsGmlStreamingParser::MultiPolygon );
  }
  else if ( parseMode == FeatureTuple )
  {
    const QString localName( QString::fromUtf8( pszLocalName, localNameLen ) );
    if ( mThematicAttributes.contains( mCurrentTypename + '|' + localName ) )
    {
      mParseModeStack.push( QgsGmlStreamingParser::AttributeTuple );
      mAttributeName = mCurrentTypename + '|' + localName;
      mStringCash.clear();
    }
  }
  else if ( parseMode == Feature )
  {
    const QString localName( QString::fromUtf8( pszLocalName, localNameLen ) );
    if ( !mMapXPathToFieldNameAndIsNestedContent.isEmpty() )
    {
      const QString nsURI( nsLen ? QString::fromUtf8( el, nsLen ) : QString() );
      const auto nsIter = mMapNamespaceURIToNamespacePrefix.constFind( nsURI );
      if ( !mCurrentXPathWithinFeature.isEmpty() )
        mCurrentXPathWithinFeature.append( '/' );
      if ( nsIter != mMapNamespaceURIToNamespacePrefix.constEnd() )
      {
        mCurrentXPathWithinFeature.append( *nsIter );
        mCurrentXPathWithinFeature.append( ':' );
      }
      mCurrentXPathWithinFeature.append( localName );
      const auto xpathIter = mMapXPathToFieldNameAndIsNestedContent.constFind( mCurrentXPathWithinFeature );
      mAttributeValIsNested = false;
      if ( xpathIter != mMapXPathToFieldNameAndIsNestedContent.end() )
      {
        mParseModeStack.push( QgsGmlStreamingParser::Attribute );
        mAttributeDepth = mParseDepth;
        mAttributeName = xpathIter->first;
        mAttributeValIsNested = xpathIter->second;
        if ( mAttributeValIsNested )
        {
          mAttributeJson = json::object();
          mAttributeJsonCurrentStack.clear();
          mAttributeJsonCurrentStack.push( &mAttributeJson );
        }
        mStringCash.clear();
      }
    }
    else if ( mThematicAttributes.contains( localName ) )
    {
      mParseModeStack.push( QgsGmlStreamingParser::Attribute );
      mAttributeDepth = mParseDepth;
      mAttributeName = localName;
      mStringCash.clear();
    }
    else
    {
      // QGIS server (2.2) is using:
      // <Attribute value="My description" name="desc"/>
      if ( localName.compare( "attribute"_L1, Qt::CaseInsensitive ) == 0 )
      {
        const QString name = readAttribute( u"name"_s, attr );
        if ( mThematicAttributes.contains( name ) )
        {
          const QString value = readAttribute( u"value"_s, attr );
          setAttribute( name, value );
        }
      }
    }
  }
  else if ( parseMode == Attribute && mAttributeValIsNested )
  {
    const std::string localName( pszLocalName, localNameLen );
    const QString nsURI( nsLen ? QString::fromUtf8( el, nsLen ) : QString() );
    const auto nsIter = mMapNamespaceURIToNamespacePrefix.constFind( nsURI );
    const std::string nodeName = nsIter != mMapNamespaceURIToNamespacePrefix.constEnd() ? ( *nsIter ).toStdString() + ':' + localName : localName;

    addStringContentToJson();

    auto &jsonParent = *( mAttributeJsonCurrentStack.top() );
    auto iter = jsonParent.find( nodeName );
    if ( iter != jsonParent.end() )
    {
      if ( iter->type() != json::value_t::array )
      {
        auto array = json::array();
        array.emplace_back( std::move( *iter ) );
        *iter = array;
      }
      iter->push_back( json::object() );
      mAttributeJsonCurrentStack.push( &( iter->back() ) );
    }
    else
    {
      auto res = jsonParent.emplace( nodeName, json::object() );
      // res.first is a json::iterator
      // Dereferencing it leads to a json reference
      // And taking a reference on it gets a pointer
      nlohmann::json *ptr = &( *( res.first ) );
      // cppcheck-suppress danglingLifetime
      mAttributeJsonCurrentStack.push( ptr );
    }
  }
  else if ( mParseDepth == 0 && LOCALNAME_EQUALS( "FeatureCollection" ) )
  {
    QString numberReturned = readAttribute( u"numberReturned"_s, attr ); // WFS 2.0
    if ( numberReturned.isEmpty() )
      numberReturned = readAttribute( u"numberOfFeatures"_s, attr ); // WFS 1.1
    bool conversionOk;
    mNumberReturned = numberReturned.toInt( &conversionOk );
    if ( !conversionOk )
      mNumberReturned = -1;

    const QString numberMatched = readAttribute( u"numberMatched"_s, attr ); // WFS 2.0
    mNumberMatched = numberMatched.toInt( &conversionOk );
    if ( !conversionOk ) // likely since numberMatched="unknown" is legal
      mNumberMatched = -1;
  }
  else if ( mParseDepth == 0 && LOCALNAME_EQUALS( "ExceptionReport" ) )
  {
    mIsException = true;
    mParseModeStack.push( QgsGmlStreamingParser::ExceptionReport );
  }
  else if ( mIsException &&  LOCALNAME_EQUALS( "ExceptionText" ) )
  {
    mStringCash.clear();
    mParseModeStack.push( QgsGmlStreamingParser::ExceptionText );
  }
  else if ( mParseDepth == 1 && LOCALNAME_EQUALS( "truncatedResponse" ) )
  {
    // e.g: http://services.cuzk.cz/wfs/inspire-cp-wfs.asp?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=cp:CadastralParcel
    mTruncatedResponse = true;
  }
  else if ( !mGeometryString.empty() &&
            !LOCALNAME_EQUALS( "exterior" ) &&
            !LOCALNAME_EQUALS( "interior" ) &&
            !LOCALNAME_EQUALS( "innerBoundaryIs" ) &&
            !LOCALNAME_EQUALS( "outerBoundaryIs" ) &&
            !LOCALNAME_EQUALS( "LinearRing" ) &&
            !LOCALNAME_EQUALS( "pointMember" ) &&
            !LOCALNAME_EQUALS( "curveMember" ) &&
            !LOCALNAME_EQUALS( "lineStringMember" ) &&
            !LOCALNAME_EQUALS( "polygonMember" ) &&
            !LOCALNAME_EQUALS( "surfaceMember" ) &&
            !LOCALNAME_EQUALS( "Curve" ) &&
            !LOCALNAME_EQUALS( "segments" ) &&
            !LOCALNAME_EQUALS( "LineStringSegment" ) )
  {
    //QgsDebugError( "Found unhandled geometry element " + QString::fromUtf8( pszLocalName, localNameLen ) );
    mFoundUnhandledGeometryElement = true;
  }

  // Handle XML attributes in XPath mode
  if ( !mParseModeStack.isEmpty() &&
       ( mParseModeStack.back() == Feature ||
         mParseModeStack.back() == Attribute ) &&
       !mMapXPathToFieldNameAndIsNestedContent.isEmpty() )
  {
    for ( const XML_Char **attrIter = attr; attrIter && *attrIter; attrIter += 2 )
    {
      const char *questionMark = strchr( attrIter[0], '?' );
      QString key( '@' );
      if ( questionMark )
      {
        const QString nsURI( QString::fromUtf8( attrIter[0], static_cast<int>( questionMark - attrIter[0] ) ) );
        const QString localName( QString::fromUtf8( questionMark + 1 ) );
        const auto nsIter = mMapNamespaceURIToNamespacePrefix.constFind( nsURI );
        if ( nsIter != mMapNamespaceURIToNamespacePrefix.constEnd() )
        {
          key.append( *nsIter );
          key.append( ':' );
        }
        key.append( localName );
      }
      else
      {
        const QString localName( QString::fromUtf8( attrIter[0] ) );
        key.append( localName );
      }

      if ( mAttributeValIsNested && mParseModeStack.back() == Attribute )
      {
        mAttributeJsonCurrentStack.top()->emplace(
          key.toStdString(),
          jsonFromString( QString::fromUtf8( attrIter[1] ) ) );
      }
      else
      {
        QString xpath( mCurrentXPathWithinFeature );
        if ( !xpath.isEmpty() )
          xpath.append( '/' );
        xpath.append( key );
        const auto xpathIter = mMapXPathToFieldNameAndIsNestedContent.constFind( xpath );
        if ( xpathIter != mMapXPathToFieldNameAndIsNestedContent.end() )
        {
          setAttribute( xpathIter->first, QString::fromUtf8( attrIter[1] ) );
        }
      }
    }
  }

  if ( !mGeometryString.empty() )
    isGeom = true;

  if ( isGeom )
  {
    elDimension = readSrsNameAndDimensionAttributes( attr );
  }

  if ( elDimension != 0 || mDimensionStack.isEmpty() )
  {
    mDimensionStack.push( elDimension );
  }
  else
  {
    mDimensionStack.push( mDimensionStack.back() );
  }

  mParseDepth ++;
}

void QgsGmlStreamingParser::endElement( const XML_Char *el )
{
  mParseDepth --;

  const int elLen = static_cast<int>( strlen( el ) );
  const char *pszSep = strchr( el, NS_SEPARATOR );
  const char *pszLocalName = ( pszSep ) ? pszSep + 1 : el;
  const int nsLen = ( pszSep ) ? ( int )( pszSep - el ) : 0;
  const int localNameLen = ( pszSep ) ? ( int )( elLen - nsLen ) - 1 : elLen;
  const ParseMode parseMode( mParseModeStack.isEmpty() ? None : mParseModeStack.top() );

  const int lastDimension = mDimensionStack.isEmpty() ? 2 : mDimensionStack.pop();

  const bool isGMLNS = ( nsLen == mGMLNameSpaceURI.size() && mGMLNameSpaceURIPtr && memcmp( el, mGMLNameSpaceURIPtr, nsLen ) == 0 );

  if ( parseMode == Feature || ( parseMode == Attribute && mAttributeDepth == mParseDepth ) )
  {
    if ( !mMapXPathToFieldNameAndIsNestedContent.isEmpty() )
    {
      const auto nPos = mCurrentXPathWithinFeature.lastIndexOf( '/' );
      if ( nPos < 0 )
        mCurrentXPathWithinFeature.clear();
      else
        mCurrentXPathWithinFeature.resize( nPos );
    }
  }

  if ( parseMode == Attribute && mAttributeValIsNested )
  {
    if ( !mStringCash.isEmpty() )
    {
      auto &jsonParent = *( mAttributeJsonCurrentStack.top() );
      if ( jsonParent.type() == json::value_t::object && jsonParent.empty() )
      {
        jsonParent = jsonFromString( mStringCash );
      }
      else if ( jsonParent.type() == json::value_t::object )
      {
        addStringContentToJson();
      }
      mStringCash.clear();
    }

    mAttributeJsonCurrentStack.pop();
  }

  if ( parseMode == Coordinate && isGMLNS && LOCALNAME_EQUALS( "coordinates" ) )
  {
    mParseModeStack.pop();
  }
  else if ( parseMode == PosList && isGMLNS &&
            ( LOCALNAME_EQUALS( "pos" ) || LOCALNAME_EQUALS( "posList" ) ) )
  {
    mDimension = lastDimension;
    mParseModeStack.pop();
  }
  else if ( parseMode == AttributeTuple &&
            mCurrentTypename + '|' + QString::fromUtf8( pszLocalName, localNameLen ) == mAttributeName ) //add a thematic attribute to the feature
  {
    mParseModeStack.pop();

    setAttribute( mAttributeName, mStringCash );
  }
  else if ( parseMode == Attribute && mAttributeDepth == mParseDepth ) //add a thematic attribute to the feature
  {
    mParseModeStack.pop();
    mParseDepth = -1;

    if ( mAttributeValIsNested )
    {
      mAttributeValIsNested = false;
      auto iter = mMapFieldNameToJSONContent.find( mAttributeName );
      if ( iter == mMapFieldNameToJSONContent.end() )
      {
        mMapFieldNameToJSONContent[mAttributeName] = QString::fromStdString( mAttributeJson.dump() );
      }
      else
      {
        QString &str = iter.value();
        if ( str[0] == '[' && str.back() == ']' )
        {
          str.back() = ',';
        }
        else
        {
          str.insert( 0, '[' );
          str.append( ',' );
        }
        str.append( QString::fromStdString( mAttributeJson.dump() ) );
        str.append( ']' );
      }
    }
    else
    {
      setAttribute( mAttributeName, mStringCash );
    }
  }
  else if ( parseMode == Geometry &&
            localNameLen == static_cast<int>( mGeometryAttributeUTF8Len ) &&
            memcmp( pszLocalName, mGeometryAttributePtr, localNameLen ) == 0 )
  {
    mParseModeStack.pop();
    if ( mFoundUnhandledGeometryElement )
    {
      const gdal::ogr_geometry_unique_ptr hGeom( OGR_G_CreateFromGML( mGeometryString.c_str() ) );
      //QgsDebugMsgLevel( u"for OGR: %1 -> %2"_s.arg(mGeometryString.c_str()).arg(hGeom != nullptr), 2);
      if ( hGeom )
      {
        const int wkbSize = OGR_G_WkbSize( hGeom.get() );
        unsigned char *pabyBuffer = new unsigned char[ wkbSize ];
        OGR_G_ExportToIsoWkb( hGeom.get(), wkbNDR, pabyBuffer );
        QgsGeometry g;
        g.fromWkb( pabyBuffer, wkbSize );
        if ( mInvertAxisOrientation )
        {
          g.transform( QTransform( 0, 1, 1, 0, 0, 0 ) );
        }
        Q_ASSERT( mCurrentFeature );
        mCurrentFeature->setGeometry( g );
      }
    }
    mGeometryString.clear();
  }
  else if ( parseMode == BoundingBox && isGMLNS && LOCALNAME_EQUALS( "boundedBy" ) )
  {
    //create bounding box from mStringCash
    if ( mCurrentExtent.isNull() &&
         !mBoundedByNullFound &&
         !createBBoxFromCoordinateString( mCurrentExtent, mStringCash ) )
    {
      QgsDebugError( u"creation of bounding box failed"_s );
    }
    if ( !mCurrentExtent.isNull() && mLayerExtent.isNull() &&
         !mCurrentFeature && mFeatureCount == 0 )
    {
      mLayerExtent = mCurrentExtent;
      mCurrentExtent = QgsRectangle();
    }

    mParseModeStack.pop();
  }
  else if ( parseMode == Null && isGMLNS && LOCALNAME_EQUALS( "null" ) )
  {
    mParseModeStack.pop();
  }
  else if ( parseMode == Envelope && isGMLNS && LOCALNAME_EQUALS( "Envelope" ) )
  {
    mParseModeStack.pop();
  }
  else if ( parseMode == LowerCorner && isGMLNS && LOCALNAME_EQUALS( "lowerCorner" ) )
  {
    QList<QgsPoint> points;
    pointsFromPosListString( points, mStringCash, 2 );
    if ( points.size() == 1 )
    {
      mCurrentExtent.setXMinimum( points[0].x() );
      mCurrentExtent.setYMinimum( points[0].y() );
    }
    mParseModeStack.pop();
  }
  else if ( parseMode == UpperCorner && isGMLNS && LOCALNAME_EQUALS( "upperCorner" ) )
  {
    QList<QgsPoint> points;
    pointsFromPosListString( points, mStringCash, 2 );
    if ( points.size() == 1 )
    {
      mCurrentExtent.setXMaximum( points[0].x() );
      mCurrentExtent.setYMaximum( points[0].y() );
    }
    mParseModeStack.pop();
  }
  else if ( parseMode == FeatureTuple && mParseDepth == mFeatureTupleDepth )
  {
    mParseModeStack.pop();
    mFeatureTupleDepth = 0;
  }
  else if ( ( parseMode == Tuple && !mTypeNamePtr &&
              LOCALNAME_EQUALS( "Tuple" ) ) ||
            ( parseMode == Feature &&
              localNameLen == static_cast<int>( mTypeNameUTF8Len ) &&
              memcmp( pszLocalName, mTypeNamePtr, mTypeNameUTF8Len ) == 0 ) )
  {
    Q_ASSERT( mCurrentFeature );
    if ( !mCurrentFeature->hasGeometry() )
    {
      if ( mCurrentWKB.size() > 0 )
      {
        QgsGeometry g;
        g.fromWkb( mCurrentWKB );
        mCurrentFeature->setGeometry( g );
        mCurrentWKB = QByteArray();
      }
      else if ( !mCurrentExtent.isEmpty() )
      {
        mCurrentFeature->setGeometry( QgsGeometry::fromRect( mCurrentExtent ) );
      }
    }
    mCurrentFeature->setValid( true );

    for ( auto iter = mMapFieldNameToJSONContent.constBegin(); iter != mMapFieldNameToJSONContent.constEnd(); ++iter )
    {
      const QMap<QString, QPair<int, QgsField> >::const_iterator att_it = mThematicAttributes.constFind( iter.key() );
      const int attrIndex = att_it.value().first;
      mCurrentFeature->setAttribute( attrIndex, iter.value() );
    }
    mMapFieldNameToJSONContent.clear();

    mFeatureList.push_back( QgsGmlFeaturePtrGmlIdPair( mCurrentFeature, mCurrentFeatureId ) );

    mCurrentFeature = nullptr;
    ++mFeatureCount;
    mParseModeStack.pop();
  }
  else if ( !mAttributeValIsNested && isGMLNS && LOCALNAME_EQUALS( "Point" ) )
  {
    QList<QgsPoint> pointList;
    int dimension = 0;
    if ( pointsFromString( pointList, mStringCash, &dimension ) != 0 )
    {
      //error
    }
    mStringCash.clear();
    mDimension = dimension;

    if ( pointList.isEmpty() )
      return;  // error

    if ( parseMode == QgsGmlStreamingParser::Geometry )
    {
      //directly add WKB point to the feature
      if ( getPointWKB( mCurrentWKB, *( pointList.constBegin() ), dimension ) != 0 )
      {
        //error
      }

      if ( QgsWkbTypes::flatType( mWkbType ) != Qgis::WkbType::MultiPoint ) //keep multitype in case of geometry type mix
      {
        mWkbType = dimension > 2 ? Qgis::WkbType::PointZ : Qgis::WkbType::Point;
      }

    }
    else //multipoint, add WKB as fragment
    {
      QByteArray wkbPtr;
      if ( getPointWKB( wkbPtr, *( pointList.constBegin() ), dimension ) != 0 )
      {
        //error
      }
      if ( !mCurrentWKBFragments.isEmpty() )
      {
        mCurrentWKBFragments.last().push_back( wkbPtr );
      }
      else
      {
        QgsDebugError( u"No wkb fragments"_s );
      }
    }
  }
  else if ( !mAttributeValIsNested &&
            isGMLNS && ( LOCALNAME_EQUALS( "LineString" ) || LOCALNAME_EQUALS( "LineStringSegment" ) ) )
  {
    //add WKB point to the feature

    QList<QgsPoint> pointList;
    int dimension = 0;
    if ( pointsFromString( pointList, mStringCash, &dimension ) != 0 )
    {
      //error
    }
    mStringCash.clear();
    mDimension = dimension;

    if ( parseMode == QgsGmlStreamingParser::Geometry )
    {
      if ( getLineWKB( mCurrentWKB, pointList, dimension ) != 0 )
      {
        //error
      }

      if ( QgsWkbTypes::flatType( mWkbType ) != Qgis::WkbType::MultiLineString )//keep multitype in case of geometry type mix
      {
        mWkbType = dimension > 2 ? Qgis::WkbType::LineStringZ : Qgis::WkbType::LineString;
      }
      else if ( dimension > 2 )
      {
        mWkbType = Qgis::WkbType::MultiLineStringZ;
      }
      mDimension = dimension;
    }
    else //multiline, add WKB as fragment
    {
      QByteArray wkbPtr;
      if ( getLineWKB( wkbPtr, pointList, dimension ) != 0 )
      {
        //error
      }
      mDimension = dimension;
      if ( !mCurrentWKBFragments.isEmpty() )
      {
        mCurrentWKBFragments.last().push_back( wkbPtr );
      }
      else
      {
        QgsDebugError( u"no wkb fragments"_s );
      }
    }
  }
  else if ( ( parseMode == Geometry || parseMode == MultiPolygon ) &&
            isGMLNS && LOCALNAME_EQUALS( "LinearRing" ) )
  {
    QList<QgsPoint> pointList;
    int dimension = 0;
    if ( pointsFromString( pointList, mStringCash, &dimension ) != 0 )
    {
      //error
    }
    mStringCash.clear();
    mDimension = dimension;

    QByteArray wkbPtr;
    if ( getRingWKB( wkbPtr, pointList, dimension ) != 0 )
    {
      //error
    }

    if ( !mCurrentWKBFragments.isEmpty() )
    {
      mCurrentWKBFragments.last().push_back( wkbPtr );
    }
    else
    {
      QgsDebugError( u"no wkb fragments"_s );
    }
  }
  else if ( ( parseMode == Geometry || parseMode == MultiPolygon ) && isGMLNS &&
            LOCALNAME_EQUALS( "Polygon" ) )
  {
    if ( QgsWkbTypes::flatType( mWkbType ) != Qgis::WkbType::MultiPolygon ) //keep multitype in case of geometry type mix
    {
      mWkbType = mDimension > 2 ? Qgis::WkbType::PolygonZ : Qgis::WkbType::Polygon;
    }

    if ( parseMode == Geometry )
    {
      createPolygonFromFragments();
    }
  }
  else if ( parseMode == MultiPoint &&  isGMLNS &&
            LOCALNAME_EQUALS( "MultiPoint" ) )
  {
    mWkbType = mDimension > 2 ? Qgis::WkbType::MultiPointZ : Qgis::WkbType::MultiPoint;
    mParseModeStack.pop();
    createMultiPointFromFragments();
  }
  else if ( parseMode == MultiLine && isGMLNS &&
            ( LOCALNAME_EQUALS( "MultiLineString" )  || LOCALNAME_EQUALS( "MultiCurve" ) ) )
  {
    mWkbType = mDimension > 2 ? Qgis::WkbType::MultiLineStringZ : Qgis::WkbType::MultiLineString;
    mParseModeStack.pop();
    createMultiLineFromFragments();
  }
  else if ( parseMode == MultiPolygon && isGMLNS &&
            ( LOCALNAME_EQUALS( "MultiPolygon" )  || LOCALNAME_EQUALS( "MultiSurface" ) ) )
  {
    mWkbType = mDimension > 2 ? Qgis::WkbType::MultiPolygonZ : Qgis::WkbType::MultiPolygon;
    mParseModeStack.pop();
    createMultiPolygonFromFragments();
  }
  else if ( mParseDepth == 0 && LOCALNAME_EQUALS( "ExceptionReport" ) )
  {
    mParseModeStack.pop();
  }
  else if ( parseMode == ExceptionText && LOCALNAME_EQUALS( "ExceptionText" ) )
  {
    mExceptionText = mStringCash;
    mParseModeStack.pop();
  }

  if ( !mGeometryString.empty() )
  {
    mGeometryString.append( "</", 2 );
    mGeometryString.append( pszLocalName, localNameLen );
    mGeometryString.append( ">", 1 );
  }

}

void QgsGmlStreamingParser::characters( const XML_Char *chars, int len )
{
  //save chars in mStringCash attribute mode or coordinate mode
  if ( mParseModeStack.isEmpty() )
  {
    return;
  }

  if ( !mGeometryString.empty() )
  {
    mGeometryString.append( chars, len );
  }

  const QgsGmlStreamingParser::ParseMode parseMode = mParseModeStack.top();
  if ( parseMode == QgsGmlStreamingParser::Attribute ||
       parseMode == QgsGmlStreamingParser::AttributeTuple ||
       parseMode == QgsGmlStreamingParser::Coordinate ||
       parseMode == QgsGmlStreamingParser::PosList ||
       parseMode == QgsGmlStreamingParser::LowerCorner ||
       parseMode == QgsGmlStreamingParser::UpperCorner ||
       parseMode == QgsGmlStreamingParser::ExceptionText )
  {
    mStringCash.append( QString::fromUtf8( chars, len ) );
  }
}

void QgsGmlStreamingParser::addStringContentToJson()
{
  const QString s( mStringCash.trimmed() );
  if ( !s.isEmpty() )
  {
    auto &jsonParent = *( mAttributeJsonCurrentStack.top() );
    auto textIter = jsonParent.find( "_text" );
    if ( textIter != jsonParent.end() )
    {
      if ( textIter->type() != json::value_t::array )
      {
        auto array = json::array();
        array.emplace_back( std::move( *textIter ) );
        *textIter = array;
      }
      textIter->emplace_back( jsonFromString( s ) );
    }
    else
    {
      jsonParent.emplace( "_text", jsonFromString( s ) );
    }
  }
  mStringCash.clear();
}

void QgsGmlStreamingParser::setAttribute( const QString &name, const QString &value )
{
  //find index with attribute name
  const QMap<QString, QPair<int, QgsField> >::const_iterator att_it = mThematicAttributes.constFind( name );
  bool conversionOk = true;
  if ( att_it != mThematicAttributes.constEnd() )
  {
    QVariant var;
    switch ( att_it.value().second.type() )
    {
      case QMetaType::Type::Double:
        var = QVariant( value.toDouble( &conversionOk ) );
        break;
      case QMetaType::Type::Int:
        var = QVariant( value.toInt( &conversionOk ) );
        break;
      case QMetaType::Type::LongLong:
        var = QVariant( value.toLongLong( &conversionOk ) );
        break;
      case QMetaType::Type::QDateTime:
        var = QVariant( QDateTime::fromString( value, Qt::ISODate ) );
        break;
      default: //string type is default
        var = QVariant( value );
        break;
    }
    if ( ! conversionOk )  // Assume is NULL
    {
      var = QVariant();
    }
    Q_ASSERT( mCurrentFeature );
    mCurrentFeature->setAttribute( att_it.value().first, var );
  }
}

int QgsGmlStreamingParser::readSrsNameAndDimensionAttributes( const XML_Char **attr )
{
  int elDimension = 0;
  for ( int i = 0; attr[i] && attr[i + 1]; i += 2 )
  {
    if ( strcmp( attr[i], "srsName" ) == 0 )
    {
      const QString srsName( attr[i + 1] );
      if ( mSrsName.isEmpty() )
      {
        QString authority;
        QString code;
        const QgsOgcCrsUtils::CRSFlavor crsFlavor = QgsOgcCrsUtils::parseCrsName( srsName, authority, code );
        if ( crsFlavor != QgsOgcCrsUtils::CRSFlavor::UNKNOWN )
        {
          const bool bIsUrn = ( crsFlavor == QgsOgcCrsUtils::CRSFlavor::OGC_URN ||
                                crsFlavor == QgsOgcCrsUtils::CRSFlavor::X_OGC_URN ||
                                crsFlavor == QgsOgcCrsUtils::CRSFlavor::OGC_HTTP_URI );
          const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( srsName );
          if ( crs.isValid() )
          {
            mSrsName = srsName;
            if ( ( ( mAxisOrientationLogic == Honour_EPSG_if_urn && bIsUrn ) ||
                   mAxisOrientationLogic == Honour_EPSG ) && crs.hasAxisInverted() )
            {
              mInvertAxisOrientation = !mInvertAxisOrientationRequest;
            }

            mDimensionForCurSrsName = crs.hasVerticalAxis() ? 3 : 2;
            if ( elDimension == 0 )
              elDimension = mDimensionForCurSrsName;
          }
        }
      }
      else if ( srsName == mSrsName && elDimension == 0 )
      {
        elDimension = mDimensionForCurSrsName;
      }
    }
    else if ( strcmp( attr[i], "srsDimension" ) == 0 )
    {
      const QString srsDimension( attr[i + 1] );
      bool ok = false;
      const int dimension = srsDimension.toInt( &ok );
      if ( ok )
      {
        elDimension = dimension;
      }
    }
  }

  return elDimension;
}

QString QgsGmlStreamingParser::readAttribute( const QString &attributeName, const XML_Char **attr ) const
{
  int i = 0;
  while ( attr[i] )
  {
    if ( attributeName.compare( attr[i] ) == 0 )
    {
      return QString::fromUtf8( attr[i + 1] );
    }
    i += 2;
  }
  return QString();
}

bool QgsGmlStreamingParser::createBBoxFromCoordinateString( QgsRectangle &r, const QString &coordString ) const
{
  QList<QgsPoint> points;
  if ( pointsFromCoordinateString( points, coordString ) != 0 )
  {
    return false;
  }

  if ( points.size() < 2 )
  {
    return false;
  }

  r.set( points[0], points[1] );

  return true;
}

int QgsGmlStreamingParser::pointsFromCoordinateString( QList<QgsPoint> &points, const QString &coordString, int *dimension ) const
{
  //tuples are separated by space, x/y by ','
  const QStringList tuples = coordString.split( mTupleSeparator, Qt::SkipEmptyParts );
  QStringList tuples_coordinates;
  double x, y, z;
  bool conversionSuccess;

  if ( dimension )
    *dimension = 0;

  QStringList::const_iterator tupleIterator;
  for ( tupleIterator = tuples.constBegin(); tupleIterator != tuples.constEnd(); ++tupleIterator )
  {
    tuples_coordinates = tupleIterator->split( mCoordinateSeparator, Qt::SkipEmptyParts );
    if ( dimension )
    {
      *dimension = std::max( *dimension, static_cast<int>( tuples_coordinates.size() ) );
    }
    if ( tuples_coordinates.size() < 2 )
    {
      continue;
    }
    x = tuples_coordinates.at( 0 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      continue;
    }
    y = tuples_coordinates.at( 1 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      continue;
    }
    if ( tuples_coordinates.size() > 2 )
    {
      z = tuples_coordinates.at( 2 ).toDouble( &conversionSuccess );
      if ( !conversionSuccess )
      {
        continue;
      }
    }
    else
    {
      z = std::numeric_limits<double>::quiet_NaN(); // no Z coordinate
    }
    points.push_back( ( mInvertAxisOrientation ) ? QgsPoint( y, x, z ) : QgsPoint( x, y, z ) );
  }

  if ( dimension && *dimension == 0 )
  {
    *dimension = 2; // default dimension is 2D
  }

  return 0;
}

int QgsGmlStreamingParser::pointsFromPosListString( QList<QgsPoint> &points, const QString &coordString, int dimension ) const
{
  // coordinates separated by spaces
  const QStringList coordinates = coordString.split( ' ', Qt::SkipEmptyParts );

  if ( coordinates.size() % dimension != 0 )
  {
    QgsDebugError( u"Wrong number of coordinates"_s );
  }

  const int ncoor = coordinates.size() / dimension;
  for ( int i = 0; i < ncoor; i++ )
  {
    bool conversionSuccess;
    const double x = coordinates.value( i * dimension ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      continue;
    }
    const double y = coordinates.value( i * dimension + 1 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      continue;
    }
    double z = std::numeric_limits<double>::quiet_NaN();
    if ( dimension > 2 )
    {
      z = coordinates.value( i * dimension + 2 ).toDouble( &conversionSuccess );
      if ( !conversionSuccess )
      {
        continue;
      }
    }
    points.append( mInvertAxisOrientation ? QgsPoint( y, x, z ) : QgsPoint( x, y, z ) );
  }
  return 0;
}

int QgsGmlStreamingParser::pointsFromString( QList<QgsPoint> &points, const QString &coordString, int *dimension ) const
{
  if ( mCoorMode == QgsGmlStreamingParser::Coordinate )
  {
    return pointsFromCoordinateString( points, coordString, dimension );
  }
  else if ( mCoorMode == QgsGmlStreamingParser::PosList )
  {
    if ( dimension )
    {
      *dimension = mDimension ? mDimension : 2; // default dimension is 2D
    }
    return pointsFromPosListString( points, coordString, mDimension ? mDimension : 2 );
  }
  return 1;
}

int QgsGmlStreamingParser::getPointWKB( QByteArray &wkbPtr, const QgsPoint &point, int dimension ) const
{
  const int wkbSize = 1 + static_cast<int>( sizeof( int ) ) + dimension * static_cast<int>( sizeof( double ) );
  wkbPtr = QByteArray( wkbSize, Qt::Uninitialized );

  QgsWkbPtr fillPtr( wkbPtr );
  fillPtr << mEndian << ( dimension > 2 ? Qgis::WkbType::PointZ : Qgis::WkbType::Point ) << point.x() << point.y();
  if ( dimension > 2 )
  {
    fillPtr << point.z(); // add Z coordinate if available
  }

  return 0;
}

int QgsGmlStreamingParser::getLineWKB( QByteArray &wkbPtr, const QList<QgsPoint> &lineCoordinates, int dimension ) const
{
  const int wkbSize = 1 + 2 * static_cast<int>( sizeof( int ) ) + static_cast<int>( lineCoordinates.size() ) * dimension * static_cast<int>( sizeof( double ) );
  wkbPtr = QByteArray( wkbSize, Qt::Uninitialized );

  QgsWkbPtr fillPtr( wkbPtr );

  fillPtr << mEndian << ( dimension > 2 ? Qgis::WkbType::LineStringZ : Qgis::WkbType::LineString ) << lineCoordinates.size();

  QList<QgsPoint>::const_iterator iter;
  for ( iter = lineCoordinates.constBegin(); iter != lineCoordinates.constEnd(); ++iter )
  {
    fillPtr << iter->x() << iter->y();
    if ( dimension > 2 )
    {
      fillPtr << iter->z(); // add Z coordinate if available
    }
  }

  return 0;
}

int QgsGmlStreamingParser::getRingWKB( QByteArray &wkbPtr, const QList<QgsPoint> &ringCoordinates, int dimension ) const
{
  const int wkbSize = static_cast<int>( sizeof( int ) ) + static_cast<int>( ringCoordinates.size() ) * dimension * static_cast<int>( sizeof( double ) );
  wkbPtr = QByteArray( wkbSize, Qt::Uninitialized );

  QgsWkbPtr fillPtr( wkbPtr );

  fillPtr << ringCoordinates.size();

  QList<QgsPoint>::const_iterator iter;
  for ( iter = ringCoordinates.constBegin(); iter != ringCoordinates.constEnd(); ++iter )
  {
    fillPtr << iter->x() << iter->y();
    if ( dimension > 2 )
    {
      fillPtr << iter->z(); // add Z coordinate if available
    }
  }

  return 0;
}

int QgsGmlStreamingParser::createMultiLineFromFragments()
{
  const int size = 1 + ( mDimension > 2 ? mDimension : 2 ) * static_cast<int>( sizeof( int ) ) + totalWKBFragmentSize();
  mCurrentWKB = QByteArray( size, Qt::Uninitialized );

  QgsWkbPtr wkbPtr( mCurrentWKB );

  wkbPtr << mEndian << ( mDimension > 2 ? Qgis::WkbType::MultiLineStringZ : Qgis::WkbType::MultiLineString ) << mCurrentWKBFragments.constBegin()->size();

  //copy (and delete) all the wkb fragments
  auto wkbIt = mCurrentWKBFragments.constBegin()->constBegin();
  for ( ; wkbIt != mCurrentWKBFragments.constBegin()->constEnd(); ++wkbIt )
  {
    memcpy( wkbPtr, *wkbIt, wkbIt->size() );
    wkbPtr += wkbIt->size();
  }

  mCurrentWKBFragments.clear();
  mWkbType = mDimension > 2 ? Qgis::WkbType::MultiLineStringZ : Qgis::WkbType::MultiLineString;
  return 0;
}

int QgsGmlStreamingParser::createMultiPointFromFragments()
{
  const int size = 1 + ( mDimension > 2 ? mDimension : 2 ) * static_cast<int>( sizeof( int ) ) + totalWKBFragmentSize();
  mCurrentWKB = QByteArray( size, Qt::Uninitialized );

  QgsWkbPtr wkbPtr( mCurrentWKB );
  wkbPtr << mEndian << ( mDimension > 2 ? Qgis::WkbType::MultiPointZ : Qgis::WkbType::MultiPoint ) << mCurrentWKBFragments.constBegin()->size();

  auto wkbIt = mCurrentWKBFragments.constBegin()->constBegin();
  for ( ; wkbIt != mCurrentWKBFragments.constBegin()->constEnd(); ++wkbIt )
  {
    memcpy( wkbPtr, *wkbIt, wkbIt->size() );
    wkbPtr += wkbIt->size();
  }

  mCurrentWKBFragments.clear();
  mWkbType = mDimension > 2 ? Qgis::WkbType::MultiPointZ : Qgis::WkbType::MultiPoint;
  return 0;
}


int QgsGmlStreamingParser::createPolygonFromFragments()
{
  const int size = 1 + ( mDimension > 2 ? mDimension : 2 ) * static_cast<int>( sizeof( int ) ) + totalWKBFragmentSize();
  mCurrentWKB = QByteArray( size, Qt::Uninitialized );

  QgsWkbPtr wkbPtr( mCurrentWKB );
  wkbPtr << mEndian << ( mDimension > 2 ? Qgis::WkbType::PolygonZ : Qgis::WkbType::Polygon ) << mCurrentWKBFragments.constBegin()->size();

  auto wkbIt = mCurrentWKBFragments.constBegin()->constBegin();
  for ( ; wkbIt != mCurrentWKBFragments.constBegin()->constEnd(); ++wkbIt )
  {
    memcpy( wkbPtr, *wkbIt, wkbIt->size() );
    wkbPtr += wkbIt->size();
  }

  mCurrentWKBFragments.clear();
  mWkbType = mDimension > 2 ? Qgis::WkbType::PolygonZ : Qgis::WkbType::Polygon;
  return 0;
}

int QgsGmlStreamingParser::createMultiPolygonFromFragments()
{
  int size = 0;
  size += 1 + ( mDimension > 2 ? mDimension : 2 ) * static_cast<int>( sizeof( int ) );
  size += totalWKBFragmentSize();
  size += mCurrentWKBFragments.size() * ( 1 + ( mDimension > 2 ? mDimension : 2 ) * static_cast<int>( sizeof( int ) ) ); //fragments are just the rings

  mCurrentWKB = QByteArray( size, Qt::Uninitialized );

  QgsWkbPtr wkbPtr( mCurrentWKB );
  wkbPtr << ( char ) mEndian << ( mDimension > 2 ? Qgis::WkbType::MultiPolygonZ : Qgis::WkbType::MultiPolygon ) << mCurrentWKBFragments.size();

  //have outer and inner iterators
  auto outerWkbIt = mCurrentWKBFragments.constBegin();

  for ( ; outerWkbIt != mCurrentWKBFragments.constEnd(); ++outerWkbIt )
  {
    //new polygon
    wkbPtr << ( char ) mEndian << ( mDimension > 2 ? Qgis::WkbType::PolygonZ : Qgis::WkbType::Polygon ) << outerWkbIt->size();

    auto innerWkbIt = outerWkbIt->constBegin();
    for ( ; innerWkbIt != outerWkbIt->constEnd(); ++innerWkbIt )
    {
      memcpy( wkbPtr, *innerWkbIt, innerWkbIt->size() );
      wkbPtr += innerWkbIt->size();
    }
  }

  mCurrentWKBFragments.clear();
  mWkbType =  mDimension > 2 ? Qgis::WkbType::MultiPolygonZ : Qgis::WkbType::MultiPolygon;
  return 0;
}

int QgsGmlStreamingParser::totalWKBFragmentSize() const
{
  int result = 0;
  for ( const QList<QByteArray> &list : std::as_const( mCurrentWKBFragments ) )
  {
    for ( const QByteArray &i : list )
    {
      result += i.size();
    }
  }
  return result;
}

void QgsGmlStreamingParser::createParser( const QByteArray &encoding )
{
  Q_ASSERT( !mParser );

  mParser = XML_ParserCreateNS( encoding.isEmpty() ? nullptr : encoding.data(), NS_SEPARATOR );
  XML_SetUserData( mParser, this );
  XML_SetElementHandler( mParser, QgsGmlStreamingParser::start, QgsGmlStreamingParser::end );
  XML_SetCharacterDataHandler( mParser, QgsGmlStreamingParser::chars );
}
