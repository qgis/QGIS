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
#include "moc_qgsstackedwidget.cpp"

#include <QStackedWidget>
#include <QSize>


QgsStackedWidget::QgsStackedWidget( QWidget *parent )
  : QStackedWidget( parent )
  , mSizeMode( SizeMode::ConsiderAllPages ) //#spellok
{
}

QSize QgsStackedWidget::sizeHint() const
{
  switch ( mSizeMode )
  {
    case SizeMode::ConsiderAllPages: //#spellok
      return QStackedWidget::sizeHint();
    case SizeMode::CurrentPageOnly:
      return currentWidget() ? currentWidget()->sizeHint() : QSize();
  }
  return QSize();
}

QSize QgsStackedWidget::minimumSizeHint() const
{
  switch ( mSizeMode )
  {
    case SizeMode::ConsiderAllPages: //#spellok
      return QStackedWidget::sizeHint();
    case SizeMode::CurrentPageOnly:
      return currentWidget() ? currentWidget()->minimumSizeHint() : QSize();
  }
  return QSize();
}
