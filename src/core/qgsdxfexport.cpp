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
  //ACADVER
  stream << "  9\n";
  stream << "$ACADVER\n";
  stream << "  1\n";
  stream << "AC1009\n";

  QgsRectangle ext = dxfExtent();
  if ( !ext.isEmpty() )
  {
    //EXTMIN
    stream << "  9\n";
    stream << "$EXTMIN\n";
    stream << " 10\n";
    stream << ext.xMinimum() << "\n";
    stream << " 20\n";
    stream << ext.yMinimum() << "\n";
    stream << " 30\n";
    stream << "0\n";
    //EXTMAX
    stream << "  9\n";
    stream << "$EXTMAX\n";
    stream << " 10\n";
    stream << ext.xMaximum() << "\n";
    stream << " 20\n";
    stream << ext.yMaximum() << "\n";
    stream << " 30\n";
    stream << "0\n";
  }
  //LTSCALE
  stream << "  9\n";
  stream << "$LTSCALE\n";
  stream << " 40\n";
  stream << "1.0\n";
  //PDMODE
  stream << "  9\n";
  stream << "$PDMODE\n";
  stream << " 70\n";
  stream << "33\n";
  //PDSIZE
  stream << "  9\n";
  stream << "$PDSIZE\n";
  stream << " 40\n";
  stream << "1\n";
  endSection( stream );
  //PSLTSCALE
  stream << "  9\n";
  stream << "$PSLTSCALE\n";
  stream << " 70\n";
  stream << "0\n";
}

void QgsDxfExport::writeTables( QTextStream& stream )
{
  startSection( stream );
  stream << "  2\n";
  stream << "TABLES\n";

  //LTYPE
  stream << "  0\n";
  stream << "TABLE\n";
  stream << "  0\n";
  stream << "LTYPE\n";
  stream << "  2\n";
  stream << "CONTINUOUS\n";
  stream << "  70\n";
  stream << "64\n";
  stream << "  3\n";
  stream << "Defaultstyle\n";
  stream << " 72\n";
  stream << "65\n";
  stream << " 73\n";
  stream << "0\n";
  stream << " 40\n"; //todo: add segments in group 49
  stream << "0\n";
  stream << "  0\n";
  stream << "ENDTAB\n";

  //LAYER
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
      if ( mSymbologyExport == NoSymbology )
      {
        addFeature( fet, stream, vl->name(), 0 ); //no symbology at all
      }
      else
      {
        if ( !renderer )
        {
          continue;
        }
        QgsSymbolV2List symbolList = renderer->symbolsForFeature( fet );
        if ( symbolList.size() < 1 )
        {
          continue;
        }

        //take first symbollayer from first symbol
        QgsSymbolV2* s = symbolList.first();
        if ( !s || s->symbolLayerCount() < 1 )
        {
          continue;
        }
        addFeature( fet, stream, vl->name(), s->symbolLayer( 0 ) );
      }
#if 0
      //get geometry and write it. Todo: consider symbolisation
      QgsGeometry* geom = fet.geometry();
      if ( geom )
      {
        //try with line first
        writePolyline( stream, geom->asPolyline(), vl->name() );
      }
#endif //0
    }
  }

  endSection( stream );
}

void QgsDxfExport::writeEntitiesSymbolLevels( QTextStream& stream )
{
  //todo....
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

void QgsDxfExport::writePolyline( QTextStream& stream, const QgsPolyline& line, const QString& layer, int color,
                                  double width, bool closed )
{
  stream << "  0\n";
  stream << "POLYLINE\n";
  stream << "  8\n";
  stream << layer << "\n";
  stream << "  6\n";
  stream << "CONTINUOUS\n"; //todo: reference to linetype here
  stream << " 62\n";
  stream << color << "\n";
  stream << " 66\n";
  stream << "1\n";
  stream << " 70\n";
  int type = closed ? 32 : 0;
  stream << type << "\n";
  stream << " 40\n";
  stream << width << "\n";
  stream << " 41\n";
  stream << width << "\n";

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

QgsRectangle QgsDxfExport::dxfExtent() const
{
  QgsRectangle extent;
  QList< QgsMapLayer* >::const_iterator layerIt = mLayers.constBegin();
  for ( ; layerIt != mLayers.constEnd(); ++layerIt )
  {
    if ( *layerIt )
    {
      if ( extent.isEmpty() )
      {
        extent = ( *layerIt )->extent();
      }
      else
      {
        QgsRectangle layerExtent = ( *layerIt )->extent();
        extent.combineExtentWith( &layerExtent );
      }
    }
  }
  return extent;
}

void QgsDxfExport::addFeature( const QgsFeature& fet, QTextStream& stream, const QString& layer, const QgsSymbolLayerV2* symbolLayer )
{
  QgsGeometry* geom = fet.geometry();
  if ( geom )
  {
    //get color from symbollayer
    int c = colorFromSymbolLayer( symbolLayer );
    //get width from symbollayer
    double width = widthFromSymbolLayer( symbolLayer );
    writePolyline( stream, geom->asPolyline(), layer, c, width );
  }
}

double QgsDxfExport::scaleToMapUnits( double value, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits ) const
{
  if ( symbolUnits == QgsSymbolV2::MapUnit )
  {
    return 1.0;
  }

  //symbology in mm
  value *= mSymbologyScaleDenominator / 1000;
  if ( mapUnits == QGis::Feet )
  {
    value *= 0.3048;
  }
  else if ( mapUnits == QGis::Degrees )
  {
    value /= 111120;
  }
  return value;
}

int QgsDxfExport::colorFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer )
{
  return 5; //todo...
}

double QgsDxfExport::widthFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer )
{
  return 50; //todo...
}
