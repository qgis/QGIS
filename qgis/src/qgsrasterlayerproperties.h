/***************************************************************************
                          qgsrasterlayerproperties.h  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2002 by Tim Sutton
    email                : tim@linfiniti.com
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
#ifndef QGSRASTERLAYERPROPERTIES_H
#define QGSRASTERLAYERPROPERTIES_H
#include "qgsrasterlayerpropertiesbase.h"
#include "qgsmaplayer.h"


/**Property sheet for a raster map layer
  *@author Tim Sutton
  */

class QgsRasterLayerProperties : public QgsRasterLayerPropertiesBase  {
public:
/*! Constructor
* @param ml Map layer for which properties will be displayed
*/
	QgsRasterLayerProperties(QgsMapLayer *lyr);
	~QgsRasterLayerProperties();

private:
};

#endif
