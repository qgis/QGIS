/***************************************************************************
    qgsrelationreferencefactory.h
     --------------------------------------
    Date                 : 29.5.2013
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

#ifndef QGSRELATIONREFERENCEFACTORY_H
#define QGSRELATIONREFERENCEFACTORY_H

#include "qgsattributeeditorcontext.h"
#include "qgseditorwidgetfactory.h"
#include "qgis_gui.h"

SIP_NO_FILE

class QgsMapCanvas;
class QgsMessageBar;

/**
 * \ingroup gui
 * \class QgsRelationReferenceFactory
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsRelationReferenceFactory : public QgsEditorWidgetFactory
{
  public:
    QgsRelationReferenceFactory( const QString &name, QgsMapCanvas *canvas, QgsMessageBar *messageBar );

    /**
     * Override this in your implementation.
     * Create a new editor widget wrapper. Call QgsEditorWidgetRegistry::create()
     * instead of calling this method directly.
     *
     * \param vl       The vector layer on which this widget will act
     * \param fieldIdx The field index on which this widget will act
     * \param editor   An editor widget if already existent. If NULL is provided, a new widget will be created.
     * \param parent   The parent for the wrapper class and any created widget.
     *
     * \returns         A new widget wrapper
     */
    virtual QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;

    QgsSearchWidgetWrapper *createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;

    /**
     * Override this in your implementation.
     * Create a new configuration widget for this widget type.
     *
     * \param vl       The layer for which the widget will be created
     * \param fieldIdx The field index for which the widget will be created
     * \param parent   The parent widget of the created config widget
     *
     * \returns         A configuration widget
     */
    virtual QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;

    virtual QHash<const char *, int> supportedWidgetTypes() override;

    virtual unsigned int fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const override;

  private:
    QgsAttributeEditorContext mEditorContext;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
};

#endif // QGSRELATIONREFERENCEFACTORY_H
