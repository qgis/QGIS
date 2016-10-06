/***************************************************************************
    qgstexteditsearchwidgetwrapper.h
     -------------------------------
    Date                 : 2016-05-23
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTEDITSEARCHWIDGETWRAPPER_H
#define QGSTEXTEDITSEARCHWIDGETWRAPPER_H

#include "qgsdefaultsearchwidgetwrapper.h"

class QgsTextEditWidgetFactory;

/** \ingroup gui
 * \class QgsTextEditSearchWidgetWrapper
 * Wraps a text edit widget for searching.
 * \note Added in version 2.16
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsTextEditSearchWidgetWrapper : public QgsDefaultSearchWidgetWrapper
{
    Q_OBJECT

  public:

    /** Constructor for QgsTextEditSearchWidgetWrapper.
     * @param vl associated vector layer
     * @param fieldIdx index of associated field
     * @param parent parent widget
     */
    explicit QgsTextEditSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent = nullptr );

    bool applyDirectly() override;

  private:

    friend class QgsTextEditWidgetFactory;
};

#endif // QGSTEXTEDITSEARCHWIDGETWRAPPER_H
