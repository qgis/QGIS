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

#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QVector>
#include <QTransform>

#include <typeinfo>

#include <cpl_port.h>
#include <cpl_error.h>
#include <cpl_string.h>
#include <gdal.h>
#include <ogr_srs_api.h>
#include <memory>

#define LOG( x ) { QgsDebugMsg( x ); QgsMessageLog::logMessage( x, QObject::tr( "DWG/DXF import" ) ); }
#define ONCE( x ) { static bool show=true; if( show ) LOG( x ); show=false; }
#define NYI( x ) { static bool show=true; if( show ) LOG( QObject::tr("Not yet implemented %1").arg( x ) ); show=false; }
#define SETSTRING(a)  setString(dfn, f, #a, data.a)
#define SETSTRINGPTR(a)  setString(dfn, f.get(), #a, data.a)
#define SETDOUBLE(a)  setDouble(dfn, f, #a, data.a)
#define SETDOUBLEPTR(a)  setDouble(dfn, f.get(), #a, data.a)
#define SETINTEGER(a) setInteger(dfn, f, #a, data.a)
#define SETINTEGERPTR(a) setInteger(dfn, f.get(), #a, data.a)

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#endif


QgsDwgImporter::QgsDwgImporter( const QString &database, const QgsCoordinateReferenceSystem &crs )
  : mDs( nullptr )
  , mDatabase( database )
  , mInTransaction( false )
  , mSplineSegs( 8 )
  , mBlockHandle( -1 )
  , mCrs( crs.srsid() )
  , mCrsH( nullptr )
  , mUseCurves( true )
{
  QgsDebugCall;

  QString crswkt( crs.toWkt() );
  mCrsH = OSRNewSpatialReference( crswkt.toLocal8Bit().constData() );
  QgsDebugMsg( QString( "CRS %1[%2]: %3" ).arg( mCrs ).arg( ( qint64 ) mCrsH, 0, 16 ).arg( crswkt ) );
}

bool QgsDwgImporter::exec( const QString &sql, bool logError )
{
  if ( !mDs )
  {
    QgsDebugMsg( "No data source" );
    return false;
  }

  CPLErrorReset();

  OGRLayerH layer = OGR_DS_ExecuteSQL( mDs.get(), sql.toUtf8().constData(), nullptr, nullptr );
  if ( layer )
  {
    QgsDebugMsg( "Unexpected result set" );
    OGR_DS_ReleaseResultSet( mDs.get(), layer );
    return false;
  }

  if ( CPLGetLastErrorType() == CE_None )
    return true;

  if ( logError )
  {
    LOG( QObject::tr( "SQL statement failed\nDatabase: %1\nSQL: %2\nError: %3" )
         .arg( mDatabase, sql, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
  }
  return false;
}

OGRLayerH QgsDwgImporter::query( const QString &sql )
{
  if ( !mDs )
  {
    QgsDebugMsg( "No data source" );
    return nullptr;
  }

  CPLErrorReset();

  OGRLayerH layer = OGR_DS_ExecuteSQL( mDs.get(), sql.toUtf8().constData(), nullptr, nullptr );
  if ( !layer )
  {
    QgsDebugMsg( "Result expected" );
    return layer;
  }

  if ( CPLGetLastErrorType() == CE_None )
    return layer;

  LOG( QObject::tr( "SQL statement failed\nDatabase: %1\nSQL: %2\nError: %3" )
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
    LOG( QObject::tr( "Could not start transaction\nDatabase: %1\nError: %2" )
         .arg( mDatabase, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
  }
}

void QgsDwgImporter::commitTransaction()
{
  Q_ASSERT( mDs );

  if ( mInTransaction && GDALDatasetCommitTransaction( mDs.get() ) != OGRERR_NONE )
  {
    LOG( QObject::tr( "Could not commit transaction\nDatabase: %1\nError: %2" )
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
}

bool QgsDwgImporter::import( const QString &drawing, QString &error, bool doExpandInserts, bool useCurves )
{
  QgsDebugCall;

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

  QFileInfo fi( drawing );
  if ( !fi.isReadable() )
  {
    error = QObject::tr( "Drawing %1 is unreadable" ).arg( drawing );
    return false;
  }

  if ( QFileInfo::exists( mDatabase ) )
  {
    mDs.reset( OGROpen( mDatabase.toUtf8().constData(), true, nullptr ) );
    if ( !mDs )
    {
      LOG( QObject::tr( "Could not open database [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      return false;
    }

    // Check whether database is uptodate
    OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(), "drawing" );
    if ( !layer )
    {
      LOG( QObject::tr( "Query for drawing %1 failed." ).arg( drawing ) );
      mDs.reset();
      return false;
    }

    OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
    //int pathIdx = OGR_FD_GetFieldIndex( dfn, "path" );
    int lastmodifiedIdx = OGR_FD_GetFieldIndex( dfn, "lastmodified" );

    OGR_L_ResetReading( layer );

    gdal::ogr_feature_unique_ptr f( OGR_L_GetNextFeature( layer ) );
    if ( !f )
    {
      LOG( QObject::tr( "Could not retrieve drawing name from database [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      mDs.reset();
      return false;
    }

    int year, month, day, hour, minute, second, tzf;
    if ( !OGR_F_GetFieldAsDateTime( f.get(), lastmodifiedIdx, &year, &month, &day, &hour, &minute, &second, &tzf ) )
    {
      LOG( QObject::tr( "Recorded last modification date unreadable [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      mDs.reset();
      return false;
    }


#if 0
    QDateTime lastModified( QDate( year, month, day ), QTime( hour, minute, second ) );
    QString path = QString::fromUtf8( OGR_F_GetFieldAsString( f, pathIdx ) );
    if ( path == fi.canonicalPath() && fi.lastModified() <= lastModified )
    {
      LOG( QObject::tr( "Drawing already uptodate in database." ) );
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
      << field( "ocolor", OFTInteger) \
      << field( "color24", OFTInteger) \
      << field( "transparency", OFTInteger) \
      << field( "lweight", OFTInteger ) \
      << field( "linewidth", OFTReal ) \
      << field( "ltscale", OFTReal ) \
      << field( "visible", OFTInteger )


  QList<table> tables = QList<table>()
                        << table( QStringLiteral( "drawing" ), QObject::tr( "Imported drawings" ), wkbNone, QList<field>()
                                  << field( QStringLiteral( "path" ), OFTString )
                                  << field( QStringLiteral( "comments" ), OFTString )
                                  << field( QStringLiteral( "importdat" ), OFTDateTime )
                                  << field( QStringLiteral( "lastmodified" ), OFTDateTime )
                                  << field( QStringLiteral( "crs" ), OFTInteger )
                                )
                        << table( QStringLiteral( "headers" ), QObject::tr( "Headers" ), wkbNone, QList<field>()
                                  << field( QStringLiteral( "k" ), OFTString )
                                  << field( QStringLiteral( "v" ), OFTString )
                                )
                        << table( QStringLiteral( "linetypes" ), QObject::tr( "Line types" ), wkbNone, QList<field>()
                                  << field( QStringLiteral( "name" ), OFTString )
                                  << field( QStringLiteral( "desc" ), OFTString )
                                  << field( QStringLiteral( "path" ), OFTRealList )
                                )
                        << table( QStringLiteral( "layers" ), QObject::tr( "Layer list" ), wkbNone, QList<field>()
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
                        << table( QStringLiteral( "dimstyles" ), QObject::tr( "Dimension styles" ), wkbNone, QList<field>()
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
                        << table( QStringLiteral( "textstyles" ), QObject::tr( "Text styles" ), wkbNone, QList<field>()
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
                        << table( QStringLiteral( "appdata" ), QObject::tr( "Application data" ), wkbNone, QList<field>()
                                  << field( QStringLiteral( "handle" ), OFTInteger )
                                  << field( QStringLiteral( "i" ), OFTInteger )
                                  << field( QStringLiteral( "value" ), OFTString )
                                )
                        << table( QStringLiteral( "blocks" ), QObject::tr( "BLOCK entities" ), wkbPoint25D, QList<field>()
                                  ENTITY_ATTRIBUTES
                                  << field( QStringLiteral( "thickness" ), OFTReal )
                                  << field( QStringLiteral( "ext" ), OFTRealList )
                                  << field( QStringLiteral( "name" ), OFTString )
                                  << field( QStringLiteral( "flags" ), OFTInteger )
                                )
                        << table( QStringLiteral( "points" ), QObject::tr( "POINT entities" ), wkbPoint25D, QList<field>()
                                  ENTITY_ATTRIBUTES
                                  << field( QStringLiteral( "thickness" ), OFTReal )
                                  << field( QStringLiteral( "ext" ), OFTRealList )
                                )
                        << table( QStringLiteral( "lines" ), QObject::tr( "LINE entities" ), lineGeomType, QList<field>()
                                  ENTITY_ATTRIBUTES
                                  << field( QStringLiteral( "thickness" ), OFTReal )
                                  << field( QStringLiteral( "ext" ), OFTRealList )
                                  << field( QStringLiteral( "width" ), OFTReal )
                                )
                        << table( QStringLiteral( "polylines" ), QObject::tr( "POLYLINE entities" ), lineGeomType, QList<field>()
                                  ENTITY_ATTRIBUTES
                                  << field( QStringLiteral( "width" ), OFTReal )
                                  << field( QStringLiteral( "thickness" ), OFTReal )
                                  << field( QStringLiteral( "ext" ), OFTRealList )
                                )
                        << table( QStringLiteral( "texts" ), QObject::tr( "TEXT entities" ), wkbPoint25D, QList<field>()
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
                        << table( QStringLiteral( "hatches" ), QObject::tr( "HATCH entities" ), hatchGeomType, QList<field>()
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
                        << table( QStringLiteral( "inserts" ), QObject::tr( "INSERT entities" ), wkbPoint25D, QList<field>()
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
    LOG( QObject::tr( "Could not load geopackage driver" ) );
    return false;
  }

  // create database
  mDs.reset( OGR_Dr_CreateDataSource( driver, mDatabase.toUtf8().constData(), nullptr ) );
  if ( !mDs )
  {
    LOG( QObject::tr( "Creation of datasource failed [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    return false;
  }

  startTransaction();

  Q_FOREACH ( const table &t, tables )
  {
    char **options = nullptr;
    options = CSLSetNameValue( options, "OVERWRITE", "YES" );
    options = CSLSetNameValue( options, "DESCRIPTION", t.mDescription.toUtf8().constData() );
    if ( t.mWkbType == wkbNone )
    {
      options = CSLSetNameValue( options, "SPATIAL_INDEX", "NO" );
    }

    OGRLayerH layer = OGR_DS_CreateLayer( mDs.get(),  t.mName.toUtf8().constData(), ( t.mWkbType != wkbNone && mCrs > 0 ) ? mCrsH : nullptr, t.mWkbType, options );

    CSLDestroy( options );
    options = nullptr;

    if ( !layer )
    {
      LOG( QObject::tr( "Creation of drawing layer %1 failed [%2]" ).arg( t.mName, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      mDs.reset();
      return false;
    }

    Q_FOREACH ( const field &f, t.mFields )
    {
      gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( f.mName.toUtf8().constData(), f.mOgrType ) );
      if ( !fld )
      {
        LOG( QObject::tr( "Creation of field definition for %1.%2 failed [%3]" ).arg( t.mName, f.mName, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
        mDs.reset();
        return false;
      }

      if ( f.mWidth >= 0 )
        OGR_Fld_SetWidth( fld.get(), f.mWidth );
      if ( f.mPrecision >= 0 )
        OGR_Fld_SetPrecision( fld.get(), f.mPrecision );

      OGRErr res = OGR_L_CreateField( layer, fld.get(), true );

      if ( res != OGRERR_NONE )
      {
        LOG( QObject::tr( "Creation of field %1.%2 failed [%3]" ).arg( t.mName, f.mName, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
        mDs.reset();
        return false;
      }
    }
  }

  commitTransaction();

  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "drawing" );
  Q_ASSERT( layer );

  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  int pathIdx = OGR_FD_GetFieldIndex( dfn, "path" );
  int importdatIdx = OGR_FD_GetFieldIndex( dfn, "importdat" );
  int lastmodifiedIdx = OGR_FD_GetFieldIndex( dfn, "lastmodified" );
  int crsIdx = OGR_FD_GetFieldIndex( dfn, "crs" );

  gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );
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
    LOG( QObject::tr( "Could not update drawing record [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    return false;
  }

  LOG( QObject::tr( "Updating database from %1 [%2]." ).arg( drawing, fi.lastModified().toString() ) );

  DRW::error result( DRW::BAD_NONE );

  if ( fi.suffix().toLower() == QLatin1String( "dxf" ) )
  {
    //loads dxf
    std::unique_ptr<dxfRW> dxf( new dxfRW( drawing.toUtf8() ) );
    if ( !dxf->read( this, false ) )
    {
      result = DRW::BAD_UNKNOWN;
    }
  }
  else if ( fi.suffix().toLower() == QLatin1String( "dwg" ) )
  {
    //loads dwg
    std::unique_ptr<dwgR> dwg( new dwgR( drawing.toUtf8() ) );
    if ( !dwg->read( this, false ) )
    {
      result = dwg->getError();
    }
  }
  else
  {
    LOG( QObject::tr( "File %1 is not a DWG/DXF file" ).arg( drawing ) );
    return false;
  }

  switch ( result )
  {
    case DRW::BAD_NONE:
      error = QObject::tr( "No error." );
      break;
    case DRW::BAD_UNKNOWN:
      error = QObject::tr( "Unknown error." );
      break;
    case DRW::BAD_OPEN:
      error = QObject::tr( "error opening file." );
      break;
    case DRW::BAD_VERSION:
      error = QObject::tr( "unsupported version." );
      break;
    case DRW::BAD_READ_METADATA:
      error = QObject::tr( "error reading metadata." );
      break;
    case DRW::BAD_READ_FILE_HEADER:
      error = QObject::tr( "error in file header read process." );
      break;
    case DRW::BAD_READ_HEADER:
      error = QObject::tr( "error in header vars read process." );
      break;
    case DRW::BAD_READ_HANDLES:
      error = QObject::tr( "error in object map read process." );
      break;
    case DRW::BAD_READ_CLASSES:
      error = QObject::tr( "error in classes read process." );
      break;
    case DRW::BAD_READ_TABLES:
      error = QObject::tr( "error in tables read process." );
      result = DRW::BAD_NONE;
      break;
    case DRW::BAD_READ_BLOCKS:
      error = QObject::tr( "error in block read process." );
      break;
    case DRW::BAD_READ_ENTITIES:
      error = QObject::tr( "error in entities read process." );
      break;
    case DRW::BAD_READ_OBJECTS:
      error = QObject::tr( "error in objects read process." );
      break;
  }

  if ( result != DRW::BAD_NONE )
  {
    QgsDebugMsg( QString( "error:%1" ).arg( error ) );
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
    OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "drawing" );
    OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
    int importdatIdx = OGR_FD_GetFieldIndex( dfn, "comments" );

    OGR_L_ResetReading( layer );
    gdal::ogr_feature_unique_ptr f( OGR_L_GetNextFeature( layer ) );
    Q_ASSERT( f );

    OGR_F_SetFieldString( f.get(), importdatIdx, data->getComments().c_str() );

    if ( OGR_L_SetFeature( layer, f.get() ) != OGRERR_NONE )
    {
      LOG( QObject::tr( "Could not update comment in drawing record [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      return;
    }
  }

  if ( data->vars.empty() )
    return;

  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "headers" );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  int kIdx = OGR_FD_GetFieldIndex( dfn, "k" );
  int vIdx = OGR_FD_GetFieldIndex( dfn, "v" );

  for ( std::map<std::string, DRW_Variant *>::const_iterator it = data->vars.begin(); it != data->vars.end(); ++it )
  {
    gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );

    QString k = it->first.c_str();

    QString v;
    switch ( it->second->type() )
    {
      case DRW_Variant::STRING:
        v = *it->second->content.s->c_str();
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
      LOG( QObject::tr( "Could not add %3 %1 [%2]" )
           .arg( k,
                 QString::fromUtf8( CPLGetLastErrorMsg() ),
                 QObject::tr( "header record" ) )
         );
    }
  }
}

void QgsDwgImporter::addLType( const DRW_LType &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "linetypes" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );

  gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );
  Q_ASSERT( f );

  SETSTRINGPTR( name );
  SETSTRINGPTR( desc );

  QVector<double> path( QVector<double>::fromStdVector( data.path ) );
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
      NYI( QObject::tr( "dotted linetypes - dot ignored" ) );
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

  QString typeName( data.name.c_str() ), dash( QLatin1String( "" ) );
  if ( !upath.empty() )
  {
    QStringList l;
    if ( upath[0] < 0 )
      l << QStringLiteral( "0" );

    Q_FOREACH ( double p, upath )
    {
      l << QString::number( std::fabs( p ) );
    }

    if ( upath.size() % 2 == 1 )
      l << QStringLiteral( "0" );

    dash = l.join( QStringLiteral( ";" ) ).toUtf8().constData();
  }
  mLinetype.insert( typeName.toLower(), dash );

  if ( OGR_L_CreateFeature( layer, f.get() ) != OGRERR_NONE )
  {
    LOG( QObject::tr( "Could not add %3 %1 [%2]" )
         .arg( data.name.c_str(),
               QString::fromUtf8( CPLGetLastErrorMsg() ),
               QObject::tr( "line type" ) )
       );
  }
}

QString QgsDwgImporter::colorString( int color, int color24, int transparency, const std::string &layer ) const
{
  QgsDebugMsgLevel( QString( "colorString(color=%1, color24=0x%2, transparency=0x%3 layer=%4" )
                    .arg( color )
                    .arg( color24, 0, 16 )
                    .arg( transparency, 0, 16 )
                    .arg( layer.c_str() ), 5 );
  if ( color24 == -1 )
  {
    if ( color == 0 )
    {
      return QStringLiteral( "byblock" );
    }
    else if ( color == 256 )
    {
      return mLayerColor.value( layer.c_str(), QStringLiteral( "0,0,0,255" ) );
    }
    else
    {
      if ( color < 0 )
        color = -color;

      return QStringLiteral( "%1,%2,%3,%4" )
             .arg( DRW::dxfColors[ color ][0] )
             .arg( DRW::dxfColors[ color ][1] )
             .arg( DRW::dxfColors[ color ][2] )
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

QString QgsDwgImporter::linetypeString( const std::string &olinetype, const std::string &layer ) const
{
  QString linetype( olinetype.c_str() );

  if ( linetype == QLatin1String( "bylayer" ) )
    return mLayerLinetype.value( layer.c_str(), QLatin1String( "" ) );
  else
    return mLinetype.value( linetype, QLatin1String( "" ) );
}

void QgsDwgImporter::addLayer( const DRW_Layer &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "layers" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );
  Q_ASSERT( f );

  SETSTRINGPTR( name );
  SETSTRINGPTR( lineType );
  SETINTEGERPTR( flags );

  QString color = colorString( data.color, data.color24, data.transparency, "" );
  mLayerColor.insert( data.name.c_str(), color );

  double linewidth = lineWidth( data.lWeight, "" );
  if ( linewidth < 0. )
    linewidth = 0.;
  mLayerLinewidth.insert( data.name.c_str(), linewidth );
  mLayerLinetype.insert( data.name.c_str(), linetypeString( data.lineType, "" ) );

  setInteger( dfn, f.get(), QStringLiteral( "ocolor" ), data.color );
  SETINTEGERPTR( color24 );
  SETINTEGERPTR( transparency );
  setString( dfn, f.get(), QStringLiteral( "color" ), color.toUtf8().constData() );
  setInteger( dfn, f.get(), QStringLiteral( "lweight" ), DRW_LW_Conv::lineWidth2dxfInt( data.lWeight ) );
  setInteger( dfn, f.get(), QStringLiteral( "linewidth" ), linewidth );

  if ( OGR_L_CreateFeature( layer, f.get() ) != OGRERR_NONE )
  {
    LOG( QObject::tr( "Could not add %3 %1 [%2]" )
         .arg( data.name.c_str() )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "layer" ) )
       );
  }
}

void QgsDwgImporter::setString( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, const std::string &value ) const
{
  int idx = OGR_FD_GetFieldIndex( dfn, field.toLower().toUtf8().constData() );
  if ( idx < 0 )
  {
    LOG( QObject::tr( "Field %1 not found" ).arg( field ) );
    return;
  }
  OGR_F_SetFieldString( f, idx, value.c_str() );
}

void QgsDwgImporter::setDouble( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, double value ) const
{
  int idx = OGR_FD_GetFieldIndex( dfn, field.toLower().toUtf8().constData() );
  if ( idx < 0 )
  {
    LOG( QObject::tr( "Field %1 not found" ).arg( field ) );
    return;
  }
  OGR_F_SetFieldDouble( f, idx, value );
}

void QgsDwgImporter::setInteger( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, int value ) const
{
  int idx = OGR_FD_GetFieldIndex( dfn, field.toLower().toUtf8().constData() );
  if ( idx < 0 )
  {
    LOG( QObject::tr( "Field %1 not found" ).arg( field ) );
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

  int idx = OGR_FD_GetFieldIndex( dfn, field.toLower().toUtf8().constData() );
  if ( idx < 0 )
  {
    LOG( QObject::tr( "Field %1 not found" ).arg( field ) );
    return;
  }

  OGR_F_SetFieldDoubleList( f, idx, 3, ext.data() );
}

double QgsDwgImporter::lineWidth( int lWeight, const std::string &layer ) const
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
      return mLayerLinewidth.value( layer.c_str(), 0.0 );
    case 30: // byblock
      return -1.0;
    case 31:
    default:
      NYI( QObject::tr( "Line width default" ) );
      return 0.0;
  }
}

void QgsDwgImporter::addDimStyle( const DRW_Dimstyle &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "dimstyles" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );
  Q_ASSERT( f );

  SETSTRINGPTR( name );
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
    LOG( QObject::tr( "Could not add %3 %1 [%2]" )
         .arg( data.name.c_str() )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "dimension style" ) )
       );
  }
}

void QgsDwgImporter::addVport( const DRW_Vport &data )
{
  Q_UNUSED( data );
}

void QgsDwgImporter::addTextStyle( const DRW_Textstyle &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "textstyles" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );
  Q_ASSERT( f );

  SETSTRINGPTR( name );
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
    LOG( QObject::tr( "Could not add %3 %1 [%2]" )
         .arg( data.name.c_str() )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "text style" ) )
       );
  }
}

void QgsDwgImporter::addAppId( const DRW_AppId &data )
{
  Q_UNUSED( data );
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

  QByteArray wkb = g->asWkb();
  OGRGeometryH geom;
  if ( OGR_G_CreateFromWkb( ( unsigned char * ) wkb.constData(), nullptr, &geom, wkb.size() ) != OGRERR_NONE )
  {
    LOG( QObject::tr( "Could not create geometry [%1]" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
  }

  OGR_F_SetGeometryDirectly( f, geom );

  return OGR_L_CreateFeature( layer, f ) == OGRERR_NONE;
}


void QgsDwgImporter::addBlock( const DRW_Block &data )
{
  Q_ASSERT( mBlockHandle < 0 );
  mBlockHandle = data.handle;
  QgsDebugMsgLevel( QString( "block %1/0x%2 starts" ).arg( data.name.c_str() ).arg( mBlockHandle, 0, 16 ), 5 );

  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "blocks" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  gdal::ogr_feature_unique_ptr f( OGR_F_Create( dfn ) );
  Q_ASSERT( f );

  addEntity( dfn, f.get(), data );

  SETSTRINGPTR( name );
  SETINTEGERPTR( flags );

  QgsPoint p( QgsWkbTypes::PointZ, data.basePoint.x, data.basePoint.y, data.basePoint.z );

  if ( !createFeature( layer, f.get(), p ) )
  {
    LOG( QObject::tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "block" ) )
       );
  }
}

void QgsDwgImporter::setBlock( const int handle )
{
  Q_UNUSED( handle );
}

void QgsDwgImporter::endBlock()
{
  QgsDebugMsgLevel( QString( "block 0x%1 ended" ).arg( mBlockHandle, 0, 16 ), 5 );
  mBlockHandle = -1;
}

void QgsDwgImporter::addEntity( OGRFeatureDefnH dfn, OGRFeatureH f, const DRW_Entity &data )
{
  QgsDebugMsgLevel( QString( "handle:0x%1 block:0x%2" ).arg( data.handle, 0, 16 ).arg( mBlockHandle, 0, 16 ), 5 );
  SETINTEGER( handle );
  setInteger( dfn, f, QStringLiteral( "block" ), mBlockHandle );
  SETINTEGER( eType );
  SETINTEGER( space );
  SETSTRING( layer );
  setString( dfn, f, QStringLiteral( "olinetype" ), data.lineType );
  QString linetype = linetypeString( data.lineType, data.layer );
  if ( linetype == QLatin1String( "1" ) )
  {
    QgsDebugMsg( "Linetype == 1" );
  }
  setString( dfn, f, QStringLiteral( "linetype" ), linetype.toUtf8().constData() );
  setInteger( dfn, f, QStringLiteral( "ocolor" ), data.color );
  SETINTEGER( color24 );
  SETINTEGER( transparency );
  setString( dfn, f, QStringLiteral( "color" ), colorString( data.color, data.color24, data.transparency, data.layer ).toUtf8().constData() );
  setInteger( dfn, f, QStringLiteral( "lweight" ), DRW_LW_Conv::lineWidth2dxfInt( data.lWeight ) );
  setDouble( dfn, f, QStringLiteral( "linewidth" ), lineWidth( data.lWeight, data.layer ) );
  setInteger( dfn, f, QStringLiteral( "ltscale" ), data.ltypeScale );
  SETINTEGER( visible );
}

void QgsDwgImporter::addPoint( const DRW_Point &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "points" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  SETDOUBLE( thickness );

  setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

  QgsPoint p( QgsWkbTypes::PointZ, data.basePoint.x, data.basePoint.y, data.basePoint.z );
  if ( !createFeature( layer, f, p ) )
  {
    LOG( QObject::tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "point" ) )
       );
  }
}

void QgsDwgImporter::addRay( const DRW_Ray &data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "RAY entities" ) );
}

void QgsDwgImporter::addXline( const DRW_Xline &data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "XLINE entities" ) );
}

void QgsDwgImporter::addArc( const DRW_Arc &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "lines" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  SETDOUBLE( thickness );

  setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

  double half = ( data.staangle + data.endangle ) / 2.0;
  if ( data.staangle > data.endangle )
    half += M_PI;

  double a0 = data.isccw ? data.staangle : -data.staangle;
  double a1 = data.isccw ? half : -half;
  double a2 = data.isccw ? data.endangle : -data.endangle;

  QgsDebugMsgLevel( QString( "arc handle=0x%1 radius=%2 staangle=%3 endangle=%4 isccw=%5 half=%6" )
                    .arg( data.handle, 0, 16 ).arg( data.mRadius ).arg( data.staangle ).arg( data.endangle ).arg( data.isccw ).arg( half ), 5 );

  QgsCircularString c;
  c.setPoints( QgsPointSequence()
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x + std::cos( a0 ) * data.mRadius, data.basePoint.y + std::sin( a0 ) * data.mRadius )
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x + std::cos( a1 ) * data.mRadius, data.basePoint.y + std::sin( a1 ) * data.mRadius )
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x + std::cos( a2 ) * data.mRadius, data.basePoint.y + std::sin( a2 ) * data.mRadius )
             );

  if ( !createFeature( layer, f, c ) )
  {
    LOG( QObject::tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "arc" ) )
       );
  }
}

void QgsDwgImporter::addCircle( const DRW_Circle &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "lines" );
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
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x - data.mRadius, data.basePoint.y, data.basePoint.z )
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x + data.mRadius, data.basePoint.y, data.basePoint.z )
               << QgsPoint( QgsWkbTypes::PointZ, data.basePoint.x - data.mRadius, data.basePoint.y, data.basePoint.z )
             );

  if ( !createFeature( layer, f, c ) )
  {
    LOG( QObject::tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "circle" ) )
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
  size_t vertexnum = data.vertlist.size();
  if ( vertexnum == 0 )
  {
    QgsDebugMsg( "polyline without points" );
    return false;
  }

  QgsPointSequence s;
  bool hadBulge( data.vertlist[0]->bulge != 0.0 );
  std::vector<DRW_Vertex2D *>::size_type n = data.flags & 1 ? vertexnum + 1 : vertexnum;
  for ( std::vector<DRW_Vertex2D *>::size_type i = 0; i < n; i++ )
  {
    size_t i0 = i % vertexnum;

    Q_ASSERT( data.vertlist[i0] );
    QgsDebugMsgLevel( QString( "%1: %2,%3 bulge:%4" ).arg( i ).arg( data.vertlist[i0]->x ).arg( data.vertlist[i0]->y ).arg( data.vertlist[i0]->bulge ), 5 );

    QgsPoint p( QgsWkbTypes::PointZ, data.vertlist[i0]->x, data.vertlist[i0]->y, data.elevation );
    s << p;

    bool hasBulge( data.vertlist[i0]->bulge != 0.0 );

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
      size_t i1 = ( i + 1 ) % vertexnum;

      double a = 2.0 * std::atan( data.vertlist[i]->bulge );
      double dx = data.vertlist[i1]->x - data.vertlist[i0]->x;
      double dy = data.vertlist[i1]->y - data.vertlist[i0]->y;
      double c = std::sqrt( dx * dx + dy * dy );
      double r = c / 2.0 / std::sin( a );
      double h = r * ( 1 - std::cos( a ) );

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
  size_t vertexnum = data.vertlist.size();
  if ( vertexnum == 0 )
  {
    QgsDebugMsg( "LWPolyline without vertices" );
    return;
  }

  QgsPointSequence s;
  QgsCompoundCurve cc;
  double width = -1.0; // width is set to correct value during first loop
  bool hadBulge( false );

  std::vector<DRW_Vertex2D *>::size_type n = data.flags & 1 ? vertexnum : vertexnum - 1;
  for ( std::vector<DRW_Vertex2D *>::size_type i = 0; i < n; i++ )
  {
    size_t i0 = i % vertexnum;
    size_t i1 = ( i + 1 ) % vertexnum;

    QgsPoint p0( QgsWkbTypes::PointZ, data.vertlist[i0]->x, data.vertlist[i0]->y, data.elevation );
    QgsPoint p1( QgsWkbTypes::PointZ, data.vertlist[i1]->x, data.vertlist[i1]->y, data.elevation );
    double staWidth = data.vertlist[i0]->stawidth == 0.0 ? data.width : data.vertlist[i0]->stawidth;
    double endWidth = data.vertlist[i0]->endwidth == 0.0 ? data.width : data.vertlist[i0]->endwidth;
    bool hasBulge( data.vertlist[i0]->bulge != 0.0 );

    QgsDebugMsgLevel( QString( "i:%1,%2/%3 width=%4 staWidth=%5 endWidth=%6 hadBulge=%7 hasBulge=%8 l=%9 <=> %10" )
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
        // QgsDebugMsg( QString( "add circular string:%1" ).arg( c->asWkt() ) );
        cc.addCurve( c );
      }
      else
      {
        QgsLineString *c = new QgsLineString();
        c->setPoints( s );
        // QgsDebugMsg( QString( "add line string:%1" ).arg( c->asWkt() ) );
        cc.addCurve( c );
      }

      s.clear();

      if ( width != staWidth || width != endWidth )
      {
        // write out entity
        OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "polylines" );
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
          LOG( QObject::tr( "Could not add %2 [%1]" )
               .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
               .arg( QObject::tr( "line string" ) )
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
        double a = 2.0 * std::atan( data.vertlist[i]->bulge );
        double dx = p1.x() - p0.x();
        double dy = p1.y() - p0.y();
        double c = std::sqrt( dx * dx + dy * dy );
        double r = c / 2.0 / std::sin( a );
        double h = r * ( 1 - std::cos( a ) );

        s << QgsPoint( QgsWkbTypes::PointZ,
                       p0.x() + 0.5 * dx + h * dy / c,
                       p0.y() + 0.5 * dy - h * dx / c,
                       data.elevation );
      }

      s << p1;
    }
    else
    {
      OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "hatches" );
      Q_ASSERT( layer );
      OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
      Q_ASSERT( dfn );
      OGRFeatureH f = OGR_F_Create( dfn );
      Q_ASSERT( f );

      addEntity( dfn, f, data );

      SETDOUBLE( thickness );

      setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

      QgsPointXY ps( p0.x(), p0.y() );
      QgsPointXY pe( p1.x(), p1.y() );
      QgsVector v( ( pe - ps ).perpVector().normalized() );
      QgsVector vs( v * 0.5 * staWidth );
      QgsVector ve( v * 0.5 * endWidth );

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
      // QgsDebugMsg( QString( "write poly:%1" ).arg( poly.asWkt() ) );

      if ( !createFeature( layer, f, poly ) )
      {
        LOG( QObject::tr( "Could not add %2 [%1]" )
             .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
             .arg( QObject::tr( "polygon" ) )
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
      // QgsDebugMsg( QString( "add circular string:%1" ).arg( c->asWkt() ) );
      cc.addCurve( c );
    }
    else
    {
      QgsLineString *c = new QgsLineString();
      c->setPoints( s );
      // QgsDebugMsg( QString( "add line string:%1" ).arg( c->asWkt() ) );
      cc.addCurve( c );
    }
  }

  if ( cc.nCurves() > 0 )
  {
    // write out entity
    OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "polylines" );
    Q_ASSERT( layer );
    OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
    Q_ASSERT( dfn );
    OGRFeatureH f = OGR_F_Create( dfn );
    Q_ASSERT( f );

    addEntity( dfn, f, data );

    SETDOUBLE( thickness );
    SETDOUBLE( width );

    setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

    // QgsDebugMsg( QString( "write curve:%1" ).arg( cc.asWkt() ) );

    if ( !createFeature( layer, f, cc ) )
    {
      LOG( QObject::tr( "Could not add %2 [%1]" )
           .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
           .arg( QObject::tr( "line string" ) )
         );
    }
  }
}

void QgsDwgImporter::addPolyline( const DRW_Polyline &data )
{
  size_t vertexnum = data.vertlist.size();
  if ( vertexnum == 0 )
  {
    QgsDebugMsg( "Polyline without vertices" );
    return;
  }

  QgsPointSequence s;
  QgsCompoundCurve cc;
  double width = -1.0; // width is set to correct value during first loop
  bool hadBulge( false );

  std::vector<DRW_Vertex *>::size_type n = data.flags & 1 ? vertexnum : vertexnum - 1;
  for ( std::vector<DRW_Vertex *>::size_type i = 0; i < n; i++ )
  {
    size_t i0 = i % vertexnum;
    size_t i1 = ( i + 1 ) % vertexnum;

    QgsPoint p0( QgsWkbTypes::PointZ, data.vertlist[i0]->basePoint.x, data.vertlist[i0]->basePoint.y, data.vertlist[i0]->basePoint.z );
    QgsPoint p1( QgsWkbTypes::PointZ, data.vertlist[i1]->basePoint.x, data.vertlist[i1]->basePoint.y, data.vertlist[i1]->basePoint.z );
    double staWidth = data.vertlist[i0]->endwidth == 0.0 ? data.defendwidth : data.vertlist[i0]->stawidth;
    double endWidth = data.vertlist[i0]->stawidth == 0.0 ? data.defstawidth : data.vertlist[i0]->endwidth;
    bool hasBulge( data.vertlist[i0]->bulge != 0.0 );

    QgsDebugMsgLevel( QString( "i:%1,%2/%3 width=%4 staWidth=%5 endWidth=%6 hadBulge=%7 hasBulge=%8 l=%9 <=> %10" )
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
        // QgsDebugMsg( QString( "add circular string:%1" ).arg( c->asWkt() ) );
        cc.addCurve( c );
      }
      else
      {
        QgsLineString *c = new QgsLineString();
        c->setPoints( s );
        // QgsDebugMsg( QString( "add line string:%1" ).arg( c->asWkt() ) );
        cc.addCurve( c );
      }

      s.clear();

      if ( width != staWidth || width != endWidth )
      {
        // write out entity
        OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "polylines" );
        Q_ASSERT( layer );
        OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
        Q_ASSERT( dfn );
        OGRFeatureH f = OGR_F_Create( dfn );
        Q_ASSERT( f );

        addEntity( dfn, f, data );

        SETDOUBLE( thickness );
        setDouble( dfn, f, QStringLiteral( "width" ), width );

        setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

        // QgsDebugMsg( QString( "write curve:%1" ).arg( cc.asWkt() ) );

        if ( !createFeature( layer, f, cc ) )
        {
          LOG( QObject::tr( "Could not add %2 [%1]" )
               .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
               .arg( QObject::tr( "line string" ) )
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
        double a = 2.0 * std::atan( data.vertlist[i]->bulge );
        double dx = p1.x() - p0.x();
        double dy = p1.y() - p0.y();
        double dz = p1.z() - p0.z();
        double c = std::sqrt( dx * dx + dy * dy );
        double r = c / 2.0 / std::sin( a );
        double h = r * ( 1 - std::cos( a ) );

        s << QgsPoint( QgsWkbTypes::PointZ,
                       p0.x() + 0.5 * dx + h * dy / c,
                       p0.y() + 0.5 * dy - h * dx / c,
                       p0.z() + 0.5 * dz );
      }

      s << p1;
    }
    else
    {
      OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "hatches" );
      Q_ASSERT( layer );
      OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
      Q_ASSERT( dfn );
      OGRFeatureH f = OGR_F_Create( dfn );
      Q_ASSERT( f );

      addEntity( dfn, f, data );

      SETDOUBLE( thickness );

      setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

      QgsPointXY ps( p0.x(), p0.y() );
      QgsPointXY pe( p1.x(), p1.y() );
      QgsVector v( ( pe - ps ).perpVector().normalized() );
      QgsVector vs( v * 0.5 * staWidth );
      QgsVector ve( v * 0.5 * endWidth );

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
      // QgsDebugMsg( QString( "write poly:%1" ).arg( poly.asWkt() ) );

      if ( !createFeature( layer, f, poly ) )
      {
        LOG( QObject::tr( "Could not add %2 [%1]" )
             .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
             .arg( QObject::tr( "polygon" ) )
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
      // QgsDebugMsg( QString( "add circular string:%1" ).arg( c->asWkt() ) );
      cc.addCurve( c );
    }
    else
    {
      QgsLineString *c = new QgsLineString();
      c->setPoints( s );
      // QgsDebugMsg( QString( "add line string:%1" ).arg( c->asWkt() ) );
      cc.addCurve( c );
    }
  }

  if ( cc.nCurves() > 0 )
  {
    // write out entity
    OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "polylines" );
    Q_ASSERT( layer );
    OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
    Q_ASSERT( dfn );
    OGRFeatureH f = OGR_F_Create( dfn );
    Q_ASSERT( f );

    addEntity( dfn, f, data );

    SETDOUBLE( thickness );
    setDouble( dfn, f, QStringLiteral( "width" ), width );

    setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

    // QgsDebugMsg( QString( "write curve:%1" ).arg( cc.asWkt() ) );

    if ( !createFeature( layer, f, cc ) )
    {
      LOG( QObject::tr( "Could not add %2 [%1]" )
           .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
           .arg( QObject::tr( "line string" ) )
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
      v[ order + i ] = i + 1;

    for ( size_t i = num + 1; i < v.size(); ++i )
      v[ i ] = v[num];

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
  size_t nplusc = npts + c;
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
    temp[ npts - 1 ] = 1;

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
  size_t nplusc = npts + k;

  // generate the open knot vector
  std::vector<double> x( knot( data, npts, k ) );

  // calculate the points on the rational B-spline curve
  double t = 0.;

  double step = x[nplusc - 1] / ( p1 - 1 );
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

void QgsDwgImporter::addSpline( const DRW_Spline *data )
{
  Q_ASSERT( data );

  if ( data->degree < 1 || data->degree > 3 )
  {
    QgsDebugMsg( QString( "%1: unknown spline degree %2" )
                 .arg( data->handle, 0, 16 )
                 .arg( data->degree ) );
    return;
  }

  QgsDebugMsgLevel( QString( "degree: %1 ncontrol:%2 knotslist.size():%3 controllist.size():%4 fitlist.size():%5" )
                    .arg( data->degree )
                    .arg( data->ncontrol )
                    .arg( data->knotslist.size() )
                    .arg( data->controllist.size() )
                    .arg( data->fitlist.size() ), 5 );

  std::vector<QgsVector> cps;
  for ( size_t i = 0; i < data->controllist.size(); ++i )
  {
    const DRW_Coord &p = *data->controllist[i];
    cps.emplace_back( QgsVector( p.x, p.y ) );
  }

  if ( data->ncontrol == 0 && data->degree != 2 )
  {
    for ( std::vector<DRW_Coord *>::size_type i = 0; i < data->fitlist.size(); ++i )
    {
      const DRW_Coord &p = *data->fitlist[i];
      cps.emplace_back( QgsVector( p.x, p.y ) );
    }
  }

  if ( !cps.empty() && data->flags & 1 )
  {
    for ( int i = 0; i < data->degree; ++i )
      cps.push_back( cps[i] );
  }

  size_t npts = cps.size();
  size_t k = data->degree + 1;
  int p1 = mSplineSegs * ( int ) npts;

  std::vector<double> h( npts + 1, 1. );
  std::vector<QgsPointXY> p( p1, QgsPointXY( 0., 0. ) );

  if ( data->flags & 1 )
  {
    rbsplinu( *data, npts, k, p1, cps, h, p );
  }
  else
  {
    rbspline( *data, npts, k, p1, cps, h, p );
  }

  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "polylines" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, *data );

  QgsLineString l;
  QgsPointSequence ps;
  for ( size_t i = 0; i < p.size(); ++i )
    ps << QgsPoint( p[i] );
  l.setPoints( ps );

  if ( !createFeature( layer, f, l ) )
  {
    LOG( QObject::tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "spline" ) )
       );
  }
}

void QgsDwgImporter::addKnot( const DRW_Entity &data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "KNOT entities" ) );
}

void QgsDwgImporter::addInsert( const DRW_Insert &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "inserts" );
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

  QgsPoint p( QgsWkbTypes::PointZ, data.basePoint.x, data.basePoint.y, data.basePoint.z );

  if ( !createFeature( layer, f, p ) )
  {
    LOG( QObject::tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "point" ) )
       );
  }
}

void QgsDwgImporter::addTrace( const DRW_Trace &data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "TRACE entities" ) );
}

void QgsDwgImporter::add3dFace( const DRW_3Dface &data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "3DFACE entities" ) );
}

void QgsDwgImporter::addSolid( const DRW_Solid &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "hatches" );
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
  s << QgsPoint( QgsWkbTypes::PointZ, data.fourthPoint.x, data.fourthPoint.y, data.basePoint.z );
  s << QgsPoint( QgsWkbTypes::PointZ,  data.thirdPoint.x,  data.thirdPoint.y, data.basePoint.z );
  s << s[0];

  QgsLineString *ls = new QgsLineString();
  ls->setPoints( s );
  poly.setExteriorRing( ls );

  if ( !createFeature( layer, f, poly ) )
  {
    LOG( QObject::tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "polygon" ) )
       );
  }
}

void QgsDwgImporter::addMText( const DRW_MText &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "texts" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  SETDOUBLE( height );
  SETSTRING( text );
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

  QgsPoint p( QgsWkbTypes::PointZ, data.basePoint.x, data.basePoint.y, data.basePoint.z );

  if ( !createFeature( layer, f, p ) )
  {
    LOG( QObject::tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "point" ) )
       );
  }
}

void QgsDwgImporter::addText( const DRW_Text &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "texts" );
  Q_ASSERT( layer );
  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( layer );
  Q_ASSERT( dfn );
  OGRFeatureH f = OGR_F_Create( dfn );
  Q_ASSERT( f );

  addEntity( dfn, f, data );

  SETDOUBLE( height );
  SETSTRING( text );
  SETDOUBLE( angle );
  SETDOUBLE( widthscale );
  SETDOUBLE( oblique );
  SETSTRING( style );
  SETINTEGER( textgen );
  SETINTEGER( alignH );
  SETINTEGER( alignV );
  SETDOUBLE( thickness );
  setDouble( dfn, f, QStringLiteral( "interlin" ), -1.0 );

  setPoint( dfn, f, QStringLiteral( "ext" ), data.extPoint );

  QgsPoint p( QgsWkbTypes::PointZ,
              ( data.alignH > 0 || data.alignV > 0 ) ? data.secPoint.x : data.basePoint.x,
              ( data.alignH > 0 || data.alignV > 0 ) ? data.secPoint.y : data.basePoint.y,
              ( data.alignH > 0 || data.alignV > 0 ) ? data.secPoint.z : data.basePoint.z );

  if ( !createFeature( layer, f, p ) )
  {
    LOG( QObject::tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "point" ) )
       );
  }
}

void QgsDwgImporter::addDimAlign( const DRW_DimAligned *data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "DIMALIGN entities" ) );
}

void QgsDwgImporter::addDimLinear( const DRW_DimLinear *data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "DIMLINEAR entities" ) );
}

void QgsDwgImporter::addDimRadial( const DRW_DimRadial *data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "DIMRADIAL entities" ) );
}

void QgsDwgImporter::addDimDiametric( const DRW_DimDiametric *data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "DIMDIAMETRIC entities" ) );
}

void QgsDwgImporter::addDimAngular( const DRW_DimAngular *data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "DIMANGULAR entities" ) );
}

void QgsDwgImporter::addDimAngular3P( const DRW_DimAngular3p *data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "DIMANGULAR3P entities" ) );
}

void QgsDwgImporter::addDimOrdinate( const DRW_DimOrdinate *data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "DIMORDINAL entities" ) );
}

void QgsDwgImporter::addLeader( const DRW_Leader *data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "LEADER entities" ) );
}

void QgsDwgImporter::addHatch( const DRW_Hatch *pdata )
{
  Q_ASSERT( pdata );
  const DRW_Hatch &data = *pdata;

  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "hatches" );
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

  Q_ASSERT( data.looplist.size() == data.loopsnum );

  for ( std::vector<DRW_HatchLoop *>::size_type i = 0; i < data.loopsnum; i++ )
  {
    const DRW_HatchLoop &hatchLoop = *data.looplist[i];

    QgsCompoundCurve *cc = new QgsCompoundCurve();

    for ( std::vector<DRW_Entity *>::size_type j = 0;  j < hatchLoop.objlist.size(); j++ )
    {
      Q_ASSERT( hatchLoop.objlist[j] );
      const DRW_Entity *entity = hatchLoop.objlist[j];

      const DRW_LWPolyline *lwp = dynamic_cast<const DRW_LWPolyline *>( entity );
      const DRW_Line *l = dynamic_cast<const DRW_Line *>( entity );
      if ( lwp )
      {
        curveFromLWPolyline( *lwp, *cc );
      }
      else if ( l )
      {
        QgsLineString *ls = new QgsLineString();
        ls->setPoints( QgsPointSequence()
                       << QgsPoint( QgsWkbTypes::PointZ, l->basePoint.x, l->basePoint.y, l->basePoint.z )
                       << QgsPoint( QgsWkbTypes::PointZ, l->secPoint.x, l->secPoint.y, l->secPoint.z ) );
        // QgsDebugMsg( QString( "add line string:%1" ).arg( ls->asWkt() ) );
        cc->addCurve( ls );
      }
      else
      {
        QgsDebugMsg( QString( "unknown obj %1.%2: %3" ).arg( i ).arg( j ).arg( typeid( *entity ).name() ) );
      }
    }

    if ( i == 0 )
    {
      // QgsDebugMsg( QString( "set exterior ring:%1" ).arg( cc->asWkt() ) );
      p.setExteriorRing( cc );
    }
    else
    {
      // QgsDebugMsg( QString( "set interior ring:%1" ).arg( cc->asWkt() ) );
      p.addInteriorRing( cc );
    }
  }

  if ( !createFeature( layer, f, p ) )
  {
    LOG( QObject::tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "polygon" ) )
       );
  }
}

void QgsDwgImporter::addLine( const DRW_Line &data )
{
  OGRLayerH layer = OGR_DS_GetLayerByName( mDs.get(),  "lines" );
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
    LOG( QObject::tr( "Could not add %2 [%1]" )
         .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) )
         .arg( QObject::tr( "line string" ) )
       );
  }
}

void QgsDwgImporter::addViewport( const DRW_Viewport &data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "VIEWPORT entities" ) );
}

void QgsDwgImporter::addImage( const DRW_Image *data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "IMAGE entities" ) );
}

void QgsDwgImporter::linkImage( const DRW_ImageDef *data )
{
  Q_UNUSED( data );
  NYI( QObject::tr( "image links" ) );
}

void QgsDwgImporter::addComment( const char *comment )
{
  Q_UNUSED( comment );
  NYI( QObject::tr( "comments" ) );
}

void QgsDwgImporter::writeHeader( DRW_Header & ) { }
void QgsDwgImporter::writeBlocks() { }
void QgsDwgImporter::writeBlockRecords() { }
void QgsDwgImporter::writeEntities() { }
void QgsDwgImporter::writeLTypes() { }
void QgsDwgImporter::writeLayers() { }
void QgsDwgImporter::writeTextstyles() { }
void QgsDwgImporter::writeVports() { }
void QgsDwgImporter::writeDimstyles() { }
void QgsDwgImporter::writeAppId() { }

bool QgsDwgImporter::expandInserts( QString &error )
{
  QgsDebugCall;

  OGRLayerH blocks = OGR_DS_GetLayerByName( mDs.get(),  "blocks" );
  if ( !blocks )
  {
    QgsDebugMsg( "could not open layer 'blocks'" );
    return false;
  }

  OGRFeatureDefnH dfn = OGR_L_GetLayerDefn( blocks );
  Q_ASSERT( dfn );

  int nameIdx = OGR_FD_GetFieldIndex( dfn, "name" );
  int handleIdx = OGR_FD_GetFieldIndex( dfn, "handle" );
  if ( nameIdx < 0 || handleIdx < 0 )
  {
    QgsDebugMsg( QString( "not all fields found (nameIdx=%1 handleIdx=%2)" ).arg( nameIdx ).arg( handleIdx ) );
    return false;
  }

  QHash<QString, int> blockhandle;

  OGR_L_ResetReading( blocks );

  gdal::ogr_feature_unique_ptr f;
  for ( ;; )
  {
    f.reset( OGR_L_GetNextFeature( blocks ) );
    if ( !f )
      break;

    QString name = QString::fromUtf8( OGR_F_GetFieldAsString( f.get(), nameIdx ) );
    int handle = OGR_F_GetFieldAsInteger( f.get(), handleIdx );
    blockhandle.insert( name, handle );
  }

  OGRLayerH inserts = OGR_DS_GetLayerByName( mDs.get(),  "inserts" );
  if ( !inserts )
  {
    QgsDebugMsg( "could not open layer 'inserts'" );
    return false;
  }

  dfn = OGR_L_GetLayerDefn( inserts );
  Q_ASSERT( dfn );

  nameIdx = OGR_FD_GetFieldIndex( dfn, "name" );
  int xscaleIdx = OGR_FD_GetFieldIndex( dfn, "xscale" );
  int yscaleIdx = OGR_FD_GetFieldIndex( dfn, "yscale" );
  int zscaleIdx = OGR_FD_GetFieldIndex( dfn, "zscale" );
  int angleIdx = OGR_FD_GetFieldIndex( dfn, "angle" );
  int layerIdx = OGR_FD_GetFieldIndex( dfn, "layer" );
  int colorIdx = OGR_FD_GetFieldIndex( dfn, "color" );
  int linetypeIdx = OGR_FD_GetFieldIndex( dfn, "linetype" );
  int linewidthIdx = OGR_FD_GetFieldIndex( dfn, "linewidth" );
  if ( xscaleIdx < 0 || yscaleIdx < 0 || zscaleIdx < 0 || angleIdx < 0 || nameIdx < 0 || layerIdx < 0 || linetypeIdx < 0 || colorIdx < 0 || linewidthIdx < 0 )
  {
    QgsDebugMsg( QString( "not all fields found (nameIdx=%1 xscaleIdx=%2 yscaleIdx=%3 zscaleIdx=%4 angleIdx=%5 layerIdx=%6 linetypeIdx=%7 color=%8 linewidthIdx=%9)" )
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

  GIntBig n = OGR_L_GetFeatureCount( inserts, 0 );
  Q_UNUSED( n );

  OGR_L_ResetReading( inserts );

  gdal::ogr_feature_unique_ptr insert;
  int i = 0, errors = 0;
  for ( int i = 0; true; ++i )
  {
    if ( i % 1000 == 0 )
    {
      QgsDebugMsg( QString( "Expanding inserts %1/%2..." ).arg( i ).arg( n ) );
    }

    insert.reset( OGR_L_GetNextFeature( inserts ) );
    if ( !insert )
      break;

    OGRGeometryH ogrG = OGR_F_GetGeometryRef( insert.get() );
    if ( !ogrG )
    {
      QgsDebugMsg( QString( "%1: insert without geometry" ).arg( OGR_F_GetFID( insert.get() ) ) );
      continue;
    }

    QgsGeometry g( QgsOgrUtils::ogrGeometryToQgsGeometry( ogrG ) );
    if ( g.isNull() )
    {
      QgsDebugMsg( QString( "%1: could not copy geometry" ).arg( OGR_F_GetFID( insert.get() ) ) );
      continue;
    }

    QgsPointXY p( g.asPoint() );

    QString name = QString::fromUtf8( OGR_F_GetFieldAsString( insert.get(), nameIdx ) );
    double xscale = OGR_F_GetFieldAsDouble( insert.get(), xscaleIdx );
    double yscale = OGR_F_GetFieldAsDouble( insert.get(), yscaleIdx );
    double angle = OGR_F_GetFieldAsDouble( insert.get(), angleIdx );
    QString blockLayer = QString::fromUtf8( OGR_F_GetFieldAsString( insert.get(), layerIdx ) );
    QString blockColor = QString::fromUtf8( OGR_F_GetFieldAsString( insert.get(), colorIdx ) );
    QString blockLinetype = QString::fromUtf8( OGR_F_GetFieldAsString( insert.get(), linetypeIdx ) );
    if ( blockLinetype == QLatin1String( "1" ) )
    {
      QgsDebugMsg( "blockLinetype == 1" );
    }
    double blockLinewidth = OGR_F_GetFieldAsDouble( insert.get(), linewidthIdx );

    int handle = blockhandle.value( name, -1 );
    if ( handle < 0 )
    {
      QgsDebugMsg( QString( "Block '%1' not found" ).arg( name ) );
      continue;
    }

    QgsDebugMsgLevel( QString( "Resolving %1/%2: p=%3,%4 scale=%5,%6 angle=%7" )
                      .arg( name ).arg( handle, 0, 16 )
                      .arg( p.x() ).arg( p.y() )
                      .arg( xscale ).arg( yscale ).arg( angle ), 5 );

    QTransform t;
    t.translate( p.x(), p.y() ).scale( xscale, yscale ).rotateRadians( angle );

    Q_FOREACH ( const QString &name, QStringList() << "hatches" << "lines" << "polylines" << "texts" << "points" )
    {
      OGRLayerH src = OGR_DS_ExecuteSQL( mDs.get(),  QStringLiteral( "SELECT * FROM %1 WHERE block=%2" ).arg( name ).arg( handle ).toUtf8().constData(), nullptr, nullptr );
      if ( !src )
      {
        QgsDebugMsg( QString( "%1: could not execute query for block %2" ).arg( name ).arg( handle ) );
        continue;
      }

      OGRLayerH dst = OGR_DS_GetLayerByName( mDs.get(),  name.toUtf8().constData() );
      Q_ASSERT( dst );

      dfn = OGR_L_GetLayerDefn( src );
      Q_ASSERT( dfn );

      int blockIdx = OGR_FD_GetFieldIndex( dfn, "block" );
      int layerIdx = OGR_FD_GetFieldIndex( dfn, "layer" );
      int colorIdx = OGR_FD_GetFieldIndex( dfn, "color" );
      if ( blockIdx < 0 || layerIdx < 0 || colorIdx < 0 )
      {
        QgsDebugMsg( QString( "%1: fields not found (blockIdx=%2, layerIdx=%3 colorIdx=%4)" )
                     .arg( name ).arg( blockIdx ).arg( layerIdx ).arg( colorIdx )
                   );
        OGR_DS_ReleaseResultSet( mDs.get(),  src );
        continue;
      }

      int linetypeIdx = OGR_FD_GetFieldIndex( dfn, "linetype" );
      int linewidthIdx = OGR_FD_GetFieldIndex( dfn, "linewidth" );
      int angleIdx = OGR_FD_GetFieldIndex( dfn, "angle" );

      OGR_L_ResetReading( src );

      gdal::ogr_feature_unique_ptr f;
      int j = 0;
      for ( ;; )
      {
        f.reset( OGR_L_GetNextFeature( src ) );
        if ( !f )
          break;

        GIntBig fid = OGR_F_GetFID( f.get() );
        Q_UNUSED( fid );

        ogrG = OGR_F_GetGeometryRef( f.get() );
        if ( !ogrG )
        {
          QgsDebugMsg( QString( "%1/%2: geometryless feature skipped" ).arg( name ).arg( fid ) );
          continue;
        }

        QgsGeometry g( QgsOgrUtils::ogrGeometryToQgsGeometry( ogrG ) );
        if ( g.isNull() )
        {
          QgsDebugMsg( QString( "%1: could not copy geometry" ).arg( fid ) );
          continue;
        }

        if ( g.transform( t ) != 0 )
        {
          QgsDebugMsg( QString( "%1/%2: could not transform geometry" ).arg( name ).arg( fid ) );
          continue;
        }

        QByteArray wkb = g.constGet()->asWkb();
        if ( OGR_G_CreateFromWkb( ( unsigned char * ) wkb.constData(), nullptr, &ogrG, wkb.size() ) != OGRERR_NONE )
        {
          QgsDebugMsg( QString( "%1/%2: could not create ogr geometry" ).arg( name ).arg( fid ) );
          continue;
        }

        if ( OGR_F_SetGeometryDirectly( f.get(), ogrG ) != OGRERR_NONE )
        {
          QgsDebugMsg( QString( "%1/%2: could not assign geometry" ).arg( name ).arg( fid ) );
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
            QgsMessageLog::logMessage( QObject::tr( "Could not copy feature of block %2 from layer %1 [Errors: %3]" )
                                       .arg( name ).arg( handle ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ),
                                       QObject::tr( "DWG/DXF import" ) );
          }
          else if ( errors == 1000 )
          {
            QgsMessageLog::logMessage( QObject::tr( "Not logging more errors" ), QObject::tr( "DWG/DXF import" ) );
          }

          ++errors;
          continue;
        }

        ++j;
      }

      OGR_DS_ReleaseResultSet( mDs.get(),  src );

      QgsDebugMsgLevel( QString( "%1: %2 features copied" ).arg( name ).arg( j ), 5 );
    }
  }

  if ( errors > 0 )
  {
    error = QObject::tr( "%1 write errors during block expansion" ).arg( errors );
    return false;
  }
  else
  {
    error = QObject::tr( "%1 block insertion expanded." ).arg( i );
    return true;
  }
}
