/***************************************************************************
                         qgsrenderer.cpp  -  description
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
#include <qstring.h>
#include "qgsrenderitem.h"
#include "qgsrenderer.h"

QgsRenderer::QgsRenderer(int _type, QString _field) : type(_type), field(_field) {
}

void QgsRenderer::addItem(QString key, QgsRenderItem ri){
items[key] = ri;
}
	



