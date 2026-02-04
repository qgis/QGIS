/***************************************************************************
                         qgsmodeldesignerconfigwidget.h
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


#ifndef QGSPROCESSINGMODELDESIGNERCONFIGWIDGET_H
#define QGSPROCESSINGMODELDESIGNERCONFIGWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgspanelwidget.h"

class QgsProcessingContext;
class QgsProcessingModelComponent;
class QgsProcessingParameterWidgetContext;

/**
 * \ingroup gui
 * \class QgsProcessingModelConfigWidget
 * \brief A panel widget that can be shown in the Processing model designer dialog for configuring part of the model.
 * \warning Not stable API
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsProcessingModelConfigWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsProcessingModelConfigWidget()
     */
    QgsProcessingModelConfigWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );
};

/**
 * \ingroup gui
 * \class QgsProcessingModelConfigWidgetFactory
 * \brief Factory class for creating panel widgets that can be shown in the Processing model designer dialog.
 * \warning Not stable API
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsProcessingModelConfigWidgetFactory : public QObject
{
    // note -- this is a QObject so we can safely store in QPointers!
    Q_OBJECT

  public:
    QgsProcessingModelConfigWidgetFactory();

    /**
     * Check if a model \a component is supported for this widget.
     */
    virtual bool supportsComponent( QgsProcessingModelComponent *component ) const = 0;

    /**
     * Factory function to create the widget on demand as needed by the dock.
     * \param component model component to create widget for
     * \param context processing context
     * \param widgetContext processing widget context
     */
    virtual QgsProcessingModelConfigWidget *createWidget( QgsProcessingModelComponent *component, QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext ) const = 0 SIP_FACTORY;
};


#endif // QGSPROCESSINGMODELDESIGNERCONFIGWIDGET_H
