/***************************************************************************
    qgsgeometrywidgetfactory.h
     -----------------------
    Date                 : February 2023
    Copyright            : (C) 2023 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYWIDGETFACTORY_H
#define QGSGEOMETRYWIDGETFACTORY_H

#include "qgseditorwidgetfactory.h"
#include "qgis_gui.h"

class QgsMessageBar;

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsGeometryWidgetFactory
 * \brief Editor widget factory for geometry widgets.
 * \note not available in Python bindings
 * \since QGIS 3.30
 */

class GUI_EXPORT QgsGeometryWidgetFactory : public QgsEditorWidgetFactory
{
  public:
    /**
     * Constructor for QgsGeometryWidgetFactory, where \a name is a human-readable
     * name for the factory.
     *
     * The \a messageBar argument can be used to link the widget to a QgsMessageBar
     * for providing user feedback.
     */
    explicit QgsGeometryWidgetFactory( const QString &name, QgsMessageBar *messageBar );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;

    unsigned int fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const override;

  private:
    QgsMessageBar *mMessageBar = nullptr;
};

#endif // QGSGEOMETRYWIDGETFACTORY_H
