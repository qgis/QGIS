/***************************************************************************
                          qgsmapcanvas.cpp  -  description
                             -------------------
    begin                : Sun Jun 30 2002
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
#include <qstring.h>
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"

QgsMapCanvas::QgsMapCanvas(QWidget *parent, const char *name ) : QWidget(parent,name) {
}
QgsMapCanvas::~QgsMapCanvas(){
}
