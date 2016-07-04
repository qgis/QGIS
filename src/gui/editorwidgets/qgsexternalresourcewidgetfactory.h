/***************************************************************************
   qgsexternalresourcewidgetfactory.h

 ---------------------
 begin                : 16.12.2015
 copyright            : (C) 2015 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTERNALRESOURCEWIDGETFACTORY_H
#define QGSEXTERNALRESOURCEWIDGETFACTORY_H

#include "qgseditorwidgetfactory.h"
#include "qgsexternalresourceconfigdlg.h"
#include "qgsexternalresourcewidgetwrapper.h"


/** \ingroup gui
 * \class QgsExternalResourceWidgetFactory
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsExternalResourceWidgetFactory : public QgsEditorWidgetFactory
{
  public:
    QgsExternalResourceWidgetFactory( const QString& name );

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

#endif // QGSEXTERNALRESOURCEWIDGETFACTORY_H
