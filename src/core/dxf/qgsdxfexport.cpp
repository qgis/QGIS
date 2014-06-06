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
#include "qgsdxfpallabeling.h"
#include "qgsvectordataprovider.h"
#include "qgspoint.h"
#include "qgsrendererv2.h"
#include "qgssymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgslinesymbollayerv2.h"
#include "qgsvectorlayer.h"
#include <QIODevice>

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

QgsDxfExport::QgsDxfExport(): mSymbologyScaleDenominator( 1.0 ), mSymbologyExport( NoSymbology ), mMapUnits( QGis::Meters ), mSymbolLayerCounter( 0 ),
    mNextHandleId( 10 ), mBlockCounter( 0 )
{
}

QgsDxfExport::QgsDxfExport( const QgsDxfExport& dxfExport )
{
  *this = dxfExport;
}

QgsDxfExport& QgsDxfExport::operator=( const QgsDxfExport & dxfExport )
{
  mLayers = dxfExport.mLayers;
  mSymbologyScaleDenominator = dxfExport.mSymbologyScaleDenominator;
  mSymbologyExport = dxfExport.mSymbologyExport;
  mMapUnits = dxfExport.mMapUnits;
  mSymbolLayerCounter = 0; //internal counter
  mNextHandleId = 0;
  mBlockCounter = 0;
  return *this;
}

QgsDxfExport::~QgsDxfExport()
{
}

void QgsDxfExport::writeGroup( int code, int i )
{
  writeGroupCode( code );
  writeInt( i );
}

void QgsDxfExport::writeGroup( int code, double d )
{
  writeGroupCode( code );
  writeDouble( d );
}

void QgsDxfExport::writeGroup( int code, const QString& s )
{
  writeGroupCode( code );
  writeString( s );
}

void QgsDxfExport::writeGroupCode( int code )
{
  if ( code < 10 )
  {
    mTextStream << QString( "  %1\n" ).arg( code );
  }
  else if ( code < 100 )
  {
    mTextStream << QString( " %1\n" ).arg( code );
  }
  else
  {
    mTextStream << QString( "%1\n" ).arg( code );
  }

}

void QgsDxfExport::writeInt( int i )
{
  mTextStream << QString( "%1\n" ).arg( i );
}

void QgsDxfExport::writeDouble( double d )
{
  mTextStream << qgsDoubleToString( d ) << "\n";
}

void QgsDxfExport::writeString( const QString& s )
{
  mTextStream << s << "\n";
}

int QgsDxfExport::writeToFile( QIODevice* d )
{
  if ( !d )
  {
    return 1;
  }

  if ( !d->isOpen() && !d->open( QIODevice::WriteOnly ) )
  {
    return 2;
  }

  mTextStream.setDevice( d );

  writeHeader();
  writeTables();
  writeBlocks();
  writeEntities();
  writeEndFile();
  return 0;
}

void QgsDxfExport::writeHeader()
{
  writeGroup( 999, "DXF created from QGIS" );
  startSection();
  writeGroup( 2, "HEADER" );

  //ACADVER
  writeGroup( 9, "$ACADVER" );
  writeGroup( 1, "AC1009" );

  QgsRectangle ext = dxfExtent();
  if ( !ext.isEmpty() )
  {
    //EXTMIN
    writeGroup( 9, "$EXTMIN" );
    writeGroup( 10, ext.xMinimum() );
    writeGroup( 20, ext.yMinimum() );
    writeGroup( 30, 0.0 );

    //EXTMAX
    writeGroup( 9, "$EXTMAX" );
    writeGroup( 10, ext.xMaximum() );
    writeGroup( 20, ext.yMaximum() );
    writeGroup( 30, 0.0 );
  }

  //LTSCALE
  writeGroup( 9, "$LTSCALE" );
  writeGroup( 40, 1.0 );

  //PDMODE
  writeGroup( 9, "$PDMODE" );
  writeGroup( 70, 33 );

  //PDSIZE
  writeGroup( 9, "$PDSIZE" );
  writeGroup( 40, 1 );

  //PSLTSCALE
  writeGroup( 9, "$PSLTSCALE" );
  writeGroup( 70, 0 );

  endSection();
}

void QgsDxfExport::writeTables()
{
  startSection();
  writeGroup( 2, "TABLES" );

  //iterate through all layers and get symbol layer pointers
  QList< QPair<QgsSymbolLayerV2*, QgsSymbolV2*> > slList;
  if ( mSymbologyExport != NoSymbology )
  {
    slList = symbolLayers();
  }

  //LTYPE
  mLineStyles.clear();
  writeGroup( 0, "TABLE" );
  writeGroup( 2, "LTYPE" );
  writeGroup( 70, nLineTypes( slList ) + 5 );

  writeDefaultLinestyles();
  //add custom linestyles
  QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2*> >::const_iterator slIt = slList.constBegin();
  for ( ; slIt != slList.constEnd(); ++slIt )
  {
    writeSymbolLayerLinestyle( slIt->first );
  }

  writeGroup( 0, "ENDTAB" );

  //LAYER
  writeGroup( 0, "TABLE" );
  writeGroup( 2, "LAYER" );
  writeGroup( 70, mLayers.count() );

  QList< QgsMapLayer* >::const_iterator layerIt = mLayers.constBegin();
  for ( ; layerIt != mLayers.constEnd(); ++layerIt )
  {
    if ( !layerIsScaleBasedVisible(( *layerIt ) ) )
    {
      continue;
    }

    writeGroup( 0, "LAYER" );
    QString layerName = *layerIt ? ( *layerIt )->name() : "";
    writeGroup( 2, dxfLayerName( layerName ) );
    writeGroup( 70, 64 );
    writeGroup( 62, 1 );
    writeGroup( 6, "CONTINUOUS" );
  }
  writeGroup( 0, "ENDTAB" );

  //STYLE
  writeGroup( 0, "TABLE" );
  writeGroup( 2, "STYLE" );
  writeGroup( 70, 1 );

  //provide only standard font for the moment
  writeGroup( 0, "STYLE" );
  writeGroup( 2, "STANDARD" );
  writeGroup( 70, 64 );
  writeGroup( 40, 0.0 );
  writeGroup( 41, 1.0 );
  writeGroup( 50, 0.0 );
  writeGroup( 71, 0 );
  writeGroup( 42, 5.0 );
  writeGroup( 3, "romans.shx" );
  writeGroup( 4, "" );

  writeGroup( 0, "ENDTAB" );

  endSection();
}

void QgsDxfExport::writeBlocks()
{
  startSection();
  writeGroup( 2, "BLOCKS" );

  //iterate through all layers and get symbol layer pointers
  QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2* > > slList;
  if ( mSymbologyExport != NoSymbology )
  {
    slList = symbolLayers();
  }

  QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2* > >::const_iterator slIt = slList.constBegin();
  for ( ; slIt != slList.constEnd(); ++slIt )
  {
    QgsMarkerSymbolLayerV2* ml = dynamic_cast< QgsMarkerSymbolLayerV2*>( slIt->first );
    if ( ml )
    {
      //if point symbol layer and no data defined properties: write block
      QgsRenderContext ct;
      QgsSymbolV2RenderContext ctx( ct, QgsSymbolV2::MapUnit, slIt->second->alpha(), false, slIt->second->renderHints(), 0 );
      ml->startRender( ctx );

      //markers with data defined properties are inserted inline
      if ( hasDataDefinedProperties( ml, slIt->second ) )
      {
        continue;
        // ml->stopRender( ctx );
      }
      writeGroup( 0, "BLOCK" );
      writeGroup( 8, 0 );
      QString blockName = QString( "symbolLayer%1" ).arg( mBlockCounter++ );
      writeGroup( 2, blockName );
      writeGroup( 70, 64 );

      //x/y/z coordinates of reference point
      //todo: consider anchor point
      // double size = ml->size();
      // size *= mapUnitScaleFactor( mSymbologyScaleDenominator, ml->sizeUnit(), mMapUnits );
      writeGroup( 10, 0 );
      writeGroup( 20, 0 );
      writeGroup( 30, 0 );
      writeGroup( 3, blockName );

      ml->writeDxf( *this, mapUnitScaleFactor( mSymbologyScaleDenominator, ml->sizeUnit(), mMapUnits ), "0", &ctx, 0 ); //maplayer 0 -> block receives layer from INSERT statement

      writeGroup( 0, "ENDBLK" );
      writeGroup( 8, 0 );

      mPointSymbolBlocks.insert( ml, blockName );
      ml->stopRender( ctx );
    }
  }
  endSection();
}

void QgsDxfExport::writeEntities()
{
  startSection();
  writeGroup( 2, "ENTITIES" );

  //label engine
  QgsDxfPalLabeling labelEngine( this, mExtent.isEmpty() ? dxfExtent() : mExtent, mSymbologyScaleDenominator, mMapUnits );
  QgsRenderContext& ctx = labelEngine.renderContext();

  //iterate through the maplayers
  QList< QgsMapLayer* >::iterator layerIt = mLayers.begin();
  for ( ; layerIt != mLayers.end(); ++layerIt )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( *layerIt );
    if ( !vl || !layerIsScaleBasedVisible( vl ) )
    {
      continue;
    }

    QgsSymbolV2RenderContext sctx( ctx, QgsSymbolV2::MM , 1.0, false, 0, 0 );
    QgsFeatureRendererV2* renderer = vl->rendererV2();
    renderer->startRender( ctx, vl->pendingFields() );

    QStringList attributes = renderer->usedAttributes();

    bool labelLayer = ( labelEngine.prepareLayer( vl, attributes, ctx ) != 0 );

    if ( mSymbologyExport == QgsDxfExport::SymbolLayerSymbology && ( renderer->capabilities() & QgsFeatureRendererV2::SymbolLevels ) &&
         renderer->usingSymbolLevels() )
    {
      writeEntitiesSymbolLevels( vl );
      renderer->stopRender( ctx );
      continue;
    }

    QgsFeatureRequest freq = QgsFeatureRequest().setSubsetOfAttributes(
                               attributes, vl->pendingFields() );
    if ( !mExtent.isEmpty() )
    {
      freq.setFilterRect( mExtent );
    }
    QgsFeatureIterator featureIt = vl->getFeatures( freq );
    QgsFeature fet;
    while ( featureIt.nextFeature( fet ) )
    {
      sctx.setFeature( &fet );
      if ( mSymbologyExport == NoSymbology )
      {
        addFeature( sctx, dxfLayerName( vl->name() ), 0, 0 ); //no symbology at all
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

        if ( mSymbologyExport == QgsDxfExport::SymbolLayerSymbology ) //symbol layer symbology, but layer does not use symbol levels
        {
          QgsSymbolV2List::iterator symbolIt = symbolList.begin();
          for ( ; symbolIt != symbolList.end(); ++symbolIt )
          {
            int nSymbolLayers = ( *symbolIt )->symbolLayerCount();
            for ( int i = 0; i < nSymbolLayers; ++i )
            {
              addFeature( sctx, dxfLayerName( vl->name() ), ( *symbolIt )->symbolLayer( i ), *symbolIt );
            }
          }
        }
        else
        {
          //take first symbollayer from first symbol
          QgsSymbolV2* s = symbolList.first();
          if ( !s || s->symbolLayerCount() < 1 )
          {
            continue;
          }
          addFeature( sctx, dxfLayerName( vl->name() ), s->symbolLayer( 0 ), s );
        }

        if ( labelLayer )
        {
          labelEngine.registerFeature( vl->id(), fet, ctx );
        }
      }
    }
    renderer->stopRender( ctx );
  }

  labelEngine.drawLabeling( ctx );
  endSection();
}

void QgsDxfExport::writeEntitiesSymbolLevels( QgsVectorLayer* layer )
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

  QgsRenderContext ctx = renderContext();
  QgsSymbolV2RenderContext sctx( ctx, QgsSymbolV2::MM , 1.0, false, 0, 0 );
  renderer->startRender( ctx, layer->pendingFields() );

  //get iterator
  QgsFeatureRequest req;
  if ( layer->wkbType() == QGis::WKBNoGeometry )
  {
    req.setFlags( QgsFeatureRequest::NoGeometry );
  }
  req.setSubsetOfAttributes( QStringList( renderer->usedAttributes() ), layer->pendingFields() );
  if ( !mExtent.isEmpty() )
  {
    req.setFilterRect( mExtent );
  }
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
        addFeature( sctx, layer->name(), levelIt.key()->symbolLayer( llayer ), levelIt.key() );
      }
    }
  }
  renderer->stopRender( ctx );
}

void QgsDxfExport::writeEndFile()
{
  writeGroup( 0, "EOF" );
}

void QgsDxfExport::startSection()
{
  writeGroup( 0, "SECTION" );
}

void QgsDxfExport::endSection()
{
  writeGroup( 0, "ENDSEC" );
}

void QgsDxfExport::writePoint( const QgsPoint& pt, const QString& layer, int color, const QgsFeature* f, const QgsSymbolLayerV2* symbolLayer, const QgsSymbolV2* symbol )
{
#if 0
  //debug: draw rectangle for debugging
  const QgsMarkerSymbolLayerV2* msl = dynamic_cast< const QgsMarkerSymbolLayerV2* >( symbolLayer );
  if ( msl )
  {
    double halfSize = msl->size() * mapUnitScaleFactor( mSymbologyScaleDenominator,
                      msl->sizeUnit(), mMapUnits ) / 2.0;
    writeGroup( 0, "SOLID" );
    writeGroup( 8, layer );
    writeGroup( 62, 1 );
    writeGroup( 10, pt.x() - halfSize );
    writeGroup( 20, pt.y() - halfSize );
    writeGroup( 30, 0.0 );
    writeGroup( 11, pt.x() + halfSize );
    writeGroup( 21, pt.y() - halfSize );
    writeGroup( 31, 0.0 );
    writeGroup( 12, pt.x() - halfSize );
    writeGroup( 22, pt.y() + halfSize );
    writeGroup( 32, 0.0 );
    writeGroup( 13, pt.x() + halfSize );
    writeGroup( 23, pt.y() + halfSize );
    writeGroup( 33, 0.0 );
  }
#endif //0

  //insert block or write point directly?
  QHash< const QgsSymbolLayerV2*, QString >::const_iterator blockIt = mPointSymbolBlocks.find( symbolLayer );
  if ( !symbolLayer || blockIt == mPointSymbolBlocks.constEnd() )
  {
    //write symbol directly here
    const QgsMarkerSymbolLayerV2* msl = dynamic_cast< const QgsMarkerSymbolLayerV2* >( symbolLayer );
    if ( symbolLayer && symbol )
    {
      QgsRenderContext ct;
      QgsSymbolV2RenderContext ctx( ct, QgsSymbolV2::MapUnit, symbol->alpha(), false, symbol->renderHints(), f );
      if ( symbolLayer->writeDxf( *this, mapUnitScaleFactor( mSymbologyScaleDenominator, msl->sizeUnit(), mMapUnits ), layer, &ctx, f, QPointF( pt.x(), pt.y() ) ) )
      {
        return;
      }
    }
    writePoint( layer, color, pt ); //write default point symbol
  }
  else
  {
    //insert block reference
    writeGroup( 0, "INSERT" );
    writeGroup( 8, layer );
    writeGroup( 2, blockIt.value() );
    writeGroup( 10, pt.x() );
    writeGroup( 20, pt.y() );
    writeGroup( 30, 0 );
  }
}

void QgsDxfExport::writePolyline( const QgsPolyline& line, const QString& layer, const QString& lineStyleName, int color,
                                  double width, bool polygon )
{
  writeGroup( 0, "POLYLINE" );
  writeGroup( 8, layer );
  writeGroup( 6, lineStyleName );
  writeGroup( 62, color );
  writeGroup( 66, 1 );
  int type = polygon ? 1 : 0;
  writeGroup( 70, type );
  if ( width > 0 ) //width -1: use default width
  {
    writeGroup( 40, width );
    writeGroup( 41, width );
  }

  QgsPolyline::const_iterator lineIt = line.constBegin();
  for ( ; lineIt != line.constEnd(); ++lineIt )
  {
    writeVertex( *lineIt, layer );
  }

  writeGroup( 0, "SEQEND" );
}

void QgsDxfExport::writeLine( const QgsPoint& pt1, const QgsPoint& pt2, const QString& layer, const QString& lineStyleName, int color, double width )
{
  QgsPolyline line( 2 );
  line[0] = pt1;
  line[1] = pt2;
  writePolyline( line, layer, lineStyleName, color, width, false );
}

void QgsDxfExport::writePoint( const QString& layer, int color, const QgsPoint& pt )
{
  writeGroup( 0, "POINT" );
  writeGroup( 8, layer );
  writeGroup( 62, color );
  writeGroup( 10, pt.x() );
  writeGroup( 20, pt.y() );
  writeGroup( 30, 0.0 );
}

void QgsDxfExport::writeCircle( const QString& layer, int color, const QgsPoint& pt, double radius )
{
  writeGroup( 0, "CIRCLE" );
  writeGroup( 8, layer );
  writeGroup( 62, color );
  writeGroup( 10, pt.x() );
  writeGroup( 20, pt.y() );
  writeGroup( 30, 0 );
  writeGroup( 40, radius );
}

void QgsDxfExport::writeText( const QString& layer, const QString& text, const QgsPoint& pt, double size, double angle, int color )
{
  writeGroup( 0, "TEXT" );
  writeGroup( 8, layer );
  writeGroup( 62, color );
  writeGroup( 10, pt.x() );
  writeGroup( 20, pt.y() );
  writeGroup( 30, 0 );
  writeGroup( 40, size );
  writeGroup( 1, text );
  writeGroup( 50, angle );
  writeGroup( 7, "STANDARD" ); //so far only support for standard font
}

void QgsDxfExport::writeSolid( const QString& layer, int color, const QgsPoint& pt1, const QgsPoint& pt2, const QgsPoint& pt3, const QgsPoint& pt4 )
{
  writeGroup( 0, "SOLID" );
  writeGroup( 8, layer );
  writeGroup( 62, color );
  writeGroup( 10, pt1.x() );
  writeGroup( 20, pt1.y() );
  writeGroup( 30, 0.0 );
  writeGroup( 11, pt2.x() );
  writeGroup( 21, pt2.y() );
  writeGroup( 31, 0.0 );
  writeGroup( 12, pt3.x() );
  writeGroup( 22, pt3.y() );
  writeGroup( 32, 0.0 );
  writeGroup( 13, pt4.x() );
  writeGroup( 23, pt4.y() );
  writeGroup( 33, 0.0 );
}

void QgsDxfExport::writeVertex( const QgsPoint& pt, const QString& layer )
{
  writeGroup( 0, "VERTEX" );
  writeGroup( 8, layer );
  writeGroup( 10, pt.x() );
  writeGroup( 20, pt.y() );
  writeGroup( 30, 0 );
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

void QgsDxfExport::addFeature( const QgsSymbolV2RenderContext& ctx, const QString& layer, const QgsSymbolLayerV2* symbolLayer, const QgsSymbolV2* symbol )
{
  const QgsFeature* fet = ctx.feature();
  if ( !fet )
  {
    return;
  }

  QgsGeometry* geom = fet->geometry();
  if ( geom )
  {
    int c = 0;
    if ( mSymbologyExport != NoSymbology )
    {
      c = colorFromSymbolLayer( symbolLayer, ctx );
    }
    double width = -1;
    if ( mSymbologyExport != NoSymbology && symbolLayer )
    {
      width = symbolLayer->dxfWidth( *this, ctx );
    }
    QString lineStyleName = "CONTINUOUS";
    if ( mSymbologyExport != NoSymbology )
    {
      lineStyleName = lineStyleFromSymbolLayer( symbolLayer );
    }
    QGis::WkbType geometryType = geom->wkbType();

    //single point
    if ( geometryType == QGis::WKBPoint || geometryType == QGis::WKBPoint25D )
    {
      writePoint( geom->asPoint(), layer, c, fet, symbolLayer, symbol );
    }

    //multipoint
    if ( geometryType == QGis::WKBMultiPoint || geometryType == QGis::WKBMultiPoint25D )
    {
      QgsMultiPoint multiPoint = geom->asMultiPoint();
      QgsMultiPoint::const_iterator it = multiPoint.constBegin();
      for ( ; it != multiPoint.constEnd(); ++it )
      {
        writePoint( *it, layer, c, fet, symbolLayer, symbol );
      }
    }

    //single line
    if ( geometryType == QGis::WKBLineString || geometryType == QGis::WKBLineString25D )
    {
      writePolyline( geom->asPolyline(), layer, lineStyleName, c, width, false );
    }

    //multiline
    if ( geometryType == QGis::WKBMultiLineString || geometryType == QGis::WKBMultiLineString25D )
    {
      QgsMultiPolyline multiLine = geom->asMultiPolyline();
      QgsMultiPolyline::const_iterator lIt = multiLine.constBegin();
      for ( ; lIt != multiLine.constEnd(); ++lIt )
      {
        writePolyline( *lIt, layer, lineStyleName, c, width, false );
      }
    }

    //polygon
    if ( geometryType == QGis::WKBPolygon || geometryType == QGis::WKBPolygon25D )
    {
      QgsPolygon polygon = geom->asPolygon();
      QgsPolygon::const_iterator polyIt = polygon.constBegin();
      for ( ; polyIt != polygon.constEnd(); ++polyIt ) //iterate over rings
      {
        writePolyline( *polyIt, layer, lineStyleName, c, width, true );
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
          writePolyline( *polyIt, layer, lineStyleName, c, width, true );
        }
      }
    }
  }
}

int QgsDxfExport::colorFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer, const QgsSymbolV2RenderContext& ctx )
{
  if ( !symbolLayer )
  {
    return 0;
  }

  QColor c = symbolLayer->dxfColor( ctx );
  return closestColorMatch( c.rgba() );
}

QString QgsDxfExport::lineStyleFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer )
{
  QString lineStyleName = "CONTINUOUS";
  if ( !symbolLayer )
  {
    return lineStyleName;
  }

  QHash< const QgsSymbolLayerV2*, QString >::const_iterator lineTypeIt = mLineStyles.find( symbolLayer );
  if ( lineTypeIt != mLineStyles.constEnd() )
  {
    lineStyleName = lineTypeIt.value();
  }
  else
  {
    return lineNameFromPenStyle( symbolLayer->dxfPenStyle() );
  }
  return lineStyleName;
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

double QgsDxfExport::mapUnitScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits )
{
  if ( symbolUnits == QgsSymbolV2::MapUnit )
  {
    return 1.0;
  }
  //MM symbol unit
  return scaleDenominator * QGis::fromUnitToUnitFactor( QGis::Meters, mapUnits ) / 1000.0;
}

QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2* > > QgsDxfExport::symbolLayers()
{
  QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2* > > symbolLayers;

  QList< QgsMapLayer* >::iterator lIt = mLayers.begin();
  for ( ; lIt != mLayers.end(); ++lIt )
  {
    //cast to vector layer
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( *lIt );
    if ( !vl )
    {
      continue;
    }

    //get rendererv2
    QgsFeatureRendererV2* r = vl->rendererV2();
    if ( !r )
    {
      continue;
    }

    //get all symbols
    QgsSymbolV2List symbols = r->symbols();
    QgsSymbolV2List::iterator symbolIt = symbols.begin();
    for ( ; symbolIt != symbols.end(); ++symbolIt )
    {
      int maxSymbolLayers = ( *symbolIt )->symbolLayerCount();
      if ( mSymbologyExport != SymbolLayerSymbology )
      {
        maxSymbolLayers = 1;
      }
      for ( int i = 0; i < maxSymbolLayers; ++i )
      {
        symbolLayers.append( qMakePair(( *symbolIt )->symbolLayer( i ), *symbolIt ) ) ;
      }
    }
  }

  return symbolLayers;
}

void QgsDxfExport::writeDefaultLinestyles()
{
  double das = dashSize();
  double dos = dotSize();
  double dss = dashSeparatorSize();

  //continuous (Qt solid line)
  writeGroup( 0, "LTYPE" );
  writeGroup( 2, "CONTINUOUS" );
  writeGroup( 70, 64 );
  writeGroup( 3, "Defaultstyle" );
  writeGroup( 72, 65 );
  writeGroup( 73, 0 );
  writeGroup( 40, 0.0 );

  QVector<qreal> dashVector( 2 );
  dashVector[0] = das;
  dashVector[1] = dss;
  writeLinestyle( "DASH", dashVector, QgsSymbolV2::MapUnit );

  QVector<qreal> dotVector( 2 );
  dotVector[0] = dos;
  dotVector[1] = dss;
  writeLinestyle( "DOT", dotVector, QgsSymbolV2::MapUnit );

  QVector<qreal> dashDotVector( 4 );
  dashDotVector[0] = das;
  dashDotVector[1] = dss;
  dashDotVector[2] = dos;
  dashDotVector[3] = dss;
  writeLinestyle( "DASHDOT", dashDotVector, QgsSymbolV2::MapUnit );

  QVector<qreal> dashDotDotVector( 6 );
  dashDotDotVector[0] = das;
  dashDotDotVector[1] = dss;
  dashDotDotVector[2] = dos;
  dashDotDotVector[3] = dss;
  dashDotDotVector[4] = dos;
  dashDotDotVector[5] = dss;
  writeLinestyle( "DASHDOTDOT", dashDotDotVector, QgsSymbolV2::MapUnit );
}

void QgsDxfExport::writeSymbolLayerLinestyle( const QgsSymbolLayerV2* symbolLayer )
{
  if ( !symbolLayer )
  {
    return;
  }

  QgsSymbolV2::OutputUnit unit;
  QVector<qreal> customLinestyle = symbolLayer->dxfCustomDashPattern( unit );
  if ( customLinestyle.size() > 0 )
  {
    QString name = QString( "symbolLayer%1" ).arg( mSymbolLayerCounter++ );
    writeLinestyle( name, customLinestyle, unit );
    mLineStyles.insert( symbolLayer, name );
  }
}

int QgsDxfExport::nLineTypes( const QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2* > >& symbolLayers )
{
  int nLineTypes = 0;
  QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2*> >::const_iterator slIt = symbolLayers.constBegin();
  for ( ; slIt != symbolLayers.constEnd(); ++slIt )
  {
    const QgsSimpleLineSymbolLayerV2* simpleLine = dynamic_cast< const QgsSimpleLineSymbolLayerV2* >( slIt->first );
    if ( simpleLine )
    {
      if ( simpleLine->useCustomDashPattern() )
      {
        ++nLineTypes;
      }
    }
  }
  return nLineTypes;
}

void QgsDxfExport::writeLinestyle( const QString& styleName, const QVector<qreal>& pattern, QgsSymbolV2::OutputUnit u )
{
  double length = 0;
  QVector<qreal>::const_iterator dashIt = pattern.constBegin();
  for ( ; dashIt != pattern.constEnd(); ++dashIt )
  {
    length += ( *dashIt * mapUnitScaleFactor( mSymbologyScaleDenominator, u, mMapUnits ) );
  }

  writeGroup( 0, "LTYPE" );
  writeGroup( 2, styleName );
  writeGroup( 70, 64 );
  writeGroup( 3, "" );
  writeGroup( 72, 65 );
  writeGroup( 73, pattern.size() );
  writeGroup( 40, length );

  dashIt = pattern.constBegin();
  bool isSpace = false;
  for ( ; dashIt != pattern.constEnd(); ++dashIt )
  {
    //map units or mm?
    double segmentLength = ( isSpace ? -*dashIt : *dashIt );
    segmentLength *= mapUnitScaleFactor( mSymbologyScaleDenominator, u, mMapUnits );
    writeGroup( 49, segmentLength );
    isSpace = !isSpace;
  }
}

bool QgsDxfExport::hasDataDefinedProperties( const QgsSymbolLayerV2* sl, const QgsSymbolV2* symbol )
{
  if ( !sl || !symbol )
  {
    return false;
  }

  if ( symbol->renderHints() & QgsSymbolV2::DataDefinedSizeScale ||
       symbol->renderHints() & QgsSymbolV2::DataDefinedRotation )
  {
    return true;
  }

  return sl->hasDataDefinedProperties();
}

double QgsDxfExport::dashSize() const
{
  double size = mSymbologyScaleDenominator * 0.002;
  return sizeToMapUnits( size );
}

double QgsDxfExport::dotSize() const
{
  double size = mSymbologyScaleDenominator * 0.0006;
  return sizeToMapUnits( size );
}

double QgsDxfExport::dashSeparatorSize() const
{
  double size = mSymbologyScaleDenominator * 0.0006;
  return sizeToMapUnits( size );
}

double QgsDxfExport::sizeToMapUnits( double s ) const
{
  double size = s * QGis::fromUnitToUnitFactor( QGis::Meters, mMapUnits );
  return size;
}

QString QgsDxfExport::lineNameFromPenStyle( Qt::PenStyle style )
{
  switch ( style )
  {
    case Qt::DashLine:
      return "DASH";
    case Qt::DotLine:
      return "DOT";
    case Qt::DashDotLine:
      return "DASHDOT";
    case Qt::DashDotDotLine:
      return "DASHDOTDOT";
    case Qt::SolidLine:
    default:
      return "CONTINUOUS";
  }
}

QString QgsDxfExport::dxfLayerName( const QString& name )
{
  //dxf layers can be max 31 characters long
  QString layerName = name.left( 31 );

  //allowed characters are 0-9, A-Z, $, -, _
  for ( int i = 0; i < layerName.size(); ++i )
  {
    QChar c = layerName.at( i );
    if ( c > 122 )
    {
      layerName[i] = '_';
      continue;
    }

    if ( c.isNumber() )
    {
      continue;
    }
    if ( c == '$' || c == '-' || c == '_' )
    {
      continue;
    }

    if ( !c.isLetter() )
    {
      layerName[i] = '_';
    }
  }
  return layerName;
}

bool QgsDxfExport::layerIsScaleBasedVisible( const QgsMapLayer* layer ) const
{
  if ( !layer )
  {
    return false;
  }

  if ( mSymbologyExport == QgsDxfExport::NoSymbology || !layer->hasScaleBasedVisibility() )
  {
    return true;
  }

  return ( layer->minimumScale() < mSymbologyScaleDenominator &&
           layer->maximumScale() > mSymbologyScaleDenominator );
}

/******************************************************Test with AC_1018 methods***************************************************************/

void QgsDxfExport::writeHeaderAC1018( QTextStream& stream )
{
  stream << "999\n";
  stream << "DXF created from QGIS\n";
  startSection();
  stream << "  2\n";
  stream << "HEADER\n";
  //ACADVER
  stream << "  9\n";
  stream << "$ACADVER\n";
  stream << "  1\n";
  stream << "AC1018\n";

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
  endSection();
  //PSLTSCALE
  stream << "  9\n";
  stream << "$PSLTSCALE\n";
  stream << " 70\n";
  stream << "0\n";
}

void QgsDxfExport::writeTablesAC1018( QTextStream& stream )
{
  startSection();
  stream << "  2\n";
  stream << "TABLES\n";

  //APPID
  stream << "  0\n";
  stream << "TABLE\n";
  stream << "  2\n";
  stream << "APPID\n";
  stream << "  5\n";
  stream << QString( "%1\n" ).arg( mNextHandleId++ );
  stream << "100\n";
  stream << "AcDbSymbolTable\n";
  stream << " 70\n";
  stream << "  1\n";
  stream << "  0\n";
  stream << "APPID\n";
  stream << "  5\n";
  stream << QString( "%1\n" ).arg( mNextHandleId++ );
  stream << "100\n";
  stream << "AcDbSymbolTableRecord\n";
  stream << "100\n";
  stream << "AcDbRegAppTableRecord\n";
  stream << "  2\n";
  stream << "ACAD\n";
  stream << " 70\n";
  stream << "  0\n";
  stream << "  0\n";
  stream << "ENDTAB\n";

  //VPORT table
  stream << "  0\n";
  stream << "TABLE\n";
  stream << "  2\n";
  stream << "VPORT\n";
  stream << "  5\n";
  stream << QString( "%1\n" ).arg( mNextHandleId++ );
  stream << "100\n";
  stream << "AcDbSymbolTable\n";
  stream << " 70\n";
  stream << "1\n";
  stream << "  0\n";
  stream << "VPORT\n";
  stream << "  5\n";
  stream << QString( "%1\n" ).arg( mNextHandleId++ );
  stream << "100\n";
  stream << "AcDbSymbolTableRecord\n";
  stream << "100\n";
  stream << "AcDbViewportTableRecord\n";
  stream << "  2\n";
  stream << "*Active\n";
  stream << " 70\n";
  stream << "  0\n";
  stream << " 10\n";
  stream << " 0.0\n";
  stream << " 20\n";
  stream << "0.0\n";
  stream << " 11\n";
  stream << " 1.0\n";
  stream << " 21\n";
  stream << "1.0\n";
  stream << " 12\n";
  stream << "80.25\n";
  stream << " 22\n";
  stream << "106.4409457059851\n";
  stream << " 40\n";
  stream << "113.3818914119703\n";
  stream << " 41\n";
  stream << "0.8863849310366128\n";
  stream << " 42\n";
  stream << "50.0\n";
  stream << "  0\n";
  stream << "ENDTAB\n";

  //iterate through all layers and get symbol layer pointers
  QList<QgsSymbolLayerV2*> slList;
  if ( mSymbologyExport != NoSymbology )
  {
    //slList = symbolLayers(); //todo...
  }

  //LTYPE
  mLineStyles.clear();
  stream << "  0\n";
  stream << "TABLE\n";
  stream << "  2\n";
  stream << "LTYPE\n";
  stream << "  5\n";
  stream << QString( "%1\n" ).arg( mNextHandleId++ );
  stream << "100\n";
  stream << "AcDbSymbolTable\n";
  stream << " 70\n";
  //stream << QString( "%1\n" ).arg( nLineTypes( slList ) + 1 ); //number of linetypes

  //add continuous style as default
  stream << "  0\n";
  stream << "LTYPE\n";
  stream << "  5\n";
  stream << QString( "%1\n" ).arg( mNextHandleId++ );
  stream << "100\n";
  stream << "AcDbSymbolTableRecord\n";
  stream << "100\n";
  stream << "AcDbLinetypeTableRecord\n";
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
  stream << "0.0\n";

  stream << "  0\n";
  stream << "LTYPE\n";
  stream << "  5\n";
  stream << QString( "%1\n" ).arg( mNextHandleId++ );
  stream << "100\n";
  stream << "AcDbSymbolTableRecord\n";
  stream << "100\n";
  stream << "AcDbLinetypeTableRecord\n";
  stream << "  2\n";
  stream << "BYBLOCK\n";
  stream << "  70\n";
  stream << "64\n";
  stream << "  3\n";
  stream << "Defaultstyle\n";
  stream << " 72\n";
  stream << "65\n";
  stream << " 73\n";
  stream << "0\n";
  stream << " 40\n"; //todo: add segments in group 49
  stream << "0.0\n";

  stream << "  0\n";
  stream << "LTYPE\n";
  stream << "  5\n";
  stream << QString( "%1\n" ).arg( mNextHandleId++ );
  stream << "100\n";
  stream << "AcDbSymbolTableRecord\n";
  stream << "100\n";
  stream << "AcDbLinetypeTableRecord\n";
  stream << "  2\n";
  stream << "BYLAYER\n";
  stream << "  70\n";
  stream << "64\n";
  stream << "  3\n";
  stream << "Defaultstyle\n";
  stream << " 72\n";
  stream << "65\n";
  stream << " 73\n";
  stream << "0\n";
  stream << " 40\n"; //todo: add segments in group 49
  stream << "0.0\n";

  //add symbol layer linestyles
  QList<QgsSymbolLayerV2*>::const_iterator slIt = slList.constBegin();
  for ( ; slIt != slList.constEnd(); ++slIt )
  {
    writeSymbolLayerLinestyleAC1018( stream, *slIt );
  }

  stream << "  0\n";
  stream << "ENDTAB\n";

  //LAYER
  stream << "  0\n";
  stream << "TABLE\n";
  stream << "  2\n";
  stream << "LAYER\n";
  stream << "  5\n";
  stream << QString( "%1\n" ).arg( mNextHandleId++ );
  stream << "100\n";
  stream << "AcDbSymbolTable\n";
  stream << " 70\n";
  stream << mLayers.count() << "\n";
  QList< QgsMapLayer* >::const_iterator layerIt = mLayers.constBegin();
  for ( ; layerIt != mLayers.constEnd(); ++layerIt )
  {
    stream << "  0\n";
    stream << "LAYER\n";
    stream << "  5\n";
    stream << QString( "%1\n" ).arg( mNextHandleId++ );
    stream << "100\n";
    stream << "AcDbSymbolTableRecord\n";
    stream << "100\n";
    stream << "AcDbLayerTableRecord\n";
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

  //todo: VIEW table

  //todo: UCS table

  //todo: DIMSTYLE table

  //todo: BLOCK_RECORD table

  endSection( );
}

void QgsDxfExport::writeSymbolLayerLinestyleAC1018( QTextStream& stream, const QgsSymbolLayerV2* symbolLayer )
{
  if ( !symbolLayer )
  {
    return;
  }

  //QgsSimpleLineSymbolLayer can have customDashVector() / customDashPatternUnit()
  const QgsSimpleLineSymbolLayerV2* simpleLine = dynamic_cast< const QgsSimpleLineSymbolLayerV2* >( symbolLayer );
  if ( simpleLine )
  {
    if ( simpleLine->useCustomDashPattern() )
    {
      ++mSymbolLayerCounter;
      QString name = QString( "symbolLayer%1" ).arg( mSymbolLayerCounter );
      QVector<qreal> dashPattern = simpleLine->customDashVector();
      writeLinestyleAC1018( stream, name, dashPattern, simpleLine->customDashPatternUnit() );
      mLineStyles.insert( symbolLayer, name );
    }
  }
}

void QgsDxfExport::writeLinestyleAC1018( QTextStream& stream, const QString& styleName, const QVector<qreal>& pattern, QgsSymbolV2::OutputUnit u )
{
  double length = 0;
  QVector<qreal>::const_iterator dashIt = pattern.constBegin();
  for ( ; dashIt != pattern.constEnd(); ++dashIt )
  {
    length += *dashIt;
  }

  stream << "  0\n";
  stream << "LTYPE\n";
  stream << "  5\n";
  stream << QString( "%1\n" ).arg( mNextHandleId++ );
  stream << "100\n";
  stream << "AcDbSymbolTableRecord\n";
  stream << "100\n";
  stream << "AcDbLinetypeTableRecord\n";
  stream << "  2\n";
  stream << QString( "%1\n" ).arg( styleName );
  stream << "  70\n";
  stream << "64\n";
  stream << "  3\n";
  stream << "\n";
  stream << " 72\n";
  stream << "65\n";
  stream << " 73\n";
  stream << QString( "%1\n" ).arg( pattern.size() ); //number of segments
  stream << " 40\n"; //total length of segments
  stream << QString( "%1\n" ).arg( length );

  dashIt = pattern.constBegin();
  bool isSpace = false;
  for ( ; dashIt != pattern.constEnd(); ++dashIt )
  {
    stream << " 49\n";

    //map units or mm?
    double segmentLength = ( isSpace ? -*dashIt : *dashIt );
    segmentLength *= mapUnitScaleFactor( mSymbologyScaleDenominator, u, mMapUnits );
    stream << QString( "%1\n" ).arg( segmentLength );
    isSpace = !isSpace;
  }
}

void QgsDxfExport::writeEntitiesAC1018( QTextStream& stream )
{
  Q_UNUSED( stream );
  //todo...
}

void QgsDxfExport::writeEntitiesSymbolLevelsAC1018( QTextStream& stream, QgsVectorLayer* layer )
{
  Q_UNUSED( stream );
  Q_UNUSED( layer );
  //todo...
}

void QgsDxfExport::writePolylineAC1018( QTextStream& stream, const QgsPolyline& line, const QString& layer, const QString& lineStyleName, int color,
                                        double width, bool polygon )
{
  stream << "  0\n";
  stream << "LWPOLYLINE\n";
  stream << "  5\n";
  stream << QString( "%1\n" ).arg( mNextHandleId++ );
  stream << "  8\n";
  stream << layer << "\n";
  stream << "100\n";
  stream << "AcDbEntity\n";
  stream << "100\n";
  stream << "AcDbPolyline\n";

  stream << "  6\n";
  stream << QString( "%1\n" ).arg( lineStyleName );

  stream << " 62\n";
  stream << color << "\n";

  stream << " 90\n";
  stream << QString( "%1\n" ).arg( line.size() );

  stream << " 70\n";
  int type = polygon ? 1 : 0;
  stream << type << "\n";

  stream << " 43\n";
  stream << width << "\n";

  QgsPolyline::const_iterator lineIt = line.constBegin();
  for ( ; lineIt != line.constEnd(); ++lineIt )
  {
    writeVertexAC1018( stream, *lineIt );
  }
}

void QgsDxfExport::writeVertexAC1018( QTextStream& stream, const QgsPoint& pt )
{
  stream << " 10\n";
  stream << pt.x() << "\n";
  stream << " 20\n";
  stream << pt.y() << "\n";
}

