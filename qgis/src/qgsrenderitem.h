/***************************************************************************
                         qgsrenderer.h  -  description
                             -------------------
    begin                : Sat Jan 4 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman @ mrcc.com
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

class QgsRenderItem {
private:
	QgsSymbol sym;
	QString value;
	QString label;
public:
QgsRenderItem();
	QgsRenderItem(QgsSymbol symbol, QString _value, QString _label);
	QgsSymbol *getSymbol();
	void setSymbol(QgsSymbol s);
	
};

#endif // QGSRENDERITEM_H
