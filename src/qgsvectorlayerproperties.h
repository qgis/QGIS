/***************************************************************************
                          qgsvectorlayerproperties.h  -  description
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
/* $Id$ */
#ifndef QGSVECTORLAYERPROPERTIES_H
#define QGSVECTORLAYERPROPERTIES_H
class QgsVectorLayer;
#include "qgssymbol.h"
class QString;
#include "qgsvectorlayerpropertiesbase.h"


/**Property sheet for a map layer
  *@author Gary E.Sherman
  */

class QgsVectorLayerProperties : public QgsVectorLayerPropertiesBase  {

Q_OBJECT

public:
/*! Constructor
* @param ml Map layer for which properties will be displayed
*/
    QgsVectorLayerProperties(QgsVectorLayer* ml);
	~QgsVectorLayerProperties();
	//! Name to display in legend
	QgsSymbol* getSymbol();
private:
	QgsVectorLayer* layer;
protected slots:
    void showSymbolSettings();
    void alterLayerDialog(const QString& string); 
};

#endif
