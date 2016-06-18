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
#include "qgsauthmanager.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgswkbptr.h"
#include "qgscrscache.h"

#include <QBuffer>
#include <QList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QSet>
#include <QSettings>
#include <QUrl>

#include "ogr_api.h"

#include <limits>

static const char NS_SEPARATOR = '?';
static const char* GML_NAMESPACE = "http://www.opengis.net/gml";
static const char* GML32_NAMESPACE = "http://www.opengis.net/gml/3.2";

QgsGml::QgsGml(
  const QString& typeName,
  const QString& geometryAttribute,
  const QgsFields & fields )
    : QObject()
    , mParser( typeName, geometryAttribute, fields )
    , mTypeName( typeName )
    , mFinished( false )
{
  int index = mTypeName.indexOf( ':' );
  if ( index != -1 && index < mTypeName.length() )
  {
    mTypeName = mTypeName.mid( index + 1 );
  }
}

QgsGml::~QgsGml()
{
}

int QgsGml::getFeatures( const QString& uri, QGis::WkbType* wkbType, QgsRectangle* extent, const QString& userName, const QString& password , const QString& authcfg )
{
  //start with empty extent
  mExtent.setMinimal();

  QNetworkRequest request( uri );
  if ( !authcfg.isEmpty() )
  {
    if ( !QgsAuthManager::instance()->updateNetworkRequest( request, authcfg ) )
    {
      QgsMessageLog::logMessage(
        tr( "GML Getfeature network request update failed for authcfg %1" ).arg( authcfg ),
        tr( "Network" ),
        QgsMessageLog::CRITICAL
      );
      return 1;
    }
  }
  else if ( !userName.isNull() || !password.isNull() )
  {
    request.setRawHeader( "Authorization", "Basic " + QString( "%1:%2" ).arg( userName, password ).toAscii().toBase64() );
  }
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->get( request );

  connect( reply, SIGNAL( finished() ), this, SLOT( setFinished() ) );
  connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( handleProgressEvent( qint64, qint64 ) ) );

  //find out if there is a QGIS main window. If yes, display a progress dialog
  QProgressDialog* progressDialog = nullptr;
  QWidget* mainWindow = nullptr;
  QWidgetList topLevelWidgets = qApp->topLevelWidgets();
  for ( QWidgetList::const_iterator it = topLevelWidgets.constBegin(); it != topLevelWidgets.constEnd(); ++it )
  {
    if (( *it )->objectName() == "QgisApp" )
    {
      mainWindow = *it;
      break;
    }
  }
  if ( mainWindow )
  {
    progressDialog = new QProgressDialog( tr( "Loading GML data\n%1" ).arg( mTypeName ), tr( "Abort" ), 0, 0, mainWindow );
    progressDialog->setWindowModality( Qt::ApplicationModal );
    connect( this, SIGNAL( dataReadProgress( int ) ), progressDialog, SLOT( setValue( int ) ) );
    connect( this, SIGNAL( totalStepsUpdate( int ) ), progressDialog, SLOT( setMaximum( int ) ) );
    connect( progressDialog, SIGNAL( canceled() ), this, SLOT( setFinished() ) );
    progressDialog->show();
  }

  int atEnd = 0;
  while ( !atEnd )
  {
    if ( mFinished )
    {
      atEnd = 1;
    }
    QByteArray readData = reply->readAll();
    if ( !readData.isEmpty() )
    {
      QString errorMsg;
      if ( !mParser.processData( readData, atEnd, errorMsg ) )
        QgsMessageLog::logMessage( errorMsg, QObject::tr( "WFS" ) );

    }
    QCoreApplication::processEvents();
  }

  fillMapsFromParser();

  QNetworkReply::NetworkError replyError = reply->error();
  QString replyErrorString = reply->errorString();

  delete reply;
  delete progressDialog;

  if ( replyError )
  {
    QgsMessageLog::logMessage(
      tr( "GML Getfeature network request failed with error: %1" ).arg( replyErrorString ),
      tr( "Network" ),
      QgsMessageLog::CRITICAL
    );
    return 1;
  }

  *wkbType = mParser.wkbType();

  if ( *wkbType != QGis::WKBNoGeometry )
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

int QgsGml::getFeatures( const QByteArray &data, QGis::WkbType* wkbType, QgsRectangle* extent )
{
  mExtent.setMinimal();

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
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = mParser.getAndStealReadyFeatures();
  Q_FOREACH ( const QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair& featPair, features )
  {
    QgsFeature* feat = featPair.first;
    const QString& gmlId = featPair.second;
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
  if ( mFeatures.size() < 1 )
  {
    return;
  }

  QgsFeature* currentFeature = nullptr;
  const QgsGeometry* currentGeometry = nullptr;
  bool bboxInitialised = false; //gets true once bbox has been set to the first geometry

  for ( int i = 0; i < mFeatures.size(); ++i )
  {
    currentFeature = mFeatures[i];
    if ( !currentFeature )
    {
      continue;
    }
    currentGeometry = currentFeature->constGeometry();
    if ( currentGeometry )
    {
      if ( !bboxInitialised )
      {
        mExtent = currentGeometry->boundingBox();
        bboxInitialised = true;
      }
      else
      {
        mExtent.unionRect( currentGeometry->boundingBox() );
      }
    }
  }
}

QgsCoordinateReferenceSystem QgsGml::crs() const
{
  QgsCoordinateReferenceSystem crs;
  if ( mParser.getEPSGCode() != 0 )
  {
    crs = QgsCRSCache::instance()->crsByOgcWmsCrs( QString( "EPSG:%1" ).arg( mParser.getEPSGCode() ) );
  }
  return crs;
}





QgsGmlStreamingParser::QgsGmlStreamingParser( const QString& typeName,
    const QString& geometryAttribute,
    const QgsFields & fields,
    AxisOrientationLogic axisOrientationLogic,
    bool invertAxisOrientation )
    : mTypeName( typeName )
    , mTypeNameBA( mTypeName.toUtf8() )
    , mTypeNamePtr( mTypeNameBA.constData() )
    , mWkbType( QGis::WKBUnknown )
    , mGeometryAttribute( geometryAttribute )
    , mGeometryAttributeBA( geometryAttribute.toUtf8() )
    , mGeometryAttributePtr( mGeometryAttributeBA.constData() )
    , mFields( fields )
    , mIsException( false )
    , mTruncatedResponse( false )
    , mParseDepth( 0 )
    , mFeatureTupleDepth( 0 )
    , mCurrentFeature( nullptr )
    , mFeatureCount( 0 )
    , mCurrentWKB( nullptr, 0 )
    , mBoundedByNullFound( false )
    , mDimension( 0 )
    , mCoorMode( coordinate )
    , mEpsg( 0 )
    , mGMLNameSpaceURIPtr( nullptr )
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
    mThematicAttributes.insert( fields[i].name(), qMakePair( i, fields[i] ) );
  }

  mEndian = QgsApplication::endian();

  int index = mTypeName.indexOf( ':' );
  if ( index != -1 && index < mTypeName.length() )
  {
    mTypeName = mTypeName.mid( index + 1 );
    mTypeNameBA = mTypeName.toUtf8();
    mTypeNamePtr = mTypeNameBA.constData();
  }

  mParser = XML_ParserCreateNS( nullptr, NS_SEPARATOR );
  XML_SetUserData( mParser, this );
  XML_SetElementHandler( mParser, QgsGmlStreamingParser::start, QgsGmlStreamingParser::end );
  XML_SetCharacterDataHandler( mParser, QgsGmlStreamingParser::chars );
}

static QString stripNS( const QString& string )
{
  int index = string.indexOf( ':' );
  if ( index != -1 && index < string.length() )
  {
    return string.mid( index + 1 );
  }
  return string;
}

QgsGmlStreamingParser::QgsGmlStreamingParser( const QList<LayerProperties>& layerProperties,
    const QgsFields & fields,
    const QMap< QString, QPair<QString, QString> >& mapFieldNameToSrcLayerNameFieldName,
    AxisOrientationLogic axisOrientationLogic,
    bool invertAxisOrientation )
    : mLayerProperties( layerProperties )
    , mTypeNamePtr( nullptr )
    , mWkbType( QGis::WKBUnknown )
    , mGeometryAttributePtr( nullptr )
    , mFields( fields )
    , mIsException( false )
    , mTruncatedResponse( false )
    , mParseDepth( 0 )
    , mFeatureTupleDepth( 0 )
    , mCurrentFeature( nullptr )
    , mFeatureCount( 0 )
    , mCurrentWKB( nullptr, 0 )
    , mBoundedByNullFound( false )
    , mDimension( 0 )
    , mCoorMode( coordinate )
    , mEpsg( 0 )
    , mGMLNameSpaceURIPtr( nullptr )
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
    QMap< QString, QPair<QString, QString> >::const_iterator att_it = mapFieldNameToSrcLayerNameFieldName.constFind( fields[i].name() );
    if ( att_it != mapFieldNameToSrcLayerNameFieldName.constEnd() )
    {
      if ( mLayerProperties.size() == 1 )
        mThematicAttributes.insert( att_it.value().second, qMakePair( i, fields[i] ) );
      else
        mThematicAttributes.insert( stripNS( att_it.value().first ) + "|" + att_it.value().second, qMakePair( i, fields[i] ) );
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
        QgsDebugMsg( QString( "Will ignore geometry field %1 from typename %2" ).
                     arg( mLayerProperties[i].mGeometryAttribute ).arg( mLayerProperties[i].mName ) );
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
    int index = mTypeName.indexOf( ':' );
    if ( index != -1 && index < mTypeName.length() )
    {
      mTypeName = mTypeName.mid( index + 1 );
    }
    mTypeNameBA = mTypeName.toUtf8();
    mTypeNamePtr = mTypeNameBA.constData();
  }

  mEndian = QgsApplication::endian();

  mParser = XML_ParserCreateNS( nullptr, NS_SEPARATOR );
  XML_SetUserData( mParser, this );
  XML_SetElementHandler( mParser, QgsGmlStreamingParser::start, QgsGmlStreamingParser::end );
  XML_SetCharacterDataHandler( mParser, QgsGmlStreamingParser::chars );
}


QgsGmlStreamingParser::~QgsGmlStreamingParser()
{
  XML_ParserFree( mParser );

  // Normally a sane user of this class should have consumed everything...
  Q_FOREACH ( QgsGmlFeaturePtrGmlIdPair featPair, mFeatureList )
  {
    delete featPair.first;
  }

  delete mCurrentFeature;
}

bool QgsGmlStreamingParser::processData( const QByteArray& data, bool atEnd )
{
  QString errorMsg;
  if ( !processData( data, atEnd, errorMsg ) )
  {
    QgsMessageLog::logMessage( errorMsg, QObject::tr( "WFS" ) );
    return false;
  }
  return true;
}

bool QgsGmlStreamingParser::processData( const QByteArray& data, bool atEnd, QString& errorMsg )
{
  if ( XML_Parse( mParser, data.data(), data.size(), atEnd ) == 0 )
  {
    XML_Error errorCode = XML_GetErrorCode( mParser );
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

#define LOCALNAME_EQUALS(string_constant) \
  ( localNameLen == strlen( string_constant ) && memcmp(pszLocalName, string_constant, localNameLen) == 0 )

void QgsGmlStreamingParser::startElement( const XML_Char* el, const XML_Char** attr )
{
  const int elLen = ( int )strlen( el );
  const char* pszSep = strchr( el, NS_SEPARATOR );
  const char* pszLocalName = ( pszSep ) ? pszSep + 1 : el;
  const int nsLen = ( pszSep ) ? ( int )( pszSep - el ) : 0;
  const int localNameLen = ( pszSep ) ? ( int )( elLen - nsLen ) - 1 : elLen;
  ParseMode theParseMode( mParseModeStack.isEmpty() ? none : mParseModeStack.top() );

  // Figure out if the GML namespace is GML_NAMESPACE or GML32_NAMESPACE
  if ( !mGMLNameSpaceURIPtr && pszSep )
  {
    if ( nsLen == ( int )strlen( GML_NAMESPACE ) && memcmp( el, GML_NAMESPACE, nsLen ) == 0 )
    {
      mGMLNameSpaceURI = GML_NAMESPACE;
      mGMLNameSpaceURIPtr = GML_NAMESPACE;
    }
    else if ( nsLen == ( int )strlen( GML32_NAMESPACE ) && memcmp( el, GML32_NAMESPACE, nsLen ) == 0 )
    {
      mGMLNameSpaceURI = GML32_NAMESPACE;
      mGMLNameSpaceURIPtr = GML32_NAMESPACE;
    }
  }

  const bool isGMLNS = ( nsLen == mGMLNameSpaceURI.size() && mGMLNameSpaceURIPtr && memcmp( el, mGMLNameSpaceURIPtr, nsLen ) == 0 );
  bool isGeom = false;

  if ( theParseMode == geometry || theParseMode == coordinate || theParseMode == posList ||
       theParseMode == multiPoint || theParseMode == multiLine || theParseMode == multiPolygon )
  {
    mGeometryString.append( "<", 1 );
    mGeometryString.append( pszLocalName, localNameLen );
    mGeometryString.append( " ", 1 );
    for ( const XML_Char** attrIter = attr; attrIter && *attrIter; attrIter += 2 )
    {
      mGeometryString.append( attrIter[0] );
      mGeometryString.append( "=\"", 2 );
      mGeometryString.append( attrIter[1] );
      mGeometryString.append( "\" ", 2 );

    }
    mGeometryString.append( ">", 1 );
  }

  if ( isGMLNS && LOCALNAME_EQUALS( "coordinates" ) )
  {
    mParseModeStack.push( coordinate );
    mCoorMode = QgsGmlStreamingParser::coordinate;
    mStringCash.clear();
    mCoordinateSeparator = readAttribute( "cs", attr );
    if ( mCoordinateSeparator.isEmpty() )
    {
      mCoordinateSeparator = ',';
    }
    mTupleSeparator = readAttribute( "ts", attr );
    if ( mTupleSeparator.isEmpty() )
    {
      mTupleSeparator = ' ';
    }
  }
  else if ( isGMLNS &&
            ( LOCALNAME_EQUALS( "pos" ) || LOCALNAME_EQUALS( "posList" ) ) )
  {
    mParseModeStack.push( QgsGmlStreamingParser::posList );
    mCoorMode = QgsGmlStreamingParser::posList;
    mStringCash.clear();
    if ( mDimension == 0 )
    {
      QString srsDimension = readAttribute( "srsDimension", attr );
      bool ok;
      int dimension = srsDimension.toInt( &ok );
      if ( ok )
      {
        mDimension = dimension;
      }
    }
  }
  else if ( localNameLen == mGeometryAttribute.size() &&
            memcmp( pszLocalName, mGeometryAttributePtr, localNameLen ) == 0 )
  {
    mParseModeStack.push( QgsGmlStreamingParser::geometry );
    mFoundUnhandledGeometryElement = false;
    mGeometryString.clear();
  }
  //else if ( mParseModeStack.size() == 0 && elementName == mGMLNameSpaceURI + NS_SEPARATOR + "boundedBy" )
  else if ( isGMLNS && LOCALNAME_EQUALS( "boundedBy" ) )
  {
    mParseModeStack.push( QgsGmlStreamingParser::boundingBox );
    mCurrentExtent = QgsRectangle();
    mBoundedByNullFound = false;
  }
  else if ( theParseMode == boundingBox &&
            isGMLNS && LOCALNAME_EQUALS( "null" ) )
  {
    mParseModeStack.push( QgsGmlStreamingParser::null );
    mBoundedByNullFound = true;
  }
  else if ( theParseMode == boundingBox &&
            isGMLNS && LOCALNAME_EQUALS( "Envelope" ) )
  {
    isGeom = true;
    mParseModeStack.push( QgsGmlStreamingParser::envelope );
  }
  else if ( theParseMode == envelope &&
            isGMLNS && LOCALNAME_EQUALS( "lowerCorner" ) )
  {
    mParseModeStack.push( QgsGmlStreamingParser::lowerCorner );
    mStringCash.clear();
  }
  else if ( theParseMode == envelope &&
            isGMLNS && LOCALNAME_EQUALS( "upperCorner" ) )
  {
    mParseModeStack.push( QgsGmlStreamingParser::upperCorner );
    mStringCash.clear();
  }
  else if ( theParseMode == none && !mTypeNamePtr &&
            LOCALNAME_EQUALS( "Tuple" ) )
  {
    Q_ASSERT( !mCurrentFeature );
    mCurrentFeature = new QgsFeature( mFeatureCount );
    mCurrentFeature->setFields( mFields ); // allow name-based attribute lookups
    QgsAttributes attributes( mThematicAttributes.size() ); //add empty attributes
    mCurrentFeature->setAttributes( attributes );
    mParseModeStack.push( QgsGmlStreamingParser::tuple );
    mCurrentFeatureId.clear();
  }
  else if ( theParseMode == tuple )
  {
    QString currentTypename( QString::fromUtf8( pszLocalName, localNameLen ) );
    QMap< QString, LayerProperties >::const_iterator iter = mMapTypeNameToProperties.constFind( currentTypename );
    if ( iter != mMapTypeNameToProperties.end() )
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
      mParseModeStack.push( QgsGmlStreamingParser::featureTuple );
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
  else if ( theParseMode == none &&
            localNameLen == mTypeName.size() && memcmp( pszLocalName, mTypeNamePtr, mTypeName.size() ) == 0 )
  {
    Q_ASSERT( !mCurrentFeature );
    mCurrentFeature = new QgsFeature( mFeatureCount );
    mCurrentFeature->setFields( mFields ); // allow name-based attribute lookups
    QgsAttributes attributes( mThematicAttributes.size() ); //add empty attributes
    mCurrentFeature->setAttributes( attributes );
    mParseModeStack.push( QgsGmlStreamingParser::feature );
    mCurrentFeatureId = readAttribute( "fid", attr );
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

  else if ( theParseMode == boundingBox && isGMLNS && LOCALNAME_EQUALS( "Box" ) )
  {
    isGeom = true;
  }
  else if ( isGMLNS && LOCALNAME_EQUALS( "Point" ) )
  {
    isGeom = true;
  }
  else if ( isGMLNS && LOCALNAME_EQUALS( "LineString" ) )
  {
    isGeom = true;
  }
  else if ( isGMLNS &&
            localNameLen == strlen( "Polygon" ) && memcmp( pszLocalName, "Polygon", localNameLen ) == 0 )
  {
    isGeom = true;
    mCurrentWKBFragments.push_back( QList<QgsWkbPtr>() );
  }
  else if ( isGMLNS && LOCALNAME_EQUALS( "MultiPoint" ) )
  {
    isGeom = true;
    mParseModeStack.push( QgsGmlStreamingParser::multiPoint );
    //we need one nested list for intermediate WKB
    mCurrentWKBFragments.push_back( QList<QgsWkbPtr>() );
  }
  else if ( isGMLNS && ( LOCALNAME_EQUALS( "MultiLineString" ) || LOCALNAME_EQUALS( "MultiCurve" ) ) )
  {
    isGeom = true;
    mParseModeStack.push( QgsGmlStreamingParser::multiLine );
    //we need one nested list for intermediate WKB
    mCurrentWKBFragments.push_back( QList<QgsWkbPtr>() );
  }
  else if ( isGMLNS && ( LOCALNAME_EQUALS( "MultiPolygon" ) || LOCALNAME_EQUALS( "MultiSurface" ) ) )
  {
    isGeom = true;
    mParseModeStack.push( QgsGmlStreamingParser::multiPolygon );
  }
  else if ( theParseMode == featureTuple )
  {
    QString localName( QString::fromUtf8( pszLocalName, localNameLen ) );
    if ( mThematicAttributes.contains( mCurrentTypename + '|' + localName ) )
    {
      mParseModeStack.push( QgsGmlStreamingParser::attributeTuple );
      mAttributeName = mCurrentTypename + '|' + localName;
      mStringCash.clear();
    }
  }
  else if ( theParseMode == feature )
  {
    QString localName( QString::fromUtf8( pszLocalName, localNameLen ) );
    if ( mThematicAttributes.contains( localName ) )
    {
      mParseModeStack.push( QgsGmlStreamingParser::attribute );
      mAttributeName = localName;
      mStringCash.clear();
    }
    else
    {
      // QGIS server (2.2) is using:
      // <Attribute value="My description" name="desc"/>
      if ( localName.compare( "attribute", Qt::CaseInsensitive ) == 0 )
      {
        QString name = readAttribute( "name", attr );
        if ( mThematicAttributes.contains( name ) )
        {
          QString value = readAttribute( "value", attr );
          setAttribute( name, value );
        }
      }
    }
  }
  else if ( mParseDepth == 0 && LOCALNAME_EQUALS( "FeatureCollection" ) )
  {
    QString numberReturned = readAttribute( "numberReturned", attr ); // WFS 2.0
    if ( numberReturned.isEmpty() )
      numberReturned = readAttribute( "numberOfFeatures", attr ); // WFS 1.1
    bool conversionOk;
    mNumberReturned = numberReturned.toInt( &conversionOk );
    if ( !conversionOk )
      mNumberReturned = -1;

    QString numberMatched = readAttribute( "numberMatched", attr ); // WFS 2.0
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
    //QgsDebugMsg( "Found unhandled geometry element " + QString::fromUtf8( pszLocalName, localNameLen ) );
    mFoundUnhandledGeometryElement = true;
  }

  if ( !mGeometryString.empty() )
    isGeom = true;

  if ( mDimension == 0 && isGeom )
  {
    // srsDimension can also be set on the top geometry element
    // e.g. https://data.linz.govt.nz/services;key=XXXXXXXX/wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=data.linz.govt.nz:layer-524
    QString srsDimension = readAttribute( "srsDimension", attr );
    bool ok;
    int dimension = srsDimension.toInt( &ok );
    if ( ok )
    {
      mDimension = dimension;
    }
  }

  if ( mEpsg == 0 && isGeom )
  {
    if ( readEpsgFromAttribute( mEpsg, attr ) != 0 )
    {
      QgsDebugMsg( "error, could not get epsg id" );
    }
    else
    {
      QgsDebugMsg( QString( "mEpsg = %1" ).arg( mEpsg ) );
    }
  }

  mParseDepth ++;
}

void QgsGmlStreamingParser::endElement( const XML_Char* el )
{
  mParseDepth --;

  const int elLen = ( int )strlen( el );
  const char* pszSep = strchr( el, NS_SEPARATOR );
  const char* pszLocalName = ( pszSep ) ? pszSep + 1 : el;
  const int nsLen = ( pszSep ) ? ( int )( pszSep - el ) : 0;
  const int localNameLen = ( pszSep ) ? ( int )( elLen - nsLen ) - 1 : elLen;
  ParseMode theParseMode( mParseModeStack.isEmpty() ? none : mParseModeStack.top() );

  const bool isGMLNS = ( nsLen == mGMLNameSpaceURI.size() && mGMLNameSpaceURIPtr && memcmp( el, mGMLNameSpaceURIPtr, nsLen ) == 0 );

  if ( theParseMode == coordinate && isGMLNS && LOCALNAME_EQUALS( "coordinates" ) )
  {
    mParseModeStack.pop();
  }
  else if ( theParseMode == posList && isGMLNS &&
            ( LOCALNAME_EQUALS( "pos" ) || LOCALNAME_EQUALS( "posList" ) ) )
  {
    mParseModeStack.pop();
  }
  else if ( theParseMode == attributeTuple &&
            mCurrentTypename + '|' + QString::fromUtf8( pszLocalName, localNameLen ) == mAttributeName ) //add a thematic attribute to the feature
  {
    mParseModeStack.pop();

    setAttribute( mAttributeName, mStringCash );
  }
  else if ( theParseMode == attribute && QString::fromUtf8( pszLocalName, localNameLen ) == mAttributeName ) //add a thematic attribute to the feature
  {
    mParseModeStack.pop();

    setAttribute( mAttributeName, mStringCash );
  }
  else if ( theParseMode == geometry && localNameLen == mGeometryAttribute.size() &&
            memcmp( pszLocalName, mGeometryAttributePtr, localNameLen ) == 0 )
  {
    mParseModeStack.pop();
    if ( mFoundUnhandledGeometryElement )
    {
      OGRGeometryH hGeom = OGR_G_CreateFromGML( mGeometryString.c_str() );
      if ( hGeom )
      {
        const int wkbSize = OGR_G_WkbSize( hGeom );
        unsigned char* pabyBuffer = new unsigned char[ wkbSize ];
#if GDAL_VERSION_MAJOR >= 2
        OGR_G_ExportToIsoWkb( hGeom, wkbNDR, pabyBuffer );
#else
        OGR_G_ExportToWkb( hGeom, wkbNDR, pabyBuffer );
#endif
        QgsGeometry *g = new QgsGeometry();
        g->fromWkb( pabyBuffer, wkbSize );
        if ( mInvertAxisOrientation )
        {
          g->transform( QTransform( 0, 1, 1, 0, 0, 0 ) );
        }
        mCurrentFeature->setGeometry( g );
        OGR_G_DestroyGeometry( hGeom );
      }
    }
    mGeometryString.clear();
  }
  else if ( theParseMode == boundingBox && isGMLNS && LOCALNAME_EQUALS( "boundedBy" ) )
  {
    //create bounding box from mStringCash
    if ( mCurrentExtent.isNull() &&
         !mBoundedByNullFound &&
         !createBBoxFromCoordinateString( mCurrentExtent, mStringCash ) )
    {
      QgsDebugMsg( "creation of bounding box failed" );
    }
    if ( !mCurrentExtent.isNull() && mLayerExtent.isNull() &&
         mCurrentFeature == nullptr && mFeatureCount == 0 )
    {
      mLayerExtent = mCurrentExtent;
    }

    mParseModeStack.pop();
  }
  else if ( theParseMode == null && isGMLNS && LOCALNAME_EQUALS( "null" ) )
  {
    mParseModeStack.pop();
  }
  else if ( theParseMode == envelope && isGMLNS && LOCALNAME_EQUALS( "Envelope" ) )
  {
    mParseModeStack.pop();
  }
  else if ( theParseMode == lowerCorner && isGMLNS && LOCALNAME_EQUALS( "lowerCorner" ) )
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
  else if ( theParseMode == upperCorner && isGMLNS && LOCALNAME_EQUALS( "upperCorner" ) )
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
  else if ( theParseMode == featureTuple && mParseDepth == mFeatureTupleDepth )
  {
    mParseModeStack.pop();
    mFeatureTupleDepth = 0;
  }
  else if (( theParseMode == tuple && !mTypeNamePtr &&
             LOCALNAME_EQUALS( "Tuple" ) ) ||
           ( theParseMode == feature && localNameLen == mTypeName.size() &&
             memcmp( pszLocalName, mTypeNamePtr, mTypeName.size() ) == 0 ) )
  {
    Q_ASSERT( mCurrentFeature );
    if ( !mCurrentFeature->geometry() )
    {
      if ( mCurrentWKB.size() > 0 )
      {
        QgsGeometry *g = new QgsGeometry();
        g->fromWkb( mCurrentWKB, mCurrentWKB.size() );
        mCurrentFeature->setGeometry( g );
        mCurrentWKB = QgsWkbPtr( nullptr, 0 );
      }
      else if ( !mCurrentExtent.isEmpty() )
      {
        mCurrentFeature->setGeometry( QgsGeometry::fromRect( mCurrentExtent ) );
      }
    }
    mCurrentFeature->setValid( true );

    mFeatureList.push_back( QgsGmlFeaturePtrGmlIdPair( mCurrentFeature, mCurrentFeatureId ) );

    mCurrentFeature = nullptr;
    ++mFeatureCount;
    mParseModeStack.pop();
  }
  else if ( isGMLNS && LOCALNAME_EQUALS( "Point" ) )
  {
    QList<QgsPoint> pointList;
    if ( pointsFromString( pointList, mStringCash ) != 0 )
    {
      //error
    }

    if ( pointList.isEmpty() )
      return;  // error

    if ( theParseMode == QgsGmlStreamingParser::geometry )
    {
      //directly add WKB point to the feature
      if ( getPointWKB( mCurrentWKB, *( pointList.constBegin() ) ) != 0 )
      {
        //error
      }

      if ( mWkbType != QGis::WKBMultiPoint ) //keep multitype in case of geometry type mix
      {
        mWkbType = QGis::WKBPoint;
      }
    }
    else //multipoint, add WKB as fragment
    {
      QgsWkbPtr wkbPtr( nullptr, 0 );
      if ( getPointWKB( wkbPtr, *( pointList.constBegin() ) ) != 0 )
      {
        //error
      }
      if ( !mCurrentWKBFragments.isEmpty() )
      {
        mCurrentWKBFragments.last().push_back( wkbPtr );
      }
      else
      {
        QgsDebugMsg( "No wkb fragments" );
        delete [] wkbPtr;
      }
    }
  }
  else if ( isGMLNS && ( LOCALNAME_EQUALS( "LineString" ) || LOCALNAME_EQUALS( "LineStringSegment" ) ) )
  {
    //add WKB point to the feature

    QList<QgsPoint> pointList;
    if ( pointsFromString( pointList, mStringCash ) != 0 )
    {
      //error
    }
    if ( theParseMode == QgsGmlStreamingParser::geometry )
    {
      if ( getLineWKB( mCurrentWKB, pointList ) != 0 )
      {
        //error
      }

      if ( mWkbType != QGis::WKBMultiLineString )//keep multitype in case of geometry type mix
      {
        mWkbType = QGis::WKBLineString;
      }
    }
    else //multiline, add WKB as fragment
    {
      QgsWkbPtr wkbPtr( nullptr, 0 );
      if ( getLineWKB( wkbPtr, pointList ) != 0 )
      {
        //error
      }
      if ( !mCurrentWKBFragments.isEmpty() )
      {
        mCurrentWKBFragments.last().push_back( wkbPtr );
      }
      else
      {
        QgsDebugMsg( "no wkb fragments" );
        delete [] wkbPtr;
      }
    }
  }
  else if (( theParseMode == geometry || theParseMode == multiPolygon ) &&
           isGMLNS && LOCALNAME_EQUALS( "LinearRing" ) )
  {
    QList<QgsPoint> pointList;
    if ( pointsFromString( pointList, mStringCash ) != 0 )
    {
      //error
    }

    QgsWkbPtr wkbPtr( nullptr, 0 );
    if ( getRingWKB( wkbPtr, pointList ) != 0 )
    {
      //error
    }

    if ( !mCurrentWKBFragments.isEmpty() )
    {
      mCurrentWKBFragments.last().push_back( wkbPtr );
    }
    else
    {
      delete[] wkbPtr;
      QgsDebugMsg( "no wkb fragments" );
    }
  }
  else if (( theParseMode == geometry || theParseMode == multiPolygon ) && isGMLNS &&
           LOCALNAME_EQUALS( "Polygon" ) )
  {
    if ( mWkbType != QGis::WKBMultiPolygon )//keep multitype in case of geometry type mix
    {
      mWkbType = QGis::WKBPolygon;
    }

    if ( theParseMode == geometry )
    {
      createPolygonFromFragments();
    }
  }
  else if ( theParseMode == multiPoint &&  isGMLNS &&
            LOCALNAME_EQUALS( "MultiPoint" ) )
  {
    mWkbType = QGis::WKBMultiPoint;
    mParseModeStack.pop();
    createMultiPointFromFragments();
  }
  else if ( theParseMode == multiLine && isGMLNS &&
            ( LOCALNAME_EQUALS( "MultiLineString" )  || LOCALNAME_EQUALS( "MultiCurve" ) ) )
  {
    mWkbType = QGis::WKBMultiLineString;
    mParseModeStack.pop();
    createMultiLineFromFragments();
  }
  else if ( theParseMode == multiPolygon && isGMLNS &&
            ( LOCALNAME_EQUALS( "MultiPolygon" )  || LOCALNAME_EQUALS( "MultiSurface" ) ) )
  {
    mWkbType = QGis::WKBMultiPolygon;
    mParseModeStack.pop();
    createMultiPolygonFromFragments();
  }
  else if ( mParseDepth == 0 && LOCALNAME_EQUALS( "ExceptionReport" ) )
  {
    mParseModeStack.pop();
  }
  else if ( theParseMode == ExceptionText && LOCALNAME_EQUALS( "ExceptionText" ) )
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

void QgsGmlStreamingParser::characters( const XML_Char* chars, int len )
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

  QgsGmlStreamingParser::ParseMode theParseMode = mParseModeStack.top();
  if ( theParseMode == QgsGmlStreamingParser::attribute ||
       theParseMode == QgsGmlStreamingParser::attributeTuple ||
       theParseMode == QgsGmlStreamingParser::coordinate ||
       theParseMode == QgsGmlStreamingParser::posList ||
       theParseMode == QgsGmlStreamingParser::lowerCorner ||
       theParseMode == QgsGmlStreamingParser::upperCorner ||
       theParseMode == QgsGmlStreamingParser::ExceptionText )
  {
    mStringCash.append( QString::fromUtf8( chars, len ) );
  }
}

void QgsGmlStreamingParser::setAttribute( const QString& name, const QString& value )
{
  //find index with attribute name
  QMap<QString, QPair<int, QgsField> >::const_iterator att_it = mThematicAttributes.constFind( name );
  if ( att_it != mThematicAttributes.constEnd() )
  {
    QVariant var;
    switch ( att_it.value().second.type() )
    {
      case QVariant::Double:
        var = QVariant( value.toDouble() );
        break;
      case QVariant::Int:
        var = QVariant( value.toInt() );
        break;
      case QVariant::LongLong:
        var = QVariant( value.toLongLong() );
        break;
      case QVariant::DateTime:
        var = QVariant( QDateTime::fromString( value, Qt::ISODate ) );
        break;
      default: //string type is default
        var = QVariant( value );
        break;
    }
    Q_ASSERT( mCurrentFeature );
    mCurrentFeature->setAttribute( att_it.value().first, var );
  }
}

int QgsGmlStreamingParser::readEpsgFromAttribute( int& epsgNr, const XML_Char** attr )
{
  int i = 0;
  while ( attr[i] )
  {
    if ( strcmp( attr[i], "srsName" ) == 0 )
    {
      QString epsgString( attr[i+1] );
      QString epsgNrString;
      bool bIsUrn = false;
      if ( epsgString.startsWith( "http" ) ) //e.g. geoserver: "http://www.opengis.net/gml/srs/epsg.xml#4326"
      {
        epsgNrString = epsgString.section( '#', 1, 1 );
      }
      // WFS >= 1.1
      else if ( epsgString.startsWith( "urn:ogc:def:crs:EPSG:" ) ||
                epsgString.startsWith( "urn:x-ogc:def:crs:EPSG:" ) )
      {
        bIsUrn = true;
        epsgNrString = epsgString.split( ':' ).last();
      }
      else //e.g. umn mapserver: "EPSG:4326">
      {
        epsgNrString = epsgString.section( ':', 1, 1 );
      }
      bool conversionOk;
      int eNr = epsgNrString.toInt( &conversionOk );
      if ( !conversionOk )
      {
        return 1;
      }
      epsgNr = eNr;
      mSrsName = epsgString;

      QgsCoordinateReferenceSystem crs = QgsCRSCache::instance()->crsByOgcWmsCrs( QString( "EPSG:%1" ).arg( epsgNr ) );
      if ( crs.isValid() )
      {
        if ((( mAxisOrientationLogic == Honour_EPSG_if_urn && bIsUrn ) ||
             mAxisOrientationLogic == Honour_EPSG ) && crs.axisInverted() )
        {
          mInvertAxisOrientation = !mInvertAxisOrientationRequest;
        }
      }

      return 0;
    }
    ++i;
  }
  return 2;
}

QString QgsGmlStreamingParser::readAttribute( const QString& attributeName, const XML_Char** attr ) const
{
  int i = 0;
  while ( attr[i] )
  {
    if ( attributeName.compare( attr[i] ) == 0 )
    {
      return QString::fromUtf8( attr[i+1] );
    }
    i += 2;
  }
  return QString();
}

bool QgsGmlStreamingParser::createBBoxFromCoordinateString( QgsRectangle &r, const QString& coordString ) const
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

int QgsGmlStreamingParser::pointsFromCoordinateString( QList<QgsPoint>& points, const QString& coordString ) const
{
  //tuples are separated by space, x/y by ','
  QStringList tuples = coordString.split( mTupleSeparator, QString::SkipEmptyParts );
  QStringList tuples_coordinates;
  double x, y;
  bool conversionSuccess;

  QStringList::const_iterator tupleIterator;
  for ( tupleIterator = tuples.constBegin(); tupleIterator != tuples.constEnd(); ++tupleIterator )
  {
    tuples_coordinates = tupleIterator->split( mCoordinateSeparator, QString::SkipEmptyParts );
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
    points.push_back(( mInvertAxisOrientation ) ? QgsPoint( y, x ) : QgsPoint( x, y ) );
  }
  return 0;
}

int QgsGmlStreamingParser::pointsFromPosListString( QList<QgsPoint>& points, const QString& coordString, int dimension ) const
{
  // coordinates separated by spaces
  QStringList coordinates = coordString.split( ' ', QString::SkipEmptyParts );

  if ( coordinates.size() % dimension != 0 )
  {
    QgsDebugMsg( "Wrong number of coordinates" );
  }

  int ncoor = coordinates.size() / dimension;
  for ( int i = 0; i < ncoor; i++ )
  {
    bool conversionSuccess;
    double x = coordinates.value( i * dimension ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      continue;
    }
    double y = coordinates.value( i * dimension + 1 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      continue;
    }
    points.append(( mInvertAxisOrientation ) ? QgsPoint( y, x ) : QgsPoint( x, y ) );
  }
  return 0;
}

int QgsGmlStreamingParser::pointsFromString( QList<QgsPoint>& points, const QString& coordString ) const
{
  if ( mCoorMode == QgsGmlStreamingParser::coordinate )
  {
    return pointsFromCoordinateString( points, coordString );
  }
  else if ( mCoorMode == QgsGmlStreamingParser::posList )
  {
    return pointsFromPosListString( points, coordString, mDimension ? mDimension : 2 );
  }
  return 1;
}

int QgsGmlStreamingParser::getPointWKB( QgsWkbPtr &wkbPtr, const QgsPoint& point ) const
{
  int wkbSize = 1 + sizeof( int ) + 2 * sizeof( double );
  wkbPtr = QgsWkbPtr( new unsigned char[wkbSize], wkbSize );

  QgsWkbPtr fillPtr( wkbPtr );
  fillPtr << mEndian << QGis::WKBPoint << point.x() << point.y();

  return 0;
}

int QgsGmlStreamingParser::getLineWKB( QgsWkbPtr &wkbPtr, const QList<QgsPoint>& lineCoordinates ) const
{
  int wkbSize = 1 + 2 * sizeof( int ) + lineCoordinates.size() * 2 * sizeof( double );
  wkbPtr = QgsWkbPtr( new unsigned char[wkbSize], wkbSize );

  QgsWkbPtr fillPtr( wkbPtr );

  fillPtr << mEndian << QGis::WKBLineString << lineCoordinates.size();

  QList<QgsPoint>::const_iterator iter;
  for ( iter = lineCoordinates.constBegin(); iter != lineCoordinates.constEnd(); ++iter )
  {
    fillPtr << iter->x() << iter->y();
  }

  return 0;
}

int QgsGmlStreamingParser::getRingWKB( QgsWkbPtr &wkbPtr, const QList<QgsPoint>& ringCoordinates ) const
{
  int wkbSize = sizeof( int ) + ringCoordinates.size() * 2 * sizeof( double );
  wkbPtr = QgsWkbPtr( new unsigned char[wkbSize], wkbSize );

  QgsWkbPtr fillPtr( wkbPtr );

  fillPtr << ringCoordinates.size();

  QList<QgsPoint>::const_iterator iter;
  for ( iter = ringCoordinates.constBegin(); iter != ringCoordinates.constEnd(); ++iter )
  {
    fillPtr << iter->x() << iter->y();
  }

  return 0;
}

int QgsGmlStreamingParser::createMultiLineFromFragments()
{
  int size = 1 + 2 * sizeof( int ) + totalWKBFragmentSize();
  mCurrentWKB = QgsWkbPtr( new unsigned char[size], size );

  QgsWkbPtr wkbPtr( mCurrentWKB );

  wkbPtr << mEndian << QGis::WKBMultiLineString << mCurrentWKBFragments.constBegin()->size();

  //copy (and delete) all the wkb fragments
  QList<QgsWkbPtr>::const_iterator wkbIt = mCurrentWKBFragments.constBegin()->constBegin();
  for ( ; wkbIt != mCurrentWKBFragments.constBegin()->constEnd(); ++wkbIt )
  {
    memcpy( wkbPtr, *wkbIt, wkbIt->size() );
    wkbPtr += wkbIt->size();
    delete[] *wkbIt;
  }

  mCurrentWKBFragments.clear();
  mWkbType = QGis::WKBMultiLineString;
  return 0;
}

int QgsGmlStreamingParser::createMultiPointFromFragments()
{
  int size = 1 + 2 * sizeof( int ) + totalWKBFragmentSize();
  mCurrentWKB = QgsWkbPtr( new unsigned char[size], size );

  QgsWkbPtr wkbPtr( mCurrentWKB );
  wkbPtr << mEndian << QGis::WKBMultiPoint << mCurrentWKBFragments.constBegin()->size();

  QList<QgsWkbPtr>::const_iterator wkbIt = mCurrentWKBFragments.constBegin()->constBegin();
  for ( ; wkbIt != mCurrentWKBFragments.constBegin()->constEnd(); ++wkbIt )
  {
    memcpy( wkbPtr, *wkbIt, wkbIt->size() );
    wkbPtr += wkbIt->size();
    delete[] *wkbIt;
  }

  mCurrentWKBFragments.clear();
  mWkbType = QGis::WKBMultiPoint;
  return 0;
}


int QgsGmlStreamingParser::createPolygonFromFragments()
{
  int size = 1 + 2 * sizeof( int ) + totalWKBFragmentSize();
  mCurrentWKB = QgsWkbPtr( new unsigned char[size], size );

  QgsWkbPtr wkbPtr( mCurrentWKB );
  wkbPtr << mEndian << QGis::WKBPolygon << mCurrentWKBFragments.constBegin()->size();

  QList<QgsWkbPtr>::const_iterator wkbIt = mCurrentWKBFragments.constBegin()->constBegin();
  for ( ; wkbIt != mCurrentWKBFragments.constBegin()->constEnd(); ++wkbIt )
  {
    memcpy( wkbPtr, *wkbIt, wkbIt->size() );
    wkbPtr += wkbIt->size();
    delete[] *wkbIt;
  }

  mCurrentWKBFragments.clear();
  mWkbType = QGis::WKBPolygon;
  return 0;
}

int QgsGmlStreamingParser::createMultiPolygonFromFragments()
{
  int size = 0;
  size += 1 + 2 * sizeof( int );
  size += totalWKBFragmentSize();
  size += mCurrentWKBFragments.size() * ( 1 + 2 * sizeof( int ) ); //fragments are just the rings

  mCurrentWKB = QgsWkbPtr( new unsigned char[size], size );

  QgsWkbPtr wkbPtr( mCurrentWKB );
  wkbPtr << ( char ) mEndian << QGis::WKBMultiPolygon << mCurrentWKBFragments.size();

  //have outer and inner iterators
  QList< QList<QgsWkbPtr> >::const_iterator outerWkbIt = mCurrentWKBFragments.constBegin();

  for ( ; outerWkbIt != mCurrentWKBFragments.constEnd(); ++outerWkbIt )
  {
    //new polygon
    wkbPtr << ( char ) mEndian << QGis::WKBPolygon << outerWkbIt->size();

    QList<QgsWkbPtr>::const_iterator innerWkbIt = outerWkbIt->constBegin();
    for ( ; innerWkbIt != outerWkbIt->constEnd(); ++innerWkbIt )
    {
      memcpy( wkbPtr, *innerWkbIt, innerWkbIt->size() );
      wkbPtr += innerWkbIt->size();
      delete[] *innerWkbIt;
    }
  }

  mCurrentWKBFragments.clear();
  mWkbType = QGis::WKBMultiPolygon;
  return 0;
}

int QgsGmlStreamingParser::totalWKBFragmentSize() const
{
  int result = 0;
  Q_FOREACH ( const QList<QgsWkbPtr> &list, mCurrentWKBFragments )
  {
    Q_FOREACH ( const QgsWkbPtr &i, list )
    {
      result += i.size();
    }
  }
  return result;
}
