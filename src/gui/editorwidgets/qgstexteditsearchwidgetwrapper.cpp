/***************************************************************************
    qgstexteditsearchwidgetwrapper.cpp
     ---------------------------------
    Date                 : 2016-05-23
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgstexteditsearchwidgetwrapper.h"

#include "qgsfields.h"
#include "qgsvectorlayer.h"

QgsTextEditSearchWidgetWrapper::QgsTextEditSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsDefaultSearchWidgetWrapper( vl, fieldIdx, parent )
{
}

bool QgsTextEditSearchWidgetWrapper::applyDirectly()
{
  return true;
}
