/***************************************************************************
						  gsrasterlayer.h  -  description
							 -------------------
	begin                : Fri Jun 28 2002
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

#ifndef QGSRASTERLAYER_H
#define QGSRASTERLAYER_H

class QgsRect;
class GDALDataset;

#include "qgsmaplayer.h"
/*! \class QgsRasterLayer
 * \brief Raster layer class
 */

class QgsRasterLayer : public QgsMapLayer  {
	Q_OBJECT
public: 
	//! Constructor
	QgsRasterLayer(QString path = 0, QString baseName = 0);
	//! Destructor
	~QgsRasterLayer();
	void draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf);
	//void identify(QgsRect *r);
	
private:
	GDALDataset  *gdalDataset;
	// values for mapping pixel to world coordinates
	double adfGeoTransform[6];
	
signals:
	void repaintRequested();
};

#endif
