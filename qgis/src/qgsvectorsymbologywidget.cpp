/***************************************************************************
                                 qgsvectorsymbologywidget.cpp
                               ------------------
        begin                : March 2005
        copyright            : (C) 2005 by Tim Sutton
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
#include "qgsvectorsymbologywidget.h"

#include <qgssymbol.h>

QgsVectorSymbologyWidget::QgsVectorSymbologyWidget(  QWidget *parent, const char * name, WFlags f)
           :QgsVectorSymbologyWidgetBase( parent, name, f)
{
}

QgsVectorSymbologyWidget::~QgsVectorSymbologyWidget()
{
}
