/***************************************************************************
                             qgslayoutpagepropertieswidget.cpp
                             ---------------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutpagepropertieswidget.h"
#include "qgslayoutitempage.h"

QgsLayoutPagePropertiesWidget::QgsLayoutPagePropertiesWidget( QWidget *parent, QgsLayoutItem *layoutItem )
  : QgsLayoutItemBaseWidget( parent, layoutItem )
  , mPage( static_cast< QgsLayoutItemPage *>( layoutItem ) )
{
  setupUi( this );

}
