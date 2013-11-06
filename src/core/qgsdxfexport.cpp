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
#include "qgssymbollayerv2.h"
#include "qgsvectorlayer.h"
#include <QIODevice>
#include <QTextStream>

//dxf color palette
double QgsDxfExport::mDxfColors[][3] =
{
  {0, 0, 0},              // unused
  {1, 0, 0},              // 1
  {1, 1, 0},
  {0, 1, 0},
  {0, 1, 1},
  {0, 0, 1},
  {1, 0, 1},
  {1, 1, 1},              // black or white
  {0.5, 0.5, 0.5},
  {0.75, 0.75, 0.75},
  {1, 0, 0},              // 10
  {1, 0.5, 0.5},
  {0.65, 0, 0},
  {0.65, 0.325, 0.325},
  {0.5, 0, 0},
  {0.5, 0.25, 0.25},
  {0.3, 0, 0},
  {0.3, 0.15, 0.15},
  {0.15, 0, 0},
  {0.15, 0.075, 0.075},
  {1, 0.25, 0},           // 20
  {1, 0.625, 0.5},
  {0.65, 0.1625, 0},
  {0.65, 0.4063, 0.325},
  {0.5, 0.125, 0},
  {0.5, 0.3125, 0.25},
  {0.3, 0.075, 0},
  {0.3, 0.1875, 0.15},
  {0.15, 0.0375, 0},
  {0.15, 0.0938, 0.075},
  {1, 0.5, 0},            // 30
  {1, 0.75, 0.5},
  {0.65, 0.325, 0},
  {0.65, 0.4875, 0.325},
  {0.5, 0.25, 0},
  {0.5, 0.375, 0.25},
  {0.3, 0.15, 0},
  {0.3, 0.225, 0.15},
  {0.15, 0.075, 0},
  {0.15, 0.1125, 0.075},
  {1, 0.75, 0},           // 40
  {1, 0.875, 0.5},
  {0.65, 0.4875, 0},
  {0.65, 0.5688, 0.325},
  {0.5, 0.375, 0},
  {0.5, 0.4375, 0.25},
  {0.3, 0.225, 0},
  {0.3, 0.2625, 0.15},
  {0.15, 0.1125, 0},
  {0.15, 0.1313, 0.075},
  {1, 1, 0},              // 50
  {1, 1, 0.5},
  {0.65, 0.65, 0},
  {0.65, 0.65, 0.325},
  {0.5, 0.5, 0},
  {0.5, 0.5, 0.25},
  {0.3, 0.3, 0},
  {0.3, 0.3, 0.15},
  {0.15, 0.15, 0},
  {0.15, 0.15, 0.075},
  {0.75, 1, 0},           // 60
  {0.875, 1, 0.5},
  {0.4875, 0.65, 0},
  {0.5688, 0.65, 0.325},
  {0.375, 0.5, 0},
  {0.4375, 0.5, 0.25},
  {0.225, 0.3, 0},
  {0.2625, 0.3, 0.15},
  {0.1125, 0.15, 0},
  {0.1313, 0.15, 0.075},
  {0.5, 1, 0},            // 70
  {0.75, 1, 0.5},
  {0.325, 0.65, 0},
  {0.4875, 0.65, 0.325},
  {0.25, 0.5, 0},
  {0.375, 0.5, 0.25},
  {0.15, 0.3, 0},
  {0.225, 0.3, 0.15},
  {0.075, 0.15, 0},
  {0.1125, 0.15, 0.075},
  {0.25, 1, 0},           // 80
  {0.625, 1, 0.5},
  {0.1625, 0.65, 0},
  {0.4063, 0.65, 0.325},
  {0.125, 0.5, 0},
  {0.3125, 0.5, 0.25},
  {0.075, 0.3, 0},
  {0.1875, 0.3, 0.15},
  {0.0375, 0.15, 0},
  {0.0938, 0.15, 0.075},
  {0, 1, 0},              // 90
  {0.5, 1, 0.5},
  {0, 0.65, 0},
  {0.325, 0.65, 0.325},
  {0, 0.5, 0},
  {0.25, 0.5, 0.25},
  {0, 0.3, 0},
  {0.15, 0.3, 0.15},
  {0, 0.15, 0},
  {0.075, 0.15, 0.075},
  {0, 1, 0.25},           // 100
  {0.5, 1, 0.625},
  {0, 0.65, 0.1625},
  {0.325, 0.65, 0.4063},
  {0, 0.5, 0.125},
  {0.25, 0.5, 0.3125},
  {0, 0.3, 0.075},
  {0.15, 0.3, 0.1875},
  {0, 0.15, 0.0375},
  {0.075, 0.15, 0.0938},
  {0, 1, 0.5},            // 110
  {0.5, 1, 0.75},
  {0, 0.65, 0.325},
  {0.325, 0.65, 0.4875},
  {0, 0.5, 0.25},
  {0.25, 0.5, 0.375},
  {0, 0.3, 0.15},
  {0.15, 0.3, 0.225},
  {0, 0.15, 0.075},
  {0.075, 0.15, 0.1125},
  {0, 1, 0.75},           // 120
  {0.5, 1, 0.875},
  {0, 0.65, 0.4875},
  {0.325, 0.65, 0.5688},
  {0, 0.5, 0.375},
  {0.25, 0.5, 0.4375},
  {0, 0.3, 0.225},
  {0.15, 0.3, 0.2625},
  {0, 0.15, 0.1125},
  {0.075, 0.15, 0.1313},
  {0, 1, 1},              // 130
  {0.5, 1, 1},
  {0, 0.65, 0.65},
  {0.325, 0.65, 0.65},
  {0, 0.5, 0.5},
  {0.25, 0.5, 0.5},
  {0, 0.3, 0.3},
  {0.15, 0.3, 0.3},
  {0, 0.15, 0.15},
  {0.075, 0.15, 0.15},
  {0, 0.75, 1},           // 140
  {0.5, 0.875, 1},
  {0, 0.4875, 0.65},
  {0.325, 0.5688, 0.65},
  {0, 0.375, 0.5},
  {0.25, 0.4375, 0.5},
  {0, 0.225, 0.3},
  {0.15, 0.2625, 0.3},
  {0, 0.1125, 0.15},
  {0.075, 0.1313, 0.15},
  {0, 0.5, 1},            // 150
  {0.5, 0.75, 1},
  {0, 0.325, 0.65},
  {0.325, 0.4875, 0.65},
  {0, 0.25, 0.5},
  {0.25, 0.375, 0.5},
  {0, 0.15, 0.3},
  {0.15, 0.225, 0.3},
  {0, 0.075, 0.15},
  {0.075, 0.1125, 0.15},
  {0, 0.25, 1},           // 160
  {0.5, 0.625, 1},
  {0, 0.1625, 0.65},
  {0.325, 0.4063, 0.65},
  {0, 0.125, 0.5},
  {0.25, 0.3125, 0.5},
  {0, 0.075, 0.3},
  {0.15, 0.1875, 0.3},
  {0, 0.0375, 0.15},
  {0.075, 0.0938, 0.15},
  {0, 0, 1},              // 170
  {0.5, 0.5, 1},
  {0, 0, 0.65},
  {0.325, 0.325, 0.65},
  {0, 0, 0.5},
  {0.25, 0.25, 0.5},
  {0, 0, 0.3},
  {0.15, 0.15, 0.3},
  {0, 0, 0.15},
  {0.075, 0.075, 0.15},
  {0.25, 0, 1},           // 180
  {0.625, 0.5, 1},
  {0.1625, 0, 0.65},
  {0.4063, 0.325, 0.65},
  {0.125, 0, 0.5},
  {0.3125, 0.25, 0.5},
  {0.075, 0, 0.3},
  {0.1875, 0.15, 0.3},
  {0.0375, 0, 0.15},
  {0.0938, 0.075, 0.15},
  {0.5, 0, 1},            // 190
  {0.75, 0.5, 1},
  {0.325, 0, 0.65},
  {0.4875, 0.325, 0.65},
  {0.25, 0, 0.5},
  {0.375, 0.25, 0.5},
  {0.15, 0, 0.3},
  {0.225, 0.15, 0.3},
  {0.075, 0, 0.15},
  {0.1125, 0.075, 0.15},
  {0.75, 0, 1},           // 200
  {0.875, 0.5, 1},
  {0.4875, 0, 0.65},
  {0.5688, 0.325, 0.65},
  {0.375, 0, 0.5},
  {0.4375, 0.25, 0.5},
  {0.225, 0, 0.3},
  {0.2625, 0.15, 0.3},
  {0.1125, 0, 0.15},
  {0.1313, 0.075, 0.15},
  {1, 0, 1},              // 210
  {1, 0.5, 1},
  {0.65, 0, 0.65},
  {0.65, 0.325, 0.65},
  {0.5, 0, 0.5},
  {0.5, 0.25, 0.5},
  {0.3, 0, 0.3},
  {0.3, 0.15, 0.3},
  {0.15, 0, 0.15},
  {0.15, 0.075, 0.15},
  {1, 0, 0.75},           // 220
  {1, 0.5, 0.875},
  {0.65, 0, 0.4875},
  {0.65, 0.325, 0.5688},
  {0.5, 0, 0.375},
  {0.5, 0.25, 0.4375},
  {0.3, 0, 0.225},
  {0.3, 0.15, 0.2625},
  {0.15, 0, 0.1125},
  {0.15, 0.075, 0.1313},
  {1, 0, 0.5},            // 230
  {1, 0.5, 0.75},
  {0.65, 0, 0.325},
  {0.65, 0.325, 0.4875},
  {0.5, 0, 0.25},
  {0.5, 0.25, 0.375},
  {0.3, 0, 0.15},
  {0.3, 0.15, 0.225},
  {0.15, 0, 0.075},
  {0.15, 0.075, 0.1125},
  {1, 0, 0.25},           // 240
  {1, 0.5, 0.625},
  {0.65, 0, 0.1625},
  {0.65, 0.325, 0.4063},
  {0.5, 0, 0.125},
  {0.5, 0.25, 0.3125},
  {0.3, 0, 0.075},
  {0.3, 0.15, 0.1875},
  {0.15, 0, 0.0375},
  {0.15, 0.075, 0.0938},
  {0.33, 0.33, 0.33},     // 250
  {0.464, 0.464, 0.464},
  {0.598, 0.598, 0.598},
  {0.732, 0.732, 0.732},
  {0.866, 0.866, 0.866},
  {1, 1, 1}               // 255
};

QgsDxfExport::QgsDxfExport(): mSymbologyScaleDenominator( 1.0 ), mSymbologyExport( NoSymbology ), mMapUnits( QGis::Meters )
{
}

QgsDxfExport::~QgsDxfExport()
{

}

int QgsDxfExport::writeToFile( QIODevice* d )
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
  stream << "  2\n";
  stream << "LTYPE\n";
  stream << " 70\n";
  stream << "  1\n"; //number of linetypes
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
  stream << "  2\n";
  stream << "LAYER\n";
  stream << " 70\n";
  stream << mLayers.count() << "\n";
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

    QgsFeatureRendererV2* renderer = vl->rendererV2();
    if ( mSymbologyExport == QgsDxfExport::SymbolLayerSymbology && renderer->usingSymbolLevels() )
    {
      writeEntitiesSymbolLevels( stream, vl );
      continue;
    }

    QgsVectorDataProvider* dp = vl->dataProvider();
    if ( !dp )
    {
      continue;
    }


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
    }
  }

  endSection( stream );
}

void QgsDxfExport::writeEntitiesSymbolLevels( QTextStream& stream, QgsVectorLayer* layer )
{
  if ( !layer )
  {
    return;
  }

  QgsFeatureRendererV2* renderer = layer->rendererV2();
  if ( !renderer )
  {
    //return error
  }
  QHash< QgsSymbolV2*, QList<QgsFeature> > features;

  startRender( layer );

  //get iterator
  QgsFeatureRequest req;
  if ( layer->wkbType() == QGis::WKBNoGeometry )
  {
    req.setFlags( QgsFeatureRequest::NoGeometry );
  }
  req.setSubsetOfAttributes( QStringList( renderer->usedAttributes() ), layer->pendingFields() );
  QgsFeatureIterator fit = layer->getFeatures( req );

  //fetch features
  QgsFeature fet;
  QgsSymbolV2* featureSymbol = 0;
  while ( fit.nextFeature( fet ) )
  {
    featureSymbol = renderer->symbolForFeature( fet );
    if ( !featureSymbol )
    {
      continue;
    }

    QHash< QgsSymbolV2*, QList<QgsFeature> >::iterator it = features.find( featureSymbol );
    if ( it == features.end() )
    {
      it = features.insert( featureSymbol, QList<QgsFeature>() );
    }
    it.value().append( fet );
  }

  //find out order
  QgsSymbolV2LevelOrder levels;
  QgsSymbolV2List symbols = renderer->symbols();
  for ( int i = 0; i < symbols.count(); i++ )
  {
    QgsSymbolV2* sym = symbols[i];
    for ( int j = 0; j < sym->symbolLayerCount(); j++ )
    {
      int level = sym->symbolLayer( j )->renderingPass();
      if ( level < 0 || level >= 1000 ) // ignore invalid levels
        continue;
      QgsSymbolV2LevelItem item( sym, j );
      while ( level >= levels.count() ) // append new empty levels
        levels.append( QgsSymbolV2Level() );
      levels[level].append( item );
    }
  }

  //export symbol layers and symbology
  for ( int l = 0; l < levels.count(); l++ )
  {
    QgsSymbolV2Level& level = levels[l];
    for ( int i = 0; i < level.count(); i++ )
    {
      QgsSymbolV2LevelItem& item = level[i];
      QHash< QgsSymbolV2*, QList<QgsFeature> >::iterator levelIt = features.find( item.symbol() );

      int llayer = item.layer();
      QList<QgsFeature>& featureList = levelIt.value();
      QList<QgsFeature>::iterator featureIt = featureList.begin();
      for ( ; featureIt != featureList.end(); ++featureIt )
      {
        addFeature( *featureIt, stream, layer->name(), levelIt.key()->symbolLayer( llayer ) );
      }
    }
  }
  stopRender( layer );
}

void QgsDxfExport::writeEndFile( QTextStream& stream )
{
  stream << "  0\n";
  stream << "EOF\n";
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
                                  double width, bool polygon )
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
  int type = polygon ? 49 : 0;
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
  stream << "  0\n";
  stream << "SEQEND\n";
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
    int c = colorFromSymbolLayer( symbolLayer );
    double width = widthFromSymbolLayer( symbolLayer );

    //todo: write point symbols as blocks

    QGis::WkbType geometryType = geom->wkbType();
    //single line
    if ( geometryType == QGis::WKBLineString || geometryType == QGis::WKBLineString25D )
    {
      writePolyline( stream, geom->asPolyline(), layer, c, width, false );
    }

    //multiline
    if ( geometryType == QGis::WKBMultiLineString || geometryType == QGis::WKBMultiLineString25D )
    {
      QgsMultiPolyline multiLine = geom->asMultiPolyline();
      QgsMultiPolyline::const_iterator lIt = multiLine.constBegin();
      for ( ; lIt != multiLine.constEnd(); ++lIt )
      {
        writePolyline( stream, *lIt, layer, c, width, false );
      }
    }

    //polygon
    if ( geometryType == QGis::WKBPolygon || geometryType == QGis::WKBPolygon25D )
    {
      QgsPolygon polygon = geom->asPolygon();
      QgsPolygon::const_iterator polyIt = polygon.constBegin();
      for ( ; polyIt != polygon.constEnd(); ++polyIt ) //iterate over rings
      {
        writePolyline( stream, *polyIt, layer, c, width, true );
      }
    }

    //multipolygon or polygon
    if ( geometryType == QGis::WKBMultiPolygon || geometryType == QGis::WKBMultiPolygon25D )
    {
      QgsMultiPolygon mp = geom->asMultiPolygon();
      QgsMultiPolygon::const_iterator mpIt = mp.constBegin();
      for ( ; mpIt != mp.constEnd(); ++mpIt )
      {
        QgsPolygon::const_iterator polyIt = mpIt->constBegin();
        for ( ; polyIt != mpIt->constEnd(); ++polyIt )
        {
          writePolyline( stream, *polyIt, layer, c, width, true );
        }
      }
    }
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
  if ( !symbolLayer )
  {
    return 0;
  }

  QColor c = symbolLayer->color();
  return closestColorMatch( c.rgba() );
}

double QgsDxfExport::widthFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer )
{
  //line symbol layer has width and width units
  if ( symbolLayer && symbolLayer->type() == QgsSymbolV2::Line )
  {
    const QgsLineSymbolLayerV2* lineSymbolLayer = static_cast<const QgsLineSymbolLayerV2*>( symbolLayer );
    return ( lineSymbolLayer->width() * mapUnitScaleFactor( mSymbologyScaleDenominator, lineSymbolLayer->widthUnit(), mMapUnits ) );
  }

  return 1.0;

  //marker symbol layer: check for embedded line layers?

  //mapUnitScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits )
}

int QgsDxfExport::closestColorMatch( QRgb pixel )
{
  int idx = 0;
  int current_distance = INT_MAX;
  for ( int i = 1; i < 256; ++i )
  {
    int dist = color_distance( pixel, i );
    if ( dist < current_distance )
    {
      current_distance = dist;
      idx = i;
    }
  }
  return idx;
}

int QgsDxfExport::color_distance( QRgb p1, int index )
{
  if ( index > 255 || index < 0 )
  {
    return 0;
  }

  double redDiff = qRed( p1 ) - mDxfColors[index][0] * 255;
  double greenDiff = qGreen( p1 ) - mDxfColors[index][1] * 255;
  double blueDiff = qBlue( p1 ) - mDxfColors[index][2] * 255;
  return ( redDiff * redDiff + greenDiff * greenDiff + blueDiff * blueDiff );
}

QRgb QgsDxfExport::createRgbEntry( qreal r, qreal g, qreal b )
{
  return QColor::fromRgbF( r, g, b ).rgb();
}

QgsRenderContext QgsDxfExport::renderContext() const
{
  QgsRenderContext context;
  context.setRendererScale( mSymbologyScaleDenominator );
  return context;
}

void QgsDxfExport::startRender( QgsVectorLayer* vl ) const
{
  if ( !vl )
  {
    return;
  }

  QgsFeatureRendererV2* renderer = vl->rendererV2();
  if ( !renderer )
  {
    return;
  }

  QgsRenderContext ctx = renderContext();
  renderer->startRender( ctx, vl );
}

void QgsDxfExport::stopRender( QgsVectorLayer* vl ) const
{
  if ( !vl )
  {
    return;
  }

  QgsFeatureRendererV2* renderer = vl->rendererV2();
  if ( !renderer )
  {
    return;
  }

  QgsRenderContext ctx = renderContext();
  renderer->stopRender( ctx );
}

double QgsDxfExport::mapUnitScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits )
{
  if ( symbolUnits == QgsSymbolV2::MapUnit )
  {
    return 1.0;
  }
  else
  {
    if ( symbolUnits == QgsSymbolV2::MM && mapUnits == QGis::Meters )
    {
      return scaleDenominator / 1000;
    }
  }
  return 1.0;
}
