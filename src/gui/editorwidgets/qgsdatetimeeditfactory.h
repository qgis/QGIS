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

#define QGSDATETIMEEDIT_DATEFORMAT "yyyy-MM-dd"
#define QGSDATETIMEEDIT_TIMEFORMAT "HH:mm:ss"
#define QGSDATETIMEEDIT_DATETIMEFORMAT "yyyy-MM-dd HH:mm:ss"

class GUI_EXPORT QgsDateTimeEditFactory : public QgsEditorWidgetFactory
{
  public:
    QgsDateTimeEditFactory( QString name );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper* create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    QgsEditorConfigWidget* configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    QgsEditorWidgetConfig readConfig( const QDomElement &configElement, QgsVectorLayer *layer, int fieldIdx ) override;
    void writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx ) override;
    QString representValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const override;
    virtual QMap<const char*, int> supportedWidgetTypes() override;
};

#endif // QGSDATETIMEEDITFACTORY_H
