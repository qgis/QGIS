/***************************************************************************
                          qgslayerproperties.h  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
        Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERPROPERTIES_H
#define QGSLAYERPROPERTIES_H
class QgsMapLayer;
class QgsSymbol;
#include "qgslayerpropertiesbase.h"


/**Property sheet for a map layer
  *@author Gary E.Sherman
  */

class QgsLayerProperties : public QgsLayerPropertiesBase  {
public:
/*! Constructor
* @param ml Map layer for which properties will be displayed
*/
	QgsLayerProperties(QgsMapLayer *ml);
	~QgsLayerProperties();
	//! Function to display the color selector and choose the fill color
	void selectFillColor();
	//! Function to display the color selector and choose the outline color
	void selectOutlineColor();
private:
	QgsMapLayer *layer;
	QgsSymbol *sym;
};

#endif
