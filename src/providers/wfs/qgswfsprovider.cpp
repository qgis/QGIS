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

#include "qgis.h"
#include "qgscplhttpfetchoverrider.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsfeature.h"
#include "qgsfeedback.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsogcutils.h"
#include "qgsogrutils.h"
#include "qgssqliteutils.h"
#include "qgswfsconstants.h"
#include "qgswfsfeatureiterator.h"
#include "qgswfsprovider.h"
#include "qgswfscapabilities.h"
#include "qgswfsdescribefeaturetype.h"
#include "qgswfstransactionrequest.h"
#include "qgswfsshareddata.h"
#include "qgswfsutils.h"
#include "qgssettings.h"

#include "cpl_string.h"
#include "gdal.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QDomDocument>
#include <QMessageBox>
#include <QDomNodeList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QWidget>
#include <QPair>
#include <QTimer>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QStandardPaths>

#include <cfloat>

const QString QgsWFSProvider::WFS_PROVIDER_KEY = QStringLiteral( "WFS" );
const QString QgsWFSProvider::WFS_PROVIDER_DESCRIPTION = QStringLiteral( "WFS data provider" );

QgsWFSProvider::QgsWFSProvider( const QString &uri, const ProviderOptions &options, const QgsWfsCapabilities::Capabilities &caps )
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
        msg += QLatin1String( ", " );
      firstOne = false;
      msg += key;
    }
    QgsMessageLog::logMessage( msg, tr( "WFS" ) );
  }

  //create mSourceCrs from url if possible [WBC 111221] refactored from GetFeatureGET()
  QString srsname = mShared->mURI.SRSName();
  if ( !srsname.isEmpty() )
  {
    if ( srsname == QLatin1String( "EPSG:900913" ) )
      mShared->mSourceCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:3857" ) );
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

  if ( mShared->mWKBType == Qgis::WkbType::Unknown &&
       mShared->mURI.hasGeometryTypeFilter() &&
       mShared->mCaps.supportsGeometryTypeFilters() )
  {
    mShared->mWKBType = mShared->mURI.geometryTypeFilter();
    if ( mShared->mWKBType != Qgis::WkbType::Unknown )
    {
      mShared->computeGeometryTypeFilter();
    }
  }

  if ( !mShared->mURI.skipInitialGetFeature() )
  {
    issueInitialGetFeature();
  }
}

void QgsWFSProvider::issueInitialGetFeature()
{
  const auto GetGeometryTypeFromOneFeature = [&]( bool includeBbox )
  {
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
    connect( downloader.get(),
             qOverload < QVector<QgsFeatureUniqueIdPair> >( &QgsFeatureDownloader::featureReceived ),
             this, &QgsWFSProvider::featureReceivedAnalyzeOneFeature );
    if ( requestMadeFromMainThread )
    {
      auto processEvents = []()
      {
        QApplication::processEvents();
      };
      connect( downloader.get(), &QgsFeatureDownloader::resumeMainThread,
               this, processEvents );
    }
    downloader->run( false, /* serialize features */
                     1 /* maxfeatures */ );

    mShared->setCurrentRect( QgsRectangle() );
  };

  const auto TryToDetectGeometryType = [&]()
  {
    const Qgis::WkbType initialGeometryType = mShared->mWKBType;

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
    else
    {
      mShared->mWKBType = initialGeometryType;
    }
  };

  // For WFS = 1.0, issue a GetFeature on one feature to check
  // if we do not known the exact geometry type from
  // describeFeatureType()
  if ( mShared->mWFSVersion.startsWith( QLatin1String( "1.0" ) ) &&
       mShared->mWKBType == Qgis::WkbType::Unknown )
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
  else if ( !mShared->mWFSVersion.startsWith( QLatin1String( "1.0" ) ) &&
            ( mShared->mWKBType == Qgis::WkbType::Unknown ||
              mShared->mFields.indexOf( QLatin1String( "gmlId" ) ) < 0 ||
              mShared->mFields.indexOf( QLatin1String( "gmlName" ) ) < 0 ||
              mShared->mFields.indexOf( QLatin1String( "gmlDescription" ) ) < 0 ) )
  {
    // Try to see if gml:description, gml:identifier, gml:name attributes are
    // present. So insert them temporarily in mShared->mFields so that the
    // GML parser can detect them.
    const auto addGMLFields = [ = ]( bool forceAdd )
    {
      if ( mShared->mFields.indexOf( QLatin1String( "description" ) ) < 0  && ( forceAdd || mSampleFeatureHasDescription ) )
        mShared->mFields.append( QgsField( QStringLiteral( "description" ), QVariant::String, QStringLiteral( "xsd:string" ) ) );
      if ( mShared->mFields.indexOf( QLatin1String( "identifier" ) ) < 0  && ( forceAdd || mSampleFeatureHasIdentifier ) )
        mShared->mFields.append( QgsField( QStringLiteral( "identifier" ), QVariant::String, QStringLiteral( "xsd:string" ) ) );
      if ( mShared->mFields.indexOf( QLatin1String( "name" ) ) < 0  && ( forceAdd || mSampleFeatureHasName ) )
        mShared->mFields.append( QgsField( QStringLiteral( "name" ), QVariant::String, QStringLiteral( "xsd:string" ) ) );
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
  QgsDebugMsgLevel( QStringLiteral( "~QgsWFSProvider()" ), 4 );
}

class QgsWFSProviderSQLFunctionValidator: public QgsSQLStatement::RecursiveVisitor
{
  public:
    QgsWFSProviderSQLFunctionValidator(
      const QList<QgsWfsCapabilities::Function> &spatialPredicatesList,
      const QList<QgsWfsCapabilities::Function> &functionList );

    bool hasError() const { return mError; }

    const QString &errorMessage() const { return mErrorMessage; }

  protected:
    void visit( const QgsSQLStatement::NodeFunction &n ) override;

  private:
    const QList<QgsWfsCapabilities::Function> &mSpatialPredicatesList;
    const QList<QgsWfsCapabilities::Function> &mFunctionList;
    bool mError;
    QString mErrorMessage;
};

QgsWFSProviderSQLFunctionValidator::QgsWFSProviderSQLFunctionValidator(
  const QList<QgsWfsCapabilities::Function> &spatialPredicatesList,
  const QList<QgsWfsCapabilities::Function> &functionList )
  : mSpatialPredicatesList( spatialPredicatesList )
  , mFunctionList( functionList )
  , mError( false )
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
      if ( n.name().compare( f.name, Qt::CaseInsensitive ) == 0 ||
           QString( "ST_" + n.name() ).compare( f.name, Qt::CaseInsensitive ) == 0 )
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

class QgsWFSProviderSQLColumnRefValidator: public QgsSQLStatement::RecursiveVisitor
{
  public:
    QgsWFSProviderSQLColumnRefValidator(
      const QgsWfsCapabilities::Capabilities &caps,
      const QString &defaultTypeName,
      const QMap< QString, QString > &mapTypenameAliasToTypename,
      const QMap < QString, QgsFields > &mapTypenameToFields,
      const QMap < QString, QString > &mapTypenameToGeometryAttribute );

    bool hasError() const { return mError; }

    const QString &errorMessage() const { return mErrorMessage; }

  protected:
    void visit( const QgsSQLStatement::NodeColumnRef &n ) override;

  private:
    const QgsWfsCapabilities::Capabilities mCaps;
    QString mDefaultTypeName;
    const QMap< QString, QString > &mMapTableAliasToName;
    const QMap < QString, QgsFields > &mMapTypenameToFields;
    const QMap < QString, QString > &mMapTypenameToGeometryAttribute;
    bool mError;
    QString mErrorMessage;
};

QgsWFSProviderSQLColumnRefValidator::QgsWFSProviderSQLColumnRefValidator(
  const QgsWfsCapabilities::Capabilities &caps,
  const QString &defaultTypeName,
  const QMap< QString, QString > &mapTypenameAliasToTypename,
  const QMap < QString, QgsFields > &mapTypenameToFields,
  const QMap < QString, QString > &mapTypenameToGeometryAttribute )
  : mCaps( caps )
  , mDefaultTypeName( defaultTypeName )
  , mMapTableAliasToName( mapTypenameAliasToTypename )
  , mMapTypenameToFields( mapTypenameToFields )
  , mMapTypenameToGeometryAttribute( mapTypenameToGeometryAttribute )
  , mError( false )
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
        mErrorMessage = QObject::tr( "Column '%1' references a non existing table" ).
                        arg( n.dump() );
        return;
      }
    }

    QgsFields tableFields = mMapTypenameToFields[typeName];
    int idx = tableFields.lookupField( n.name() );
    if ( idx < 0 && mMapTypenameToGeometryAttribute[typeName] != n.name() )
    {
      mError = true;
      mErrorMessage = QObject::tr( "Column '%1' references a non existing field" ).
                      arg( n.dump() );
      return;
    }

    QgsSQLStatement::RecursiveVisitor::visit( n );
  }
}


bool QgsWFSProvider::processSQL( const QString &sqlString, QString &errorMsg, QString &warningMsg )
{
  QgsDebugMsgLevel( QStringLiteral( "Processing SQL: %1" ).arg( sqlString ), 4 );
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
      if ( part == QLatin1String( "syntax error" ) )
        newPart = tr( "Syntax error." );
      else if ( part == QLatin1String( " unexpected $end" ) )
        newPart = tr( "Missing content at end of string." );
      else if ( part.startsWith( QLatin1String( " unexpected " ) ) )
        newPart = tr( "%1 is unexpected." ).arg( part.mid( QStringLiteral( " unexpected " ).size() ) );
      else if ( part.startsWith( QLatin1String( " expecting " ) ) )
        newPart = tr( "%1 is expected instead." ).arg( part.mid( QStringLiteral( " expecting " ).size() ) );
      if ( !parserErrorString.isEmpty() )
        parserErrorString += QLatin1Char( ' ' );
      parserErrorString += newPart;
    }
    parserErrorString.replace( QLatin1String( " or " ), tr( "%1 or %2" ).arg( QString(), QString() ) );
    parserErrorString.replace( QLatin1String( "COMMA" ), tr( "comma" ) );
    parserErrorString.replace( QLatin1String( "IDENTIFIER" ), tr( "an identifier" ) );
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
    QgsDebugError( QStringLiteral( "SQL statement is not a SELECT. This should not happen" ) );
    return false;
  }
  const QgsSQLStatement::NodeSelect *select = dynamic_cast<const QgsSQLStatement::NodeSelect *>( sql.rootNode() );
  if ( !select )
  {
    // Makes Coverity happy, but cannot happen in practice
    QgsDebugError( QStringLiteral( "should not happen" ) );
    return false;
  }
  mShared->mDistinctSelect = select->distinct();

  QMap< QString, QString > mapTypenameAliasToTypename;
  QMap< QString, QString > mapTypenameToTypenameAlias; // reverse map of the previous one
  QList<QgsSQLStatement::NodeTableDef *> tables = select->tables();
  QList< QString> typenameList;
  bool severalTablesWithSameNameButDifferentPrefix = false;
  QSet< QString > unprefixTypenames;
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
      mapTypenameAliasToTypename[ prefixedTypename ] = prefixedTypename;
      mapTypenameToTypenameAlias[ prefixedTypename ] = prefixedTypename;
    }
    else
    {
      mapTypenameAliasToTypename[ table->alias()] = prefixedTypename;
      mapTypenameToTypenameAlias[ prefixedTypename ] = table->alias();
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
      mapTypenameAliasToTypename[ prefixedTypename ] = prefixedTypename;
      mapTypenameToTypenameAlias[ prefixedTypename ] = prefixedTypename;
    }
    else
    {
      mapTypenameAliasToTypename[ table->alias()] = prefixedTypename;
      mapTypenameToTypenameAlias[ prefixedTypename ] = table->alias();
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
      concatenatedTypenames += QLatin1Char( ',' );
    concatenatedTypenames += typeName;
  }

  QgsWFSDescribeFeatureType describeFeatureType( mShared->mURI );
  if ( !describeFeatureType.requestFeatureType( mShared->mWFSVersion,
       concatenatedTypenames, mShared->mCaps ) )
  {
    errorMsg = tr( "DescribeFeatureType failed for url %1: %2" ).
               arg( dataSourceUri(), describeFeatureType.errorMessage() );
    return false;
  }

  QByteArray  response = describeFeatureType.response();


  QDomDocument describeFeatureDocument;
  errorMsg.clear();
  if ( !describeFeatureDocument.setContent( response, true, &errorMsg ) )
  {
    QgsDebugMsgLevel( response, 4 );
    errorMsg = tr( "DescribeFeatureType failed for url %1: %2" ).
               arg( dataSourceUri(), errorMsg );
    return false;
  }

  QMap < QString, QgsFields > mapTypenameToFields;
  QMap < QString, QString > mapTypenameToGeometryAttribute;
  for ( const QString &typeName : std::as_const( typenameList ) )
  {
    QString geometryAttribute;
    QgsFields fields;
    Qgis::WkbType geomType;
    bool geometryMaybeMissing;
    if ( !readAttributesFromSchema( describeFeatureDocument,
                                    response,
                                    /* singleLayerContext = */ typenameList.size() == 1,
                                    typeName,
                                    geometryAttribute,
                                    fields,
                                    geomType,
                                    geometryMaybeMissing,
                                    errorMsg ) )
    {
      errorMsg = tr( "Analysis of DescribeFeatureType response failed for url %1, typeName %2: %3" ).
                 arg( dataSourceUri(), typeName, errorMsg );
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
    mapTypenameToGeometryAttribute );
  sql.acceptVisitor( oColumnValidator );
  if ( oColumnValidator.hasError() )
  {
    errorMsg = oColumnValidator.errorMessage();
    return false;
  }

  if ( mShared->mURI.validateSqlFunctions() )
  {
    QgsWFSProviderSQLFunctionValidator oValidator( mShared->mCaps.spatialPredicatesList,
        mShared->mCaps.functionList );
    sql.acceptVisitor( oValidator );
    if ( oValidator.hasError() )
    {
      errorMsg = oValidator.errorMessage();
      return false;
    }
  }

  QList<QgsSQLStatement::NodeSelectedColumn *> columns = select->columns();
  QMap< QString, QPair<QString, QString> > mapFieldNameToSrcLayerNameFieldName;
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

          mapFieldNameToSrcLayerNameFieldName[ field.name()] =
            QPair<QString, QString>( columnTableTypename, srcField.name() );
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
            mapFieldNameToSrcLayerNameFieldName[ field.name()] =
              QPair<QString, QString>( typeName, srcField.name() );
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
        QgsDebugError( QStringLiteral( "Should not happen. Cannot find field for %1" ).arg( columnRef->name() ) );
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
      mapFieldNameToSrcLayerNameFieldName[ field.name()] =
        QPair<QString, QString>( columnTableTypename, orig.name() );
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
    if ( !readAttributesFromSchema( describeFeatureDocument,
                                    response,
                                    /* singleLayerContext = */ typenameList.size() == 1,
                                    typeName,
                                    geometryAttribute, fields, geomType, geometryMaybeMissing, errorMsg ) )
    {
      errorMsg = tr( "Analysis of DescribeFeatureType response failed for url %1, typeName %2: %3" ).
                 arg( dataSourceUri(), typeName, errorMsg );
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

void QgsWFSProvider::featureReceivedAnalyzeOneFeature( QVector<QgsFeatureUniqueIdPair> list )
{
  if ( list.size() != 0 && mShared->mWKBType == Qgis::WkbType::Unknown )
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
            QgsDebugMsgLevel( QStringLiteral( "Layer of unknown type. First element is a GeometryCollection of Point. Advertizing optimistically as MultiPoint" ), 2 );
            mShared->mWKBType = Qgis::WkbType::MultiPoint;
          }
          else if ( mShared->mWKBType == Qgis::WkbType::LineString )
          {
            QgsDebugMsgLevel( QStringLiteral( "Layer of unknown type. First element is a GeometryCollection of LineString. Advertizing optimistically as MultiLineString" ), 2 );
            mShared->mWKBType = Qgis::WkbType::MultiLineString;
          }
          else if ( mShared->mWKBType == Qgis::WkbType::Polygon )
          {
            QgsDebugMsgLevel( QStringLiteral( "Layer of unknown type. First element is a GeometryCollection of Polygon. Advertizing optimistically as MultiPolygon" ), 2 );
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
}

QString QgsWFSProvider::subsetString() const
{
  return mSubsetString;
}

bool QgsWFSProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
{
  Q_UNUSED( updateFeatureCount )

  QgsDebugMsgLevel( QStringLiteral( "theSql = '%1'" ).arg( theSQL ), 4 );

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
  if ( theSQL.startsWith( QLatin1String( "SELECT " ), Qt::CaseInsensitive ) ||
       theSQL.startsWith( QLatin1String( "SELECT\t" ), Qt::CaseInsensitive ) ||
       theSQL.startsWith( QLatin1String( "SELECT\r" ), Qt::CaseInsensitive ) ||
       theSQL.startsWith( QLatin1String( "SELECT\n" ), Qt::CaseInsensitive ) )
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


QgsAbstractFeatureSource *QgsWFSProvider::featureSource() const
{
  auto fs = new QgsBackgroundCachedFeatureSource( mShared );
  /*connect( fs, SIGNAL( extentRequested( const QgsRectangle & ) ),
           this, SLOT( extendExtent( const QgsRectangle & ) ) );*/
  return fs;
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

  if ( mShared->mWFSVersion.startsWith( QLatin1String( "1.1" ) ) )
  {
    // WFS 1.1.0 uses preferably GML 3, but ESRI mapserver in 2020 doesn't like it so we stick to GML2
    if ( ! mShared->mServerPrefersCoordinatesForTransactions_1_1 )
    {
      gmlVersion = QgsOgcUtils::GML_3_1_0;
    }
    else
    {
      gmlVersion = QgsOgcUtils::GML_2_1_2;
    }
    // For servers like Geomedia and QGIS Server that advertise EPSG:XXXX in capabilities even in WFS 1.1 or 2.0
    // cpabilities useEPSGColumnFormat is set.
    // We follow GeoServer convention here which is to treat EPSG:4326 as lon/lat
    applyAxisInversion = ( crs().hasAxisInverted() && ! mShared->mURI.ignoreAxisOrientation() && ! mShared->mCaps.useEPSGColumnFormat )
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

QString QgsWFSProvider::geometryAttribute() const
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
    QDomElement insertElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Insert" ) );
    transactionElem.appendChild( insertElem );

    QDomElement featureElem = transactionDoc.createElementNS( mApplicationNamespace, tname );

    QgsAttributes featureAttributes = featureIt->attributes();
    int nAttrs = featureAttributes.size();
    for ( int i = 0; i < nAttrs; ++i )
    {
      const QVariant &value = featureAttributes.at( i );
      if ( value.isValid() && !QgsVariantUtils::isNull( value ) )
      {
        QDomElement fieldElem = transactionDoc.createElementNS( mApplicationNamespace, mShared->mFields.at( i ).name() );
        QDomText fieldText = transactionDoc.createTextNode( convertToXML( value ) );
        fieldElem.appendChild( fieldText );
        featureElem.appendChild( fieldElem );
      }
    }

    //add geometry column (as gml)
    QgsGeometry geometry = featureIt->geometry();
    if ( !geometry.isNull() )
    {
      QDomElement geomElem = transactionDoc.createElementNS( mApplicationNamespace, mShared->mGeometryAttribute );
      QgsGeometry the_geom( geometry );
      // convert to multi if the layer geom type is multi and the geom is not
      if ( QgsWkbTypes::isMultiType( this->wkbType() ) && ! the_geom.isMultipart() )
      {
        the_geom.convertToMultiType();
      }

      const QDomElement gmlElem { geometryElement( the_geom, transactionDoc ) };
      if ( ! gmlElem.isNull() )
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
      if ( v.startsWith( QLatin1String( "new" ) ) )
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
      QMap< QString, QgsFeatureId > map;
      for ( int idx = 0; idx < serializedFeatureList.size(); idx++ )
        map[ serializedFeatureList[idx].second ] = serializedFeatureList[idx].first.id();

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
  QDomElement deleteElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Delete" ) );
  deleteElem.setAttribute( QStringLiteral( "typeName" ), tname );
  QDomElement filterElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, QStringLiteral( "Filter" ) );


  QgsFeatureIds::const_iterator idIt = id.constBegin();
  for ( ; idIt != id.constEnd(); ++idIt )
  {
    //find out feature id
    QString gmlid = mShared->findUniqueId( *idIt );
    if ( gmlid.isEmpty() )
    {
      QgsDebugError( QStringLiteral( "Cannot identify feature of id %1" ).arg( *idIt ) );
      continue;
    }
    QDomElement featureIdElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, QStringLiteral( "FeatureId" ) );
    featureIdElem.setAttribute( QStringLiteral( "fid" ), gmlid );
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
      QgsDebugError( QStringLiteral( "Cannot identify feature of id %1" ).arg( geomIt.key() ) );
      continue;
    }
    QDomElement updateElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Update" ) );
    updateElem.setAttribute( QStringLiteral( "typeName" ), tname );
    //Property
    QDomElement propertyElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Property" ) );
    QDomElement nameElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Name" ) );
    QDomText nameText = transactionDoc.createTextNode( namespacePrefix + mShared->mGeometryAttribute );
    nameElem.appendChild( nameText );
    propertyElem.appendChild( nameElem );
    QDomElement valueElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Value" ) );

    valueElem.appendChild( geometryElement( geomIt.value(), transactionDoc ) );

    propertyElem.appendChild( valueElem );
    updateElem.appendChild( propertyElem );

    //filter
    QDomElement filterElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, QStringLiteral( "Filter" ) );
    QDomElement featureIdElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, QStringLiteral( "FeatureId" ) );
    featureIdElem.setAttribute( QStringLiteral( "fid" ), gmlid );
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
  if ( value.type() == QVariant::DateTime )
  {
    QDateTime dt = value.toDateTime().toUTC();
    if ( !dt.isNull() )
    {
      valueStr = dt.toString( QStringLiteral( "yyyy-MM-ddThh:mm:ss.zzzZ" ) );
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
      QgsDebugError( QStringLiteral( "Cannot identify feature of id %1" ).arg( attIt.key() ) );
      continue;
    }

    QDomElement updateElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Update" ) );
    updateElem.setAttribute( QStringLiteral( "typeName" ), tname );

    QgsAttributeMap::const_iterator attMapIt = attIt.value().constBegin();
    for ( ; attMapIt != attIt.value().constEnd(); ++attMapIt )
    {
      QString fieldName = mShared->mFields.at( attMapIt.key() ).name();
      QDomElement propertyElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Property" ) );

      QDomElement nameElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Name" ) );
      QDomText nameText = transactionDoc.createTextNode( namespacePrefix + fieldName );
      nameElem.appendChild( nameText );
      propertyElem.appendChild( nameElem );

      QDomElement valueElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Value" ) );

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
    QDomElement filterElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, QStringLiteral( "Filter" ) );
    QDomElement featureIdElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, QStringLiteral( "FeatureId" ) );
    featureIdElem.setAttribute( QStringLiteral( "fid" ), gmlid );
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
  result[QStringLiteral( "MaxFeatures" )] = mShared->mCaps.maxFeatures;
  result[QStringLiteral( "SupportsPaging" )] = mShared->mCaps.supportsPaging;
  result[QStringLiteral( "SupportsJoins" )] = mShared->mCaps.supportsJoins;
  return result;
}

QString QgsWFSProvider::translateMetadataKey( const QString &mdKey ) const
{
  if ( mdKey == QLatin1String( "MaxFeatures" ) )
  {
    return tr( "Max Features" );
  }
  else if ( mdKey == QLatin1String( "SupportsPaging" ) )
  {
    return tr( "Supports Paging" );
  }
  else if ( mdKey == QLatin1String( "SupportsJoins" ) )
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
  if ( mdKey == QLatin1String( "MaxFeatures" ) )
  {
    return value.toInt() == 0 ? tr( "not provided" ) : value.toString();
  }
  else if ( mdKey == QLatin1String( "SupportsPaging" ) || mdKey == QLatin1String( "SupportsJoins" ) )
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

  if ( !describeFeatureType.requestFeatureType( mShared->mWFSVersion,
       mShared->mURI.typeName(), mShared->mCaps ) )
  {
    QgsMessageLog::logMessage( tr( "DescribeFeatureType network request failed for url %1: %2" ).
                               arg( dataSourceUri(), describeFeatureType.errorMessage() ), tr( "WFS" ) );
    return false;
  }

  QByteArray response = describeFeatureType.response();

  QgsDebugMsgLevel( response, 4 );

  QDomDocument describeFeatureDocument;
  QString errorMsg;
  if ( !describeFeatureDocument.setContent( response, true, &errorMsg ) )
  {
    QgsDebugMsgLevel( response, 4 );
    QgsMessageLog::logMessage( tr( "DescribeFeatureType XML parse failed for url %1: %2" ).
                               arg( dataSourceUri(), errorMsg ), tr( "WFS" ) );
    return false;
  }

  if ( !readAttributesFromSchema( describeFeatureDocument,
                                  response,
                                  /* singleLayerContext = */ true,
                                  mShared->mURI.typeName(),
                                  geometryAttribute, fields, geomType, geometryMaybeMissing, errorMsg ) )
  {
    QgsDebugMsgLevel( response, 4 );
    QgsMessageLog::logMessage( tr( "Analysis of DescribeFeatureType response failed for url %1: %2" ).
                               arg( dataSourceUri(), errorMsg ), tr( "WFS" ) );
    pushError( errorMsg );
    return false;
  }

  setLayerPropertiesListFromDescribeFeature( describeFeatureDocument, response, {mShared->mURI.typeName()}, errorMsg );

  return true;
}


bool QgsWFSProvider::readAttributesFromSchema( QDomDocument &schemaDoc,
    const QByteArray &response,
    bool singleLayerContext,
    const QString &prefixedTypename,
    QString &geometryAttribute,
    QgsFields &fields,
    Qgis::WkbType &geomType,
    bool &geometryMaybeMissing,
    QString &errorMsg )
{
  geometryMaybeMissing = false;
  bool mayTryWithGMLAS = false;
  bool ret = readAttributesFromSchemaWithoutGMLAS( schemaDoc, prefixedTypename, geometryAttribute, fields, geomType, errorMsg, mayTryWithGMLAS );
  if ( singleLayerContext &&
       mayTryWithGMLAS &&
       GDALGetDriverByName( "GMLAS" ) )
  {
    QString geometryAttributeGMLAS;
    QgsFields fieldsGMLAS;
    Qgis::WkbType geomTypeGMLAS;
    QString errorMsgGMLAS;
    if ( readAttributesFromSchemaWithGMLAS( response, prefixedTypename, geometryAttributeGMLAS, fieldsGMLAS, geomTypeGMLAS, geometryMaybeMissing, errorMsgGMLAS ) )
    {
      geometryAttribute = geometryAttributeGMLAS;
      fields = fieldsGMLAS;
      geomType = geomTypeGMLAS;
      ret = true;
    }
    else if ( !ret )
    {
      errorMsg = errorMsgGMLAS;
    }
    else
    {
      pushError( errorMsgGMLAS );
    }
  }
  return ret;
}

static QVariant::Type getVariantTypeFromXML( const QString &xmlType )
{
  QVariant::Type attributeType = QVariant::Invalid;

  const QString type = QString( xmlType )
                       .replace( QLatin1String( "xs:" ), QString() )
                       .replace( QLatin1String( "xsd:" ), QString() );

  if ( type.compare( QLatin1String( "string" ), Qt::CaseInsensitive ) == 0 ||
       type.compare( QLatin1String( "token" ), Qt::CaseInsensitive ) == 0 ||
       type.compare( QLatin1String( "NMTOKEN" ), Qt::CaseInsensitive ) == 0 ||
       type.compare( QLatin1String( "NCName" ), Qt::CaseInsensitive ) == 0 ||
       type.compare( QLatin1String( "QName" ), Qt::CaseInsensitive ) == 0 ||
       type.compare( QLatin1String( "ID" ), Qt::CaseInsensitive ) == 0 ||
       type.compare( QLatin1String( "IDREF" ), Qt::CaseInsensitive ) == 0 ||
       type.compare( QLatin1String( "anyURI" ), Qt::CaseInsensitive ) == 0 ||
       type.compare( QLatin1String( "anySimpleType" ), Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QVariant::String;
  }
  else if ( type.compare( QLatin1String( "boolean" ), Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QVariant::Bool;
  }
  else if ( type.compare( QLatin1String( "double" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "float" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "decimal" ), Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QVariant::Double;
  }
  else if ( type.compare( QLatin1String( "byte" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "unsignedByte" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "int" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "short" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "unsignedShort" ), Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QVariant::Int;
  }
  else if ( type.compare( QLatin1String( "long" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "unsignedLong" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "integer" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "negativeInteger" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "nonNegativeInteger" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "positiveInteger" ), Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QVariant::LongLong;
  }
  else if ( type.compare( QLatin1String( "date" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "gYear" ), Qt::CaseInsensitive ) == 0 ||
            type.compare( QLatin1String( "gYearMonth" ), Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QVariant::Date;
  }
  else if ( type.compare( QLatin1String( "time" ), Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QVariant::Time;
  }
  else if ( type.compare( QLatin1String( "dateTime" ), Qt::CaseInsensitive ) == 0 )
  {
    attributeType = QVariant::DateTime;
  }
  return attributeType;
}

static void CPL_STDCALL QgsWFSProviderGMLASErrorHandler( CPLErr eErr, CPLErrorNum /*eErrorNum*/, const char *pszErrorMsg )
{
  // Silence harmless warnings like "GeographicalName_pronunciation_PronunciationOfName_pronunciationSoundLink_nilReason identifier truncated to geographicalname_pronunciation_pronunciationofname_pronunciatio"
  if ( !( eErr == CE_Warning && strstr( pszErrorMsg, " truncated to " ) ) )
  {
    if ( eErr == CE_Failure )
    {
      void *pUserData = CPLGetErrorHandlerUserData();
      QString *pString = static_cast<QString *>( pUserData );
      if ( pString->isEmpty() )
        *pString = QObject::tr( "Error while analyzing schema: %1" ).arg( pszErrorMsg );
      QgsMessageLog::logMessage( QObject::tr( "GMLAS error: %1" ).arg( pszErrorMsg ), QObject::tr( "WFS" ) );
    }
    else if ( eErr == CE_Debug )
    {
      QgsDebugMsgLevel( QStringLiteral( "GMLAS debug msg: %1" ).arg( pszErrorMsg ), 5 );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "GMLAS eErr=%1, msg=%2" ).arg( eErr ).arg( pszErrorMsg ), 2 );
    }
  }
}

bool QgsWFSProvider::readAttributesFromSchemaWithGMLAS( const QByteArray &response,
    const QString &prefixedTypename,
    QString &geometryAttribute,
    QgsFields &fields,
    Qgis::WkbType &geomType,
    bool &geometryMaybeMissing,
    QString &errorMsg )
{
  geomType = Qgis::WkbType::NoGeometry;
  geometryMaybeMissing = false;

  QUrl url( mShared->mURI.requestUrl( QStringLiteral( "DescribeFeatureType" ) ) );
  QUrlQuery query( url );
  query.addQueryItem( QStringLiteral( "TYPENAME" ), prefixedTypename );
  url.setQuery( query );

  // If a previous attempt with the same URL failed because of cancellation
  // in the past second, do not retry.
  // The main use case for that is when QgsWfsProviderMetadata::querySublayers()
  // is called when adding a layer, and several QgsWFSProvider instances are
  // quickly created.
  static QMutex mutex;
  static QUrl lastCanceledURL;
  static QDateTime lastCanceledDateTime;
  {
    QMutexLocker lock( &mutex );
    if ( lastCanceledURL == url && lastCanceledDateTime + 1 > QDateTime::currentDateTime() )
    {
      mMetadataRetrievalCanceled = true;
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
  VSILFILE *fp = VSIFileFromMemBuffer( pszSchemaTempFilename,
                                       reinterpret_cast<GByte *>( pszSchema ), strlen( pszSchema ), /* bTakeOwnership=*/ true );
  if ( fp )
    VSIFCloseL( fp );

  QgsFeedback feedback;
  GDALDatasetH hDS = nullptr;

  // Analyze the DescribeFeatureType response schema with the OGR GMLAS driver
  // in a thread, so it can get interrupted (with GDAL 3.9: https://github.com/OSGeo/gdal/pull/9019)

  const auto downloaderLambda = [pszSchemaTempFilename, &feedback, &hDS, &errorMsg]()
  {
    QgsCPLHTTPFetchOverrider cplHTTPFetchOverrider( QString(), &feedback );
    QgsSetCPLHTTPFetchOverriderInitiatorClass( cplHTTPFetchOverrider, QStringLiteral( "WFSProviderDownloadSchema" ) )

    char **papszOpenOptions = nullptr;
    papszOpenOptions = CSLSetNameValue( papszOpenOptions, "XSD", pszSchemaTempFilename );

    QgsSettings settings;
    QString cacheDirectory = settings.value( QStringLiteral( "cache/directory" ) ).toString();
    if ( cacheDirectory.isEmpty() )
      cacheDirectory = QStandardPaths::writableLocation( QStandardPaths::CacheLocation );
    if ( !cacheDirectory.endsWith( QDir::separator() ) )
    {
      cacheDirectory.push_back( QDir::separator() );
    }
    // Must be kept in sync with QgsOptions::clearCache()
    cacheDirectory += QLatin1String( "gmlas_xsd_cache" );
    QgsDebugMsgLevel( QStringLiteral( "cacheDirectory = %1" ).arg( cacheDirectory ), 4 );
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
                                     "</Configuration>" ).arg( pszEscaped );
    CPLFree( pszEscaped );
    papszOpenOptions = CSLSetNameValue( papszOpenOptions, "CONFIG_FILE", config.toStdString().c_str() );

    CPLPushErrorHandlerEx( QgsWFSProviderGMLASErrorHandler, &errorMsg );
    hDS = GDALOpenEx( "GMLAS:", GDAL_OF_VECTOR, nullptr, papszOpenOptions, nullptr );
    CPLPopErrorHandler();
    CSLDestroy( papszOpenOptions );
  };

  std::unique_ptr<_DownloaderThread> downloaderThread =
    std::make_unique<_DownloaderThread>( downloaderLambda );
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
        if ( widget->objectName() == QLatin1String( "QgisApp" ) )
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
      QMessageBox::Information, tr( "Information" ), tr( "Download of schemas in progress..." ),
      QMessageBox::Cancel,
      parentWidget );
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,9,0)
    connect( box, &QDialog::rejected, &feedback, &QgsFeedback::cancel );
#else
    box->button( QMessageBox::Cancel )->setEnabled( false );
#endif

    QgsSettings s;
    const double settingDefaultValue = 2.0;
    const QString settingName = QStringLiteral( "qgis/wfsDownloadSchemasPopupTimeout" );
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
      connect( &timerForHits, &QTimer::timeout, box, &QDialog::exec );
    }

    // Close dialog when download theread finishes.
    // Will actually trigger the QDialog::rejected signal...
    connect( downloaderThread.get(), &QThread::finished, box, &QDialog::accept );
  }

  // Run an event loop until download thread finishes
  QEventLoop loop;
  connect( downloaderThread.get(), &QThread::finished, &loop, &QEventLoop::quit );
  loop.exec( QEventLoop::ExcludeUserInputEvents );
  downloaderThread->wait();

  VSIUnlink( pszSchemaTempFilename );
  VSIFree( pszSchemaTempFilename );

  if ( !errorMsg.isEmpty() )
    return false;

  bool ret = hDS != nullptr;
  if ( feedback.isCanceled() && !ret )
  {
    QMutexLocker lock( &mutex );
    mMetadataRetrievalCanceled = true;
    lastCanceledURL = url;
    lastCanceledDateTime = QDateTime::currentDateTime();
    errorMsg = tr( "Schema analysis interrupted by user." );
    return false;
  }
  if ( !ret )
  {
    if ( errorMsg.isEmpty() )
      errorMsg = tr( "Cannot analyze schema indicated in DescribeFeatureType response." );
    return false;
  }

  gdal::dataset_unique_ptr oDSCloser( hDS );

  // Retrieve namespace prefix and URIs
  OGRLayerH hOtherMetadataLayer = GDALDatasetGetLayerByName( hDS, "_ogr_other_metadata" );
  if ( !hOtherMetadataLayer )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find _ogr_other_metadata layer" ), 4 );
    return false;
  }

  auto hOtherMetadataLayerDefn = OGR_L_GetLayerDefn( hOtherMetadataLayer );

  const int keyIdx = OGR_FD_GetFieldIndex( hOtherMetadataLayerDefn, "key" );
  if ( keyIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find key field in _ogr_other_metadata" ), 4 );
    return false;
  }

  const int valueIdx = OGR_FD_GetFieldIndex( hOtherMetadataLayerDefn, "value" );
  if ( valueIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find value field in _ogr_other_metadata" ), 4 );
    return false;
  }

  std::map<int, QPair<QString, QString>> mapPrefixIdxToPrefixAndUri;
  while ( true )
  {
    gdal::ogr_feature_unique_ptr hFeatureOtherMD(
      OGR_L_GetNextFeature( hOtherMetadataLayer ) );
    if ( !hFeatureOtherMD )
      break;

    const QString key = QString::fromUtf8(
                          OGR_F_GetFieldAsString( hFeatureOtherMD.get(), keyIdx ) );
    const QString value = QString::fromUtf8(
                            OGR_F_GetFieldAsString( hFeatureOtherMD.get(), valueIdx ) );

    if ( key.startsWith( QLatin1String( "namespace_prefix_" ) ) )
    {
      mapPrefixIdxToPrefixAndUri[key.mid( int( strlen( "namespace_prefix_" ) ) ).toInt()].first = value;
    }
    else if ( key.startsWith( QLatin1String( "namespace_uri_" ) ) )
    {
      mapPrefixIdxToPrefixAndUri[key.mid( int( strlen( "namespace_uri_" ) ) ).toInt()].second = value;
    }
  }
  for ( const auto &kv : mapPrefixIdxToPrefixAndUri )
  {
    if ( !kv.second.first.isEmpty() && !kv.second.second.isEmpty() )
    {
      mShared->mNamespacePrefixToURIMap[kv.second.first] = kv.second.second;
      QgsDebugMsgLevel( QStringLiteral( "%1 -> %2" ).arg( kv.second.first ).arg( kv.second.second ), 4 );
    }
  }

  // Find the layer of interest
  OGRLayerH hLayersMetadata = GDALDatasetGetLayerByName( hDS, "_ogr_layers_metadata" );
  if ( !hLayersMetadata )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find _ogr_layers_metadata layer" ), 4 );
    return false;
  }
  OGR_L_SetAttributeFilter( hLayersMetadata,
                            ( "layer_xpath = " + QgsSqliteUtils::quotedString( prefixedTypename ).toStdString() ).c_str() );
  gdal::ogr_feature_unique_ptr hFeatureLayersMD( OGR_L_GetNextFeature( hLayersMetadata ) );
  if ( !hFeatureLayersMD )
  {
    QgsDebugMsgLevel(
      QStringLiteral( "Cannot find feature with layer_xpath = %1 in _ogr_layers_metadata" ).arg( prefixedTypename ), 4 );
    return false;
  }
  const int fldIdx = OGR_F_GetFieldIndex( hFeatureLayersMD.get(), "layer_name" );
  if ( fldIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find layer_name field in _ogr_layers_metadata" ), 4 );
    return false;
  }
  const QString layerName = QString::fromUtf8(
                              OGR_F_GetFieldAsString( hFeatureLayersMD.get(), fldIdx ) );

  OGRLayerH hLayer = GDALDatasetGetLayerByName(
                       hDS, layerName.toStdString().c_str() );
  if ( !hLayer )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find %& layer" ).arg( layerName ), 4 );
    return false;
  }

  // Get field information
  OGRLayerH hFieldsMetadata = GDALDatasetGetLayerByName( hDS, "_ogr_fields_metadata" );
  if ( !hFieldsMetadata )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find _ogr_fields_metadata layer" ), 4 );
    return false;
  }
  OGR_L_SetAttributeFilter( hFieldsMetadata,
                            ( "layer_name = " + QgsSqliteUtils::quotedString( layerName ).toStdString() ).c_str() );

  auto hFieldsMetadataDefn = OGR_L_GetLayerDefn( hFieldsMetadata );

  const int fieldNameIdx = OGR_FD_GetFieldIndex( hFieldsMetadataDefn, "field_name" );
  if ( fieldNameIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find field_name field in _ogr_fields_metadata" ), 4 );
    return false;
  }

  const int fieldXPathIdx = OGR_FD_GetFieldIndex( hFieldsMetadataDefn, "field_xpath" );
  if ( fieldXPathIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find field_xpath field in _ogr_fields_metadata" ), 4 );
    return false;
  }

  const int fieldIsListIdx = OGR_FD_GetFieldIndex( hFieldsMetadataDefn, "field_is_list" );
  if ( fieldIsListIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find field_is_list field in _ogr_fields_metadata" ), 4 );
    return false;
  }

  const int fieldMinOccursIdx = OGR_FD_GetFieldIndex( hFieldsMetadataDefn, "field_min_occurs" );
  if ( fieldMinOccursIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find field_min_occurs field in _ogr_fields_metadata" ), 4 );
    return false;
  }

  const int fieldTypeIdx = OGR_FD_GetFieldIndex( hFieldsMetadataDefn, "field_type" );
  if ( fieldTypeIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find field_type field in _ogr_fields_metadata" ), 4 );
    return false;
  }

  const int fieldCategoryIdx = OGR_FD_GetFieldIndex( hFieldsMetadataDefn, "field_category" );
  if ( fieldCategoryIdx < 0 )
  {
    // should not happen
    QgsDebugMsgLevel( QStringLiteral( "Cannot find field_category field in _ogr_fields_metadata" ), 4 );
    return false;
  }

  mShared->mFieldNameToXPathAndIsNestedContentMap.clear();
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
    if ( EQUAL( fieldCategory, "PATH_TO_CHILD_ELEMENT_WITH_LINK" ) &&
         fieldName.endsWith( QLatin1String( "_pkid" ) ) )
    {
      fieldName.resize( fieldName.size() - int( strlen( "_pkid" ) ) );
    }

    QgsDebugMsgLevel(
      QStringLiteral( "field %1: xpath=%2 is_list=%3 type=%4 category=%5" ).
      arg( fieldName ).arg( fieldXPath ).arg( fieldIsList ).arg( fieldType ).arg( fieldCategory ), 5 );
    if ( EQUAL( fieldCategory, "REGULAR" ) && ( EQUAL( fieldType, "geometry" ) || fieldName.endsWith( QLatin1String( "_abstractgeometricprimitive" ) ) ) )
    {
      if ( geometryAttribute.isEmpty() )
      {
        geomType = QgsWkbTypes::multiType( QgsOgrUtils::ogrGeometryTypeToQgsWkbType(
                                             OGR_L_GetGeomType( hLayer ) ) );

        QString qFieldXPath = QString::fromUtf8( fieldXPath );
        if ( fieldName.endsWith( QLatin1String( "_abstractgeometricprimitive" ) ) && strstr( fieldXPath, "/gml:Point" ) )
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

          const auto pos_gmlPoint = qFieldXPath.indexOf( QLatin1String( "/gml:Point," ) );
          qFieldXPath.resize( pos_gmlPoint );
          geomType = Qgis::WkbType::Unknown;
        }

        mShared->mFieldNameToXPathAndIsNestedContentMap[fieldName] =
          QPair<QString, bool>( qFieldXPath, false );
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

        QgsDebugMsgLevel( QStringLiteral( "geometry field: %1, xpath: %2" ).arg( geometryAttribute ).arg( qFieldXPath ), 4 );
        geometryMaybeMissing = OGR_F_GetFieldAsInteger( hFeatureFieldsMD.get(), fieldMinOccursIdx ) == 0;
      }
    }
    else if ( EQUAL( fieldCategory, "REGULAR" ) && !fieldIsList )
    {
      QVariant::Type type = getVariantTypeFromXML( QString::fromUtf8( fieldType ) );
      if ( type != QVariant::Invalid )
      {
        fields.append( QgsField( fieldName, type, fieldType ) );
      }
      else
      {
        // unhandled:duration, base64Binary, hexBinary, anyType
        QgsDebugMsgLevel(
          QStringLiteral( "unhandled type for field %1: xpath=%2 is_list=%3 type=%4 category=%5" ).
          arg( fieldName ).arg( fieldXPath ).arg( fieldIsList ).arg( fieldType ).arg( fieldCategory ), 3 );
        fields.append( QgsField( fieldName, QVariant::String, fieldType ) );
      }
      mShared->mFieldNameToXPathAndIsNestedContentMap[fieldName] =
        QPair<QString, bool>( fieldXPath, false );
    }
    else
    {
      QgsField field( fieldName, QVariant::String );
      field.setEditorWidgetSetup( QgsEditorWidgetSetup( QStringLiteral( "JsonEdit" ), QVariantMap() ) );
      fields.append( field );
      mShared->mFieldNameToXPathAndIsNestedContentMap[fieldName] =
        QPair<QString, bool>( fieldXPath, true );
    }
  }

  return true;
}

bool QgsWFSProvider::readAttributesFromSchemaWithoutGMLAS( QDomDocument &schemaDoc,
    const QString &prefixedTypename,
    QString &geometryAttribute,
    QgsFields &fields,
    Qgis::WkbType &geomType,
    QString &errorMsg, bool &mayTryWithGMLAS )
{
  mayTryWithGMLAS = false;

  //get the <schema> root element
  QDomNodeList schemaNodeList = schemaDoc.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, QStringLiteral( "schema" ) );
  if ( schemaNodeList.length() < 1 )
  {
    errorMsg = tr( "Cannot find schema root element" );
    return false;
  }
  QDomElement schemaElement = schemaNodeList.at( 0 ).toElement();
  mApplicationNamespace = schemaElement.attribute( QStringLiteral( "targetNamespace" ) );

  // Remove the namespace on the typename
  QString unprefixedTypename = prefixedTypename;
  if ( unprefixedTypename.contains( ':' ) )
  {
    unprefixedTypename = unprefixedTypename.section( ':', 1 );
  }

  // Find the element whose name is the typename that interests us, and
  // collect the correspond type.
  QDomElement elementElement = schemaElement.firstChildElement( QStringLiteral( "element" ) );
  QString elementTypeString;
  QDomElement complexTypeElement;
  while ( !elementElement.isNull() )
  {
    QString name = elementElement.attribute( QStringLiteral( "name" ) );
    if ( name == unprefixedTypename )
    {
      elementTypeString = elementElement.attribute( QStringLiteral( "type" ) );
      if ( elementTypeString.isEmpty() )
      {
        // e.g http://afnemers.ruimtelijkeplannen.nl/afnemers2012/services?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=app:Bouwvlak
        complexTypeElement = elementElement.firstChildElement( QStringLiteral( "complexType" ) );
      }
      break;
    }
    elementElement = elementElement.nextSiblingElement( QStringLiteral( "element" ) );
  }
  // Try to get a complex type whose name contains the unprefixed typename
  if ( elementTypeString.isEmpty() && complexTypeElement.isNull() )
  {
    const QDomNodeList complexElements = schemaElement.elementsByTagName( QStringLiteral( "complexType" ) );
    for ( int i = 0; i < complexElements.size(); i++ )
    {
      if ( complexElements.at( i ).toElement().attribute( QStringLiteral( "name" ) ).contains( unprefixedTypename ) )
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
      if ( iter.tagName() == QLatin1String( "import" ) )
        foundImport = true;
      else if ( iter.tagName() == QLatin1String( "include" ) )
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
      errorMsg = tr( "It is probably a schema for Complex Features." );
      mayTryWithGMLAS = true;
    }
    // e.g http://services.cuzk.cz/wfs/inspire-CP-wfs.asp?SERVICE=WFS&VERSION=2.0.0&REQUEST=DescribeFeatureType
    // which has a single  <include schemaLocation="http://inspire.ec.europa.eu/schemas/cp/4.0/CadastralParcels.xsd"/>
    // In that case, follow the link.
    else if ( !foundImport && countInclude == 1 )
    {
      QString schemaLocation =
        includeElement.attribute( QStringLiteral( "schemaLocation" ) );
      QgsDebugMsgLevel( QStringLiteral( "DescribeFeatureType response redirects to: %1" ).arg( schemaLocation ), 4 );

      QgsWFSDescribeFeatureType describeFeatureType( mShared->mURI );
      if ( !describeFeatureType.sendGET( schemaLocation, QString(), true, false ) )
      {
        errorMsg = tr( "Cannot find schema indicated in DescribeFeatureType response." );
        QgsMessageLog::logMessage( tr( "DescribeFeatureType network request failed for url %1: %2" ).
                                   arg( schemaLocation, describeFeatureType.errorMessage() ), tr( "WFS" ) );
        return false;
      }

      QByteArray response = describeFeatureType.response();
      QDomDocument describeFeatureDocument;
      if ( !describeFeatureDocument.setContent( response, true, &errorMsg ) )
      {
        QgsDebugMsgLevel( response, 4 );
        errorMsg = tr( "DescribeFeatureType XML parse failed for url %1: %2" ).
                   arg( schemaLocation, errorMsg );
      }

      return readAttributesFromSchemaWithoutGMLAS( describeFeatureDocument,
             prefixedTypename,
             geometryAttribute,
             fields,
             geomType,
             errorMsg, mayTryWithGMLAS );

    }
    else
    {
      errorMsg = tr( "Cannot find element '%1'" ).arg( unprefixedTypename );
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
    complexTypeElement = schemaElement.firstChildElement( QStringLiteral( "complexType" ) );
    while ( !complexTypeElement.isNull() )
    {
      QString name = complexTypeElement.attribute( QStringLiteral( "name" ) );
      if ( name == elementTypeString )
      {
        break;
      }
      complexTypeElement = complexTypeElement.nextSiblingElement( QStringLiteral( "complexType" ) );
    }
    if ( complexTypeElement.isNull() )
    {
      errorMsg = tr( "Cannot find ComplexType element '%1'" ).arg( elementTypeString );
      return false;
    }
  }

  //we have the relevant <complexType> element. Now find out the geometry and the thematic attributes
  QDomNodeList attributeNodeList = complexTypeElement.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, QStringLiteral( "element" ) );
  if ( attributeNodeList.size() < 1 )
  {
    errorMsg = tr( "Cannot find attribute elements" );
    mayTryWithGMLAS = true;
    return false;
  }

  bool foundGeometryAttribute = false;

  for ( int i = 0; i < attributeNodeList.size(); ++i )
  {
    QDomElement attributeElement = attributeNodeList.at( i ).toElement();

    //attribute name
    QString name = attributeElement.attribute( QStringLiteral( "name" ) );
    // Some servers like http://ogi.state.ok.us/geoserver/wfs on layer ogi:doq_centroids
    // return attribute names padded with spaces. See https://github.com/qgis/QGIS/issues/13486
    // I'm not completely sure how legal this
    // is but this validates with Xerces 3.1, and its schema analyzer does also the trimming.
    name = name.trimmed();

    //attribute type
    QString type = attributeElement.attribute( QStringLiteral( "type" ) );
    if ( type.isEmpty() )
    {
      QDomElement extension = attributeElement.firstChildElement( QStringLiteral( "complexType" ) ).
                              firstChildElement( QStringLiteral( "simpleContent" ) ).firstChildElement( QStringLiteral( "extension" ) );
      if ( !extension.isNull() )
      {
        type = extension.attribute( QStringLiteral( "base" ) );
      }
    }

    // attribute ref
    QString ref = attributeElement.attribute( QStringLiteral( "ref" ) );

    const thread_local QRegularExpression gmlPT( QStringLiteral( "gml:(.*)PropertyType" ) );
    const thread_local QRegularExpression gmlRefProperty( QStringLiteral( "gml:(.*)Property" ) );

    // gmgml: is Geomedia Web Server
    if ( ! foundGeometryAttribute && type == QLatin1String( "gmgml:Polygon_Surface_MultiSurface_CompositeSurfacePropertyType" ) )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = Qgis::WkbType::MultiPolygon;
    }
    else if ( ! foundGeometryAttribute && type == QLatin1String( "gmgml:LineString_Curve_MultiCurve_CompositeCurvePropertyType" ) )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = Qgis::WkbType::MultiLineString;
    }
    // such as http://go.geozug.ch/Zug_WFS_Baumkataster/service.svc/get
    else if ( type == QLatin1String( "gmgml:Point_MultiPointPropertyType" ) )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = Qgis::WkbType::MultiPoint;
    }
    //is it a geometry attribute?
    // the GeometryAssociationType has been seen in #11785
    else if ( ! foundGeometryAttribute && ( type.indexOf( gmlPT ) == 0 || type == QLatin1String( "gml:GeometryAssociationType" ) ) )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      // We have a choice parent element we cannot assume any valid information over the geometry type
      if ( attributeElement.parentNode().nodeName() == QLatin1String( "choice" ) && ! attributeElement.nextSibling().isNull() )
        geomType = Qgis::WkbType::Unknown;
      else
      {
        const QRegularExpressionMatch match = gmlPT.match( type );
        geomType = geomTypeFromPropertyType( geometryAttribute, match.captured( 1 ) );
      }
    }
    //MH 090428: sometimes the <element> tags for geometry attributes have only attribute ref="gml:polygonProperty"
    //Note: this was deprecated with GML3.
    else if ( ! foundGeometryAttribute &&  ref.indexOf( gmlRefProperty ) == 0 )
    {
      foundGeometryAttribute = true;
      geometryAttribute = ref.mid( 4 ); // Strip gml: prefix

      const QRegularExpressionMatch match = gmlRefProperty.match( ref );
      QString propertyType( match.captured( 1 ) );
      // Set the first character in upper case
      propertyType = propertyType.at( 0 ).toUpper() + propertyType.mid( 1 );
      geomType = geomTypeFromPropertyType( geometryAttribute, propertyType );
    }
    else if ( !name.isEmpty() )
    {
      const QVariant::Type attributeType = getVariantTypeFromXML( type );
      if ( attributeType != QVariant::Invalid )
      {
        fields.append( QgsField( name, attributeType, type ) );
      }
      else
      {
        mayTryWithGMLAS = true;
        fields.append( QgsField( name, QVariant::String, type ) );
      }
    }
  }
  if ( !foundGeometryAttribute )
  {
    geomType = Qgis::WkbType::NoGeometry;
  }

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

QgsVectorDataProvider::Capabilities QgsWFSProvider::capabilities() const
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
  QDomElement transactionElem = doc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Transaction" ) );
  const QString WfsVersion = mShared->mWFSVersion;
  // only 1.1.0 and 1.0.0 are supported
  if ( WfsVersion == QLatin1String( "1.1.0" ) )
  {
    transactionElem.setAttribute( QStringLiteral( "version" ), WfsVersion );
  }
  else
  {
    transactionElem.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0.0" ) );
  }
  transactionElem.setAttribute( QStringLiteral( "service" ), QStringLiteral( "WFS" ) );
  transactionElem.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );

  QUrl describeFeatureTypeURL = mShared->mURI.requestUrl( QStringLiteral( "DescribeFeatureType" ) );
  // For tests (since the URL contains part of random data, we need to replace it with a fixed content)
  if ( describeFeatureTypeURL.toString().contains( QLatin1String( "fake_qgis_http_endpoint" ) ) )
  {
    describeFeatureTypeURL = QUrl( QStringLiteral( "http://fake_qgis_http_endpoint" ) );
    QUrlQuery query( describeFeatureTypeURL );
    query.addQueryItem( QStringLiteral( "REQUEST" ), QStringLiteral( "DescribeFeatureType" ) );
    describeFeatureTypeURL.setQuery( query );
  }
  QUrlQuery query( describeFeatureTypeURL );
  query.addQueryItem( QStringLiteral( "VERSION" ), QStringLiteral( "1.0.0" ) );
  query.addQueryItem( QStringLiteral( "TYPENAME" ), mShared->mURI.typeName() );
  describeFeatureTypeURL.setQuery( query );

  transactionElem.setAttribute( QStringLiteral( "xsi:schemaLocation" ), mApplicationNamespace + ' '
                                + describeFeatureTypeURL.toEncoded() );

  QString namespacePrefix = QgsWFSUtils::nameSpacePrefix( mShared->mURI.typeName() );
  if ( !namespacePrefix.isEmpty() )
  {
    transactionElem.setAttribute( "xmlns:" + namespacePrefix, mApplicationNamespace );
  }
  transactionElem.setAttribute( QStringLiteral( "xmlns:gml" ), QgsWFSConstants::GML_NAMESPACE );

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

  if ( WfsVersion == QLatin1String( "1.1.0" ) )
  {
    const QDomNodeList transactionSummaryList = documentElem.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "TransactionSummary" ) );
    if ( transactionSummaryList.size() < 1 )
    {
      return false;
    }

    QDomElement transactionElement { transactionSummaryList.at( 0 ).toElement() };
    QDomNodeList totalInserted = transactionElement.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "totalInserted" ) );
    QDomNodeList totalUpdated = transactionElement.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "totalUpdated" ) );
    QDomNodeList totalDeleted = transactionElement.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "totalDeleted" ) );
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
    totalInserted = transactionElement.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "TotalInserted" ) );
    totalUpdated = transactionElement.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "TotalUpdated" ) );
    totalDeleted = transactionElement.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "TotalDeleted" ) );
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
    const QDomNodeList transactionResultList = documentElem.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "TransactionResult" ) );
    if ( transactionResultList.size() < 1 )
    {
      return false;
    }

    const QDomNodeList statusList = transactionResultList.at( 0 ).toElement().elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Status" ) );
    if ( statusList.size() < 1 )
    {
      return false;
    }

    return statusList.at( 0 ).firstChildElement().localName() == QLatin1String( "SUCCESS" );
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
  if ( mShared->mWFSVersion == QLatin1String( "1.1.0" ) )
  {
    insertResultTagName = QStringLiteral( "InsertResults" );
  }
  else
  {
    insertResultTagName = QStringLiteral( "InsertResult" );
  }
  QDomNodeList insertResultList = rootElem.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, insertResultTagName );
  for ( int i = 0; i < insertResultList.size(); ++i )
  {
    QDomNodeList featureIdList = insertResultList.at( i ).toElement().elementsByTagNameNS( QgsWFSConstants::OGC_NAMESPACE, QStringLiteral( "FeatureId" ) );
    for ( int j = 0; j < featureIdList.size(); ++j )
    {
      QString fidString = featureIdList.at( j ).toElement().attribute( QStringLiteral( "fid" ) );
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
  mCapabilities = QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::Capability::ReloadData;

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
  else if ( mShared->mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
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
      mShared->mPageSize = settings.value( QStringLiteral( "wfs/max_feature_count_if_not_provided" ), "1000" ).toInt();
      QgsDebugError( QStringLiteral( "Server declares paging but does not advertise max feature count and user did not specify it. Using %1" ).arg( mShared->mPageSize ) );
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
          QgsCoordinateReferenceSystem src = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "CRS:84" ) );
          QgsCoordinateTransform ct( src, mShared->mSourceCrs, transformContext() );
          QgsDebugMsgLevel( "latlon ext:" + r.toString(), 4 );
          QgsDebugMsgLevel( "src:" + src.authid(), 4 );
          QgsDebugMsgLevel( "dst:" + mShared->mSourceCrs.authid(), 4 );

          ct.setBallparkTransformsAreAppropriate( true );
          mShared->mCapabilityExtent = ct.transformBoundingBox( r, Qgis::TransformDirection::Forward );
        }
        else
        {
          mShared->mCapabilityExtent = r;
        }

        QgsDebugMsgLevel( "layer ext:" + mShared->mCapabilityExtent.toString(), 4 );
      }
      if ( mShared->mCaps.featureTypes[i].insertCap )
      {
        mCapabilities |= QgsVectorDataProvider::AddFeatures;
      }
      if ( mShared->mCaps.featureTypes[i].updateCap )
      {
        mCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
        mCapabilities |= QgsVectorDataProvider::ChangeGeometries;
      }
      if ( mShared->mCaps.featureTypes[i].deleteCap )
      {
        mCapabilities |= QgsVectorDataProvider::DeleteFeatures;
      }

      foundLayer = true;
    }
  }

  if ( !foundLayer )
  {
    QgsMessageLog::logMessage( tr( "Could not find typename %1 in capabilities for url %2" ).
                               arg( thisLayerName, dataSourceUri() ), tr( "WFS" ) );
  }

  return foundLayer;
}

Qgis::WkbType QgsWFSProvider::geomTypeFromPropertyType( const QString &attName, const QString &propType )
{
  Q_UNUSED( attName )

  QgsDebugMsgLevel( QStringLiteral( "DescribeFeatureType geometry attribute \"%1\" type is \"%2\"" )
                    .arg( attName, propType ), 4 );
  if ( propType == QLatin1String( "Point" ) )
    return Qgis::WkbType::Point;
  if ( propType == QLatin1String( "LineString" ) || propType == QLatin1String( "Curve" ) )
    return Qgis::WkbType::LineString;
  if ( propType == QLatin1String( "Polygon" ) || propType == QLatin1String( "Surface" ) )
    return Qgis::WkbType::Polygon;
  if ( propType == QLatin1String( "MultiPoint" ) )
    return Qgis::WkbType::MultiPoint;
  if ( propType == QLatin1String( "MultiLineString" ) || propType == QLatin1String( "MultiCurve" ) )
    return Qgis::WkbType::MultiLineString;
  if ( propType == QLatin1String( "MultiPolygon" ) || propType == QLatin1String( "MultiSurface" ) )
    return Qgis::WkbType::MultiPolygon;
  return Qgis::WkbType::Unknown;
}

void QgsWFSProvider::handleException( const QDomDocument &serverResponse )
{
  QgsDebugMsgLevel( QStringLiteral( "server response: %1" ).arg( serverResponse.toString() ), 4 );

  QDomElement exceptionElem = serverResponse.documentElement();
  if ( exceptionElem.isNull() )
  {
    pushError( tr( "Empty response" ) );
    return;
  }

  if ( exceptionElem.tagName() == QLatin1String( "ServiceExceptionReport" ) )
  {
    pushError( tr( "WFS service exception: %1" ).arg( exceptionElem.firstChildElement( QStringLiteral( "ServiceException" ) ).text() ) );
    return;
  }

  if ( exceptionElem.tagName() == QLatin1String( "WFS_TransactionResponse" ) )
  {
    pushError( tr( "Unsuccessful service response: %1" ).arg( exceptionElem.firstChildElement( QStringLiteral( "TransactionResult" ) ).firstChildElement( QStringLiteral( "Message" ) ).text() ) );
    return;
  }

  // WFS 1.1.0
  if ( exceptionElem.tagName() == QLatin1String( "TransactionResponse" ) )
  {
    pushError( tr( "Unsuccessful service response: no features were added, deleted or changed." ) );
    return;
  }


  if ( exceptionElem.tagName() == QLatin1String( "ExceptionReport" ) )
  {
    QDomElement exception = exceptionElem.firstChildElement( QStringLiteral( "Exception" ) );
    // The XML schema at http://schemas.opengis.net/ows/1.1.0/owsExceptionReport.xsd uses
    // the "exceptionCode" attribute, but http://docs.opengeospatial.org/is/04-094r1/04-094r1.html#36
    // mentions "code". Accept both...
    pushError( tr( "WFS exception report (code=%1 text=%2)" )
               .arg( exception.attribute( QStringLiteral( "exceptionCode" ),
                                          exception.attribute( QStringLiteral( "code" ), tr( "missing" ) ) ),
                     exception.firstChildElement( QStringLiteral( "ExceptionText" ) ).text() )
             );
    return;
  }

  pushError( tr( "Unhandled response: %1" ).arg( exceptionElem.tagName() ) );
}

QgsWfsCapabilities::Capabilities QgsWFSProvider::getCachedCapabilities( const QString &uri )
{
  static QMutex mutex;
  static std::map<QUrl, std::pair<QDateTime, QgsWfsCapabilities::Capabilities>> gCacheCaps;
  QgsWfsCapabilities getCapabilities( uri );
  QUrl requestUrl = getCapabilities.requestUrl();

  QDateTime now = QDateTime::currentDateTime();
  {
    QMutexLocker lock( &mutex );
    auto iter = gCacheCaps.find( requestUrl );
    QgsSettings s;
    const int delayOfCachingInSecs = s.value( QStringLiteral( "qgis/wfsMemoryCacheDelay" ), 60 ).toInt();
    if ( iter != gCacheCaps.end() && iter->second.first.secsTo( now ) < delayOfCachingInSecs )
    {
      QgsDebugMsgLevel( QStringLiteral( "Reusing cached GetCapabilities response for %1" ).arg( requestUrl.toString() ), 4 );
      return iter->second.second;
    }
  }
  QgsWfsCapabilities::Capabilities caps;
  const bool synchronous = true;
  const bool forceRefresh = false;
  if ( !getCapabilities.requestCapabilities( synchronous, forceRefresh ) )
  {
    QgsMessageLog::logMessage( QObject::tr( "GetCapabilities failed for url %1: %2" ).
                               arg( uri, getCapabilities.errorMessage() ),
                               QObject::tr( "WFS" ) );
    return caps;
  }

  caps = getCapabilities.capabilities();

  QgsSettings s;
  if ( s.value( QStringLiteral( "qgis/wfsMemoryCacheAllowed" ), true ).toBool() )
  {
    QMutexLocker lock( &mutex );
    gCacheCaps[requestUrl] = std::make_pair( now, caps );
  }
  return caps;
}
