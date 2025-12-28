/***************************************************************************
                              qgswfsprovider.cpp
                              -------------------
  begin                : July 25, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsprovider.h"

#include <cfloat>
#include <cpl_string.h>
#include <gdal.h>

#include "qgis.h"
#include "qgsbackgroundcachedfeaturesource.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfeature.h"
#include "qgsfeaturedownloader.h"
#include "qgsfeedback.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsogcutils.h"
#include "qgssettings.h"
#include "qgssqliteutils.h"
#include "qgswfscapabilities.h"
#include "qgswfsconstants.h"
#include "qgswfsdescribefeaturetype.h"
#include "qgswfsfeaturedownloaderimpl.h"
#include "qgswfsgetcapabilities.h"
#include "qgswfsshareddata.h"
#include "qgswfstransactionrequest.h"
#include "qgswfsutils.h"
#include "qgsxmlschemaanalyzer.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QDomDocument>
#include <QDomNodeList>
#include <QFile>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPair>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QWidget>

#include "moc_qgswfsprovider.cpp"

const QString QgsWFSProvider::WFS_PROVIDER_KEY = u"WFS"_s;
const QString QgsWFSProvider::WFS_PROVIDER_DESCRIPTION = u"WFS data provider"_s;

QgsWFSProvider::QgsWFSProvider( const QString &uri, const ProviderOptions &options, const QgsWfsCapabilities &caps )
  : QgsVectorDataProvider( uri, options )
  , mShared( new QgsWFSSharedData( uri ) )
{
  mShared->mCaps = caps;
  mShared->mServerMaxFeatures = caps.maxFeatures;

  connect( mShared.get(), &QgsWFSSharedData::raiseError, this, &QgsWFSProvider::pushErrorSlot );
  connect( mShared.get(), &QgsWFSSharedData::extentUpdated, this, &QgsWFSProvider::fullExtentCalculated );

  if ( uri.isEmpty() )
  {
    mValid = false;
    return;
  }

  if ( mShared->mURI.typeName().isEmpty() )
  {
    QgsMessageLog::logMessage( tr( "Missing or empty 'typename' URI parameter" ), tr( "WFS" ) );
    mValid = false;
    return;
  }

  const QSet<QString> &unknownParamKeys = mShared->mURI.unknownParamKeys();
  if ( !unknownParamKeys.isEmpty() )
  {
    QString msg = tr( "The following unknown parameter(s) have been found in the URI: " );
    bool firstOne = true;
    for ( const QString &key : unknownParamKeys )
    {
      if ( !firstOne )
        msg += ", "_L1;
      firstOne = false;
      msg += key;
    }
    QgsMessageLog::logMessage( msg, tr( "WFS" ) );
  }

  //create mSourceCrs from url if possible [WBC 111221] refactored from GetFeatureGET()
  QString srsname = mShared->mURI.SRSName();
  if ( !srsname.isEmpty() )
  {
    if ( srsname == "EPSG:900913"_L1 )
      mShared->mSourceCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( u"EPSG:3857"_s );
    else
      mShared->mSourceCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( srsname );
  }

  // Must be called first to establish the version, in case we are in auto-detection
  if ( !getCapabilities() )
  {
    mValid = false;
    return;
  }

  if ( !mShared->mURI.sql().isEmpty() )
  {
    if ( !processSQL( mShared->mURI.sql(), mProcessSQLErrorMsg, mProcessSQLWarningMsg ) )
    {
      QgsMessageLog::logMessage( mProcessSQLErrorMsg, tr( "WFS" ) );
      mValid = false;
      return;
    }
    mSubsetString = mShared->mURI.sql();
  }
  else
  {
    mSubsetString = mShared->mURI.filter();

    //fetch attributes of layer and type of its geometry attribute
    //WBC 111221: extracting geometry type here instead of getFeature allows successful
    //layer creation even when no features are retrieved (due to, e.g., BBOX or FILTER)
    if ( !describeFeatureType( mShared->mGeometryAttribute, mShared->mFields, mShared->mWKBType, mGeometryMaybeMissing ) )
    {
      mValid = false;
      return;
    }
    mThisTypenameFields = mShared->mFields;
    mLayerPropertiesListWhenNoSqlRequest = mShared->mLayerPropertiesList;
  }

  if ( !mShared->computeFilter( mProcessSQLErrorMsg ) )
  {
    QgsMessageLog::logMessage( mProcessSQLErrorMsg, tr( "WFS" ) );
    mValid = false;
    return;
  }

  if ( mShared->mWKBType == Qgis::WkbType::Unknown && mShared->mURI.hasGeometryTypeFilter() && mShared->mCaps.supportsGeometryTypeFilters() )
  {
    mShared->mWKBType = mShared->mURI.geometryTypeFilter();
    if ( mShared->mWKBType != Qgis::WkbType::Unknown )
    {
      mShared->computeGeometryTypeFilter();
    }
  }

  if ( !mShared->mURI.skipInitialGetFeature() || mShared->mURI.forceInitialGetFeature() )
  {
    issueInitialGetFeature( mShared->mURI.forceInitialGetFeature() );
  }
}

void QgsWFSProvider::issueInitialGetFeature( bool force )
{
  // Skip if already issued
  if ( mShared->initialGetFeatureIssued() )
  {
    return;
  }

  const auto GetGeometryTypeFromOneFeature = [&]( bool includeBbox ) {
    const bool requestMadeFromMainThread = QThread::currentThread() == QApplication::instance()->thread();
    auto downloader = std::make_unique<QgsFeatureDownloader>();

    if ( includeBbox )
    {
      // include a large BBOX filter to get features with a non-null geometry
      if ( mShared->mSourceCrs.isGeographic() )
        mShared->setCurrentRect( QgsRectangle( -180, -90, 180, 90 ) );
      else
        mShared->setCurrentRect( QgsRectangle( -1e8, -1e8, 1e8, 1e8 ) );
    }

    downloader->setImpl( std::make_unique<QgsWFSFeatureDownloaderImpl>( mShared.get(), downloader.get(), requestMadeFromMainThread ) );
    connect( downloader.get(), qOverload<QVector<QgsFeatureUniqueIdPair>>( &QgsFeatureDownloader::featureReceived ), this, [&]( QVector<QgsFeatureUniqueIdPair> list ) {
      // Download was ok, set it as done
      mShared->setInitialGetFeatureIssued( true );
      QgsWFSProvider::featureReceivedAnalyzeOneFeature( list, force );
    } );

    if ( requestMadeFromMainThread )
    {
      auto processEvents = []() {
        QApplication::processEvents();
      };
      connect( downloader.get(), &QgsFeatureDownloader::resumeMainThread, this, processEvents );
    }
    downloader->run( false, /* serialize features */
                     1 /* maxfeatures */ );

    mShared->setCurrentRect( QgsRectangle() );
  };

  const Qgis::WkbType initialGeometryType = mShared->mWKBType;

  const auto TryToDetectGeometryType = [&]() {
    // try first without a BBOX, because some servers exhibit very poor
    // performance when being requested on a large extent
    GetGeometryTypeFromOneFeature( false );
    if ( initialGeometryType == Qgis::WkbType::Unknown )
    {
      bool noGeometryFound = ( mShared->mWKBType == Qgis::WkbType::NoGeometry );
      if ( noGeometryFound )
        mShared->mWKBType = Qgis::WkbType::Unknown;

      // If we still didn't get the geometry type, and have a filter, temporarily
      // disable the filter.
      // See https://github.com/qgis/QGIS/issues/43950
      if ( mShared->mWKBType == Qgis::WkbType::Unknown && !mSubsetString.isEmpty() )
      {
        const QString oldFilter = mShared->setWFSFilter( QString() );
        GetGeometryTypeFromOneFeature( false );
        if ( mShared->mWKBType == Qgis::WkbType::NoGeometry )
        {
          noGeometryFound = true;
          mShared->mWKBType = Qgis::WkbType::Unknown;
        }
        if ( mShared->mWKBType == Qgis::WkbType::Unknown )
          GetGeometryTypeFromOneFeature( true );
        mShared->setWFSFilter( oldFilter );
      }
      else if ( mShared->mWKBType == Qgis::WkbType::Unknown )
        GetGeometryTypeFromOneFeature( true );

      if ( noGeometryFound && mShared->mWKBType == Qgis::WkbType::Unknown )
        mShared->mWKBType = Qgis::WkbType::NoGeometry;
    }
  };

  // For WFS = 1.0, issue a GetFeature on one feature to check
  // if we do not known the exact geometry type from
  // describeFeatureType()
  if ( mShared->mWFSVersion.startsWith( "1.0"_L1 ) && mShared->mWKBType == Qgis::WkbType::Unknown )
  {
    TryToDetectGeometryType();
  }
  // For WFS >= 1.1, by default, issue a GetFeature on one feature to check
  // if gml:description, gml:identifier, gml:name attributes are
  // present (unless mShared->mURI.skipInitialGetFeature() returns false),
  // unless gmlId, gmlName, gmlDescription fields are found (some servers use
  // that hack since gml:description/identifier/name attributes might be
  // missed by clients...).
  // Another reason to issue it if we do not known the exact geometry type
  // from describeFeatureType()
  else if ( !mShared->mWFSVersion.startsWith( "1.0"_L1 ) && ( mShared->mWKBType == Qgis::WkbType::Unknown || mShared->mFields.indexOf( "gmlId"_L1 ) < 0 || mShared->mFields.indexOf( "gmlName"_L1 ) < 0 || mShared->mFields.indexOf( "gmlDescription"_L1 ) < 0 ) )
  {
    // Try to see if gml:description, gml:identifier, gml:name attributes are
    // present. So insert them temporarily in mShared->mFields so that the
    // GML parser can detect them.
    const auto addGMLFields = [this]( bool forceAdd ) {
      if ( mShared->mFields.indexOf( "description"_L1 ) < 0 && ( forceAdd || mSampleFeatureHasDescription ) )
        mShared->mFields.append( QgsField( u"description"_s, QMetaType::Type::QString, u"xsd:string"_s ) );
      if ( mShared->mFields.indexOf( "identifier"_L1 ) < 0 && ( forceAdd || mSampleFeatureHasIdentifier ) )
        mShared->mFields.append( QgsField( u"identifier"_s, QMetaType::Type::QString, u"xsd:string"_s ) );
      if ( mShared->mFields.indexOf( "name"_L1 ) < 0 && ( forceAdd || mSampleFeatureHasName ) )
        mShared->mFields.append( QgsField( u"name"_s, QMetaType::Type::QString, u"xsd:string"_s ) );
    };

    const QgsFields fieldsBackup = mShared->mFields;
    addGMLFields( true );

    TryToDetectGeometryType();

    // Edit the final mFields to add the description, identifier, name fields
    // if they were actually found.
    mShared->mFields.clear();
    addGMLFields( false );
    for ( const QgsField &field : fieldsBackup )
      mShared->mFields.append( field );
  }
}

QgsWFSProvider::~QgsWFSProvider()
{
  QgsDebugMsgLevel( u"~QgsWFSProvider()"_s, 4 );
}

class QgsWFSProviderSQLFunctionValidator : public QgsSQLStatement::RecursiveVisitor
{
  public:
    QgsWFSProviderSQLFunctionValidator(
      const QList<QgsWfsCapabilities::Function> &spatialPredicatesList,
      const QList<QgsWfsCapabilities::Function> &functionList
    );

    bool hasError() const { return mError; }

    const QString &errorMessage() const { return mErrorMessage; }

  protected:
    using QgsSQLStatement::RecursiveVisitor::visit;
    void visit( const QgsSQLStatement::NodeFunction &n ) override;

  private:
    const QList<QgsWfsCapabilities::Function> &mSpatialPredicatesList;
    const QList<QgsWfsCapabilities::Function> &mFunctionList;
    bool mError = false;
    QString mErrorMessage;
};

QgsWFSProviderSQLFunctionValidator::QgsWFSProviderSQLFunctionValidator(
  const QList<QgsWfsCapabilities::Function> &spatialPredicatesList,
  const QList<QgsWfsCapabilities::Function> &functionList
)
  : mSpatialPredicatesList( spatialPredicatesList )
  , mFunctionList( functionList )
{
}

void QgsWFSProviderSQLFunctionValidator::visit( const QgsSQLStatement::NodeFunction &n )
{
  if ( !mError )
  {
    bool foundMatch = false;
    const auto constMSpatialPredicatesList = mSpatialPredicatesList;
    for ( const QgsWfsCapabilities::Function &f : constMSpatialPredicatesList )
    {
      if ( n.name().compare( f.name, Qt::CaseInsensitive ) == 0 || QString( "ST_" + n.name() ).compare( f.name, Qt::CaseInsensitive ) == 0 )
      {
        foundMatch = true;
      }
    }
    const auto constMFunctionList = mFunctionList;
    for ( const QgsWfsCapabilities::Function &f : constMFunctionList )
    {
      if ( n.name().compare( f.name, Qt::CaseInsensitive ) == 0 )
      {
        foundMatch = true;
      }
    }
    if ( !foundMatch )
    {
      mError = true;
      mErrorMessage = QObject::tr( "Function '%1' is not declared by the WFS server" ).arg( n.name() );
    }
    QgsSQLStatement::RecursiveVisitor::visit( n );
  }
}

class QgsWFSProviderSQLColumnRefValidator : public QgsSQLStatement::RecursiveVisitor
{
  public:
    QgsWFSProviderSQLColumnRefValidator(
      const QgsWfsCapabilities &caps,
      const QString &defaultTypeName,
      const QMap<QString, QString> &mapTypenameAliasToTypename,
      const QMap<QString, QgsFields> &mapTypenameToFields,
      const QMap<QString, QString> &mapTypenameToGeometryAttribute
    );

    bool hasError() const { return mError; }

    const QString &errorMessage() const { return mErrorMessage; }

  protected:
    using QgsSQLStatement::RecursiveVisitor::visit;
    void visit( const QgsSQLStatement::NodeColumnRef &n ) override;

  private:
    const QgsWfsCapabilities mCaps;
    QString mDefaultTypeName;
    const QMap<QString, QString> &mMapTableAliasToName;
    const QMap<QString, QgsFields> &mMapTypenameToFields;
    const QMap<QString, QString> &mMapTypenameToGeometryAttribute;
    bool mError = false;
    QString mErrorMessage;
};

QgsWFSProviderSQLColumnRefValidator::QgsWFSProviderSQLColumnRefValidator(
  const QgsWfsCapabilities &caps,
  const QString &defaultTypeName,
  const QMap<QString, QString> &mapTypenameAliasToTypename,
  const QMap<QString, QgsFields> &mapTypenameToFields,
  const QMap<QString, QString> &mapTypenameToGeometryAttribute
)
  : mCaps( caps )
  , mDefaultTypeName( defaultTypeName )
  , mMapTableAliasToName( mapTypenameAliasToTypename )
  , mMapTypenameToFields( mapTypenameToFields )
  , mMapTypenameToGeometryAttribute( mapTypenameToGeometryAttribute )
{
}

void QgsWFSProviderSQLColumnRefValidator::visit( const QgsSQLStatement::NodeColumnRef &n )
{
  if ( !mError && !n.star() )
  {
    QString typeName = mDefaultTypeName;
    if ( !n.tableName().isEmpty() )
    {
      if ( mMapTableAliasToName.contains( n.tableName() ) )
        typeName = mMapTableAliasToName[n.tableName()];
      else if ( mMapTableAliasToName.contains( mCaps.addPrefixIfNeeded( n.tableName() ) ) )
        typeName = mMapTableAliasToName[mCaps.addPrefixIfNeeded( n.tableName() )];
      else
      {
        mError = true;
        mErrorMessage = QObject::tr( "Column '%1' references a non existing table" ).arg( n.dump() );
        return;
      }
    }

    QgsFields tableFields = mMapTypenameToFields[typeName];
    int idx = tableFields.lookupField( n.name() );
    if ( idx < 0 && mMapTypenameToGeometryAttribute[typeName] != n.name() )
    {
      mError = true;
      mErrorMessage = QObject::tr( "Column '%1' references a non existing field" ).arg( n.dump() );
      return;
    }

    QgsSQLStatement::RecursiveVisitor::visit( n );
  }
}


bool QgsWFSProvider::processSQL( const QString &sqlString, QString &errorMsg, QString &warningMsg )
{
  QgsDebugMsgLevel( u"Processing SQL: %1"_s.arg( sqlString ), 4 );
  errorMsg.clear();
  warningMsg.clear();
  QgsSQLStatement sql( sqlString );
  if ( sql.hasParserError() )
  {
    QString parserErrorString( sql.parserErrorString() );
    QStringList parts( parserErrorString.split( ',' ) );
    parserErrorString.clear();
    const auto constParts = parts;
    for ( const QString &part : constParts )
    {
      QString newPart( part );
      if ( part == "syntax error"_L1 )
        newPart = tr( "Syntax error." );
      else if ( part == " unexpected $end"_L1 )
        newPart = tr( "Missing content at end of string." );
      else if ( part.startsWith( " unexpected "_L1 ) )
        newPart = tr( "%1 is unexpected." ).arg( part.mid( u" unexpected "_s.size() ) );
      else if ( part.startsWith( " expecting "_L1 ) )
        newPart = tr( "%1 is expected instead." ).arg( part.mid( u" expecting "_s.size() ) );
      if ( !parserErrorString.isEmpty() )
        parserErrorString += ' '_L1;
      parserErrorString += newPart;
    }
    parserErrorString.replace( " or "_L1, tr( "%1 or %2" ).arg( QString(), QString() ) );
    parserErrorString.replace( "COMMA"_L1, tr( "comma" ) );
    parserErrorString.replace( "IDENTIFIER"_L1, tr( "an identifier" ) );
    errorMsg = tr( "SQL query is invalid: %1" ).arg( parserErrorString );
    return false;
  }
  if ( !sql.doBasicValidationChecks( errorMsg ) )
  {
    errorMsg = tr( "SQL query is invalid: %1" ).arg( errorMsg );
    return false;
  }
  if ( sql.rootNode()->nodeType() != QgsSQLStatement::ntSelect )
  {
    // Shouldn't happen
    QgsDebugError( u"SQL statement is not a SELECT. This should not happen"_s );
    return false;
  }
  const QgsSQLStatement::NodeSelect *select = dynamic_cast<const QgsSQLStatement::NodeSelect *>( sql.rootNode() );
  if ( !select )
  {
    // Makes Coverity happy, but cannot happen in practice
    QgsDebugError( u"should not happen"_s );
    return false;
  }
  mShared->mDistinctSelect = select->distinct();

  QMap<QString, QString> mapTypenameAliasToTypename;
  QMap<QString, QString> mapTypenameToTypenameAlias; // reverse map of the previous one
  QList<QgsSQLStatement::NodeTableDef *> tables = select->tables();
  QList<QString> typenameList;
  bool severalTablesWithSameNameButDifferentPrefix = false;
  QSet<QString> unprefixTypenames;
  const auto constTables = tables;
  for ( QgsSQLStatement::NodeTableDef *table : constTables )
  {
    QString prefixedTypename( mShared->mCaps.addPrefixIfNeeded( table->name() ) );
    if ( prefixedTypename.isEmpty() )
    {
      if ( mShared->mCaps.setAmbiguousUnprefixedTypename.contains( table->name() ) )
      {
        errorMsg = tr( "Typename '%1' is ambiguous without prefix" ).arg( table->name() );
      }
      else
      {
        errorMsg = tr( "Typename '%1' is unknown" ).arg( table->name() );
      }
      return false;
    }
    typenameList << prefixedTypename;
    if ( unprefixTypenames.contains( QgsWFSUtils::removeNamespacePrefix( prefixedTypename ) ) )
    {
      severalTablesWithSameNameButDifferentPrefix = true;
    }
    unprefixTypenames.insert( QgsWFSUtils::removeNamespacePrefix( prefixedTypename ) );
    if ( table->alias().isEmpty() )
    {
      mapTypenameAliasToTypename[prefixedTypename] = prefixedTypename;
      mapTypenameToTypenameAlias[prefixedTypename] = prefixedTypename;
    }
    else
    {
      mapTypenameAliasToTypename[table->alias()] = prefixedTypename;
      mapTypenameToTypenameAlias[prefixedTypename] = table->alias();
    }
  }

  QList<QgsSQLStatement::NodeJoin *> joins = select->joins();
  const auto constJoins = joins;
  for ( QgsSQLStatement::NodeJoin *join : constJoins )
  {
    QgsSQLStatement::NodeTableDef *table = join->tableDef();
    QString prefixedTypename( mShared->mCaps.addPrefixIfNeeded( table->name() ) );
    if ( prefixedTypename.isEmpty() )
    {
      if ( mShared->mCaps.setAmbiguousUnprefixedTypename.contains( table->name() ) )
      {
        errorMsg = tr( "Typename '%1' is ambiguous without prefix" ).arg( table->name() );
      }
      else
      {
        errorMsg = tr( "Typename '%1' is unknown" ).arg( table->name() );
      }
      return false;
    }
    typenameList << prefixedTypename;
    if ( unprefixTypenames.contains( QgsWFSUtils::removeNamespacePrefix( prefixedTypename ) ) )
    {
      severalTablesWithSameNameButDifferentPrefix = true;
    }
    unprefixTypenames.insert( QgsWFSUtils::removeNamespacePrefix( prefixedTypename ) );
    if ( table->alias().isEmpty() )
    {
      mapTypenameAliasToTypename[prefixedTypename] = prefixedTypename;
      mapTypenameToTypenameAlias[prefixedTypename] = prefixedTypename;
    }
    else
    {
      mapTypenameAliasToTypename[table->alias()] = prefixedTypename;
      mapTypenameToTypenameAlias[prefixedTypename] = table->alias();
    }
  }

  if ( typenameList.size() > 1 && !mShared->mCaps.supportsJoins )
  {
    errorMsg = tr( "JOINs are not supported by this server" );
    return false;
  }

  if ( !typenameList.contains( mShared->mURI.typeName() ) )
  {
    errorMsg = tr( "FROM or JOIN clause should contain the table name '%1'" ).arg( mShared->mURI.typeName() );
    return false;
  }

  QString concatenatedTypenames;
  for ( const QString &typeName : std::as_const( typenameList ) )
  {
    if ( !concatenatedTypenames.isEmpty() )
      concatenatedTypenames += ','_L1;
    concatenatedTypenames += typeName;
  }

  QgsWFSDescribeFeatureType describeFeatureType( mShared->mURI );
  if ( !describeFeatureType.requestFeatureType( mShared->mWFSVersion, concatenatedTypenames, mShared->mCaps ) )
  {
    errorMsg = tr( "DescribeFeatureType failed for url %1: %2" ).arg( dataSourceUri(), describeFeatureType.errorMessage() );
    return false;
  }

  QByteArray response = describeFeatureType.response();


  QDomDocument describeFeatureDocument;
  errorMsg.clear();
  if ( !describeFeatureDocument.setContent( response, true, &errorMsg ) )
  {
    QgsDebugMsgLevel( response, 4 );
    errorMsg = tr( "DescribeFeatureType failed for url %1: %2" ).arg( dataSourceUri(), errorMsg );
    return false;
  }

  QMap<QString, QgsFields> mapTypenameToFields;
  QMap<QString, QString> mapTypenameToGeometryAttribute;
  for ( const QString &typeName : std::as_const( typenameList ) )
  {
    QString geometryAttribute;
    QgsFields fields;
    Qgis::WkbType geomType;
    bool geometryMaybeMissing;
    if ( !QgsXmlSchemaAnalyzer::readAttributesFromSchema( QObject::tr( "WFS" ), mShared.get(), mCapabilities, describeFeatureDocument, response,
                                                          /* singleLayerContext = */ typenameList.size() == 1, typeName, geometryAttribute, fields, geomType, geometryMaybeMissing, errorMsg, mMetadataRetrievalCanceled ) )
    {
      errorMsg = tr( "Analysis of DescribeFeatureType response failed for url %1, typeName %2: %3" ).arg( dataSourceUri(), typeName, errorMsg );
      return false;
    }

    mapTypenameToFields[typeName] = fields;
    mapTypenameToGeometryAttribute[typeName] = geometryAttribute;

    if ( typeName == mShared->mURI.typeName() )
    {
      mShared->mGeometryAttribute = geometryAttribute;
      mShared->mWKBType = geomType;
      mGeometryMaybeMissing = geometryMaybeMissing;
      mThisTypenameFields = fields;
    }
  }

  setLayerPropertiesListFromDescribeFeature( describeFeatureDocument, response, typenameList, errorMsg );

  const QString &defaultTypeName = mShared->mURI.typeName();
  QgsWFSProviderSQLColumnRefValidator oColumnValidator(
    mShared->mCaps,
    defaultTypeName,
    mapTypenameAliasToTypename,
    mapTypenameToFields,
    mapTypenameToGeometryAttribute
  );
  sql.acceptVisitor( oColumnValidator );
  if ( oColumnValidator.hasError() )
  {
    errorMsg = oColumnValidator.errorMessage();
    return false;
  }

  if ( mShared->mURI.validateSqlFunctions() )
  {
    QgsWFSProviderSQLFunctionValidator oValidator( mShared->mCaps.spatialPredicatesList, mShared->mCaps.functionList );
    sql.acceptVisitor( oValidator );
    if ( oValidator.hasError() )
    {
      errorMsg = oValidator.errorMessage();
      return false;
    }
  }

  QList<QgsSQLStatement::NodeSelectedColumn *> columns = select->columns();
  QMap<QString, QPair<QString, QString>> mapFieldNameToSrcLayerNameFieldName;
  mShared->mFields.clear();
  const auto constColumns = columns;
  for ( QgsSQLStatement::NodeSelectedColumn *selectedcolumn : constColumns )
  {
    QgsSQLStatement::Node *column = selectedcolumn->column();
    if ( column->nodeType() != QgsSQLStatement::ntColumnRef )
    {
      errorMsg = tr( "Column '%1' is not a direct reference to a table column." ).arg( column->dump() );
      return false;
    }
    QgsSQLStatement::NodeColumnRef *columnRef = dynamic_cast<QgsSQLStatement::NodeColumnRef *>( column );
    Q_ASSERT( columnRef );

    QString columnTableTypename = defaultTypeName;
    if ( !columnRef->tableName().isEmpty() )
    {
      if ( mapTypenameAliasToTypename.contains( columnRef->tableName() ) )
        columnTableTypename = mapTypenameAliasToTypename[columnRef->tableName()];
      else
        columnTableTypename = mShared->mCaps.addPrefixIfNeeded( columnRef->tableName() );
    }

    if ( columnRef->star() )
    {
      // table.* syntax
      if ( !columnRef->tableName().isEmpty() )
      {
        const QgsFields tableFields = mapTypenameToFields[columnTableTypename];
        for ( int i = 0; i < tableFields.size(); i++ )
        {
          QgsField srcField = tableFields.at( i );
          QString fieldName( srcField.name() );
          // If several tables selected, prefix by table name
          if ( typenameList.size() > 1 )
          {
            QString tablePrefix( mShared->mCaps.addPrefixIfNeeded( columnRef->tableName() ) );
            if ( tablePrefix.isEmpty() ) // might be an alias
              tablePrefix = columnRef->tableName();
            if ( !severalTablesWithSameNameButDifferentPrefix )
              tablePrefix = QgsWFSUtils::removeNamespacePrefix( tablePrefix );
            fieldName = tablePrefix + "." + fieldName;
          }
          QgsField field( srcField );
          field.setName( fieldName );
          if ( mapFieldNameToSrcLayerNameFieldName.contains( fieldName ) )
          {
            errorMsg = tr( "Field '%1': a field with the same name already exists." ).arg( field.name() );
            return false;
          }

          mapFieldNameToSrcLayerNameFieldName[field.name()] = QPair<QString, QString>( columnTableTypename, srcField.name() );
          mShared->mFields.append( field );
        }
      }
      else
      {
        // * syntax
        const auto constTypenameList = typenameList;
        for ( const QString &typeName : constTypenameList )
        {
          const QgsFields tableFields = mapTypenameToFields[typeName];
          for ( int i = 0; i < tableFields.size(); i++ )
          {
            QgsField srcField = tableFields.at( i );
            QString fieldName( srcField.name() );
            // If several tables selected, prefix by table name
            if ( typenameList.size() > 1 )
            {
              QString tablePrefix( mapTypenameToTypenameAlias[typeName] );
              if ( !severalTablesWithSameNameButDifferentPrefix )
                tablePrefix = QgsWFSUtils::removeNamespacePrefix( tablePrefix );
              fieldName = tablePrefix + "." + fieldName;
            }
            QgsField field( srcField );
            field.setName( fieldName );
            mapFieldNameToSrcLayerNameFieldName[field.name()] = QPair<QString, QString>( typeName, srcField.name() );
            mShared->mFields.append( field );
          }
        }
      }
    }
    // Geometry field
    else if ( mapTypenameToGeometryAttribute[columnTableTypename] == columnRef->name() )
    {
      if ( columnTableTypename != mShared->mURI.typeName() )
      {
        warningMsg = tr( "The geometry field of a typename that is not the main typename is ignored in the selected fields." );
        QgsDebugError( warningMsg );
      }
    }
    // Regular field
    else
    {
      const QgsFields tableFields = mapTypenameToFields[columnTableTypename];
      int idx = tableFields.lookupField( columnRef->name() );
      if ( idx < 0 )
      {
        QgsDebugError( u"Should not happen. Cannot find field for %1"_s.arg( columnRef->name() ) );
        continue;
      }

      QString fieldName( columnRef->name() );
      if ( !selectedcolumn->alias().isEmpty() )
        fieldName = selectedcolumn->alias();
      else if ( !columnRef->tableName().isEmpty() )
      {
        QString tablePrefix( mShared->mCaps.addPrefixIfNeeded( columnRef->tableName() ) );
        if ( tablePrefix.isEmpty() ) // might be an alias
          tablePrefix = columnRef->tableName();
        if ( !severalTablesWithSameNameButDifferentPrefix )
          tablePrefix = QgsWFSUtils::removeNamespacePrefix( tablePrefix );
        fieldName = tablePrefix + "." + fieldName;
      }
      if ( mapFieldNameToSrcLayerNameFieldName.contains( fieldName ) )
      {
        errorMsg = tr( "Field '%1': a field with the same name already exists." ).arg( column->dump() );
        return false;
      }

      QgsField orig = tableFields.at( idx );
      QgsField field( orig );
      field.setName( fieldName );
      mapFieldNameToSrcLayerNameFieldName[field.name()] = QPair<QString, QString>( columnTableTypename, orig.name() );
      mShared->mFields.append( field );
    }
  }

  mShared->mMapFieldNameToSrcLayerNameFieldName = mapFieldNameToSrcLayerNameFieldName;

  return true;
}

bool QgsWFSProvider::setLayerPropertiesListFromDescribeFeature( QDomDocument &describeFeatureDocument, const QByteArray &response, const QStringList &typenameList, QString &errorMsg )
{
  mShared->mLayerPropertiesList.clear();
  for ( const QString &typeName : typenameList )
  {
    QString geometryAttribute;
    QgsFields fields;
    Qgis::WkbType geomType;
    bool geometryMaybeMissing;
    if ( !QgsXmlSchemaAnalyzer::readAttributesFromSchema( QObject::tr( "WFS" ), mShared.get(), mCapabilities, describeFeatureDocument, response,
                                                          /* singleLayerContext = */ typenameList.size() == 1, typeName, geometryAttribute, fields, geomType, geometryMaybeMissing, errorMsg, mMetadataRetrievalCanceled ) )
    {
      errorMsg = tr( "Analysis of DescribeFeatureType response failed for url %1, typeName %2: %3" ).arg( dataSourceUri(), typeName, errorMsg );
      return false;
    }

    QgsOgcUtils::LayerProperties layerProperties;
    layerProperties.mName = typeName;
    layerProperties.mGeometryAttribute = geometryAttribute;
    if ( typeName == mShared->mURI.typeName() )
      layerProperties.mSRSName = mShared->srsName();

    if ( typeName.contains( ':' ) )
    {
      layerProperties.mNamespaceURI = mShared->mCaps.getNamespaceForTypename( typeName );
      layerProperties.mNamespacePrefix = QgsWFSUtils::nameSpacePrefix( typeName );
    }

    mShared->mLayerPropertiesList << layerProperties;
  }
  return true;
}

void QgsWFSProvider::pushErrorSlot( const QString &errorMsg )
{
  pushError( errorMsg );
}

void QgsWFSProvider::featureReceivedAnalyzeOneFeature( const QVector<QgsFeatureUniqueIdPair> &list, bool force )
{
  const Qgis::WkbType originalType { mShared->mWKBType };

  if ( list.size() != 0 && ( force || originalType == Qgis::WkbType::Unknown ) )
  {
    QgsFeature feat = list[0].first;
    QgsGeometry geometry = feat.geometry();
    if ( geometry.isNull() )
    {
      mShared->mWKBType = Qgis::WkbType::NoGeometry;
    }
    else
    {
      mShared->mWKBType = geometry.wkbType();

      // Fragile heuristics that helps for
      // https://sampleservices.luciad.com/ogc/wfs/sampleswfs?REQUEST=GetFeature&SERVICE=WFS&TYPENAME=rivers&VERSION=1.1.0
      // In case the geometry is a geometry collection, analyze its members to
      // see if they are of the same type. If then, assume that all features
      // will be similar, and report the proper MultiPoint/MultiLineString/
      // MultiPolygon type instead.
      if ( mShared->mWKBType == Qgis::WkbType::GeometryCollection )
      {
        auto geomColl = geometry.asGeometryCollection();
        mShared->mWKBType = Qgis::WkbType::Unknown;
        for ( const auto &geom : geomColl )
        {
          if ( mShared->mWKBType == Qgis::WkbType::Unknown )
          {
            mShared->mWKBType = geom.wkbType();
          }
          else if ( mShared->mWKBType != geom.wkbType() )
          {
            mShared->mWKBType = Qgis::WkbType::Unknown;
            break;
          }
        }
        if ( mShared->mWKBType != Qgis::WkbType::Unknown )
        {
          if ( mShared->mWKBType == Qgis::WkbType::Point )
          {
            QgsDebugMsgLevel( u"Layer of unknown type. First element is a GeometryCollection of Point. Advertizing optimistically as MultiPoint"_s, 2 );
            mShared->mWKBType = Qgis::WkbType::MultiPoint;
          }
          else if ( mShared->mWKBType == Qgis::WkbType::LineString )
          {
            QgsDebugMsgLevel( u"Layer of unknown type. First element is a GeometryCollection of LineString. Advertizing optimistically as MultiLineString"_s, 2 );
            mShared->mWKBType = Qgis::WkbType::MultiLineString;
          }
          else if ( mShared->mWKBType == Qgis::WkbType::Polygon )
          {
            QgsDebugMsgLevel( u"Layer of unknown type. First element is a GeometryCollection of Polygon. Advertizing optimistically as MultiPolygon"_s, 2 );
            mShared->mWKBType = Qgis::WkbType::MultiPolygon;
          }
          else
          {
            mShared->mWKBType = Qgis::WkbType::Unknown;
          }
        }
      }
    }
  }

  if ( list.size() != 0 )
  {
    QgsFeature feat = list[0].first;
    feat.padAttributes( mShared->mFields.count() );
    mSampleFeatureHasDescription = !feat.attribute( "description" ).isNull();
    mSampleFeatureHasIdentifier = !feat.attribute( "identifier" ).isNull();
    mSampleFeatureHasName = !feat.attribute( "name" ).isNull();
  }

  // Re-assign the original type if type was not determined and force was set
  if ( force && mShared->mWKBType == Qgis::WkbType::Unknown )
  {
    mShared->mWKBType = originalType;
  }
}

QString QgsWFSProvider::subsetString() const
{
  return mSubsetString;
}

bool QgsWFSProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
{
  Q_UNUSED( updateFeatureCount )

  QgsDebugMsgLevel( u"theSql = '%1'"_s.arg( theSQL ), 4 );

  if ( theSQL == mSubsetString )
    return true;

  disconnect( mShared.get(), &QgsWFSSharedData::raiseError, this, &QgsWFSProvider::pushErrorSlot );
  disconnect( mShared.get(), &QgsWFSSharedData::extentUpdated, this, &QgsWFSProvider::fullExtentCalculated );

  // We must not change the subset string of the shared data used in another iterator/data provider ...
  mShared.reset( mShared->clone() );

  connect( mShared.get(), &QgsWFSSharedData::raiseError, this, &QgsWFSProvider::pushErrorSlot );
  connect( mShared.get(), &QgsWFSSharedData::extentUpdated, this, &QgsWFSProvider::fullExtentCalculated );

  mSubsetString = theSQL;
  clearMinMaxCache();

  // update URI
  mShared->mFields = mThisTypenameFields;
  mShared->mLayerPropertiesList.clear();
  mShared->mMapFieldNameToSrcLayerNameFieldName.clear();
  mShared->mDistinctSelect = false;
  if ( theSQL.startsWith( "SELECT "_L1, Qt::CaseInsensitive ) || theSQL.startsWith( "SELECT\t"_L1, Qt::CaseInsensitive ) || theSQL.startsWith( "SELECT\r"_L1, Qt::CaseInsensitive ) || theSQL.startsWith( "SELECT\n"_L1, Qt::CaseInsensitive ) )
  {
    QString errorMsg, warningMsg;
    if ( !processSQL( theSQL, errorMsg, warningMsg ) )
    {
      QgsMessageLog::logMessage( errorMsg, tr( "WFS" ) );
      return false;
    }
    mShared->mURI.setSql( theSQL );
    mShared->mURI.setFilter( QString() );
  }
  else
  {
    mShared->mLayerPropertiesList = mLayerPropertiesListWhenNoSqlRequest;
    mShared->mURI.setSql( QString() );
    mShared->mURI.setFilter( theSQL );
  }

  setDataSourceUri( mShared->mURI.uri() );
  QString errorMsg;
  if ( !mShared->computeFilter( errorMsg ) )
    QgsMessageLog::logMessage( errorMsg, tr( "WFS" ) );

  reloadData();

  return true;
}

QString QgsWFSProvider::subsetStringDialect() const
{
  return tr( "WFS query expression" );
}

QString QgsWFSProvider::subsetStringHelpUrl() const
{
  return u"https://docs.ogc.org/is/09-025r2/09-025r2.html#83"_s;
}

bool QgsWFSProvider::supportsSubsetString() const
{
  return true;
}

QgsAbstractFeatureSource *QgsWFSProvider::featureSource() const
{
  return new QgsBackgroundCachedFeatureSource( mShared );
}

void QgsWFSProvider::reloadProviderData()
{
  mShared->invalidateCache();
}

QDomElement QgsWFSProvider::geometryElement( const QgsGeometry &geometry, QDomDocument &transactionDoc )
{
  QDomElement gmlElem;

  // Determine axis orientation and gml version
  bool applyAxisInversion;
  QgsOgcUtils::GMLVersion gmlVersion;

  if ( mShared->mWFSVersion.startsWith( "1.1"_L1 ) || mShared->mWFSVersion.startsWith( '2'_L1 ) )
  {
    // WFS 1.1.0 uses preferably GML 3, but ESRI mapserver in 2020 doesn't like it so we stick to GML2
    if ( !mShared->mServerPrefersCoordinatesForTransactions_1_1 )
    {
      gmlVersion = QgsOgcUtils::GML_3_1_0;
    }
    else
    {
      gmlVersion = QgsOgcUtils::GML_2_1_2;
    }
    // For servers like Geomedia and QGIS Server that advertise EPSG:XXXX in capabilities even in WFS 1.1 or 2.0
    // capabilities useEPSGColumnFormat is set.
    // We follow GeoServer convention here which is to treat EPSG:4326 as lon/lat
    applyAxisInversion = ( crs().hasAxisInverted() && !mShared->mURI.ignoreAxisOrientation() && !mShared->mCaps.useEPSGColumnFormat )
                         || mShared->mURI.invertAxisOrientation();
  }
  else // 1.0
  {
    gmlVersion = QgsOgcUtils::GML_2_1_2;
    applyAxisInversion = mShared->mURI.invertAxisOrientation();
  }

  gmlElem = QgsOgcUtils::geometryToGML(
    geometry,
    transactionDoc,
    gmlVersion,
    mShared->srsName(),
    applyAxisInversion,
    QString()
  );

  return gmlElem;
}

Qgis::WkbType QgsWFSProvider::wkbType() const
{
  return mShared->mWKBType;
}

long long QgsWFSProvider::featureCount() const
{
  return mShared->getFeatureCount();
}

QgsFields QgsWFSProvider::fields() const
{
  return mShared->mFields;
}

QString QgsWFSProvider::geometryColumnName() const
{
  return mShared->mGeometryAttribute;
}

QgsCoordinateReferenceSystem QgsWFSProvider::crs() const
{
  return mShared->mSourceCrs;
}

QgsRectangle QgsWFSProvider::extent() const
{
  return mShared->consolidatedExtent();
}

bool QgsWFSProvider::isValid() const
{
  return mValid;
}

QgsFeatureIterator QgsWFSProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return QgsFeatureIterator( new QgsBackgroundCachedFeatureIterator( new QgsBackgroundCachedFeatureSource( mShared ), true, mShared, request ) );
}

bool QgsWFSProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  //create <Transaction> xml
  QDomDocument transactionDoc;
  QDomElement transactionElem = createTransactionElement( transactionDoc );
  transactionDoc.appendChild( transactionElem );

  //find out typename from uri and strip namespace prefix
  QString tname = mShared->mURI.typeName();
  if ( tname.isNull() )
  {
    return false;
  }
  tname = QgsWFSUtils::removeNamespacePrefix( tname );

  //Add the features
  QgsFeatureList::iterator featureIt = flist.begin();
  for ( ; featureIt != flist.end(); ++featureIt )
  {
    //Insert element
    QDomElement insertElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, u"Insert"_s );
    transactionElem.appendChild( insertElem );

    QDomElement featureElem = transactionDoc.createElementNS( mShared->mApplicationNamespace, tname );

    QgsAttributes featureAttributes = featureIt->attributes();
    int nAttrs = featureAttributes.size();
    for ( int i = 0; i < nAttrs; ++i )
    {
      const QVariant &value = featureAttributes.at( i );
      if ( value.isValid() && !QgsVariantUtils::isNull( value ) )
      {
        QDomElement fieldElem = transactionDoc.createElementNS( mShared->mApplicationNamespace, mShared->mFields.at( i ).name() );
        QDomText fieldText = transactionDoc.createTextNode( convertToXML( value ) );
        fieldElem.appendChild( fieldText );
        featureElem.appendChild( fieldElem );
      }
    }

    //add geometry column (as gml)
    QgsGeometry geometry = featureIt->geometry();
    if ( !geometry.isNull() )
    {
      QDomElement geomElem = transactionDoc.createElementNS( mShared->mApplicationNamespace, mShared->mGeometryAttribute );
      QgsGeometry the_geom( geometry );
      // convert to multi if the layer geom type is multi and the geom is not
      if ( QgsWkbTypes::isMultiType( this->wkbType() ) && !the_geom.isMultipart() )
      {
        the_geom.convertToMultiType();
      }

      const QDomElement gmlElem { geometryElement( the_geom, transactionDoc ) };
      if ( !gmlElem.isNull() )
      {
        geomElem.appendChild( gmlElem );
        featureElem.appendChild( geomElem );
      }
    }

    insertElem.appendChild( featureElem );
  }

  QDomDocument serverResponse;
  bool success = sendTransactionDocument( transactionDoc, serverResponse );
  if ( !success )
  {
    return false;
  }

  if ( transactionSuccess( serverResponse ) )
  {
    //transaction successful. Add the features to the cache
    QStringList idList = insertedFeatureIds( serverResponse );
    /* Fix issue with GeoServer and shapefile feature stores when no real
       feature id are returned but new0 returned instead of the featureId*/
    const auto constIdList = idList;
    for ( const QString &v : constIdList )
    {
      if ( v.startsWith( "new"_L1 ) )
      {
        reloadData();
        return true;
      }
    }
    QStringList::const_iterator idIt = idList.constBegin();
    featureIt = flist.begin();

    QVector<QgsFeatureUniqueIdPair> serializedFeatureList;
    for ( ; idIt != idList.constEnd() && featureIt != flist.end(); ++idIt, ++featureIt )
    {
      serializedFeatureList.push_back( QgsFeatureUniqueIdPair( *featureIt, *idIt ) );
    }
    mShared->serializeFeatures( serializedFeatureList );

    if ( !( flags & QgsFeatureSink::FastInsert ) )
    {
      // And now set the feature id from the one got from the database
      QMap<QString, QgsFeatureId> map;
      for ( int idx = 0; idx < serializedFeatureList.size(); idx++ )
        map[serializedFeatureList[idx].second] = serializedFeatureList[idx].first.id();

      idIt = idList.constBegin();
      featureIt = flist.begin();
      for ( ; idIt != idList.constEnd() && featureIt != flist.end(); ++idIt, ++featureIt )
      {
        if ( map.find( *idIt ) != map.end() )
          featureIt->setId( map[*idIt] );
      }
    }

    return true;
  }
  else
  {
    handleException( serverResponse );
    return false;
  }
}

bool QgsWFSProvider::deleteFeatures( const QgsFeatureIds &id )
{
  if ( id.size() < 1 )
  {
    return true;
  }

  //find out typename from uri and strip namespace prefix
  QString tname = mShared->mURI.typeName();
  if ( tname.isNull() )
  {
    return false;
  }

  //create <Transaction> xml
  QDomDocument transactionDoc;
  QDomElement transactionElem = createTransactionElement( transactionDoc );
  transactionDoc.appendChild( transactionElem );
  //delete element
  QDomElement deleteElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, u"Delete"_s );
  deleteElem.setAttribute( u"typeName"_s, tname );
  QDomElement filterElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, u"Filter"_s );


  QgsFeatureIds::const_iterator idIt = id.constBegin();
  for ( ; idIt != id.constEnd(); ++idIt )
  {
    //find out feature id
    QString gmlid = mShared->findUniqueId( *idIt );
    if ( gmlid.isEmpty() )
    {
      QgsDebugError( u"Cannot identify feature of id %1"_s.arg( *idIt ) );
      continue;
    }
    QDomElement featureIdElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, u"FeatureId"_s );
    featureIdElem.setAttribute( u"fid"_s, gmlid );
    filterElem.appendChild( featureIdElem );
  }

  deleteElem.appendChild( filterElem );
  transactionElem.appendChild( deleteElem );

  QDomDocument serverResponse;
  bool success = sendTransactionDocument( transactionDoc, serverResponse );
  if ( !success )
  {
    return false;
  }

  if ( transactionSuccess( serverResponse ) )
  {
    mShared->deleteFeatures( id );
    return true;
  }
  else
  {
    handleException( serverResponse );
    return false;
  }
}

bool QgsWFSProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  //find out typename from uri
  QString tname = mShared->mURI.typeName();
  if ( tname.isNull() )
  {
    return false;
  }

  QString namespacePrefix = QgsWFSUtils::nameSpacePrefix( tname );
  if ( !namespacePrefix.isEmpty() )
  {
    namespacePrefix += ':';
  }

  //create <Transaction> xml
  QDomDocument transactionDoc;
  QDomElement transactionElem = createTransactionElement( transactionDoc );
  transactionDoc.appendChild( transactionElem );

  QgsGeometryMap::const_iterator geomIt = geometry_map.constBegin();
  for ( ; geomIt != geometry_map.constEnd(); ++geomIt )
  {
    QString gmlid = mShared->findUniqueId( geomIt.key() );
    if ( gmlid.isEmpty() )
    {
      QgsDebugError( u"Cannot identify feature of id %1"_s.arg( geomIt.key() ) );
      continue;
    }
    QDomElement updateElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, u"Update"_s );
    updateElem.setAttribute( u"typeName"_s, tname );
    //Property
    QDomElement propertyElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, u"Property"_s );
    QDomElement nameElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, u"Name"_s );
    QDomText nameText = transactionDoc.createTextNode( namespacePrefix + mShared->mGeometryAttribute );
    nameElem.appendChild( nameText );
    propertyElem.appendChild( nameElem );
    QDomElement valueElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, u"Value"_s );

    valueElem.appendChild( geometryElement( geomIt.value(), transactionDoc ) );

    propertyElem.appendChild( valueElem );
    updateElem.appendChild( propertyElem );

    //filter
    QDomElement filterElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, u"Filter"_s );
    QDomElement featureIdElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, u"FeatureId"_s );
    featureIdElem.setAttribute( u"fid"_s, gmlid );
    filterElem.appendChild( featureIdElem );
    updateElem.appendChild( filterElem );

    transactionElem.appendChild( updateElem );
  }

  QDomDocument serverResponse;
  bool success = sendTransactionDocument( transactionDoc, serverResponse );
  if ( !success )
  {
    return false;
  }

  if ( transactionSuccess( serverResponse ) )
  {
    mShared->changeGeometryValues( geometry_map );
    return true;
  }
  else
  {
    handleException( serverResponse );
    return false;
  }
}

QString QgsWFSProvider::convertToXML( const QVariant &value )
{
  QString valueStr( value.toString() );
  if ( value.userType() == QMetaType::Type::QDateTime )
  {
    QDateTime dt = value.toDateTime().toUTC();
    if ( !dt.isNull() )
    {
      valueStr = dt.toString( u"yyyy-MM-ddThh:mm:ss.zzzZ"_s );
    }
    else
    {
      valueStr = QString();
    }
  }
  return valueStr;
}

bool QgsWFSProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  //find out typename from uri
  QString tname = mShared->mURI.typeName();
  if ( tname.isNull() )
  {
    return false;
  }

  QString namespacePrefix = QgsWFSUtils::nameSpacePrefix( tname );
  if ( !namespacePrefix.isEmpty() )
  {
    namespacePrefix += ':';
  }

  //create <Transaction> xml
  QDomDocument transactionDoc;
  QDomElement transactionElem = createTransactionElement( transactionDoc );
  transactionDoc.appendChild( transactionElem );

  QgsChangedAttributesMap::const_iterator attIt = attr_map.constBegin();
  for ( ; attIt != attr_map.constEnd(); ++attIt )
  {
    QString gmlid = mShared->findUniqueId( attIt.key() );
    if ( gmlid.isEmpty() )
    {
      QgsDebugError( u"Cannot identify feature of id %1"_s.arg( attIt.key() ) );
      continue;
    }

    QDomElement updateElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, u"Update"_s );
    updateElem.setAttribute( u"typeName"_s, tname );

    QgsAttributeMap::const_iterator attMapIt = attIt.value().constBegin();
    for ( ; attMapIt != attIt.value().constEnd(); ++attMapIt )
    {
      QString fieldName = mShared->mFields.at( attMapIt.key() ).name();
      QDomElement propertyElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, u"Property"_s );

      QDomElement nameElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, u"Name"_s );
      QDomText nameText = transactionDoc.createTextNode( namespacePrefix + fieldName );
      nameElem.appendChild( nameText );
      propertyElem.appendChild( nameElem );

      QDomElement valueElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, u"Value"_s );

      if ( attMapIt.value().isValid() && !QgsVariantUtils::isNull( attMapIt.value() ) )
      {
        // WFS does not support :nil='true'
        // if value is NULL, do not add value element
        QDomText valueText = transactionDoc.createTextNode( convertToXML( attMapIt.value() ) );
        valueElem.appendChild( valueText );
        propertyElem.appendChild( valueElem );
      }

      updateElem.appendChild( propertyElem );
    }

    //Filter
    QDomElement filterElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, u"Filter"_s );
    QDomElement featureIdElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, u"FeatureId"_s );
    featureIdElem.setAttribute( u"fid"_s, gmlid );
    filterElem.appendChild( featureIdElem );
    updateElem.appendChild( filterElem );

    transactionElem.appendChild( updateElem );
  }

  QDomDocument serverResponse;
  bool success = sendTransactionDocument( transactionDoc, serverResponse );
  if ( !success )
  {
    return false;
  }

  if ( transactionSuccess( serverResponse ) )
  {
    mShared->changeAttributeValues( attr_map );
    return true;
  }
  else
  {
    handleException( serverResponse );
    return false;
  }
}

QVariantMap QgsWFSProvider::metadata() const
{
  QVariantMap result;
  result[u"MaxFeatures"_s] = mShared->mCaps.maxFeatures;
  result[u"SupportsPaging"_s] = mShared->mCaps.supportsPaging;
  result[u"SupportsJoins"_s] = mShared->mCaps.supportsJoins;
  return result;
}

QString QgsWFSProvider::translateMetadataKey( const QString &mdKey ) const
{
  if ( mdKey == "MaxFeatures"_L1 )
  {
    return tr( "Max Features" );
  }
  else if ( mdKey == "SupportsPaging"_L1 )
  {
    return tr( "Supports Paging" );
  }
  else if ( mdKey == "SupportsJoins"_L1 )
  {
    return tr( "Supports Joins" );
  }
  else
  {
    return mdKey;
  }
};

QString QgsWFSProvider::translateMetadataValue( const QString &mdKey, const QVariant &value ) const
{
  if ( mdKey == "MaxFeatures"_L1 )
  {
    return value.toInt() == 0 ? tr( "not provided" ) : value.toString();
  }
  else if ( mdKey == "SupportsPaging"_L1 || mdKey == "SupportsJoins"_L1 )
  {
    return value.toBool() ? tr( "supported" ) : tr( "unsupported" );
  }
  else
  {
    return value.toString();
  }
}

bool QgsWFSProvider::empty() const
{
  QgsFeature f;
  QgsFeatureRequest request;
  request.setNoAttributes();
  request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );

  // Whoops, the WFS provider returns an empty iterator when we are using
  // a setLimit call in combination with a subsetString.
  // Remove this method (and default to the QgsVectorDataProvider one)
  // once this is fixed
#if 0
  request.setLimit( 1 );
#endif
  return !getFeatures( request ).nextFeature( f );
}

void QgsWFSProvider::handlePostCloneOperations( QgsVectorDataProvider *source )
{
  mShared = qobject_cast<QgsWFSProvider *>( source )->mShared;
};

bool QgsWFSProvider::describeFeatureType( QString &geometryAttribute, QgsFields &fields, Qgis::WkbType &geomType, bool &geometryMaybeMissing )
{
  fields.clear();

  QgsWFSDescribeFeatureType describeFeatureType( mShared->mURI );

  if ( !describeFeatureType.requestFeatureType( mShared->mWFSVersion, mShared->mURI.typeName(), mShared->mCaps ) )
  {
    QgsMessageLog::logMessage( tr( "DescribeFeatureType network request failed for url %1: %2" ).arg( dataSourceUri(), describeFeatureType.errorMessage() ), tr( "WFS" ) );
    return false;
  }

  QByteArray response = describeFeatureType.response();

  QgsDebugMsgLevel( response, 4 );

  QDomDocument describeFeatureDocument;
  QString errorMsg;
  if ( !describeFeatureDocument.setContent( response, true, &errorMsg ) )
  {
    QgsDebugMsgLevel( response, 4 );
    QgsMessageLog::logMessage( tr( "DescribeFeatureType XML parse failed for url %1: %2" ).arg( dataSourceUri(), errorMsg ), tr( "WFS" ) );
    return false;
  }

  if ( !QgsXmlSchemaAnalyzer::readAttributesFromSchema( QObject::tr( "WFS" ), mShared.get(), mCapabilities, describeFeatureDocument, response,
                                                        /* singleLayerContext = */ true, mShared->mURI.typeName(), geometryAttribute, fields, geomType, geometryMaybeMissing, errorMsg, mMetadataRetrievalCanceled ) )
  {
    QgsDebugMsgLevel( response, 4 );
    QgsMessageLog::logMessage( tr( "Analysis of DescribeFeatureType response failed for url %1: %2" ).arg( dataSourceUri(), errorMsg ), tr( "WFS" ) );
    pushError( errorMsg );
    return false;
  }

  setLayerPropertiesListFromDescribeFeature( describeFeatureDocument, response, { mShared->mURI.typeName() }, errorMsg );

  return true;
}

QString QgsWFSProvider::name() const
{
  return WFS_PROVIDER_KEY;
}

QString QgsWFSProvider::providerKey()
{
  return WFS_PROVIDER_KEY;
}

QString QgsWFSProvider::description() const
{
  return WFS_PROVIDER_DESCRIPTION;
}

Qgis::VectorProviderCapabilities QgsWFSProvider::capabilities() const
{
  return mCapabilities;
}

bool QgsWFSProvider::sendTransactionDocument( const QDomDocument &doc, QDomDocument &serverResponse )
{
  if ( doc.isNull() )
  {
    return false;
  }

  QgsDebugMsgLevel( doc.toString(), 4 );

  QgsWFSTransactionRequest request( mShared->mURI );
  return request.send( doc, serverResponse );
}

QDomElement QgsWFSProvider::createTransactionElement( QDomDocument &doc ) const
{
  QDomElement transactionElem = doc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, u"Transaction"_s );
  const QString WfsVersion = mShared->mWFSVersion;
  // only 1.1.0 and 1.0.0 are supported
  if ( WfsVersion == "1.1.0"_L1 || WfsVersion.startsWith( "2."_L1 ) )
  {
    transactionElem.setAttribute( u"version"_s, u"1.1.0"_s );
  }
  else
  {
    transactionElem.setAttribute( u"version"_s, u"1.0.0"_s );
  }
  transactionElem.setAttribute( u"service"_s, u"WFS"_s );
  transactionElem.setAttribute( u"xmlns:xsi"_s, u"http://www.w3.org/2001/XMLSchema-instance"_s );

  QUrl describeFeatureTypeURL = mShared->mURI.requestUrl( u"DescribeFeatureType"_s );
  // For tests (since the URL contains part of random data, we need to replace it with a fixed content)
  if ( describeFeatureTypeURL.toString().contains( "fake_qgis_http_endpoint"_L1 ) )
  {
    describeFeatureTypeURL = QUrl( u"http://fake_qgis_http_endpoint"_s );
    QUrlQuery query( describeFeatureTypeURL );
    query.addQueryItem( u"REQUEST"_s, u"DescribeFeatureType"_s );
    describeFeatureTypeURL.setQuery( query );
  }
  QUrlQuery query( describeFeatureTypeURL );
  query.addQueryItem( u"VERSION"_s, u"1.0.0"_s );
  query.addQueryItem( u"TYPENAME"_s, mShared->mURI.typeName() );
  describeFeatureTypeURL.setQuery( query );

  transactionElem.setAttribute( u"xsi:schemaLocation"_s, mShared->mApplicationNamespace + ' ' + describeFeatureTypeURL.toEncoded() );

  QString namespacePrefix = QgsWFSUtils::nameSpacePrefix( mShared->mURI.typeName() );
  if ( !namespacePrefix.isEmpty() )
  {
    transactionElem.setAttribute( "xmlns:" + namespacePrefix, mShared->mApplicationNamespace );
  }
  transactionElem.setAttribute( u"xmlns:gml"_s, QgsWFSConstants::GML_NAMESPACE );

  return transactionElem;
}

bool QgsWFSProvider::transactionSuccess( const QDomDocument &serverResponse ) const
{
  if ( serverResponse.isNull() )
  {
    return false;
  }

  QDomElement documentElem = serverResponse.documentElement();
  if ( documentElem.isNull() )
  {
    return false;
  }

  const QString WfsVersion = mShared->mWFSVersion;

  if ( WfsVersion == "1.1.0"_L1 || WfsVersion.startsWith( "2."_L1 ) )
  {
    const QDomNodeList transactionSummaryList = documentElem.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, u"TransactionSummary"_s );
    if ( transactionSummaryList.size() < 1 )
    {
      QgsDebugMsgLevel( u"TransactionSummary not found in response: %1"_s.arg( serverResponse.toString() ), 4 );
      return false;
    }

    QDomElement transactionElement { transactionSummaryList.at( 0 ).toElement() };
    QDomNodeList totalInserted = transactionElement.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, u"totalInserted"_s );
    QDomNodeList totalUpdated = transactionElement.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, u"totalUpdated"_s );
    QDomNodeList totalDeleted = transactionElement.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, u"totalDeleted"_s );
    if ( totalInserted.size() > 0 && totalInserted.at( 0 ).toElement().text().toInt() > 0 )
    {
      return true;
    }
    if ( totalUpdated.size() > 0 && totalUpdated.at( 0 ).toElement().text().toInt() > 0 )
    {
      return true;
    }
    if ( totalDeleted.size() > 0 && totalDeleted.at( 0 ).toElement().text().toInt() > 0 )
    {
      return true;
    }

    // Handle wrong QGIS server response (capital initial letter)
    totalInserted = transactionElement.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, u"TotalInserted"_s );
    totalUpdated = transactionElement.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, u"TotalUpdated"_s );
    totalDeleted = transactionElement.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, u"TotalDeleted"_s );
    if ( totalInserted.size() > 0 && totalInserted.at( 0 ).toElement().text().toInt() > 0 )
    {
      return true;
    }
    if ( totalUpdated.size() > 0 && totalUpdated.at( 0 ).toElement().text().toInt() > 0 )
    {
      return true;
    }
    if ( totalDeleted.size() > 0 && totalDeleted.at( 0 ).toElement().text().toInt() > 0 )
    {
      return true;
    }

    return false;
  }
  else
  {
    const QDomNodeList transactionResultList = documentElem.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, u"TransactionResult"_s );
    if ( transactionResultList.size() < 1 )
    {
      return false;
    }

    const QDomNodeList statusList = transactionResultList.at( 0 ).toElement().elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, u"Status"_s );
    if ( statusList.size() < 1 )
    {
      return false;
    }

    return statusList.at( 0 ).firstChildElement().localName() == "SUCCESS"_L1;
  }
}

QStringList QgsWFSProvider::insertedFeatureIds( const QDomDocument &serverResponse ) const
{
  QStringList ids;
  if ( serverResponse.isNull() )
  {
    return ids;
  }

  QDomElement rootElem = serverResponse.documentElement();
  if ( rootElem.isNull() )
  {
    return ids;
  }

  // Handles WFS 1.1.0
  QString insertResultTagName;
  if ( mShared->mWFSVersion == "1.1.0"_L1 || mShared->mWFSVersion.startsWith( "2."_L1 ) )
  {
    insertResultTagName = u"InsertResults"_s;
  }
  else
  {
    insertResultTagName = u"InsertResult"_s;
  }
  QDomNodeList insertResultList = rootElem.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, insertResultTagName );
  for ( int i = 0; i < insertResultList.size(); ++i )
  {
    QDomNodeList featureIdList = insertResultList.at( i ).toElement().elementsByTagNameNS( QgsWFSConstants::OGC_NAMESPACE, u"FeatureId"_s );
    for ( int j = 0; j < featureIdList.size(); ++j )
    {
      QString fidString = featureIdList.at( j ).toElement().attribute( u"fid"_s );
      if ( !fidString.isEmpty() )
      {
        ids << fidString;
      }
    }
  }
  return ids;
}

bool QgsWFSProvider::getCapabilities()
{
  mCapabilities = Qgis::VectorProviderCapability::SelectAtId | Qgis::VectorProviderCapability::ReloadData;

  if ( mShared->mCaps.version.isEmpty() )
  {
    mShared->mCaps = getCachedCapabilities( mShared->mURI.uri() );
    if ( mShared->mCaps.version.isEmpty() )
      return false;
  }
  mShared->mURI.setGetEndpoints( mShared->mCaps.operationGetEndpoints );
  mShared->mURI.setPostEndpoints( mShared->mCaps.operationPostEndpoints );

  mShared->mWFSVersion = mShared->mCaps.version;

  bool pagingEnabled = false;
  if ( mShared->mURI.pagingStatus() == QgsWFSDataSourceURI::PagingStatus::ENABLED )
  {
    pagingEnabled = true;
  }
  else if ( mShared->mWFSVersion.startsWith( "2.0"_L1 ) )
  {
    if ( mShared->mCaps.supportsPaging && mShared->mURI.pagingStatus() == QgsWFSDataSourceURI::PagingStatus::DEFAULT )
      pagingEnabled = true;
  }

  if ( mShared->mURI.maxNumFeatures() > 0 && mShared->mCaps.maxFeatures > 0 && !pagingEnabled )
  {
    mShared->mMaxFeatures = std::min( mShared->mURI.maxNumFeatures(), mShared->mCaps.maxFeatures );
  }
  else if ( mShared->mURI.maxNumFeatures() > 0 )
  {
    mShared->mMaxFeatures = mShared->mURI.maxNumFeatures();
  }
  else if ( mShared->mCaps.maxFeatures > 0 && !pagingEnabled )
  {
    mShared->mMaxFeatures = mShared->mCaps.maxFeatures;
  }
  else
  {
    mShared->mMaxFeatures = 0;
  }

  if ( pagingEnabled )
  {
    if ( mShared->mURI.pageSize() > 0 )
    {
      if ( mShared->mCaps.maxFeatures > 0 )
      {
        mShared->mPageSize = std::min( mShared->mURI.pageSize(), mShared->mCaps.maxFeatures );
      }
      else
      {
        mShared->mPageSize = mShared->mURI.pageSize();
      }
    }
    else if ( mShared->mCaps.maxFeatures > 0 )
    {
      mShared->mPageSize = mShared->mCaps.maxFeatures;
    }
    else
    {
      QgsSettings settings;
      mShared->mPageSize = settings.value( u"wfs/max_feature_count_if_not_provided"_s, "1000" ).toInt();
      QgsDebugError( u"Server declares paging but does not advertise max feature count and user did not specify it. Using %1"_s.arg( mShared->mPageSize ) );
    }
  }
  else
  {
    mShared->mPageSize = 0;
  }

  //find the <FeatureType> for this layer
  QString thisLayerName = mShared->mURI.typeName();
  const QString searchName = mShared->mCaps.addPrefixIfNeeded( thisLayerName );
  bool foundLayer = false;
  for ( int i = 0; i < mShared->mCaps.featureTypes.size(); i++ )
  {
    if ( searchName == mShared->mCaps.featureTypes[i].name )
    {
      if ( mShared->mURI.typeName() != mShared->mCaps.featureTypes[i].name )
        mShared->mURI.setTypeName( mShared->mCaps.featureTypes[i].name );

      const QgsRectangle &r = mShared->mCaps.featureTypes[i].bbox;
      if ( mShared->mSourceCrs.authid().isEmpty() && mShared->mCaps.featureTypes[i].crslist.size() != 0 )
      {
        mShared->mSourceCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mShared->mCaps.featureTypes[i].crslist[0] );
      }
      if ( !r.isNull() )
      {
        if ( mShared->mCaps.featureTypes[i].bboxSRSIsWGS84 )
        {
          QgsCoordinateReferenceSystem src = QgsCoordinateReferenceSystem::fromOgcWmsCrs( u"CRS:84"_s );
          QgsCoordinateTransform ct( src, mShared->mSourceCrs, transformContext() );
          QgsDebugMsgLevel( "latlon ext:" + r.toString(), 4 );
          QgsDebugMsgLevel( "src:" + src.authid(), 4 );
          QgsDebugMsgLevel( "dst:" + mShared->mSourceCrs.authid(), 4 );

          ct.setBallparkTransformsAreAppropriate( true );
          try
          {
            mShared->mCapabilityExtent = ct.transformBoundingBox( r, Qgis::TransformDirection::Forward );
          }
          catch ( QgsCsException &e )
          {
            QgsDebugError( u"Error transforming layer extent: %1"_s.arg( e.what() ) );
            mShared->mCapabilityExtent = r;
          }
        }
        else
        {
          mShared->mCapabilityExtent = r;
        }

        QgsDebugMsgLevel( "layer ext:" + mShared->mCapabilityExtent.toString(), 4 );
      }
      if ( mShared->mCaps.featureTypes[i].insertCap )
      {
        mCapabilities |= Qgis::VectorProviderCapability::AddFeatures;
      }
      if ( mShared->mCaps.featureTypes[i].updateCap )
      {
        mCapabilities |= Qgis::VectorProviderCapability::ChangeAttributeValues;
        mCapabilities |= Qgis::VectorProviderCapability::ChangeGeometries;
      }
      if ( mShared->mCaps.featureTypes[i].deleteCap )
      {
        mCapabilities |= Qgis::VectorProviderCapability::DeleteFeatures;
      }

      foundLayer = true;
    }
  }

  if ( !foundLayer )
  {
    QgsMessageLog::logMessage( tr( "Could not find typename %1 in capabilities for url %2" ).arg( thisLayerName, dataSourceUri() ), tr( "WFS" ) );
  }

  return foundLayer;
}

void QgsWFSProvider::handleException( const QDomDocument &serverResponse )
{
  QgsDebugMsgLevel( u"server response: %1"_s.arg( serverResponse.toString() ), 4 );

  QDomElement exceptionElem = serverResponse.documentElement();
  if ( exceptionElem.isNull() )
  {
    pushError( tr( "Empty response" ) );
    return;
  }

  if ( exceptionElem.tagName() == "ServiceExceptionReport"_L1 )
  {
    pushError( tr( "WFS service exception: %1" ).arg( exceptionElem.firstChildElement( u"ServiceException"_s ).text() ) );
    return;
  }

  if ( exceptionElem.tagName() == "WFS_TransactionResponse"_L1 )
  {
    pushError( tr( "Unsuccessful service response: %1" ).arg( exceptionElem.firstChildElement( u"TransactionResult"_s ).firstChildElement( u"Message"_s ).text() ) );
    return;
  }

  // WFS 1.1.0
  if ( exceptionElem.tagName() == "TransactionResponse"_L1 )
  {
    pushError( tr( "Unsuccessful service response: no features were added, deleted or changed." ) );
    return;
  }


  if ( exceptionElem.tagName() == "ExceptionReport"_L1 )
  {
    QDomElement exception = exceptionElem.firstChildElement( u"Exception"_s );
    // The XML schema at http://schemas.opengis.net/ows/1.1.0/owsExceptionReport.xsd uses
    // the "exceptionCode" attribute, but http://docs.opengeospatial.org/is/04-094r1/04-094r1.html#36
    // mentions "code". Accept both...
    pushError( tr( "WFS exception report (code=%1 text=%2)" )
                 .arg( exception.attribute( u"exceptionCode"_s, exception.attribute( u"code"_s, tr( "missing" ) ) ), exception.firstChildElement( u"ExceptionText"_s ).text() )
    );
    return;
  }

  pushError( tr( "Unhandled response: %1" ).arg( exceptionElem.tagName() ) );
}

QgsWfsCapabilities QgsWFSProvider::getCachedCapabilities( const QString &uri )
{
  static QMutex mutex;
  static std::map<QUrl, std::pair<QDateTime, QgsWfsCapabilities>> gCacheCaps;
  QgsWfsGetCapabilitiesRequest getCapabilities( uri );
  QUrl requestUrl = getCapabilities.requestUrl();

  QDateTime now = QDateTime::currentDateTime();
  {
    QMutexLocker lock( &mutex );
    auto iter = gCacheCaps.find( requestUrl );
    QgsSettings s;
    const int delayOfCachingInSecs = s.value( u"qgis/wfsMemoryCacheDelay"_s, 60 ).toInt();
    if ( iter != gCacheCaps.end() && iter->second.first.secsTo( now ) < delayOfCachingInSecs )
    {
      QgsDebugMsgLevel( u"Reusing cached GetCapabilities response for %1"_s.arg( requestUrl.toString() ), 4 );
      return iter->second.second;
    }
  }
  QgsWfsCapabilities caps;
  const bool synchronous = true;
  const bool forceRefresh = false;
  if ( !getCapabilities.requestCapabilities( synchronous, forceRefresh ) )
  {
    QgsMessageLog::logMessage( QObject::tr( "GetCapabilities failed for url %1: %2" ).arg( uri, getCapabilities.errorMessage() ), QObject::tr( "WFS" ) );
    return caps;
  }

  caps = getCapabilities.capabilities();

  QgsSettings s;
  if ( s.value( u"qgis/wfsMemoryCacheAllowed"_s, true ).toBool() )
  {
    QMutexLocker lock( &mutex );
    gCacheCaps[requestUrl] = std::make_pair( now, caps );
  }
  return caps;
}
