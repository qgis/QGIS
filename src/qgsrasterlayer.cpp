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

#include "qgspoint.h"
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
	
	double        adfGeoTransform[6];
	if( gdalDataset->GetGeoTransform( adfGeoTransform ) == CE_None )
	{
		printf( "Origin = (%.6f,%.6f)\n",
				adfGeoTransform[0], adfGeoTransform[3] );
		
		printf( "Pixel Size = (%.6f,%.6f)\n",
				adfGeoTransform[1], adfGeoTransform[5] );
	}
	
	const char *projRef = gdalDataset->GetProjectionRef();
	
	double XMax = adfGeoTransform[0] + gdalDataset->GetRasterXSize() * adfGeoTransform[1] +  
				  gdalDataset->GetRasterYSize() * adfGeoTransform[2];
	double YMin  = adfGeoTransform[3] + gdalDataset->GetRasterXSize() * adfGeoTransform[4] + 
				   gdalDataset->GetRasterYSize() * adfGeoTransform[5];
	
	layerExtent.setXmax(XMax);
	layerExtent.setXmin(adfGeoTransform[0]);
	layerExtent.setYmax(adfGeoTransform[3]);
	layerExtent.setYmin(YMin);
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
	std::cout << "Layer extent: " << layerExtent.stringRep() << std::endl;
	
	// get dimensions of raster image in map coordinate space
	QgsPoint topLeft = cXf->transform(layerExtent.xMin(), layerExtent.yMax());
	QgsPoint bottomRight = cXf->transform(layerExtent.xMax(), layerExtent.yMin());
	//QPoint topLeft = cXf->transform(topLeft);
	//QPoint bottomRight = layerExtent.bottomRight();
	//bottomRight = cXf->transform(bottomRight);
	int lXSize = bottomRight.xToInt() - topLeft.xToInt();
	int lYSize = bottomRight.yToInt() - topLeft.yToInt();
	std::cout << "xMin: " << layerExtent.xMin() <<std::endl;
	std::cout << "yMax: " << layerExtent.yMax() <<std::endl;
	std::cout << "xMax: " << layerExtent.xMax() <<std::endl;
	std::cout << "yMin: " << layerExtent.yMin() <<std::endl;
	std::cout << "lXSize: " << lXSize <<std::endl;
	std::cout << "lYSize: " << lYSize <<std::endl;
	
	// if there is more than one raster band they can be for red, green, blue, etc.
	// so this loop doesn't make much sense right now
	// only handling the case of 1 raster band that uses a palette
	// to index the rgb color values
	// this works for GeoTIFFs from:
	// http://cugir.mannlib.cornell.edu/browse_map/quad_map.html
	for (int i = 1; i <= gdalDataset->GetRasterCount(); i++) {
		GDALRasterBand  *gdalBand = gdalDataset->GetRasterBand( i );
		
		std::cout << "gdalBand->GetOverviewCount(): " << gdalBand->GetOverviewCount() <<std::endl;
		
		int nXSize = gdalBand->GetXSize();
		int nYSize = gdalBand->GetYSize();
		// read entire raster
		// treat scandata as a pseudo-multidimensional array
		// RasterIO() takes care of scaling down image
		// TODO: need to clip to map size or we run out of memory when zooming in
		uint *scandata = (uint*) CPLMalloc(sizeof(uint)*lXSize * sizeof(uint)*lYSize);
		CPLErr result = gdalBand->RasterIO( 
				GF_Read, 0, 0, nXSize, nYSize, scandata, lXSize, lYSize, GDT_UInt32, 0, 0 );
			
		GDALColorTable *colorTable = gdalBand->GetColorTable();
			
		// print each point in scandata using color looked up in color table
		for (int i = 0; i < lXSize; i++) {
			for (int j =0; j < lYSize; j++) {
				const GDALColorEntry *colorEntry = GDALGetColorEntry(colorTable, scandata[i*lXSize + j]);
				p->setPen(QColor(colorEntry->c1, colorEntry->c2, colorEntry->c3));
				p->drawPoint(topLeft.xToInt() + j, topLeft.yToInt() + i);
			}
		}
			
		CPLFree(scandata);
	}
}

//void QgsRasterLayer::identify(QgsRect * r)
//{
//}
