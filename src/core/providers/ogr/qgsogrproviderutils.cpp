/***************************************************************************
                     qgsogrproviderutils.cpp
begin                : June 2021
copyright            : (C) 2021 by Nyall Dawson
email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrproviderutils.h"
#include "qgsogrprovidermetadata.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgsvectorfilewriter.h"
#include "qgsauthmanager.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderregistry.h"
#include "qgsgeopackageproviderconnection.h"
#include "qgsogrdbconnection.h"
#include "qgsfileutils.h"

#include <ogr_srs_api.h>
#include <cpl_port.h>

#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QStorageInfo>
#include <QRegularExpression>
#include <QFileDialog>
#include <QInputDialog>

#ifdef Q_OS_WIN
#include <windows.h>
#endif
#ifdef Q_OS_LINUX
#include <sys/vfs.h>
#endif

///@cond PRIVATE

static bool IsLocalFile( const QString &path );

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
Q_GLOBAL_STATIC_WITH_ARGS( QMutex, sGlobalMutex, ( QMutex::Recursive ) )
#else
Q_GLOBAL_STATIC( QRecursiveMutex, sGlobalMutex )
#endif


//! Map a dataset name to the number of opened GDAL dataset objects on it (if opened with GDALOpenWrapper, only for GPKG)
typedef QMap< QString, int > OpenedDsCountMap;
Q_GLOBAL_STATIC( OpenedDsCountMap, sMapCountOpenedDS )

QMap< QgsOgrProviderUtils::DatasetIdentification,
      QList<QgsOgrProviderUtils::DatasetWithLayers *> > QgsOgrProviderUtils::sMapSharedDS;

typedef QHash< GDALDatasetH, bool> DsHandleToUpdateModeHash;
Q_GLOBAL_STATIC( DsHandleToUpdateModeHash, sMapDSHandleToUpdateMode )

typedef QMap< QString, QDateTime > DsNameToLastModifiedDateMap;
Q_GLOBAL_STATIC( DsNameToLastModifiedDateMap, sMapDSNameToLastModifiedDate )

QString QgsOgrProviderUtils::analyzeURI( QString const &uri,
    bool &isSubLayer,
    int &layerIndex,
    QString &layerName,
    QString &subsetString,
    OGRwkbGeometryType &ogrGeometryTypeFilter,
    QStringList &openOptions )
{
  isSubLayer = false;
  layerIndex = 0;
  layerName = QString();
  subsetString = QString();
  ogrGeometryTypeFilter = wkbUnknown;
  openOptions.clear();

  QgsDebugMsgLevel( "Data source uri is [" + uri + ']', 2 );

  QVariantMap parts = QgsOgrProviderMetadata().decodeUri( uri );

  if ( parts.contains( QStringLiteral( "layerName" ) ) )
  {
    layerName = parts.value( QStringLiteral( "layerName" ) ).toString();
    isSubLayer = !layerName.isEmpty();
  }

  if ( parts.contains( QStringLiteral( "layerId" ) ) &&
       parts.value( QStringLiteral( "layerId" ) ) != QVariant() )
  {
    bool ok;
    layerIndex = parts.value( QStringLiteral( "layerId" ) ).toInt( &ok );
    if ( ok && layerIndex >= 0 )
      isSubLayer = true;
    else
      layerIndex = -1;
  }

  if ( parts.contains( QStringLiteral( "subset" ) ) )
  {
    subsetString = parts.value( QStringLiteral( "subset" ) ).toString();
  }

  if ( parts.contains( QStringLiteral( "geometryType" ) ) )
  {
    ogrGeometryTypeFilter = ogrWkbGeometryTypeFromName( parts.value( QStringLiteral( "geometryType" ) ).toString() );
  }

  if ( parts.contains( QStringLiteral( "openOptions" ) ) )
  {
    openOptions = parts.value( QStringLiteral( "openOptions" ) ).toStringList();
  }

  const QString fullPath = parts.value( QStringLiteral( "vsiPrefix" ) ).toString()
                           + parts.value( QStringLiteral( "path" ) ).toString()
                           + parts.value( QStringLiteral( "vsiSuffix" ) ).toString();
  return fullPath;
}

void QgsOgrProviderUtils::setRelevantFields( OGRLayerH ogrLayer, int fieldCount,
    bool fetchGeometry,
    const QgsAttributeList &fetchAttributes,
    bool firstAttrIsFid,
    const QString &subsetString )
{
  if ( OGR_L_TestCapability( ogrLayer, OLCIgnoreFields ) )
  {
    QVector<const char *> ignoredFields;
    OGRFeatureDefnH featDefn = OGR_L_GetLayerDefn( ogrLayer );
    for ( int i = ( firstAttrIsFid ? 1 : 0 ); i < fieldCount; i++ )
    {
      if ( !fetchAttributes.contains( i ) )
      {
        // add to ignored fields
        if ( OGRFieldDefnH field = OGR_FD_GetFieldDefn( featDefn, firstAttrIsFid ? i - 1 : i ) )
        {
          const char *fieldName = OGR_Fld_GetNameRef( field );
          // This is implemented a bit in a hacky way, but in case we are acting on a layer
          // with a subset filter, do not ignore fields that are found in the
          // where clause. We do this in a rough way, by looking, in a case
          // insensitive way, if the current field name is in the subsetString,
          // so we potentially don't ignore fields we could, in situations like
          // subsetFilter == "foobar = 2", and there's a "foo" or "bar" field.
          // Better be safe than sorry.
          // We could argue that OGR_L_SetIgnoredFields() should be aware of
          // the fields of the attribute filter, and do not ignore them.
          if ( subsetString.isEmpty() ||
               subsetString.indexOf( QString::fromUtf8( fieldName ), 0, Qt::CaseInsensitive ) < 0 )
          {
            ignoredFields.append( fieldName );
          }
        }
      }
    }

    if ( !fetchGeometry )
      ignoredFields.append( "OGR_GEOMETRY" );
    ignoredFields.append( "OGR_STYLE" ); // not used by QGIS
    ignoredFields.append( nullptr );

    OGR_L_SetIgnoredFields( ogrLayer, ignoredFields.data() );
  }
}

/**
 *  Convenience function for readily creating file filters.
 *
 *  Given a long name for a file filter and a regular expression, return
 *  a file filter string suitable for use in a QFileDialog::OpenFiles()
 *  call.  The regular express, glob, will have both all lower and upper
 *  case versions added.
 *  \note Copied from qgisapp.cpp.
 *  \todo XXX This should probably be generalized and moved to a standard utility type thingy.
*/
static QString createFileFilter_( QString const &longName, QString const &glob )
{
  // return longName + " [OGR] (" + glob.toLower() + ' ' + glob.toUpper() + ");;";
  return longName + " (" + glob.toLower() + ' ' + glob.toUpper() + ");;";
}

QString createFilters( const QString &type )
{
  //! Database drivers available
  static QString sDatabaseDrivers;
  //! Protocol drivers available
  static QString sProtocolDrivers;
  //! File filters
  static QString sFileFilters;
  //! Directory drivers
  static QString sDirectoryDrivers;
  //! Extensions
  static QStringList sExtensions;
  //! Directory extensions
  static QStringList sDirectoryExtensions;
  //! Wildcards
  static QStringList sWildcards;

  // if we've already built the supported vector string, just return what
  // we've already built

  if ( sFileFilters.isEmpty() || sFileFilters.isNull() )
  {
    // register ogr plugins
    QgsApplication::registerOgrDrivers();

    // first get the GDAL driver manager
    GDALDriverH driver;          // current driver
    QString driverName;           // current driver name

    // Grind through all the drivers and their respective metadata.
    // We'll add a file filter for those drivers that have a file
    // extension defined for them; the others, welll, even though
    // theoreticaly we can open those files because there exists a
    // driver for them, the user will have to use the "All Files" to
    // open datasets with no explicitly defined file name extension.
    QgsDebugMsgLevel( QStringLiteral( "Driver count: %1" ).arg( OGRGetDriverCount() ), 3 );

    bool kmlFound = false;
    bool dwgFound = false;
    bool dgnFound = false;

    for ( int i = 0; i < OGRGetDriverCount(); ++i )
    {
      driver = OGRGetDriver( i );

      Q_CHECK_PTR( driver ); // NOLINT

      if ( !driver )
      {
        QgsMessageLog::logMessage( QObject::tr( "Unable to get driver %1" ).arg( i ), QObject::tr( "OGR" ) );
        continue;
      }

      driverName = GDALGetDriverShortName( driver );

      if ( driverName.startsWith( QLatin1String( "AVCBin" ) ) )
      {
        sDirectoryDrivers += QObject::tr( "Arc/Info Binary Coverage" ) + ",AVCBin;";
      }
      else if ( driverName.startsWith( QLatin1String( "AVCE00" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Arc/Info ASCII Coverage" ), QStringLiteral( "*.e00" ) );
        sExtensions << QStringLiteral( "e00" );
      }
      else if ( driverName.startsWith( QLatin1String( "BNA" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Atlas BNA" ), QStringLiteral( "*.bna" ) );
        sExtensions << QStringLiteral( "bna" );
      }
      else if ( driverName.startsWith( QLatin1String( "CSV" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Comma Separated Value" ), QStringLiteral( "*.csv" ) );
        sExtensions << QStringLiteral( "csv" );
      }
      else if ( driverName.startsWith( QObject::tr( "DODS" ) ) )
      {
        sProtocolDrivers += QLatin1String( "DODS/OPeNDAP,DODS;" );
      }
      else if ( driverName.startsWith( QObject::tr( "CouchDB" ) ) )
      {
        sProtocolDrivers += QLatin1String( "CouchDB;" );
      }
      else if ( driverName.startsWith( QLatin1String( "FileGDB" ) ) )
      {
        sDirectoryDrivers += QObject::tr( "ESRI FileGDB" ) + ",FileGDB;";
        if ( !sDirectoryExtensions.contains( QStringLiteral( "gdb" ) ) )
          sDirectoryExtensions << QStringLiteral( "gdb" );
      }
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0)
      else if ( driverName.startsWith( QLatin1String( "FlatGeobuf" ) ) )
      {
        sProtocolDrivers += QLatin1String( "FlatGeobuf;" );
        sFileFilters += createFileFilter_( QObject::tr( "FlatGeobuf" ), QStringLiteral( "*.fgb" ) );
        sExtensions << QStringLiteral( "fgb" );
      }
#endif
      else if ( driverName.startsWith( QLatin1String( "PGeo" ) ) )
      {
        sDatabaseDrivers += QObject::tr( "ESRI Personal GeoDatabase" ) + ",PGeo;";
#ifdef Q_OS_WIN
        sFileFilters += createFileFilter_( QObject::tr( "ESRI Personal GeoDatabase" ), "*.mdb" );
        sExtensions << "mdb";
#endif
      }
      else if ( driverName.startsWith( QLatin1String( "SDE" ) ) )
      {
        sDatabaseDrivers += QObject::tr( "ESRI ArcSDE" ) + ",SDE;";
      }
      else if ( driverName.startsWith( QLatin1String( "ESRI Shapefile" ) ) )
      {
        QString exts = GDALGetMetadataItem( driver, GDAL_DMD_EXTENSIONS, "" );
        sFileFilters += createFileFilter_( QObject::tr( "ESRI Shapefiles" ), exts.contains( "shz" ) ? QStringLiteral( "*.shp *.shz *.shp.zip" ) : QStringLiteral( "*.shp" ) );
        sExtensions << QStringLiteral( "shp" ) << QStringLiteral( "dbf" );
        if ( exts.contains( "shz" ) )
          sExtensions << QStringLiteral( "shz" ) << QStringLiteral( "shp.zip" );
      }
      else if ( driverName.startsWith( QObject::tr( "FMEObjects Gateway" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "FMEObjects Gateway" ), QStringLiteral( "*.fdd" ) );
        sExtensions << QStringLiteral( "fdd" );
      }
      else if ( driverName.startsWith( QLatin1String( "GeoJSONSeq" ) ) )
      {
        sProtocolDrivers += QLatin1String( "GeoJSON - Newline Delimited;" );
        sFileFilters += createFileFilter_( QObject::tr( "GeoJSON Newline Delimited JSON" ), QStringLiteral( "*.geojsonl *.geojsons *.nlgeojson *.json" ) );
        sExtensions << QStringLiteral( "geojsonl" ) << QStringLiteral( "geojsons" ) << QStringLiteral( "nlgeojson" ) << QStringLiteral( "json" );
      }
      else if ( driverName.startsWith( QLatin1String( "GeoJSON" ) ) )
      {
        sProtocolDrivers += QLatin1String( "GeoJSON;" );
        sFileFilters += createFileFilter_( QObject::tr( "GeoJSON" ), QStringLiteral( "*.geojson" ) );
        sExtensions << QStringLiteral( "geojson" );
      }
      else if ( driverName.startsWith( QLatin1String( "GeoRSS" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "GeoRSS" ), QStringLiteral( "*.xml" ) );
        sExtensions << QStringLiteral( "xml" );
      }
      else if ( driverName == QLatin1String( "GML" ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Geography Markup Language [GML]" ), QStringLiteral( "*.gml" ) );
        sExtensions << QStringLiteral( "gml" );
      }
      else if ( driverName == QLatin1String( "GMLAS" ) )
      {
        continue;
      }
      else if ( driverName.startsWith( QLatin1String( "GMT" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Generic Mapping Tools [GMT]" ), QStringLiteral( "*.gmt" ) );
        sExtensions << QStringLiteral( "gmt" );
      }
      else if ( driverName.startsWith( QLatin1String( "GPX" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "GPS eXchange Format [GPX]" ), QStringLiteral( "*.gpx" ) );
        sExtensions << QStringLiteral( "gpx" );
      }
      else if ( driverName.startsWith( QLatin1String( "GPKG" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "GeoPackage" ), QStringLiteral( "*.gpkg" ) );
        sExtensions << QStringLiteral( "gpkg" );
      }
      else if ( driverName.startsWith( QLatin1String( "GRASS" ) ) )
      {
        sDirectoryDrivers += QObject::tr( "Grass Vector" ) + ",GRASS;";
      }
      else if ( driverName.startsWith( QLatin1String( "IDB" ) ) )
      {
        sDatabaseDrivers += QObject::tr( "Informix DataBlade" ) + ",IDB;";
      }
      else if ( driverName.startsWith( QLatin1String( "Interlis 1" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "INTERLIS 1" ), QStringLiteral( "*.itf *.xml *.ili" ) );
        sExtensions << QStringLiteral( "itf" ) << QStringLiteral( "xml" ) << QStringLiteral( "ili" );
      }
      else if ( driverName.startsWith( QLatin1String( "Interlis 2" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "INTERLIS 2" ), QStringLiteral( "*.xtf *.xml *.ili" ) );
        sExtensions << QStringLiteral( "xtf" ) << QStringLiteral( "xml" ) << QStringLiteral( "ili" );
      }
      else if ( driverName.startsWith( QLatin1String( "Ingres" ) ) )
      {
        sDatabaseDrivers += QObject::tr( "Ingres" ) + ",Ingres;";
      }
      else if ( driverName == QLatin1String( "KML" ) || driverName == QLatin1String( "LIBKML" ) )
      {
        if ( kmlFound )
          continue;
        kmlFound = true;
        sFileFilters += createFileFilter_( QObject::tr( "Keyhole Markup Language [KML]" ), QStringLiteral( "*.kml *.kmz" ) );
        sExtensions << QStringLiteral( "kml" ) << QStringLiteral( "kmz" );
      }
      else if ( driverName.startsWith( QLatin1String( "MapInfo File" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Mapinfo File" ), QStringLiteral( "*.mif *.tab" ) );
        sExtensions << QStringLiteral( "mif" ) << QStringLiteral( "tab" );
      }
      else if ( driverName == QLatin1String( "DGN" ) || driverName == QLatin1String( "DGNV8" ) )
      {
        if ( dgnFound )
          continue;
        dgnFound = true;
        sFileFilters += createFileFilter_( QObject::tr( "Microstation DGN" ), QStringLiteral( "*.dgn" ) );
        sExtensions << QStringLiteral( "dgn" );
      }
      else if ( driverName.startsWith( QLatin1String( "MySQL" ) ) )
      {
        sDatabaseDrivers += QObject::tr( "MySQL" ) + ",MySQL;";
      }
      else if ( driverName.startsWith( QLatin1String( "MSSQL" ) ) )
      {
        sDatabaseDrivers += QObject::tr( "MSSQL" ) + ",MSSQL;";
      }
      else if ( driverName.startsWith( QLatin1String( "OCI" ) ) )
      {
        sDatabaseDrivers += QObject::tr( "Oracle Spatial" ) + ",OCI;";
      }
      else if ( driverName.startsWith( QLatin1String( "ODBC" ) ) )
      {
        sDatabaseDrivers += QObject::tr( "ODBC" ) + ",ODBC;";
      }
      else if ( driverName.startsWith( QLatin1String( "OGDI" ) ) )
      {
        sDatabaseDrivers += QObject::tr( "OGDI Vectors" ) + ",OGDI;";
      }
      else if ( driverName.startsWith( QLatin1String( "OpenFileGDB" ) ) )
      {
        sDirectoryDrivers += QObject::tr( "OpenFileGDB" ) + ",OpenFileGDB;";
        if ( !sDirectoryExtensions.contains( QStringLiteral( "gdb" ) ) )
          sDirectoryExtensions << QStringLiteral( "gdb" );
      }
      else if ( driverName.startsWith( QLatin1String( "PostgreSQL" ) ) )
      {
        sDatabaseDrivers += QObject::tr( "PostgreSQL" ) + ",PostgreSQL;";
      }
      else if ( driverName.startsWith( QLatin1String( "S57" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "S-57 Base file" ),
                                           QStringLiteral( "*.000" ) );
        sExtensions << QStringLiteral( "000" );
      }
      else if ( driverName.startsWith( QLatin1String( "SDTS" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Spatial Data Transfer Standard [SDTS]" ),
                                           QStringLiteral( "*catd.ddf" ) );
        sWildcards << QStringLiteral( "*catd.ddf" );
      }
      else if ( driverName.startsWith( QLatin1String( "SOSI" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Systematic Organization of Spatial Information [SOSI]" ), QStringLiteral( "*.sos" ) );
        sExtensions << QStringLiteral( "sos" );
      }
      else if ( driverName.startsWith( QLatin1String( "SQLite" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "SQLite/SpatiaLite" ), QStringLiteral( "*.sqlite *.db *.sqlite3 *.db3 *.s3db *.sl3" ) );
        sExtensions << QStringLiteral( "sqlite" ) << QStringLiteral( "db" ) << QStringLiteral( "sqlite3" ) << QStringLiteral( "db3" ) << QStringLiteral( "s3db" ) << QStringLiteral( "sl3" );
      }
      else if ( driverName.startsWith( QLatin1String( "SXF" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Storage and eXchange Format" ), QStringLiteral( "*.sxf" ) );
        sExtensions << QStringLiteral( "sxf" );
      }
      else if ( driverName.startsWith( QLatin1String( "UK .NTF" ) ) )
      {
        sDirectoryDrivers += QObject::tr( "UK. NTF2" ) + ",UK. NTF;";
      }
      else if ( driverName.startsWith( QLatin1String( "TIGER" ) ) )
      {
        sDirectoryDrivers += QObject::tr( "U.S. Census TIGER/Line" ) + ",TIGER;";
      }
      else if ( driverName.startsWith( QLatin1String( "OGR_VRT" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "VRT - Virtual Datasource" ),
                                           QStringLiteral( "*.vrt *.ovf" ) );
        sExtensions << QStringLiteral( "vrt" ) << QStringLiteral( "ovf" );
      }
      else if ( driverName.startsWith( QLatin1String( "XPlane" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "X-Plane/Flightgear" ),
                                           QStringLiteral( "apt.dat nav.dat fix.dat awy.dat" ) );
        sWildcards << QStringLiteral( "apt.dat" ) << QStringLiteral( "nav.dat" ) << QStringLiteral( "fix.dat" ) << QStringLiteral( "awy.dat" );
      }
      else if ( driverName.startsWith( QLatin1String( "Geoconcept" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Geoconcept" ), QStringLiteral( "*.gxt *.txt" ) );
        sExtensions << QStringLiteral( "gxt" ) << QStringLiteral( "txt" );
      }
      else if ( driverName.startsWith( QLatin1String( "DXF" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "AutoCAD DXF" ), QStringLiteral( "*.dxf" ) );
        sExtensions << QStringLiteral( "dxf" );
      }
      else if ( driverName.startsWith( QLatin1String( "ODS" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Open Document Spreadsheet" ), QStringLiteral( "*.ods" ) );
        sExtensions << QStringLiteral( "ods" );
      }
      else if ( driverName.startsWith( QLatin1String( "XLSX" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "MS Office Open XML spreadsheet" ), QStringLiteral( "*.xlsx" ) );
        sExtensions << QStringLiteral( "xlsx" );
      }
      else if ( driverName.endsWith( QLatin1String( "XLS" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "MS Excel format" ), QStringLiteral( "*.xls" ) );
        sExtensions << QStringLiteral( "xls" );
      }
      else if ( driverName.startsWith( QLatin1String( "EDIGEO" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "EDIGEO" ), QStringLiteral( "*.thf" ) );
        sExtensions << QStringLiteral( "thf" );
      }
      else if ( driverName.startsWith( QLatin1String( "NAS" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "NAS - ALKIS" ), QStringLiteral( "*.xml" ) );
        sExtensions << QStringLiteral( "xml" );
      }
      else if ( driverName.startsWith( QLatin1String( "WAsP" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "WAsP" ), QStringLiteral( "*.map" ) );
        sExtensions << QStringLiteral( "map" );
      }
      else if ( driverName.startsWith( QLatin1String( "PCIDSK" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "PCI Geomatics Database File" ), QStringLiteral( "*.pix" ) );
        sExtensions << QStringLiteral( "pix" );
      }
      else if ( driverName.startsWith( QLatin1String( "GPSTrackMaker" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "GPSTrackMaker" ), QStringLiteral( "*.gtm *.gtz" ) );
        sExtensions << QStringLiteral( "gtm" ) << QStringLiteral( "gtz" );
      }
      else if ( driverName.startsWith( QLatin1String( "VFK" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Czech Cadastral Exchange Data Format" ), QStringLiteral( "*.vfk" ) );
        sExtensions << QStringLiteral( "vfk" );
      }
      else if ( driverName.startsWith( QLatin1String( "OSM" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "OpenStreetMap" ), QStringLiteral( "*.osm *.pbf" ) );
        sExtensions << QStringLiteral( "osm" ) << QStringLiteral( "pbf" );
      }
      else if ( driverName.startsWith( QLatin1String( "SUA" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Special Use Airspace Format" ), QStringLiteral( "*.sua" ) );
        sExtensions << QStringLiteral( "sua" );
      }
      else if ( driverName.startsWith( QLatin1String( "OpenAir" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "OpenAir Special Use Airspace Format" ), QStringLiteral( "*.txt" ) );
        sExtensions << QStringLiteral( "txt" );
      }
      else if ( driverName.startsWith( QLatin1String( "PDS" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Planetary Data Systems TABLE" ), QStringLiteral( "*.xml" ) );
        sExtensions << QStringLiteral( "xml" );
      }
      else if ( driverName.startsWith( QLatin1String( "HTF" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Hydrographic Transfer Format" ), QStringLiteral( "*.htf" ) );
        sExtensions << QStringLiteral( "htf" );
      }
      else if ( driverName.startsWith( QLatin1String( "SVG" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Scalable Vector Graphics" ), QStringLiteral( "*.svg" ) );
        sExtensions << QStringLiteral( "svg" );
      }
      else if ( driverName.startsWith( QLatin1String( "ARCGEN" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Arc/Info Generate" ), QStringLiteral( "*.gen" ) );
        sExtensions << QStringLiteral( "gen" );
      }
      else if ( driverName.startsWith( QLatin1String( "PDF" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "Geospatial PDF" ), QStringLiteral( "*.pdf" ) );
        sExtensions << QStringLiteral( "pdf" );
      }
      else if ( driverName.startsWith( QLatin1String( "SEGY" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "SEG-Y" ), QStringLiteral( "*.sgy *.segy" ) );
        sExtensions << QStringLiteral( "sgy" ) << QStringLiteral( "segy" );
      }
      else if ( driverName.startsWith( QLatin1String( "SEGUKOOA" ) ) )
      {
        sFileFilters += createFileFilter_( QObject::tr( "SEG-P1" ), QStringLiteral( "*.seg *.seg1 *.sp1" ) );
        sFileFilters += createFileFilter_( QObject::tr( "UKOOA P1/90" ), QStringLiteral( "*.uko *.ukooa" ) );
        sExtensions << QStringLiteral( "seg" ) << QStringLiteral( "seg1" ) << QStringLiteral( "sp1" ) << QStringLiteral( "uko" ) << QStringLiteral( "ukooa" );
      }
      else
      {
        if ( driverName == QLatin1String( "CAD" ) || driverName == QLatin1String( "DWG" ) )
        {
          if ( dwgFound )
            continue;
          dwgFound = true;
        }

        QString myGdalDriverExtensions = GDALGetMetadataItem( driver, GDAL_DMD_EXTENSIONS, "" );
        QString myGdalDriverLongName = GDALGetMetadataItem( driver, GDAL_DMD_LONGNAME, "" );
        if ( !( myGdalDriverExtensions.isEmpty() || myGdalDriverLongName.isEmpty() ) )
        {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
          const QStringList splitExtensions = myGdalDriverExtensions.split( ' ', QString::SkipEmptyParts );
#else
          const QStringList splitExtensions = myGdalDriverExtensions.split( ' ', Qt::SkipEmptyParts );
#endif
          QString glob;

          for ( const QString &ext : splitExtensions )
          {
            sExtensions << ext;
            if ( !glob.isEmpty() )
              glob += QLatin1Char( ' ' );
            glob += "*." + ext;
          }

          if ( driverName == QLatin1String( "JPEG2000" ) ||
               driverName.startsWith( QLatin1String( "JP2" ) ) )
          {
            // Skip over JPEG2000 drivers, as their vector capabilities are just
            // a marginal use case
            continue;
          }

          sFileFilters += createFileFilter_( myGdalDriverLongName, glob );

        }
        else
        {
          // NOP, we don't know anything about the current driver
          // with regards to a proper file filter string
          QgsDebugMsgLevel( QStringLiteral( "Unknown driver %1 for file filters." ).arg( driverName ), 2 );
        }
      }

    }                          // each loaded OGR driver

    // sort file filters alphabetically
    QgsDebugMsgLevel( "myFileFilters: " + sFileFilters, 2 );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QStringList filters = sFileFilters.split( QStringLiteral( ";;" ), QString::SkipEmptyParts );
#else
    QStringList filters = sFileFilters.split( QStringLiteral( ";;" ), Qt::SkipEmptyParts );
#endif
    filters.sort();
    sFileFilters = filters.join( QLatin1String( ";;" ) ) + ";;";
    QgsDebugMsgLevel( "myFileFilters: " + sFileFilters, 2 );

    // VSIFileHandler (.zip and .gz files) - second
    //   see http://trac.osgeo.org/gdal/wiki/UserDocs/ReadInZip
    // Requires GDAL>=1.6.0 with libz support, let's assume we have it.
    // This does not work for some file types, see VSIFileHandler doc.
    QgsSettings settings;
    if ( settings.value( QStringLiteral( "qgis/scanZipInBrowser2" ), "basic" ).toString() != QLatin1String( "no" ) )
    {
      sFileFilters.prepend( createFileFilter_( QObject::tr( "GDAL/OGR VSIFileHandler" ), QStringLiteral( "*.zip *.gz *.tar *.tar.gz *.tgz" ) ) );
      sExtensions << QStringLiteral( "zip" ) << QStringLiteral( "gz" ) << QStringLiteral( "tar" ) << QStringLiteral( "tar.gz" ) << QStringLiteral( "tgz" );
    }

    // can't forget the all supported case
    QStringList exts;
    for ( const QString &ext : std::as_const( sExtensions ) )
      exts << QStringLiteral( "*.%1 *.%2" ).arg( ext, ext.toUpper() );
    sFileFilters.prepend( QObject::tr( "All supported files" ) + QStringLiteral( " (%1);;" ).arg( exts.join( QLatin1Char( ' ' ) ) ) );

    // can't forget the default case - first
    sFileFilters.prepend( QObject::tr( "All files" ) + " (*);;" );


    // cleanup
    if ( sFileFilters.endsWith( QLatin1String( ";;" ) ) ) sFileFilters.chop( 2 );

    QgsDebugMsgLevel( "myFileFilters: " + sFileFilters, 2 );
  }

  if ( type == QLatin1String( "file" ) )
  {
    return sFileFilters;
  }
  if ( type == QLatin1String( "database" ) )
  {
    return sDatabaseDrivers;
  }
  if ( type == QLatin1String( "protocol" ) )
  {
    return sProtocolDrivers;
  }
  if ( type == QLatin1String( "directory" ) )
  {
    return sDirectoryDrivers;
  }
  if ( type == QLatin1String( "extensions" ) )
  {
    return sExtensions.join( QLatin1Char( '|' ) );
  }
  if ( type == QLatin1String( "directory_extensions" ) )
  {
    return sDirectoryExtensions.join( QLatin1Char( '|' ) );
  }
  if ( type == QLatin1String( "wildcards" ) )
  {
    return sWildcards.join( QLatin1Char( '|' ) );
  }
  else
  {
    return QString();
  }
}

QString QgsOgrProviderUtils::fileVectorFilters()
{
  return createFilters( QStringLiteral( "file" ) );
}

QString QgsOgrProviderUtils::databaseDrivers()
{
  return createFilters( QStringLiteral( "database" ) );
}

QString QgsOgrProviderUtils::protocolDrivers()
{
  return createFilters( QStringLiteral( "protocol" ) );
}


QString QgsOgrProviderUtils::directoryDrivers()
{
  return createFilters( QStringLiteral( "directory" ) );
}

QStringList QgsOgrProviderUtils::fileExtensions()
{
  return createFilters( QStringLiteral( "extensions" ) ).split( '|' );
}

QStringList QgsOgrProviderUtils::directoryExtensions()
{
  return createFilters( QStringLiteral( "directory_extensions" ) ).split( '|' );
}

QStringList QgsOgrProviderUtils::wildcards()
{
  return createFilters( QStringLiteral( "wildcards" ) ).split( '|' );
}

bool QgsOgrProviderUtils::createEmptyDataSource( const QString &uri,
    const QString &format,
    const QString &encoding,
    QgsWkbTypes::Type vectortype,
    const QList< QPair<QString, QString> > &attributes,
    const QgsCoordinateReferenceSystem &srs,
    QString &errorMessage )
{
  QgsDebugMsgLevel( QStringLiteral( "Creating empty vector layer with format: %1" ).arg( format ), 2 );
  errorMessage.clear();

  QgsApplication::registerOgrDrivers();
  OGRSFDriverH driver = OGRGetDriverByName( format.toLatin1() );
  if ( !driver )
  {
    return false;
  }

  QString driverName = GDALGetDriverShortName( driver );

  if ( driverName == QLatin1String( "ESRI Shapefile" ) )
  {
    if ( !uri.endsWith( QLatin1String( ".shp" ), Qt::CaseInsensitive ) &&
         !uri.endsWith( QLatin1String( ".shz" ), Qt::CaseInsensitive ) &&
         !uri.endsWith( QLatin1String( ".dbf" ), Qt::CaseInsensitive ) )
    {
      errorMessage = QObject::tr( "URI %1 doesn't end with .shp or .dbf" ).arg( uri );
      QgsDebugMsg( errorMessage );
      return false;
    }

    // check for duplicate fieldnames
    QSet<QString> fieldNames;
    QList<QPair<QString, QString> >::const_iterator fldIt;
    for ( fldIt = attributes.begin(); fldIt != attributes.end(); ++fldIt )
    {
      QString name = fldIt->first.left( 10 );
      if ( fieldNames.contains( name ) )
      {
        errorMessage = QObject::tr( "Duplicate field (10 significant characters): %1" ).arg( name );
        QgsMessageLog::logMessage( errorMessage, QObject::tr( "OGR" ) );
        return false;
      }
      fieldNames << name;
    }

    QgsVectorFileWriter::deleteShapeFile( uri );
  }
  else
  {
    QFile::remove( uri );
  }

  gdal::dataset_unique_ptr dataSource;
  dataSource.reset( OGR_Dr_CreateDataSource( driver, uri.toUtf8().constData(), nullptr ) );
  if ( !dataSource )
  {
    errorMessage = QObject::tr( "Creating the data source %1 failed: %2" ).arg( uri, QString::fromUtf8( CPLGetLastErrorMsg() ) );
    QgsMessageLog::logMessage( errorMessage, QObject::tr( "OGR" ) );
    return false;
  }

  //consider spatial reference system

  QgsCoordinateReferenceSystem mySpatialRefSys;
  if ( srs.isValid() )
  {
    mySpatialRefSys = srs;
  }
  else
  {
    mySpatialRefSys.validate();
  }

  OGRSpatialReferenceH reference = QgsOgrUtils::crsToOGRSpatialReference( mySpatialRefSys );

  // Map the qgis geometry type to the OGR geometry type
  OGRwkbGeometryType OGRvectortype = wkbUnknown;
  switch ( vectortype )
  {
    case QgsWkbTypes::GeometryCollection:
    case QgsWkbTypes::GeometryCollectionZ:
    case QgsWkbTypes::GeometryCollectionM:
    case QgsWkbTypes::GeometryCollectionZM:
    case QgsWkbTypes::Unknown:
    {
      errorMessage = QObject::tr( "Unknown vector type of %1" ).arg( static_cast< int >( vectortype ) );
      QgsMessageLog::logMessage( errorMessage, QObject::tr( "OGR" ) );
      return false;
    }

    default:
      OGRvectortype = QgsOgrProviderUtils::ogrTypeFromQgisType( vectortype );
  }

  char **papszOptions = nullptr;
  if ( driverName == QLatin1String( "ESRI Shapefile" ) )
  {
    papszOptions = CSLSetNameValue( papszOptions, "ENCODING", QgsVectorFileWriter::convertCodecNameForEncodingOption( encoding ).toLocal8Bit().data() );
    // OGR Shapefile fails to create fields if given encoding is not supported by its side
    // so disable encoding conversion of OGR Shapefile layer
    CPLSetConfigOption( "SHAPE_ENCODING", "" );
  }

  OGRLayerH layer;
  layer = GDALDatasetCreateLayer( dataSource.get(), QFileInfo( uri ).completeBaseName().toUtf8().constData(), reference, OGRvectortype, papszOptions );
  CSLDestroy( papszOptions );

  if ( !layer )
  {
    errorMessage = QString::fromUtf8( CPLGetLastErrorMsg() );
    QgsMessageLog::logMessage( errorMessage, QObject::tr( "OGR" ) );
    return false;
  }

  //create the attribute fields

  QTextCodec *codec = QTextCodec::codecForName( encoding.toLocal8Bit().data() );
  if ( !codec )
  {
    // fall back to "System" codec
    codec = QTextCodec::codecForLocale();
    Q_ASSERT( codec );
  }

  for ( QList<QPair<QString, QString> >::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
  {
    QStringList fields = it->second.split( ';' );

    if ( fields.isEmpty() )
      continue;

    int width = fields.size() > 1 ? fields[1].toInt() : -1;
    int precision = fields.size() > 2 ? fields[2].toInt() : -1;
    if ( precision > 0 )
      width += 1;

    OGRFieldDefnH field;
    if ( fields[0] == QLatin1String( "Real" ) )
    {
      if ( width < 0 )
        width = 32;
      if ( precision < 0 )
        precision = 3;

      field = OGR_Fld_Create( codec->fromUnicode( it->first ).constData(), OFTReal );
      OGR_Fld_SetWidth( field, width );
      OGR_Fld_SetPrecision( field, precision );
    }
    else if ( fields[0] == QLatin1String( "Integer" ) )
    {
      if ( width < 0 || width > 10 )
        width = 10;

      field = OGR_Fld_Create( codec->fromUnicode( it->first ).constData(), OFTInteger );
      // limit to 10.  otherwise OGR sets it to 11 and recognizes as OFTDouble later
      OGR_Fld_SetWidth( field, width );
    }
    else if ( fields[0] == QLatin1String( "String" ) )
    {
      if ( width < 0 || width > 255 )
        width = 255;

      field = OGR_Fld_Create( codec->fromUnicode( it->first ).constData(), OFTString );
      OGR_Fld_SetWidth( field, width );
    }
    else if ( fields[0] == QLatin1String( "Date" ) )
    {
      field = OGR_Fld_Create( codec->fromUnicode( it->first ).constData(), OFTDate );
    }
    else if ( fields[0] == QLatin1String( "Time" ) )
    {
      field = OGR_Fld_Create( codec->fromUnicode( it->first ).constData(), OFTTime );
    }
    else if ( fields[0] == QLatin1String( "DateTime" ) )
    {
      field = OGR_Fld_Create( codec->fromUnicode( it->first ).constData(), OFTDateTime );
    }
    else
    {
      QgsMessageLog::logMessage( QObject::tr( "field %1 with unsupported type %2 skipped" ).arg( it->first, fields[0] ), QObject::tr( "OGR" ) );
      continue;
    }

    if ( OGR_L_CreateField( layer, field, true ) != OGRERR_NONE )
    {
      QgsMessageLog::logMessage( QObject::tr( "creation of field %1 failed" ).arg( it->first ), QObject::tr( "OGR" ) );
    }
  }

  dataSource.reset();

  if ( driverName == QLatin1String( "ESRI Shapefile" ) )
  {
    int index = uri.indexOf( QLatin1String( ".shp" ), Qt::CaseInsensitive );
    if ( index > 0 )
    {
      QString layerName = uri.left( index );
      QFile prjFile( layerName + ".qpj" );
      if ( prjFile.exists() )
        prjFile.remove();
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "GDAL Version number %1" ).arg( GDAL_VERSION_NUM ), 2 );
  if ( reference )
  {
    OSRRelease( reference );
  }
  return true;
}

QString QgsOgrProviderUtils::connectionPoolId( const QString &dataSourceURI, bool shareSameDatasetAmongLayers )
{
  if ( shareSameDatasetAmongLayers )
  {
    // If the file part of the URI is really a file, then use it as the
    // connection pool id (for example, so that all layers of a .gpkg file can
    // use the same GDAL dataset object)
    // Otherwise use the datasourceURI
    // Not completely sure about this logic. But at least, for GeoPackage this
    // works fine with multi layer datasets.
    QString filePath = dataSourceURI.left( dataSourceURI.indexOf( QLatin1Char( '|' ) ) );
    QFileInfo fi( filePath );
    if ( fi.isFile() )
    {
      // Preserve open options so pooled connections always carry those on
      QString openOptions;
      static thread_local QRegularExpression openOptionsRegex( QStringLiteral( "((?:\\|option:(?:[^|]*))+)" ) );
      QRegularExpressionMatch match = openOptionsRegex.match( dataSourceURI );
      if ( match.hasMatch() )
      {
        openOptions = match.captured( 1 );
      }
      return filePath + openOptions;
    }
  }
  return dataSourceURI;
}

GDALDatasetH QgsOgrProviderUtils::GDALOpenWrapper( const char *pszPath, bool bUpdate, char **papszOpenOptionsIn, GDALDriverH *phDriver )
{
  CPLErrorReset();

  // Avoid getting too close to the limit of files allowed in the process,
  // to avoid potential crashes such as in https://github.com/qgis/QGIS/issues/43620
  // This heuristics is not perfect because during rendering, files will be opened again
  // Typically for a GPKG file we need 6 file descriptors: 3 for the .gpkg,
  // .gpkg-wal, .gpkg-shm for the provider, and 2 for the .gpkg + for the .gpkg-wal
  // used by feature iterator for rendering
  // So if we hit the limit, some layers will not be displayable.
  if ( QgsFileUtils::isCloseToLimitOfOpenedFiles() )
  {
#ifdef Q_OS_UNIX
    QgsMessageLog::logMessage( QObject::tr( "Too many files opened (%1). Cannot open %2. You may raise the limit with the 'ulimit -n number_of_files' command before starting QGIS." ).arg( QgsFileUtils::openedFileCount() ).arg( QString( pszPath ) ), QObject::tr( "OGR" ) );
#else
    QgsMessageLog::logMessage( QObject::tr( "Too many files opened (%1). Cannot open %2" ).arg( QgsFileUtils::openedFileCount() ).arg( QString( pszPath ) ), QObject::tr( "OGR" ) );
#endif
    return nullptr;
  }

  char **papszOpenOptions = CSLDuplicate( papszOpenOptionsIn );

  QString filePath( QString::fromUtf8( pszPath ) );

  bool bIsGpkg = QFileInfo( filePath ).suffix().compare( QLatin1String( "gpkg" ), Qt::CaseInsensitive ) == 0;
  bool bIsLocalGpkg = false;

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,2)
  if ( bIsGpkg && !bUpdate )
  {
    papszOpenOptions = CSLSetNameValue( papszOpenOptions, "NOLOCK", "ON" );
  }
#endif

  if ( bIsGpkg &&
       IsLocalFile( filePath ) &&
       !CPLGetConfigOption( "OGR_SQLITE_JOURNAL", nullptr ) &&
       QgsSettings().value( QStringLiteral( "qgis/walForSqlite3" ), true ).toBool() )
  {
    // Starting with GDAL 3.4.2, QgsOgrProvider::open(OpenModeInitial) will set
    // a fake DO_NOT_ENABLE_WAL=ON when doing the initial open in update mode
    // to indicate that we should not enable WAL.
    // And NOLOCK=ON will be set in read-only attempts.
    // Only enable it when enterUpdateMode() has been executed.
    if ( CSLFetchNameValue( papszOpenOptions, "DO_NOT_ENABLE_WAL" ) == nullptr &&
         CSLFetchNameValue( papszOpenOptions, "NOLOCK" ) == nullptr )
    {
      // For GeoPackage, we force opening of the file in WAL (Write Ahead Log)
      // mode so as to avoid readers blocking writer(s), and vice-versa.
      // https://www.sqlite.org/wal.html
      // But only do that on a local file since WAL is advertised not to work
      // on network shares
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,2)
      QgsDebugMsgLevel( QStringLiteral( "Enabling WAL" ), 3 );
#endif
      CPLSetThreadLocalConfigOption( "OGR_SQLITE_JOURNAL", "WAL" );
    }
    bIsLocalGpkg = true;
  }
  else if ( bIsGpkg )
  {
    // If WAL isn't set, we explicitly disable it, as it is persistent and it
    // may have been set on a previous connection.
    CPLSetThreadLocalConfigOption( "OGR_SQLITE_JOURNAL", "DELETE" );
  }

  if ( CSLFetchNameValue( papszOpenOptions, "DO_NOT_ENABLE_WAL" ) != nullptr )
  {
    papszOpenOptions = CSLSetNameValue( papszOpenOptions, "DO_NOT_ENABLE_WAL", nullptr );
  }

  bool modify_OGR_GPKG_FOREIGN_KEY_CHECK = !CPLGetConfigOption( "OGR_GPKG_FOREIGN_KEY_CHECK", nullptr );
  if ( modify_OGR_GPKG_FOREIGN_KEY_CHECK )
  {
    CPLSetThreadLocalConfigOption( "OGR_GPKG_FOREIGN_KEY_CHECK", "NO" );
  }

  const int nOpenFlags = GDAL_OF_VECTOR | ( bUpdate ? GDAL_OF_UPDATE : 0 );
  GDALDatasetH hDS = GDALOpenEx( pszPath, nOpenFlags, nullptr, papszOpenOptions, nullptr );
  CSLDestroy( papszOpenOptions );

  CPLSetThreadLocalConfigOption( "OGR_SQLITE_JOURNAL", nullptr );
  if ( modify_OGR_GPKG_FOREIGN_KEY_CHECK )
  {
    CPLSetThreadLocalConfigOption( "OGR_GPKG_FOREIGN_KEY_CHECK", nullptr );
  }

  if ( !hDS )
  {
    if ( phDriver )
      *phDriver = nullptr;
    return nullptr;
  }
  GDALDriverH hDrv = GDALGetDatasetDriver( hDS );
  if ( bIsLocalGpkg && strcmp( GDALGetDriverShortName( hDrv ), "GPKG" ) == 0 )
  {
    QMutexLocker locker( sGlobalMutex() );
    ( *sMapCountOpenedDS() )[ filePath ]++;
    ( *sMapDSHandleToUpdateMode() )[ hDS ] = bUpdate;
  }
  if ( phDriver )
    *phDriver = hDrv;

  if ( bUpdate && bIsGpkg && strcmp( GDALGetDriverShortName( hDrv ), "GPKG" ) == 0 )
  {
    // Explicitly enable foreign key enforcement (for new transactions)
    GDALDatasetExecuteSQL( hDS, "PRAGMA foreign_keys = ON", nullptr, nullptr );

#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3, 3, 1)
    // Fix wrong gpkg_metadata_reference_column_name_update trigger that was
    // generated by GDAL < 2.4.0
    OGRLayerH hSqlLyr = GDALDatasetExecuteSQL(
                          hDS,
                          "SELECT sql FROM sqlite_master WHERE type = 'trigger' AND "
                          "NAME ='gpkg_metadata_reference_column_name_update' AND "
                          "sql LIKE '%column_nameIS%'",
                          nullptr, nullptr );
    if ( hSqlLyr )
    {
      QString triggerSql;
      gdal::ogr_feature_unique_ptr hFeat( OGR_L_GetNextFeature( hSqlLyr ) );
      if ( hFeat )
      {
        triggerSql = OGR_F_GetFieldAsString( hFeat.get(), 0 );
      }
      GDALDatasetReleaseResultSet( hDS, hSqlLyr );

      if ( !triggerSql.isEmpty() )
      {
        GDALDatasetExecuteSQL(
          hDS, "DROP TRIGGER gpkg_metadata_reference_column_name_update",
          nullptr, nullptr );
        GDALDatasetExecuteSQL(
          hDS,
          triggerSql.replace( QLatin1String( "column_nameIS" ),
                              QLatin1String( "column_name IS" ) ).toUtf8().toStdString().c_str(),
          nullptr, nullptr );
      }
    }
#endif
  }

  return hDS;
}

static bool IsLocalFile( const QString &path )
{
  QString dirName( QFileInfo( path ).absolutePath() );
  // Start with the OS specific methods since the QT >= 5.4 method just
  // return a string and not an enumerated type.
#if defined(Q_OS_WIN)
  if ( dirName.startsWith( "\\\\" ) || dirName.startsWith( "//" ) )
    return false;
  if ( dirName.length() >= 3 && dirName[1] == ':' &&
       ( dirName[2] == '\\' || dirName[2] == '/' ) )
  {
    dirName.resize( 3 );
    return GetDriveType( dirName.toLatin1().constData() ) != DRIVE_REMOTE;
  }
  return true;
#elif defined(Q_OS_LINUX)
  struct statfs sStatFS;
  if ( statfs( dirName.toLatin1().constData(), &sStatFS ) == 0 )
  {
    // Codes from http://man7.org/linux/man-pages/man2/statfs.2.html
    if ( sStatFS.f_type == 0x6969 /* NFS */ ||
         sStatFS.f_type == 0x517b /* SMB */ ||
         sStatFS.f_type == 0xff534d42ul /* CIFS */ ||
         sStatFS.f_type == 0xfe534d42ul /* CIFS */ )
    {
      return false;
    }
  }
  return true;
#else
  QStorageInfo info( dirName );
  const QString fileSystem( info.fileSystemType() );
  bool isLocal = path != QLatin1String( "nfs" ) && path != QLatin1String( "smbfs" );
  if ( !isLocal )
    QgsDebugMsgLevel( QStringLiteral( "Filesystem for %1 is %2" ).arg( path, fileSystem ), 2 );
  return isLocal;
#endif
}

void QgsOgrProviderUtils::GDALCloseWrapper( GDALDatasetH hDS )
{
  if ( !hDS )
    return;
  GDALDriverH mGDALDriver = GDALGetDatasetDriver( hDS );
  QString mGDALDriverName = GDALGetDriverShortName( mGDALDriver );
  QString datasetName( QString::fromUtf8( GDALGetDescription( hDS ) ) );
  if ( mGDALDriverName == QLatin1String( "GPKG" ) &&
       IsLocalFile( datasetName ) &&
       !CPLGetConfigOption( "OGR_SQLITE_JOURNAL", nullptr ) )
  {
    bool openedAsUpdate = false;
    bool tryReturnToDelete = false;
    {
      QMutexLocker locker( sGlobalMutex() );
      ( *sMapCountOpenedDS() )[ datasetName ] --;
      if ( ( *sMapCountOpenedDS() )[ datasetName ] == 0 )
      {
        sMapCountOpenedDS()->remove( datasetName );
        openedAsUpdate = ( *sMapDSHandleToUpdateMode() )[hDS];
        tryReturnToDelete = true;
      }
      sMapDSHandleToUpdateMode()->remove( hDS );
    }
    if ( tryReturnToDelete )
    {
      bool bSuccess = false;

      // Check if the current journal_mode is not already delete
      {
        OGRLayerH hSqlLyr = GDALDatasetExecuteSQL( hDS,
                            "PRAGMA journal_mode",
                            nullptr, nullptr );
        if ( hSqlLyr )
        {
          gdal::ogr_feature_unique_ptr hFeat( OGR_L_GetNextFeature( hSqlLyr ) );
          if ( hFeat )
          {
            const char *pszRet = OGR_F_GetFieldAsString( hFeat.get(), 0 );
            bSuccess = EQUAL( pszRet, "delete" );
            QgsDebugMsgLevel( QStringLiteral( "Return: %1" ).arg( pszRet ), 2 );
          }
          GDALDatasetReleaseResultSet( hDS, hSqlLyr );
        }
      }

      if ( openedAsUpdate )
      {
        // We need to reset all iterators on layers, otherwise we will not
        // be able to change journal_mode.
        int layerCount = GDALDatasetGetLayerCount( hDS );
        for ( int i = 0; i < layerCount; i ++ )
        {
          OGR_L_ResetReading( GDALDatasetGetLayer( hDS, i ) );
        }

        if ( !bSuccess )
        {
          CPLPushErrorHandler( CPLQuietErrorHandler );
          QgsDebugMsgLevel( QStringLiteral( "GPKG: Trying to return to delete mode" ), 2 );
          OGRLayerH hSqlLyr = GDALDatasetExecuteSQL( hDS,
                              "PRAGMA journal_mode = delete",
                              nullptr, nullptr );
          if ( hSqlLyr )
          {
            gdal::ogr_feature_unique_ptr hFeat( OGR_L_GetNextFeature( hSqlLyr ) );
            if ( hFeat )
            {
              const char *pszRet = OGR_F_GetFieldAsString( hFeat.get(), 0 );
              bSuccess = EQUAL( pszRet, "delete" );
              QgsDebugMsgLevel( QStringLiteral( "Return: %1" ).arg( pszRet ), 2 );
            }
          }
          else if ( CPLGetLastErrorType() != CE_None )
          {
            QgsDebugMsg( QStringLiteral( "Return: %1" ).arg( CPLGetLastErrorMsg() ) );
          }
          GDALDatasetReleaseResultSet( hDS, hSqlLyr );
          CPLPopErrorHandler();
        }
      }
      GDALClose( hDS );

      // If the file was opened in read-only mode, or if the above failed,
      // we need to reopen it in update mode
      if ( !bSuccess )
      {
        if ( openedAsUpdate )
        {
          QgsDebugMsgLevel( QStringLiteral( "GPKG: Trying again" ), 2 );
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "GPKG: Trying to return to delete mode" ), 2 );
        }
        CPLSetThreadLocalConfigOption( "OGR_SQLITE_JOURNAL", "DELETE" );
        hDS = GDALOpenEx( datasetName.toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr );
        CPLSetThreadLocalConfigOption( "OGR_SQLITE_JOURNAL", nullptr );
        if ( hDS )
        {
#ifdef QGISDEBUG
          CPLPushErrorHandler( CPLQuietErrorHandler );
          OGRLayerH hSqlLyr = GDALDatasetExecuteSQL( hDS,
                              "PRAGMA journal_mode",
                              nullptr, nullptr );
          CPLPopErrorHandler();
          if ( hSqlLyr != nullptr )
          {
            gdal::ogr_feature_unique_ptr hFeat( OGR_L_GetNextFeature( hSqlLyr ) );
            if ( hFeat != nullptr )
            {
              const char *pszRet = OGR_F_GetFieldAsString( hFeat.get(), 0 );
              QgsDebugMsgLevel( QStringLiteral( "Return: %1" ).arg( pszRet ), 2 );
            }
            GDALDatasetReleaseResultSet( hDS, hSqlLyr );
          }
#endif
          GDALClose( hDS );
        }
      }
    }
    else
    {
      GDALClose( hDS );
    }
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0) && GDAL_VERSION_NUM <= GDAL_COMPUTE_VERSION(3,1,3)
  else if ( mGDALDriverName == QLatin1String( "XLSX" ) ||
            mGDALDriverName == QLatin1String( "ODS" ) )
  {
    // Workaround bug in GDAL 3.1.0 to 3.1.3 that creates XLSX and ODS files incompatible with LibreOffice due to use of ZIP64
    CPLSetThreadLocalConfigOption( "CPL_CREATE_ZIP64", "NO" );
    GDALClose( hDS );
    CPLSetThreadLocalConfigOption( "CPL_CREATE_ZIP64", nullptr );
  }
#endif

  else
  {
    GDALClose( hDS );
  }
}

QByteArray QgsOgrProviderUtils::quotedIdentifier( QByteArray field, const QString &driverName )
{
  if ( driverName == QLatin1String( "MySQL" ) )
  {
    field.replace( '\\', "\\\\" );
    field.replace( '`', "``" );
    return field.prepend( '`' ).append( '`' );
  }
  else
  {
    field.replace( '\\', "\\\\" );
    field.replace( '"', "\\\"" );
    field.replace( '\'', "\\'" );
    return field.prepend( '\"' ).append( '\"' );
  }
}

QString QgsOgrProviderUtils::quotedValue( const QVariant &value )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  switch ( value.type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();

    case QVariant::Bool:
      //OGR does not support boolean literals
      return value.toBool() ? "1" : "0";

    default:
    case QVariant::String:
      QString v = value.toString();
      v.replace( '\'', QLatin1String( "''" ) );
      if ( v.contains( '\\' ) )
        return v.replace( '\\', QLatin1String( "\\\\" ) ).prepend( "E'" ).append( '\'' );
      else
        return v.prepend( '\'' ).append( '\'' );
  }
}

OGRLayerH QgsOgrProviderUtils::setSubsetString( OGRLayerH layer, GDALDatasetH ds, QTextCodec *encoding, const QString &subsetString )
{
  // Remove any comments
  QStringList lines {subsetString.split( QChar( '\n' ) )};
  lines.erase( std::remove_if( lines.begin(), lines.end(), []( const QString & line )
  {
    return line.startsWith( QLatin1String( "--" ) );
  } ), lines.end() );
  for ( auto &line : lines )
  {
    bool inLiteral {false};
    QChar literalChar { ' ' };
    for ( int i = 0; i < line.length(); ++i )
    {
      if ( ( ( line[i] == QChar( '\'' ) || line[i] == QChar( '"' ) ) && ( i == 0 || line[i - 1] != QChar( '\\' ) ) ) && ( line[i] != literalChar ) )
      {
        inLiteral = !inLiteral;
        literalChar = inLiteral ? line[i] : QChar( ' ' );
      }
      if ( !inLiteral && line.mid( i ).startsWith( QLatin1String( "--" ) ) )
      {
        line = line.left( i );
        break;
      }
    }
  }
  const QString cleanedSubsetString {lines.join( QChar( ' ' ) ).trimmed() };

  QByteArray layerName = OGR_FD_GetName( OGR_L_GetLayerDefn( layer ) );
  GDALDriverH driver = GDALGetDatasetDriver( ds );
  QString driverName = GDALGetDriverShortName( driver );

  if ( driverName == QLatin1String( "ODBC" ) ) //the odbc driver does not like schema names for subset
  {
    QString layerNameString = encoding->toUnicode( layerName );
    int dotIndex = layerNameString.indexOf( '.' );
    if ( dotIndex > 1 )
    {
      QString modifiedLayerName = layerNameString.right( layerNameString.size() - dotIndex - 1 );
      layerName = encoding->fromUnicode( modifiedLayerName );
    }
  }
  OGRLayerH subsetLayer = nullptr;
  if ( cleanedSubsetString.startsWith( QLatin1String( "SELECT " ), Qt::CaseInsensitive ) )
  {
    QByteArray sql = encoding->fromUnicode( cleanedSubsetString );

    QgsDebugMsgLevel( QStringLiteral( "SQL: %1" ).arg( encoding->toUnicode( sql ) ), 2 );
    subsetLayer = GDALDatasetExecuteSQL( ds, sql.constData(), nullptr, nullptr );
  }
  else
  {
    if ( OGR_L_SetAttributeFilter( layer, encoding->fromUnicode( cleanedSubsetString ).constData() ) != OGRERR_NONE )
    {
      return nullptr;
    }
    subsetLayer = layer;
  }

  return subsetLayer;
}

QString QgsOgrProviderUtils::DatasetIdentification::toString() const
{
  return dsName +
         ( updateMode ?
           QStringLiteral( "update" ) : QStringLiteral( "read-only" ) ) +
         options.join( ',' );
}

bool QgsOgrProviderUtils::DatasetIdentification::operator<
( const QgsOgrProviderUtils::DatasetIdentification &other ) const
{
  return toString() < other.toString();
}

static GDALDatasetH OpenHelper( const QString &dsName,
                                bool updateMode,
                                const QStringList &options )
{
  char **papszOpenOptions = nullptr;
  const auto constOptions = options;
  for ( QString option : constOptions )
  {
    papszOpenOptions = CSLAddString( papszOpenOptions,
                                     option.toUtf8().constData() );
  }
  GDALDatasetH hDS = QgsOgrProviderUtils::GDALOpenWrapper(
                       QgsOgrProviderUtils::expandAuthConfig( dsName ).toUtf8().constData(), updateMode, papszOpenOptions, nullptr );
  CSLDestroy( papszOpenOptions );
  return hDS;
}

void QgsOgrProviderUtils::invalidateCachedDatasets( const QString &dsName )
{
  QMutexLocker locker( sGlobalMutex() );
  while ( true )
  {
    bool erased = false;
    for ( auto iter = sMapSharedDS.begin(); iter != sMapSharedDS.end(); ++iter )
    {
      if ( iter.key().dsName == dsName )
      {
        sMapSharedDS.erase( iter );
        erased = true;
        break;
      }
    }
    if ( !erased )
      break;
  }
}

QgsOgrDatasetSharedPtr QgsOgrProviderUtils::getAlreadyOpenedDataset( const QString &dsName )
{
  QMutexLocker locker( sGlobalMutex() );
  for ( auto iter = sMapSharedDS.begin(); iter != sMapSharedDS.end(); ++iter )
  {
    auto ident = iter.key();
    if ( ident.dsName == dsName && ident.updateMode )
    {
      // Browse through this list, to look for the first available DatasetWithLayers*
      // instance that is in update mode (hoping there's only one...)
      auto &datasetList = iter.value();
      for ( const auto &ds : datasetList )
      {
        Q_ASSERT( ds->refCount > 0 );
        return QgsOgrDataset::create( ident, ds );
      }
    }
  }
  return nullptr;
}

QgsOgrLayerUniquePtr QgsOgrProviderUtils::getLayer( const QString &dsName,
    int layerIndex,
    QString &errCause )
{
  QMutexLocker locker( sGlobalMutex() );
  for ( auto iter = sMapSharedDS.begin(); iter != sMapSharedDS.end(); ++iter )
  {
    if ( iter.key().dsName == dsName )
    {
      // Browse through this list, to look for a DatasetWithLayers*
      // instance that don't use yet our layer of interest
      auto &datasetList = iter.value();
      const auto constDatasetList = datasetList;
      for ( QgsOgrProviderUtils::DatasetWithLayers *ds : constDatasetList )
      {
        if ( !ds->canBeShared )
          continue;
        Q_ASSERT( ds->refCount > 0 );

        QString layerName;
        OGRLayerH hLayer;
        {
          QMutexLocker lockerDS( &ds->mutex );
          hLayer = GDALDatasetGetLayer(
                     ds->hDS, layerIndex );
          if ( hLayer )
          {
            layerName = QString::fromUtf8( OGR_L_GetName( hLayer ) );
          }
        }
        if ( !hLayer )
        {
          errCause = QObject::tr( "Cannot find layer %1." ).arg( layerIndex );
          return nullptr;
        }
        return getLayer( dsName, iter.key().updateMode, iter.key().options, layerName, errCause, true );
      }
    }
  }
  return getLayer( dsName, false, QStringList(), layerIndex, errCause, true );
}

QgsOgrLayerUniquePtr QgsOgrProviderUtils::getLayer( const QString &dsName,
    bool updateMode,
    const QStringList &options,
    int layerIndex,
    QString &errCause,
    bool checkModificationDateAgainstCache )
{
  QMutexLocker locker( sGlobalMutex() );

  // The idea is that we want to minimize the number of GDALDatasetH
  // handles openeded. But we have constraints. We do not want that 2
  // callers of getLayer() with the same input parameters get the same
  // GDALDatasetH since iterators over features of that layer would conflict

  QgsOgrProviderUtils::DatasetIdentification ident;
  ident.dsName = dsName;
  ident.updateMode = updateMode;
  ident.options = options;
  // Find if there's a list of DatasetWithLayers* that match our
  // (dsName, updateMode, options) criteria
  auto iter = sMapSharedDS.find( ident );
  if ( iter != sMapSharedDS.end() )
  {
    // Browse through this list, to look for a DatasetWithLayers*
    // instance that don't use yet our layer of interest
    auto datasetList = iter.value();
    const auto constDatasetList = datasetList;
    for ( QgsOgrProviderUtils::DatasetWithLayers *ds : constDatasetList )
    {
      if ( !ds->canBeShared )
        continue;
      Q_ASSERT( ds->refCount > 0 );

      QString layerName;
      OGRLayerH hLayer;
      {
        QMutexLocker lockerDS( &ds->mutex );
        hLayer = GDALDatasetGetLayer(
                   ds->hDS, layerIndex );
        if ( hLayer )
        {
          layerName = QString::fromUtf8( OGR_L_GetName( hLayer ) );
        }
      }
      if ( !hLayer )
      {
        errCause = QObject::tr( "Cannot find layer %1." ).arg( layerIndex );
        return nullptr;
      }
      return getLayer( dsName, updateMode, options, layerName, errCause,
                       checkModificationDateAgainstCache );
    }
  }

  GDALDatasetH hDS = OpenHelper( dsName, updateMode, options );
  if ( !hDS )
  {
    errCause = QObject::tr( "Cannot open %1 (%2)." ).arg( dsName, QString::fromUtf8( CPLGetLastErrorMsg() ) );
    return nullptr;
  }

  OGRLayerH hLayer = GDALDatasetGetLayer(
                       hDS, layerIndex );
  if ( !hLayer )
  {
    QgsOgrProviderUtils::GDALCloseWrapper( hDS );
    errCause = QObject::tr( "Cannot find layer %1." ).arg( layerIndex );
    return nullptr;
  }
  QString layerName = QString::fromUtf8( OGR_L_GetName( hLayer ) );

  QgsOgrProviderUtils::DatasetWithLayers *ds =
    new QgsOgrProviderUtils::DatasetWithLayers;
  ds->hDS = hDS;

  GDALDriverH driver = GDALGetDatasetDriver( hDS );
  QString driverName = GDALGetDriverShortName( driver );
  ds->canBeShared = canDriverShareSameDatasetAmongLayers( driverName, updateMode, dsName );

  QgsOgrLayerUniquePtr layer = QgsOgrLayer::CreateForLayer(
                                 ident, layerName, ds, hLayer );
  ds->setLayers[layerName] = layer.get();

  QList<DatasetWithLayers *> datasetList;
  datasetList.push_back( ds );
  sMapSharedDS[ident] = datasetList;

  return layer;
}

QgsOgrLayerUniquePtr QgsOgrProviderUtils::getLayer( const QString &dsName,
    const QString &layerName,
    QString &errCause )
{
  QMutexLocker locker( sGlobalMutex() );

  for ( auto iter = sMapSharedDS.begin(); iter != sMapSharedDS.end(); ++iter )
  {
    if ( iter.key().dsName == dsName )
    {
      // Browse through this list, to look for a DatasetWithLayers*
      // instance that don't use yet our layer of interest
      auto &datasetList = iter.value();
      const auto constDatasetList = datasetList;
      for ( QgsOgrProviderUtils::DatasetWithLayers *ds : constDatasetList )
      {
        if ( !ds->canBeShared )
          continue;
        Q_ASSERT( ds->refCount > 0 );

        auto iter2 = ds->setLayers.find( layerName );
        if ( iter2 == ds->setLayers.end() )
        {
          OGRLayerH hLayer;
          {
            QMutexLocker lockerDS( &ds->mutex );
            hLayer = GDALDatasetGetLayerByName(
                       ds->hDS, layerName.toUtf8().constData() );
          }
          if ( !hLayer )
          {
            // Shouldn't happen really !
            errCause = QObject::tr( "Cannot find layer %1." ).arg( layerName );
            return nullptr;
          }
          OGR_L_SetAttributeFilter( hLayer, nullptr );

          QgsOgrLayerUniquePtr layer = QgsOgrLayer::CreateForLayer(
                                         iter.key(), layerName, ds, hLayer );
          ds->setLayers[layerName] = layer.get();
          return layer;
        }
      }
    }
  }
  return getLayer( dsName, false, QStringList(), layerName, errCause, true );
}

static QDateTime getLastModified( const QString &dsName )
{
  if ( dsName.endsWith( ".gpkg", Qt::CaseInsensitive ) )
  {
    QFileInfo info( dsName + "-wal" );
    if ( info.exists() )
      return info.lastModified();
  }
  return QFileInfo( dsName ).lastModified();
}

// In case we do very fast structural changes within the same second,
// the last modified date might not change enough, so artificially
// decrement the cache modified date, so that the file appears newer to it
void QgsOgrProviderUtils::invalidateCachedLastModifiedDate( const QString &dsName )
{
  QMutexLocker locker( sGlobalMutex() );

  auto iter = sMapDSNameToLastModifiedDate()->find( dsName );
  if ( iter != sMapDSNameToLastModifiedDate()->end() )
  {
    QgsDebugMsgLevel( QStringLiteral( "invalidating last modified date for %1" ).arg( dsName ), 2 );
    iter.value() = iter.value().addSecs( -10 );
  }
}

OGRwkbGeometryType QgsOgrProviderUtils::ogrTypeFromQgisType( QgsWkbTypes::Type type )
{
  switch ( type )
  {
    case QgsWkbTypes::Point:
      return wkbPoint;
    case QgsWkbTypes::Point25D:
    case QgsWkbTypes::PointZ:
      return wkbPoint25D;
    case QgsWkbTypes::PointM:
      return wkbPointM;
    case QgsWkbTypes::PointZM:
      return wkbPointZM;

    case QgsWkbTypes::LineString:
      return wkbLineString;
    case QgsWkbTypes::LineString25D:
    case QgsWkbTypes::LineStringZ:
      return wkbLineString25D;
    case QgsWkbTypes::LineStringM:
      return wkbLineStringM;
    case QgsWkbTypes::LineStringZM:
      return wkbLineStringZM;

    case QgsWkbTypes::Polygon:
      return wkbPolygon;
    case QgsWkbTypes::Polygon25D:
    case QgsWkbTypes::PolygonZ:
      return wkbPolygon25D;
    case QgsWkbTypes::PolygonM:
      return wkbPolygonM;
    case QgsWkbTypes::PolygonZM:
      return wkbPolygonZM;

    case QgsWkbTypes::MultiPoint:
      return wkbMultiPoint;
    case QgsWkbTypes::MultiPoint25D:
    case QgsWkbTypes::MultiPointZ:
      return wkbMultiPoint25D;
    case QgsWkbTypes::MultiPointM:
      return wkbMultiPointM;
    case QgsWkbTypes::MultiPointZM:
      return wkbMultiPointZM;

    case QgsWkbTypes::MultiLineString:
      return wkbMultiLineString;
    case QgsWkbTypes::MultiLineString25D:
    case QgsWkbTypes::MultiLineStringZ:
      return wkbMultiLineString25D;
    case QgsWkbTypes::MultiLineStringM:
      return wkbMultiLineStringM;
    case QgsWkbTypes::MultiLineStringZM:
      return wkbMultiLineStringZM;

    case QgsWkbTypes::MultiPolygon:
      return wkbMultiPolygon;
    case QgsWkbTypes::MultiPolygon25D:
    case QgsWkbTypes::MultiPolygonZ:
      return wkbMultiPolygon25D;
    case QgsWkbTypes::MultiPolygonM:
      return wkbMultiPolygonM;
    case QgsWkbTypes::MultiPolygonZM:
      return wkbMultiPolygonZM;

    case QgsWkbTypes::CircularString:
      return wkbCircularString;
    case QgsWkbTypes::CircularStringZ:
      return wkbCircularStringZ;
    case QgsWkbTypes::CircularStringM:
      return wkbCircularStringM;
    case QgsWkbTypes::CircularStringZM:
      return wkbCircularStringZM;

    case QgsWkbTypes::CompoundCurve:
      return wkbCompoundCurve;
    case QgsWkbTypes::CompoundCurveZ:
      return wkbCompoundCurveZ;
    case QgsWkbTypes::CompoundCurveM:
      return wkbCompoundCurveM;
    case QgsWkbTypes::CompoundCurveZM:
      return wkbCompoundCurveZM;

    case QgsWkbTypes::CurvePolygon:
      return wkbCurvePolygon;
    case QgsWkbTypes::CurvePolygonZ:
      return wkbCurvePolygonZ;
    case QgsWkbTypes::CurvePolygonM:
      return wkbCurvePolygonM;
    case QgsWkbTypes::CurvePolygonZM:
      return wkbCurvePolygonZM;

    case QgsWkbTypes::MultiCurve:
      return wkbMultiCurve;
    case QgsWkbTypes::MultiCurveZ:
      return wkbMultiCurveZ;
    case QgsWkbTypes::MultiCurveM:
      return wkbMultiCurveM;
    case QgsWkbTypes::MultiCurveZM:
      return wkbMultiCurveZM;

    case QgsWkbTypes::MultiSurface:
      return wkbMultiSurface;
    case QgsWkbTypes::MultiSurfaceZ:
      return wkbMultiSurfaceZ;
    case QgsWkbTypes::MultiSurfaceM:
      return wkbMultiSurfaceM;
    case QgsWkbTypes::MultiSurfaceZM:
      return wkbMultiSurfaceZM;

    case QgsWkbTypes::Triangle:
      return wkbTriangle;
    case QgsWkbTypes::TriangleZ:
      return wkbTriangleZ;
    case QgsWkbTypes::TriangleM:
      return wkbTriangleM;
    case QgsWkbTypes::TriangleZM:
      return wkbTriangleZM;

    case QgsWkbTypes::NoGeometry:
      return wkbNone;

    case QgsWkbTypes::GeometryCollection:
      return wkbGeometryCollection;
    case QgsWkbTypes::GeometryCollectionZ:
      return wkbGeometryCollection25D;
    case QgsWkbTypes::GeometryCollectionM:
      return wkbGeometryCollectionM;
    case QgsWkbTypes::GeometryCollectionZM:
      return wkbGeometryCollectionZM;

    case QgsWkbTypes::Unknown:
      return wkbUnknown;
  }
  // no warnings!
  return wkbUnknown;
}

QgsWkbTypes::Type QgsOgrProviderUtils::qgisTypeFromOgrType( OGRwkbGeometryType type )
{
  switch ( type )
  {
    case wkbUnknown:
      return QgsWkbTypes::Unknown;
    case wkbPoint:
      return QgsWkbTypes::Point;
    case wkbLineString:
      return QgsWkbTypes::LineString;
    case wkbPolygon:
      return QgsWkbTypes::Polygon;
    case wkbMultiPoint:
      return QgsWkbTypes::MultiPoint;
    case wkbMultiLineString:
      return QgsWkbTypes::MultiLineString;
    case wkbMultiPolygon:
      return QgsWkbTypes::MultiPolygon;
    case wkbGeometryCollection:
      return QgsWkbTypes::GeometryCollection;
    case wkbCircularString:
      return QgsWkbTypes::CircularString;
    case wkbCompoundCurve:
      return QgsWkbTypes::CompoundCurve;
    case wkbCurvePolygon:
      return QgsWkbTypes::CurvePolygon;
    case wkbMultiCurve:
      return QgsWkbTypes::MultiCurve;
    case wkbMultiSurface:
      return QgsWkbTypes::MultiSurface;
    case wkbTriangle:
      return QgsWkbTypes::Triangle;
    case wkbNone:
      return QgsWkbTypes::NoGeometry;

    case wkbCircularStringZ:
      return QgsWkbTypes::CircularStringZ;
    case wkbCompoundCurveZ:
      return QgsWkbTypes::CompoundCurveZ;
    case wkbCurvePolygonZ:
      return QgsWkbTypes::PolygonZ;
    case wkbMultiCurveZ:
      return QgsWkbTypes::MultiCurveZ;
    case wkbMultiSurfaceZ:
      return QgsWkbTypes::MultiSurfaceZ;
    case wkbTriangleZ:
      return QgsWkbTypes::TriangleZ;

    case wkbPointM:
      return QgsWkbTypes::PointM;
    case wkbLineStringM:
      return QgsWkbTypes::LineStringM;
    case wkbPolygonM:
      return QgsWkbTypes::PolygonM;
    case wkbMultiPointM:
      return QgsWkbTypes::PointM;
    case wkbMultiLineStringM:
      return QgsWkbTypes::LineStringM;
    case wkbMultiPolygonM:
      return QgsWkbTypes::PolygonM;
    case wkbGeometryCollectionM:
      return QgsWkbTypes::GeometryCollectionM;
    case wkbCircularStringM:
      return QgsWkbTypes::CircularStringM;
    case wkbCompoundCurveM:
      return QgsWkbTypes::CompoundCurveM;
    case wkbCurvePolygonM:
      return QgsWkbTypes::PolygonM;
    case wkbMultiCurveM:
      return QgsWkbTypes::MultiCurveM;
    case wkbMultiSurfaceM:
      return QgsWkbTypes::MultiSurfaceM;
    case wkbTriangleM:
      return QgsWkbTypes::TriangleM;

    case wkbPointZM:
      return QgsWkbTypes::PointZM;
    case wkbLineStringZM:
      return QgsWkbTypes::LineStringZM;
    case wkbPolygonZM:
      return QgsWkbTypes::PolygonZM;
    case wkbMultiPointZM:
      return QgsWkbTypes::MultiPointZM;
    case wkbMultiLineStringZM:
      return QgsWkbTypes::MultiLineStringZM;
    case wkbMultiPolygonZM:
      return QgsWkbTypes::MultiPolygonZM;
    case wkbGeometryCollectionZM:
      return QgsWkbTypes::GeometryCollectionZM;
    case wkbCircularStringZM:
      return QgsWkbTypes::CircularStringZM;
    case wkbCompoundCurveZM:
      return QgsWkbTypes::CompoundCurveZM;
    case wkbCurvePolygonZM:
      return QgsWkbTypes::CurvePolygonZM;
    case wkbMultiCurveZM:
      return QgsWkbTypes::MultiCurveZM;
    case wkbMultiSurfaceZM:
      return QgsWkbTypes::MultiSurfaceZM;
    case wkbTriangleZM:
      return QgsWkbTypes::TriangleZM;

    case wkbPoint25D:
      return QgsWkbTypes::Point25D;
    case wkbLineString25D:
      return QgsWkbTypes::LineString25D;
    case wkbPolygon25D:
      return QgsWkbTypes::Polygon25D;
    case wkbMultiPoint25D:
      return QgsWkbTypes::MultiPoint25D;
    case wkbMultiLineString25D:
      return QgsWkbTypes::MultiLineString25D;
    case wkbMultiPolygon25D:
      return QgsWkbTypes::MultiPolygon25D;
    case wkbGeometryCollection25D:
      return QgsWkbTypes::GeometryCollectionZ;

    case wkbCurve:
    case wkbSurface:
    case wkbCurveZ:
    case wkbSurfaceZ:
    case wkbCurveM:
    case wkbSurfaceM:
    case wkbCurveZM:
    case wkbSurfaceZM:
      return QgsWkbTypes::Unknown; // abstract types - no direct mapping to QGIS types

    case wkbLinearRing:
    case wkbTIN:
    case wkbTINZ:
    case wkbTINM:
    case wkbTINZM:
    case wkbPolyhedralSurface:
    case wkbPolyhedralSurfaceZ:
    case wkbPolyhedralSurfaceM:
    case wkbPolyhedralSurfaceZM:
      return QgsWkbTypes::Unknown; // unsupported types
  }
  return QgsWkbTypes::Unknown;
}

OGRwkbGeometryType QgsOgrProviderUtils::ogrWkbGeometryTypeFromName( const QString &typeName )
{
  if ( typeName == QLatin1String( "Point" ) )
    return wkbPoint;
  else if ( typeName == QLatin1String( "LineString" ) )
    return wkbLineString;
  else if ( typeName == QLatin1String( "Polygon" ) )
    return wkbPolygon;
  else if ( typeName == QLatin1String( "MultiPoint" ) )
    return wkbMultiPoint;
  else if ( typeName == QLatin1String( "MultiLineString" ) )
    return wkbMultiLineString;
  else if ( typeName == QLatin1String( "MultiPolygon" ) )
    return wkbMultiPolygon;
  else if ( typeName == QLatin1String( "GeometryCollection" ) )
    return wkbGeometryCollection;
  else if ( typeName == QLatin1String( "None" ) )
    return wkbNone;
  else if ( typeName == QLatin1String( "Point25D" ) )
    return wkbPoint25D;
  else if ( typeName == QLatin1String( "LineString25D" ) )
    return wkbLineString25D;
  else if ( typeName == QLatin1String( "Polygon25D" ) )
    return wkbPolygon25D;
  else if ( typeName == QLatin1String( "MultiPoint25D" ) )
    return wkbMultiPoint25D;
  else if ( typeName == QLatin1String( "MultiLineString25D" ) )
    return wkbMultiLineString25D;
  else if ( typeName == QLatin1String( "MultiPolygon25D" ) )
    return wkbMultiPolygon25D;
  else if ( typeName == QLatin1String( "GeometryCollection25D" ) )
    return wkbGeometryCollection25D;

  QgsDebugMsg( QStringLiteral( "unknown geometry type: %1" ).arg( typeName ) );
  return wkbUnknown;
}

OGRwkbGeometryType QgsOgrProviderUtils::ogrWkbSingleFlatten( OGRwkbGeometryType type )
{
  type = wkbFlatten( type );
  switch ( type )
  {
    case wkbMultiPoint:
      return wkbPoint;
    case wkbMultiLineString:
      return wkbLineString;
    case wkbMultiPolygon:
      return wkbPolygon;
    case wkbMultiCurve:
      return wkbCompoundCurve;
    case wkbMultiSurface:
      return wkbCurvePolygon;
    default:
      return type;
  }
}

QString QgsOgrProviderUtils::ogrWkbGeometryTypeName( OGRwkbGeometryType type )
{
  QString geom;

  // GDAL 2.1 can return M/ZM geometries
  if ( wkbHasM( type ) )
  {
    geom = ogrWkbGeometryTypeName( wkbFlatten( type ) );
    if ( wkbHasZ( type ) )
      geom += QLatin1Char( 'Z' );
    if ( wkbHasM( type ) )
      geom += QLatin1Char( 'M' );
    return geom;
  }

  switch ( static_cast<unsigned>( type ) )
  {
    case wkbUnknown:
      geom = QStringLiteral( "Unknown" );
      break;
    case wkbPoint:
      geom = QStringLiteral( "Point" );
      break;
    case wkbLineString:
      geom = QStringLiteral( "LineString" );
      break;
    case wkbPolygon:
      geom = QStringLiteral( "Polygon" );
      break;
    case wkbMultiPoint:
      geom = QStringLiteral( "MultiPoint" );
      break;
    case wkbMultiLineString:
      geom = QStringLiteral( "MultiLineString" );
      break;
    case wkbMultiPolygon:
      geom = QStringLiteral( "MultiPolygon" );
      break;
    case wkbGeometryCollection:
      geom = QStringLiteral( "GeometryCollection" );
      break;
    case wkbCircularString:
      geom = QStringLiteral( "CircularString" );
      break;
    case wkbCompoundCurve:
      geom = QStringLiteral( "CompoundCurve" );
      break;
    case wkbCurvePolygon:
      geom = QStringLiteral( "CurvePolygon" );
      break;
    case wkbMultiCurve:
      geom = QStringLiteral( "MultiCurve" );
      break;
    case wkbMultiSurface:
      geom = QStringLiteral( "MultiSurface" );
      break;
    case wkbCircularStringZ:
      geom = QStringLiteral( "CircularStringZ" );
      break;
    case wkbCompoundCurveZ:
      geom = QStringLiteral( "CompoundCurveZ" );
      break;
    case wkbCurvePolygonZ:
      geom = QStringLiteral( "CurvePolygonZ" );
      break;
    case wkbMultiCurveZ:
      geom = QStringLiteral( "MultiCurveZ" );
      break;
    case wkbMultiSurfaceZ:
      geom = QStringLiteral( "MultiSurfaceZ" );
      break;
    case wkbNone:
      geom = QStringLiteral( "None" );
      break;
    case static_cast<unsigned>( wkbUnknown ) | static_cast<unsigned>( wkb25DBit ):
      geom = QStringLiteral( "Unknown25D" );
      break;
    case static_cast<unsigned>( wkbPoint25D ):
      geom = QStringLiteral( "Point25D" );
      break;
    case static_cast<unsigned>( wkbLineString25D ):
      geom = QStringLiteral( "LineString25D" );
      break;
    case static_cast<unsigned>( wkbPolygon25D ):
      geom = QStringLiteral( "Polygon25D" );
      break;
    case static_cast<unsigned>( wkbMultiPoint25D ):
      geom = QStringLiteral( "MultiPoint25D" );
      break;
    case static_cast<unsigned>( wkbMultiLineString25D ):
      geom = QStringLiteral( "MultiLineString25D" );
      break;
    case static_cast<unsigned>( wkbMultiPolygon25D ):
      geom = QStringLiteral( "MultiPolygon25D" );
      break;
    case static_cast<unsigned>( wkbGeometryCollection25D ):
      geom = QStringLiteral( "GeometryCollection25D" );
      break;
    default:
      // Do not use ':', as it will mess with the separator used by QgsSublayersDialog::populateLayers()
      geom = QStringLiteral( "Unknown WKB (%1)" ).arg( type );
  }
  return geom;
}

OGRwkbGeometryType QgsOgrProviderUtils::resolveGeometryTypeForFeature( OGRFeatureH feature, const QString &driverName )
{
  if ( OGRGeometryH geom = OGR_F_GetGeometryRef( feature ) )
  {
    OGRwkbGeometryType gType = OGR_G_GetGeometryType( geom );

    // ESRI MultiPatch can be reported as GeometryCollectionZ of TINZ
    if ( wkbFlatten( gType ) == wkbGeometryCollection &&
         ( driverName == QLatin1String( "ESRI Shapefile" ) || driverName == QLatin1String( "OpenFileGDB" ) || driverName == QLatin1String( "FileGDB" ) ) &&
         OGR_G_GetGeometryCount( geom ) >= 1 &&
         wkbFlatten( OGR_G_GetGeometryType( OGR_G_GetGeometryRef( geom, 0 ) ) ) == wkbTIN )
    {
      gType = wkbMultiPolygon25D;
    }
    return gType;
  }
  else
  {
    return wkbNone;
  }
}

QString QgsOgrProviderUtils::expandAuthConfig( const QString &dsName )
{
  QString uri( dsName );
  // Check for authcfg
  QRegularExpression authcfgRe( " authcfg='([^']+)'" );
  QRegularExpressionMatch match;
  if ( uri.contains( authcfgRe, &match ) )
  {
    uri = uri.remove( match.captured( 0 ) );
    QString configId( match.captured( 1 ) );
    QStringList connectionItems;
    connectionItems << uri;
    if ( QgsApplication::authManager()->updateDataSourceUriItems( connectionItems, configId, QStringLiteral( "ogr" ) ) )
    {
      uri = connectionItems.first( );
    }
  }
  return uri;
}

// Must be called under the globalMutex
bool QgsOgrProviderUtils::canUseOpenedDatasets( const QString &dsName )
{
  auto iter = sMapDSNameToLastModifiedDate()->find( dsName );
  if ( iter == sMapDSNameToLastModifiedDate()->end() )
    return true;
  return getLastModified( dsName ) <= iter.value();
}

QgsOgrProviderUtils::DatasetWithLayers *QgsOgrProviderUtils::createDatasetWithLayers(
  const QString &dsName,
  bool updateMode,
  const QStringList &options,
  const QString &layerName,
  const DatasetIdentification &ident,
  QgsOgrLayerUniquePtr &layer,
  QString &errCause )
{
  GDALDatasetH hDS = OpenHelper( dsName, updateMode, options );
  if ( !hDS )
  {
    errCause = QObject::tr( "Cannot open %1." ).arg( dsName );
    return nullptr;
  }
  ( *sMapDSNameToLastModifiedDate() )[dsName] = getLastModified( dsName );

  OGRLayerH hLayer = GDALDatasetGetLayerByName(
                       hDS, layerName.toUtf8().constData() );
  if ( !hLayer )
  {
    errCause = QObject::tr( "Cannot find layer %1." ).arg( layerName );
    QgsOgrProviderUtils::GDALCloseWrapper( hDS );
    return nullptr;
  }

  QgsOgrProviderUtils::DatasetWithLayers *ds =
    new QgsOgrProviderUtils::DatasetWithLayers;
  ds->hDS = hDS;

  GDALDriverH driver = GDALGetDatasetDriver( hDS );
  QString driverName = GDALGetDriverShortName( driver );
  ds->canBeShared = canDriverShareSameDatasetAmongLayers( driverName, updateMode, dsName );

  layer = QgsOgrLayer::CreateForLayer(
            ident, layerName, ds, hLayer );
  ds->setLayers[layerName] = layer.get();
  return ds;
}

QgsOgrLayerUniquePtr QgsOgrProviderUtils::getLayer( const QString &dsName,
    bool updateMode,
    const QStringList &options,
    const QString &layerName,
    QString &errCause,
    bool checkModificationDateAgainstCache )
{
  QMutexLocker locker( sGlobalMutex() );

  // The idea is that we want to minimize the number of GDALDatasetH
  // handles openeded. But we have constraints. We do not want that 2
  // callers of getLayer() with the same input parameters get the same
  // GDALDatasetH since iterators over features of that layer would conflict

  QgsOgrProviderUtils::DatasetIdentification ident;
  ident.dsName = dsName;
  ident.updateMode = updateMode;
  ident.options = options;
  // Find if there's a list of DatasetWithLayers* that match our
  // (dsName, updateMode, options) criteria
  auto iter = sMapSharedDS.find( ident );
  if ( iter != sMapSharedDS.end() )
  {
    if ( checkModificationDateAgainstCache && !canUseOpenedDatasets( dsName ) )
    {
      QgsDebugMsg( QStringLiteral( "Cannot reuse existing opened dataset(s) on %1 since it has been modified" ).arg( dsName ) );
      invalidateCachedDatasets( dsName );
      iter = sMapSharedDS.find( ident );
      Q_ASSERT( iter == sMapSharedDS.end() );
    }
  }
  if ( iter != sMapSharedDS.end() )
  {
    // Browse through this list, to look for a DatasetWithLayers*
    // instance that don't use yet our layer of interest
    auto &datasetList = iter.value();
    const auto constDatasetList = datasetList;
    for ( QgsOgrProviderUtils::DatasetWithLayers *ds : constDatasetList )
    {
      if ( !ds->canBeShared )
        continue;
      Q_ASSERT( ds->refCount > 0 );

      auto iter2 = ds->setLayers.find( layerName );
      if ( iter2 == ds->setLayers.end() )
      {
        OGRLayerH hLayer;
        {
          QMutexLocker lockerDS( &ds->mutex );
          hLayer = GDALDatasetGetLayerByName(
                     ds->hDS, layerName.toUtf8().constData() );
        }
        if ( !hLayer )
        {
          // Shouldn't happen really !
          errCause = QObject::tr( "Cannot find layer %1." ).arg( layerName );
          return nullptr;
        }
        OGR_L_SetAttributeFilter( hLayer, nullptr );

        QgsOgrLayerUniquePtr layer = QgsOgrLayer::CreateForLayer(
                                       ident, layerName, ds, hLayer );
        ds->setLayers[layerName] = layer.get();
        return layer;
      }
    }

    // All existing DatasetWithLayers* already reference our layer of
    // interest, so instantiate a new DatasetWithLayers*
    QgsOgrLayerUniquePtr layer;
    QgsOgrProviderUtils::DatasetWithLayers *ds =
      createDatasetWithLayers( dsName, updateMode, options, layerName, ident, layer, errCause );
    if ( !ds )
      return nullptr;

    datasetList.push_back( ds );

    return layer;
  }

  QgsOgrLayerUniquePtr layer;
  QgsOgrProviderUtils::DatasetWithLayers *ds =
    createDatasetWithLayers( dsName, updateMode, options, layerName, ident, layer, errCause );
  if ( !ds )
    return nullptr;

  QList<DatasetWithLayers *> datasetList;
  datasetList.push_back( ds );
  sMapSharedDS[ident] = datasetList;

  return layer;
}

QgsOgrLayerUniquePtr QgsOgrProviderUtils::getSqlLayer( QgsOgrLayer *baseLayer,
    OGRLayerH hSqlLayer,
    const QString &sql )
{
  QgsOgrProviderUtils::DatasetIdentification ident;
  ident.dsName = baseLayer->datasetName();
  ident.updateMode = baseLayer->updateMode();
  ident.options = baseLayer->options();
  return QgsOgrLayer::CreateForSql( ident, sql, baseLayer->ds, hSqlLayer );
}

void QgsOgrProviderUtils::releaseInternal( const DatasetIdentification &ident,
    DatasetWithLayers *ds,
    bool removeFromDatasetList )
{

  ds->refCount --;
  if ( ds->refCount == 0 )
  {
    Q_ASSERT( ds->setLayers.isEmpty() );

    if ( removeFromDatasetList )
    {
      auto iter = sMapSharedDS.find( ident );
      if ( iter != sMapSharedDS.end() )
      {
        auto &datasetList = iter.value();
        int i = 0;

        // Normally there should be a match, except for datasets that
        // have been invalidated
        const auto constDatasetList = datasetList;
        for ( QgsOgrProviderUtils::DatasetWithLayers *dsIter : constDatasetList )
        {
          if ( dsIter == ds )
          {
            datasetList.removeAt( i );
            break;
          }
          i ++;
        }

        if ( datasetList.isEmpty() )
          sMapSharedDS.erase( iter );
      }
    }
    QgsOgrProviderUtils::GDALCloseWrapper( ds->hDS );
    delete ds;
  }
}

void QgsOgrProviderUtils::release( QgsOgrLayer *&layer )
{
  if ( !layer )
    return;

  QMutexLocker locker( sGlobalMutex() );

  if ( !layer->isSqlLayer )
  {
    layer->ds->setLayers.remove( layer->layerName );
  }
  else
  {
    QMutexLocker lockerDS( &layer->ds->mutex );
    GDALDatasetReleaseResultSet( layer->ds->hDS, layer->hLayer );
  }

  releaseInternal( layer->ident, layer->ds,  !layer->isSqlLayer );

  delete layer;
  layer = nullptr;
}


void QgsOgrProviderUtils::releaseDataset( QgsOgrDataset *ds )
{
  if ( !ds )
    return;

  QMutexLocker locker( sGlobalMutex() );
  releaseInternal( ds->mIdent, ds->mDs, true );
  delete ds;
}

bool QgsOgrProviderUtils::canDriverShareSameDatasetAmongLayers( const QString &driverName )
{
  return driverName != QLatin1String( "OSM" );
}

bool QgsOgrProviderUtils::canDriverShareSameDatasetAmongLayers( const QString &driverName,
    bool updateMode,
    const QString &dsName )
{
  // For .shp.zip with multiple layers, in update mode, we want that each
  // layer has its own dataset, so that when its gets closed and reopened,
  // the .shp.zip is updated. Otherwise if we share the same dataset, the .shp.zip
  // would only be updated when all layers are unloaded, and thus readers will see
  // an outdated version of the .shp.zip. This works only if editing operations are
  // done separately on layers (which is how it works from the GUI)
  return canDriverShareSameDatasetAmongLayers( driverName ) &&
         !( updateMode && dsName.endsWith( QLatin1String( ".shp.zip" ), Qt::CaseInsensitive ) );
}

QList< QgsProviderSublayerDetails > QgsOgrProviderUtils::querySubLayerList( int i, QgsOgrLayer *layer, GDALDatasetH hDS, const QString &driverName, Qgis::SublayerQueryFlags flags, const QString &baseUri, bool hasSingleLayerOnly, QgsFeedback *feedback )
{
  const QString layerName = QString::fromUtf8( layer->name() );

  QStringList privateLayerNames { QStringLiteral( "layer_styles" ),
                                  QStringLiteral( "qgis_projects" )};
  if ( driverName == QLatin1String( "SQLite" ) )
  {
    privateLayerNames.append( QgsSqliteUtils::systemTables() );
  }

  Qgis::SublayerFlags layerFlags;
  if ( ( driverName == QLatin1String( "SQLite" ) && layerName.contains( QRegularExpression( QStringLiteral( "idx_.*_geom(etry)?($|_.*)" ), QRegularExpression::PatternOption::CaseInsensitiveOption ) ) )
       || privateLayerNames.contains( layerName ) )
  {
    layerFlags |= Qgis::SublayerFlag::SystemTable;
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,0)
  // mark private layers, using GDAL API
  if ( hDS && GDALDatasetIsLayerPrivate( hDS, i ) )
  {
    layerFlags |= Qgis::SublayerFlag::SystemTable;
  }
#else
  ( void )hDS;
#endif

  if ( !( flags & Qgis::SublayerQueryFlag::IncludeSystemTables ) && ( layerFlags & Qgis::SublayerFlag::SystemTable ) )
  {
    // layer is a system table, and we are not scanning for them
    return {};
  }

  // Get first column name,
  // TODO: add support for multiple
  QString geometryColumnName;
  OGRwkbGeometryType layerGeomType = wkbUnknown;
  const bool slowGeomTypeRetrieval = driverName == QLatin1String( "OAPIF" ) || driverName == QLatin1String( "WFS3" );
  if ( !slowGeomTypeRetrieval )
  {
    QgsOgrFeatureDefn &fdef = layer->GetLayerDefn();
    if ( fdef.GetGeomFieldCount() )
    {
      OGRGeomFieldDefnH geomH = fdef.GetGeomFieldDefn( 0 );
      geometryColumnName = QString::fromUtf8( OGR_GFld_GetNameRef( geomH ) );
    }
    layerGeomType = fdef.GetGeomType();
  }

  QString longDescription;
  if ( driverName == QLatin1String( "OAPIF" ) || driverName == QLatin1String( "WFS3" ) )
  {
    longDescription = layer->GetMetadataItem( QStringLiteral( "TITLE" ) );
  }

  QgsDebugMsgLevel( QStringLiteral( "id = %1 name = %2 layerGeomType = %3 longDescription = %4" ).arg( i ).arg( layerName ).arg( layerGeomType ). arg( longDescription ), 2 );


  QVariantMap parts = QgsOgrProviderMetadata().decodeUri( baseUri );
  if ( !hasSingleLayerOnly )
    parts.insert( QStringLiteral( "layerName" ), layerName );
  else
    parts.remove( QStringLiteral( "layerName" ) );

  if ( slowGeomTypeRetrieval || wkbFlatten( layerGeomType ) != wkbUnknown || !( flags & Qgis::SublayerQueryFlag::ResolveGeometryType ) )
  {
    const long long layerFeatureCount = ( flags & Qgis::SublayerQueryFlag::CountFeatures )
                                        ? layer->GetApproxFeatureCount()
                                        : static_cast< long >( Qgis::FeatureCountState::Uncounted );

    QgsProviderSublayerDetails details;
    details.setLayerNumber( i );
    if ( parts.value( QStringLiteral( "vsiSuffix" ) ).toString().isEmpty() )
      details.setName( layerName );
    else
    {
      if ( parts.value( QStringLiteral( "layerName" ) ).toString().isEmpty() )
        parts.insert( QStringLiteral( "layerName" ), layerName );
      if ( hasSingleLayerOnly )
      {
        details.setName( parts.value( QStringLiteral( "vsiSuffix" ) ).toString().mid( 1 ) );
      }
      else
      {
        details.setName( layerName );
      }
    }
    details.setFeatureCount( layerFeatureCount );
    details.setWkbType( QgsOgrUtils::ogrGeometryTypeToQgsWkbType( layerGeomType ) );
    details.setGeometryColumnName( geometryColumnName );
    details.setDescription( longDescription );
    details.setProviderKey( QStringLiteral( "ogr" ) );
    details.setDriverName( driverName );
    details.setFlags( layerFlags );

    const QString uri = QgsOgrProviderMetadata().encodeUri( parts );
    details.setUri( uri );

    return { details };
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Unknown geometry type, count features for each geometry type" ), 2 );
    // Add virtual sublayers for supported geometry types if layer type is unknown
    // Count features for geometry types
    QMap<OGRwkbGeometryType, int> fCount;
    QSet<OGRwkbGeometryType> fHasZ;
    // TODO: avoid reading attributes, setRelevantFields cannot be called here because it is not constant

    layer->ResetReading();
    gdal::ogr_feature_unique_ptr fet;
    while ( fet.reset( layer->GetNextFeature() ), fet )
    {
      OGRwkbGeometryType gType =  resolveGeometryTypeForFeature( fet.get(), driverName );
      if ( gType != wkbNone )
      {
        bool hasZ = wkbHasZ( gType );
        gType = QgsOgrProviderUtils::ogrWkbSingleFlatten( gType );
        fCount[gType] = fCount.value( gType ) + 1;
        if ( hasZ )
          fHasZ.insert( gType );
      }

      if ( feedback && feedback->isCanceled() )
        break;
    }
    layer->ResetReading();
    // it may happen that there are no features in the layer, in that case add unknown type
    // to show to user that the layer exists but it is empty
    if ( fCount.isEmpty() )
    {
      fCount[wkbUnknown] = 0;
    }

    // List TIN and PolyhedralSurface as Polygon
    if ( fCount.contains( wkbTIN ) )
    {
      fCount[wkbPolygon] = fCount.value( wkbPolygon ) + fCount[wkbTIN];
      fCount.remove( wkbTIN );
    }
    if ( fCount.contains( wkbPolyhedralSurface ) )
    {
      fCount[wkbPolygon] = fCount.value( wkbPolygon ) + fCount[wkbPolyhedralSurface];
      fCount.remove( wkbPolyhedralSurface );
    }
    // When there are CurvePolygons, promote Polygons
    if ( fCount.contains( wkbPolygon ) && fCount.contains( wkbCurvePolygon ) )
    {
      fCount[wkbCurvePolygon] += fCount.value( wkbPolygon );
      fCount.remove( wkbPolygon );
    }
    // When there are CompoundCurves, promote LineStrings and CircularStrings
    if ( fCount.contains( wkbLineString ) && fCount.contains( wkbCompoundCurve ) )
    {
      fCount[wkbCompoundCurve] += fCount.value( wkbLineString );
      fCount.remove( wkbLineString );
    }
    if ( fCount.contains( wkbCircularString ) && fCount.contains( wkbCompoundCurve ) )
    {
      fCount[wkbCompoundCurve] += fCount.value( wkbCircularString );
      fCount.remove( wkbCircularString );
    }

    QList< QgsProviderSublayerDetails > res;
    res.reserve( fCount.size() );

    bool bIs25D = wkbHasZ( layerGeomType );
    QMap<OGRwkbGeometryType, int>::const_iterator countIt = fCount.constBegin();
    for ( ; countIt != fCount.constEnd(); ++countIt )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      QgsProviderSublayerDetails details;
      details.setLayerNumber( i );
      if ( parts.value( QStringLiteral( "vsiSuffix" ) ).toString().isEmpty() )
        details.setName( layerName );
      else
      {
        if ( parts.value( QStringLiteral( "layerName" ) ).toString().isEmpty() )
          parts.insert( QStringLiteral( "layerName" ), layerName );
        if ( hasSingleLayerOnly )
        {
          details.setName( parts.value( QStringLiteral( "vsiSuffix" ) ).toString().mid( 1 ) );
        }
        else
        {
          details.setName( layerName );
        }
      }
      details.setFeatureCount( fCount.value( countIt.key() ) );
      details.setWkbType( QgsOgrUtils::ogrGeometryTypeToQgsWkbType( ( bIs25D || fHasZ.contains( countIt.key() ) ) ? wkbSetZ( countIt.key() ) : countIt.key() ) );
      details.setGeometryColumnName( geometryColumnName );
      details.setDescription( longDescription );
      details.setProviderKey( QStringLiteral( "ogr" ) );
      details.setDriverName( driverName );
      details.setFlags( layerFlags );

      // if we had to iterate through the table to find geometry types, make sure to include these
      // in the uri for the sublayers (otherwise we'll be forced to re-do this iteration whenever
      // the uri from the sublayer is used to construct an actual vector layer)
      if ( details.wkbType() != QgsWkbTypes::Unknown )
        parts.insert( QStringLiteral( "geometryType" ),
                      ogrWkbGeometryTypeName( ( bIs25D || fHasZ.contains( countIt.key() ) ) ? wkbSetZ( countIt.key() ) : countIt.key() ) );
      else
        parts.remove( QStringLiteral( "geometryType" ) );

      details.setUri( QgsOgrProviderMetadata().encodeUri( parts ) );

      res << details;
    }
    return res;
  }
}

bool QgsOgrProviderUtils::createConnection( const QString &name, const QString &extensions, const QString &ogrDriverName )
{
  QString path = QFileDialog::getOpenFileName( nullptr, QObject::tr( "Open %1" ).arg( name ), QString(), extensions );
  return saveConnection( path, ogrDriverName );
}

bool QgsOgrProviderUtils::saveConnection( const QString &path, const QString &ogrDriverName )
{
  QFileInfo fileInfo( path );
  QString connName = fileInfo.fileName();
  if ( ! path.isEmpty() )
  {
    bool ok = true;
    while ( ok && ! QgsOgrDbConnection( connName, ogrDriverName ).path( ).isEmpty( ) )
    {

      connName = QInputDialog::getText( nullptr, QObject::tr( "Add Connection" ),
                                        QObject::tr( "A connection with the same name already exists,\nplease provide a new name:" ), QLineEdit::Normal,
                                        QString(), &ok );
    }
    if ( ok && ! connName.isEmpty() )
    {
      QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) );
      std::unique_ptr< QgsGeoPackageProviderConnection > providerConnection( qgis::down_cast<QgsGeoPackageProviderConnection *>( providerMetadata->createConnection( connName ) ) );
      providerConnection->setUri( path );
      providerMetadata->saveConnection( providerConnection.get(), connName );
      return true;
    }
  }
  return false;
}

QgsOgrDatasetSharedPtr QgsOgrDataset::create( const QgsOgrProviderUtils::DatasetIdentification &ident,
    QgsOgrProviderUtils::DatasetWithLayers *ds )
{
  QgsOgrDatasetSharedPtr dsRet = QgsOgrDatasetSharedPtr( new QgsOgrDataset(), QgsOgrProviderUtils::releaseDataset );
  dsRet->mIdent = ident;
  dsRet->mDs = ds;
  dsRet->mDs->refCount ++;
  return dsRet;
}

bool QgsOgrDataset::executeSQLNoReturn( const QString &sql )
{
  QMutexLocker locker( &mutex() );
  CPLErrorReset();
  OGRLayerH hSqlLayer = GDALDatasetExecuteSQL(
                          mDs->hDS, sql.toUtf8().constData(), nullptr, nullptr );
  bool ret = CPLGetLastErrorType() == CE_None;
  GDALDatasetReleaseResultSet( mDs->hDS, hSqlLayer );
  return ret;
}


OGRLayerH QgsOgrDataset::getLayerFromNameOrIndex( const QString &layerName, int layerIndex )
{
  QMutexLocker locker( &mutex() );

  OGRLayerH layer;
  if ( !layerName.isEmpty() )
  {
    layer = GDALDatasetGetLayerByName( mDs->hDS, layerName.toUtf8().constData() );
  }
  else
  {
    layer = GDALDatasetGetLayer( mDs->hDS, layerIndex );
  }
  return layer;
}

void QgsOgrDataset::releaseResultSet( OGRLayerH hSqlLayer )
{
  QMutexLocker locker( &mutex() );
  GDALDatasetReleaseResultSet( mDs->hDS, hSqlLayer );
}

QgsOgrLayer::QgsOgrLayer()
{
  oFDefn.layer = this;
}

QgsOgrLayerUniquePtr QgsOgrLayer::CreateForLayer(
  const QgsOgrProviderUtils::DatasetIdentification &ident,
  const QString &layerName,
  QgsOgrProviderUtils::DatasetWithLayers *ds,
  OGRLayerH hLayer )
{
  QgsOgrLayerUniquePtr layer( new QgsOgrLayer() );
  layer->ident = ident;
  layer->isSqlLayer = false;
  layer->layerName = layerName;
  layer->ds = ds;
  layer->hLayer = hLayer;
  {
    QMutexLocker locker( &ds->mutex );
    OGR_L_ResetReading( hLayer );
  }
  ds->refCount ++;
  return layer;
}

QgsOgrLayerUniquePtr QgsOgrLayer::CreateForSql(
  const QgsOgrProviderUtils::DatasetIdentification &ident,
  const QString &sql,
  QgsOgrProviderUtils::DatasetWithLayers *ds,
  OGRLayerH hLayer )
{
  QgsOgrLayerUniquePtr layer( new QgsOgrLayer() );
  layer->ident = ident;
  layer->isSqlLayer = true;
  layer->sql = sql;
  layer->ds = ds;
  layer->hLayer = hLayer;
  {
    QMutexLocker locker( &ds->mutex );
    OGR_L_ResetReading( hLayer );
  }
  ds->refCount ++;
  return layer;
}

int QgsOgrLayer::GetLayerCount()
{
  QMutexLocker locker( &ds->mutex );
  return GDALDatasetGetLayerCount( ds->hDS );
}

GDALDriverH QgsOgrLayer::driver()
{
  return GDALGetDatasetDriver( ds->hDS );
}

QString  QgsOgrLayer::driverName()
{
  return QString::fromUtf8( GDALGetDriverShortName( GDALGetDatasetDriver( ds->hDS ) ) );
}

QByteArray QgsOgrLayer::name()
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_GetName( hLayer );
}

void QgsOgrLayer::ResetReading()
{
  QMutexLocker locker( &ds->mutex );
  OGR_L_ResetReading( hLayer );
}

QByteArray QgsOgrLayer::GetFIDColumn()
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_GetFIDColumn( hLayer );
}

OGRSpatialReferenceH QgsOgrLayer::GetSpatialRef()
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_GetSpatialRef( hLayer );
}

OGRFeatureH QgsOgrLayer::GetNextFeature()
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_GetNextFeature( hLayer );
}

OGRFeatureH QgsOgrLayer::GetFeature( GIntBig fid )
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_GetFeature( hLayer, fid );
}

QgsOgrFeatureDefn &QgsOgrLayer::GetLayerDefn()
{
  return oFDefn;
}

GIntBig QgsOgrLayer::GetFeatureCount( bool force )
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_GetFeatureCount( hLayer, force );
}

GIntBig QgsOgrLayer::GetApproxFeatureCount()
{
  QMutexLocker locker( &ds->mutex );

  // OGR_L_GetFeatureCount() can be super slow on huge geopackage files
  // so implement some approximation strategy that has reasonable runtime.
  QString driverName = GDALGetDriverShortName( GDALGetDatasetDriver( ds->hDS ) );
  if ( driverName == QLatin1String( "GPKG" ) )
  {
    CPLPushErrorHandler( CPLQuietErrorHandler );
    OGRLayerH hSqlLayer = GDALDatasetExecuteSQL(
                            ds->hDS, "SELECT 1 FROM gpkg_ogr_contents LIMIT 0", nullptr, nullptr );
    CPLPopErrorHandler();
    if ( hSqlLayer )
    {
      GDALDatasetReleaseResultSet( ds->hDS, hSqlLayer );
      return OGR_L_GetFeatureCount( hLayer, TRUE );
    }

    // Enumerate features up to a limit of 100000.
    const GIntBig nLimit = CPLAtoGIntBig(
                             CPLGetConfigOption( "QGIS_GPKG_FC_THRESHOLD", "100000" ) );
    QByteArray layerName = OGR_L_GetName( hLayer );
    QByteArray sql( "SELECT COUNT(*) FROM (SELECT 1 FROM " );
    sql += QgsOgrProviderUtils::quotedIdentifier( layerName, driverName );
    sql += " LIMIT ";
    sql += CPLSPrintf( CPL_FRMT_GIB, nLimit );
    sql += ")";
    hSqlLayer = GDALDatasetExecuteSQL( ds->hDS, sql, nullptr, nullptr );
    GIntBig res = -1;
    if ( hSqlLayer )
    {
      gdal::ogr_feature_unique_ptr fet( OGR_L_GetNextFeature( hSqlLayer ) );
      if ( fet )
      {
        res = OGR_F_GetFieldAsInteger64( fet.get(), 0 );
      }
      GDALDatasetReleaseResultSet( ds->hDS, hSqlLayer );
    }
    if ( res >= 0 && res < nLimit )
    {
      // Less than 100000 features ? This is the final count
      return res;
    }
    if ( res == nLimit )
    {
      // If we reach the threshold, then use the min and max values of the rowid
      // hoping there are not a lot of holes.
      // Do it in 2 separate SQL queries otherwise SQLite apparently does a
      // full table scan...
      sql = "SELECT MAX(ROWID) FROM ";
      sql += QgsOgrProviderUtils::quotedIdentifier( layerName, driverName );
      hSqlLayer = GDALDatasetExecuteSQL( ds->hDS, sql, nullptr, nullptr );
      GIntBig maxrowid = -1;
      if ( hSqlLayer )
      {
        gdal::ogr_feature_unique_ptr fet( OGR_L_GetNextFeature( hSqlLayer ) );
        if ( fet )
        {
          maxrowid = OGR_F_GetFieldAsInteger64( fet.get(), 0 );
        }
        GDALDatasetReleaseResultSet( ds->hDS, hSqlLayer );
      }

      sql = "SELECT MIN(ROWID) FROM ";
      sql += QgsOgrProviderUtils::quotedIdentifier( layerName, driverName );
      hSqlLayer = GDALDatasetExecuteSQL( ds->hDS, sql, nullptr, nullptr );
      GIntBig minrowid = 0;
      if ( hSqlLayer )
      {
        gdal::ogr_feature_unique_ptr fet( OGR_L_GetNextFeature( hSqlLayer ) );
        if ( fet )
        {
          minrowid = OGR_F_GetFieldAsInteger64( fet.get(), 0 );
        }
        GDALDatasetReleaseResultSet( ds->hDS, hSqlLayer );
      }

      if ( maxrowid >= minrowid )
      {
        return maxrowid - minrowid + 1;
      }
    }
  }
  if ( driverName == QLatin1String( "OAPIF" ) || driverName == QLatin1String( "WFS3" ) )
  {
    return -1;
  }

  return OGR_L_GetFeatureCount( hLayer, TRUE );
}

OGRErr QgsOgrLayer::GetExtent( OGREnvelope *psExtent, bool bForce )
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_GetExtent( hLayer, psExtent, bForce );
}

OGRGeometryH QgsOgrLayer::GetSpatialFilter()
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_GetSpatialFilter( hLayer );
}

void QgsOgrLayer::SetSpatialFilter( OGRGeometryH hGeometry )
{
  QMutexLocker locker( &ds->mutex );
  OGR_L_SetSpatialFilter( hLayer, hGeometry );
}

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
GDALDatasetH QgsOgrLayer::getDatasetHandleAndMutex( QMutex *&mutex )
#else
GDALDatasetH QgsOgrLayer::getDatasetHandleAndMutex( QRecursiveMutex *&mutex )
#endif
{
  mutex = &( ds->mutex );
  return ds->hDS;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
OGRLayerH QgsOgrLayer::getHandleAndMutex( QMutex *&mutex )
#else
OGRLayerH QgsOgrLayer::getHandleAndMutex( QRecursiveMutex *&mutex )
#endif
{
  mutex = &( ds->mutex );
  return hLayer;
}

OGRErr QgsOgrLayer::CreateFeature( OGRFeatureH hFeature )
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_CreateFeature( hLayer, hFeature );
}

OGRErr QgsOgrLayer::SetFeature( OGRFeatureH hFeature )
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_SetFeature( hLayer, hFeature );
}

OGRErr QgsOgrLayer::DeleteFeature( GIntBig fid )
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_DeleteFeature( hLayer, fid );
}

OGRErr QgsOgrLayer::CreateField( OGRFieldDefnH hFieldDefn, bool bStrict )
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_CreateField( hLayer, hFieldDefn, bStrict );
}

OGRErr QgsOgrLayer::DeleteField( int iField )
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_DeleteField( hLayer, iField );
}

OGRErr QgsOgrLayer::AlterFieldDefn( int iField, OGRFieldDefnH hNewFieldDefn, int flags )
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_AlterFieldDefn( hLayer, iField, hNewFieldDefn, flags );
}

int QgsOgrLayer::TestCapability( const char *cap )
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_TestCapability( hLayer, cap );
}

OGRErr QgsOgrLayer::StartTransaction()
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_StartTransaction( hLayer );
}

OGRErr QgsOgrLayer::CommitTransaction()
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_CommitTransaction( hLayer );
}

OGRErr QgsOgrLayer::RollbackTransaction()
{
  QMutexLocker locker( &ds->mutex );
  return OGR_L_RollbackTransaction( hLayer );
}

OGRErr QgsOgrLayer::SyncToDisk()
{
  QMutexLocker locker( &ds->mutex );

  OGRErr eErr;
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0) && GDAL_VERSION_NUM <= GDAL_COMPUTE_VERSION(3,1,3)
  // Workaround bug in GDAL 3.1.0 to 3.1.3 that creates XLSX and ODS files incompatible with LibreOffice due to use of ZIP64
  QString drvName = GDALGetDriverShortName( GDALGetDatasetDriver( ds->hDS ) );
  if ( drvName == QLatin1String( "XLSX" ) ||
       drvName == QLatin1String( "ODS" ) )
  {
    CPLSetThreadLocalConfigOption( "CPL_CREATE_ZIP64", "NO" );
    eErr = OGR_L_SyncToDisk( hLayer );
    CPLSetThreadLocalConfigOption( "CPL_CREATE_ZIP64", nullptr );
  }
  else
#endif
  {
    eErr = OGR_L_SyncToDisk( hLayer );
  }

  return eErr;
}

void QgsOgrLayer::ExecuteSQLNoReturn( const QByteArray &sql )
{
  QMutexLocker locker( &ds->mutex );
  OGRLayerH hSqlLayer = GDALDatasetExecuteSQL( ds->hDS,
                        sql.constData(),
                        nullptr,
                        nullptr );
  GDALDatasetReleaseResultSet( ds->hDS, hSqlLayer );
}

QgsOgrLayerUniquePtr QgsOgrLayer::ExecuteSQL( const QByteArray &sql )
{
  QMutexLocker locker( &ds->mutex );
  OGRLayerH hSqlLayer = GDALDatasetExecuteSQL( ds->hDS,
                        sql.constData(),
                        nullptr,
                        nullptr );
  if ( !hSqlLayer )
    return nullptr;

  return QgsOgrLayer::CreateForSql( ident,
                                    QString::fromUtf8( sql ),
                                    ds,
                                    hSqlLayer );
}

QString QgsOgrLayer::GetMetadataItem( const QString &key, const QString &domain )
{
  QMutexLocker locker( &ds->mutex );
  return GDALGetMetadataItem( hLayer, key.toUtf8().constData(),
                              domain.toUtf8().constData() );
}

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
QMutex &QgsOgrFeatureDefn::mutex()
#else
QRecursiveMutex &QgsOgrFeatureDefn::mutex()
#endif
{
  return layer->mutex();
}

OGRFeatureDefnH QgsOgrFeatureDefn::get()
{
  if ( !hDefn )
  {
    QMutexLocker locker( &mutex() );
    hDefn = OGR_L_GetLayerDefn( layer->hLayer );
  }
  return hDefn;
}

int QgsOgrFeatureDefn::GetFieldCount()
{
  QMutexLocker locker( &mutex() );
  return OGR_FD_GetFieldCount( get() );
}

OGRFieldDefnH QgsOgrFeatureDefn::GetFieldDefn( int idx )
{
  QMutexLocker locker( &mutex() );
  return OGR_FD_GetFieldDefn( get(), idx );
}

int QgsOgrFeatureDefn::GetFieldIndex( const QByteArray &name )
{
  QMutexLocker locker( &mutex() );
  return OGR_FD_GetFieldIndex( get(), name.constData() );
}

int QgsOgrFeatureDefn::GetGeomFieldCount()
{
  QMutexLocker locker( &mutex() );
  return OGR_FD_GetGeomFieldCount( get() );
}

OGRGeomFieldDefnH  QgsOgrFeatureDefn::GetGeomFieldDefn( int idx )
{
  QMutexLocker locker( &mutex() );
  return OGR_FD_GetGeomFieldDefn( get(), idx );
}

OGRwkbGeometryType QgsOgrFeatureDefn::GetGeomType()
{
  QMutexLocker locker( &mutex() );
  return OGR_FD_GetGeomType( get() );
}

OGRFeatureH QgsOgrFeatureDefn::CreateFeature()
{
  QMutexLocker locker( &mutex() );
  return OGR_F_Create( get() );
}

bool QgsOgrProviderUtils::deleteLayer( const QString &uri, QString &errCause )
{
  bool isSubLayer;
  int layerIndex;
  QString layerName;
  QString subsetString;
  OGRwkbGeometryType ogrGeometryType;
  QStringList openOptions;
  QString filePath = analyzeURI( uri,
                                 isSubLayer,
                                 layerIndex,
                                 layerName,
                                 subsetString,
                                 ogrGeometryType,
                                 openOptions );


  gdal::dataset_unique_ptr hDS( GDALOpenEx( filePath.toUtf8().constData(), GDAL_OF_RASTER | GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
  if ( hDS  && ( ! layerName.isEmpty() || layerIndex != -1 ) )
  {
    // If we have got a name we convert it into an index
    if ( ! layerName.isEmpty() )
    {
      layerIndex = -1;
      for ( int i = 0; i < GDALDatasetGetLayerCount( hDS.get() ); i++ )
      {
        OGRLayerH hL = GDALDatasetGetLayer( hDS.get(), i );
        if ( layerName == QString::fromUtf8( OGR_L_GetName( hL ) ) )
        {
          layerIndex = i;
          break;
        }
      }
    }
    // Do delete!
    if ( layerIndex != -1 )
    {
      OGRErr error = GDALDatasetDeleteLayer( hDS.get(), layerIndex );
      switch ( error )
      {
        case OGRERR_NOT_ENOUGH_DATA:
          errCause = QObject::tr( "Not enough data to deserialize" );
          break;
        case OGRERR_NOT_ENOUGH_MEMORY:
          errCause = QObject::tr( "Not enough memory" );
          break;
        case OGRERR_UNSUPPORTED_GEOMETRY_TYPE:
          errCause = QObject::tr( "Unsupported geometry type" );
          break;
        case OGRERR_UNSUPPORTED_OPERATION:
          errCause = QObject::tr( "Unsupported operation" );
          break;
        case OGRERR_CORRUPT_DATA:
          errCause = QObject::tr( "Corrupt data" );
          break;
        case OGRERR_FAILURE:
          errCause = QObject::tr( "Failure" );
          break;
        case OGRERR_UNSUPPORTED_SRS:
          errCause = QObject::tr( "Unsupported SRS" );
          break;
        case OGRERR_INVALID_HANDLE:
          errCause = QObject::tr( "Invalid handle" );
          break;
        case OGRERR_NON_EXISTING_FEATURE:
          errCause = QObject::tr( "Non existing feature" );
          break;

        case OGRERR_NONE:
        {
          errCause = QObject::tr( "Success" );
          break;
        }
      }
      errCause = QObject::tr( "GDAL result code: %1" ).arg( errCause );
      return error == OGRERR_NONE;
    }
  }
  // This should never happen:
  errCause = QObject::tr( "Layer not found: %1" ).arg( uri );
  return false;
}

void QgsOgrLayerReleaser::operator()( QgsOgrLayer *layer )
{
  QgsOgrProviderUtils::release( layer );
}

///@endcond
