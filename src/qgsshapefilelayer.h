/***************************************************************************
                          gsshapefilelayer.h  -  description
                             -------------------
    begin                : Wed Jun 26 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman@mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSHAPEFILELAYER_H
#define QGSSHAPEFILELAYER_H
class QPainter;
class QgsRect;
class QgsCoordinateTransform;
class OGRLayer;
class OGRDataSource;

#include "qgsmaplayer.h"

/*! \class QgsShapeFileLayer
 * \brief Shapefile layer
 */

class QgsShapeFileLayer : public QgsMapLayer  {
public: 
    //! Constructor
	QgsShapeFileLayer(QString path=0, QString baseName=0);
	//! Destructor
	~QgsShapeFileLayer();
       
enum SHAPETYPE {
	Point,
	Line,
	Polygon
};

private: // Private attributes
void draw(QPainter *p, QgsRect *viewExtent, QgsCoordinateTransform *cXf);
OGRDataSource *ogrDataSource;
	
OGRLayer *ogrLayer; 
  /**  */
  bool registered;
     enum ENDIAN{
	NDR=1,
	XDR=0
    };
    enum WKBTYPE{
	WKBPoint=1,
	WKBLineString,
	WKBPolygon,
	WKBMultiPoint,
	WKBMultiLineString,
	WKBMultiPolygon
    };	
private: // Private methods
  /** No descriptions */
  void registerFormats();
  int endian();
  
};

#endif
