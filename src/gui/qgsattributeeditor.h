/***************************************************************************
                         qgsattributeeditor.h  -  description
                             -------------------
    begin                : July 2009
    copyright            : (C) 2009 by Jürgen E. Fischer
    email                : jef@norbit.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEEDITOR_H
#define QGSATTRIBUTEEDITOR_H

#include <QVariant>
#include <QMetaType>
#include <QGridLayout>

#include "qgsfeature.h"
#include "qgsrelationmanager.h"

class QgsAttributeEditorContext;
class QgsAttributeEditorElement;
class QgsDualView;
class QgsRelationManager;
class QgsVectorLayer;

/** \ingroup gui
 * \brief create attribute widget for editing
 *
 * @deprecated
 */
// TODO QGIS 3.0 - remove
class GUI_EXPORT QgsAttributeEditor : public QObject
{
    Q_OBJECT

  public:
    QgsAttributeEditor( QObject* parent, QgsVectorLayer* vl = nullptr, int idx = -1 )
        : QObject( parent )
    {
      Q_UNUSED( vl )
      Q_UNUSED( idx )
    }
    /**
     * Creates or prepares a attribute editor widget
     * @param parent The parent object
     * @param editor The widget to prepare. Set to null if it should be generated
     * @param vl The vector layer to use as data source
     * @param idx The field index this widget refers to
     * @param value the value to initiate this widget with
     * @param proxyWidgets an array of widgets, which will act as a value proxy if the same field is inserted multiple times
     *
     * @deprecated
     */
    static Q_DECL_DEPRECATED QWidget* createAttributeEditor( QWidget* parent, QWidget* editor, QgsVectorLayer* vl, int idx, const QVariant &value, QMap<int, QWidget*>& proxyWidgets );

    /**
     * Creates or prepares a attribute editor widget
     * @param parent The parent object
     * @param editor The widget to prepare. Set to null if it should be generated
     * @param vl The vector layer to use as data source
     * @param idx The field index this widget refers to
     * @param value the value to initiate this widget with
     *
     */
    static QWidget* createAttributeEditor( QWidget* parent, QWidget* editor, QgsVectorLayer* vl, int idx, const QVariant& value );
    /**
     * Creates or prepares a attribute editor widget
     * @param parent The parent object
     * @param editor The widget to prepare. Set to null if it should be generated
     * @param vl The vector layer to use as data source
     * @param idx The field index this widget refers to
     * @param value the value to initiate this widget with
     * @param context the context used for the created attribute editor
     *
     */
    static QWidget* createAttributeEditor( QWidget* parent, QWidget* editor, QgsVectorLayer* vl, int idx, const QVariant& value, QgsAttributeEditorContext& context );

    static bool retrieveValue( QWidget *widget, QgsVectorLayer *vl, int idx, QVariant &value );
    static bool setValue( QWidget *widget, QgsVectorLayer *vl, int idx, const QVariant &value );
};

#endif
