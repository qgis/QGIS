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
//#include "qtiffio.h"

QgsRasterLayer::QgsRasterLayer(QString path, QString baseName)
:QgsMapLayer(RASTER, baseName, path)
{
	std::cout << "QgsRasterLayer::QgsRasterLayer()" << std::endl;
	
	GDALAllRegister();
	gdalDataset = (GDALDataset *) GDALOpen( path, GA_ReadOnly );
	std::cout << "Raster Count: " << gdalDataset->GetRasterCount() << std::endl;
	
}

QgsRasterLayer::~QgsRasterLayer()
{
	GDALClose(gdalDataset);
}

void QgsRasterLayer::draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf)
{
	/*
	// init the tiff handler
	//qInitTiffIO();
	//std::cout << "Image source is " << source() << std::endl;
	//QImage *image = new QImage(source());
	//if(!image){
	//	qWarning("Failed to load the image using path stored in source()");
	//}else{
	//	qWarning("Loaded image");
	//}
	//
	//qWarning("Attempting to draw the image");
	//p->drawImage(0, 0, *image);
	//delete image;
	*/
	
	std::cout << "QgsRasterLayer::draw()" << std::endl;
	std::cout << "gdalDataset->GetRasterCount(): " << gdalDataset->GetRasterCount() << std::endl;
	
	// if there is more than one raster band they can be for red, green, blue, etc.
	// so this loop doesn't make much sense right now
	// only handling the case of 1 raster band that uses a palette
	// to index the rgb color values
	// this works for GeoTIFFs from:
	// http://cugir.mannlib.cornell.edu/browse_map/quad_map.html
	for (int i = 1; i <= gdalDataset->GetRasterCount(); i++) {
		GDALRasterBand  *gdalBand = gdalDataset->GetRasterBand( i );

		int nXSize = gdalBand->GetXSize();
		int nYSize = gdalBand->GetYSize();
		// read raster 1 line at a time
		// display image at 0,0
		for (int nYOff = 0; nYOff < nYSize; nYOff++) {
			uint *scanline = (uint*) CPLMalloc(sizeof(uint)*nXSize);
			CPLErr result = gdalBand->RasterIO( 
					GF_Read, 0, nYOff, nXSize, 1, scanline, nXSize, 1, GDT_UInt32, 0, 0 );
			
			GDALColorTable *colorTable = gdalBand->GetColorTable();
			
			// print each point in scanline using color looked up in color table
			for (int i = 0; i < nXSize; i++) {
				const GDALColorEntry *colorEntry = GDALGetColorEntry (colorTable, scanline[i]);
				p->setPen(QColor(colorEntry->c1, colorEntry->c2, colorEntry->c3));
				p->drawPoint(i, nYOff);
			}
			
			CPLFree(scanline);
		}
	}
}

//void QgsRasterLayer::identify(QgsRect * r)
//{
//}
