/***************************************************************************
    qgsabstractdatasourcewidget.cpp  -  base class for source selector widgets
                             -------------------
    begin                : 10 July 2017
    original             : (C) 2017 by Alessandro Pasotti
    email                : apasotti at boundlessgeo dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractdatasourcewidget.h"

QgsAbstractDataSourceWidget::QgsAbstractDataSourceWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ):
  QDialog( parent, fl ),
  mWidgetMode( widgetMode )
{

}

QgsProviderRegistry::WidgetMode QgsAbstractDataSourceWidget::widgetMode() const
{
  return mWidgetMode;
}

const QgsMapCanvas *QgsAbstractDataSourceWidget::mapCanvas() const
{
  return mMapCanvas;
}


void QgsAbstractDataSourceWidget::setMapCanvas( const QgsMapCanvas *mapCanvas )
{
  mMapCanvas = mapCanvas;
}
