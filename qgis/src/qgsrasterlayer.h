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

#include <qvaluevector.h>

#include "qgsmaplayer.h"

class QgsRect;
class GDALDataset;



struct RasterBandStats {
  double minValDouble;
  double maxValDouble;
  //the distance between min & max
  double rangeDouble;
  double meanDouble;
  double sumSqrDevDouble; //used to calculate stddev
  double stdDevDouble;
  double sumDouble;
  int elementCountInt;
  double noDataDouble;
};
typedef QMap<QString, RasterBandStats> RasterStatsMap;

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
	
        //
        // Accessor and mutator for showGrayAsColorFlag
        //
        bool getShowGrayAsColorFlag() { return showGrayAsColorFlag;}
        void setShowGrayAsColorFlag(bool theFlag) {showGrayAsColorFlag=theFlag;}
        //
        // Accessor and mutator for invertHistogramFlag
        //
        bool getInvertHistogramFlag() { return invertHistogramFlag; }
        void setInvertHistogramFlag(bool theFlag) { invertHistogramFlag=theFlag; }
                
        // Get RasterBandStats for a band given its numer (read only)
        const  RasterBandStats getRasterBandStats(int);
        // Overloaded method that also returns stats for a band, but uses the band color name 
        const  RasterBandStats getRasterBandStats(QString theBandName);
private:
        //private method to calculate various stats about this layer
        void calculateStats();
        
	GDALDataset  *gdalDataset;
	// values for mapping pixel to world coordinates
	double adfGeoTransform[6];
	// flag indicating whether grayscale images should be rendered as pseudocolor
        bool showGrayAsColorFlag;
        // flag indicating whether the histogram should be inverted or not
        bool invertHistogramFlag;
        // Number of stddev to plot (0) to ignore
        double stdDevsToPlotDouble;
        // a collection of stats - one for each band in the layer 
        // the map key corresonds to the gdal GetColorInterpretation for that band
        RasterStatsMap rasterStatsMap;
signals:
	void repaintRequested();
};

#endif
