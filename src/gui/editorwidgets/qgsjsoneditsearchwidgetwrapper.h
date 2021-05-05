/***************************************************************************
    qgsjsoneditsearchwidgetwrapper.h
     -------------------------------
    Date                 : 3.5.2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSJSONEDITSEARCHWIDGETWRAPPER_H
#define QGSJSONEDITSEARCHWIDGETWRAPPER_H

#include "qgsdefaultsearchwidgetwrapper.h"
#include "qgis_gui.h"

SIP_NO_FILE

class QgsJsonEditWidgetFactory;

/**
 * \ingroup gui
 * \class QgsJsonEditSearchWidgetWrapper
 * \brief Wraps a text edit widget for searching.
 * \note not available in Python bindings
 * \since QGIS 2.16
 */

class GUI_EXPORT QgsJsonEditSearchWidgetWrapper : public QgsDefaultSearchWidgetWrapper
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsJsonEditSearchWidgetWrapper.
     * \param vl associated vector layer
     * \param fieldIdx index of associated field
     * \param parent parent widget
     */
    explicit QgsJsonEditSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent = nullptr );

    bool applyDirectly() override;

  private:

    friend class QgsJsonEditWidgetFactory;
};

#endif // QGSJSONEDITSEARCHWIDGETWRAPPER_H
