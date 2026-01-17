/***************************************************************************
                         qgsmodeldesignerconfigdockwidget.h
                         ----------------------------------------
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


#ifndef QGSPROCESSINGMODELDESIGNERCONFIGDOCKWIDGET_H
#define QGSPROCESSINGMODELDESIGNERCONFIGDOCKWIDGET_H

#include "ui_qgsmodeldesignerconfigwidgetbase.h"

#include "qgis_gui.h"

#define SIP_NO_FILE

class QgsProcessingContext;
class QgsProcessingModelComponent;
class QgsProcessingParameterWidgetContext;

/**
 * A dockable panel widget stack which allows users to specify the properties of a Processing model component.
 *
 * A virtual equivalent of the QGIS app "layer styling dock", but for editing Processing model components.
 *
 * \ingroup gui
 * \note Not available in Python bindings.
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsModelDesignerConfigDockWidget : public QWidget, private Ui::QgsModelDesignerConfigWidgetBase
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsModelDesignerConfigDockWidget.
     */
    QgsModelDesignerConfigDockWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void showComponentConfig( QgsProcessingModelComponent *component, QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext );
};

#endif // QGSPROCESSINGMODELDESIGNERCONFIGDOCKWIDGET_H
