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
#include "qgsfield.h"
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
#include "qgscrscache.h"

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
#include <QSettings>

#include <cfloat>

static const QString TEXT_PROVIDER_KEY = "WFS";
static const QString TEXT_PROVIDER_DESCRIPTION = "WFS data provider";

QgsWFSProvider::QgsWFSProvider( const QString& uri, const QgsWFSCapabilities::Capabilities &caps )
    : QgsVectorDataProvider( uri )
    , mShared( new QgsWFSSharedData( uri ) )
    , mWKBType( QGis::WKBUnknown )
    , mValid( true )
    , mCapabilities( 0 )
{
  mShared->mCaps = caps;
  connect( mShared.data(), SIGNAL( raiseError( const QString& ) ), this, SLOT( pushErrorSlot( const QString& ) ) );
  connect( mShared.data(), SIGNAL( extentUpdated() ), this, SIGNAL( fullExtentCalculated() ) );

  if ( uri.isEmpty() )
  {
    mValid = false;
    return;
  }

  //create mSourceCRS from url if possible [WBC 111221] refactored from GetFeatureGET()
  QString srsname = mShared->mURI.SRSName();
  if ( !srsname.isEmpty() )
  {
    if ( srsname == "EPSG:900913" )
      mShared->mSourceCRS = QgsCRSCache::instance()->crsByOgcWmsCrs( "EPSG:3857" );
    else
      mShared->mSourceCRS = QgsCRSCache::instance()->crsByOgcWmsCrs( srsname );
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
    if ( !describeFeatureType( mShared->mGeometryAttribute, mShared->mFields, mWKBType ) )
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
  if ( mWKBType == QGis::WKBUnknown )
  {
    QgsWFSFeatureDownloader downloader( mShared.data() );
    connect( &downloader, SIGNAL( featureReceived( QVector<QgsWFSFeatureGmlIdPair> ) ),
             this, SLOT( featureReceivedAnalyzeOneFeature( QVector<QgsWFSFeatureGmlIdPair> ) ) );
    downloader.run( false, /* serialize features */
                    1 /* maxfeatures */ );
  }

  qRegisterMetaType<QgsRectangle>( "QgsRectangle" );
}

QgsWFSProvider::~QgsWFSProvider()
{
  QgsDebugMsg( "~QgsWFSProvider()" );
}

class QgsWFSProviderSQLFunctionValidator: public QgsSQLStatement::RecursiveVisitor
{
  public:
    QgsWFSProviderSQLFunctionValidator(
      const QList<QgsWFSCapabilities::Function>& spatialPredicatesList,
      const QList<QgsWFSCapabilities::Function>& functionList );

    bool hasError() const { return mError; }

    const QString& errorMessage() const { return mErrorMessage; }

  protected:
    void visit( const QgsSQLStatement::NodeFunction& n ) override;

  private:
    const QList<QgsWFSCapabilities::Function>& mSpatialPredicatesList;
    const QList<QgsWFSCapabilities::Function>& mFunctionList;
    bool mError;
    QString mErrorMessage;
};

QgsWFSProviderSQLFunctionValidator::QgsWFSProviderSQLFunctionValidator(
  const QList<QgsWFSCapabilities::Function>& spatialPredicatesList,
  const QList<QgsWFSCapabilities::Function>& functionList )
    : mSpatialPredicatesList( spatialPredicatesList )
    , mFunctionList( functionList )
    , mError( false )
{
}

void QgsWFSProviderSQLFunctionValidator::visit( const QgsSQLStatement::NodeFunction& n )
{
  if ( !mError )
  {
    bool foundMatch = false;
    Q_FOREACH ( const QgsWFSCapabilities::Function& f, mSpatialPredicatesList )
    {
      if ( n.name().compare( f.name, Qt::CaseInsensitive ) == 0 ||
           ( "ST_" + n.name() ).compare( f.name, Qt::CaseInsensitive ) == 0 )
      {
        foundMatch = true;
      }
    }
    Q_FOREACH ( const QgsWFSCapabilities::Function& f, mFunctionList )
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
      const QgsWFSCapabilities::Capabilities& caps,
      const QString& defaultTypeName,
      const QMap< QString, QString >& mapTypenameAliasToTypename,
      const QMap < QString, QgsFields >& mapTypenameToFields,
      const QMap < QString, QString >& mapTypenameToGeometryAttribute );

    bool hasError() const { return mError; }

    const QString& errorMessage() const { return mErrorMessage; }

  protected:
    void visit( const QgsSQLStatement::NodeColumnRef& n ) override;

  private:
    const QgsWFSCapabilities::Capabilities mCaps;
    QString mDefaultTypeName;
    const QMap< QString, QString >& mMapTableAliasToName;
    const QMap < QString, QgsFields >& mMapTypenameToFields;
    const QMap < QString, QString >& mMapTypenameToGeometryAttribute;
    bool mError;
    QString mErrorMessage;
};

QgsWFSProviderSQLColumnRefValidator::QgsWFSProviderSQLColumnRefValidator(
  const QgsWFSCapabilities::Capabilities& caps,
  const QString& defaultTypeName,
  const QMap< QString, QString >& mapTypenameAliasToTypename,
  const QMap < QString, QgsFields >& mapTypenameToFields,
  const QMap < QString, QString >& mapTypenameToGeometryAttribute )
    : mCaps( caps )
    , mDefaultTypeName( defaultTypeName )
    , mMapTableAliasToName( mapTypenameAliasToTypename )
    , mMapTypenameToFields( mapTypenameToFields )
    , mMapTypenameToGeometryAttribute( mapTypenameToGeometryAttribute )
    , mError( false )
{
}

void QgsWFSProviderSQLColumnRefValidator::visit( const QgsSQLStatement::NodeColumnRef& n )
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
    int idx = tableFields.fieldNameIndex( n.name() );
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


bool QgsWFSProvider::processSQL( const QString& sqlString, QString& errorMsg, QString& warningMsg )
{
  QgsDebugMsg( QString( "Processing SQL: %1" ).arg( sqlString ) );
  errorMsg.clear();
  warningMsg.clear();
  QgsSQLStatement sql( sqlString );
  if ( sql.hasParserError() )
  {
    QString parserErrorString( sql.parserErrorString() );
    QStringList parts( parserErrorString.split( "," ) );
    parserErrorString = "";
    Q_FOREACH ( const QString& part, parts )
    {
      QString newPart( part );
      if ( part == "syntax error" )
        newPart = tr( "Syntax error." );
      else if ( part == " unexpected $end" )
        newPart = tr( "Missing content at end of string." );
      else if ( part.startsWith( " unexpected " ) )
        newPart = tr( "%1 is unexpected." ).arg( part.mid( QString( " unexpected " ).size() ) );
      else if ( part.startsWith( " expecting " ) )
        newPart = tr( "%1 is expected instead." ).arg( part.mid( QString( " expecting " ).size() ) );
      if ( !parserErrorString.isEmpty() )
        parserErrorString += " ";
      parserErrorString += newPart;
    }
    parserErrorString.replace( " or ", tr( "%1 or %2" ).arg( "", "" ) );
    parserErrorString.replace( "COMMA", tr( "comma" ) );
    parserErrorString.replace( "IDENTIFIER", tr( "an identifier" ) );
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
    QgsDebugMsg( "SQL statement is not a SELECT. This should not happen" );
    return false;
  }
  const QgsSQLStatement::NodeSelect* select = dynamic_cast<const QgsSQLStatement::NodeSelect*>( sql.rootNode() );
  if ( !select )
  {
    // Makes Coverity happy, but cannot happen in practice
    QgsDebugMsg( "should not happen" );
    return false;
  }
  mShared->mDistinctSelect = select->distinct();

  QMap< QString, QString > mapTypenameAliasToTypename;
  QMap< QString, QString > mapTypenameToTypenameAlias; // reverse map of the previous one
  QList<QgsSQLStatement::NodeTableDef*> tables = select->tables();
  QList< QString> typenameList;
  bool severalTablesWithSameNameButDifferentPrefix = false;
  QSet< QString > unprefixTypenames;
  Q_FOREACH ( QgsSQLStatement::NodeTableDef* table, tables )
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

  QList<QgsSQLStatement::NodeJoin*> joins = select->joins();
  Q_FOREACH ( QgsSQLStatement::NodeJoin* join, joins )
  {
    QgsSQLStatement::NodeTableDef* table = join->tableDef();
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
  Q_FOREACH ( QString typeName, typenameList )
  {
    if ( !concatenatedTypenames.isEmpty() )
      concatenatedTypenames += ",";
    concatenatedTypenames += typeName;
  }

  QgsWFSDescribeFeatureType describeFeatureType( mShared->mURI.uri() );
  if ( !describeFeatureType.requestFeatureType( mShared->mWFSVersion,
       concatenatedTypenames ) )
  {
    errorMsg = tr( "DescribeFeatureType failed for url %1: %2" ).
               arg( dataSourceUri() ).arg( describeFeatureType.errorMessage() );
    return false;
  }

  const QByteArray& response = describeFeatureType.response();

  QDomDocument describeFeatureDocument;
  errorMsg.clear();
  if ( !describeFeatureDocument.setContent( response, true, &errorMsg ) )
  {
    QgsDebugMsg( response );
    errorMsg = tr( "DescribeFeatureType failed for url %1: %2" ).
               arg( dataSourceUri() ).arg( errorMsg );
    return false;
  }

  mShared->mLayerPropertiesList.clear();
  QMap < QString, QgsFields > mapTypenameToFields;
  QMap < QString, QString > mapTypenameToGeometryAttribute;
  Q_FOREACH ( QString typeName, typenameList )
  {
    QString geometryAttribute;
    QgsFields fields;
    QGis::WkbType geomType;
    if ( !readAttributesFromSchema( describeFeatureDocument,
                                    typeName,
                                    geometryAttribute, fields, geomType, errorMsg ) )
    {
      errorMsg = tr( "Analysis of DescribeFeatureType response failed for url %1, typeName %2: %3" ).
                 arg( dataSourceUri() ).arg( typeName ).arg( errorMsg );
      return false;
    }

    mapTypenameToFields[typeName] = fields;
    mapTypenameToGeometryAttribute[typeName] = geometryAttribute;
    if ( typeName == mShared->mURI.typeName() )
    {
      mShared->mGeometryAttribute = geometryAttribute;
      mWKBType = geomType;
      mThisTypenameFields = fields;
    }

    QgsOgcUtils::LayerProperties layerProperties;
    layerProperties.mName = typeName;
    layerProperties.mGeometryAttribute = geometryAttribute;
    if ( typeName == mShared->mURI.typeName() )
      layerProperties.mSRSName = mShared->srsName();

    mShared->mLayerPropertiesList << layerProperties;
  }

  const QString& defaultTypeName = mShared->mURI.typeName();
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

  QList<QgsSQLStatement::NodeSelectedColumn*> columns = select->columns();
  QMap< QString, QPair<QString, QString> > mapFieldNameToSrcLayerNameFieldName;
  mShared->mFields.clear();
  Q_FOREACH ( QgsSQLStatement::NodeSelectedColumn* selectedcolumn, columns )
  {
    QgsSQLStatement::Node* column = selectedcolumn->column();
    if ( column->nodeType() != QgsSQLStatement::ntColumnRef )
    {
      errorMsg = tr( "Column '%1' is not a direct reference to a table column" ).arg( column->dump() );
      return false;
    }
    QgsSQLStatement::NodeColumnRef* columnRef = dynamic_cast<QgsSQLStatement::NodeColumnRef*>( column );
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
        for ( int i = 0; i < tableFields.size();i++ )
        {
          const QgsField& srcField = tableFields[i];
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
          QgsField field( fieldName, srcField.type(), srcField.typeName() );
          if ( mapFieldNameToSrcLayerNameFieldName.contains( fieldName ) )
          {
            errorMsg = tr( "Field '%1': a field with the same name already exists" ).arg( field.name() );
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
        Q_FOREACH ( QString typeName, typenameList )
        {
          const QgsFields tableFields = mapTypenameToFields[typeName];
          for ( int i = 0; i < tableFields.size();i++ )
          {
            const QgsField& srcField = tableFields[i];
            QString fieldName( srcField.name() );
            // If several tables selected, prefix by table name
            if ( typenameList.size() > 1 )
            {
              QString tablePrefix( mapTypenameToTypenameAlias[typeName] );
              if ( !severalTablesWithSameNameButDifferentPrefix )
                tablePrefix = QgsWFSUtils::removeNamespacePrefix( tablePrefix );
              fieldName = tablePrefix + "." + fieldName;
            }
            QgsField field( fieldName, srcField.type(), srcField.typeName() );
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
        warningMsg = tr( "The geometry field of a typename that is not the main typename is ignored in the selected fields" );
        QgsDebugMsg( warningMsg );
      }
    }
    // Regular field
    else
    {
      const QgsFields tableFields = mapTypenameToFields[columnTableTypename];
      int idx = tableFields.fieldNameIndex( columnRef->name() );
      if ( idx < 0 )
      {
        QgsDebugMsg( QString( "Should not happen. Cannot find field for %1" ).arg( columnRef->name() ) );
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
        errorMsg = tr( "Field '%1': a field with the same name already exists" ).arg( column->dump() );
        return false;
      }

      QgsField field( fieldName, tableFields[idx].type(), tableFields[idx].typeName() );
      mapFieldNameToSrcLayerNameFieldName[ field.name()] =
        QPair<QString, QString>( columnTableTypename, tableFields[idx].name() );
      mShared->mFields.append( field );
    }
  }

  mShared->mMapFieldNameToSrcLayerNameFieldName = mapFieldNameToSrcLayerNameFieldName;

  return true;
}

void QgsWFSProvider::pushErrorSlot( const QString& errorMsg )
{
  pushError( errorMsg );
}

void QgsWFSProvider::featureReceivedAnalyzeOneFeature( QVector<QgsWFSFeatureGmlIdPair> list )
{
  if ( list.size() != 0 )
  {
    QgsFeature feat = list[0].first;
    const QgsGeometry* geometry = feat.constGeometry();
    if ( geometry )
    {
      mWKBType = geometry->wkbType();
    }
  }
}

QString QgsWFSProvider::subsetString()
{
  return mSubsetString;
}

bool QgsWFSProvider::setSubsetString( const QString& theSQL, bool updateFeatureCount )
{
  QgsDebugMsg( QString( "theSql = '%1'" ).arg( theSQL ) );

  // Invalid and cancel current download before altering fields, etc...
  // (crashes might happen if not done at the beginning)
  mShared->invalidateCache();

  mSubsetString = theSQL;
  mCacheMinMaxDirty = true;

  // update URI
  mShared->mFields = mThisTypenameFields;
  mShared->mLayerPropertiesList.clear();
  mShared->mMapFieldNameToSrcLayerNameFieldName.clear();
  mShared->mDistinctSelect = false;
  if ( theSQL.startsWith( "SELECT ", Qt::CaseInsensitive ) ||
       theSQL.startsWith( "SELECT\t", Qt::CaseInsensitive ) ||
       theSQL.startsWith( "SELECT\r", Qt::CaseInsensitive ) ||
       theSQL.startsWith( "SELECT\n", Qt::CaseInsensitive ) )
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
  return true;
}


QgsAbstractFeatureSource* QgsWFSProvider::featureSource() const
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

QGis::WkbType QgsWFSProvider::geometryType() const
{
  return mWKBType;
}

long QgsWFSProvider::featureCount() const
{
  return mShared->getFeatureCount();
}

const QgsFields& QgsWFSProvider::fields() const
{
  return mShared->mFields;
}

QString QgsWFSProvider::geometryAttribute() const
{
  return mShared->mGeometryAttribute;
}

QgsCoordinateReferenceSystem QgsWFSProvider::crs()
{
  return mShared->mSourceCRS;
}

QgsRectangle QgsWFSProvider::extent()
{
  // Some servers return completely buggy extent in their capabilities response
  // so mix it with the extent actually got from the downloaded features
  QgsRectangle computedExtent( mShared->computedExtent() );
  QgsDebugMsg( "computedExtent: " + computedExtent.toString() );
  QgsDebugMsg( "mCapabilityExtent: " + mShared->mCapabilityExtent.toString() );

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

bool QgsWFSProvider::isValid()
{
  return mValid;
}

QgsFeatureIterator QgsWFSProvider::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsWFSFeatureIterator( new QgsWFSFeatureSource( this ), true, request ) );
}

bool QgsWFSProvider::addFeatures( QgsFeatureList &flist )
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
    QDomElement insertElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Insert" );
    transactionElem.appendChild( insertElem );

    QDomElement featureElem = transactionDoc.createElementNS( mApplicationNamespace, tname );

    QgsAttributes featureAttributes = featureIt->attributes();
    int nAttrs = featureAttributes.size();
    for ( int i = 0; i < nAttrs; ++i )
    {
      const QVariant& value = featureAttributes.at( i );
      if ( value.isValid() && !value.isNull() )
      {
        QDomElement fieldElem = transactionDoc.createElementNS( mApplicationNamespace, mShared->mFields.at( i ).name() );
        QDomText fieldText = transactionDoc.createTextNode( convertToXML( value ) );
        fieldElem.appendChild( fieldText );
        featureElem.appendChild( fieldElem );
      }
    }

    //add geometry column (as gml)
    const QgsGeometry* geometry = featureIt->constGeometry();
    if ( geometry != nullptr )
    {
      QDomElement geomElem = transactionDoc.createElementNS( mApplicationNamespace, mShared->mGeometryAttribute );
      QgsGeometry the_geom( *geometry );
      // convert to multi if the layer geom type is multi and the geom is not
      if ( QGis::isMultiType( this->geometryType( ) ) && ! the_geom.isMultipart( ) )
      {
        the_geom.convertToMultiType();
      }
      QDomElement gmlElem = QgsOgcUtils::geometryToGML( &the_geom, transactionDoc );
      if ( !gmlElem.isNull() )
      {
        gmlElem.setAttribute( "srsName", crs().authid() );
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
    QStringList::const_iterator idIt = idList.constBegin();
    featureIt = flist.begin();

    QVector<QgsWFSFeatureGmlIdPair> serializedFeatureList;
    for ( ; idIt != idList.constEnd() && featureIt != flist.end(); ++idIt, ++featureIt )
    {
      serializedFeatureList.push_back( QgsWFSFeatureGmlIdPair( *featureIt, *idIt ) );
    }
    mShared->serializeFeatures( serializedFeatureList );

    // And now set the feature id from the one got from the database
    QMap< QString, QgsFeatureId > map;
    for ( int idx = 0; idx < serializedFeatureList.size(); idx++ )
      map[ serializedFeatureList[idx].second ] = serializedFeatureList[idx].first.id();

    idIt = idList.constBegin();
    featureIt = flist.begin();
    for ( ; idIt != idList.constEnd() && featureIt != flist.end(); ++idIt, ++featureIt )
    {
      if ( map.find( *idIt ) != map.end() )
        featureIt->setFeatureId( map[*idIt] );
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
  QDomElement deleteElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Delete" );
  deleteElem.setAttribute( "typeName", tname );
  QDomElement filterElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, "Filter" );


  QgsFeatureIds::const_iterator idIt = id.constBegin();
  for ( ; idIt != id.constEnd(); ++idIt )
  {
    //find out feature id
    QString gmlid = mShared->findGmlId( *idIt );
    if ( gmlid.isEmpty() )
    {
      QgsDebugMsg( QString( "Cannot identify feature of id %1" ).arg( *idIt ) );
      continue;
    }
    QDomElement featureIdElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, "FeatureId" );
    featureIdElem.setAttribute( "fid", gmlid );
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

  QgsGeometryMap::const_iterator geomIt = geometry_map.constBegin();
  for ( ; geomIt != geometry_map.constEnd(); ++geomIt )
  {
    QString gmlid = mShared->findGmlId( geomIt.key() );
    if ( gmlid.isEmpty() )
    {
      QgsDebugMsg( QString( "Cannot identify feature of id %1" ).arg( geomIt.key() ) );
      continue;
    }
    QDomElement updateElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Update" );
    updateElem.setAttribute( "typeName", tname );
    //Property
    QDomElement propertyElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Property" );
    QDomElement nameElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Name" );
    QDomText nameText = transactionDoc.createTextNode( mShared->mGeometryAttribute );
    nameElem.appendChild( nameText );
    propertyElem.appendChild( nameElem );
    QDomElement valueElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Value" );
    QDomElement gmlElem = QgsOgcUtils::geometryToGML( &geomIt.value(), transactionDoc );
    gmlElem.setAttribute( "srsName", crs().authid() );
    valueElem.appendChild( gmlElem );
    propertyElem.appendChild( valueElem );
    updateElem.appendChild( propertyElem );

    //filter
    QDomElement filterElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, "Filter" );
    QDomElement featureIdElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, "FeatureId" );
    featureIdElem.setAttribute( "fid", gmlid );
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

QString QgsWFSProvider::convertToXML( const QVariant& value )
{
  QString valueStr( value.toString() );
  if ( value.type() == QVariant::DateTime )
  {
    QDateTime dt = value.toDateTime().toUTC();
    valueStr.sprintf( "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                      dt.date().year(), dt.date().month(), dt.date().day(),
                      dt.time().hour(), dt.time().minute(), dt.time().second(), dt.time().msec() );
  }
  return valueStr;
}

bool QgsWFSProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
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

  QgsChangedAttributesMap::const_iterator attIt = attr_map.constBegin();
  for ( ; attIt != attr_map.constEnd(); ++attIt )
  {
    QString gmlid = mShared->findGmlId( attIt.key() );
    if ( gmlid.isEmpty() )
    {
      QgsDebugMsg( QString( "Cannot identify feature of id %1" ).arg( attIt.key() ) );
      continue;
    }

    QDomElement updateElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Update" );
    updateElem.setAttribute( "typeName", tname );

    QgsAttributeMap::const_iterator attMapIt = attIt.value().constBegin();
    for ( ; attMapIt != attIt.value().constEnd(); ++attMapIt )
    {
      QString fieldName = mShared->mFields.at( attMapIt.key() ).name();
      QDomElement propertyElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Property" );

      QDomElement nameElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Name" );
      QDomText nameText = transactionDoc.createTextNode( fieldName );
      nameElem.appendChild( nameText );
      propertyElem.appendChild( nameElem );

      QDomElement valueElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Value" );
      QDomText valueText = transactionDoc.createTextNode( convertToXML( attMapIt.value() ) );
      valueElem.appendChild( valueText );
      propertyElem.appendChild( valueElem );

      updateElem.appendChild( propertyElem );
    }

    //Filter
    QDomElement filterElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, "Filter" );
    QDomElement featureIdElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, "FeatureId" );
    featureIdElem.setAttribute( "fid", gmlid );
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

bool QgsWFSProvider::describeFeatureType( QString& geometryAttribute, QgsFields& fields, QGis::WkbType& geomType )
{
  fields.clear();

  QgsWFSDescribeFeatureType describeFeatureType( mShared->mURI.uri() );
  if ( !describeFeatureType.requestFeatureType( mShared->mWFSVersion,
       mShared->mURI.typeName() ) )
  {
    QgsMessageLog::logMessage( tr( "DescribeFeatureType failed for url %1: %2" ).
                               arg( dataSourceUri() ).arg( describeFeatureType.errorMessage() ), tr( "WFS" ) );
    return false;
  }

  const QByteArray& response = describeFeatureType.response();

  QDomDocument describeFeatureDocument;
  QString errorMsg;
  if ( !describeFeatureDocument.setContent( response, true, &errorMsg ) )
  {
    QgsDebugMsg( response );
    QgsMessageLog::logMessage( tr( "DescribeFeatureType failed for url %1: %2" ).
                               arg( dataSourceUri() ).arg( errorMsg ), tr( "WFS" ) );
    return false;
  }

  if ( !readAttributesFromSchema( describeFeatureDocument,
                                  mShared->mURI.typeName(),
                                  geometryAttribute, fields, geomType, errorMsg ) )
  {
    QgsMessageLog::logMessage( tr( "Analysis of DescribeFeatureType response failed for url %1: %2" ).
                               arg( dataSourceUri() ).arg( errorMsg ), tr( "WFS" ) );
    return false;
  }

  return true;
}

bool QgsWFSProvider::readAttributesFromSchema( QDomDocument& schemaDoc,
    const QString& prefixedTypename,
    QString& geometryAttribute,
    QgsFields& fields,
    QGis::WkbType& geomType,
    QString& errorMsg )
{
  //get the <schema> root element
  QDomNodeList schemaNodeList = schemaDoc.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, "schema" );
  if ( schemaNodeList.length() < 1 )
  {
    errorMsg = tr( "Cannot find schema root element" );
    return false;
  }
  QDomElement schemaElement = schemaNodeList.at( 0 ).toElement();
  mApplicationNamespace = schemaElement.attribute( "targetNamespace" );

  // Remove the namespace on the typename
  QString unprefixedTypename = prefixedTypename;
  if ( unprefixedTypename.contains( ':' ) )
  {
    unprefixedTypename = unprefixedTypename.section( ':', 1 );
  }

  // Find the element whose name is the typename that interests us, and
  // collect the correspond type.
  QDomElement elementElement = schemaElement.firstChildElement( "element" );
  QString elementTypeString;
  while ( !elementElement.isNull() )
  {
    QString name = elementElement.attribute( "name" );
    if ( name == unprefixedTypename )
    {
      elementTypeString = elementElement.attribute( "type" );
      break;
    }
    elementElement = elementElement.nextSiblingElement( "element" );
  }
  if ( elementTypeString.isEmpty() )
  {
    // "http://demo.deegree.org/inspire-workspace/services/wfs?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=ad:Address"
    QDomElement iter = schemaElement.firstChildElement();
    bool onlyIncludeOrImport = true;
    bool foundImport = false;
    while ( !iter.isNull() )
    {
      if ( iter.tagName() == "import" )
        foundImport = true;
      else if ( iter.tagName() != "include" )
      {
        onlyIncludeOrImport = false;
        break;
      }
      iter = iter.nextSiblingElement();
    }
    if ( foundImport && onlyIncludeOrImport )
    {
      errorMsg = tr( "it is probably a schema for Complex Features" );
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

  //the <complexType> element corresponding to the feature type
  QDomElement complexTypeElement = schemaElement.firstChildElement( "complexType" );
  while ( !complexTypeElement.isNull() )
  {
    QString name = complexTypeElement.attribute( "name" );
    if ( name == elementTypeString )
    {
      break;
    }
    complexTypeElement = complexTypeElement.nextSiblingElement( "complexType" );
  }
  if ( complexTypeElement.isNull() )
  {
    errorMsg = tr( "Cannot find ComplexType element '%1'" ).arg( elementTypeString );
    return false;
  }

  //we have the relevant <complexType> element. Now find out the geometry and the thematic attributes
  QDomNodeList attributeNodeList = complexTypeElement.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, "element" );
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
    QString name = attributeElement.attribute( "name" );
    //attribute type
    QString type = attributeElement.attribute( "type" );
    if ( type.isEmpty() )
    {
      QDomElement extension = attributeElement.firstChildElement( "complexType" ).
                              firstChildElement( "simpleContent" ).firstChildElement( "extension" );
      if ( !extension.isNull() )
      {
        type = extension.attribute( "base" );
      }
    }

    QRegExp gmlPT( "gml:(.*)PropertyType" );
    // gmgml: is Geomedia Web Server
    if ( type == "gmgml:Polygon_Surface_MultiSurface_CompositeSurfacePropertyType" )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = QGis::WKBMultiPolygon;
    }
    //is it a geometry attribute?
    //MH 090428: sometimes the <element> tags for geometry attributes have only attribute ref="gml:polygonProperty" and no name
    // the GeometryAssociationType has been seen in #11785
    else if ( type.indexOf( gmlPT ) == 0 || type == "gml:GeometryAssociationType" || name.isEmpty() )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = geomTypeFromPropertyType( geometryAttribute, gmlPT.cap( 1 ) );
    }
    else //todo: distinguish between numerical and non-numerical types
    {
      QVariant::Type  attributeType = QVariant::String; //string is default type
      if ( type.contains( "double", Qt::CaseInsensitive ) || type.contains( "float", Qt::CaseInsensitive ) || type.contains( "decimal", Qt::CaseInsensitive ) )
      {
        attributeType = QVariant::Double;
      }
      else if ( type.contains( "int", Qt::CaseInsensitive ) ||
                type.contains( "short", Qt::CaseInsensitive ) )
      {
        attributeType = QVariant::Int;
      }
      else if ( type.contains( "long", Qt::CaseInsensitive ) )
      {
        attributeType = QVariant::LongLong;
      }
      else if ( type.contains( "dateTime", Qt::CaseInsensitive ) )
      {
        attributeType = QVariant::DateTime;
      }
      fields.append( QgsField( name, attributeType, type ) );
    }
  }
  if ( !foundGeometryAttribute )
  {
    geomType = QGis::WKBNoGeometry;
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

int QgsWFSProvider::capabilities() const
{
  return mCapabilities;
}

bool QgsWFSProvider::sendTransactionDocument( const QDomDocument& doc, QDomDocument& serverResponse )
{
  if ( doc.isNull() )
  {
    return false;
  }

  QgsWFSTransactionRequest request( mShared->mURI.uri() );
  return request.send( doc, serverResponse );
}

QDomElement QgsWFSProvider::createTransactionElement( QDomDocument& doc ) const
{
  QDomElement transactionElem = doc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Transaction" );
  transactionElem.setAttribute( "version", "1.0.0" );
  transactionElem.setAttribute( "service", "WFS" );
  transactionElem.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );

  QUrl describeFeatureTypeURL( mShared->mURI.baseURL() );
  // For tests (since the URL contains part of random data, we need to replace it with a fixed content)
  if ( mShared->mURI.baseURL().toString().contains( "fake_qgis_http_endpoint" ) )
    describeFeatureTypeURL = QUrl( "http://fake_qgis_http_endpoint" );
  describeFeatureTypeURL.addQueryItem( "REQUEST", "DescribeFeatureType" );
  describeFeatureTypeURL.addQueryItem( "VERSION", "1.0.0" );
  describeFeatureTypeURL.addQueryItem( "TYPENAME", mShared->mURI.typeName() );

  transactionElem.setAttribute( "xsi:schemaLocation", mApplicationNamespace + ' '
                                + describeFeatureTypeURL.toEncoded() );

  QString namespacePrefix = QgsWFSUtils::nameSpacePrefix( mShared->mURI.typeName() );
  if ( !namespacePrefix.isEmpty() )
  {
    transactionElem.setAttribute( "xmlns:" + namespacePrefix, mApplicationNamespace );
  }
  transactionElem.setAttribute( "xmlns:gml", QgsWFSConstants::GML_NAMESPACE );

  return transactionElem;
}

bool QgsWFSProvider::transactionSuccess( const QDomDocument& serverResponse ) const
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

  QDomNodeList transactionResultList = documentElem.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, "TransactionResult" );
  if ( transactionResultList.size() < 1 )
  {
    return false;
  }

  QDomNodeList statusList = transactionResultList.at( 0 ).toElement().elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, "Status" );
  if ( statusList.size() < 1 )
  {
    return false;
  }

  if ( statusList.at( 0 ).firstChildElement().localName() == "SUCCESS" )
  {
    return true;
  }
  else
  {
    return false;
  }
}

QStringList QgsWFSProvider::insertedFeatureIds( const QDomDocument& serverResponse ) const
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

  QDomNodeList insertResultList = rootElem.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, "InsertResult" );
  for ( int i = 0; i < insertResultList.size(); ++i )
  {
    QDomNodeList featureIdList = insertResultList.at( i ).toElement().elementsByTagNameNS( QgsWFSConstants::OGC_NAMESPACE, "FeatureId" );
    for ( int j = 0; j < featureIdList.size(); ++j )
    {
      QString fidString = featureIdList.at( j ).toElement().attribute( "fid" );
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
    QgsWFSCapabilities getCapabilities( mShared->mURI.uri() );
    if ( !getCapabilities.requestCapabilities( true ) )
    {
      QgsMessageLog::logMessage( tr( "GetCapabilities failed for url %1: %2" ).
                                 arg( dataSourceUri() ).arg( getCapabilities.errorMessage() ), tr( "WFS" ) );
      return false;
    }

    const QgsWFSCapabilities::Capabilities caps = getCapabilities.capabilities();
    mShared->mCaps = caps;
  }

  mShared->mWFSVersion = mShared->mCaps.version;
  if ( mShared->mURI.maxNumFeatures() > 0 )
    mShared->mMaxFeatures = mShared->mURI.maxNumFeatures();
  else
    mShared->mMaxFeatures = mShared->mCaps.maxFeatures;

  if ( mShared->mMaxFeatures <= 0 && mShared->mCaps.supportsPaging )
  {
    QSettings settings;
    mShared->mMaxFeatures = settings.value( "wfs/max_feature_count_if_not_provided", "1000" ).toInt();
    mShared->mMaxFeaturesWasSetFromDefaultForPaging = true;
    QgsDebugMsg( QString( "Server declares paging but does not advertize max feature count and user did not specify it. Using %1" ).arg( mShared->mMaxFeatures ) );
  }

  //find the <FeatureType> for this layer
  QString thisLayerName = mShared->mURI.typeName();
  bool foundLayer = false;
  for ( int i = 0; i < mShared->mCaps.featureTypes.size(); i++ )
  {
    if ( thisLayerName == mShared->mCaps.featureTypes[i].name )
    {
      const QgsRectangle& r = mShared->mCaps.featureTypes[i].bbox;
      if ( mShared->mSourceCRS.authid().isEmpty() && mShared->mCaps.featureTypes[i].crslist.size() != 0 )
      {
        mShared->mSourceCRS = QgsCRSCache::instance()->crsByOgcWmsCrs( mShared->mCaps.featureTypes[i].crslist[0] );
      }
      if ( !r.isNull() )
      {
        if ( mShared->mCaps.featureTypes[i].bboxSRSIsWGS84 )
        {
          QgsCoordinateReferenceSystem src = QgsCRSCache::instance()->crsByOgcWmsCrs( "CRS:84" );
          QgsCoordinateTransform ct( src, mShared->mSourceCRS );

          QgsDebugMsg( "latlon ext:" + r.toString() );
          QgsDebugMsg( "src:" + src.authid() );
          QgsDebugMsg( "dst:" + mShared->mSourceCRS.authid() );

          mShared->mCapabilityExtent = ct.transformBoundingBox( r, QgsCoordinateTransform::ForwardTransform );
        }
        else
        {
          mShared->mCapabilityExtent = r;
        }

        QgsDebugMsg( "layer ext:" + mShared->mCapabilityExtent.toString() );
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
                               arg( thisLayerName ).arg( dataSourceUri() ), tr( "WFS" ) );
  }

  return foundLayer;
}

QGis::WkbType QgsWFSProvider::geomTypeFromPropertyType( const QString& attName, const QString& propType )
{
  Q_UNUSED( attName );

  QgsDebugMsg( QString( "DescribeFeatureType geometry attribute \"%1\" type is \"%2\"" )
               .arg( attName, propType ) );
  if ( propType == "Point" )
    return QGis::WKBPoint;
  if ( propType == "LineString" || propType == "Curve" )
    return QGis::WKBLineString;
  if ( propType == "Polygon" || propType == "Surface" )
    return QGis::WKBPolygon;
  if ( propType == "MultiPoint" )
    return QGis::WKBMultiPoint;
  if ( propType == "MultiLineString" || propType == "MultiCurve" )
    return QGis::WKBMultiLineString;
  if ( propType == "MultiPolygon" || propType == "MultiSurface" )
    return QGis::WKBMultiPolygon;
  return QGis::WKBUnknown;
}

void QgsWFSProvider::handleException( const QDomDocument& serverResponse )
{
  QgsDebugMsg( QString( "server response: %1" ).arg( serverResponse.toString() ) );

  QDomElement exceptionElem = serverResponse.documentElement();
  if ( exceptionElem.isNull() )
  {
    pushError( tr( "empty response" ) );
    return;
  }

  if ( exceptionElem.tagName() == "ServiceExceptionReport" )
  {
    pushError( tr( "WFS service exception:%1" ).arg( exceptionElem.firstChildElement( "ServiceException" ).text() ) );
    return;
  }

  if ( exceptionElem.tagName() == "WFS_TransactionResponse" )
  {
    pushError( tr( "unsuccessful service response: %1" ).arg( exceptionElem.firstChildElement( "TransactionResult" ).firstChildElement( "Message" ).text() ) );
    return;
  }

  if ( exceptionElem.tagName() == "ExceptionReport" )
  {
    QDomElement exception = exceptionElem.firstChildElement( "Exception" );
    pushError( tr( "WFS exception report (code=%1 text=%2)" )
               .arg( exception.attribute( "exceptionCode", tr( "missing" ) ),
                     exception.firstChildElement( "ExceptionText" ).text() )
             );
    return;
  }

  pushError( tr( "unhandled response: %1" ).arg( exceptionElem.tagName() ) );
}

QGISEXTERN QgsWFSProvider* classFactory( const QString *uri )
{
  return new QgsWFSProvider( *uri );
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
  static bool firstTime = true;
  if ( firstTime )
  {
    QgsWFSUtils::init();
    firstTime = false;
  }

  return true;
}
