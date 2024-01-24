/***************************************************************************
    qgsstackedwidget.cpp
    --------------------
    begin                : January 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstackedwidget.h"

#include <QStackedWidget>
#include <QSize>


QgsStackedWidget::QgsStackedWidget( QWidget *parent )
  : QStackedWidget( parent )
{
}

QSize QgsStackedWidget::sizeHint() const
{
  return currentWidget() ? currentWidget()->sizeHint() : QSize();
}

QSize QgsStackedWidget::minimumSizeHint() const
{
  return currentWidget() ? currentWidget()->minimumSizeHint() : QSize();
}
