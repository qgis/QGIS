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
#include <qimage.h>
#include <qpainter.h>
#include <stdio.h>

#include "qgsrect.h"
#include "qgsrasterlayer.h"
#include "gdal_priv.h"
QgsRasterLayer::QgsRasterLayer(QString path, QString baseName)
:QgsMapLayer(RASTER, baseName, path)
{
	std::cout << "QgsRasterLayer::QgsRasterLayer()" << std::endl;
	
	GDALAllRegister();
	gdalDataset = (GDALDataset *) GDALOpen( path, GA_ReadOnly );
}

QgsRasterLayer::~QgsRasterLayer()
{
}
void QgsRasterLayer::draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf)
{
	std::cout << "QgsRasterLayer::draw()" << std::endl;
	
	for (int i = 1; i <= gdalDataset->GetRasterCount(); i++) {
		GDALRasterBand  *gdalBand = gdalDataset->GetRasterBand( i );
		
		int nXSize = gdalBand->GetXSize();
		int *pafScanline = (int *) CPLMalloc(nXSize);
		gdalBand->RasterIO( GF_Read, 0, 0, nXSize, 1, pafScanline, nXSize, 1, GDT_UInt32, 0, 0 );
		
		QImage image;
		image.loadFromData(reinterpret_cast <const uchar*> (pafScanline), nXSize);
		CPLFree(pafScanline);
		
		p->drawImage(0, i - 1, image);
	}
}

//void QgsRasterLayer::identify(QgsRect * r)
//{
//}
