/***************************************************************************
                              qgsinterpolationlayerbuilder.cpp
                              --------------------------------
  begin                : July, 2008
  copyright            : (C) 2008 by Marco Hugentobler
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

#include "qgsinterpolationlayerbuilder.h"
#include "qgslogger.h"
#include "qgsrasterlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QDomElement>
#include <QTemporaryFile>

//for interpolation
#include "qgsgridfilewriter.h"
#include "qgsidwinterpolator.h"
#include "qgstininterpolator.h"

QgsInterpolationLayerBuilder::QgsInterpolationLayerBuilder( QgsVectorLayer *vl ): mVectorLayer( vl )
{

}

QgsInterpolationLayerBuilder::QgsInterpolationLayerBuilder(): mVectorLayer( nullptr )
{

}

QgsMapLayer *QgsInterpolationLayerBuilder::createMapLayer( const QDomElement &elem,
    const QString &layerName,
    QList<QTemporaryFile *> &filesToRemove,
    QList<QgsMapLayer *> &layersToRemove,
    bool allowCaching ) const
{
  Q_UNUSED( layerName );
  Q_UNUSED( allowCaching );
  if ( !mVectorLayer )
  {
    return nullptr;
  }

  QDomNodeList interpolationList = elem.elementsByTagName( QStringLiteral( "Interpolation" ) );
  if ( interpolationList.isEmpty() )
  {
    QgsDebugMsg( "No Interpolation element found" );
    return nullptr;
  }
  QDomElement interpolationElem = interpolationList.at( 0 ).toElement();

  //create QgsInterpolator object from XML
  QDomNodeList tinList = interpolationElem.elementsByTagName( QStringLiteral( "TINMethod" ) );
  QDomNodeList idwList = interpolationElem.elementsByTagName( QStringLiteral( "IDWMethod" ) );

  QgsInterpolator *interpolator = nullptr;
  QList<QgsInterpolator::LayerData> layerDataList;
  QgsInterpolator::LayerData currentLayerData;
  currentLayerData.vectorLayer = mVectorLayer;
  QDomNodeList propertyNameList = interpolationElem.elementsByTagName( QStringLiteral( "PropertyName" ) );
  if ( propertyNameList.isEmpty() )
  {
    currentLayerData.zCoordInterpolation = true;
  }
  else
  {
    currentLayerData.zCoordInterpolation = false;

    //set attribute field interpolation or z-Coordinate interpolation
    QString attributeName = propertyNameList.at( 0 ).toElement().text();
    QgsVectorDataProvider *provider = mVectorLayer->dataProvider();
    if ( !provider )
    {
      return nullptr;
    }
    int attributeIndex = provider->fieldNameIndex( attributeName );
    if ( attributeIndex == -1 )
    {
      return nullptr; //attribute field not found
    }
    currentLayerData.interpolationAttribute = attributeIndex;
  }
  currentLayerData.mInputType = QgsInterpolator::POINTS;

  layerDataList.push_back( currentLayerData );

  if ( !idwList.isEmpty() ) //inverse distance interpolator
  {
    interpolator = new QgsIDWInterpolator( layerDataList );

    //todo: parse <DistanceWeightingCoefficient>
  }
  else //tin is default
  {
    interpolator = new QgsTINInterpolator( layerDataList );
    //todo: parse <InterpolationFunction>
  }

  //Resolution
  int nCols, nRows;

  QDomNodeList resolutionNodeList = elem.elementsByTagName( QStringLiteral( "Resolution" ) );
  if ( resolutionNodeList.isEmpty() )
  {
    //use default values...
    nCols = 100;
    nRows = 100;
  }
  else
  {
    QDomElement resolutionElem = resolutionNodeList.at( 0 ).toElement();
    nCols = resolutionElem.attribute( QStringLiteral( "ncols" ) ).toInt();
    nRows = resolutionElem.attribute( QStringLiteral( "nrows" ) ).toInt();
    if ( nCols == 0 || nRows == 0 )
    {
      QgsDebugMsg( "Reading of resolution failed" );
      delete interpolator;
      return nullptr;
    }
  }

  QTemporaryFile *tmpFile = new QTemporaryFile();
  if ( !tmpFile->open() )
  {
    QgsDebugMsg( "Opening temporary file failed" );
    delete tmpFile;
    delete interpolator;
    return nullptr;
  }

  QgsRectangle extent = mVectorLayer->extent();
  QgsGridFileWriter gridWriter( interpolator, tmpFile->fileName(), extent, nCols, nRows, extent.width() / nCols, extent.height() / nRows );
  if ( gridWriter.writeFile() != 0 )
  {
    QgsDebugMsg( "Interpolation of raster failed" );
    return nullptr;
  }

  filesToRemove.push_back( tmpFile ); //store raster in temporary file and remove after request
  QgsRasterLayer *raster = new QgsRasterLayer( tmpFile->fileName() );
  layersToRemove.push_back( raster );
  return raster;
}
