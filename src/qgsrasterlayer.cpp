/***************************************************************************
					qgsrasterlayer.cpp -  description
							 -------------------
	begin                : Sat Jun 22 2002
	copyright            : (C) 2002 by Gary E.Sherman
	email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#include <qimage.h>
#include <qpainter.h>
#include <stdio.h>

#include "qgsrect.h"
#include "qgsrasterlayer.h"
#include "gdal_priv.h"
#include "qtiffio.h"

QgsRasterLayer::QgsRasterLayer(QString path, QString baseName)
:QgsMapLayer(RASTER, baseName, path)
{
	std::cout << "QgsRasterLayer::QgsRasterLayer()" << std::endl;
	// test image handling using only Qt classes (don't use gdal)
	
	//~ GDALAllRegister();
	//~ gdalDataset = (GDALDataset *) GDALOpen( path, GA_ReadOnly );
	//~ std::cout << "Raster Count: " << gdalDataset->GetRasterCount() << std::endl;
	
}

QgsRasterLayer::~QgsRasterLayer()
{
}
void QgsRasterLayer::draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf)
{
	std::cout << "QgsRasterLayer::draw()" << std::endl;
	// init the tiff handler
	qInitTiffIO();
	std::cout << "Image source is " << source() << std::endl;
	QImage *image = new QImage(source());
	if(!image){
		qWarning("Failed to load the image using path stored in source()");
	}else{
		qWarning("Loaded image");
	}
	//~ for (int i = 1; i <= gdalDataset->GetRasterCount(); i++) {
		//~ GDALRasterBand  *gdalBand = gdalDataset->GetRasterBand( i );
		//~ int xBlk;
		//~ int yBlk;
		//~ gdalBand->GetBlockSize(&xBlk, &yBlk);
		//~ std::cout << "Block Size: " << xBlk << ", " << yBlk << std::endl;
		//~ std::cout << "Raster Data Type: " << gdalBand->GetRasterDataType() << std::endl
		//~ QImage image;
		//~ bool good = image.
		//~ /*
		//~ std::cout << "Raster X, Y: : " << gdalBand->GetXSize() << ", "
			//~ << gdalBand->GetYSize()  << std::endl;
		//~ int *pafScanline = (int *) CPLMalloc(nXSize);
		//~ gdalBand->RasterIO( GF_Read, 0, 0, nXSize, 1, pafScanline, nXSize, 1, GDT_UInt32, 0, 0 );
		
		//~ QImage image;
		//~ bool good = image.loadFromData(reinterpret_cast <const uchar*> (pafScanline), nXSize);
		//~ if(!good){
			//~ qWarning("Unable to load image data from GDAL scan line");
		//~ }
		//~ CPLFree(pafScanline);
		//~ */
		qWarning("Attempting to draw the image");
		p->drawImage(0, 0, *image);
		delete image;
//	}
}

//void QgsRasterLayer::identify(QgsRect * r)
//{
//}
