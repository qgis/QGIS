/***************************************************************************
    qgsfilenamewidgetfactory.h
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

#ifndef QGSFILENAMEWIDGETFACTORY_H
#define QGSFILENAMEWIDGETFACTORY_H

#include "qgseditorwidgetfactory.h"
#include "qgsfilenameconfigdlg.h"
#include "qgsfilenamewidgetwrapper.h"

/** \class QgsFileNameWidgetFactory
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsFileNameWidgetFactory : public QgsEditorWidgetFactory
{
  public:
    QgsFileNameWidgetFactory( const QString& name );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper* create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const override;
    QgsEditorConfigWidget* configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const override;
    
    // QgsEditorWidgetFactory interface
  public:
    void writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx ) override;

  private:
    QgsEditorWidgetConfig readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx ) override;
    bool isFieldSupported( QgsVectorLayer* vl, int fieldIdx ) override;
};

#endif // QGSFILENAMEWIDGETFACTORY_H
