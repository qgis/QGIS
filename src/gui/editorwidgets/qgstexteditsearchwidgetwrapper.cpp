/***************************************************************************
    qgstexteditsearchwidgetwrapper.cpp
     ---------------------------------
    Date                 : 2016-05-23
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstexteditsearchwidgetwrapper.h"

#include "qgsfield.h"
#include "qgsvectorlayer.h"

QgsTextEditSearchWidgetWrapper::QgsTextEditSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsDefaultSearchWidgetWrapper( vl, fieldIdx, parent )
{
}

bool QgsTextEditSearchWidgetWrapper::applyDirectly()
{
  return true;
}
