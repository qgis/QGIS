/***************************************************************************
                         qgsrenderer.cpp  -  description
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

#include <qstring.h>
#include "qgssymbol.h"

#include "qgsrenderitem.h"

QgsRenderItem::QgsRenderItem(){
};

QgsRenderItem::QgsRenderItem(QgsSymbol symbol, QString _value, QString _label) :
sym(symbol), value(_value), label(_label){

}
QgsSymbol *QgsRenderItem::getSymbol(){
	return &sym;
}
void QgsRenderItem::setSymbol(QgsSymbol s){
	sym = s;
}
