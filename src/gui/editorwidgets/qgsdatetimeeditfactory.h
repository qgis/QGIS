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

#include "qgseditorwidgetfactory.h"

class GUI_EXPORT QgsDateTimeEditFactory : public QgsEditorWidgetFactory
{
  public:
    QgsDateTimeEditFactory( QString name );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const;
    QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const;
    QgsEditorWidgetConfig readConfig( const QDomElement &configElement, QgsVectorLayer *layer, int fieldIdx );
    void writeConfig( const QgsEditorWidgetConfig &config, QDomElement &configElement, const QDomDocument &doc, const QgsVectorLayer *layer, int fieldIdx );
};

#endif // QGSDATETIMEEDITFACTORY_H
