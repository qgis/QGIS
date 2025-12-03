/***************************************************************************
    qgsjsoneditwidgetfactory.h
     --------------------------------------
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

#ifndef QGSJSONEDITWIDGETFACTORY_H
#define QGSJSONEDITWIDGETFACTORY_H

#include "qgis_gui.h"
#include "qgseditorwidgetfactory.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsJsonEditWidgetFactory
 * \brief Editor widget factory for JSON edit widgets.
 * \note not available in Python bindings
 * \since QGIS 3.20
 */

class GUI_EXPORT QgsJsonEditWidgetFactory : public QgsEditorWidgetFactory
{
  public:
    /**
     * Constructor for QgsJsonEditWidgetFactory, where \a name is a human-readable
     * name for the factory and \a icon provides a visual representation of this widget type.
     */
    QgsJsonEditWidgetFactory( const QString &name, const QIcon &icon = QIcon() );

    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;

    QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;

    unsigned int fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const override;

    bool isReadOnly() const override;
};

#endif // QGSJSONEDITWIDGETFACTORY_H
