/***************************************************************************
    qgsdatetimeeditfactory.h
     --------------------------------------
    Date                 : 03.2014
    Copyright            : (C) 2014 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATETIMEEDITFACTORY_H
#define QGSDATETIMEEDITFACTORY_H

#include "qgis_gui.h"
#include "qgseditorwidgetfactory.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsDateTimeEditFactory
 * \brief Editor widget factory for datetime widgets.
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsDateTimeEditFactory : public QgsEditorWidgetFactory
{
  public:
    /**
     * Constructor for QgsDateTimeEditFactory, where \a name is a human-readable
     * name for the factory and \a icon provides a visual representation of this widget type.
     */
    QgsDateTimeEditFactory( const QString &name, const QIcon &icon = QIcon() );

    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    QgsSearchWidgetWrapper *createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    QHash<const char *, int> supportedWidgetTypes() override;
    unsigned int fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const override;
};

#endif // QGSDATETIMEEDITFACTORY_H
