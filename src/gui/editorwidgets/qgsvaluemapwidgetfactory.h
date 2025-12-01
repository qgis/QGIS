/***************************************************************************
    qgsvaluemapwidgetfactory.h
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVALUEMAPWIDGETFACTORY_H
#define QGSVALUEMAPWIDGETFACTORY_H

#include "qgis_gui.h"
#include "qgseditorwidgetfactory.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsValueMapWidgetFactory
 * \brief Editor widget factory for value map widgets.
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsValueMapWidgetFactory : public QgsEditorWidgetFactory
{
  public:
    /**
     * Constructor for QgsValueMapWidgetFactory, where \a name is a human-readable
     * name for the factory and \a icon provides a visual representation of this widget type.
     */
    QgsValueMapWidgetFactory( const QString &name, const QIcon &icon = QIcon() );

    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    QgsSearchWidgetWrapper *createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    QHash<const char *, int> supportedWidgetTypes() override;
};

#endif // QGSVALUEMAPWIDGETFACTORY_H
