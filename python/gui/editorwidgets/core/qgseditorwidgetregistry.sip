/***************************************************************************
    qgseditorwidgetregistry.sip
     --------------------------------------
    Date                 : 21.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * This class manages all known edit widget factories
 */
class QgsEditorWidgetRegistry : QObject
{
%TypeHeaderCode
#include <qgseditorwidgetregistry.h>
%End

  public:
    static QgsEditorWidgetRegistry* instance();
    void registerWidget( const QString& widgetType, QgsEditorWidgetFactory* widgetFactory /Transfer/ );
    QgsEditorWidgetWrapper* create( const QString& widgetType, QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, QWidget* editor = NULL, QWidget* parent = NULL ) /Factory/;
    QgsEditorConfigWidget* createConfigWidget( const QString& widgetId, QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) /Factory/;

    const QMap<QString, QgsEditorWidgetFactory*> factories();
};
