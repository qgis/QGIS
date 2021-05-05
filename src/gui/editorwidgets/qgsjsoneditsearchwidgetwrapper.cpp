/***************************************************************************
    qgsjsoneditsearchwidgetwrapper.cpp
     ---------------------------------
    Date                 : 3.5.2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsjsoneditsearchwidgetwrapper.h"

#include "qgsfields.h"
#include "qgsvectorlayer.h"

QgsJsonEditSearchWidgetWrapper::QgsJsonEditSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsDefaultSearchWidgetWrapper( vl, fieldIdx, parent )
{
}

bool QgsJsonEditSearchWidgetWrapper::applyDirectly()
{
  return true;
}
