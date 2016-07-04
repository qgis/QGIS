/***************************************************************************
    qgseditorconfigwidget.h
     --------------------------------------
    Date                 : 24.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEDITORCONFIGWIDGET_H
#define QGSEDITORCONFIGWIDGET_H

#include <QWidget>

#include "qgseditorwidgetwrapper.h"

class QgsVectorLayer;

/** \ingroup gui
 * This class should be subclassed for every configurable editor widget type.
 *
 * It implements the GUI configuration widget and transforms this to/from a configuration.
 *
 * It will only be instantiated by {@see QgsEditorWidgetFactory}
 */

class GUI_EXPORT QgsEditorConfigWidget : public QWidget
{
    Q_OBJECT
  public:
    /**
     * Create a new configuration widget
     *
     * @param vl       The layer for which the configuration dialog will be created
     * @param fieldIdx The index of the field on the layer for which this dialog will be created
     * @param parent   A parent widget
     */
    explicit QgsEditorConfigWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent );

    /**
     * @brief Create a configuration from the current GUI state
     *
     * @return A widget configuration
     */
    virtual QgsEditorWidgetConfig config() = 0;

    /**
     * @brief Update the configuration widget to represent the given configuration.
     *
     * @param config The configuration which should be represented by this widget
     */
    virtual void setConfig( const QgsEditorWidgetConfig& config ) = 0;

    /**
     * Returns the field for which this configuration widget applies
     *
     * @return The field index
     */
    int field();

    /**
     * Returns the layer for which this configuration widget applies
     *
     * @return The layer
     */
    QgsVectorLayer* layer();

    /**
     * Destructor
     */
    virtual ~QgsEditorConfigWidget() {}

  private:
    QgsVectorLayer* mLayer;
    int mField;
};

#endif // QGSEDITORCONFIGWIDGET_H
