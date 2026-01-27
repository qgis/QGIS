/***************************************************************************
                         qgsmodeldesignerconfigdockwidget.cpp
                         ------------------------------------------
    begin                : January 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodeldesignerconfigdockwidget.h"

#include "qgsgui.h"
#include "qgsmodeldesignerconfigwidget.h"
#include "qgsprocessingguiregistry.h"

#include "moc_qgsmodeldesignerconfigdockwidget.cpp"

QgsModelDesignerConfigDockWidget::QgsModelDesignerConfigDockWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mStackedWidget->setCurrentWidget( mNoComponentPage );
}

void QgsModelDesignerConfigDockWidget::showComponentConfig( QgsProcessingModelComponent *component, QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext )
{
  delete mWidgetStack->takeMainPanel();

  QgsProcessingModelConfigWidget *widget = QgsGui::processingGuiRegistry()->createModelConfigWidgetForComponent( component, context, widgetContext );
  if ( widget )
  {
    widget->setDockMode( true );
    mStackedWidget->setCurrentWidget( mComponentConfigPage );
    mWidgetStack->setMainPanel( widget );
  }
  else
  {
    mStackedWidget->setCurrentWidget( mNoComponentPage );
  }
}
