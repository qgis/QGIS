/***************************************************************************
    qgskeyvaluewidgetfactory.h
     --------------------------------------
    Date                 : 08.2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick.valsecchi@camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSKEYVALUEEDITFACTORY_H
#define QGSKEYVALUEEDITFACTORY_H

#include "qgseditorwidgetfactory.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \brief Factory for widgets for editing a QVariantMap
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsKeyValueWidgetFactory : public QgsEditorWidgetFactory
{
  public:
    /**
     * Constructor for QgsKeyValueWidgetFactory, where \a name is a human-readable
     * name for the factory.
     */
    QgsKeyValueWidgetFactory( const QString &name );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    //QgsSearchWidgetWrapper* createSearchWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const override;
    QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    unsigned int fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const override;
};

#endif // QGSKEYVALUEEDITFACTORY_H
