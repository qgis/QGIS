/***************************************************************************
                         qgsrenderer.h  -  description
                             -------------------
    begin                : Sat Jan 4 2003
    copyright            : (C) 2003 by Gary E.Sherman
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
#ifndef QGSRENDERITEM_H
#define QGSRENDERITEM_H
#include <map>
#include "qgssymbol.h"
class QString;
/*! \class QgsRenderItem
* \brief A render item (also known as a class) that represents how to
* render a feature of type "value".
*/
class QgsRenderItem {
private:
//! Symbol to use in rendering the class
	QgsSymbol sym;
	//! Value of the field 
	QString value;
	//! Label to use when rendering (may be same as value of field)
	QString label;
public:
//! Default Constructor
QgsRenderItem();
/*! Constructor
* @param symbol Symbol to use for rendering matching features
* @param _value Value of the field
* @param _label Label to use in the legend
*/
	QgsRenderItem(QgsSymbol symbol, QString _value, QString _label);
	/*! Gets the symbol associated with this render item
	* @return QgsSymbol pointer
	*/
	QgsSymbol *getSymbol();
	/*! Sets the symbol associated with this render item
	* @param s Symbol 
	*/
	void setSymbol(QgsSymbol s);
	
};

#endif // QGSRENDERITEM_H
