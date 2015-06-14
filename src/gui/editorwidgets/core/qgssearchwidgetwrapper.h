/***************************************************************************
    qgssearchwidgetwrapper.h
     --------------------------------------
    Date                 : 31.5.2015
    Copyright            : (C) 2015 Karolina Alexiou (carolinux)
    Email                : carolinegr at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSEARCHWIDGETWRAPPER_H
#define QGSSEARCHWIDGETWRAPPER_H

#include <QObject>
#include <QMap>
#include <QVariant>

class QgsVectorLayer;
class QgsField;

#include "qgseditorwidgetconfig.h"
#include "qgsattributeeditorcontext.h"
#include "qgswidgetwrapper.h"

/**
 * Manages an editor widget
 * Widget and wrapper share the same parent
 *
 * A wrapper controls one attribute editor widget and is able to create a default
 * widget or use a pre-existent widget. It is able to set the widget to the value implied
 * by a field of a vector layer, or return the value it currently holds. Every time it is changed
 * it has to emit a valueChanged signal. If it fails to do so, there is no guarantee that the
 * changed status of the widget will be saved.
 *
 */
class GUI_EXPORT QgsSearchWidgetWrapper : public QgsWidgetWrapper
{
    Q_OBJECT
  public:
    /**
     * Create a new widget wrapper
     *
     * @param vl        The layer on which the field is
     * @param fieldIdx  The field which will be controlled
     * @param parent    A parent widget for this widget wrapper and the created widget.
     */
    explicit QgsSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent = 0 );

    /**
     * Will be used to access the widget's value. Read the value from the widget and
     * return it properly formatted to be saved in the attribute.
     *
     * If an invalid variant is returned this will be interpreted as no change.
     * Be sure to return a NULL QVariant if it should be set to NULL.
     *
     * @return The current value the widget represents
     */
    virtual QString expression() = 0;
    /**
     * If this is true, then this search widget should take effect directly
     * when its expression changes
     */
    virtual bool applyDirectly() = 0;

  signals:

   void expressionChanged(QString exp);

  protected slots:

    virtual void setExpression(QString value) = 0;
    void setFeature( const QgsFeature& feature ) override;

  protected:
    QString mExpression;
    int mFieldIdx;

};
// We'll use this class inside a QVariant in the widgets properties
Q_DECLARE_METATYPE( QgsSearchWidgetWrapper* )

#endif // QGSSEARCHWIDGETWRAPPER_H
