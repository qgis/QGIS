/***************************************************************************
                              qgsxmlschemaanalyzer.cpp
                              ------------------------
  begin                : December 2025
  copyright            : (C) 2006 by Marco Hugentobler
                         (C) 2016 by Even Rouault
  email                : even.rouault at spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsxmlschemaanalyzer.h"

#include <cpl_string.h>
#include <gdal.h>
#include <ogr_api.h>

#include "qgsbackgroundcachedshareddata.h"
#include "qgsbasenetworkrequest.h"
#include "qgscplhttpfetchoverrider.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"
#include "qgsogcutils.h"
#include "qgsogrutils.h"
#include "qgssetrequestinitiator_p.h"
#include "qgssettings.h"
#include "qgswfsconstants.h"
#include "qgswfsdescribefeaturetype.h"

#include <QDialog>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>
#include <QUrlQuery>

bool QgsXmlSchemaAnalyzer::readAttributesFromSchema(
  const QString &translatedProviderName,
  QgsBackgroundCachedSharedData *sharedData,
  const Qgis::VectorProviderCapabilities &capabilities,
  QDomDocument &schemaDoc,
  const QByteArray &response,
  bool singleLayerContext,
  const QString &prefixedTypename,
  QString &geometryAttribute,
  QgsFields &fields,
  Qgis::WkbType &geomType,
  bool &geometryMaybeMissing,
  QString &errorMsg,
  bool &metadataRetrievalCanceled
)
{
  geometryMaybeMissing = false;
  metadataRetrievalCanceled = false;
  bool mayTryWithGMLAS = false;
  bool ret = readAttributesFromSchemaWithoutGMLAS(
    translatedProviderName, sharedData, schemaDoc, prefixedTypename,
    geometryAttribute, fields, geomType, errorMsg, mayTryWithGMLAS
  );

  // Only consider GMLAS / ComplexFeatures mode if FeatureMode=DEFAULT and there
  // is no edition capabilities, or if explicitly requested.
  // Cf https://github.com/qgis/QGIS/pull/61493
  if ( ( ( sharedData->mURI.featureMode() == QgsWFSDataSourceURI::FeatureMode::Default && ( capabilities & Qgis::VectorProviderCapability::AddFeatures ) == 0 ) || sharedData->mURI.featureMode() == QgsWFSDataSourceURI::FeatureMode::ComplexFeatures ) && singleLayerContext && mayTryWithGMLAS && GDALGetDriverByName( "GMLAS" ) )
  {
    QString geometryAttributeGMLAS;
    QgsFields fieldsGMLAS;
    Qgis::WkbType geomTypeGMLAS;
    QString errorMsgGMLAS;
    if ( readAttributesFromSchemaWithGMLAS( translatedProviderName, sharedData, response, prefixedTypename, geometryAttributeGMLAS, fieldsGMLAS, geomTypeGMLAS, geometryMaybeMissing, errorMsgGMLAS, metadataRetrievalCanceled ) )
    {
      geometryAttribute = geometryAttributeGMLAS;
      fields = fieldsGMLAS;
      geomType = geomTypeGMLAS;
      errorMsg.clear();
      ret = true;
    }
    else
    {
      errorMsg = errorMsgGMLAS;
    }
  }
  return ret;
}

static QMetaType::Type getVariantTypeFromXML( const QString &xmlType )
{
  QMetaType::Type attributeType = QMetaType::Type::UnknownType;

  const QString type = QString( xmlType )
                         .replace( "xs:"_L1, QString() )
                         .replace( "xsd:"_L1, QString() );

  if ( type.compare( "string"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "token"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "NMTOKEN"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "NCName"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "QName"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "ID"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "IDREF"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "anyURI"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "anySimpleType"_L1, Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QMetaType::Type::QString;
  }
  else if ( type.compare( "boolean"_L1, Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QMetaType::Type::Bool;
  }
  else if ( type.compare( "double"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "float"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "decimal"_L1, Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QMetaType::Type::Double;
  }
  else if ( type.compare( "byte"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "unsignedByte"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "int"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "short"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "unsignedShort"_L1, Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QMetaType::Type::Int;
  }
  else if ( type.compare( "long"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "unsignedLong"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "integer"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "negativeInteger"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "nonNegativeInteger"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "positiveInteger"_L1, Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QMetaType::Type::LongLong;
  }
  else if ( type.compare( "date"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "gYear"_L1, Qt::CaseInsensitive ) == 0 || type.compare( "gYearMonth"_L1, Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QMetaType::Type::QDate;
  }
  else if ( type.compare( "time"_L1, Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QMetaType::Type::QTime;
  }
  else if ( type.compare( "dateTime"_L1, Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QMetaType::Type::QDateTime;
  }
  return attributeType;
}

struct QgsXmlSchemaAnalyzerGMLASErrorHandlerUserData
{
    QString errorMessage;
    QString translatedProviderName;
};

static void CPL_STDCALL QgsXmlSchemaAnalyzerGMLASErrorHandler(
  CPLErr eErr, CPLErrorNum /*eErrorNum*/, const char *pszErrorMsg
)
{
  // Silence harmless warnings like "GeographicalName_pronunciation_PronunciationOfName_pronunciationSoundLink_nilReason identifier truncated to geographicalname_pronunciation_pronunciationofname_pronunciatio"
  if ( !( eErr == CE_Warning && strstr( pszErrorMsg, " truncated to " ) ) )
  {
    if ( eErr == CE_Failure )
    {
      QgsXmlSchemaAnalyzerGMLASErrorHandlerUserData *pUserData = static_cast<QgsXmlSchemaAnalyzerGMLASErrorHandlerUserData *>( CPLGetErrorHandlerUserData() );
      if ( pUserData->errorMessage.isEmpty() )
        pUserData->errorMessage = QObject::tr( "Error while analyzing schema: %1" ).arg( pszErrorMsg );
      QgsMessageLog::logMessage( QObject::tr( "GMLAS error: %1" ).arg( pszErrorMsg ), pUserData->translatedProviderName );
    }
    else if ( eErr == CE_Debug )
    {
      QgsDebugMsgLevel( u"GMLAS debug msg: %1"_s.arg( pszErrorMsg ), 5 );
    }
    else
    {
      QgsDebugMsgLevel( u"GMLAS eErr=%1, msg=%2"_s.arg( eErr ).arg( pszErrorMsg ), 2 );
    }
  }
}

bool QgsXmlSchemaAnalyzer::readAttributesFromSchemaWithGMLAS(
  const QString &translatedProviderName,
  QgsBackgroundCachedSharedData *sharedData,
  const QByteArray &response,
  const QString &prefixedTypename,
  QString &geometryAttribute,
  QgsFields &fields,
  Qgis::WkbType &geomType,
  bool &geometryMaybeMissing,
  QString &errorMsg,
  bool &metadataRetrievalCanceled
)
{
  geomType = Qgis::WkbType::NoGeometry;
  geometryMaybeMissing = false;

  QUrl url( sharedData->mURI.requestUrl( u"DescribeFeatureType"_s ) );
  QUrlQuery query( url );
  query.addQueryItem( u"TYPENAME"_s, prefixedTypename );
  url.setQuery( query );

  // If a previous attempt with the same URL failed because of cancellation
  // in the past second, do not retry.
  // The main use case for that is when QgsWfsProviderMetadata::querySublayers()
  // is called when adding a layer, and several QgsXmlSchemaAnalyzer instances are
  // quickly created.
  static QMutex mutex;
  static QUrl lastCanceledURL;
  static QDateTime lastCanceledDateTime;
  {
    QMutexLocker lock( &mutex );
    if ( lastCanceledURL == url && lastCanceledDateTime + 1 > QDateTime::currentDateTime() )
    {
      metadataRetrievalCanceled = true;
      return false;
    }
  }

  // Create a unique /vsimem/ filename
  constexpr int TEMP_FILENAME_SIZE = 128;
  void *p = malloc( TEMP_FILENAME_SIZE );
  char *pszSchemaTempFilename = static_cast<char *>( p );
  snprintf( pszSchemaTempFilename, TEMP_FILENAME_SIZE, "/vsimem/schema_%p.xsd", p );

  // Serialize the main schema into a temporary /vsimem/ filename
  char *pszSchema = VSIStrdup( response.constData() );
  VSILFILE *fp = VSIFileFromMemBuffer( pszSchemaTempFilename, reinterpret_cast<GByte *>( pszSchema ), strlen( pszSchema ), /* bTakeOwnership=*/true );
  if ( fp )
    VSIFCloseL( fp );

  QgsFeedback feedback;
  GDALDatasetH hDS = nullptr;

  // Analyze the DescribeFeatureType response schema with the OGR GMLAS driver
  // in a thread, so it can get interrupted (with GDAL 3.9: https://github.com/OSGeo/gdal/pull/9019)

  const auto downloaderLambda = [pszSchemaTempFilename, &feedback, &hDS, &errorMsg, &translatedProviderName]() {
    QgsCPLHTTPFetchOverrider cplHTTPFetchOverrider( QString(), &feedback );
    QgsSetCPLHTTPFetchOverriderInitiatorClass( cplHTTPFetchOverrider, u"WFSProviderDownloadSchema"_s )

      char **papszOpenOptions
      = nullptr;
    papszOpenOptions = CSLSetNameValue( papszOpenOptions, "XSD", pszSchemaTempFilename );

    QgsSettings settings;
    QString cacheDirectory = settings.value( u"cache/directory"_s ).toString();
    if ( cacheDirectory.isEmpty() )
      cacheDirectory = QStandardPaths::writableLocation( QStandardPaths::CacheLocation );
    if ( !cacheDirectory.endsWith( QDir::separator() ) )
    {
      cacheDirectory.push_back( QDir::separator() );
    }
    // Must be kept in sync with QgsOptions::clearCache()
    cacheDirectory += "gmlas_xsd_cache"_L1;
    QgsDebugMsgLevel( u"cacheDirectory = %1"_s.arg( cacheDirectory ), 4 );
    char *pszEscaped = CPLEscapeString( cacheDirectory.toStdString().c_str(), -1, CPLES_XML );
    QString config = QStringLiteral( "<Configuration><SchemaCache><Directory>%1</Directory></SchemaCache>"
                                     "<IgnoredXPaths>"
                                     "    <WarnIfIgnoredXPathFoundInDocInstance>true</WarnIfIgnoredXPathFoundInDocInstance>"
                                     "    <Namespaces>"
                                     "        <Namespace prefix=\"gml\" uri=\"http://www.opengis.net/gml\"/>"
                                     "        <Namespace prefix=\"gml32\" uri=\"http://www.opengis.net/gml/3.2\"/>"
                                     "        <Namespace prefix=\"swe\" uri=\"http://www.opengis.net/swe/2.0\"/>"
                                     "    </Namespaces>"
                                     "    <XPath warnIfIgnoredXPathFoundInDocInstance=\"false\">gml:boundedBy</XPath>"
                                     "    <XPath warnIfIgnoredXPathFoundInDocInstance=\"false\">gml32:boundedBy</XPath>"
                                     "    <XPath>gml:priorityLocation</XPath>"
                                     "    <XPath>gml32:priorityLocation</XPath>"
                                     "    <XPath>gml32:descriptionReference/@owns</XPath>"
                                     "    <XPath>@xlink:show</XPath>"
                                     "    <XPath>@xlink:type</XPath>"
                                     "    <XPath>@xlink:role</XPath>"
                                     "    <XPath>@xlink:arcrole</XPath>"
                                     "    <XPath>@xlink:actuate</XPath>"
                                     "    <XPath>@gml:remoteSchema</XPath>"
                                     "    <XPath>@gml32:remoteSchema</XPath>"
                                     "    <XPath>swe:Quantity/swe:extension</XPath>"
                                     "    <XPath>swe:Quantity/@referenceFrame</XPath>"
                                     "    <XPath>swe:Quantity/@axisID</XPath>"
                                     "    <XPath>swe:Quantity/@updatable</XPath>"
                                     "    <XPath>swe:Quantity/@optional</XPath>"
                                     "    <XPath>swe:Quantity/@id</XPath>"
                                     "    <XPath>swe:Quantity/swe:identifier</XPath>"
                                     "    <!-- <XPath>swe:Quantity/@definition</XPath> -->"
                                     "    <XPath>swe:Quantity/swe:label</XPath>"
                                     "    <XPath>swe:Quantity/swe:nilValues</XPath>"
                                     "    <XPath>swe:Quantity/swe:constraint</XPath>"
                                     "    <XPath>swe:Quantity/swe:quality</XPath>"
                                     "</IgnoredXPaths>"
                                     "</Configuration>" )
                       .arg( pszEscaped );
    CPLFree( pszEscaped );
    papszOpenOptions = CSLSetNameValue( papszOpenOptions, "CONFIG_FILE", config.toStdString().c_str() );

    QgsXmlSchemaAnalyzerGMLASErrorHandlerUserData userData;
    userData.errorMessage = errorMsg;
    userData.translatedProviderName = translatedProviderName;
    CPLPushErrorHandlerEx( QgsXmlSchemaAnalyzerGMLASErrorHandler, &userData );
    hDS = GDALOpenEx( "GMLAS:", GDAL_OF_VECTOR, nullptr, papszOpenOptions, nullptr );
    errorMsg = userData.errorMessage;
    CPLPopErrorHandler();
    CSLDestroy( papszOpenOptions );
  };

  auto downloaderThread = std::make_unique<_DownloaderThread>( downloaderLambda );
  downloaderThread->start();

  QTimer timerForHits;

  QMessageBox *box = nullptr;
  QWidget *parentWidget = nullptr;
  if ( qApp->thread() == QThread::currentThread() )
  {
    parentWidget = QApplication::activeWindow();
    if ( !parentWidget )
    {
      const QWidgetList widgets = QgsApplication::topLevelWidgets();
      for ( QWidget *widget : widgets )
      {
        if ( widget->objectName() == "QgisApp"_L1 )
        {
          parentWidget = widget;
          break;
        }
      }
    }
  }
  if ( parentWidget )
  {
    // Display an information box if within 2 seconds, the schema has not
    // been analyzed.
    box = new QMessageBox(
      QMessageBox::Information, QObject::tr( "Information" ), QObject::tr( "Download of XML schemas to which the WFS refers in progress..." ),
      QMessageBox::Cancel,
      parentWidget
    );
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION( 3, 9, 0 )
    QObject::connect( box, &QDialog::rejected, &feedback, &QgsFeedback::cancel );
#else
    box->button( QMessageBox::Cancel )->setEnabled( false );
#endif

    QgsSettings s;
    const double settingDefaultValue = 2.0;
    const QString settingName = u"qgis/wfsDownloadSchemasPopupTimeout"_s;
    if ( !s.contains( settingName ) )
    {
      s.setValue( settingName, settingDefaultValue );
    }
    const double timeout = s.value( settingName, settingDefaultValue ).toDouble();
    if ( timeout > 0 )
    {
      timerForHits.setInterval( static_cast<int>( 1000 * timeout ) );
      timerForHits.setSingleShot( true );
      timerForHits.start();
      QObject::connect( &timerForHits, &QTimer::timeout, box, &QDialog::exec );
    }

    // Close dialog when download theread finishes.
    // Will actually trigger the QDialog::rejected signal...
    QObject::connect( downloaderThread.get(), &QThread::finished, box, &QDialog::accept );
  }

  // Run an event loop until download thread finishes
  QEventLoop loop;
  QObject::connect( downloaderThread.get(), &QThread::finished, &loop, &QEventLoop::quit );
  loop.exec( QEventLoop::ExcludeUserInputEvents );
  downloaderThread->wait();

  VSIUnlink( pszSchemaTempFilename );
  VSIFree( pszSchemaTempFilename );

  if ( !errorMsg.isEmpty() )
    return false;

  bool ret = hDS;
  if ( feedback.isCanceled() && !ret )
  {
    QMutexLocker lock( &mutex );
    metadataRetrievalCanceled = true;
    lastCanceledURL = url;
    lastCanceledDateTime = QDateTime::currentDateTime();
    errorMsg = QObject::tr( "Schema analysis interrupted by user." );
    return false;
  }
  if ( !ret )
  {
    if ( errorMsg.isEmpty() )
      errorMsg = QObject::tr( "Cannot analyze schema indicated in DescribeFeatureType response." );
    return false;
  }

  gdal::dataset_unique_ptr oDSCloser( hDS );

  // Retrieve namespace prefix and URIs
  OGRLayerH hOtherMetadataLayer = GDALDatasetGetLayerByName( hDS, "_ogr_other_metadata" );
  if ( !hOtherMetadataLayer )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find _ogr_other_metadata layer"_s, 4 );
    return false;
  }

  auto hOtherMetadataLayerDefn = OGR_L_GetLayerDefn( hOtherMetadataLayer );

  const int keyIdx = OGR_FD_GetFieldIndex( hOtherMetadataLayerDefn, "key" );
  if ( keyIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find key field in _ogr_other_metadata"_s, 4 );
    return false;
  }

  const int valueIdx = OGR_FD_GetFieldIndex( hOtherMetadataLayerDefn, "value" );
  if ( valueIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find value field in _ogr_other_metadata"_s, 4 );
    return false;
  }

  std::map<int, QPair<QString, QString>> mapPrefixIdxToPrefixAndUri;
  while ( true )
  {
    gdal::ogr_feature_unique_ptr hFeatureOtherMD(
      OGR_L_GetNextFeature( hOtherMetadataLayer )
    );
    if ( !hFeatureOtherMD )
      break;

    const QString key = QString::fromUtf8(
      OGR_F_GetFieldAsString( hFeatureOtherMD.get(), keyIdx )
    );
    const QString value = QString::fromUtf8(
      OGR_F_GetFieldAsString( hFeatureOtherMD.get(), valueIdx )
    );

    if ( key.startsWith( "namespace_prefix_"_L1 ) )
    {
      mapPrefixIdxToPrefixAndUri[key.mid( int( strlen( "namespace_prefix_" ) ) ).toInt()].first = value;
    }
    else if ( key.startsWith( "namespace_uri_"_L1 ) )
    {
      mapPrefixIdxToPrefixAndUri[key.mid( int( strlen( "namespace_uri_" ) ) ).toInt()].second = value;
    }
  }
  for ( const auto &kv : mapPrefixIdxToPrefixAndUri )
  {
    if ( !kv.second.first.isEmpty() && !kv.second.second.isEmpty() )
    {
      sharedData->mNamespacePrefixToURIMap[kv.second.first] = kv.second.second;
      QgsDebugMsgLevel( u"%1 -> %2"_s.arg( kv.second.first ).arg( kv.second.second ), 4 );
    }
  }

  // Find the layer of interest
  OGRLayerH hLayersMetadata = GDALDatasetGetLayerByName( hDS, "_ogr_layers_metadata" );
  if ( !hLayersMetadata )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find _ogr_layers_metadata layer"_s, 4 );
    return false;
  }
  OGR_L_SetAttributeFilter( hLayersMetadata, ( "layer_xpath = " + QgsSqliteUtils::quotedString( prefixedTypename ).toStdString() ).c_str() );
  gdal::ogr_feature_unique_ptr hFeatureLayersMD( OGR_L_GetNextFeature( hLayersMetadata ) );
  if ( !hFeatureLayersMD && !prefixedTypename.contains( QLatin1Char( ':' ) ) )
  {
    // In OAPIF use case, we don't have a typename prefixed with a namespace prefix
    OGR_L_SetAttributeFilter( hLayersMetadata, ( "layer_xpath LIKE " + QgsSqliteUtils::quotedString( u"%:"_s + prefixedTypename ).toStdString() ).c_str() );
    OGR_L_ResetReading( hLayersMetadata );
    hFeatureLayersMD.reset( OGR_L_GetNextFeature( hLayersMetadata ) );
  }
  if ( !hFeatureLayersMD )
  {
    QgsDebugMsgLevel(
      u"Cannot find feature with layer_xpath = %1 in _ogr_layers_metadata"_s.arg( prefixedTypename ), 4
    );
    return false;
  }
  const int fldIdx = OGR_F_GetFieldIndex( hFeatureLayersMD.get(), "layer_name" );
  if ( fldIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find layer_name field in _ogr_layers_metadata"_s, 4 );
    return false;
  }
  const QString layerName = QString::fromUtf8(
    OGR_F_GetFieldAsString( hFeatureLayersMD.get(), fldIdx )
  );

  OGRLayerH hLayer = GDALDatasetGetLayerByName(
    hDS, layerName.toStdString().c_str()
  );
  if ( !hLayer )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find %& layer"_s.arg( layerName ), 4 );
    return false;
  }

  // Get field information
  OGRLayerH hFieldsMetadata = GDALDatasetGetLayerByName( hDS, "_ogr_fields_metadata" );
  if ( !hFieldsMetadata )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find _ogr_fields_metadata layer"_s, 4 );
    return false;
  }
  OGR_L_SetAttributeFilter( hFieldsMetadata, ( "layer_name = " + QgsSqliteUtils::quotedString( layerName ).toStdString() ).c_str() );

  auto hFieldsMetadataDefn = OGR_L_GetLayerDefn( hFieldsMetadata );

  const int fieldNameIdx = OGR_FD_GetFieldIndex( hFieldsMetadataDefn, "field_name" );
  if ( fieldNameIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find field_name field in _ogr_fields_metadata"_s, 4 );
    return false;
  }

  const int fieldXPathIdx = OGR_FD_GetFieldIndex( hFieldsMetadataDefn, "field_xpath" );
  if ( fieldXPathIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find field_xpath field in _ogr_fields_metadata"_s, 4 );
    return false;
  }

  const int fieldIsListIdx = OGR_FD_GetFieldIndex( hFieldsMetadataDefn, "field_is_list" );
  if ( fieldIsListIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find field_is_list field in _ogr_fields_metadata"_s, 4 );
    return false;
  }

  const int fieldMinOccursIdx = OGR_FD_GetFieldIndex( hFieldsMetadataDefn, "field_min_occurs" );
  if ( fieldMinOccursIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find field_min_occurs field in _ogr_fields_metadata"_s, 4 );
    return false;
  }

  const int fieldTypeIdx = OGR_FD_GetFieldIndex( hFieldsMetadataDefn, "field_type" );
  if ( fieldTypeIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find field_type field in _ogr_fields_metadata"_s, 4 );
    return false;
  }

  const int fieldCategoryIdx = OGR_FD_GetFieldIndex( hFieldsMetadataDefn, "field_category" );
  if ( fieldCategoryIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( u"Cannot find field_category field in _ogr_fields_metadata"_s, 4 );
    return false;
  }

  sharedData->mFieldNameToXPathAndIsNestedContentMap.clear();
  while ( true )
  {
    gdal::ogr_feature_unique_ptr hFeatureFieldsMD( OGR_L_GetNextFeature( hFieldsMetadata ) );
    if ( !hFeatureFieldsMD )
      break;

    QString fieldName = QString::fromUtf8( OGR_F_GetFieldAsString( hFeatureFieldsMD.get(), fieldNameIdx ) );
    const char *fieldXPath = OGR_F_GetFieldAsString( hFeatureFieldsMD.get(), fieldXPathIdx );
    // The xpath includes the one of the feature itself. We can strip it off
    const char *slash = strchr( fieldXPath, '/' );
    if ( slash )
      fieldXPath = slash + 1;
    const bool fieldIsList = OGR_F_GetFieldAsInteger( hFeatureFieldsMD.get(), fieldIsListIdx ) == 1;
    const char *fieldType = OGR_F_GetFieldAsString( hFeatureFieldsMD.get(), fieldTypeIdx );
    const char *fieldCategory = OGR_F_GetFieldAsString( hFeatureFieldsMD.get(), fieldCategoryIdx );

    // For fields that should be linked to other tables and that we will
    // get as JSON, remove the "_pkid" suffix from the name created by GMLAS.
    if ( EQUAL( fieldCategory, "PATH_TO_CHILD_ELEMENT_WITH_LINK" ) && fieldName.endsWith( "_pkid"_L1 ) )
    {
      fieldName.resize( fieldName.size() - int( strlen( "_pkid" ) ) );
    }

    QgsDebugMsgLevel(
      u"field %1: xpath=%2 is_list=%3 type=%4 category=%5"_s.arg( fieldName ).arg( fieldXPath ).arg( fieldIsList ).arg( fieldType ).arg( fieldCategory ), 5
    );
    if ( EQUAL( fieldCategory, "REGULAR" ) && ( EQUAL( fieldType, "geometry" ) || fieldName.endsWith( "_abstractgeometricprimitive"_L1 ) ) )
    {
      if ( geometryAttribute.isEmpty() )
      {
        geomType = QgsOgrUtils::ogrGeometryTypeToQgsWkbType(
          OGR_L_GetGeomType( hLayer )
        );
        if ( geomType != Qgis::WkbType::Point )
          geomType = QgsWkbTypes::multiType( geomType );

        QString qFieldXPath = QString::fromUtf8( fieldXPath );
        if ( fieldName.endsWith( "_abstractgeometricprimitive"_L1 ) && strstr( fieldXPath, "/gml:Point" ) )
        {
          // Note: this particular case will not be needed in GDAL >= 3.8.4
          // The _abstractgeometricprimitive case is for a layer like
          //  "https://www.wfs.nrw.de/geobasis/wfs_nw_inspire-gewaesser-physisch_atkis-basis-dlm?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=hy-p:Embankment&NAMESPACES=xmlns(hy-p,http://inspire.ec.europa.eu/schemas/hy-p/4.0)&TYPENAME=hy-p:Embankment&NAMESPACE=xmlns(hy-p,http://inspire.ec.europa.eu/schemas/hy-p/4.0)"
          // which has a geometry element as:
          //   layer_name (String) = embankment
          //   field_index (Integer) = 30
          //   field_name (String) = geometry_abstractgeometricprimitive
          //   field_xpath (String) = hy-p:Embankment/hy-p:geometry/gml:Point,hy-p:Embankment/hy-p:geometry/gml:LineString,hy-p:Embankment/hy-p:geometry/gml:LinearRing,hy-p:Embankment/hy-p:geometry/gml:Ring,hy-p:Embankment/hy-p:geometry/gml:Curve,hy-p:Embankment/hy-p:geometry/gml:OrientableCurve,hy-p:Embankment/hy-p:geometry/gml:CompositeCurve,hy-p:Embankment/hy-p:geometry/gml:Polygon,hy-p:Embankment/hy-p:geometry/gml:Surface,hy-p:Embankment/hy-p:geometry/gml:PolyhedralSurface,hy-p:Embankment/hy-p:geometry/gml:TriangulatedSurface,hy-p:Embankment/hy-p:geometry/gml:Tin,hy-p:Embankment/hy-p:geometry/gml:OrientableSurface,hy-p:Embankment/hy-p:geometry/gml:Shell,hy-p:Embankment/hy-p:geometry/gml:CompositeSurface,hy-p:Embankment/hy-p:geometry/gml:Solid,hy-p:Embankment/hy-p:geometry/gml:CompositeSolid
          //   field_type (String) = anyType
          //   field_is_list (Integer(Boolean)) = 0
          //   field_min_occurs (Integer) = 0
          //   field_max_occurs (Integer) = 1
          //   field_category (String) = REGULAR

          const auto pos_gmlPoint = qFieldXPath.indexOf( "/gml:Point,"_L1 );
          qFieldXPath.resize( pos_gmlPoint );
          geomType = Qgis::WkbType::Unknown;
        }

        sharedData->mFieldNameToXPathAndIsNestedContentMap[fieldName] = QPair<QString, bool>( qFieldXPath, false );
        geometryAttribute = qFieldXPath;

        {
          const auto parts = geometryAttribute.split( '/' );
          if ( parts.size() > 1 )
            geometryAttribute = parts[0];
        }
        {
          const auto parts = geometryAttribute.split( ':' );
          if ( parts.size() == 2 )
            geometryAttribute = parts[1];
        }
        if ( geomType == Qgis::WkbType::MultiPolygon )
          geomType = Qgis::WkbType::MultiSurface;
        else if ( geomType == Qgis::WkbType::MultiLineString )
          geomType = Qgis::WkbType::MultiCurve;

        QgsDebugMsgLevel( u"geometry field: %1, xpath: %2"_s.arg( geometryAttribute ).arg( qFieldXPath ), 4 );
        geometryMaybeMissing = OGR_F_GetFieldAsInteger( hFeatureFieldsMD.get(), fieldMinOccursIdx ) == 0;
      }
    }
    else if ( EQUAL( fieldCategory, "REGULAR" ) && !fieldIsList )
    {
      QMetaType::Type type = getVariantTypeFromXML( QString::fromUtf8( fieldType ) );
      if ( type != QMetaType::Type::UnknownType )
      {
        fields.append( QgsField( fieldName, type, fieldType ) );
      }
      else
      {
        // unhandled:duration, base64Binary, hexBinary, anyType
        QgsDebugMsgLevel(
          u"unhandled type for field %1: xpath=%2 is_list=%3 type=%4 category=%5"_s.arg( fieldName ).arg( fieldXPath ).arg( fieldIsList ).arg( fieldType ).arg( fieldCategory ), 3
        );
        fields.append( QgsField( fieldName, QMetaType::Type::QString, fieldType ) );
      }
      sharedData->mFieldNameToXPathAndIsNestedContentMap[fieldName] = QPair<QString, bool>( fieldXPath, false );
    }
    else
    {
      QgsField field( fieldName, QMetaType::Type::QString );
      field.setEditorWidgetSetup( QgsEditorWidgetSetup( u"JsonEdit"_s, QVariantMap() ) );
      fields.append( field );
      sharedData->mFieldNameToXPathAndIsNestedContentMap[fieldName] = QPair<QString, bool>( fieldXPath, true );
    }
  }

  return true;
}

bool QgsXmlSchemaAnalyzer::readAttributesFromSchemaWithoutGMLAS(
  const QString &translatedProviderName,
  QgsBackgroundCachedSharedData *sharedData,
  QDomDocument &schemaDoc,
  const QString &prefixedTypename,
  QString &geometryAttribute,
  QgsFields &fields,
  Qgis::WkbType &geomType,
  QString &errorMsg,
  bool &mayTryWithGMLAS
)
{
  mayTryWithGMLAS = false;

  //get the <schema> root element
  QDomNodeList schemaNodeList = schemaDoc.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, u"schema"_s );
  if ( schemaNodeList.length() < 1 )
  {
    errorMsg = QObject::tr( "Cannot find schema root element" );
    return false;
  }
  QDomElement schemaElement = schemaNodeList.at( 0 ).toElement();
  sharedData->mApplicationNamespace = schemaElement.attribute( u"targetNamespace"_s );

  // Remove the namespace on the typename
  QString unprefixedTypename = prefixedTypename;
  if ( unprefixedTypename.contains( ':' ) )
  {
    unprefixedTypename = unprefixedTypename.section( ':', 1 );
  }

  // Find the element whose name is the typename that interests us, and
  // collect the correspond type.
  QDomElement elementElement = schemaElement.firstChildElement( u"element"_s );
  QString elementTypeString;
  QDomElement complexTypeElement;
  while ( !elementElement.isNull() )
  {
    QString name = elementElement.attribute( u"name"_s );
    if ( name == unprefixedTypename )
    {
      elementTypeString = elementElement.attribute( u"type"_s );
      if ( elementTypeString.isEmpty() )
      {
        // e.g http://afnemers.ruimtelijkeplannen.nl/afnemers2012/services?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=app:Bouwvlak
        complexTypeElement = elementElement.firstChildElement( u"complexType"_s );
      }
      break;
    }
    elementElement = elementElement.nextSiblingElement( u"element"_s );
  }
  // Try to get a complex type whose name contains the unprefixed typename
  if ( elementTypeString.isEmpty() && complexTypeElement.isNull() )
  {
    const QDomNodeList complexElements = schemaElement.elementsByTagName( u"complexType"_s );
    for ( int i = 0; i < complexElements.size(); i++ )
    {
      if ( complexElements.at( i ).toElement().attribute( u"name"_s ).contains( unprefixedTypename ) )
      {
        complexTypeElement = complexElements.at( i ).toElement();
        break;
      }
    }
  }
  // Give up :(
  if ( elementTypeString.isEmpty() && complexTypeElement.isNull() )
  {
    // "http://demo.deegree.org/inspire-workspace/services/wfs?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=ad:Address"
    QDomElement iter = schemaElement.firstChildElement();
    bool onlyIncludeOrImport = true;
    bool foundImport = false;
    int countInclude = 0;
    QDomElement includeElement;
    while ( !iter.isNull() )
    {
      if ( iter.tagName() == "import"_L1 )
        foundImport = true;
      else if ( iter.tagName() == "include"_L1 )
      {
        countInclude++;
        if ( countInclude == 1 )
        {
          includeElement = iter;
        }
      }
      else
      {
        onlyIncludeOrImport = false;
        break;
      }
      iter = iter.nextSiblingElement();
    }
    if ( foundImport && onlyIncludeOrImport )
    {
      errorMsg = QObject::tr( "It is probably a schema for Complex Features." );
      mayTryWithGMLAS = true;
    }
    // e.g http://services.cuzk.cz/wfs/inspire-CP-wfs.asp?SERVICE=WFS&VERSION=2.0.0&REQUEST=DescribeFeatureType
    // which has a single  <include schemaLocation="http://inspire.ec.europa.eu/schemas/cp/4.0/CadastralParcels.xsd"/>
    // In that case, follow the link.
    else if ( !foundImport && countInclude == 1 )
    {
      QString schemaLocation = includeElement.attribute( u"schemaLocation"_s );
      QgsDebugMsgLevel( u"DescribeFeatureType response redirects to: %1"_s.arg( schemaLocation ), 4 );

      QgsWFSDescribeFeatureType describeFeatureType( sharedData->mURI );
      if ( !describeFeatureType.sendGET( schemaLocation, QString(), true, false ) )
      {
        errorMsg = QObject::tr( "Cannot find schema indicated in DescribeFeatureType response." );
        QgsMessageLog::logMessage( QObject::tr( "DescribeFeatureType network request failed for url %1: %2" ).arg( schemaLocation, describeFeatureType.errorMessage() ), translatedProviderName );
        return false;
      }

      QByteArray response = describeFeatureType.response();
      QDomDocument describeFeatureDocument;
      if ( !describeFeatureDocument.setContent( response, true, &errorMsg ) )
      {
        QgsDebugMsgLevel( response, 4 );
        errorMsg = QObject::tr( "DescribeFeatureType XML parse failed for url %1: %2" ).arg( schemaLocation, errorMsg );
      }

      return readAttributesFromSchemaWithoutGMLAS( translatedProviderName, sharedData, describeFeatureDocument, prefixedTypename, geometryAttribute, fields, geomType, errorMsg, mayTryWithGMLAS );
    }
    else
    {
      errorMsg = QObject::tr( "Cannot find element '%1'" ).arg( unprefixedTypename );
      mayTryWithGMLAS = true;
    }
    return false;
  }

  //remove the namespace on type
  if ( elementTypeString.contains( ':' ) )
  {
    elementTypeString = elementTypeString.section( ':', 1 );
  }

  if ( complexTypeElement.isNull() )
  {
    //the <complexType> element corresponding to the feature type
    complexTypeElement = schemaElement.firstChildElement( u"complexType"_s );
    while ( !complexTypeElement.isNull() )
    {
      QString name = complexTypeElement.attribute( u"name"_s );
      if ( name == elementTypeString )
      {
        break;
      }
      complexTypeElement = complexTypeElement.nextSiblingElement( u"complexType"_s );
    }
    if ( complexTypeElement.isNull() )
    {
      errorMsg = QObject::tr( "Cannot find ComplexType element '%1'" ).arg( elementTypeString );
      return false;
    }
  }

  //we have the relevant <complexType> element. Now find out the geometry and the thematic attributes
  QDomNodeList attributeNodeList = complexTypeElement.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, u"element"_s );
  if ( attributeNodeList.size() < 1 )
  {
    errorMsg = QObject::tr( "Cannot find attribute elements" );
    mayTryWithGMLAS = true;
    return false;
  }

  bool foundGeometryAttribute = false;

  for ( int i = 0; i < attributeNodeList.size(); ++i )
  {
    QDomElement attributeElement = attributeNodeList.at( i ).toElement();

    //attribute name
    QString name = attributeElement.attribute( u"name"_s );
    // Some servers like http://ogi.state.ok.us/geoserver/wfs on layer ogi:doq_centroids
    // return attribute names padded with spaces. See https://github.com/qgis/QGIS/issues/13486
    // I'm not completely sure how legal this
    // is but this validates with Xerces 3.1, and its schema analyzer does also the trimming.
    name = name.trimmed();

    //attribute type
    QString type = attributeElement.attribute( u"type"_s );
    if ( type.isEmpty() )
    {
      QDomElement extension = attributeElement.firstChildElement( u"complexType"_s ).firstChildElement( u"simpleContent"_s ).firstChildElement( u"extension"_s );
      if ( !extension.isNull() )
      {
        type = extension.attribute( u"base"_s );
      }
    }

    // attribute ref
    QString ref = attributeElement.attribute( u"ref"_s );

    const thread_local QRegularExpression gmlPT( u"gml:(.*)PropertyType"_s );
    const thread_local QRegularExpression gmlRefProperty( u"gml:(.*)Property"_s );

    // gmgml: is Geomedia Web Server
    if ( !foundGeometryAttribute && type == "gmgml:Polygon_Surface_MultiSurface_CompositeSurfacePropertyType"_L1 )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = Qgis::WkbType::MultiPolygon;
    }
    else if ( !foundGeometryAttribute && type == "gmgml:LineString_Curve_MultiCurve_CompositeCurvePropertyType"_L1 )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = Qgis::WkbType::MultiLineString;
    }
    // such as http://go.geozug.ch/Zug_WFS_Baumkataster/service.svc/get
    else if ( type == "gmgml:Point_MultiPointPropertyType"_L1 )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = Qgis::WkbType::MultiPoint;
    }
    //is it a geometry attribute?
    // the GeometryAssociationType has been seen in #11785
    else if ( !foundGeometryAttribute && ( type.indexOf( gmlPT ) == 0 || type == "gml:GeometryAssociationType"_L1 ) )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      // We have a choice parent element we cannot assume any valid information over the geometry type
      if ( attributeElement.parentNode().nodeName() == "choice"_L1 && !attributeElement.nextSibling().isNull() )
        geomType = Qgis::WkbType::Unknown;
      else
      {
        const QRegularExpressionMatch match = gmlPT.match( type );
        geomType = QgsOgcUtils::geomTypeFromPropertyType( match.captured( 1 ) );
      }
    }
    //MH 090428: sometimes the <element> tags for geometry attributes have only attribute ref="gml:polygonProperty"
    //Note: this was deprecated with GML3.
    else if ( !foundGeometryAttribute && ref.indexOf( gmlRefProperty ) == 0 )
    {
      foundGeometryAttribute = true;
      geometryAttribute = ref.mid( 4 ); // Strip gml: prefix

      const QRegularExpressionMatch match = gmlRefProperty.match( ref );
      QString propertyType( match.captured( 1 ) );
      // Set the first character in upper case
      propertyType = propertyType.at( 0 ).toUpper() + propertyType.mid( 1 );
      geomType = QgsOgcUtils::geomTypeFromPropertyType( propertyType );
    }
    else if ( !name.isEmpty() )
    {
      const QMetaType::Type attributeType = getVariantTypeFromXML( type );
      if ( attributeType != QMetaType::Type::UnknownType )
      {
        fields.append( QgsField( name, attributeType, type ) );
      }
      else
      {
        mayTryWithGMLAS = true;
        fields.append( QgsField( name, QMetaType::Type::QString, type ) );
      }
    }
  }
  if ( !foundGeometryAttribute )
  {
    geomType = Qgis::WkbType::NoGeometry;
  }

  return true;
}
