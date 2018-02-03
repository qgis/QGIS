/***************************************************************************
           qgsspatialiteprovider.cpp Data provider for SpatiaLite DBMS
begin                : Dec, 2008
copyright            : (C) 2008 Sandro Furieri
email                : a.furieri@lqt.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
extern "C"
{
#include <sqlite3.h>
#include <spatialite.h>
}

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsmessageoutput.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsvectorlayerexporter.h"
#include "qgsspatialiteprovider.h"
#include "qgsspatialitefeatureiterator.h"

#ifdef HAVE_GUI
#include "qgssourceselectprovider.h"
#include "qgsspatialitesourceselect.h"
#endif

#include "qgsfeedback.h"

#include <qgsjsonutils.h>
#include <qgsvectorlayer.h>

#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include "qgsspatialiteutils.h"
//-----------------------------------------------------------------
// SPATIALITE_KEY, SPATIALITE_DESCRIPTION
//-----------------------------------------------------------------
//-- Mandatory functions for each Provider
//--> each Provider must be created in an extra library [extra library]
//--> when creating inside a internal library [core library], create as class internal functions
//-----------------------------------------------------------------
const QString SPATIALITE_KEY = QStringLiteral( "spatialite" );
const QString SPATIALITE_DESCRIPTION = QStringLiteral( "SpatiaLite 5.0 data provider" );
//-----------------------------------------------------------------
// QGISEXTERN isProvider [Required Provider function]
//-----------------------------------------------------------------
// Used to determine if this shared library is a data provider plugin
//-----------------------------------------------------------------
QGISEXTERN bool isProvider()
{
  return true;
}
//-----------------------------------------------------------------
// QGISEXTERN providerKey [Required Provider function]
//-----------------------------------------------------------------
// Used to map the plugin to a data store type
//-----------------------------------------------------------------
QGISEXTERN QString providerKey()
{
  return SPATIALITE_KEY;
}
//-----------------------------------------------------------------
// QGISEXTERN description [Required Provider function]
//-----------------------------------------------------------------
// Required description function
//-----------------------------------------------------------------
QGISEXTERN QString description()
{
  return SPATIALITE_DESCRIPTION;
}
//-----------------------------------------------------------------
// QGISEXTERN classFactory [Required Provider function]
//-----------------------------------------------------------------
// Class factory to return a pointer to a newly created
// QgsSpatiaLiteProvider object
//-----------------------------------------------------------------
QGISEXTERN QgsSpatiaLiteProvider *classFactory( const QString *uri )
{
  return new QgsSpatiaLiteProvider( *uri );
}
#ifdef HAVE_GUI
//-----------------------------------------------------------------
// class QgsSourceSelectProvider
//-----------------------------------------------------------------
//! Provider for SQLite vector/raster source select
//-----------------------------------------------------------------
class QgsSpatiaLiteSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    virtual QString providerKey() const override { return QStringLiteral( "spatialite" ); }
    virtual QString text() const override { return QObject::tr( "SpatiaLite 5.0" ); }
    virtual int ordering() const override { return QgsSourceSelectProvider::OrderLocalProvider + 40; }
    virtual QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewSpatiaLiteLayer.svg" ) ); }
    virtual QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsSpatiaLiteSourceSelect( parent, fl, widgetMode );
      // return new QgsOgrDbSourceSelect( QStringLiteral( "SQLite" ), QObject::tr( "SQLite" ),  QObject::tr( "SpatiaLite Database (*.db *.sqlite)" ), parent, fl, widgetMode );
    }

};
//-----------------------------------------------------------------
// QGISEXTERN sourceSelectProviders
//-----------------------------------------------------------------
QGISEXTERN QList<QgsSourceSelectProvider *> *sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> *providers = new QList<QgsSourceSelectProvider *>();

  *providers << new QgsSpatiaLiteSourceSelectProvider;

  return providers;
}
#endif
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::QgsSpatiaLiteProvider
//-----------------------------------------------------------------
QgsSpatiaLiteProvider::QgsSpatiaLiteProvider( QString const &uri )
  : QgsVectorDataProvider( uri )
  , mIsQuery( false )
  , mTableBased( false )
  , mViewBased( false )
  , mVShapeBased( false )
  , mReadOnly( false )
  , mUriTableName( QString::null )
  , mUriGeometryColumn( QString::null )
  , mUriLayerName( QString::null )
  , mGeometryType( QgsWkbTypes::Unknown )
  , mSpatialIndexRTree( false )
  , mSpatialIndexMbrCache( false )
{
  QgsDataSourceUri anUri = QgsDataSourceUri( uri );
  // parsing members from the uri structure
  mUriTableName = anUri.table();
  mUriGeometryColumn = anUri.geometryColumn().toLower();
  mSqlitePath = anUri.database();
  mSubsetString = anUri.sql(); // \"id_admin\"  < 2
  mUriPrimaryKey = anUri.keyColumn();
  mQuery = mUriTableName;
  // trying to open the SQLite DB
  bool bShared = true;
  bool bLoadLayers = true;
#if 1
  mUriLayerName = mUriTableName;
  if ( !mUriGeometryColumn.isEmpty() )
  {
    mUriLayerName = QString( "%1(%2)" ).arg( mUriTableName ).arg( mUriGeometryColumn );
    bLoadLayers = true;
  }
#endif
  if ( setSqliteHandle( QgsSqliteHandle::openDb( mSqlitePath, bShared, mUriLayerName, bLoadLayers ) ) )
  {
    QStringList pragmaList = anUri.params( QStringLiteral( "pragma" ) );
    Q_FOREACH ( const QString &pragma, pragmaList )
    {
      char *errMsg = nullptr;
      int ret = sqlite3_exec( dbSqliteHandle(), ( "PRAGMA " + pragma ).toUtf8(), nullptr, nullptr, &errMsg );
      if ( ret != SQLITE_OK )
      {
        QgsDebugMsgLevel( QString( "PRAGMA %1 %2" ).arg( pragma ).arg( QString( " failed : %1" ).arg( errMsg ? errMsg : "" ) ), 3 );
      }
      sqlite3_free( errMsg );
    }
    mValid = true;
  }
  else
  {
    mValid = false;
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::~QgsSpatiaLiteProvider
//-----------------------------------------------------------------
QgsSpatiaLiteProvider::~QgsSpatiaLiteProvider()
{
  closeDb();
  invalidateConnections( mSqlitePath );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::isValid
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::isValid() const
{
  return isLayerValid();
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::name
//-----------------------------------------------------------------
QString QgsSpatiaLiteProvider::name() const
{
  return SPATIALITE_KEY;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::description
//-----------------------------------------------------------------
QString QgsSpatiaLiteProvider::description() const
{
  return SPATIALITE_DESCRIPTION;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::fields
//-----------------------------------------------------------------
QgsFields QgsSpatiaLiteProvider::fields() const
{
  return getAttributeFields();
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::closeDb
//-----------------------------------------------------------------
void QgsSpatiaLiteProvider::closeDb()
{
  if ( isDbValid() )
  {
    if ( mSpatialiteDbInfo->getConnectionRef() <= 1 )
    {
      // Delete only if not being used elsewhere, Connection will be closed
      delete mSpatialiteDbInfo;
    }
    else
    {
      // Inform QgsSqliteHandle that this connection is no longer being used
      mSpatialiteDbInfo->removeConnectionRef();
    }
  }
  mSpatialiteDbInfo = nullptr;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::invalidateConnections
//-----------------------------------------------------------------
void QgsSpatiaLiteProvider::invalidateConnections( const QString &connection )
{
  QgsSpatiaLiteConnPool::instance()->invalidateConnections( connection );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::setSqliteHandle
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::setSqliteHandle( QgsSqliteHandle *qSqliteHandle )
{
  bool bRc = false;
  mQSqliteHandle = qSqliteHandle;
  if ( getQSqliteHandle() )
  {
    mSpatialiteDbInfo = getQSqliteHandle()->getSpatialiteDbInfo();
    if ( getSpatialiteDbInfo() )
    {
      //-----------------------------------------------------------------
      // Setting needed values from Database
      //-----------------------------------------------------------------
      mIsDbValid = getSpatialiteDbInfo()->isDbValid();
      if ( ( isDbValid() ) && ( isDbSpatialite() ) )
      {
        // Calls 'mod_spatialite', if not done allready
        if ( dbHasSpatialite() )
        {
          // -- ---------------------------------- --
          // The combination isDbValid() and isDbSpatialite()
          //  - means that the given Layer is supported by the QgsSpatiaLiteProvider
          //  --> i.e. not GeoPackage, MBTiles etc.
          //  - RasterLite1 will return isDbGdalOgr() == 1
          //  -> which renders the RasterLayers with gdal
          //  --> so a check must be done later
          //  ---> that the layer is not a RasterLite1-Layer
          // -- ---------------------------------- --
          QString sLayerName = mUriTableName;
          if ( !mUriGeometryColumn.isEmpty() )
          {
            sLayerName = QString( "%1(%2)" ).arg( mUriTableName ).arg( mUriGeometryColumn );
          }
          bool bLoadLayer = true;
          bRc = setDbLayer( getSpatialiteDbInfo()->getQgsSpatialiteDbLayer( sLayerName, bLoadLayer ) );
        }
        else
        {
          QgsDebugMsgLevel( QString( "QgsSpatiaLiteProvider failed: Spatialite Driver is not active Spatialite[%1] LayerName[%2]" ).arg( isDbSpatialiteActive() ).arg( mUriTableName ), 7 );
        }
      }
      else
      {
        if ( isDbValid() )
        {
          if ( !isDbSpatialite() )
          {
            QgsDebugMsgLevel( QString( "QgsSpatiaLiteProvider failed: Database not supported by QgsSpatiaLiteProvider LayerName[%1]" ).arg( mUriTableName ), 7 );
          }
        }
        else
        {
          QgsDebugMsgLevel( QString( "QgsSpatiaLiteProvider failed: Database is invalid LayerName[%1]" ).arg( mUriTableName ), 7 );
        }
      }
    }
    else
    {
      QgsDebugMsgLevel( QString( "QgsSpatiaLiteProvider failed QgsSpatialiteDbInfo invalid LayerName[%1]" ).arg( mUriTableName ), 7 );
    }
  }
  else
  {
    QgsDebugMsgLevel( QString( "QgsSpatiaLiteProvider failed: invalid QgsSqliteHandle[%1] " ).arg( mSqlitePath ), 7 );
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::setDbLayer
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::setDbLayer( QgsSpatialiteDbLayer *dbLayer )
{
  bool bRc = false;
  if ( ( dbLayer ) && ( dbLayer->isLayerValid() ) && ( dbLayer->isLayerSpatialite() ) )
  {
    mDbLayer = dbLayer;
    if ( isDbSpatialiteActive() )
    {
      //-----------------------------------------------------------------
      // Setting needed values from Layer
      //-----------------------------------------------------------------
      mLayerName = getDbLayer()->getLayerName();
      mCopyright = getDbLayer()->getCopyright();
      mAbstract = getDbLayer()->getAbstract();
      mTitle = getDbLayer()->getTitle();
      mLayerType = getDbLayer()->getLayerType();
      mSrid = getDbLayer()->getSrid();
      mGeometryType = getDbLayer()->getGeometryType();
      // Stor a local version based on values returned from Layer
      mUriLayerName = getDbLayer()->getLayerName();
      mUriTableName = getDbLayer()->getTableName();
      mTableName = getDbLayer()->getTableName();
      mGeometryColumn = getDbLayer()->getGeometryColumn();
      mUriGeometryColumn = getDbLayer()->getGeometryColumn();
      mGeometryType = getDbLayer()->getGeometryType();
      mGeometryTypeString = getDbLayer()->getGeometryTypeString();
      mViewTableName = getDbLayer()->getViewTableName();
      mViewTableGeometryColumn = getDbLayer()->getViewTableGeometryColumn();
      mAttributeFields = getDbLayer()->getAttributeFields();
      mSpatialIndexType = getDbLayer()->getSpatialIndexType();
      mPrimaryKey = getDbLayer()->getPrimaryKey();
      mPrimaryKeyCId = getDbLayer()->getPrimaryKeyCId();
      mPrimaryKeyAttrs = getDbLayer()->getPrimaryKeyAttrs();
      mProj4text = getDbLayer()->getProj4text();
      mAuthId = getDbLayer()->getAuthId();

      //-----------------------------------------------------------------
      if ( getLayerType() == QgsSpatialiteDbInfo::SpatialView )
      {
        mIndexTable = mViewTableName;
        mIndexGeometry = getDbLayer()->getViewTableGeometryColumn();
      }
      else
      {
        mIndexTable = mTableName;
        mIndexGeometry = getDbLayer()->getGeometryColumn();;
      }
#if 0
      // If/When QgsLayerMetadata has been added to QgsDataProvider
      // - then this should be done here
      setLayerMetadata( getDbLayer()->getLayerMetadata() );
#endif
      if ( getSpatialIndexType() == QgsSpatialiteDbInfo::SpatialIndexRTree )
      {
        mSpatialIndexRTree = true;
      }
      if ( getSpatialIndexType() == QgsSpatialiteDbInfo::SpatialIndexMbrCache )
      {
        mSpatialIndexMbrCache = true;
      }
      if ( ( getLayerType() == QgsSpatialiteDbInfo::SpatialTable ) ||
           ( getLayerType() == QgsSpatialiteDbInfo::TopologyExport ) )
      {
        mTableBased = true;
      }
      if ( getLayerType() == QgsSpatialiteDbInfo::SpatialView )
      {
        mViewBased = true;
      }
      if ( getLayerType() == QgsSpatialiteDbInfo::VirtualShape )
      {
        mVShapeBased = true;
      }
      //fill type names into sets
      setNativeTypes( QList<NativeType>()
                      << QgsVectorDataProvider::NativeType( tr( "Binary object (BLOB)" ), QStringLiteral( "BLOB" ), QVariant::ByteArray )
                      << QgsVectorDataProvider::NativeType( tr( "Text" ), QStringLiteral( "TEXT" ), QVariant::String )
                      << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), QStringLiteral( "FLOAT" ), QVariant::Double )
                      << QgsVectorDataProvider::NativeType( tr( "Whole number (integer)" ), QStringLiteral( "INTEGER" ), QVariant::LongLong )

                      // date type
                      << QgsVectorDataProvider::NativeType( tr( "Date" ), QStringLiteral( "date" ), QVariant::Date, -1, -1, -1, -1 )
                      << QgsVectorDataProvider::NativeType( tr( "Time" ), QStringLiteral( "time" ), QVariant::Time, -1, -1, -1, -1 )
                      << QgsVectorDataProvider::NativeType( tr( "Date & Time" ), QStringLiteral( "timestamp without time zone" ), QVariant::DateTime, -1, -1, -1, -1 )

                      << QgsVectorDataProvider::NativeType( tr( "Array of text" ), QgsSpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX.toUpper() + "TEXT" + QgsSpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX.toUpper(), QVariant::StringList, 0, 0, 0, 0, QVariant::String )
                      << QgsVectorDataProvider::NativeType( tr( "Array of decimal numbers (double)" ), QgsSpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX.toUpper() + "REAL" + QgsSpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX.toUpper(), QVariant::List, 0, 0, 0, 0, QVariant::Double )
                      << QgsVectorDataProvider::NativeType( tr( "Array of whole numbers (integer)" ), QgsSpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX.toUpper() + "INTEGER" + QgsSpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX.toUpper(), QVariant::List, 0, 0, 0, 0, QVariant::LongLong )
                    );
      // bRc = checkQuery();
    }
    else
    {
      QgsDebugMsgLevel( QString( "QgsSpatiaLiteProvider failed while loading Layer : Spatialite Drivers is not active Spatialite[%1] LayerName[%2]" ).arg( isDbSpatialiteActive() ).arg( getLayerName() ), 7 );
    }
  }
  else
  {
    if ( dbLayer )
    {
      QgsDebugMsgLevel( QString( " QgsSpatiaLiteProvider setting Layer failed: isLayerValid[%1] isLayerSpatialite[%2] LayerName[%3]" ).arg( dbLayer->isLayerValid() ).arg( dbLayer->isLayerSpatialite() ).arg( dbLayer->getLayerName() ), 7 );
    }
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::convertField
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::convertField( QgsField &field )
{
  QString fieldType = QStringLiteral( "TEXT" ); //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();

  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = QStringLiteral( "BIGINT" );
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::DateTime:
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::String:
      fieldType = QStringLiteral( "TEXT" );
      fieldPrec = -1;
      break;

    case QVariant::Int:
      fieldType = QStringLiteral( "INTEGER" );
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::Double:
      if ( fieldSize <= 0 || fieldPrec <= 0 )
      {
        fieldType = QStringLiteral( "REAL" );
        fieldSize = -1;
        fieldPrec = -1;
      }
      else
      {
        fieldType = QStringLiteral( "NUMERIC" );
      }
      break;

    case QVariant::List:
    case QVariant::StringList:
    {
      QgsField subField = field;
      subField.setType( field.subType() );
      subField.setSubType( QVariant::Invalid );
      if ( !convertField( subField ) ) return false;
      fieldType = QgsSpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX + subField.typeName() + QgsSpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX;
      fieldSize = subField.length();
      fieldPrec = subField.precision();
      break;
    }

    default:
      return false;
  }

  field.setTypeName( fieldType );
  field.setLength( fieldSize );
  field.setPrecision( fieldPrec );
  return true;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::createEmptyLayer
//-----------------------------------------------------------------
// Import a vector layer into the database
//-----------------------------------------------------------------
QgsVectorLayerExporter::ExportError
QgsSpatiaLiteProvider::createEmptyLayer( const QString &uri,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> *oldToNewAttrIdxMap,
    QString *errorMessage,
    const QMap<QString, QVariant> *options )
{
  Q_UNUSED( options );

  // populate members from the uri structure

  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QString tableName = dsUri.table();

  QString geometryColumn = dsUri.geometryColumn();
  QString geometryType;

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  QgsDebugMsgLevel( QString( "Database is: %1 \nTable name is %2 Geometry column is: %3" ).arg( sqlitePath ).arg( tableName ).arg( geometryColumn ), 3 );

  // create the table
  {
    char *errMsg = nullptr;
    int toCommit = false;
    QString sql;

    // trying to open the SQLite DB
    QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
    if ( !handle )
    {
      QgsDebugMsgLevel( QString( "Connection to database failed. Import of layer aborted." ), 3 );
      if ( errorMessage )
        *errorMessage = QObject::tr( "Connection to database failed" );
      return QgsVectorLayerExporter::ErrConnectionFailed;
    }

    sqlite3 *sqliteHandle = handle->handle();

    // get the pk's name and type
    if ( primaryKey.isEmpty() )
    {
      // if no pk name was passed, define the new pk field name
      int index = 0;
      QString pk = primaryKey = QStringLiteral( "pk" );
      for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
      {
        if ( fields.at( fldIdx ).name() == primaryKey )
        {
          // it already exists, try again with a new name
          primaryKey = QStringLiteral( "%1_%2" ).arg( pk ).arg( index++ );
          fldIdx = -1; // it is incremented in the for loop, i.e. restarts at 0
        }
      }
    }
    else
    {
      // search for the passed field
      for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
      {
        if ( fields.at( fldIdx ).name() == primaryKey )
        {
          // found it, get the field type
          QgsField fld = fields.at( fldIdx );
          if ( convertField( fld ) )
          {
            primaryKeyType = fld.typeName();
          }
        }
      }
    }

    // if the pk field doesn't exist yet, create an integer pk field
    // as it's autoincremental
    if ( primaryKeyType.isEmpty() )
    {
      primaryKeyType = QStringLiteral( "INTEGER" );
    }
    else
    {
      // if the pk field's type is bigint, use the autoincremental
      // integer type instead
      if ( primaryKeyType == QLatin1String( "BIGINT" ) )
      {
        primaryKeyType = QStringLiteral( "INTEGER" );
      }
    }

    try
    {
      int ret = sqlite3_exec( sqliteHandle, "BEGIN", nullptr, nullptr, &errMsg );
      if ( ret != SQLITE_OK )
        throw QgsSpatiaLiteUtils::SLException( errMsg );

      toCommit = true;

      if ( overwrite )
      {
        // delete the table if exists and the related entry in geometry_columns, then re-create it
        sql = QStringLiteral( "DROP TABLE IF EXISTS %1" )
              .arg( QgsSpatiaLiteUtils::quotedIdentifier( tableName ) );

        ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
        if ( ret != SQLITE_OK )
          throw QgsSpatiaLiteUtils::SLException( errMsg );

        sql = QStringLiteral( "DELETE FROM geometry_columns WHERE upper(f_table_name) = upper(%1)" )
              .arg( QgsSpatiaLiteUtils::quotedValue( tableName ) );

        ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
        if ( ret != SQLITE_OK )
          throw QgsSpatiaLiteUtils::SLException( errMsg );
      }

      sql = QStringLiteral( "CREATE TABLE %1 (%2 %3 PRIMARY KEY%4)" )
            .arg( quotedIdentifier( tableName ),
                  quotedIdentifier( primaryKey ),
                  primaryKeyType,
                  primaryKeyType == QLatin1String( "INTEGER" ) ? QStringLiteral( " AUTOINCREMENT" ) : QString() );

      ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
      if ( ret != SQLITE_OK )
        throw QgsSpatiaLiteUtils::SLException( errMsg );

      // get geometry type, dim and srid
      int dim = 2;
      long srid = srs.postgisSrid();

      switch ( wkbType )
      {
        case QgsWkbTypes::Point25D:
          dim = 3;
          FALLTHROUGH;
        case QgsWkbTypes::Point:
          geometryType = QStringLiteral( "POINT" );
          break;

        case QgsWkbTypes::LineString25D:
          dim = 3;
          FALLTHROUGH;
        case QgsWkbTypes::LineString:
          geometryType = QStringLiteral( "LINESTRING" );
          break;

        case QgsWkbTypes::Polygon25D:
          dim = 3;
          FALLTHROUGH;
        case QgsWkbTypes::Polygon:
          geometryType = QStringLiteral( "POLYGON" );
          break;

        case QgsWkbTypes::MultiPoint25D:
          dim = 3;
          FALLTHROUGH;
        case QgsWkbTypes::MultiPoint:
          geometryType = QStringLiteral( "MULTIPOINT" );
          break;

        case QgsWkbTypes::MultiLineString25D:
          dim = 3;
          FALLTHROUGH;
        case QgsWkbTypes::MultiLineString:
          geometryType = QStringLiteral( "MULTILINESTRING" );
          break;

        case QgsWkbTypes::MultiPolygon25D:
          dim = 3;
          FALLTHROUGH;
        case QgsWkbTypes::MultiPolygon:
          geometryType = QStringLiteral( "MULTIPOLYGON" );
          break;

        case QgsWkbTypes::Unknown:
          geometryType = QStringLiteral( "GEOMETRY" );
          break;

        case QgsWkbTypes::NoGeometry:
        default:
          dim = 0;
          break;
      }

      // create geometry column
      if ( !geometryType.isEmpty() )
      {
        sql = QStringLiteral( "SELECT AddGeometryColumn(%1, %2, %3, %4, %5)" )
              .arg( QgsSpatiaLiteUtils::quotedValue( tableName ),
                    QgsSpatiaLiteUtils::quotedValue( geometryColumn ) )
              .arg( srid )
              .arg( QgsSpatiaLiteUtils::quotedValue( geometryType ) )
              .arg( dim );

        ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
        if ( ret != SQLITE_OK )
          throw QgsSpatiaLiteUtils::SLException( errMsg );
      }
      else
      {
        geometryColumn = QString();
      }

      ret = sqlite3_exec( sqliteHandle, "COMMIT", nullptr, nullptr, &errMsg );
      if ( ret != SQLITE_OK )
        throw QgsSpatiaLiteUtils::SLException( errMsg );

    }
    catch ( QgsSpatiaLiteUtils::SLException &e )
    {
      QgsDebugMsgLevel( QString( "creation of data source %1 failed. %2" ).arg( tableName ).arg( e.errorMessage() ), 3 );

      if ( errorMessage )
        *errorMessage = QObject::tr( "creation of data source %1 failed. %2" )
                        .arg( tableName,
                              e.errorMessage() );


      if ( toCommit )
      {
        // ROLLBACK after some previous error
        sqlite3_exec( sqliteHandle, "ROLLBACK", nullptr, nullptr, nullptr );
      }

      QgsSqliteHandle::closeDb( handle );
      return QgsVectorLayerExporter::ErrCreateLayer;
    }

    QgsSqliteHandle::closeDb( handle );
    QgsDebugMsgLevel( QString( "layer %1 created." ).arg( tableName ), 3 );
  }

  // use the provider to edit the table
  dsUri.setDataSource( QLatin1String( "" ), tableName, geometryColumn, QString(), primaryKey );
  QgsSpatiaLiteProvider *provider = new QgsSpatiaLiteProvider( dsUri.uri() );
  if ( !provider->isValid() )
  {
    QgsDebugMsgLevel( QString( "The layer %1 just created is not valid or not supported by the provider." ).arg( tableName ), 3 );
    if ( errorMessage )
      *errorMessage = QObject::tr( "loading of the layer %1 failed" )
                      .arg( tableName );

    delete provider;
    return QgsVectorLayerExporter::ErrInvalidLayer;
  }

  QgsDebugMsgLevel( QString( "layer loaded" ), 3 );

  // add fields to the layer
  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();

  if ( fields.size() > 0 )
  {
    int offset = 1;

    // get the list of fields
    QList<QgsField> flist;
    for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
    {
      QgsField fld = fields.at( fldIdx );
      if ( fld.name() == primaryKey )
        continue;

      if ( fld.name() == geometryColumn )
      {
        QgsDebugMsgLevel( QString( "Found a field with the same name of the geometry column. Skip it!" ), 3 );
        continue;
      }

      if ( !convertField( fld ) )
      {
        QgsDebugMsgLevel( QString( "error creating field %1: unsupported type" ).arg( fld.name() ), 3 );
        if ( errorMessage )
          *errorMessage = QObject::tr( "unsupported type for field %1" )
                          .arg( fld.name() );

        delete provider;
        return QgsVectorLayerExporter::ErrAttributeTypeUnsupported;
      }

      QgsDebugMsgLevel( QString( "creating field # %1  -> # %2 name  %3 type %4 typename %5 width %6 precision %7" ).arg( QString::number( fldIdx ) ).arg( QString::number( offset ) )
                        .arg( fld.name() ).arg( QString( QVariant::typeToName( fld.type() ) ) ).arg( fld.typeName() ).arg( QString::number( fld.length() ) ).arg( QString::number( fld.precision() ) ), 3 );

      flist.append( fld );
      if ( oldToNewAttrIdxMap )
        oldToNewAttrIdxMap->insert( fldIdx, offset );

      offset++;
    }

    if ( !provider->addAttributes( flist ) )
    {
      QgsDebugMsgLevel( QString( "error creating fields " ), 3 );
      if ( errorMessage )
        *errorMessage = QObject::tr( "creation of fields failed" );

      delete provider;
      return QgsVectorLayerExporter::ErrAttributeCreationFailed;
    }

    QgsDebugMsgLevel( QString( "Done creating fields" ), 3 );
  }
  return QgsVectorLayerExporter::NoError;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::featureSource
//-----------------------------------------------------------------
QgsAbstractFeatureSource *QgsSpatiaLiteProvider::featureSource() const
{
  return new QgsSpatiaLiteFeatureSource( this );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::storageType
//-----------------------------------------------------------------
QString QgsSpatiaLiteProvider::storageType() const
{
  return QStringLiteral( "SQLite database with SpatiaLite [%1] container[%2]" ).arg( dbSpatialiteVersionInfo() ).arg( dbSpatialMetadataString() );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::getFeatures
//-----------------------------------------------------------------
QgsFeatureIterator QgsSpatiaLiteProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  if ( !isValid() )
  {
    QgsDebugMsgLevel( QString( "Read attempt on an invalid SpatiaLite data source" ), 3 );
    return QgsFeatureIterator();
  }
  return QgsFeatureIterator( new QgsSpatiaLiteFeatureIterator( new QgsSpatiaLiteFeatureSource( this ), true, request ) );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::subsetString
//-----------------------------------------------------------------
QString QgsSpatiaLiteProvider::subsetString() const
{
  return mSubsetString;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::setSubsetString
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
{
  QString prevSubsetString = mSubsetString;
  mSubsetString = theSQL;
  // update URI
  QgsDataSourceUri uri = QgsDataSourceUri( dataSourceUri() );
  uri.setSql( mSubsetString );
  setDataSourceUri( uri.uri() );
  // update feature count and extents
  if ( updateFeatureCount )
  {
    getLayerExtent( true, true );
    return true;
  }
  mSubsetString = prevSubsetString;
  // restore URI
  uri = QgsDataSourceUri( dataSourceUri() );
  uri.setSql( mSubsetString );
  setDataSourceUri( uri.uri() );
  getLayerExtent( true, true );
  emit dataChanged();
  return false;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::extent
//-----------------------------------------------------------------
QgsRectangle QgsSpatiaLiteProvider::extent() const
{
  return getLayerExtent( false, false );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::updateExtents
//-----------------------------------------------------------------
void QgsSpatiaLiteProvider::updateExtents()
{
  getLayerExtent( true, true );
}
size_t QgsSpatiaLiteProvider::layerCount() const
{
  return 1;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::wkbType
//-----------------------------------------------------------------
//  Return the feature type
//-----------------------------------------------------------------
QgsWkbTypes::Type QgsSpatiaLiteProvider::wkbType() const
{
  return getGeometryType() ;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::featureCount
//-----------------------------------------------------------------
//  Return the feature count
//-----------------------------------------------------------------
long QgsSpatiaLiteProvider::featureCount() const
{
  return getNumberFeatures( true );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::crs
//-----------------------------------------------------------------
QgsCoordinateReferenceSystem QgsSpatiaLiteProvider::crs() const
{
  QgsCoordinateReferenceSystem srs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( getAuthId() );
  if ( !srs.isValid() )
  {
    srs = QgsCoordinateReferenceSystem::fromProj4( getProj4text() );
    //TODO: createFromProj4 used to save to the user database any new CRS
    // this behavior was changed in order to separate creation and saving.
    // Not sure if it necessary to save it here, should be checked by someone
    // familiar with the code (should also give a more descriptive name to the generated CRS)
    if ( srs.srsid() == 0 )
    {
      QString myName = QStringLiteral( " * %1 (%2)" )
                       .arg( QObject::tr( "Generated CRS", "A CRS automatically generated from layer info get this prefix for description" ),
                             srs.toProj4() );
      srs.saveAsUserCrs( myName );
    }
  }
  return srs;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::minimumValue
//-----------------------------------------------------------------
//  QgsSpatialiteDbLayer should never store the
// - mQuery and mSubsetString members
// --> since other source may be using the Layer
// Functions using these 'filters' must remain in QgsSpatiaLiteProvider
//-----------------------------------------------------------------
// Returns the minimum value of an attribute
// - uses mQuery and mSubsetString
//-----------------------------------------------------------------
QVariant QgsSpatiaLiteProvider::minimumValue( int index ) const
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  QString minValue;
  QString sql;
  try
  {
    // get the field name
    QgsField fld = field( index );
    sql = QStringLiteral( "SELECT Min(%1) FROM %2" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ), mQuery );
    if ( !mSubsetString.isEmpty() )
    {
      sql += " WHERE ( " + mSubsetString + ')';
    }
    ret = sqlite3_get_table( dbSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errMsg ? errMsg : tr( "unknown cause" ) ), tr( "SpatiaLite" ) );
      // unexpected error
      if ( errMsg )
      {
        sqlite3_free( errMsg );
      }
      minValue = QString();
    }
    else
    {
      if ( rows < 1 )
        ;
      else
      {
        for ( i = 1; i <= rows; i++ )
        {
          minValue = results[( i * columns ) + 0];
        }
      }
      sqlite3_free_table( results );

      if ( minValue.isEmpty() )
      {
        // NULL or not found
        minValue = QString();
      }
    }
    return convertValue( fld.type(), minValue );
  }
  catch ( QgsSpatiaLiteUtils::SLFieldNotFound )
  {
    return QVariant( QVariant::Int );
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::maximumValue
//-----------------------------------------------------------------
// Returns the maximum value of an attribute
// - uses mQuery and mSubsetString
//-----------------------------------------------------------------
QVariant QgsSpatiaLiteProvider::maximumValue( int index ) const
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  QString maxValue;
  QString sql;
  try
  {
    // get the field name
    QgsField fld = field( index );
    sql = QStringLiteral( "SELECT Max(%1) FROM %2" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ), mQuery );
    if ( !mSubsetString.isEmpty() )
    {
      sql += " WHERE ( " + mSubsetString + ')';
    }
    ret = sqlite3_get_table( dbSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errMsg ? errMsg : tr( "unknown cause" ) ), tr( "SpatiaLite" ) );
      // unexpected error
      if ( errMsg )
      {
        sqlite3_free( errMsg );
      }
      maxValue = QString();
    }
    else
    {
      if ( rows < 1 )
        ;
      else
      {
        for ( i = 1; i <= rows; i++ )
        {
          maxValue = results[( i * columns ) + 0];
        }
      }
      sqlite3_free_table( results );
      if ( maxValue.isEmpty() )
      {
        // NULL or not found
        maxValue = QString();
      }
    }
    return convertValue( fld.type(), maxValue );
  }
  catch ( QgsSpatiaLiteUtils::SLFieldNotFound )
  {
    return QVariant( QVariant::Int );
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::uniqueValues
//-----------------------------------------------------------------
// Returns the list of unique values of an attribute
// - uses mQuery and mSubsetString
//-----------------------------------------------------------------
QSet<QVariant> QgsSpatiaLiteProvider::uniqueValues( int index, int limit ) const
{
  sqlite3_stmt *stmt = nullptr;
  QString sql;
  QSet<QVariant> uniqueValues;
  // get the field name
  if ( index < 0 || index >= getAttributeFields().count() )
  {
    return uniqueValues; //invalid field
  }
  QgsField fld = getAttributeFields().at( index );
  sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ), mQuery );
  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE ( " + mSubsetString + ')';
  }
  sql += QStringLiteral( " ORDER BY %1" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ) );
  if ( limit >= 0 )
  {
    sql += QStringLiteral( " LIMIT %1" ).arg( limit );
  }
  // SQLite prepared statement
  if ( sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( dbSqliteHandle() ) ), tr( "SpatiaLite" ) );
    return uniqueValues;
  }
  while ( 1 )
  {
    // this one is an infinitive loop, intended to fetch any row
    int ret = sqlite3_step( stmt );
    if ( ret == SQLITE_DONE )
    {
      // there are no more rows to fetch - we can stop looping
      break;
    }
    if ( ret == SQLITE_ROW )
    {
      // fetching one column value
      switch ( sqlite3_column_type( stmt, 0 ) )
      {
        case SQLITE_INTEGER:
          uniqueValues.insert( QVariant( sqlite3_column_int( stmt, 0 ) ) );
          break;
        case SQLITE_FLOAT:
          uniqueValues.insert( QVariant( sqlite3_column_double( stmt, 0 ) ) );
          break;
        case SQLITE_TEXT:
          uniqueValues.insert( QVariant( QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) ) ) );
          break;
        default:
          uniqueValues.insert( QVariant( getAttributeFields().at( index ).type() ) );
          break;
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( dbSqliteHandle() ) ), tr( "SpatiaLite" ) );
      sqlite3_finalize( stmt );
      return uniqueValues;
    }
  }
  sqlite3_finalize( stmt );

  return uniqueValues;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::uniqueStringsMatching
//-----------------------------------------------------------------
// Returns the list of uniqueStringsMatching
// - uses mQuery and mSubsetString
//-----------------------------------------------------------------
QStringList QgsSpatiaLiteProvider::uniqueStringsMatching( int index, const QString &substring, int limit, QgsFeedback *feedback ) const
{
  QStringList results;
  sqlite3_stmt *stmt = nullptr;
  QString sql;
  // get the field name
  if ( index < 0 || index >= getAttributeFields().count() )
  {
    return results; //invalid field
  }
  QgsField fld = getAttributeFields().at( index );
  sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2 " ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ), mQuery );
  sql += QStringLiteral( " WHERE " ) + QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ) + QStringLiteral( " LIKE '%" ) + substring + QStringLiteral( "%'" );
  if ( !mSubsetString.isEmpty() )
  {
    sql += QStringLiteral( " AND ( " ) + mSubsetString + ')';
  }
  sql += QStringLiteral( " ORDER BY %1" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ) );
  if ( limit >= 0 )
  {
    sql += QStringLiteral( " LIMIT %1" ).arg( limit );
  }
  // SQLite prepared statement
  if ( sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( dbSqliteHandle() ) ), tr( "SpatiaLite" ) );
    return results;
  }
  while ( ( limit < 0 || results.size() < limit ) && ( !feedback || !feedback->isCanceled() ) )
  {
    // this one is an infinitive loop, intended to fetch any row
    int ret = sqlite3_step( stmt );
    if ( ret == SQLITE_DONE )
    {
      // there are no more rows to fetch - we can stop looping
      break;
    }
    if ( ret == SQLITE_ROW )
    {
      // fetching one column value
      switch ( sqlite3_column_type( stmt, 0 ) )
      {
        case SQLITE_TEXT:
          results.append( QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) ) );
          break;
        default:
          break;
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( dbSqliteHandle() ) ), tr( "SpatiaLite" ) );
      sqlite3_finalize( stmt );
      return results;
    }
  }
  sqlite3_finalize( stmt );
  return results;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::addFeatures
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  QString errorMessage;
  if ( !getDbLayer()->addLayerFeatures( flist, flags, errorMessage ) )
  {
    pushError( errorMessage );
    return false;
  }
  return true;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::createAttributeIndex
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::createAttributeIndex( int field )
{
  return getDbLayer()->createLayerAttributeIndex( field );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::deleteFeatures
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::deleteFeatures( const QgsFeatureIds &id )
{
  QString errorMessage;
  if ( !getDbLayer()->deleteLayerFeatures( id, errorMessage ) )
  {
    pushError( errorMessage );
    return false;
  }
  return true;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::truncate
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::truncate()
{
  return getDbLayer()->truncateLayerTableRows();
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::addAttributes
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::addAttributes( const QList<QgsField> &attributes )
{
  return getDbLayer()->addLayerAttributes( attributes );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::changeAttributeValues
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  return getDbLayer()->changeLayerAttributeValues( attr_map );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::changeGeometryValues
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  return getDbLayer()->changeLayerGeometryValues( geometry_map );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::capabilities
//-----------------------------------------------------------------
QgsVectorDataProvider::Capabilities QgsSpatiaLiteProvider::capabilities() const
{
  return getCapabilities();
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::defaultValue
//-----------------------------------------------------------------
QVariant QgsSpatiaLiteProvider::defaultValue( int fieldId ) const
{
  return getDefaultValues().value( fieldId, QVariant() );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::checkQuery
//-----------------------------------------------------------------
// Check the validaty of a possible sugstring query
// - not documented, so I am guessing
//-----------------------------------------------------------------
bool QgsSpatiaLiteProvider::checkQuery()
{
  int count = 0;
  mIsQuery = false;
  if ( mQuery.startsWith( '(' ) && mQuery.endsWith( ')' ) )
  {
    QString sql;
    // checking if this one is a select query
    int ret;
    char **results = nullptr;
    char *errMsg = nullptr;
    int rows, columns;
    // get a new alias for the subquery
    int index = 0;
    QString alias;
    QRegExp regex;
    do
    {
      alias = QStringLiteral( "subQuery_%1" ).arg( QString::number( index++ ) );
      QString pattern = QStringLiteral( "(\\\"?)%1\\1" ).arg( QRegExp::escape( alias ) );
      regex.setPattern( pattern );
      regex.setCaseSensitivity( Qt::CaseInsensitive );
    }
    while ( mQuery.contains( regex ) );
    // convert the custom query into a subquery
    mQuery = QStringLiteral( "%1 as %2" ).arg( mQuery, QgsSpatiaLiteUtils::quotedIdentifier( alias ) );
    sql = QStringLiteral( "SELECT 0 FROM %1 LIMIT 1" ).arg( mQuery );
    ret = sqlite3_get_table( dbSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret == SQLITE_OK && rows == 1 )
    {
      mIsQuery = true;
      mReadOnly = true;
      count++;
    }
    sqlite3_free_table( results );
  }
  if ( !mIsQuery )
  {
    mQuery = QgsSpatiaLiteUtils::quotedIdentifier( getTableName() );
  }
  // checking for validity
  return count == 1;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::field
//-----------------------------------------------------------------
QgsField QgsSpatiaLiteProvider::field( int index ) const
{
  return getDbLayer()->getAttributeField( index );
}
//-----------------------------------------------------------------
// QGISEXTERN  createEmptyLayer
//-----------------------------------------------------------------
QGISEXTERN QgsVectorLayerExporter::ExportError createEmptyLayer(
  const QString &uri,
  const QgsFields &fields,
  QgsWkbTypes::Type wkbType,
  const QgsCoordinateReferenceSystem &srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  return QgsSpatiaLiteProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           oldToNewAttrIdxMap, errorMessage, options
         );
}
//-----------------------------------------------------------------
// QGISEXTERN  createDb
//-----------------------------------------------------------------
// TODO: determine if these functions should be moved to QgsSpatiaLiteUtils
// - not documented:
// used  for 'QgsSLRootItem::createDatabase()' and
// and for 'QgsNewSpatialiteLayerDialog::createDb'
//-----------------------------------------------------------------
// this function now calls: QgsSpatiaLiteUtils::createSpatialDatabase
//-----------------------------------------------------------------
QGISEXTERN bool createDb( const QString &dbPath, QString &errCause )
{
  bool bRc = false;
  QgsDebugMsgLevel( QString( "creating a new db" ), 3 );

  QFileInfo fullPath = QFileInfo( dbPath );
  QDir path = fullPath.dir();
  QgsDebugMsgLevel( QString( "making this dir: %1" ).arg( path.absolutePath() ), 3 );

  // Must be sure there is destination directory ~/.qgis
  QDir().mkpath( path.absolutePath() );
  QString sDatabaseFileName = dbPath;
  QgsSpatialiteDbInfo::SpatialMetadata dbCreateOption = QgsSpatialiteDbInfo::Spatialite50;
  bRc = QgsSpatiaLiteUtils::createSpatialDatabase( sDatabaseFileName, errCause, dbCreateOption );
  return bRc;
}
//-----------------------------------------------------------------
// QGISEXTERN  deleteLayer
//-----------------------------------------------------------------
QGISEXTERN bool deleteLayer( const QString &dbPath, const QString &tableName, QString &errCause )
{
  bool bRc = false;

  bool bLoadLayers = false;
  bool bShared = true;
  QgsSpatialiteDbInfo *spatialiteDbInfo = QgsSpatiaLiteUtils::CreateQgsSpatialiteDbInfo( dbPath, bLoadLayers, bShared );
  if ( spatialiteDbInfo )
  {
    // TODO: remove after testing:  cp middle_earth.3035.RasterLite2.save.db middle_earth.3035.RasterLite2.db
    // QgsSpatiaLiteProvider] deleting layer [middle_earth_farthings] LoadedLayers[0] Uuid[{65fec2db-09f2-4daf-98d1-57fa6f110056}]
    QgsDebugMsgLevel( QString( "[QgsSpatiaLiteProvider] deleting layer [%1] LoadedLayers[%2] Uuid[%3]" ).arg( tableName ).arg( spatialiteDbInfo->dbLoadedLayersCount() ).arg( spatialiteDbInfo->getConnectionUuid() ), 3 );
    // The file exists, is a Spatialite-Database and a connection has been made.
    if ( spatialiteDbInfo->isDbSpatialite() )
    {
      bRc = spatialiteDbInfo->dropGeoTable( tableName, errCause );
    }
    QgsDebugMsgLevel( QString( "[QgsSpatiaLiteProvider] deleting layer [%1] result[%2] LoadedLayers[%3]" ).arg( tableName ).arg( errCause ).arg( spatialiteDbInfo->dbLoadedLayersCount() ), 3 );
    if ( !spatialiteDbInfo->checkConnectionNeeded() )
    {
      // Delete only if not being used elsewhere, Connection will be closed
      delete spatialiteDbInfo;
    }
    spatialiteDbInfo = nullptr;
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::pkAttributeIndexes
//-----------------------------------------------------------------
QgsAttributeList QgsSpatiaLiteProvider::pkAttributeIndexes() const
{
  return getPrimaryKeyAttrs();
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::searchLayers
//-----------------------------------------------------------------
QList<QgsVectorLayer *> QgsSpatiaLiteProvider::searchLayers( const QList<QgsVectorLayer *> &layers, const QString &connectionInfo, const QString &tableName )
{
  QList<QgsVectorLayer *> result;
  Q_FOREACH ( QgsVectorLayer *layer, layers )
  {
    const QgsSpatiaLiteProvider *slProvider = qobject_cast<QgsSpatiaLiteProvider *>( layer->dataProvider() );
    if ( slProvider && slProvider->mSqlitePath == connectionInfo && slProvider->getTableName() == tableName )
    {
      result.append( layer );
    }
  }
  return result;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider::discoverRelations
//-----------------------------------------------------------------
QList<QgsRelation> QgsSpatiaLiteProvider::discoverRelations( const QgsVectorLayer *self, const QList<QgsVectorLayer *> &layers ) const
{
  QList<QgsRelation> output;
  const QString sql = QStringLiteral( "PRAGMA foreign_key_list(%1)" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( getTableName() ) );
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( dbSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret == SQLITE_OK )
  {
    int nbFound = 0;
    for ( int row = 1; row <= rows; ++row )
    {
      const QString name = "fk_" + getTableName() + "_" + QString::fromUtf8( results[row * columns + 0] );
      const QString position = QString::fromUtf8( results[row * columns + 1] );
      const QString refTable = QString::fromUtf8( results[row * columns + 2] );
      const QString fkColumn = QString::fromUtf8( results[row * columns + 3] );
      const QString refColumn = QString::fromUtf8( results[row * columns + 4] );
      if ( position == QLatin1String( "0" ) )
      {
        // first reference field => try to find if we have layers for the referenced table
        const QList<QgsVectorLayer *> foundLayers = searchLayers( layers, mSqlitePath, refTable );
        Q_FOREACH ( const QgsVectorLayer *foundLayer, foundLayers )
        {
          QgsRelation relation;
          relation.setName( name );
          relation.setReferencingLayer( self->id() );
          relation.setReferencedLayer( foundLayer->id() );
          relation.addFieldPair( fkColumn, refColumn );
          relation.generateId();
          if ( relation.isValid() )
          {
            output.append( relation );
            ++nbFound;
          }
          else
          {
            QgsLogger::warning( "Invalid relation for " + name );
          }
        }
      }
      else
      {
        // multi reference field => add the field pair to all the referenced layers found
        for ( int i = 0; i < nbFound; ++i )
        {
          output[output.size() - 1 - i].addFieldPair( fkColumn, refColumn );
        }
      }
    }
    sqlite3_free_table( results );
  }
  else
  {
    QgsLogger::warning( QStringLiteral( "SQLite error discovering relations: %1" ).arg( errMsg ) );
    sqlite3_free( errMsg );
  }
  return output;
}
//-----------------------------------------------------------------
// QGISEXTERN saveStyle
//-----------------------------------------------------------------
QGISEXTERN bool saveStyle( const QString &uri, const QString &qmlStyle, const QString &sldStyle,
                           const QString &styleName, const QString &styleDescription,
                           const QString &uiFileContent, bool useAsDefault, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QgsDebugMsgLevel( QString( "Database is: %1" ).arg( sqlitePath ), 3 );
  // Avoid sql-statement errors
  if ( dsUri.schema().isEmpty() )
    return false;

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugMsgLevel( QString( "Connection to database failed. Save style aborted." ), 3 );
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  sqlite3 *sqliteHandle = handle->handle();

  // check if layer_styles table already exist
  QString countIfExist = QStringLiteral( "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1';" ).arg( QStringLiteral( "layer_styles" ) );

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle, countIfExist.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( countIfExist ) );
    errCause = QObject::tr( "Error looking for style. The query was logged" );
    return false;
  }

  // if not table exist... create is
  int howMany = 0;
  if ( 1 == rows )
  {
    howMany = atoi( results[( rows * columns ) + 0 ] );
  }
  sqlite3_free_table( results );

  // create table if not exist
  if ( 0 == howMany )
  {

    QString createQuery = QString( "CREATE TABLE layer_styles("
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT"
                                   ",f_table_catalog varchar(256)"
                                   ",f_table_schema varchar(256)"
                                   ",f_table_name varchar(256)"
                                   ",f_geometry_column varchar(256)"
                                   ",styleName text"
                                   ",styleQML text"
                                   ",styleSLD text"
                                   ",useAsDefault boolean"
                                   ",description text"
                                   ",owner varchar(30)"
                                   ",ui text"
                                   ",update_time timestamp DEFAULT CURRENT_TIMESTAMP"
                                   ")" );
    ret = sqlite3_exec( sqliteHandle, createQuery.toUtf8().constData(), nullptr, nullptr, &errMsg );
    if ( SQLITE_OK != ret )
    {
      QgsSqliteHandle::closeDb( handle );
      errCause = QObject::tr( "Unable to save layer style. It's not possible to create the destination table on the database." );
      return false;
    }
  }

  QString uiFileColumn;
  QString uiFileValue;
  if ( !uiFileContent.isEmpty() )
  {
    uiFileColumn = QStringLiteral( ",ui" );
    uiFileValue = QStringLiteral( ",%1" ).arg( QgsSpatiaLiteUtils::quotedValue( uiFileContent ) );
  }

  QString sql = QString( "INSERT INTO layer_styles("
                         "f_table_catalog,f_table_schema,f_table_name,f_geometry_column,styleName,styleQML,styleSLD,useAsDefault,description,owner%11"
                         ") VALUES ("
                         "%1,%2,%3,%4,%5,%6,%7,%8,%9,%10%12"
                         ")" )
                .arg( QgsSpatiaLiteUtils::quotedValue( QString() ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( qmlStyle ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( sldStyle ) )
                .arg( useAsDefault ? "1" : "0" )
                .arg( QgsSpatiaLiteUtils::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.username() ) )
                .arg( uiFileColumn )
                .arg( uiFileValue );

  QString checkQuery = QString( "SELECT styleName"
                                " FROM layer_styles"
                                " WHERE f_table_schema=%1"
                                " AND f_table_name=%2"
                                " AND f_geometry_column=%3"
                                " AND styleName=%4" )
                       .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
                       .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
                       .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) )
                       .arg( QgsSpatiaLiteUtils::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) );

  ret = sqlite3_get_table( sqliteHandle, checkQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( checkQuery ) );
    errCause = QObject::tr( "Error looking for style. The query was logged" );
    return false;
  }

  if ( 0 != rows )
  {
    sqlite3_free_table( results );
    if ( QMessageBox::question( nullptr, QObject::tr( "Save style in database" ),
                                QObject::tr( "A style named \"%1\" already exists in the database for this layer. Do you want to overwrite it?" )
                                .arg( styleName.isEmpty() ? dsUri.table() : styleName ),
                                QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
    {
      QgsSqliteHandle::closeDb( handle );
      errCause = QObject::tr( "Operation aborted" );
      return false;
    }

    sql = QString( "UPDATE layer_styles"
                   " SET useAsDefault=%1"
                   ",styleQML=%2"
                   ",styleSLD=%3"
                   ",description=%4"
                   ",owner=%5"
                   " WHERE f_table_schema=%6"
                   " AND f_table_name=%7"
                   " AND f_geometry_column=%8"
                   " AND styleName=%9" )
          .arg( useAsDefault ? "1" : "0" )
          .arg( QgsSpatiaLiteUtils::quotedValue( qmlStyle ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( sldStyle ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.username() ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) );
  }

  if ( useAsDefault )
  {
    QString removeDefaultSql = QString( "UPDATE layer_styles"
                                        " SET useAsDefault=0"
                                        " WHERE f_table_schema=%1"
                                        " AND f_table_name=%2"
                                        " AND f_geometry_column=%3" )
                               .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
                               .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
                               .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) );
    sql = QStringLiteral( "BEGIN; %1; %2; COMMIT;" ).arg( removeDefaultSql, sql );
  }

  ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( sql ) );
    errCause = QObject::tr( "Error looking for style. The query was logged" );
    return false;
  }

  if ( errMsg )
    sqlite3_free( errMsg );

  QgsSqliteHandle::closeDb( handle );
  return true;
}
//-----------------------------------------------------------------
// QGISEXTERN loadStyle
//-----------------------------------------------------------------
QGISEXTERN QString loadStyle( const QString &uri, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QgsDebugMsgLevel( QString( "Database is: %1" ).arg( sqlitePath ), 3 );
  // Avoid sql-statement errors
  if ( dsUri.schema().isEmpty() )
  {
    QgsDebugMsgLevel( QString( "Retrieving style failed. Load style aborted." ), 3 );
    errCause = QObject::tr( "Missing table_schema " );
    return QString();
  }

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugMsgLevel( QString( "Connection to database failed. Load style aborted." ), 3 );
    errCause = QObject::tr( "Connection to database failed" );
    return QString();
  }

  sqlite3 *sqliteHandle = handle->handle();

  QString selectQmlQuery = QString( "SELECT styleQML"
                                    " FROM layer_styles"
                                    " WHERE f_table_schema=%1"
                                    " AND f_table_name=%2"
                                    " AND f_geometry_column=%3"
                                    " ORDER BY CASE WHEN useAsDefault THEN 1 ELSE 2 END"
                                    ",update_time DESC LIMIT 1" )
                           .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
                           .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
                           .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) );

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle, selectQmlQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectQmlQuery ) );
    errCause = QObject::tr( "Error executing loading style. The query was logged" );
    return QLatin1String( "" );
  }

  QString style = ( rows == 1 ) ? QString::fromUtf8( results[( rows * columns ) + 0 ] ) : QLatin1String( "" );
  sqlite3_free_table( results );

  QgsSqliteHandle::closeDb( handle );
  return style;
}
//-----------------------------------------------------------------
// QGISEXTERN listStyles
//-----------------------------------------------------------------
QGISEXTERN int listStyles( const QString &uri, QStringList &ids, QStringList &names,
                           QStringList &descriptions, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QgsDebugMsgLevel( QString( "Database is: %1" ).arg( sqlitePath ), 3 );

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugMsgLevel( QString( "Connection to database failed. Save style aborted." ), 3 );
    errCause = QObject::tr( "Connection to database failed" );
    return -1;
  }

  sqlite3 *sqliteHandle = handle->handle();

  // check if layer_styles table already exist
  QString countIfExist = QStringLiteral( "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1';" ).arg( QStringLiteral( "layer_styles" ) );

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle, countIfExist.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( countIfExist ) );
    errCause = QObject::tr( "Error looking for style. The query was logged" );
    return -1;
  }

  int howMany = 0;
  if ( 1 == rows )
  {
    howMany = atoi( results[( rows * columns ) + 0 ] );
  }
  sqlite3_free_table( results );

  if ( 0 == howMany )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "No styles available on DB" ) );
    errCause = QObject::tr( "No styles available on DB" );
    return false;
  }

  // get them
  QString selectRelatedQuery = QString( "SELECT id,styleName,description"
                                        " FROM layer_styles"
                                        " WHERE f_table_schema=%1"
                                        " AND f_table_name=%2"
                                        " AND f_geometry_column=%3" )
                               .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
                               .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
                               .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) );

  ret = sqlite3_get_table( sqliteHandle, selectRelatedQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectRelatedQuery ) );
    errCause = QObject::tr( "Error loading styles. The query was logged" );
    return -1;
  }

  int numberOfRelatedStyles = rows;
  for ( int i = 1; i <= rows; i++ )
  {
    ids.append( results[( i * columns ) + 0 ] );
    names.append( QString::fromUtf8( results[( i * columns ) + 1 ] ) );
    descriptions.append( QString::fromUtf8( results[( i * columns ) + 2 ] ) );
  }
  sqlite3_free_table( results );

  QString selectOthersQuery = QString( "SELECT id,styleName,description"
                                       " FROM layer_styles"
                                       " WHERE NOT (f_table_schema=%1 AND f_table_name=%2 AND f_geometry_column=%3)"
                                       " ORDER BY update_time DESC" )
                              .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
                              .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
                              .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) );

  ret = sqlite3_get_table( sqliteHandle, selectOthersQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectOthersQuery ) );
    errCause = QObject::tr( "Error executing the select query for unrelated styles. The query was logged" );
    return -1;
  }

  for ( int i = 1; i <= rows; i++ )
  {
    ids.append( results[( i * columns ) + 0 ] );
    names.append( QString::fromUtf8( results[( i * columns ) + 1 ] ) );
    descriptions.append( QString::fromUtf8( results[( i * columns ) + 2 ] ) );
  }
  sqlite3_free_table( results );

  QgsSqliteHandle::closeDb( handle );
  return numberOfRelatedStyles;
}
//-----------------------------------------------------------------
// QGISEXTERN getStyleById
//-----------------------------------------------------------------
QGISEXTERN QString getStyleById( const QString &uri, QString styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QgsDebugMsgLevel( QString( "Database is: %1" ).arg( sqlitePath ), 3 );

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugMsgLevel( QString( "Connection to database failed. Save style aborted." ), 3 );
    errCause = QObject::tr( "Connection to database failed" );
    return QLatin1String( "" );
  }

  sqlite3 *sqliteHandle = handle->handle();

  QString style;
  QString selectQmlQuery = QStringLiteral( "SELECT styleQml FROM layer_styles WHERE id=%1" ).arg( QgsSpatiaLiteUtils::quotedValue( styleId ) );
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle, selectQmlQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK == ret )
  {
    if ( 1 == rows )
      style = QString::fromUtf8( results[( rows * columns ) + 0 ] );
    else
      errCause = QObject::tr( "Consistency error in table '%1'. Style id should be unique" ).arg( QStringLiteral( "layer_styles" ) );
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectQmlQuery ) );
    errCause = QObject::tr( "Error executing the select query. The query was logged" );
  }

  QgsSqliteHandle::closeDb( handle );
  sqlite3_free_table( results );
  return style;
}
//-----------------------------------------------------------------
// QGISEXTERN cleanupProvider
//-----------------------------------------------------------------
QGISEXTERN void cleanupProvider()
{
  QgsSqliteHandle::closeAll();
}

