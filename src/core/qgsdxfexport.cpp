/***************************************************************************
                         qgsdxfexport.cpp
                         ----------------
    begin                : September 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdxfexport.h"
#include "qgsvectordataprovider.h"
#include "qgspoint.h"
#include "qgsrendererv2.h"
#include "qgsvectorlayer.h"
#include <QIODevice>
#include <QTextStream>

QgsDxfExport::QgsDxfExport()
{
}

QgsDxfExport::~QgsDxfExport()
{

}

int QgsDxfExport::writeToFile( QIODevice* d, SymbologyExport s )
{
  if ( !d )
  {
    return 1;
  }

  if ( !d->open( QIODevice::WriteOnly ) )
  {
    return 2;
  }

  QTextStream outStream( d );
  writeHeader( outStream );
  writeTables( outStream );
  writeEntities( outStream );
  writeEndFile( outStream );
  return 0;
}

void QgsDxfExport::writeHeader( QTextStream& stream )
{
  stream << "999\n";
  stream << "DXF created from QGIS\n";
  startSection( stream );
  stream << "  2\n";
  stream << "HEADER\n";
  stream << "  9\n";
  stream << "$ACADVER";
  stream << "AC1009\n";
  stream << "  9\n";
  stream << "$LTSCALE\n";
  stream << " 40\n";
  stream << "1\n";
  endSection( stream );
}

void QgsDxfExport::writeTables( QTextStream& stream )
{
  startSection( stream );
  stream << "  2\n";
  stream << "TABLES\n";

  //
  stream << "  0\n";
  stream << "TABLE\n";
  stream << "  0\n";
  stream << "LAYER\n";
  QList< QgsMapLayer* >::iterator layerIt = mLayers.begin();
  for ( ; layerIt != mLayers.end(); ++layerIt )
  {
    stream << "  0\n";
    stream << "LAYER\n";
    stream << "  2\n";
    if ( *layerIt )
    {
      stream << ( *layerIt )->name() << "\n";
    }
    stream << " 70\n"; //layer property
    stream << "64\n";
    stream << " 62\n"; //layer color
    stream << "1\n";
    stream << "  6\n"; //layer line type
    stream << "CONTINUOUS\n";
  }
  stream << "  0\n";
  stream << "ENDTAB\n";

  endSection( stream );
}

void QgsDxfExport::writeEntities( QTextStream& stream )
{
  startSection( stream );
  stream << "  2\n";
  stream << "ENTITIES\n";

  //iterate through the maplayers
  QList< QgsMapLayer* >::iterator layerIt = mLayers.begin();
  for ( ; layerIt != mLayers.end(); ++layerIt )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( *layerIt );
    if ( !vl )
    {
      continue;
    }
    QgsVectorDataProvider* dp = vl->dataProvider();
    if ( !dp )
    {
      continue;
    }

    QgsFeatureRendererV2* renderer = vl->rendererV2();
    QgsFeatureIterator featureIt = vl->getFeatures( QgsFeatureRequest().setSubsetOfAttributes(
                                     renderer->usedAttributes(), dp->fields() ) );
    QgsFeature fet;
    while ( featureIt.nextFeature( fet ) )
    {
      //get geometry and write it. Todo: consider symbolisation
      QgsGeometry* geom = fet.geometry();
      if ( geom )
      {
        //try with line first
        writePolyline( stream, geom->asPolyline(), vl->name() );
      }
    }
  }

  endSection( stream );
}

void QgsDxfExport::writeEndFile( QTextStream& stream )
{
  endSection( stream );
}

void QgsDxfExport::startSection( QTextStream& stream )
{
  stream << "  0\n";
  stream << "SECTION\n";
}

void QgsDxfExport::endSection( QTextStream& stream )
{
  stream << "  0\n";
  stream << "ENDSEC\n";
}

void QgsDxfExport::writePolyline( QTextStream& stream, const QgsPolyline& line, const QString& layer, bool closed )
{
  stream << "  0\n";
  stream << "POLYLINE\n";
  stream << "  8\n";
  stream << layer << "\n";
  stream << " 66\n";
  stream << "  1\n";
  stream << " 70\n";
  int type = closed ? 32 : 0;
  stream << type << "\n";

  QgsPolyline::const_iterator lineIt = line.constBegin();
  for ( ; lineIt != line.constEnd(); ++lineIt )
  {
    writeVertex( stream, *lineIt, layer );
  }
}

void QgsDxfExport::writeVertex( QTextStream& stream, const QgsPoint& pt, const QString& layer )
{
  stream << "  0\n";
  stream << "VERTEX\n";
  stream << "  8\n";
  stream << layer << "\n";
  stream << " 10\n";
  stream << pt.x() << "\n";
  stream << " 20\n";
  stream << pt.y() << "\n";
  stream << " 30\n";
  stream << "0.0\n";
}
