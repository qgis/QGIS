/***************************************************************************
    qgseditorwidgetfactory.sip
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
 * Every attribute editor widget needs a factory, which inherits this class
 */
class QgsEditorWidgetFactory
{
%TypeHeaderCode
#include <qgseditorwidgetfactory.h>
%End

  public:
    QgsEditorWidgetFactory( const QString& name );
    virtual ~QgsEditorWidgetFactory();
    virtual QgsEditorWidgetWrapper* create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const = 0 /Factory/;
    virtual QgsEditorConfigWidget* configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const = 0 /Factory/;
    virtual QgsEditorWidgetConfig readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx );
    virtual void writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, const QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx );

    virtual QString name();
};
