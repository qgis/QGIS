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


#include "qgsmaplayer.h"

/*! \class QgsShapeFileLayer
 * \brief Shapefile layer
 */

class QgsShapeFileLayer : public QgsMapLayer  {
public: 
    //! Constructor
	QgsShapeFileLayer();
	//! Destructor
	~QgsShapeFileLayer();
       
enum SHAPETYPE {
	Point,
	Line,
	Polygon
};

private: // Private attributes
  /**  */
  bool registered;
private: // Private methods
  /** No descriptions */
  void registerFormats();
};

#endif
