/***************************************************************************
                         qgsdwgimporter.cpp
                         --------------
    begin                : May 2016
    copyright            : (C) 2016 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdwgimporter.h"

#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "libdwgr.h"
#include "libdxfrw.h"
#include "qgis.h"
#include "qgspoint.h"
#include "qgslinestring.h"
#include "qgscircularstring.h"
#include "qgscurvepolygon.h"
#include "qgscompoundcurve.h"
#include "qgspolygon.h"
#include "qgsgeometry.h"
#include "qgsogrutils.h"
#include "qgscoordinatereferencesystem.h"

#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QVector>
#include <QTransform>
#include <QLabel>
#include <QTextCodec>
#include <QRegularExpression>
#include <QTextStream>

#include <typeinfo>

#include <cpl_port.h>
#include <cpl_error.h>
#include <cpl_string.h>
#include <gdal.h>
#include <ogr_srs_api.h>
#include <memory>
#include <mutex>

#define LOG( x ) { QgsDebugMsg( x ); QgsMessageLog::logMessage( x, QObject::tr( "DWG/DXF import" ) ); }
#define ONCE( x ) { static bool show=true; if( show ) LOG( x ); show=false; }
#define NYI( x ) { static bool show=true; if( show ) LOG( QObject::tr("Not yet implemented %1").arg( x ) ); show=false; }
#define SETSTRING(a)  setString(dfn, f, #a, decode(data.a))
#define SETSTRINGPTR(a)  setString(dfn, f.get(), #a, decode(data.a))
#define SETDOUBLE(a)  setDouble(dfn, f, #a, data.a)
#define SETDOUBLEPTR(a)  setDouble(dfn, f.get(), #a, data.a)
#define SETINTEGER(a) setInteger(dfn, f, #a, data.a)
#define SETINTEGERPTR(a) setInteger(dfn, f.get(), #a, data.a)

#ifdef _MSC_VER
#define strcasecmp( a, b ) stricmp( a, b )
#endif


class QgsDrwDebugPrinter : public DRW::DebugPrinter
{
  public:

    explicit QgsDrwDebugPrinter( int debugLevel = 4 )
      : mTS( &mBuf )
      , mLevel( debugLevel )
    { }

    ~QgsDrwDebugPrinter() override
    {
      QgsDebugMsgLevel( mBuf, mLevel );
    }

    void printS( const std::string &s, const char *file, const char *function, int line ) override
    {
      if ( mLevel > QgsLogger::debugLevel() )
        return;

      mFile = file;
      mFunction = function;
      mLine = line;
      mTS << QString::fromStdString( s );
      flush();
    }

    void printI( long long int i, const char *file, const char *function, int line ) override
    {
      if ( mLevel > QgsLogger::debugLevel() )
        return;

      mFile = file;
      mFunction = function;
      mLine = line;
      mTS << i;
      flush();
    }

    void printUI( long long unsigned int i, const char *file, const char *function, int line ) override
    {
      if ( mLevel > QgsLogger::debugLevel() )
        return;

      mFile = file;
      mFunction = function;
      mLine = line;
      mTS << i;
      flush();
    }

    void printD( double d, const char *file, const char *function, int line ) override
    {
      if ( mLevel > QgsLogger::debugLevel() )
        return;

      mFile = file;
      mFunction = function;
      mLine = line;
      mTS << QStringLiteral( "%1 " ).arg( d, 0, 'g' );
      flush();
    }

    void printH( long long int i, const char *file, const char *function, int line ) override
    {
      if ( mLevel > QgsLogger::debugLevel() )
        return;

      mFile = file;
      mFunction = function;
      mLine = line;
      mTS << QStringLiteral( "0x%1" ).arg( i, 0, 16 );
      flush();
    }

    void printB( int i, const char *file, const char *function, int line ) override
    {
      if ( mLevel > QgsLogger::debugLevel() )
        return;

      mFile = file;
      mFunction = function;
      mLine = line;
      mTS << QStringLiteral( "0%1" ).arg( i, 0, 8 );
      flush();
    }

    void printHL( int c, int s, int h, const char *file, const char *function, int line ) override
    {
      if ( mLevel > QgsLogger::debugLevel() )
        return;

      mFile = file;
      mFunction = function;
      mLine = line;
      mTS << QStringLiteral( "%1.%2 0x%3" ).arg( c ).arg( s ).arg( h, 0, 16 );
      flush();
    }

    void printPT( double x, double y, double z, const char *file, const char *function, int line ) override
    {
      if ( mLevel > QgsLogger::debugLevel() )
        return;

      mFile = file;
      mFunction = function;
      mLine = line;
      mTS << QStringLiteral( "x:%1 y:%2 z:%3" ).arg( x, 0, 'g' ).arg( y, 0, 'g' ).arg( z, 0, 'g' );
      flush();
    }

  private:
    std::ios_base::fmtflags flags{std::cerr.flags()};
    QString mBuf;
    QTextStream mTS;
    QString mFile;
    QString mFunction;
    int mLine = 0;
    int mLevel = 4;

    void flush()
    {
      const QStringList lines = mBuf.split( '\n' );
      for ( int i = 0; i < lines.size() - 1; ++i )
      {
        QgsLogger::debug( lines.at( i ), mLevel, mFile.toLocal8Bit().constData(), mFunction.toLocal8Bit().constData(), mLine );
      }
      mBuf = lines.last();
    }
};


QgsDwgImporter::QgsDwgImporter( const QString &database, const QgsCoordinateReferenceSystem &crs )
  : mDs( nullptr )
  , mDatabase( database )
  , mInTransaction( false )
  , mSplineSegs( 8 )
  , mBlockHandle( -1 )
  , mCrs( crs.srsid() )
  , mUseCurves( true )
  , mEntities( 0 )
{
  QgsDebugCall;

  // setup custom debug printer for libdxfrw
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]( )
  {
    DRW::setCustomDebugPrinter( new QgsDrwDebugPrinter( 4 ) );
  } );

  const QString crswkt( crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED_GDAL ) );
  mCrsH = QgsOgrUtils::crsToOGRSpatialReference( crs );
  QgsDebugMsg( QStringLiteral( "CRS %1[%2]: %3" ).arg( mCrs ).arg( ( qint64 ) mCrsH, 0, 16 ).arg( crswkt ) );
}

bool QgsDwgImporter::exec( const QString &sql, bool logError )
{
  if ( !mDs )
  {
    QgsDebugMsg( QStringLiteral( "No data source" ) );
    return false;
  }

  CPLErrorReset();

  OGRLayerH layer = OGR_DS_ExecuteSQL( mDs.get(), sql.toUtf8().constData(), nullptr, nullptr );
  if ( layer )
  {
    QgsDebugMsg( QStringLiteral( "Unexpected result set" ) );
    OGR_DS_ReleaseResultSet( mDs.get(), layer );
    return false;
  }

  if ( CPLGetLastErrorType() == CE_None )
    return true;

  if ( logError )
  {
    LOG( tr( "SQL statement failed\nDatabase: %1\nSQL: %2\nError: %3" )
         .arg( mDatabase, sql, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
  }
  return false;
}

OGRLayerH QgsDwgImporter::query( const QString &sql )
{
  if ( !mDs )
  {
    QgsDebugMsg( QStringLiteral( "No data source" ) );
    return nullptr;
  }

  CPLErrorReset();

  OGRLayerH layer = OGR_DS_ExecuteSQL( mDs.get(), sql.toUtf8().constData(), nullptr, nullptr );
  if ( !layer )
  {
    QgsDebugMsg( QStringLiteral( "Result expected" ) );
    return layer;
  }

  if ( CPLGetLastErrorType() == CE_None )
    return layer;

  LOG( tr( "SQL statement failed\nDatabase: %1\nSQL: %2\nError: %3" )
       .arg( mDatabase, sql, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );

  OGR_DS_ReleaseResultSet( mDs.get(), layer );

  return nullptr;
}

void QgsDwgImporter::startTransaction()
{
  Q_ASSERT( mDs );

  mInTransaction = GDALDatasetStartTransaction( mDs.get(), 0 ) == OGRERR_NONE;
  if ( !mInTransaction )
  {
    LOG( tr( "Could not start transaction\nDatabase: %1\nError: %2" )
         .arg( mDatabase, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
  }
}

void QgsDwgImporter::commitTransaction()
{
  Q_ASSERT( mDs );

  if ( mInTransaction && GDALDatasetCommitTransaction( mDs.get() ) != OGRERR_NONE )
  {
    LOG( tr( "Could not commit transaction\nDatabase: %1\nError: %2" )
         .arg( mDatabase, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
  }
  mInTransaction = false;
}

QgsDwgImporter::~QgsDwgImporter()
{
  QgsDebugCall;

  if ( mDs )
  {
    commitTransaction();
  }
  if ( mCrsH )
  {
    OSRRelease( mCrsH );
    mCrsH = nullptr;
  }
}

QString drwVersionToString( DRW::Version version )
{
  switch ( version )
  {
    case DRW::UNKNOWNV:
      return QObject::tr( "Unknown version" );
    case DRW::MC00:
      return QObject::tr( "AutoCAD Release 1.1" );
    case DRW::AC12:
      return QObject::tr( "AutoCAD Release 1.2" );
    case DRW::AC14:
      return QObject::tr( "AutoCAD Release 1.4" );
    case DRW::AC150:
      return QObject::tr( "AutoCAD Release 2.0" );
    case DRW::AC210:
      return QObject::tr( "AutoCAD Release 2.10" );
    case DRW::AC1002:
      return QObject::tr( "AutoCAD Release 2.5" );
    case DRW::AC1003:
      return QObject::tr( "AutoCAD Release 2.6" );
    case DRW::AC1004:
      return QObject::tr( "AutoCAD Release 9" );
    case DRW::AC1006:
      return QObject::tr( "AutoCAD Release 10" );
    case DRW::AC1009:
      return QObject::tr( "AutoCAD Release 11/12 (LT R1/R2)" );
    case DRW::AC1012:
      return QObject::tr( "AutoCAD Release 13 (LT95)" );
    case DRW::AC1014:
      return QObject::tr( "AutoCAD Release 14/14.01 (LT97/LT98)" );
    case DRW::AC1015:
      return QObject::tr( "AutoCAD 2000/2000i/2002" );
    case DRW::AC1018:
      return QObject::tr( "AutoCAD 2004/2005/2006" );
    case DRW::AC1021:
      return QObject::tr( "AutoCAD 2007/2008/2009" );
    case DRW::AC1024:
      return QObject::tr( "AutoCAD 2010/2011/2012" );
    case DRW::AC1027:
      return QObject::tr( "AutoCAD 2013/2014/2015/2016/2017" );
    case DRW::AC1032:
      return QObject::tr( "AutoCAD 2018/2019/2020" );
  }
  return QString();
}

bool QgsDwgImporter::import( const QString &drawing, QString &error, bool doExpandInserts, bool useCurves, QLabel *label )
{
  QgsDebugCall;

  mLabel = label;

  OGRwkbGeometryType lineGeomType, hatchGeomType;
  if ( useCurves )
  {
    lineGeomType = wkbCompoundCurveZ;
    hatchGeomType = wkbCurvePolygonZ;
  }
  else
  {
    lineGeomType = wkbLineString25D;
    hatchGeomType = wkbPolygon25D;
  }

  mUseCurves = useCurves;

  const QFileInfo fi( drawing );
  if ( !fi.isReadable() )
  {
    error = tr( "Drawing %1 is unreadable" ).arg( drawing );
    return false;
  }

  if ( QFileInfo::exists( mDatabase ) )
  {
    mDs.reset( OGROpen( mDatabase.toUtf8().constData(), true, nullptr ) );
    if ( !mDs )
    {
      LOG( tr( "Could not open database [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      return false;
    }

    // Check whether database is up-to-date
    OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "drawing" );
    if ( !layer )
    {
      LOG( tr( "Query for drawing %1 failed." ).arg( drawing ) );
      mDs.reset();
      return false;
    }

    OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
    //int pathIdx = OGR_FD_GetFieldIndex( dfn, "path" );
    const int lastmodifiedIdx = OGR_FD_GetFieldIndex( dfn, "lastmodified" );

    OGR_L_ResetReading( layer );

    const gdal::ogr_feature_unique_ptr f( OGR_L_GetNextFeature( layer ) );
    if ( !f )
    {
      LOG( tr( "Could not retrieve drawing name from database [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      mDs.reset();
      return false;
    }

    int year, month, day, hour, minute, second, tzf;
    if ( !OGR_F_GetFieldAsDateTime( f.get(), lastmodifiedIdx, &year, &month, &day, &hour, &minute, &second, &tzf ) )
    {
      LOG( tr( "Recorded last modification date unreadable [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      mDs.reset();
      return false;
    }

#if 0
    QDateTime lastModified( QDate( year, month, day ), QTime( hour, minute, second ) );
    QString path = QString::fromUtf8( OGR_F_GetFieldAsString( f, pathIdx ) );
    if ( path == fi.canonicalPath() && fi.lastModified() <= lastModified )
    {
      LOG( tr( "Drawing already up-to-date in database." ) );
      OGR_F_Destroy( f );
      return true;
    }
#endif

    mDs.reset();

    QFile::remove( mDatabase );
  }

  struct field
  {
    field( const QString &name, OGRFieldType ogrType, int width = -1, int precision = -1 )
      : mName( name ), mOgrType( ogrType ), mWidth( width ), mPrecision( precision )
    {}

    QString mName;
    OGRFieldType mOgrType;
    int mWidth;
    int mPrecision;
  };

  struct table
  {
    table( const QString &name, const QString &desc, OGRwkbGeometryType wkbType, const QList<field> &fields )
      : mName( name ), mDescription( desc ), mWkbType( wkbType ), mFields( fields )
    {}

    QString mName;
    QString mDescription;
    OGRwkbGeometryType mWkbType;
    QList<field> mFields;
  };

#define ENTITY_ATTRIBUTES \
      << field( "handle", OFTInteger ) \
      << field( "block", OFTInteger ) \
      << field( "etype", OFTInteger ) \
      << field( "space", OFTInteger ) \
      << field( "layer", OFTString ) \
      << field( "olinetype", OFTString ) \
      << field( "linetype", OFTString ) \
      << field( "color", OFTString ) \
      << field( "ocolor", OFTInteger ) \
      << field( "color24", OFTInteger ) \
      << field( "transparency", OFTInteger ) \
      << field( "lweight", OFTInteger ) \
      << field( "linewidth", OFTReal ) \
      << field( "ltscale", OFTReal ) \
      << field( "visible", OFTInteger )


  const QList<table> tables = QList<table>()
                              << table( QStringLiteral( "drawing" ), tr( "Imported drawings" ), wkbNone, QList<field>()
                                        << field( QStringLiteral( "path" ), OFTString )
                                        << field( QStringLiteral( "comments" ), OFTString )
                                        << field( QStringLiteral( "importdat" ), OFTDateTime )
                                        << field( QStringLiteral( "lastmodified" ), OFTDateTime )
                                        << field( QStringLiteral( "crs" ), OFTInteger )
                                      )
                              << table( QStringLiteral( "headers" ), tr( "Headers" ), wkbNone, QList<field>()
                                        << field( QStringLiteral( "k" ), OFTString )
                                        << field( QStringLiteral( "v" ), OFTString )
                                      )
                              << table( QStringLiteral( "linetypes" ), tr( "Line types" ), wkbNone, QList<field>()
                                        << field( QStringLiteral( "name" ), OFTString )
                                        << field( QStringLiteral( "desc" ), OFTString )
                                        << field( QStringLiteral( "path" ), OFTRealList )
                                      )
                              << table( QStringLiteral( "layers" ), tr( "Layer list" ), wkbNone, QList<field>()
                                        << field( QStringLiteral( "name" ), OFTString )
                                        << field( QStringLiteral( "linetype" ), OFTString )
                                        << field( QStringLiteral( "color" ), OFTString )
                                        << field( QStringLiteral( "ocolor" ), OFTInteger )
                                        << field( QStringLiteral( "color24" ), OFTInteger )
                                        << field( QStringLiteral( "transparency" ), OFTInteger )
                                        << field( QStringLiteral( "lweight" ), OFTInteger )
                                        << field( QStringLiteral( "linewidth" ), OFTReal )
                                        << field( QStringLiteral( "flags" ), OFTInteger )
                                      )
                              << table( QStringLiteral( "dimstyles" ), tr( "Dimension styles" ), wkbNone, QList<field>()
                                        << field( QStringLiteral( "name" ), OFTString )
                                        << field( QStringLiteral( "dimpost" ), OFTString )
                                        << field( QStringLiteral( "dimapost" ), OFTString )
                                        << field( QStringLiteral( "dimblk" ), OFTString )
                                        << field( QStringLiteral( "dimblk1" ), OFTString )
                                        << field( QStringLiteral( "dimblk2" ), OFTString )
                                        << field( QStringLiteral( "dimscale" ), OFTReal )
                                        << field( QStringLiteral( "dimasz" ), OFTReal )
                                        << field( QStringLiteral( "dimexo" ), OFTReal )
                                        << field( QStringLiteral( "dimdli" ), OFTReal )
                                        << field( QStringLiteral( "dimexe" ), OFTReal )
                                        << field( QStringLiteral( "dimrnd" ), OFTReal )
                                        << field( QStringLiteral( "dimdle" ), OFTReal )
                                        << field( QStringLiteral( "dimtp" ), OFTReal )
                                        << field( QStringLiteral( "dimtm" ), OFTReal )
                                        << field( QStringLiteral( "dimfxl" ), OFTReal )
                                        << field( QStringLiteral( "dimtxt" ), OFTReal )
                                        << field( QStringLiteral( "dimcen" ), OFTReal )
                                        << field( QStringLiteral( "dimtsz" ), OFTReal )
                                        << field( QStringLiteral( "dimaltf" ), OFTReal )
                                        << field( QStringLiteral( "dimlfac" ), OFTReal )
                                        << field( QStringLiteral( "dimtvp" ), OFTReal )
                                        << field( QStringLiteral( "dimtfac" ), OFTReal )
                                        << field( QStringLiteral( "dimgap" ), OFTReal )
                                        << field( QStringLiteral( "dimaltrnd" ), OFTReal )
                                        << field( QStringLiteral( "dimtol" ), OFTInteger )
                                        << field( QStringLiteral( "dimlim" ), OFTInteger )
                                        << field( QStringLiteral( "dimtih" ), OFTInteger )
                                        << field( QStringLiteral( "dimtoh" ), OFTInteger )
                                        << field( QStringLiteral( "dimse1" ), OFTInteger )
                                        << field( QStringLiteral( "dimse2" ), OFTInteger )
                                        << field( QStringLiteral( "dimtad" ), OFTInteger )
                                        << field( QStringLiteral( "dimzin" ), OFTInteger )
                                        << field( QStringLiteral( "dimazin" ), OFTInteger )
                                        << field( QStringLiteral( "dimalt" ), OFTInteger )
                                        << field( QStringLiteral( "dimaltd" ), OFTInteger )
                                        << field( QStringLiteral( "dimtofl" ), OFTInteger )
                                        << field( QStringLiteral( "dimsah" ), OFTInteger )
                                        << field( QStringLiteral( "dimtix" ), OFTInteger )
                                        << field( QStringLiteral( "dimsoxd" ), OFTInteger )
                                        << field( QStringLiteral( "dimclrd" ), OFTInteger )
                                        << field( QStringLiteral( "dimclre" ), OFTInteger )
                                        << field( QStringLiteral( "dimclrt" ), OFTInteger )
                                        << field( QStringLiteral( "dimadec" ), OFTInteger )
                                        << field( QStringLiteral( "dimunit" ), OFTInteger )
                                        << field( QStringLiteral( "dimdec" ), OFTInteger )
                                        << field( QStringLiteral( "dimtdec" ), OFTInteger )
                                        << field( QStringLiteral( "dimaltu" ), OFTInteger )
                                        << field( QStringLiteral( "dimalttd" ), OFTInteger )
                                        << field( QStringLiteral( "dimaunit" ), OFTInteger )
                                        << field( QStringLiteral( "dimfrac" ), OFTInteger )
                                        << field( QStringLiteral( "dimlunit" ), OFTInteger )
                                        << field( QStringLiteral( "dimdsep" ), OFTInteger )
                                        << field( QStringLiteral( "dimtmove" ), OFTInteger )
                                        << field( QStringLiteral( "dimjust" ), OFTInteger )
                                        << field( QStringLiteral( "dimsd1" ), OFTInteger )
                                        << field( QStringLiteral( "dimsd2" ), OFTInteger )
                                        << field( QStringLiteral( "dimtolj" ), OFTInteger )
                                        << field( QStringLiteral( "dimtzin" ), OFTInteger )
                                        << field( QStringLiteral( "dimaltz" ), OFTInteger )
                                        << field( QStringLiteral( "dimaltttz" ), OFTInteger )
                                        << field( QStringLiteral( "dimfit" ), OFTInteger )
                                        << field( QStringLiteral( "dimupt" ), OFTInteger )
                                        << field( QStringLiteral( "dimatfit" ), OFTInteger )
                                        << field( QStringLiteral( "dimfxlon" ), OFTInteger )
                                        << field( QStringLiteral( "dimtxsty" ), OFTString )
                                        << field( QStringLiteral( "dimldrblk" ), OFTString )
                                        << field( QStringLiteral( "dimlwd" ), OFTInteger )
                                        << field( QStringLiteral( "dimlwe" ), OFTInteger )
                                      )
                              << table( QStringLiteral( "textstyles" ), tr( "Text styles" ), wkbNone, QList<field>()
                                        << field( QStringLiteral( "name" ), OFTString )
                                        << field( QStringLiteral( "height" ), OFTReal )
                                        << field( QStringLiteral( "width" ), OFTReal )
                                        << field( QStringLiteral( "oblique" ), OFTReal )
                                        << field( QStringLiteral( "genFlag" ), OFTInteger )
                                        << field( QStringLiteral( "lastHeight" ), OFTReal )
                                        << field( QStringLiteral( "font" ), OFTString )
                                        << field( QStringLiteral( "bigFont" ), OFTString )
                                        << field( QStringLiteral( "fontFamily" ), OFTInteger )
                                      )
                              << table( QStringLiteral( "appdata" ), tr( "Application data" ), wkbNone, QList<field>()
                                        << field( QStringLiteral( "handle" ), OFTInteger )
                                        << field( QStringLiteral( "i" ), OFTInteger )
                                        << field( QStringLiteral( "value" ), OFTString )
                                      )
                              << table( QStringLiteral( "blocks" ), tr( "BLOCK entities" ), wkbPoint25D, QList<field>()
                                        ENTITY_ATTRIBUTES
                                        << field( QStringLiteral( "thickness" ), OFTReal )
                                        << field( QStringLiteral( "ext" ), OFTRealList )
                                        << field( QStringLiteral( "name" ), OFTString )
                                        << field( QStringLiteral( "flags" ), OFTInteger )
                                      )
                              << table( QStringLiteral( "points" ), tr( "POINT entities" ), wkbPoint25D, QList<field>()
                                        ENTITY_ATTRIBUTES
                                        << field( QStringLiteral( "thickness" ), OFTReal )
                                        << field( QStringLiteral( "ext" ), OFTRealList )
                                      )
                              << table( QStringLiteral( "lines" ), tr( "LINE entities" ), lineGeomType, QList<field>()
                                        ENTITY_ATTRIBUTES
                                        << field( QStringLiteral( "thickness" ), OFTReal )
                                        << field( QStringLiteral( "ext" ), OFTRealList )
                                        << field( QStringLiteral( "width" ), OFTReal )
                                      )
                              << table( QStringLiteral( "polylines" ), tr( "POLYLINE entities" ), lineGeomType, QList<field>()
                                        ENTITY_ATTRIBUTES
                                        << field( QStringLiteral( "width" ), OFTReal )
                                        << field( QStringLiteral( "thickness" ), OFTReal )
                                        << field( QStringLiteral( "ext" ), OFTRealList ) )
                              << table( QStringLiteral( "texts" ), tr( "TEXT entities" ), wkbPoint25D, QList<field>()
                                        ENTITY_ATTRIBUTES
                                        << field( QStringLiteral( "thickness" ), OFTReal )
                                        << field( QStringLiteral( "ext" ), OFTRealList )
                                        << field( QStringLiteral( "height" ), OFTReal )
                                        << field( QStringLiteral( "text" ), OFTString )
                                        << field( QStringLiteral( "angle" ), OFTReal )
                                        << field( QStringLiteral( "widthscale" ), OFTReal )
                                        << field( QStringLiteral( "oblique" ), OFTReal )
                                        << field( QStringLiteral( "style" ), OFTString )
                                        << field( QStringLiteral( "textgen" ), OFTInteger )
                                        << field( QStringLiteral( "alignh" ), OFTInteger )
                                        << field( QStringLiteral( "alignv" ), OFTInteger )
                                        << field( QStringLiteral( "interlin" ), OFTReal )
                                      )
                              << table( QStringLiteral( "hatches" ), tr( "HATCH entities" ), hatchGeomType, QList<field>()
                                        ENTITY_ATTRIBUTES
                                        << field( QStringLiteral( "thickness" ), OFTReal )
                                        << field( QStringLiteral( "ext" ), OFTRealList )
                                        << field( QStringLiteral( "name" ), OFTString )
                                        << field( QStringLiteral( "solid" ), OFTInteger )
                                        << field( QStringLiteral( "associative" ), OFTInteger )
                                        << field( QStringLiteral( "hstyle" ), OFTInteger )
                                        << field( QStringLiteral( "hpattern" ), OFTInteger )
                                        << field( QStringLiteral( "doubleflag" ), OFTInteger )
                                        << field( QStringLiteral( "angle" ), OFTReal )
                                        << field( QStringLiteral( "scale" ), OFTReal )
                                        << field( QStringLiteral( "deflines" ), OFTInteger )
                                      )
                              << table( QStringLiteral( "inserts" ), tr( "INSERT entities" ), wkbPoint25D, QList<field>()
                                        ENTITY_ATTRIBUTES
                                        << field( QStringLiteral( "thickness" ), OFTReal )
                                        << field( QStringLiteral( "ext" ), OFTRealList )
                                        << field( QStringLiteral( "name" ), OFTString )
                                        << field( QStringLiteral( "xscale" ), OFTReal )
                                        << field( QStringLiteral( "yscale" ), OFTReal )
                                        << field( QStringLiteral( "zscale" ), OFTReal )
                                        << field( QStringLiteral( "angle" ), OFTReal )
                                        << field( QStringLiteral( "colcount" ), OFTReal )
                                        << field( QStringLiteral( "rowcount" ), OFTReal )
                                        << field( QStringLiteral( "colspace" ), OFTReal )
                                        << field( QStringLiteral( "rowspace" ), OFTReal )
                                      )
                              ;

  OGRSFDriverH driver = OGRGetDriverByName( "GPKG" );
  if ( !driver )
  {
    LOG( tr( "Could not load geopackage driver" ) );
    return false;
  }

  progress( tr( "Creating database…" ) );

  // create database
  mDs.reset( OGR_Dr_CreateDataSource( driver, mDatabase.toUtf8().constData(), nullptr ) );
  if ( !mDs )
  {
    LOG( tr( "Creation of datasource failed [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    return false;
  }

  progress( tr( "Creating tables…" ) );

  startTransaction();

  for ( const table &t : tables )
  {
    char **options = nullptr;
    options = CSLSetNameValue( options, "OVERWRITE", "YES" );
    options = CSLSetNameValue( options, "DESCRIPTION", t.mDescription.toUtf8().constData() );
    if ( t.mWkbType == wkbNone )
    {
      options = CSLSetNameValue( options, "SPATIAL_INDEX", "NO" );
    }

    OGRLayerH layer = OGR_DS_CreateLayer( mDs.get(), t.mName.toUtf8().constData(), ( t.mWkbType != wkbNone && mCrs > 0 ) ? mCrsH : nullptr, t.mWkbType, options );

    CSLDestroy( options );
    options = nullptr;

    if ( !layer )
    {
      LOG( tr( "Creation of drawing layer %1 failed [%2]" ).arg( t.mName, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      mDs.reset();
      return false;
    }

    for ( const field &f : t.mFields )
    {
      const gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( f.mName.toUtf8().constData(), f.mOgrType ) );
      if ( !fld )
      {
        LOG( tr( "Creation of field definition for %1.%2 failed [%3]" ).arg( t.mName, f.mName, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
        mDs.reset();
        return false;
      }

      if ( f.mWidth >= 0 )
        OGR_Fld_SetWidth( fld.get(), f.mWidth );
      if ( f.mPrecision >= 0 )
        OGR_Fld_SetPrecision( fld.get(), f.mPrecision );

      const OGRErr res = OGR_L_CreateField( layer, fld.get(), true );

      if ( res != OGRERR_NONE )
      {
        LOG( tr( "Creation of field %1.%2 failed [%3]" ).arg( t.mName, f.mName, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
        mDs.reset();
        return false;
      }
    }
  }

  commitTransaction();

  progress( tr( "Importing drawing…" ) );

  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "drawing" );
  Q_ASSERT( layer );

  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  const int pathIdx = OGR_FD_GetFieldIndex( dfn, "path" );
  const int importdatIdx = OGR_FD_GetFieldIndex( dfn, "importdat" );
  const int lastmodifiedIdx = OGR_FD_GetFieldIndex( dfn, "lastmodified" );
  const int crsIdx = OGR_FD_GetFieldIndex( dfn, "crs" );

  const gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );
  Q_ASSERT( f );

  OGR_F_SetFieldString( f.get(), pathIdx, fi.canonicalFilePath().toUtf8().constData() );

  QDateTime d( fi.lastModified() );
  OGR_F_SetFieldDateTime( f.get(), lastmodifiedIdx,
                          d.date().year(),
                          d.date().month(),
                          d.date().day(),
                          d.time().hour(),
                          d.time().minute(),
                          d.time().second(),
                          0
                        );

  d = QDateTime::currentDateTime();
  OGR_F_SetFieldDateTime( f.get(), importdatIdx,
                          d.date().year(),
                          d.date().month(),
                          d.date().day(),
                          d.time().hour(),
                          d.time().minute(),
                          d.time().second(),
                          0
                        );

  OGR_F_SetFieldInteger( f.get(), crsIdx, mCrs );

  if ( OGR_L_CreateFeature( layer, f.get() ) != OGRERR_NONE )
  {
    LOG( tr( "Could not update drawing record [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    return false;
  }

  LOG( tr( "Updating database from %1 [%2]." ).arg( drawing, fi.lastModified().toString() ) );

  DRW::error result( DRW::BAD_NONE );
  DRW::Version version = DRW::Version::UNKNOWNV;

  mTime.start();
  mEntities = 0;

  if ( fi.suffix().compare( QLatin1String( "dxf" ), Qt::CaseInsensitive ) == 0 )
  {
    //loads dxf
    std::unique_ptr<dxfRW> dxf( new dxfRW( drawing.toLocal8Bit() ) );
    if ( !dxf->read( this, true ) )
    {
      result = DRW::BAD_UNKNOWN;
    }
  }
  else if ( fi.suffix().compare( QLatin1String( "dwg" ), Qt::CaseInsensitive ) == 0 )
  {
    //loads dwg
    std::unique_ptr<dwgR> dwg( new dwgR( drawing.toLocal8Bit() ) );
    if ( !dwg->read( this, true ) )
    {
      result = dwg->getError();
    }
    version = dwg->getVersion();
  }
  else
  {
    LOG( tr( "File %1 is not a DWG/DXF file" ).arg( drawing ) );
    return false;
  }

  switch ( result )
  {
    case DRW::BAD_NONE:
      error = tr( "No error." );
      break;
    case DRW::BAD_UNKNOWN:
      error = tr( "Unknown error." );
      break;
    case DRW::BAD_OPEN:
      error = tr( "error opening file." );
      break;
    case DRW::BAD_VERSION:
      error = tr( "unsupported version. Cannot read %1 documents." ).arg( drwVersionToString( version ) );
      break;
    case DRW::BAD_READ_METADATA:
      error = tr( "error reading metadata." );
      break;
    case DRW::BAD_READ_FILE_HEADER:
      error = tr( "error in file header read process." );
      break;
    case DRW::BAD_READ_HEADER:
      error = tr( "error in header vars read process." );
      break;
    case DRW::BAD_READ_HANDLES:
      error = tr( "error in object map read process." );
      break;
    case DRW::BAD_READ_CLASSES:
      error = tr( "error in classes read process." );
      break;
    case DRW::BAD_READ_TABLES:
      error = tr( "error in tables read process." );
      result = DRW::BAD_NONE;
      break;
    case DRW::BAD_READ_BLOCKS:
      error = tr( "error in block read process." );
      break;
    case DRW::BAD_READ_ENTITIES:
      error = tr( "error in entities read process." );
      break;
    case DRW::BAD_READ_OBJECTS:
      error = tr( "error in objects read process." );
      break;
  }

  if ( result != DRW::BAD_NONE )
  {
    QgsDebugMsg( QStringLiteral( "error:%1" ).arg( error ) );
    return false;
  }

  return !doExpandInserts || expandInserts( error );
}

void QgsDwgImporter::addHeader( const DRW_Header *data )
{
  QgsDebugCall;

  Q_ASSERT( data );

  if ( !data->getComments().empty() )
  {
    OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "drawing" );
    OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
    const int importdatIdx = OGR_FD_GetFieldIndex( dfn, "comments" );

    OGR_L_ResetReading( layer );
    const gdal::ogr_feature_unique_ptr f( OGR_L_GetNextFeature( layer ) );
    Q_ASSERT( f );

    OGR_F_SetFieldString( f.get(), importdatIdx, data->getComments().c_str() );

    if ( OGR_L_SetFeature( layer, f.get() ) != OGRERR_NONE )
    {
      LOG( tr( "Could not update comment in drawing record [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      return;
    }
  }

  if ( data->vars.empty() )
    return;

  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "headers" );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  const int kIdx = OGR_FD_GetFieldIndex( dfn, "k" );
  const int vIdx = OGR_FD_GetFieldIndex( dfn, "v" );

  for ( std::map<std::string, DRW_Variant *>::const_iterator it = data->vars.begin(); it != data->vars.end(); ++it )
  {
    const gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );

    const QString k = it->first.c_str();

    QString v;
    switch ( it->second->type() )
    {
      case DRW_Variant::STRING:
        v = it->second->content.s->c_str();

        if ( k == QLatin1String( "$DWGCODEPAGE" ) )
        {
          const QHash<QString, QString> encodingMap
          {
            { "ASCII", "" },
            { "8859_1", "ISO-8859-1" },
            { "8859_2", "ISO-8859-2" },
            { "8859_3", "ISO-8859-3" },
            { "8859_4", "ISO-8859-4" },
            { "8859_5", "ISO-8859-5" },
            { "8859_6", "ISO-8859-6" },
            { "8859_7", "ISO-8859-7" },
            { "8859_8", "ISO-8859-8" },
            { "8859_9", "ISO-8859-9" },
            //  { "DOS437", "" },
            { "DOS850", "CP850" },
            //  { "DOS852", "" },
            //  { "DOS855", "" },
            //  { "DOS857", "" },
            //  { "DOS860", "" },
            //  { "DOS861", "" },
            //  { "DOS863", "" },
            //  { "DOS864", "" },
            //  { "DOS865", "" },
            //  { "DOS869", "" },
            //  { "DOS932", "" },
            { "MACINTOSH", "MacRoman" },
            { "BIG5", "Big5" },
            { "KSC5601", "ksc5601.1987-0" },
            //   { "JOHAB", "" },
            { "DOS866", "CP866" },
            { "ANSI_1250", "CP1250" },
            { "ANSI_1251", "CP1251" },
            { "ANSI_1252", "CP1252" },
            { "GB2312", "GB2312" },
            { "ANSI_1253", "CP1253" },
            { "ANSI_1254", "CP1254" },
            { "ANSI_1255", "CP1255" },
            { "ANSI_1256", "CP1256" },
            { "ANSI_1257", "CP1257" },
            { "ANSI_874", "CP874" },
            { "ANSI_932", "Shift_JIS" },
            { "ANSI_936", "CP936" },
            { "ANSI_949", "CP949" },
            { "ANSI_950", "CP950" },
            //  { "ANSI_1361", "" },
            //  { "ANSI_1200", "" },
            { "ANSI_1258", "CP1258" },
          };

          mCodec = QTextCodec::codecForName( encodingMap.value( v, QStringLiteral( "CP1252" ) ).toLocal8Bit() );

          QgsDebugMsg( QString( "codec set to %1" ).arg( mCodec ? QString( mCodec->name() ) : QStringLiteral( "(unset)" ) ) );
        }
        break;

      case DRW_Variant::INTEGER:
        v = QString::number( it->second->content.i );

        if ( k == QLatin1String( "SPLINESEGS" ) )
          mSplineSegs = it->second->content.i;

        break;

      case DRW_Variant::DOUBLE:
        v = qgsDoubleToString( it->second->content.d );
        break;

      case DRW_Variant::COORD:
        v = QStringLiteral( "%1,%2,%3" )
            .arg( qgsDoubleToString( it->second->content.v->x ),
                  qgsDoubleToString( it->second->content.v->y ),
                  qgsDoubleToString( it->second->content.v->z ) );
        break;

      case DRW_Variant::INVALID:
        break;
    }

    OGR_F_SetFieldString( f.get(), kIdx, k.toUtf8().constData() );
    OGR_F_SetFieldString( f.get(), vIdx, v.toUtf8().constData() );

    if ( OGR_L_CreateFeature( layer, f.get() ) != OGRERR_NONE )
    {
      LOG( tr( "Could not add %3 %1 [%2]" )
           .arg( k,
                 QString::fromUtf8( CPLGetLastErrorMsg() ),
                 tr( "header record" ) )
         );
    }
  }
}

void QgsDwgImporter::addLType( const DRW_LType &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "linetypes" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );

  const gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );
  Q_ASSERT( f );

  const QString name( decode( data.name ) );

  setString( dfn, f.get(), QStringLiteral( "name" ), name );
  SETSTRINGPTR( desc );

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QVector<double> path( QVector<double>::fromStdVector( data.path ) );
#else
  QVector<double> path( data.path.begin(), data.path.end() );
#endif
  OGR_F_SetFieldDoubleList( f.get(), OGR_FD_GetFieldIndex( dfn, "path" ), path.size(), path.data() );

  QVector<double> upath;
  for ( int i = 0; i < path.size(); i++ )
  {
    if ( i == 0 )
    {
      upath.push_back( path[i] );
      continue;
    }

    if ( path[i] == 0.0 )
    {
      NYI( tr( "dotted linetypes - dot ignored" ) );
      continue;
    }

    if ( ( upath.back() < 0. && path[i] < 0. ) || ( upath.back() > 0. && path[i] > 0. ) )
    {
      upath.back() += path[i];
    }
    else
    {
      upath.push_back( path[i] );
    }
  }

  QString dash;
  if ( !upath.empty() )
  {
    QStringList l;
    if ( upath[0] < 0 )
      l << QStringLiteral( "0" );

    const auto constUpath = upath;
    for ( const double p : constUpath )
    {
      l << QString::number( std::fabs( p ) );
    }

    if ( upath.size() % 2 == 1 )
      l << QStringLiteral( "0" );

    dash = l.join( QLatin1Char( ';' ) ).toUtf8().constData();
  }
  mLinetype.insert( name.toLower(), dash );

  if ( OGR_L_CreateFeature( layer, f.get() ) != OGRERR_NONE )
  {
    LOG( tr( "Could not add %3 %1 [%2]" )
         .arg( data.name.c_str(),
               QString::fromUtf8( CPLGetLastErrorMsg() ),
               tr( "line type" ) )
       );
  }
}

QString QgsDwgImporter::colorString( int color, int color24, int transparency, const QString &layer ) const
{
#if 0
  QgsDebugMsgLevel( QStringLiteral( "colorString(color=%1, color24=0x%2, transparency=0x%3 layer=%4" )
                    .arg( color )
                    .arg( color24, 0, 16 )
                    .arg( transparency, 0, 16 )
                    .arg( layer.c_str() ), 5 );
#endif
  if ( color24 == -1 )
  {
    if ( color == 0 )
    {
      return QStringLiteral( "byblock" );
    }
    else if ( color == 256 )
    {
      return mLayerColor.value( layer, QStringLiteral( "0,0,0,255" ) );
    }
    else
    {
      if ( color < 0 )
        color = -color;

      return QStringLiteral( "%1,%2,%3,%4" )
             .arg( DRW::dxfColors[color][0] )
             .arg( DRW::dxfColors[color][1] )
             .arg( DRW::dxfColors[color][2] )
             .arg( 255 - ( transparency & 0xff ) );
    }
  }
  else
  {
    return QStringLiteral( "%1,%2,%3,%4" )
           .arg( ( color24 & 0xff0000 ) >> 16 )
           .arg( ( color24 & 0x00ff00 ) >> 8 )
           .arg( ( color24 & 0x0000ff ) )
           .arg( 255 - ( transparency & 0xff ) );
  }
}

QString QgsDwgImporter::linetypeString( const QString &olinetype, const QString &layer ) const
{
  if ( olinetype == QLatin1String( "bylayer" ) )
    return mLayerLinetype.value( layer, QString() );
  else
    return mLinetype.value( olinetype, QString() );
}

void QgsDwgImporter::addLayer( const DRW_Layer &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "layers" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  const gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );
  Q_ASSERT( f );

  const QString name( decode( data.name ) );
  const QString linetype( decode( data.lineType ) );

  setString( dfn, f.get(), QStringLiteral( "name" ), name );
  setString( dfn, f.get(), QStringLiteral( "linetype" ), linetype );

  SETINTEGERPTR( flags );

  const QString color = colorString( data.color, data.color24, data.transparency, "" );
  mLayerColor.insert( name, color );

  double linewidth = lineWidth( data.lWeight, "" );
  if ( linewidth < 0. )
    linewidth = 0.;
  mLayerLinewidth.insert( name, linewidth );
  mLayerLinetype.insert( name, linetypeString( linetype, "" ) );

  setInteger( dfn, f.get(), QStringLiteral( "ocolor" ), data.color );
  SETINTEGERPTR( color24 );
  SETINTEGERPTR( transparency );
  setString( dfn, f.get(), QStringLiteral( "color" ), color );
  setInteger( dfn, f.get(), QStringLiteral( "lweight" ), DRW_LW_Conv::lineWidth2dxfInt( data.lWeight ) );
  setInteger( dfn, f.get(), QStringLiteral( "linewidth" ), linewidth );

  if ( OGR_L_CreateFeature( layer, f.get() ) != OGRERR_NONE )
  {
    LOG( tr( "Could not add %3 %1 [%2]" )
         .arg( name, QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "layer" ) )
       );
  }
}

void QgsDwgImporter::setString( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, const char *value ) const
{
  const int idx = OGR_FD_GetFieldIndex( dfn, field.toLower().toUtf8().constData() );
  if ( idx < 0 )
  {
    LOG( tr( "Field %1 not found" ).arg( field ) );
    return;
  }
  OGR_F_SetFieldString( f, idx, value );
}

void QgsDwgImporter::setString( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, const std::string &value ) const
{
  setString( dfn, f, field, value.c_str() );
}

void QgsDwgImporter::setString( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, const QString &value ) const
{
  setString( dfn, f, field, value.toUtf8().constData() );
}

void QgsDwgImporter::setDouble( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, double value ) const
{
  const int idx = OGR_FD_GetFieldIndex( dfn, field.toLower().toUtf8().constData() );
  if ( idx < 0 )
  {
    LOG( tr( "Field %1 not found" ).arg( field ) );
    return;
  }
  OGR_F_SetFieldDouble( f, idx, value );
}

void QgsDwgImporter::setInteger( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, int value ) const
{
  const int idx = OGR_FD_GetFieldIndex( dfn, field.toLower().toUtf8().constData() );
  if ( idx < 0 )
  {
    LOG( tr( "Field %1 not found" ).arg( field ) );
    return;
  }
  OGR_F_SetFieldInteger( f, idx, value );
}

void QgsDwgImporter::setPoint( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, const DRW_Coord &p ) const
{
  QVector<double> ext( 3 );
  ext[0] = p.x;
  ext[1] = p.y;
  ext[2] = p.z;

  const int idx = OGR_FD_GetFieldIndex( dfn, field.toLower().toUtf8().constData() );
  if ( idx < 0 )
  {
    LOG( tr( "Field %1 not found" ).arg( field ) );
    return;
  }

  OGR_F_SetFieldDoubleList( f, idx, 3, ext.data() );
}

double QgsDwgImporter::lineWidth( int lWeight, const QString &layer ) const
{
  switch ( lWeight )
  {
    case 0:
      return 0.00;
    case 1:
      return 0.05;
    case 2:
      return 0.09;
    case 3:
      return 0.13;
    case 4:
      return 0.15;
    case 5:
      return 0.18;
    case 6:
      return 0.20;
    case 7:
      return 0.25;
    case 8:
      return 0.30;
    case 9:
      return 0.35;
    case 10:
      return 0.40;
    case 11:
      return 0.50;
    case 12:
      return 0.53;
    case 13:
      return 0.60;
    case 14:
      return 0.70;
    case 15:
      return 0.80;
    case 16:
      return 0.90;
    case 17:
      return 1.00;
    case 18:
      return 1.06;
    case 19:
      return 1.20;
    case 20:
      return 1.40;
    case 21:
      return 1.58;
    case 22:
      return 2.00;
    case 23:
      return 2.11;
    case 29: // bylayer
      return mLayerLinewidth.value( layer, 0.0 );
    case 30: // byblock
      return -1.0;
    case 31:
    default:
      NYI( tr( "Line width default" ) );
      return 0.0;
  }
}

void QgsDwgImporter::addDimStyle( const DRW_Dimstyle &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "dimstyles" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  const gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );
  Q_ASSERT( f );

  const QString name( decode( data.name ) );

  setString( dfn, f.get(), QStringLiteral( "name" ), name );
  SETSTRINGPTR( dimpost );
  SETSTRINGPTR( dimapost );
  SETSTRINGPTR( dimblk );
  SETSTRINGPTR( dimblk1 );
  SETSTRINGPTR( dimblk2 );
  SETDOUBLEPTR( dimscale );
  SETDOUBLEPTR( dimasz );
  SETDOUBLEPTR( dimexo );
  SETDOUBLEPTR( dimdli );
  SETDOUBLEPTR( dimexe );
  SETDOUBLEPTR( dimrnd );
  SETDOUBLEPTR( dimdle );
  SETDOUBLEPTR( dimtp );
  SETDOUBLEPTR( dimtm );
  SETDOUBLEPTR( dimfxl );
  SETDOUBLEPTR( dimtxt );
  SETDOUBLEPTR( dimcen );
  SETDOUBLEPTR( dimtsz );
  SETDOUBLEPTR( dimaltf );
  SETDOUBLEPTR( dimlfac );
  SETDOUBLEPTR( dimtvp );
  SETDOUBLEPTR( dimtfac );
  SETDOUBLEPTR( dimgap );
  SETDOUBLEPTR( dimaltrnd );
  SETINTEGERPTR( dimtol );
  SETINTEGERPTR( dimlim );
  SETINTEGERPTR( dimtih );
  SETINTEGERPTR( dimtoh );
  SETINTEGERPTR( dimse1 );
  SETINTEGERPTR( dimse2 );
  SETINTEGERPTR( dimtad );
  SETINTEGERPTR( dimzin );
  SETINTEGERPTR( dimazin );
  SETINTEGERPTR( dimalt );
  SETINTEGERPTR( dimaltd );
  SETINTEGERPTR( dimtofl );
  SETINTEGERPTR( dimsah );
  SETINTEGERPTR( dimtix );
  SETINTEGERPTR( dimsoxd );
  SETINTEGERPTR( dimclrd );
  SETINTEGERPTR( dimclre );
  SETINTEGERPTR( dimclrt );
  SETINTEGERPTR( dimadec );
  SETINTEGERPTR( dimunit );
  SETINTEGERPTR( dimdec );
  SETINTEGERPTR( dimtdec );
  SETINTEGERPTR( dimaltu );
  SETINTEGERPTR( dimalttd );
  SETINTEGERPTR( dimaunit );
  SETINTEGERPTR( dimfrac );
  SETINTEGERPTR( dimlunit );
  SETINTEGERPTR( dimdsep );
  SETINTEGERPTR( dimtmove );
  SETINTEGERPTR( dimjust );
  SETINTEGERPTR( dimsd1 );
  SETINTEGERPTR( dimsd2 );
  SETINTEGERPTR( dimtolj );
  SETINTEGERPTR( dimtzin );
  SETINTEGERPTR( dimaltz );
  SETINTEGERPTR( dimaltttz );
  SETINTEGERPTR( dimfit );
  SETINTEGERPTR( dimupt );
  SETINTEGERPTR( dimatfit );
  SETINTEGERPTR( dimfxlon );
  SETSTRINGPTR( dimtxsty );
  SETSTRINGPTR( dimldrblk );
  SETINTEGERPTR( dimlwd );
  SETINTEGERPTR( dimlwe );

  if ( OGR_L_CreateFeature( layer, f.get() ) != OGRERR_NONE )
  {
    LOG( tr( "Could not add %3 %1 [%2]" )
         .arg( name, QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "dimension style" ) )
       );
  }
}

void QgsDwgImporter::addVport( const DRW_Vport &data )
{
  Q_UNUSED( data )
}

void QgsDwgImporter::addTextStyle( const DRW_Textstyle &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "textstyles" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  const gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );
  Q_ASSERT( f );

  const QString name( decode( data.name ) );
  setString( dfn, f.get(), QStringLiteral( "name" ), name );
  SETDOUBLEPTR( height );
  SETDOUBLEPTR( width );
  SETDOUBLEPTR( oblique );
  SETINTEGERPTR( genFlag );
  SETDOUBLEPTR( lastHeight );
  SETSTRINGPTR( font );
  SETSTRINGPTR( bigFont );
  SETINTEGERPTR( fontFamily );

  if ( OGR_L_CreateFeature( layer, f.get() ) != OGRERR_NONE )
  {
    LOG( tr( "Could not add %3 %1 [%2]" )
         .arg( name, QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "text style" ) )
       );
  }
}

void QgsDwgImporter::addAppId( const DRW_AppId &data )
{
  Q_UNUSED( data )
}

bool QgsDwgImporter::createFeature( OGRLayerH layer, OGRFeatureH f, const QgsAbstractGeometry &g0 ) const
{
  const QgsAbstractGeometry *g = nullptr;
  std::unique_ptr<QgsAbstractGeometry> sg( nullptr );

  if ( !mUseCurves && g0.hasCurvedSegments() )
  {
    sg.reset( g0.segmentize() );
    g = sg.get();
  }
  else
  {
    g = &g0;
  }

  const QByteArray wkb = g->asWkb();
  OGRGeometryH geom;
  if ( OGR_G_CreateFromWkb( ( unsigned char * ) wkb.constData(), nullptr, &geom, wkb.size() ) != OGRERR_NONE )
  {
    QgsDebugMsg( QStringLiteral( "Could not create geometry [%1][%2]" ).arg( g->asWkt() ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    LOG( tr( "Could not create geometry [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    return false;
  }

  OGR_F_SetGeometryDirectly( f, geom );

  return OGR_L_CreateFeature( layer, f ) == OGRERR_NONE;
}


void QgsDwgImporter::addBlock( const DRW_Block &data )
{
  Q_ASSERT( mBlockHandle < 0 );
  mBlockHandle = data.handle;

  const QString name( decode( data.name ) );

  QgsDebugMsgLevel( QStringLiteral( "block %1/0x%2 starts" ).arg( name ).arg( mBlockHandle, 0, 16 ), 5 );

  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "blocks" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  const gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );
  Q_ASSERT( f );

  addEntity( dfn, f.get(), data );

  setString( dfn, f.get(), QStringLiteral( "name" ), name );
  SETINTEGERPTR( flags );

  const QgsPoint p( QgsWkbTypes::PointZ, data.basePoint.x, data.basePoint.y, data.basePoint.z );

  if ( !createFeature( layer, f.get(), p ) )
  {
    LOG( tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "block" ) )
       );
  }
}

void QgsDwgImporter::setBlock( const int handle )
{
  Q_UNUSED( handle )
}

void QgsDwgImporter::endBlock()
{
  QgsDebugMsgLevel( QStringLiteral( "block 0x%1 ended" ).arg( mBlockHandle, 0, 16 ), 5 );
  mBlockHandle = -1;
}

void QgsDwgImporter::addEntity( OGRFeatureDefnH dfn, OGRFeatureH f, const DRW_Entity &data )
{
  QgsDebugMsgLevel( QStringLiteral( "handle:0x%1 block:0x%2" ).arg( data.handle, 0, 16 ).arg( mBlockHandle, 0, 16 ), 5 );

  mEntities++;
  if ( mTime.elapsed() > 1000 )
  {
    progress( tr( "%n entities processed.", nullptr, mEntities ) );
    mTime.restart();
  }

  SETINTEGER( handle );
  setInteger( dfn, f, QStringLiteral( "block" ), mBlockHandle );
  SETINTEGER( eType );
  SETINTEGER( space );

  const QString layer( decode( data.layer ) );
  const QString linetype( decode( data.lineType ) );

  setString( dfn, f, QStringLiteral( "layer" ), layer );
  setString( dfn, f, QStringLiteral( "olinetype" ), linetype );
  setString( dfn, f, QStringLiteral( "linetype" ), linetypeString( linetype, layer ) );
  setInteger( dfn, f, QStringLiteral( "ocolor" ), data.color );
  SETINTEGER( color24 );
  SETINTEGER( transparency );
  setString( dfn, f, QStringLiteral( "color" ), colorString( data.color, data.color24, data.transparency, layer ) );
  setInteger( dfn, f, QStringLiteral( "lweight" ), DRW_LW_Conv::lineWidth2dxfInt( data.lWeight ) );
  setDouble( dfn, f, QStringLiteral( "linewidth" ), lineWidth( data.lWeight, layer ) );
  setInteger( dfn, f, QStringLiteral( "ltscale" ), data.ltypeScale );
  SETINTEGER( visible );
}

void QgsDwgImporter::addPoint( const DRW_Point &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "points" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  SETDOUBLE( thickness );

  setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

  const QgsPoint p( QgsWkbTypes::PointZ, data.basePoint.x, data.basePoint.y, data.basePoint.z );
  if ( !createFeature( layer, f, p ) )
  {
    LOG( tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "point" ) )
       );
  }
}

void QgsDwgImporter::addRay( const DRW_Ray &data )
{
  Q_UNUSED( data )
  NYI( tr( "RAY entities" ) );
}

void QgsDwgImporter::addXline( const DRW_Xline &data )
{
  Q_UNUSED( data )
  NYI( tr( "XLINE entities" ) );
}

bool QgsDwgImporter::circularStringFromArc( const DRW_Arc &data, QgsCircularString &c )
{
  double half = ( data.staangle + data.endangle ) / 2.0;
  if ( data.staangle > data.endangle )
    half += M_PI;

  const double a0 = data.isccw ? data.staangle : -data.staangle;
  const double a1 = data.isccw ? half : -half;
  const double a2 = data.isccw ? data.endangle : -data.endangle;

  c.setPoints( QgsPointSequence()
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x + std::cos( a0 ) * data.radius, data.basePoint.y + std::sin( a0 ) * data.radius )
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x + std::cos( a1 ) * data.radius, data.basePoint.y + std::sin( a1 ) * data.radius )
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x + std::cos( a2 ) * data.radius, data.basePoint.y + std::sin( a2 ) * data.radius )
             );

  return true;
}

void QgsDwgImporter::addArc( const DRW_Arc &data )
{
  QgsCircularString c;
  if ( !circularStringFromArc( data, c ) )
  {
    LOG( tr( "Could not create circular string from  %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "arc" ) )
       );
    return;
  }

  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "lines" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  SETDOUBLE( thickness );
  setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

  if ( !createFeature( layer, f, c ) )
  {
    LOG( tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "arc" ) )
       );
  }
}

void QgsDwgImporter::addCircle( const DRW_Circle &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "lines" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  SETDOUBLE( thickness );

  setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

  QgsCircularString c;
  c.setPoints( QgsPointSequence()
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x - data.radius, data.basePoint.y, data.basePoint.z )
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x + data.radius, data.basePoint.y, data.basePoint.z )
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x - data.radius, data.basePoint.y, data.basePoint.z )
             );

  if ( !createFeature( layer, f, c ) )
  {
    LOG( tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "circle" ) )
       );
  }
}

void QgsDwgImporter::addEllipse( const DRW_Ellipse &data )
{
  DRW_Polyline pol;
  data.toPolyline( &pol );
  addPolyline( pol );
}

bool QgsDwgImporter::curveFromLWPolyline( const DRW_LWPolyline &data, QgsCompoundCurve &cc )
{
  const size_t vertexnum = data.vertlist.size();
  if ( vertexnum == 0 )
  {
    QgsDebugMsg( QStringLiteral( "polyline without points" ) );
    return false;
  }

  QgsPointSequence s;
  bool hadBulge( data.vertlist[0]->bulge != 0.0 );
  const std::vector<DRW_Vertex2D *>::size_type n = ( data.flags & 1 ) ? vertexnum + 1 : vertexnum;
  for ( std::vector<DRW_Vertex2D *>::size_type i = 0; i < n; i++ )
  {
    const size_t i0 = i % vertexnum;

    Q_ASSERT( data.vertlist[i0] );
    QgsDebugMsgLevel( QStringLiteral( "%1: %2,%3 bulge:%4" ).arg( i ).arg( data.vertlist[i0]->x ).arg( data.vertlist[i0]->y ).arg( data.vertlist[i0]->bulge ), 5 );

    const QgsPoint p( QgsWkbTypes::PointZ, data.vertlist[i0]->x, data.vertlist[i0]->y, data.elevation );
    s << p;

    const bool hasBulge( data.vertlist[i0]->bulge != 0.0 );

    if ( hasBulge != hadBulge || i == n - 1 )
    {
      if ( hadBulge )
      {
        QgsCircularString *c = new QgsCircularString();
        c->setPoints( s );
        cc.addCurve( c );
      }
      else
      {
        QgsLineString *c = new QgsLineString();
        c->setPoints( s );
        cc.addCurve( c );
      }

      hadBulge = hasBulge;
      s.clear();
      s << p;
    }

    if ( hasBulge && i < n - 1 )
    {
      const size_t i1 = ( i + 1 ) % vertexnum;

      const double a = 2.0 * std::atan( data.vertlist[i]->bulge );
      const double dx = data.vertlist[i1]->x - data.vertlist[i0]->x;
      const double dy = data.vertlist[i1]->y - data.vertlist[i0]->y;
      const double c = std::sqrt( dx * dx + dy * dy );
      const double r = c / 2.0 / std::sin( a );
      const double h = r * ( 1 - std::cos( a ) );

      s << QgsPoint( QgsWkbTypes::PointZ,
                     data.vertlist[i0]->x + 0.5 * dx + h * dy / c,
                     data.vertlist[i0]->y + 0.5 * dy - h * dx / c,
                     data.elevation );
    }
  }

  return true;
}


void QgsDwgImporter::addLWPolyline( const DRW_LWPolyline &data )
{
  const size_t vertexnum = data.vertlist.size();
  if ( vertexnum == 0 )
  {
    QgsDebugMsg( QStringLiteral( "LWPolyline without vertices" ) );
    return;
  }

  QgsPointSequence s;
  QgsCompoundCurve cc;
  double width = -1.0; // width is set to correct value during first loop
  bool hadBulge( false );

  const std::vector<DRW_Vertex2D *>::size_type n = ( data.flags & 1 ) ? vertexnum : vertexnum - 1;
  for ( std::vector<DRW_Vertex2D *>::size_type i = 0; i < n; i++ )
  {
    const size_t i0 = i % vertexnum;
    const size_t i1 = ( i + 1 ) % vertexnum;

    const QgsPoint p0( QgsWkbTypes::PointZ, data.vertlist[i0]->x, data.vertlist[i0]->y, data.elevation );
    const QgsPoint p1( QgsWkbTypes::PointZ, data.vertlist[i1]->x, data.vertlist[i1]->y, data.elevation );
    const double staWidth = data.vertlist[i0]->stawidth == 0.0 ? data.width : data.vertlist[i0]->stawidth;
    const double endWidth = data.vertlist[i0]->endwidth == 0.0 ? data.width : data.vertlist[i0]->endwidth;
    const bool hasBulge( data.vertlist[i0]->bulge != 0.0 );

#if 0
    QgsDebugMsgLevel( QStringLiteral( "i:%1,%2/%3 width=%4 staWidth=%5 endWidth=%6 hadBulge=%7 hasBulge=%8 l=%9 <=> %10" )
                      .arg( i0 ).arg( i1 ).arg( n )
                      .arg( width ).arg( staWidth ).arg( endWidth )
                      .arg( hadBulge ).arg( hasBulge )
                      .arg( p0.asWkt() )
                      .arg( p1.asWkt() ), 5
                    );
#endif

    if ( !s.empty() && ( width != staWidth || width != endWidth || hadBulge != hasBulge ) )
    {
      if ( hadBulge )
      {
        QgsCircularString *c = new QgsCircularString();
        c->setPoints( s );
        cc.addCurve( c );
      }
      else
      {
        QgsLineString *c = new QgsLineString();
        c->setPoints( s );
        cc.addCurve( c );
      }

      s.clear();

      if ( width != staWidth || width != endWidth )
      {
        // write out entity
        OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "polylines" );
        Q_ASSERT( layer );
        OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
        Q_ASSERT( dfn );
        OGRFeatureH f = OGR_F_Create( dfn );
        Q_ASSERT( f );

        addEntity( dfn, f, data );

        SETDOUBLE( thickness );
        SETDOUBLE( width );

        setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

        if ( !createFeature( layer, f, cc ) )
        {
          LOG( tr( "Could not add %2 [%1]" )
               .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "line string" ) )
             );
        }

        cc.clear();
      }
    }

    if ( p0 == p1 )
    {
      QgsDebugMsg( QStringLiteral( "i:%1,%2 empty segment skipped" ).arg( i0 ).arg( i1 ) );
      continue;
    }

    if ( staWidth == endWidth )
    {
      if ( s.empty() )
      {
        s << p0;
        hadBulge = hasBulge;
        width = staWidth;
      }

      if ( hasBulge )
      {
        const double a = 2.0 * std::atan( data.vertlist[i]->bulge );
        const double dx = p1.x() - p0.x();
        const double dy = p1.y() - p0.y();
        const double c = std::sqrt( dx * dx + dy * dy );
        const double r = c / 2.0 / std::sin( a );
        const double h = r * ( 1 - std::cos( a ) );

        s << QgsPoint( QgsWkbTypes::PointZ,
                       p0.x() + 0.5 * dx + h * dy / c,
                       p0.y() + 0.5 * dy - h * dx / c,
                       data.elevation );
      }

      s << p1;
    }
    else
    {
      OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "hatches" );
      Q_ASSERT( layer );
      OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
      Q_ASSERT( dfn );
      OGRFeatureH f = OGR_F_Create( dfn );
      Q_ASSERT( f );

      addEntity( dfn, f, data );

      SETDOUBLE( thickness );

      setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

      const QgsPointXY ps( p0.x(), p0.y() );
      const QgsPointXY pe( p1.x(), p1.y() );
      const QgsVector v( ( pe - ps ).perpVector().normalized() );
      const QgsVector vs( v * 0.5 * staWidth );
      const QgsVector ve( v * 0.5 * endWidth );

      QgsPolygon poly;
      QgsLineString *ls = new QgsLineString();
      ls->setPoints( QgsPointSequence()
                     << QgsPoint( ps + vs )
                     << QgsPoint( pe + ve )
                     << QgsPoint( pe - ve )
                     << QgsPoint( ps - vs )
                     << QgsPoint( ps + vs )
                   );
      ls->addZValue( data.elevation );
      poly.setExteriorRing( ls );
      // QgsDebugMsg( QStringLiteral( "write poly:%1" ).arg( poly.asWkt() ) );

      if ( !createFeature( layer, f, poly ) )
      {
        LOG( tr( "Could not add %2 [%1]" )
             .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "polygon" ) )
           );
      }
    }
  }

  if ( !s.empty() )
  {
    if ( hadBulge )
    {
      QgsCircularString *c = new QgsCircularString();
      c->setPoints( s );
      cc.addCurve( c );
    }
    else
    {
      QgsLineString *c = new QgsLineString();
      c->setPoints( s );
      cc.addCurve( c );
    }
  }

  if ( cc.nCurves() > 0 )
  {
    // write out entity
    OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "polylines" );
    Q_ASSERT( layer );
    OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
    Q_ASSERT( dfn );
    OGRFeatureH f = OGR_F_Create( dfn );
    Q_ASSERT( f );

    addEntity( dfn, f, data );

    SETDOUBLE( thickness );
    SETDOUBLE( width );

    setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

    // QgsDebugMsg( QStringLiteral( "write curve:%1" ).arg( cc.asWkt() ) );

    if ( !createFeature( layer, f, cc ) )
    {
      LOG( tr( "Could not add %2 [%1]" )
           .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "line string" ) )
         );
    }
  }
}

void QgsDwgImporter::addPolyline( const DRW_Polyline &data )
{
  const size_t vertexnum = data.vertlist.size();
  if ( vertexnum == 0 )
  {
    QgsDebugMsg( QStringLiteral( "Polyline without vertices" ) );
    return;
  }

  QgsPointSequence s;
  QgsCompoundCurve cc;
  double width = -1.0; // width is set to correct value during first loop
  bool hadBulge( false );

  const std::vector<DRW_Vertex *>::size_type n = ( data.flags & 1 ) ? vertexnum : vertexnum - 1;
  for ( std::vector<DRW_Vertex *>::size_type i = 0; i < n; i++ )
  {
    const size_t i0 = i % vertexnum;
    const size_t i1 = ( i + 1 ) % vertexnum;

    const QgsPoint p0( QgsWkbTypes::PointZ, data.vertlist[i0]->basePoint.x, data.vertlist[i0]->basePoint.y, data.vertlist[i0]->basePoint.z );
    const QgsPoint p1( QgsWkbTypes::PointZ, data.vertlist[i1]->basePoint.x, data.vertlist[i1]->basePoint.y, data.vertlist[i1]->basePoint.z );
    const double staWidth = data.vertlist[i0]->endwidth == 0.0 ? data.defendwidth : data.vertlist[i0]->stawidth;
    const double endWidth = data.vertlist[i0]->stawidth == 0.0 ? data.defstawidth : data.vertlist[i0]->endwidth;
    const bool hasBulge( data.vertlist[i0]->bulge != 0.0 );

    QgsDebugMsgLevel( QStringLiteral( "i:%1,%2/%3 width=%4 staWidth=%5 endWidth=%6 hadBulge=%7 hasBulge=%8 l=%9 <=> %10" )
                      .arg( i0 ).arg( i1 ).arg( n )
                      .arg( width ).arg( staWidth ).arg( endWidth )
                      .arg( hadBulge ).arg( hasBulge )
                      .arg( p0.asWkt() )
                      .arg( p1.asWkt() ), 5
                    );

    if ( !s.empty() && ( width != staWidth || width != endWidth || hadBulge != hasBulge ) )
    {
      if ( hadBulge )
      {
        QgsCircularString *c = new QgsCircularString();
        c->setPoints( s );

        if ( i > 0 )
          c->moveVertex( QgsVertexId( 0, 0, 0 ), cc.endPoint() );

        cc.addCurve( c );
      }
      else
      {
        QgsLineString *c = new QgsLineString();
        c->setPoints( s );

        if ( i > 0 )
          c->moveVertex( QgsVertexId( 0, 0, 0 ), cc.endPoint() );

        cc.addCurve( c );
      }

      s.clear();

      if ( width != staWidth || width != endWidth )
      {
        // write out entity
        OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "polylines" );
        Q_ASSERT( layer );
        OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
        Q_ASSERT( dfn );
        OGRFeatureH f = OGR_F_Create( dfn );
        Q_ASSERT( f );

        addEntity( dfn, f, data );

        SETDOUBLE( thickness );
        setDouble( dfn, f, QStringLiteral( "width" ), width );

        setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

        // QgsDebugMsg( QStringLiteral( "write curve:%1" ).arg( cc.asWkt() ) );

        if ( !createFeature( layer, f, cc ) )
        {
          LOG( tr( "Could not add %2 [%1]" )
               .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "line string" ) )
             );
        }

        cc.clear();
      }
    }

    if ( staWidth == endWidth )
    {
      if ( s.empty() )
      {
        s << p0;
        hadBulge = hasBulge;
        width = staWidth;
      }

      if ( hasBulge )
      {
        const double a = 2.0 * std::atan( data.vertlist[i]->bulge );
        const double dx = p1.x() - p0.x();
        const double dy = p1.y() - p0.y();
        const double dz = p1.z() - p0.z();
        const double c = std::sqrt( dx * dx + dy * dy );
        const double r = c / 2.0 / std::sin( a );
        const double h = r * ( 1 - std::cos( a ) );

        s << QgsPoint( QgsWkbTypes::PointZ,
                       p0.x() + 0.5 * dx + h * dy / c,
                       p0.y() + 0.5 * dy - h * dx / c,
                       p0.z() + 0.5 * dz );
      }

      s << p1;
    }
    else
    {
      OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "hatches" );
      Q_ASSERT( layer );
      OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
      Q_ASSERT( dfn );
      OGRFeatureH f = OGR_F_Create( dfn );
      Q_ASSERT( f );

      addEntity( dfn, f, data );

      SETDOUBLE( thickness );

      setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

      const QgsPointXY ps( p0.x(), p0.y() );
      const QgsPointXY pe( p1.x(), p1.y() );
      const QgsVector v( ( pe - ps ).perpVector().normalized() );
      const QgsVector vs( v * 0.5 * staWidth );
      const QgsVector ve( v * 0.5 * endWidth );

      QgsPolygon poly;
      QgsLineString *ls = new QgsLineString();
      QgsPointSequence s;
      s << QgsPoint( ps + vs );
      s.last().addZValue( p0.z() );
      s << QgsPoint( pe + ve );
      s.last().addZValue( p1.z() );
      s << QgsPoint( pe - ve );
      s.last().addZValue( p1.z() );
      s << QgsPoint( ps - vs );
      s.last().addZValue( p0.z() );
      s << QgsPoint( ps + vs );
      s.last().addZValue( p0.z() );
      ls->setPoints( s );
      poly.setExteriorRing( ls );
      // QgsDebugMsg( QStringLiteral( "write poly:%1" ).arg( poly.asWkt() ) );

      if ( !createFeature( layer, f, poly ) )
      {
        LOG( tr( "Could not add %2 [%1]" )
             .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "polygon" ) )
           );
      }
    }
  }

  if ( !s.empty() )
  {
    if ( hadBulge )
    {
      QgsCircularString *c = new QgsCircularString();
      c->setPoints( s );
      cc.addCurve( c );
    }
    else
    {
      QgsLineString *c = new QgsLineString();
      c->setPoints( s );
      cc.addCurve( c );
    }
  }

  if ( cc.nCurves() > 0 )
  {
    // write out entity
    OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "polylines" );
    Q_ASSERT( layer );
    OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
    Q_ASSERT( dfn );
    OGRFeatureH f = OGR_F_Create( dfn );
    Q_ASSERT( f );

    addEntity( dfn, f, data );

    SETDOUBLE( thickness );
    setDouble( dfn, f, QStringLiteral( "width" ), width );

    setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

    // QgsDebugMsg( QStringLiteral( "write curve:%1" ).arg( cc.asWkt() ) );

    if ( !createFeature( layer, f, cc ) )
    {
      LOG( tr( "Could not add %2 [%1]" )
           .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "line string" ) )
         );
    }
  }
}


/**
 * Generates B-Spline open knot vector with multiplicity
 * equal to the order at the ends.
 */
static std::vector<double> knot( const DRW_Spline &data, size_t num, size_t order )
{
  if ( data.knotslist.size() == num + order )
  {
    return data.knotslist;
  }
  else
  {
    std::vector<double> v( num + order, 0. );

    for ( size_t i = 0; i < num; ++i )
      v[order + i] = i + 1;

    for ( size_t i = num + 1; i < v.size(); ++i )
      v[i] = v[num];

    return v;
  }
}

static std::vector<double> knotu( const DRW_Spline &data, size_t num, size_t order )
{
  if ( data.knotslist.size() == num + order )
  {
    return data.knotslist;
  }
  else
  {
    std::vector<double> v( num + order, 0. );

    for ( size_t i = 0; i < v.size(); ++i )
      v[i] = i;

    return v;
  }
}

static std::vector<double> rbasis( size_t c, double t, size_t npts,
                                   const std::vector<double> &x,
                                   const std::vector<double> &h )
{
  const size_t nplusc = npts + c;
  std::vector<double> temp( nplusc, 0. );

  // calculate the first order nonrational basis functions n[i]
  for ( size_t i = 0; i < nplusc - 1; ++i )
  {
    if ( t >= x[i] && t < x[i + 1] )
      temp[i] = 1;
  }

  // calculate the higher order nonrational base functions
  for ( size_t k = 2; k <= c; ++k )
  {
    for ( size_t i = 0; i < nplusc - k; ++i )
    {
      // if the lower order basis function is zero skip the calculation
      if ( temp[i] != 0 )
        temp[i] = ( ( t - x[i] ) * temp[i] ) / ( x[i + k - 1] - x[i] );

      // if the lower order basis function is zero skip the calculation
      if ( temp[i + 1] != 0 )
        temp[i] += ( ( x[i + k] - t ) * temp[i + 1] ) / ( x[i + k] - x[i + 1] );
    }
  }

  // pick up last point
  if ( t >= x[nplusc - 1] )
    temp[npts - 1] = 1;

  // calculate sum for denominator of rational basis functions
  double sum = 0.;
  for ( size_t i = 0; i < npts; ++i )
  {
    sum += temp[i] * h[i];
  }

  std::vector<double> r( npts, 0. );

  // form rational basis functions and put in r vector
  if ( sum != 0.0 )
  {
    for ( size_t i = 0; i < npts; i++ )
    {
      r[i] = ( temp[i] * h[i] ) / sum;
    }
  }

  return r;
}

/**
 * Generates a rational B-spline curve using a uniform open knot vector.
 */
static void rbspline( const DRW_Spline &data,
                      size_t npts, size_t k, int p1,
                      const std::vector<QgsVector> &b,
                      const std::vector<double> &h,
                      std::vector<QgsPointXY> &p )
{
  const size_t nplusc = npts + k;

  // generate the open knot vector
  std::vector<double> x( knot( data, npts, k ) );

  // calculate the points on the rational B-spline curve
  double t = 0.;

  const double step = x[nplusc - 1] / ( p1 - 1 );
  for ( size_t i = 0; i < p.size(); ++i, t += step )
  {
    if ( x[nplusc - 1] - t < 5e-6 )
      t = x[nplusc - 1];

    // generate the basis function for this value of t
    std::vector<double> nbasis( rbasis( k, t, npts, x, h ) );

    // generate a point on the curve
    for ( size_t j = 0; j < npts; j++ )
      p[i] += b[j] * nbasis[j];
  }
}

static void rbsplinu( const DRW_Spline &data,
                      size_t npts, size_t k, int p1,
                      const std::vector<QgsVector> &b,
                      const std::vector<double> &h,
                      std::vector<QgsPointXY> &p )
{
  size_t const nplusc = npts + k;

  // generate the periodic knot vector
  std::vector<double> x( knotu( data, npts, k ) );

  // calculate the points on the rational B-spline curve
  double t = k - 1;
  double const step = double( npts - k + 1 ) / ( p1 - 1 );

  for ( size_t i = 0; i < p.size(); ++i, t += step )
  {
    if ( x[nplusc - 1] - t < 5e-6 )
      t = x[nplusc - 1];

    // generate the base function for this value of t
    std::vector<double> nbasis( rbasis( k, t, npts, x, h ) );

    // generate a point on the curve, for x, y, z
    for ( size_t j = 0; j < npts; ++j )
    {
      p[i] += b[j] * nbasis[j];
    }
  }
}

bool QgsDwgImporter::lineFromSpline( const DRW_Spline &data, QgsLineString &l )
{
  if ( data.degree < 1 || data.degree > 3 )
  {
    QgsDebugMsg( QStringLiteral( "%1: unknown spline degree %2" )
                 .arg( data.handle, 0, 16 )
                 .arg( data.degree ) );
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "degree: %1 ncontrol:%2 knotslist.size():%3 controllist.size():%4 fitlist.size():%5" )
                    .arg( data.degree )
                    .arg( data.ncontrol )
                    .arg( data.knotslist.size() )
                    .arg( data.controllist.size() )
                    .arg( data.fitlist.size() ), 5 );

  std::vector<QgsVector> cps;
  for ( size_t i = 0; i < data.controllist.size(); ++i )
  {
    const DRW_Coord &p = *data.controllist[i];
    cps.emplace_back( QgsVector( p.x, p.y ) );
  }

  if ( data.ncontrol == 0 && data.degree != 2 )
  {
    for ( std::vector<DRW_Coord *>::size_type i = 0; i < data.fitlist.size(); ++i )
    {
      const DRW_Coord &p = *data.fitlist[i];
      cps.emplace_back( QgsVector( p.x, p.y ) );
    }
  }

  if ( !cps.empty() && data.flags & 1 )
  {
    for ( int i = 0; i < data.degree; ++i )
      cps.push_back( cps[i] );
  }

  const size_t npts = cps.size();
  const size_t k = data.degree + 1;
  const int p1 = mSplineSegs * ( int ) npts;

  const std::vector<double> h( npts + 1, 1. );
  std::vector<QgsPointXY> p( p1, QgsPointXY( 0., 0. ) );

  if ( data.flags & 1 )
  {
    rbsplinu( data, npts, k, p1, cps, h, p );
  }
  else
  {
    rbspline( data, npts, k, p1, cps, h, p );
  }


  QgsPointSequence ps;
  for ( size_t i = 0; i < p.size(); ++i )
    ps << QgsPoint( p[i] );
  l.setPoints( ps );

  return true;
}

void QgsDwgImporter::addSpline( const DRW_Spline *data )
{
  Q_ASSERT( data );

  QgsLineString l;
  if ( !lineFromSpline( *data, l ) )
  {
    LOG( tr( "Could not create line from %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "spline" ) )
       );
    return;
  }

  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "polylines" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, *data );

  if ( !createFeature( layer, f, l ) )
  {
    LOG( tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "spline" ) )
       );
  }
}

void QgsDwgImporter::addKnot( const DRW_Entity &data )
{
  Q_UNUSED( data )
  NYI( tr( "KNOT entities" ) );
}

void QgsDwgImporter::addInsert( const DRW_Insert &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "inserts" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  SETDOUBLE( thickness );

  setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

  SETSTRING( name );
  SETDOUBLE( xscale );
  SETDOUBLE( yscale );
  SETDOUBLE( zscale );
  SETDOUBLE( angle );
  SETINTEGER( colcount );
  SETINTEGER( rowcount );
  SETDOUBLE( colspace );
  SETDOUBLE( rowspace );

  const QgsPoint p( QgsWkbTypes::PointZ, data.basePoint.x, data.basePoint.y, data.basePoint.z );

  if ( !createFeature( layer, f, p ) )
  {
    LOG( tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "point" ) )
       );
  }
}

void QgsDwgImporter::addTrace( const DRW_Trace &data )
{
  Q_UNUSED( data )
  NYI( tr( "TRACE entities" ) );
}

void QgsDwgImporter::add3dFace( const DRW_3Dface &data )
{
  Q_UNUSED( data )
  NYI( tr( "3DFACE entities" ) );
}

void QgsDwgImporter::addSolid( const DRW_Solid &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "hatches" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  SETDOUBLE( thickness );

  setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );
  setString( dfn, f, QStringLiteral( "hpattern" ), "SOLID" );

  QgsPolygon poly;

  // pt1 pt2
  // pt3 pt4
  QgsPointSequence s;
  s << QgsPoint( QgsWkbTypes::PointZ,   data.basePoint.x,   data.basePoint.y, data.basePoint.z );
  s << QgsPoint( QgsWkbTypes::PointZ,    data.secPoint.x,    data.secPoint.y, data.basePoint.z );
  s << QgsPoint( QgsWkbTypes::PointZ, data.forthPoint.x, data.forthPoint.y, data.basePoint.z );
  s << QgsPoint( QgsWkbTypes::PointZ,  data.thirdPoint.x,  data.thirdPoint.y, data.basePoint.z );
  s << s[0];

  QgsLineString *ls = new QgsLineString();
  ls->setPoints( s );
  poly.setExteriorRing( ls );

  if ( !createFeature( layer, f, poly ) )
  {
    LOG( tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "polygon" ) )
       );
  }
}

void QgsDwgImporter::addMText( const DRW_MText &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "texts" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  QString text( decode( data.text ) );
  cleanText( text );
  setString( dfn, f, QStringLiteral( "text" ), text );

  SETDOUBLE( height );
  SETDOUBLE( angle );
  SETDOUBLE( widthscale );
  SETDOUBLE( oblique );
  SETSTRING( style );
  SETINTEGER( textgen );
  SETINTEGER( alignH );
  SETINTEGER( alignV );
  SETDOUBLE( thickness );
  SETDOUBLE( interlin );

  setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

  const QgsPoint p( QgsWkbTypes::PointZ, data.basePoint.x, data.basePoint.y, data.basePoint.z );

  if ( !createFeature( layer, f, p ) )
  {
    LOG( tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "point" ) )
       );
  }
}

void QgsDwgImporter::addText( const DRW_Text &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "texts" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  SETDOUBLE( height );

  QString text( decode( data.text ) );
  cleanText( text );
  setString( dfn, f, QStringLiteral( "text" ), text );

  SETDOUBLE( angle );
  setDouble( dfn, f, "angle", data.angle * 180.0 / M_PI );
  SETDOUBLE( widthscale );
  SETDOUBLE( oblique );
  SETSTRING( style );
  SETINTEGER( textgen );
  SETINTEGER( alignH );
  SETINTEGER( alignV );
  SETDOUBLE( thickness );
  setDouble( dfn, f, QStringLiteral( "interlin" ), -1.0 );

  setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

  const QgsPoint p( QgsWkbTypes::PointZ,
                    ( data.alignH > 0 || data.alignV > 0 ) ? data.secPoint.x : data.basePoint.x,
                    ( data.alignH > 0 || data.alignV > 0 ) ? data.secPoint.y : data.basePoint.y,
                    ( data.alignH > 0 || data.alignV > 0 ) ? data.secPoint.z : data.basePoint.z );

  if ( !createFeature( layer, f, p ) )
  {
    LOG( tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "point" ) )
       );
  }
}

void QgsDwgImporter::addDimAlign( const DRW_DimAligned *data )
{
  Q_UNUSED( data )
  NYI( tr( "DIMALIGN entities" ) );
}

void QgsDwgImporter::addDimLinear( const DRW_DimLinear *data )
{
  Q_UNUSED( data )
  NYI( tr( "DIMLINEAR entities" ) );
}

void QgsDwgImporter::addDimRadial( const DRW_DimRadial *data )
{
  Q_UNUSED( data )
  NYI( tr( "DIMRADIAL entities" ) );
}

void QgsDwgImporter::addDimDiametric( const DRW_DimDiametric *data )
{
  Q_UNUSED( data )
  NYI( tr( "DIMDIAMETRIC entities" ) );
}

void QgsDwgImporter::addDimAngular( const DRW_DimAngular *data )
{
  Q_UNUSED( data )
  NYI( tr( "DIMANGULAR entities" ) );
}

void QgsDwgImporter::addDimAngular3P( const DRW_DimAngular3p *data )
{
  Q_UNUSED( data )
  NYI( tr( "DIMANGULAR3P entities" ) );
}

void QgsDwgImporter::addDimOrdinate( const DRW_DimOrdinate *data )
{
  Q_UNUSED( data )
  NYI( tr( "DIMORDINAL entities" ) );
}

void QgsDwgImporter::addLeader( const DRW_Leader *data )
{
  Q_UNUSED( data )
  NYI( tr( "LEADER entities" ) );
}

void QgsDwgImporter::addHatch( const DRW_Hatch *pdata )
{
  Q_ASSERT( pdata );
  const DRW_Hatch &data = *pdata;

  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "hatches" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  SETDOUBLE( thickness );

  setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

  SETSTRING( name );
  SETINTEGER( solid );
  SETINTEGER( associative );
  SETINTEGER( hstyle );
  SETINTEGER( hpattern );
  SETINTEGER( doubleflag );
  SETDOUBLE( angle );
  SETDOUBLE( scale );
  SETINTEGER( deflines );

  QgsCurvePolygon p;

  if ( static_cast< int >( data.looplist.size() ) != data.loopsnum )
  {
    LOG( tr( "0x%1: %2 instead of %3 loops found" )
         .arg( data.handle, 0, 16 )
         .arg( data.looplist.size() )
         .arg( data.loopsnum )
       );
  }

  for ( std::vector<DRW_HatchLoop *>::size_type i = 0; i < data.looplist.size(); i++ )
  {
    const DRW_HatchLoop &hatchLoop = *data.looplist[i];

    QgsCompoundCurve *cc = new QgsCompoundCurve();

    for ( std::vector<DRW_Entity *>::size_type j = 0; j < hatchLoop.objlist.size(); j++ )
    {
      Q_ASSERT( hatchLoop.objlist[j] );
      const DRW_Entity *entity = hatchLoop.objlist[j].get();
      if ( const DRW_LWPolyline *lwp = dynamic_cast<const DRW_LWPolyline *>( entity ) )
      {
        curveFromLWPolyline( *lwp, *cc );
      }
      else if ( const DRW_Line *l = dynamic_cast<const DRW_Line *>( entity ) )
      {
        QgsLineString *ls = new QgsLineString();
        ls->setPoints( QgsPointSequence()
                       << QgsPoint( QgsWkbTypes::PointZ, l->basePoint.x, l->basePoint.y, l->basePoint.z )
                       << QgsPoint( QgsWkbTypes::PointZ, l->secPoint.x, l->secPoint.y, l->secPoint.z ) );

        if ( j > 0 )
          ls->moveVertex( QgsVertexId( 0, 0, 0 ), cc->endPoint() );

        cc->addCurve( ls );
      }
      else if ( const DRW_Arc *a = dynamic_cast<const DRW_Arc *>( entity ) )
      {
        QgsCircularString *cs = new QgsCircularString();
        circularStringFromArc( *a, *cs );

        if ( j > 0 )
          cs->moveVertex( QgsVertexId( 0, 0, 0 ), cc->endPoint() );

        cc->addCurve( cs );
      }
      else if ( const DRW_Spline *sp = dynamic_cast<const DRW_Spline *>( entity ) )
      {
        QgsLineString *ls = new QgsLineString();
        lineFromSpline( *sp, *ls );

        if ( j > 0 )
          ls->moveVertex( QgsVertexId( 0, 0, 0 ), cc->endPoint() );

        cc->addCurve( ls );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "unknown obj %1.%2: %3" ).arg( i ).arg( j ).arg( typeid( *entity ).name() ) );
      }

#if 0
      QgsDebugMsg( QStringLiteral( "curve %1.%2\n  start %3\n  end   %4\n  compare %5" )
                   .arg( i ).arg( j )
                   .arg( cc->startPoint().asWkt() )
                   .arg( cc->endPoint().asWkt() )
                   .arg( cc->startPoint() == cc->endPoint() )
                 );
#endif
    }

    cc->moveVertex( QgsVertexId( 0, 0, 0 ), cc->endPoint() );

    if ( i == 0 )
    {
      // QgsDebugMsg( QStringLiteral( "set exterior ring:%1" ).arg( cc->asWkt() ) );
      p.setExteriorRing( cc );
    }
    else
    {
      // QgsDebugMsg( QStringLiteral( "set interior ring:%1" ).arg( cc->asWkt() ) );
      p.addInteriorRing( cc );
    }
  }

  if ( !createFeature( layer, f, p ) )
  {
    LOG( tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "polygon" ) )
       );
  }
}

void QgsDwgImporter::addLine( const DRW_Line &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "lines" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  SETDOUBLE( thickness );

  setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

  QgsLineString l;

  l.setPoints( QgsPointSequence()
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x, data.basePoint.y, data.basePoint.z )
               << QgsPoint( QgsWkbTypes::PointZ, data.secPoint.x, data.secPoint.y, data.secPoint.z ) );

  if ( !createFeature( layer, f, l ) )
  {
    LOG( tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "line string" ) )
       );
  }
}

void QgsDwgImporter::addViewport( const DRW_Viewport &data )
{
  Q_UNUSED( data )
  NYI( tr( "VIEWPORT entities" ) );
}

void QgsDwgImporter::addImage( const DRW_Image *data )
{
  Q_UNUSED( data )
  NYI( tr( "IMAGE entities" ) );
}

void QgsDwgImporter::linkImage( const DRW_ImageDef *data )
{
  Q_UNUSED( data )
  NYI( tr( "image links" ) );
}

void QgsDwgImporter::addComment( const char *comment )
{
  Q_UNUSED( comment )
  NYI( tr( "comments" ) );
}

void QgsDwgImporter::writeHeader( DRW_Header & ) {}
void QgsDwgImporter::writeBlocks() {}
void QgsDwgImporter::writeBlockRecords() {}
void QgsDwgImporter::writeEntities() {}
void QgsDwgImporter::writeLTypes() {}
void QgsDwgImporter::writeLayers() {}
void QgsDwgImporter::writeTextstyles() {}
void QgsDwgImporter::writeVports() {}
void QgsDwgImporter::writeDimstyles() {}
void QgsDwgImporter::writeAppId() {}

bool QgsDwgImporter::expandInserts( QString &error )
{
  QgsDebugCall;

  OGRLayerH blocks = OGR_DS_GetLayerByName( mDs.get(), "blocks" );
  if ( !blocks )
  {
    QgsDebugMsg( QStringLiteral( "could not open layer 'blocks'" ) );
    return false;
  }

  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( blocks );
  Q_ASSERT( dfn );

  const int nameIdx = OGR_FD_GetFieldIndex( dfn, "name" );
  const int handleIdx = OGR_FD_GetFieldIndex( dfn, "handle" );
  if ( nameIdx < 0 || handleIdx < 0 )
  {
    QgsDebugMsg( QStringLiteral( "not all fields found (nameIdx=%1 handleIdx=%2)" ).arg( nameIdx ).arg( handleIdx ) );
    return false;
  }

  OGR_L_ResetReading( blocks );

  mBlockNames.clear();
  mBlockBases.clear();

  gdal::ogr_feature_unique_ptr f;
  for ( ;; )
  {
    f.reset( OGR_L_GetNextFeature( blocks ) );
    if ( !f )
      break;

    const QString name = QString::fromUtf8( OGR_F_GetFieldAsString( f.get(), nameIdx ) );
    const int handle = OGR_F_GetFieldAsInteger( f.get(), handleIdx );
    OGRGeometryH ogrG = OGR_F_GetGeometryRef( f.get() );

    const QgsGeometry g( QgsOgrUtils::ogrGeometryToQgsGeometry( ogrG ) );
    if ( g.isNull() )
    {
      QgsDebugMsg( QStringLiteral( "%1: could not copy geometry" ).arg( OGR_F_GetFID( f.get() ) ) );
      continue;
    }

    mBlockNames.insert( name, handle );
    mBlockBases.insert( name, g.asPoint() );
  }

  return expandInserts( error, -1, QTransform() );
}

bool QgsDwgImporter::expandInserts( QString &error, int block, QTransform base )
{
  QgsDebugMsg( QString( "expanding block:%1" ).arg( block ) );
  OGRLayerH inserts = OGR_DS_ExecuteSQL( mDs.get(), QStringLiteral( "SELECT * FROM inserts WHERE block=%1" ).arg( block ).toUtf8().constData(), nullptr, nullptr );
  if ( !inserts )
  {
    QgsDebugMsg( QStringLiteral( "could not query layer 'inserts'" ) );
    return false;
  }

  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( inserts );
  Q_ASSERT( dfn );

  const int nameIdx = OGR_FD_GetFieldIndex( dfn, "name" );
  const int xscaleIdx = OGR_FD_GetFieldIndex( dfn, "xscale" );
  const int yscaleIdx = OGR_FD_GetFieldIndex( dfn, "yscale" );
  const int zscaleIdx = OGR_FD_GetFieldIndex( dfn, "zscale" );
  const int angleIdx = OGR_FD_GetFieldIndex( dfn, "angle" );
  const int layerIdx = OGR_FD_GetFieldIndex( dfn, "layer" );
  const int colorIdx = OGR_FD_GetFieldIndex( dfn, "color" );
  const int linetypeIdx = OGR_FD_GetFieldIndex( dfn, "linetype" );
  const int linewidthIdx = OGR_FD_GetFieldIndex( dfn, "linewidth" );
  if ( xscaleIdx < 0 || yscaleIdx < 0 || zscaleIdx < 0 || angleIdx < 0 || nameIdx < 0 || layerIdx < 0 || linetypeIdx < 0 || colorIdx < 0 || linewidthIdx < 0 )
  {
    QgsDebugMsg( QStringLiteral( "not all fields found (nameIdx=%1 xscaleIdx=%2 yscaleIdx=%3 zscaleIdx=%4 angleIdx=%5 layerIdx=%6 linetypeIdx=%7 color=%8 linewidthIdx=%9)" )
                 .arg( nameIdx )
                 .arg( xscaleIdx ).arg( yscaleIdx ).arg( zscaleIdx )
                 .arg( angleIdx )
                 .arg( layerIdx )
                 .arg( linetypeIdx )
                 .arg( colorIdx )
                 .arg( linewidthIdx )
               );
    return false;
  }

  const GIntBig n = OGR_L_GetFeatureCount( inserts, 0 );
  Q_UNUSED( n )

  OGR_L_ResetReading( inserts );

  if ( block == -1 )
    mTime.start();

  gdal::ogr_feature_unique_ptr insert;
  int i = 0, errors = 0;
  for ( int i = 0; true; ++i )
  {
    if ( block == -1 && mTime.elapsed() > 1000 )
    {
      progress( tr( "Expanding block reference %1/%2…" ).arg( i ).arg( n ) );
      mTime.restart();
    }
    else if ( i % 1000 == 0 )
    {
      QgsDebugMsg( QStringLiteral( "%1: expanding insert %2/%3…" ).arg( block, 0, 16 ).arg( i ).arg( n ) );
    }

    insert.reset( OGR_L_GetNextFeature( inserts ) );
    if ( !insert )
      break;

    OGRGeometryH ogrG = OGR_F_GetGeometryRef( insert.get() );
    if ( !ogrG )
    {
      QgsDebugMsg( QStringLiteral( "%1: insert without geometry" ).arg( OGR_F_GetFID( insert.get() ) ) );
      continue;
    }

    const QgsGeometry g( QgsOgrUtils::ogrGeometryToQgsGeometry( ogrG ) );
    if ( g.isNull() )
    {
      QgsDebugMsg( QStringLiteral( "%1: could not copy geometry" ).arg( OGR_F_GetFID( insert.get() ) ) );
      continue;
    }

    const QgsPointXY p( g.asPoint() );

    const QString name = QString::fromUtf8( OGR_F_GetFieldAsString( insert.get(), nameIdx ) );
    const double xscale = OGR_F_GetFieldAsDouble( insert.get(), xscaleIdx );
    const double yscale = OGR_F_GetFieldAsDouble( insert.get(), yscaleIdx );
    const double angle = OGR_F_GetFieldAsDouble( insert.get(), angleIdx );
    const QString blockLayer = QString::fromUtf8( OGR_F_GetFieldAsString( insert.get(), layerIdx ) );
    const QString blockColor = QString::fromUtf8( OGR_F_GetFieldAsString( insert.get(), colorIdx ) );
    const QString blockLinetype = QString::fromUtf8( OGR_F_GetFieldAsString( insert.get(), linetypeIdx ) );
    if ( blockLinetype == QLatin1String( "1" ) )
    {
      QgsDebugMsg( QStringLiteral( "blockLinetype == 1" ) );
    }
    const double blockLinewidth = OGR_F_GetFieldAsDouble( insert.get(), linewidthIdx );

    const int handle = mBlockNames.value( name, -1 );
    if ( handle < 0 )
    {
      QgsDebugMsg( QStringLiteral( "Block '%1' not found" ).arg( name ) );
      continue;
    }

    const QgsPointXY b = mBlockBases.value( name );

    QgsDebugMsgLevel( QStringLiteral( "Resolving %1/%2: p=%3,%4 b=%5,%6 scale=%7,%8 angle=%9" )
                      .arg( name ).arg( handle, 0, 16 )
                      .arg( p.x() ).arg( p.y() )
                      .arg( b.x() ).arg( b.y() )
                      .arg( xscale ).arg( yscale ).arg( angle ), 5 );


    QTransform t;
    t.translate( p.x(), p.y() ).scale( xscale, yscale ).rotateRadians( angle ).translate( -b.x(), -b.y() );
    t *= base;

    OGRLayerH src = nullptr;
    const QStringList types {"hatches", "lines", "polylines", "texts", "points"};
    for ( const QString &name : types )
    {
      if ( src )
        OGR_DS_ReleaseResultSet( mDs.get(), src );

      OGRLayerH src = OGR_DS_ExecuteSQL( mDs.get(), QStringLiteral( "SELECT * FROM %1 WHERE block=%2" ).arg( name ).arg( handle ).toUtf8().constData(), nullptr, nullptr );
      if ( !src )
      {
        QgsDebugMsg( QStringLiteral( "%1: could not open layer %1" ).arg( name ) );
        continue;
      }

      const GIntBig n = OGR_L_GetFeatureCount( src, 0 );
      Q_UNUSED( n )

      dfn = OGR_L_GetLayerDefn( src );
      Q_ASSERT( dfn );

      const int blockIdx = OGR_FD_GetFieldIndex( dfn, "block" );
      const int layerIdx = OGR_FD_GetFieldIndex( dfn, "layer" );
      const int colorIdx = OGR_FD_GetFieldIndex( dfn, "color" );
      if ( blockIdx < 0 || layerIdx < 0 || colorIdx < 0 )
      {
        QgsDebugMsg( QStringLiteral( "%1: fields not found (blockIdx=%2, layerIdx=%3 colorIdx=%4)" )
                     .arg( name ).arg( blockIdx ).arg( layerIdx ).arg( colorIdx )
                   );
        OGR_DS_ReleaseResultSet( mDs.get(), src );
        continue;
      }

      const int linetypeIdx = OGR_FD_GetFieldIndex( dfn, "linetype" );
      const int linewidthIdx = OGR_FD_GetFieldIndex( dfn, "linewidth" );
      const int angleIdx = OGR_FD_GetFieldIndex( dfn, "angle" );

      OGR_L_ResetReading( src );

      OGRLayerH dst = OGR_DS_GetLayerByName( mDs.get(), name.toUtf8().constData() );
      Q_ASSERT( dst );

      gdal::ogr_feature_unique_ptr f;
      int j = 0;
      for ( ;; )
      {
        if ( j % 1000 == 0 )
        {
          QgsDebugMsg( QStringLiteral( "%1.%2: %3/%4 copied" ).arg( block, 0, 16 ).arg( handle, 0, 16 ).arg( j ).arg( n ) );
        }

        f.reset( OGR_L_GetNextFeature( src ) );
        if ( !f )
          break;

        const GIntBig fid = OGR_F_GetFID( f.get() );
        Q_UNUSED( fid )

        ogrG = OGR_F_GetGeometryRef( f.get() );
        if ( !ogrG )
        {
          QgsDebugMsg( QStringLiteral( "%1/%2: geometryless feature skipped" ).arg( name ).arg( fid ) );
          continue;
        }

        QgsGeometry g( QgsOgrUtils::ogrGeometryToQgsGeometry( ogrG ) );
        if ( g.isNull() )
        {
          QgsDebugMsg( QStringLiteral( "%1: could not copy geometry" ).arg( fid ) );
          continue;
        }

        if ( g.transform( t ) != Qgis::GeometryOperationResult::Success )
        {
          QgsDebugMsg( QStringLiteral( "%1/%2: could not transform geometry" ).arg( name ).arg( fid ) );
          continue;
        }

        const QByteArray wkb = g.constGet()->asWkb();
        if ( OGR_G_CreateFromWkb( ( unsigned char * ) wkb.constData(), nullptr, &ogrG, wkb.size() ) != OGRERR_NONE )
        {
          QgsDebugMsg( QStringLiteral( "%1/%2: could not create ogr geometry" ).arg( name ).arg( fid ) );
          continue;
        }

        if ( OGR_F_SetGeometryDirectly( f.get(), ogrG ) != OGRERR_NONE )
        {
          QgsDebugMsg( QStringLiteral( "%1/%2: could not assign geometry" ).arg( name ).arg( fid ) );
          continue;
        }

        OGR_F_SetFID( f.get(), OGRNullFID );
        OGR_F_SetFieldInteger( f.get(), blockIdx, -1 );

        if ( strcasecmp( OGR_F_GetFieldAsString( f.get(), layerIdx ), "byblock" ) == 0 )
          OGR_F_SetFieldString( f.get(), layerIdx, blockLayer.toUtf8().constData() );

        if ( strcasecmp( OGR_F_GetFieldAsString( f.get(), colorIdx ), "byblock" ) == 0 )
          OGR_F_SetFieldString( f.get(), colorIdx, blockColor.toUtf8().constData() );

        if ( angleIdx >= 0 )
          OGR_F_SetFieldDouble( f.get(), angleIdx, OGR_F_GetFieldAsDouble( f.get(), angleIdx ) + angle );

        if ( linetypeIdx >= 0 && strcasecmp( OGR_F_GetFieldAsString( f.get(), linetypeIdx ), "byblock" ) == 0 )
          OGR_F_SetFieldString( f.get(), linetypeIdx, blockLinetype.toUtf8().constData() );

        if ( linewidthIdx >= 0 && OGR_F_GetFieldAsDouble( f.get(), linewidthIdx ) < 0.0 )
          OGR_F_SetFieldDouble( f.get(), linewidthIdx, blockLinewidth );

        if ( OGR_L_CreateFeature( dst, f.get() ) != OGRERR_NONE )
        {
          if ( errors < 1000 )
          {
            QgsMessageLog::logMessage( tr( "Could not copy feature of block %2 from layer %1 [Errors: %3]" )
                                       .arg( name ).arg( handle ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ),
                                       tr( "DWG/DXF import" ) );
          }
          else if ( errors == 1000 )
          {
            QgsMessageLog::logMessage( tr( "Not logging more errors" ), tr( "DWG/DXF import" ) );
          }

          ++errors;
          continue;
        }

        ++j;
      }

      QgsDebugMsgLevel( QStringLiteral( "%1: %2 features copied" ).arg( name ).arg( j ), 5 );
    }

    if ( src )
      OGR_DS_ReleaseResultSet( mDs.get(), src );

    if ( !expandInserts( error, handle, t ) )
    {
      QgsDebugMsg( QString( "%1: Expanding %2 failed" ).arg( block ).arg( handle ) );
    }
  }

  if ( errors > 0 )
  {
    error = tr( "%1 write errors during block expansion" ).arg( errors );
    return false;
  }
  else
  {
    error = tr( "%1 block insertion expanded." ).arg( i );
    return true;
  }
}

void QgsDwgImporter::progress( const QString &msg )
{
  mLabel->setText( msg );
  qApp->processEvents();
}

QString QgsDwgImporter::decode( const std::string &s ) const
{
  if ( mCodec )
    return mCodec->toUnicode( s.c_str() );
  else
    return s.c_str();
}

void QgsDwgImporter::cleanText( QString &res )
{
  bool ok = true;
  QRegularExpression re;

  res = res.replace( "\\~", " " );             // short space
  res = res.replace( "%%c", QChar( 0x2205 ) ); // diameter
  res = res.replace( "%%d", QChar( 0x00B0 ) ); // degree
  res = res.replace( "%%p", QChar( 0x00B1 ) ); // plus/minus
  res = res.replace( "%%%", QChar( '%' ) );    // percent
  res = res.replace( "%%u", "" );              // underline

  re.setPattern( QStringLiteral( "\\\\U\\+[0-9A-Fa-f]{4,4}" ) );
  for ( ;; )
  {
    const QRegularExpressionMatch m = re.match( res );
    if ( !m.hasMatch() )
      break;
    res.replace( m.captured( 1 ), QChar( m.captured( 1 ).right( 4 ).toInt( &ok, 16 ) ) );
  }

  re.setPattern( QStringLiteral( "%%[0-9]{3,3}" ) );
  for ( ;; )
  {
    const QRegularExpressionMatch m = re.match( res );
    if ( !m.hasMatch() )
      break;
    res.replace( m.captured( 1 ), QChar( m.captured( 1 ).mid( 2 ).toInt( &ok, 10 ) ) );
  }

  for ( ;; )
  {
    const QString prev( res );

    res = res.replace( QRegularExpression( "\\\\f[0-9A-Za-z| ]{0,};" ),                          QString( "" ) );            // font setting
    res = res.replace( QRegularExpression( "([^\\\\]|^){" ),                                     QStringLiteral( "\\1" ) );  // grouping
    res = res.replace( QRegularExpression( "([^\\\\])}" ),                                       QStringLiteral( "\\1" ) );
    res = res.replace( QRegularExpression( "([^\\\\]|^)\\\\[loLOkx]" ),                          QStringLiteral( "\\1" ) );  // underline, overstrike, strike through
    res = res.replace( QRegularExpression( "([^\\\\]|^)\\\\[HhWwAaCcQq]\\d*(\\.\\d*)?[xX]?;?" ), QStringLiteral( "\\1" ) );  // text height, width, alignment, color and slanting
    res = res.replace( QRegularExpression( "([^\\\\]|^)\\\\[ACQ]\\d+;" ),                        QStringLiteral( "\\1" ) );  // alignment, color and slanting

    if ( res == prev )
      break;
  }

  // QgsDebugMsg( QStringLiteral( "%1 => %2" ).arg( s.c_str() ).arg( res ) );
}
