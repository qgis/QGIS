/***************************************************************************
    qgssearchwidgetwrapper.cpp
     --------------------------------------
    Date                 : 10.6.2015
    Copyright            : (C) 2015 Karolina Alexiou
    Email                : carolinegr at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssearchwidgetwrapper.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfield.h"

#include <QWidget>

QgsSearchWidgetWrapper::QgsSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsWidgetWrapper( vl, 0, parent )
    , mExpression( QString() )
    , mFieldIdx( fieldIdx )
{
}


void QgsSearchWidgetWrapper::setFeature( const QgsFeature& feature )
{
  Q_UNUSED( feature )
}

