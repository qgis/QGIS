/***************************************************************************
    qgslistwidgetfactory.h
     --------------------------------------
    Date                 : 09.2016
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

#ifndef QGSLISTWIDGETFACTORY_H
#define QGSLISTWIDGETFACTORY_H

#include "qgseditorwidgetfactory.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * Factory for widgets for editing a QVariantList or a QStringList
 * \since QGIS 3.0
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsListWidgetFactory : public QgsEditorWidgetFactory
{
  public:

    /**
     * Constructor.
     */
    QgsListWidgetFactory( const QString &name );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    //QgsSearchWidgetWrapper* createSearchWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const override;
    QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    unsigned int fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const override;
};

#endif // QGSLISTWIDGETFACTORY_H
