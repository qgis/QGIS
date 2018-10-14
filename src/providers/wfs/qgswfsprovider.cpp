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
#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsogcutils.h"

#include "qgswfsconstants.h"
#include "qgswfsfeatureiterator.h"
#include "qgswfsprovider.h"
#include "qgswfscapabilities.h"
#include "qgswfsdescribefeaturetype.h"
#include "qgswfstransactionrequest.h"
#include "qgswfsshareddata.h"
#include "qgswfsutils.h"
#include "qgssettings.h"

#ifdef HAVE_GUI
#include "qgswfssourceselect.h"
#include "qgssourceselectprovider.h"
#endif

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

#include <cfloat>

static const QString TEXT_PROVIDER_KEY = QStringLiteral( "WFS" );
static const QString TEXT_PROVIDER_DESCRIPTION = QStringLiteral( "WFS data provider" );

QgsWFSProvider::QgsWFSProvider( const QString &uri, const ProviderOptions &options, const QgsWfsCapabilities::Capabilities &caps )
  : QgsVectorDataProvider( uri, options )
  , mShared( new QgsWFSSharedData( uri ) )
{
  mShared->mCaps = caps;
  connect( mShared.get(), &QgsWFSSharedData::raiseError, this, &QgsWFSProvider::pushErrorSlot );
  connect( mShared.get(), &QgsWFSSharedData::extentUpdated, this, &QgsWFSProvider::fullExtentCalculated );

  if ( uri.isEmpty() )
  {
    mValid = false;
    return;
  }

  //create mSourceCRS from url if possible [WBC 111221] refactored from GetFeatureGET()
  QString srsname = mShared->mURI.SRSName();
  if ( !srsname.isEmpty() )
  {
    if ( srsname == QLatin1String( "EPSG:900913" ) )
      mShared->mSourceCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:3857" ) );
    else
      mShared->mSourceCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( srsname );
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
    if ( !describeFeatureType( mShared->mGeometryAttribute, mShared->mFields, mShared->mWKBType ) )
    {
      mValid = false;
      return;
    }
    mThisTypenameFields = mShared->mFields;
  }

  if ( !mShared->computeFilter( mProcessSQLErrorMsg ) )
  {
    QgsMessageLog::logMessage( mProcessSQLErrorMsg, tr( "WFS" ) );
    mValid = false;
    return;
  }

  //Failed to detect feature type from describeFeatureType -> get first feature from layer to detect type
  if ( mShared->mWKBType == QgsWkbTypes::Unknown )
  {
    QgsWFSFeatureDownloader downloader( mShared.get() );
    connect( &downloader, static_cast<void ( QgsWFSFeatureDownloader::* )( QVector<QgsWFSFeatureGmlIdPair> )>( &QgsWFSFeatureDownloader::featureReceived ),
             this, &QgsWFSProvider::featureReceivedAnalyzeOneFeature );
    downloader.run( false, /* serialize features */
                    1 /* maxfeatures */ );
  }

  qRegisterMetaType<QgsRectangle>( "QgsRectangle" );
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
    Q_FOREACH ( const QgsWfsCapabilities::Function &f, mSpatialPredicatesList )
    {
      if ( n.name().compare( f.name, Qt::CaseInsensitive ) == 0 ||
           ( "ST_" + n.name() ).compare( f.name, Qt::CaseInsensitive ) == 0 )
      {
        foundMatch = true;
      }
    }
    Q_FOREACH ( const QgsWfsCapabilities::Function &f, mFunctionList )
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
    Q_FOREACH ( const QString &part, parts )
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
        parserErrorString += QLatin1String( " " );
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
    QgsDebugMsg( QStringLiteral( "SQL statement is not a SELECT. This should not happen" ) );
    return false;
  }
  const QgsSQLStatement::NodeSelect *select = dynamic_cast<const QgsSQLStatement::NodeSelect *>( sql.rootNode() );
  if ( !select )
  {
    // Makes Coverity happy, but cannot happen in practice
    QgsDebugMsg( QStringLiteral( "should not happen" ) );
    return false;
  }
  mShared->mDistinctSelect = select->distinct();

  QMap< QString, QString > mapTypenameAliasToTypename;
  QMap< QString, QString > mapTypenameToTypenameAlias; // reverse map of the previous one
  QList<QgsSQLStatement::NodeTableDef *> tables = select->tables();
  QList< QString> typenameList;
  bool severalTablesWithSameNameButDifferentPrefix = false;
  QSet< QString > unprefixTypenames;
  Q_FOREACH ( QgsSQLStatement::NodeTableDef *table, tables )
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
  Q_FOREACH ( QgsSQLStatement::NodeJoin *join, joins )
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
  Q_FOREACH ( const QString &typeName, typenameList )
  {
    if ( !concatenatedTypenames.isEmpty() )
      concatenatedTypenames += QLatin1String( "," );
    concatenatedTypenames += typeName;
  }

  QgsWFSDescribeFeatureType describeFeatureType( mShared->mURI );
  bool bUsePlural = false;
  QByteArray response;
  for ( int i = 0; i < 2; i++ )
  {
    if ( !describeFeatureType.requestFeatureType( mShared->mWFSVersion,
         concatenatedTypenames, bUsePlural ) )
    {
      errorMsg = tr( "DescribeFeatureType failed for url %1: %2" ).
                 arg( dataSourceUri(), describeFeatureType.errorMessage() );
      return false;
    }

    response = describeFeatureType.response();
    // "http://geoportal.samregion.ru/wfs12?SERVICE=WFS&REQUEST=DescribeFeatureType&TYPENAME=EC_1_132&VERSION=2.0.0"
    // returns a <ExceptionText><![CDATA[Missing typeNames parameter]]></ExceptionText>
    if ( i == 0 && response.indexOf( "<![CDATA[Missing typeNames parameter]]>" ) >= 0 )
    {
      QgsDebugMsg( QStringLiteral( "Server does not accept TYPENAME parameter for DescribeFeatureType. Re-trying with TYPENAMES" ) );
      bUsePlural = true;
    }
    else
    {
      break;
    }
  }

  QDomDocument describeFeatureDocument;
  errorMsg.clear();
  if ( !describeFeatureDocument.setContent( response, true, &errorMsg ) )
  {
    QgsDebugMsgLevel( response, 4 );
    errorMsg = tr( "DescribeFeatureType failed for url %1: %2" ).
               arg( dataSourceUri(), errorMsg );
    return false;
  }

  mShared->mLayerPropertiesList.clear();
  QMap < QString, QgsFields > mapTypenameToFields;
  QMap < QString, QString > mapTypenameToGeometryAttribute;
  Q_FOREACH ( const QString &typeName, typenameList )
  {
    QString geometryAttribute;
    QgsFields fields;
    QgsWkbTypes::Type geomType;
    if ( !readAttributesFromSchema( describeFeatureDocument,
                                    typeName,
                                    geometryAttribute, fields, geomType, errorMsg ) )
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
      mThisTypenameFields = fields;
    }

    QgsOgcUtils::LayerProperties layerProperties;
    layerProperties.mName = typeName;
    layerProperties.mGeometryAttribute = geometryAttribute;
    if ( typeName == mShared->mURI.typeName() )
      layerProperties.mSRSName = mShared->srsName();

    mShared->mLayerPropertiesList << layerProperties;
  }

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
  Q_FOREACH ( QgsSQLStatement::NodeSelectedColumn *selectedcolumn, columns )
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
        Q_FOREACH ( const QString &typeName, typenameList )
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
        QgsDebugMsg( warningMsg );
      }
    }
    // Regular field
    else
    {
      const QgsFields tableFields = mapTypenameToFields[columnTableTypename];
      int idx = tableFields.lookupField( columnRef->name() );
      if ( idx < 0 )
      {
        QgsDebugMsg( QStringLiteral( "Should not happen. Cannot find field for %1" ).arg( columnRef->name() ) );
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

void QgsWFSProvider::pushErrorSlot( const QString &errorMsg )
{
  pushError( errorMsg );
}

void QgsWFSProvider::featureReceivedAnalyzeOneFeature( QVector<QgsWFSFeatureGmlIdPair> list )
{
  if ( list.size() != 0 )
  {
    QgsFeature feat = list[0].first;
    QgsGeometry geometry = feat.geometry();
    if ( !geometry.isNull() )
    {
      mShared->mWKBType = geometry.wkbType();

      // Fragile heuristics that helps for
      // https://sampleservices.luciad.com/ogc/wfs/sampleswfs?REQUEST=GetFeature&SERVICE=WFS&TYPENAME=rivers&VERSION=1.1.0
      // In case the geometry is a geometry collection, analyze its members to
      // see if they are of the same type. If then, assume that all features
      // will be similar, and report the proper MultiPoint/MultiLineString/
      // MultiPolygon type instead.
      if ( mShared->mWKBType == QgsWkbTypes::GeometryCollection )
      {
        auto geomColl = geometry.asGeometryCollection();
        mShared->mWKBType = QgsWkbTypes::Unknown;
        for ( const auto &geom : geomColl )
        {
          if ( mShared->mWKBType == QgsWkbTypes::Unknown )
          {
            mShared->mWKBType = geom.wkbType();
          }
          else if ( mShared->mWKBType != geom.wkbType() )
          {
            mShared->mWKBType = QgsWkbTypes::Unknown;
            break;
          }
        }
        if ( mShared->mWKBType != QgsWkbTypes::Unknown )
        {
          if ( mShared->mWKBType == QgsWkbTypes::Point )
          {
            QgsDebugMsg( QStringLiteral( "Layer of unknown type. First element is a GeometryCollection of Point. Advertizing optimistically as MultiPoint" ) );
            mShared->mWKBType = QgsWkbTypes::MultiPoint;
          }
          else if ( mShared->mWKBType == QgsWkbTypes::LineString )
          {
            QgsDebugMsg( QStringLiteral( "Layer of unknown type. First element is a GeometryCollection of LineString. Advertizing optimistically as MultiLineString" ) );
            mShared->mWKBType = QgsWkbTypes::MultiLineString;
          }
          else if ( mShared->mWKBType == QgsWkbTypes::Polygon )
          {
            QgsDebugMsg( QStringLiteral( "Layer of unknown type. First element is a GeometryCollection of Polygon. Advertizing optimistically as MultiPolygon" ) );
            mShared->mWKBType = QgsWkbTypes::MultiPolygon;
          }
          else
          {
            mShared->mWKBType = QgsWkbTypes::Unknown;
          }
        }
      }
    }
  }
}

QString QgsWFSProvider::subsetString() const
{
  return mSubsetString;
}

bool QgsWFSProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
{
  QgsDebugMsgLevel( QStringLiteral( "theSql = '%1'" ).arg( theSQL ), 4 );

  if ( theSQL == mSubsetString )
    return true;

  // Invalid and cancel current download before altering fields, etc...
  // (crashes might happen if not done at the beginning)
  mShared->invalidateCache();

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
    mShared->mURI.setSql( QString() );
    mShared->mURI.setFilter( theSQL );
  }

  setDataSourceUri( mShared->mURI.uri() );
  QString errorMsg;
  if ( !mShared->computeFilter( errorMsg ) )
    QgsMessageLog::logMessage( errorMsg, tr( "WFS" ) );
  reloadData();
  if ( updateFeatureCount )
    featureCount();

  emit dataChanged();

  return true;
}


QgsAbstractFeatureSource *QgsWFSProvider::featureSource() const
{
  QgsWFSFeatureSource *fs = new QgsWFSFeatureSource( this );
  /*connect( fs, SIGNAL( extentRequested( const QgsRectangle & ) ),
           this, SLOT( extendExtent( const QgsRectangle & ) ) );*/
  return fs;
}

void QgsWFSProvider::reloadData()
{
  mShared->invalidateCache();
  QgsVectorDataProvider::reloadData();
}

QgsWkbTypes::Type QgsWFSProvider::wkbType() const
{
  return mShared->mWKBType;
}

long QgsWFSProvider::featureCount() const
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
  return mShared->mSourceCRS;
}

QgsRectangle QgsWFSProvider::extent() const
{
  // Some servers return completely buggy extent in their capabilities response
  // so mix it with the extent actually got from the downloaded features
  QgsRectangle computedExtent( mShared->computedExtent() );
  QgsDebugMsgLevel( QStringLiteral( "computedExtent: " ) + computedExtent.toString(), 4 );
  QgsDebugMsgLevel( QStringLiteral( "mCapabilityExtent: " ) + mShared->mCapabilityExtent.toString(), 4 );

  // If we didn't get any feature, then return capabilities extent.
  if ( computedExtent.isNull() )
    return mShared->mCapabilityExtent;

  // If the capabilities extent is completely off from the features, then
  // use feature extent.
  // Case of standplaats layer of http://geodata.nationaalgeoregister.nl/bag/wfs
  if ( !computedExtent.intersects( mShared->mCapabilityExtent ) )
    return computedExtent;

  if ( mShared->downloadFinished() )
  {
    return computedExtent;
  }

  computedExtent.combineExtentWith( mShared->mCapabilityExtent );
  return computedExtent;
}

bool QgsWFSProvider::isValid() const
{
  return mValid;
}

QgsFeatureIterator QgsWFSProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return QgsFeatureIterator( new QgsWFSFeatureIterator( new QgsWFSFeatureSource( this ), true, request ) );
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
      if ( value.isValid() && !value.isNull() )
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
      QDomElement gmlElem = QgsOgcUtils::geometryToGML( the_geom, transactionDoc );
      if ( !gmlElem.isNull() )
      {
        gmlElem.setAttribute( QStringLiteral( "srsName" ), crs().authid() );
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
    Q_FOREACH ( const QString &v, idList )
    {
      if ( v.startsWith( QLatin1String( "new" ) ) )
      {
        reloadData();
        return true;
      }
    }
    QStringList::const_iterator idIt = idList.constBegin();
    featureIt = flist.begin();

    QVector<QgsWFSFeatureGmlIdPair> serializedFeatureList;
    for ( ; idIt != idList.constEnd() && featureIt != flist.end(); ++idIt, ++featureIt )
    {
      serializedFeatureList.push_back( QgsWFSFeatureGmlIdPair( *featureIt, *idIt ) );
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
    QString gmlid = mShared->findGmlId( *idIt );
    if ( gmlid.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "Cannot identify feature of id %1" ).arg( *idIt ) );
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
    QString gmlid = mShared->findGmlId( geomIt.key() );
    if ( gmlid.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "Cannot identify feature of id %1" ).arg( geomIt.key() ) );
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
    QDomElement gmlElem = QgsOgcUtils::geometryToGML( geomIt.value(), transactionDoc );
    gmlElem.setAttribute( QStringLiteral( "srsName" ), crs().authid() );
    valueElem.appendChild( gmlElem );
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
      valueStr.sprintf( "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                        dt.date().year(), dt.date().month(), dt.date().day(),
                        dt.time().hour(), dt.time().minute(), dt.time().second(), dt.time().msec() );
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
    QString gmlid = mShared->findGmlId( attIt.key() );
    if ( gmlid.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "Cannot identify feature of id %1" ).arg( attIt.key() ) );
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

      if ( attMapIt.value().isValid() && !attMapIt.value().isNull() )
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
  request.setSubsetOfAttributes( QgsAttributeList() );
  request.setFlags( QgsFeatureRequest::NoGeometry );

  // Whoops, the WFS provider returns an empty iterator when we are using
  // a setLimit call in combination with a subsetString.
  // Remove this method (and default to the QgsVectorDataProvider one)
  // once this is fixed
#if 0
  request.setLimit( 1 );
#endif
  return !getFeatures( request ).nextFeature( f );

};

bool QgsWFSProvider::describeFeatureType( QString &geometryAttribute, QgsFields &fields, QgsWkbTypes::Type &geomType )
{
  fields.clear();

  QgsWFSDescribeFeatureType describeFeatureType( mShared->mURI );
  bool bUsePlural = false;
  QByteArray response;
  for ( int i = 0; i < 2; i++ )
  {
    if ( !describeFeatureType.requestFeatureType( mShared->mWFSVersion,
         mShared->mURI.typeName(), bUsePlural ) )
    {
      QgsMessageLog::logMessage( tr( "DescribeFeatureType network request failed for url %1: %2" ).
                                 arg( dataSourceUri(), describeFeatureType.errorMessage() ), tr( "WFS" ) );
      return false;
    }

    response = describeFeatureType.response();
    // "http://geoportal.samregion.ru/wfs12?SERVICE=WFS&REQUEST=DescribeFeatureType&TYPENAME=EC_1_132&VERSION=2.0.0"
    // returns a <ExceptionText><![CDATA[Missing typeNames parameter]]></ExceptionText>
    if ( i == 0 && response.indexOf( "<![CDATA[Missing typeNames parameter]]>" ) >= 0 )
    {
      QgsDebugMsg( QStringLiteral( "Server does not accept TYPENAME parameter for DescribeFeatureType. Re-trying with TYPENAMES" ) );
      bUsePlural = true;
    }
    // "http://services.cuzk.cz/wfs/inspire-cp-wfs.asp?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=cp:CadastralParcel" returns
    // <!--Generated by Marushka, version 4.2.5.0, GEOVAP, spol. s r.o., 31.05.2018.-->
    // <ExceptionReport xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" version="1.0.0" xml:lang="en-US" xmlns="http://www.opengis.net/ows/1.1">
    // <Exception exceptionCode="OperationProcessingFailed" />
    // </ExceptionReport>
    else if ( i == 0 && response.indexOf( "<!--Generated by Marushka" ) >= 0 &&
              response.indexOf( "OperationProcessingFailed" ) >= 0 )
    {
      QgsDebugMsg( QStringLiteral( "Server does not accept TYPENAME parameter for DescribeFeatureType. Re-trying with TYPENAMES" ) );
      bUsePlural = true;
    }
    else
    {
      break;
    }
  }

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
                                  mShared->mURI.typeName(),
                                  geometryAttribute, fields, geomType, errorMsg ) )
  {
    QgsDebugMsgLevel( response, 4 );
    QgsMessageLog::logMessage( tr( "Analysis of DescribeFeatureType response failed for url %1: %2" ).
                               arg( dataSourceUri(), errorMsg ), tr( "WFS" ) );
    return false;
  }

  return true;
}

bool QgsWFSProvider::readAttributesFromSchema( QDomDocument &schemaDoc,
    const QString &prefixedTypename,
    QString &geometryAttribute,
    QgsFields &fields,
    QgsWkbTypes::Type &geomType,
    QString &errorMsg )
{
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
      if ( !describeFeatureType.sendGET( schemaLocation, true, false ) )
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

      return readAttributesFromSchema( describeFeatureDocument,
                                       prefixedTypename,
                                       geometryAttribute,
                                       fields,
                                       geomType,
                                       errorMsg );

    }
    else
    {
      errorMsg = tr( "Cannot find element '%1'" ).arg( unprefixedTypename );
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
    return false;
  }

  bool foundGeometryAttribute = false;

  for ( int i = 0; i < attributeNodeList.size(); ++i )
  {
    QDomElement attributeElement = attributeNodeList.at( i ).toElement();

    //attribute name
    QString name = attributeElement.attribute( QStringLiteral( "name" ) );
    // Some servers like http://ogi.state.ok.us/geoserver/wfs on layer ogi:doq_centroids
    // return attribute names padded with spaces. See https://issues.qgis.org/issues/3426
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

    QRegExp gmlPT( "gml:(.*)PropertyType" );
    QRegExp gmlRefProperty( "gml:(.*)Property" );

    // gmgml: is Geomedia Web Server
    if ( ! foundGeometryAttribute && type == QLatin1String( "gmgml:Polygon_Surface_MultiSurface_CompositeSurfacePropertyType" ) )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = QgsWkbTypes::MultiPolygon;
    }
    else if ( ! foundGeometryAttribute && type == QLatin1String( "gmgml:LineString_Curve_MultiCurve_CompositeCurvePropertyType" ) )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = QgsWkbTypes::MultiLineString;
    }
    // such as http://go.geozug.ch/Zug_WFS_Baumkataster/service.svc/get
    else if ( type == QLatin1String( "gmgml:Point_MultiPointPropertyType" ) )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = QgsWkbTypes::MultiPoint;
    }
    //is it a geometry attribute?
    // the GeometryAssociationType has been seen in #11785
    else if ( ! foundGeometryAttribute && ( type.indexOf( gmlPT ) == 0 || type == QLatin1String( "gml:GeometryAssociationType" ) ) )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      // We have a choice parent element we cannot assume any valid information over the geometry type
      if ( attributeElement.parentNode().nodeName() == QStringLiteral( "choice" ) && ! attributeElement.nextSibling().isNull() )
        geomType = QgsWkbTypes::Unknown;
      else
        geomType = geomTypeFromPropertyType( geometryAttribute, gmlPT.cap( 1 ) );
    }
    //MH 090428: sometimes the <element> tags for geometry attributes have only attribute ref="gml:polygonProperty"
    //Note: this was deprecated with GML3.
    else if ( ! foundGeometryAttribute &&  ref.indexOf( gmlRefProperty ) == 0 )
    {
      foundGeometryAttribute = true;
      geometryAttribute = ref.mid( 4 ); // Strip gml: prefix
      QString propertyType( gmlRefProperty.cap( 1 ) );
      // Set the first character in upper case
      propertyType = propertyType.at( 0 ).toUpper() + propertyType.mid( 1 );
      geomType = geomTypeFromPropertyType( geometryAttribute, propertyType );
    }
    else if ( !name.isEmpty() ) //todo: distinguish between numerical and non-numerical types
    {
      QVariant::Type  attributeType = QVariant::String; //string is default type
      if ( type.contains( QLatin1String( "double" ), Qt::CaseInsensitive ) || type.contains( QLatin1String( "float" ), Qt::CaseInsensitive ) || type.contains( QLatin1String( "decimal" ), Qt::CaseInsensitive ) )
      {
        attributeType = QVariant::Double;
      }
      else if ( type.contains( QLatin1String( "int" ), Qt::CaseInsensitive ) ||
                type.contains( QLatin1String( "short" ), Qt::CaseInsensitive ) )
      {
        attributeType = QVariant::Int;
      }
      else if ( type.contains( QLatin1String( "long" ), Qt::CaseInsensitive ) )
      {
        attributeType = QVariant::LongLong;
      }
      else if ( type.contains( QLatin1String( "dateTime" ), Qt::CaseInsensitive ) )
      {
        attributeType = QVariant::DateTime;
      }
      fields.append( QgsField( name, attributeType, type ) );
    }
  }
  if ( !foundGeometryAttribute )
  {
    geomType = QgsWkbTypes::NoGeometry;
  }

  return true;
}

QString QgsWFSProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsWFSProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
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

  QgsWFSTransactionRequest request( mShared->mURI );
  return request.send( doc, serverResponse );
}

QDomElement QgsWFSProvider::createTransactionElement( QDomDocument &doc ) const
{
  QDomElement transactionElem = doc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Transaction" ) );
  // QString WfsVersion = mShared->mWFSVersion;
  // For now: hardcoded to 1.0.0
  QString WfsVersion = QStringLiteral( "1.0.0" );
  transactionElem.setAttribute( QStringLiteral( "version" ), WfsVersion );
  transactionElem.setAttribute( QStringLiteral( "service" ), QStringLiteral( "WFS" ) );
  transactionElem.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );

  QUrl describeFeatureTypeURL = mShared->mURI.requestUrl( QStringLiteral( "DescribeFeatureType" ) );
  // For tests (since the URL contains part of random data, we need to replace it with a fixed content)
  if ( describeFeatureTypeURL.toString().contains( QLatin1String( "fake_qgis_http_endpoint" ) ) )
  {
    describeFeatureTypeURL = QUrl( QStringLiteral( "http://fake_qgis_http_endpoint" ) );
    describeFeatureTypeURL.addQueryItem( QStringLiteral( "REQUEST" ), QStringLiteral( "DescribeFeatureType" ) );
  }
  describeFeatureTypeURL.addQueryItem( QStringLiteral( "VERSION" ), QStringLiteral( "1.0.0" ) );
  describeFeatureTypeURL.addQueryItem( QStringLiteral( "TYPENAME" ), mShared->mURI.typeName() );

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

  QDomNodeList transactionResultList = documentElem.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "TransactionResult" ) );
  if ( transactionResultList.size() < 1 )
  {
    return false;
  }

  QDomNodeList statusList = transactionResultList.at( 0 ).toElement().elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "Status" ) );
  if ( statusList.size() < 1 )
  {
    return false;
  }

  return statusList.at( 0 ).firstChildElement().localName() == QLatin1String( "SUCCESS" );
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

  QDomNodeList insertResultList = rootElem.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, QStringLiteral( "InsertResult" ) );
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
  mCapabilities = QgsVectorDataProvider::SelectAtId;

  if ( mShared->mCaps.version.isEmpty() )
  {
    QgsWfsCapabilities getCapabilities( mShared->mURI.uri() );
    const bool synchronous = true;
    const bool forceRefresh = false;
    if ( !getCapabilities.requestCapabilities( synchronous, forceRefresh ) )
    {
      QgsMessageLog::logMessage( tr( "GetCapabilities failed for url %1: %2" ).
                                 arg( dataSourceUri(), getCapabilities.errorMessage() ), tr( "WFS" ) );
      return false;
    }

    const QgsWfsCapabilities::Capabilities caps = getCapabilities.capabilities();
    mShared->mCaps = caps;
    mShared->mURI.setGetEndpoints( caps.operationGetEndpoints );
    mShared->mURI.setPostEndpoints( caps.operationPostEndpoints );
  }

  mShared->mWFSVersion = mShared->mCaps.version;
  if ( mShared->mURI.maxNumFeatures() > 0 && mShared->mCaps.maxFeatures > 0 && !( mShared->mCaps.supportsPaging && mShared->mURI.pagingEnabled() ) )
  {
    mShared->mMaxFeatures = std::min( mShared->mURI.maxNumFeatures(), mShared->mCaps.maxFeatures );
  }
  else if ( mShared->mURI.maxNumFeatures() > 0 )
  {
    mShared->mMaxFeatures = mShared->mURI.maxNumFeatures();
  }
  else if ( mShared->mCaps.maxFeatures > 0 && !( mShared->mCaps.supportsPaging && mShared->mURI.pagingEnabled() ) )
  {
    mShared->mMaxFeatures = mShared->mCaps.maxFeatures;
  }
  else
  {
    mShared->mMaxFeatures = 0;
  }

  if ( mShared->mCaps.supportsPaging && mShared->mURI.pagingEnabled() )
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
      QgsDebugMsg( QStringLiteral( "Server declares paging but does not advertize max feature count and user did not specify it. Using %1" ).arg( mShared->mPageSize ) );
    }
  }
  else
  {
    mShared->mPageSize = 0;
  }

  //find the <FeatureType> for this layer
  QString thisLayerName = mShared->mURI.typeName();
  bool foundLayer = false;
  for ( int i = 0; i < mShared->mCaps.featureTypes.size(); i++ )
  {
    if ( thisLayerName == mShared->mCaps.featureTypes[i].name )
    {
      const QgsRectangle &r = mShared->mCaps.featureTypes[i].bbox;
      if ( mShared->mSourceCRS.authid().isEmpty() && mShared->mCaps.featureTypes[i].crslist.size() != 0 )
      {
        mShared->mSourceCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mShared->mCaps.featureTypes[i].crslist[0] );
      }
      if ( !r.isNull() )
      {
        if ( mShared->mCaps.featureTypes[i].bboxSRSIsWGS84 )
        {
          QgsCoordinateReferenceSystem src = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "CRS:84" ) );
          Q_NOWARN_DEPRECATED_PUSH
          QgsCoordinateTransform ct( src, mShared->mSourceCRS );
          Q_NOWARN_DEPRECATED_POP

          QgsDebugMsgLevel( "latlon ext:" + r.toString(), 4 );
          QgsDebugMsgLevel( "src:" + src.authid(), 4 );
          QgsDebugMsgLevel( "dst:" + mShared->mSourceCRS.authid(), 4 );

          mShared->mCapabilityExtent = ct.transformBoundingBox( r, QgsCoordinateTransform::ForwardTransform );
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

QgsWkbTypes::Type QgsWFSProvider::geomTypeFromPropertyType( const QString &attName, const QString &propType )
{
  Q_UNUSED( attName );

  QgsDebugMsgLevel( QStringLiteral( "DescribeFeatureType geometry attribute \"%1\" type is \"%2\"" )
                    .arg( attName, propType ), 4 );
  if ( propType == QLatin1String( "Point" ) )
    return QgsWkbTypes::Point;
  if ( propType == QLatin1String( "LineString" ) || propType == QLatin1String( "Curve" ) )
    return QgsWkbTypes::LineString;
  if ( propType == QLatin1String( "Polygon" ) || propType == QLatin1String( "Surface" ) )
    return QgsWkbTypes::Polygon;
  if ( propType == QLatin1String( "MultiPoint" ) )
    return QgsWkbTypes::MultiPoint;
  if ( propType == QLatin1String( "MultiLineString" ) || propType == QLatin1String( "MultiCurve" ) )
    return QgsWkbTypes::MultiLineString;
  if ( propType == QLatin1String( "MultiPolygon" ) || propType == QLatin1String( "MultiSurface" ) )
    return QgsWkbTypes::MultiPolygon;
  return QgsWkbTypes::Unknown;
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

  if ( exceptionElem.tagName() == QLatin1String( "ExceptionReport" ) )
  {
    QDomElement exception = exceptionElem.firstChildElement( QStringLiteral( "Exception" ) );
    pushError( tr( "WFS exception report (code=%1 text=%2)" )
               .arg( exception.attribute( QStringLiteral( "exceptionCode" ), tr( "missing" ) ),
                     exception.firstChildElement( QStringLiteral( "ExceptionText" ) ).text() )
             );
    return;
  }

  pushError( tr( "Unhandled response: %1" ).arg( exceptionElem.tagName() ) );
}

QGISEXTERN QgsWFSProvider *classFactory( const QString *uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsWFSProvider( *uri, options );
}

QGISEXTERN QString providerKey()
{
  return TEXT_PROVIDER_KEY;
}

QGISEXTERN QString description()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QGISEXTERN bool isProvider()
{
  // This function should normally be called just once, but better check
  // so as to avoid doing twice the initial cleanup of the temporary cache
  // (which should normally be empty, unless QGIS was killed)
  static bool sFirstTime = true;
  if ( sFirstTime )
  {
    QgsWFSUtils::init();
    sFirstTime = false;
  }

  return true;
}

#ifdef HAVE_GUI

//! Provider for WFS layers source select
class QgsWfsSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "WFS" ); }
    QString text() const override { return QObject::tr( "WFS" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 40; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddWfsLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsWFSSourceSelect( parent, fl, widgetMode );
    }
};


QGISEXTERN QList<QgsSourceSelectProvider *> *sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> *providers = new QList<QgsSourceSelectProvider *>();

  *providers
      << new QgsWfsSourceSelectProvider;

  return providers;
}
#endif
