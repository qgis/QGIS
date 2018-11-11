/***************************************************************************
    qgsbinarywidgetfactory.h
     -----------------------
    Date                 : November 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBINARYWIDGETFACTORY_H
#define QGSBINARYWIDGETFACTORY_H

#include "qgseditorwidgetfactory.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsBinaryWidgetFactory
 * Editor widget factory for binary (BLOB) widgets.
 * \note not available in Python bindings
 * \since QGIS 3.6
 */

class GUI_EXPORT QgsBinaryWidgetFactory : public QgsEditorWidgetFactory
{
  public:
    explicit QgsBinaryWidgetFactory( const QString &name );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;

    unsigned int fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const override;
};

#endif // QGSBINARYWIDGETFACTORY_H
