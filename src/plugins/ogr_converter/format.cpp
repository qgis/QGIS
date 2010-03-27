// $Id$
//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 by Mateusz Loskot <mateusz@loskot.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
//////////////////////////////////////////////////////////////////////////////

// qgis::plugin::ogrconv
#include "format.h"
// Qt4
#include <QString>

Format::Format()
{
}

Format::Format( QString const& c, QString const& n )
    : mCode( c ), mName( n ), mTypeFlags( 0 )
{
}

Format::Format( QString const& c, QString const& n, unsigned char const& t )
    : mCode( c ), mName( n ), mTypeFlags( t )
{
}

Format::Format( QString const& c, QString const& n, QString const& p, unsigned char const& t )
    : mCode( c ), mName( n ), mProtocol( p ), mTypeFlags( t )
{
}

QString const& Format::code() const
{
  return mCode;
}

QString const& Format::name() const
{
  return mName;
}

QString const& Format::protocol() const
{
  return mProtocol;
}

unsigned char const& Format::type() const
{
  return mTypeFlags;
}


FormatsRegistry::FormatsRegistry()
{
  init();
}

void FormatsRegistry::add( Format const& frmt )
{
  QString code = frmt.code();
  mFrmts[code] = frmt;
}

Format const& FormatsRegistry::find( QString const& code )
{
  mCache = mFrmts.value( code );
  return mCache;
}

void FormatsRegistry::init()
{
  add( Format( "AVCBin", "Arc/Info Binary Coverage", Format::eFile ) );
  add( Format( "AVCE00", "Arc/Info .E00 (ASCII) Coverage", Format::eFile ) );
  add( Format( "BNA", "Atlas BNA", Format::eFile ) );
  add( Format( "CSV", "Comma Separated Value", Format::eFile | Format::eDirectory ) );
  add( Format( "DODS", "DODS/OPeNDAP", Format::eFile ) );
  add( Format( "DGN", "Microstation DGN", Format::eFile ) );
  add( Format( "ESRI Shapefile", "ESRI Shapefile", Format::eFile | Format::eDirectory ) );
  add( Format( "FMEObjects Gateway", "FMEObjects Gateway", Format::eFile ) );
  add( Format( "Geoconcept Text Export", "Geoconcept", Format::eFile ) );
  add( Format( "GML", "Geography Markup Language", Format::eFile ) );
  add( Format( "GMT", "GMT ASCII Vectors", Format::eFile ) );
  add( Format( "GPX", "GPS Exchange Format", Format::eFile ) );
  add( Format( "GeoJSON", "GeoJSON", Format::eFile ) ); // FIXME: Format::eProtocol));
  add( Format( "GRASS", "GRASS", Format::eDirectory ) );
  add( Format( "Informix DataBlade", "IDB", "IDB:", Format::eProtocol ) );
  add( Format( "Interlis 1", "INTERLIS", Format::eFile ) );
  add( Format( "Interlis 2", "INTERLIS", Format::eFile ) );
  add( Format( "Ingres Database", "INGRES", "@driver=ingres,", Format::eProtocol ) );
  add( Format( "KML", "KML", Format::eFile ) );
  add( Format( "MapInfo", "MapInfo File", Format::eFile ) );
  add( Format( "Memory", "Memory", Format::eFile ) );
  add( Format( "MySQL", "MySQL", "MySQL:", Format::eProtocol ) );
  add( Format( "ODBC", "Open DataBase Connectivity", "ODBC:", Format::eProtocol ) );
  add( Format( "OGDI", "Open Geographic Datastore Interface Vectors", "gltp:", Format::eProtocol ) );
  add( Format( "OCI", "Oracle Spatial", "OCI:", Format::eProtocol ) );
  add( Format( "PGeo", "ESRI Personal GeoDatabase", "PGeo:", Format::eFile | Format::eProtocol ) );
  add( Format( "PostgreSQL", "PostgreSQL", "PG:", Format::eProtocol ) );
  add( Format( "S57", "IHO S-57", Format::eFile ) );
  add( Format( "SDE", "ESRI ArcSDE", "SDE:", Format::eProtocol ) );
  add( Format( "SDTS", "SDTS Topological Vector Profile", Format::eFile ) );
  add( Format( "SQLite", "SQLite Database File", Format::eFile ) );
  add( Format( "UK.NTF", "UK National Transfer Format", Format::eFile ) );
  add( Format( "TIGER", "U.S. Census TIGER/Line", Format::eFile ) );
  add( Format( "VRT", "Virtual Datasource", Format::eFile ) );
  add( Format( "XPLANE", "X-Plane/Flightgear Aeronautical Data", Format::eFile ) );
}
