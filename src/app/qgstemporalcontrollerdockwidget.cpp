/***************************************************************************
                         qgstemporalcontrollerdockwidget.cpp
                         ------------------------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstemporalcontrollerdockwidget.h"
#include "qgstemporalcontrollerwidget.h"
#include "qgspanelwidgetstack.h"

QgsTemporalControllerDockWidget::QgsTemporalControllerDockWidget( const QString &name, QWidget *parent )
  : QgsDockWidget( parent )
{
  setWindowTitle( name );
  mControllerWidget = new QgsTemporalControllerWidget();
  mControllerWidget->setDockMode( true );

  QgsPanelWidgetStack *stack = new QgsPanelWidgetStack();
  stack->setMainPanel( mControllerWidget );
  setWidget( stack );
}

QgsTemporalController *QgsTemporalControllerDockWidget::temporalController()
{
  return mControllerWidget->temporalController();
}
