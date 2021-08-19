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
#include "qgis_gui.h"

SIP_NO_FILE

class QgsMessageBar;

/**
 * \ingroup gui
 * \class QgsExternalResourceWidgetFactory
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsExternalResourceWidgetFactory : public QgsEditorWidgetFactory
{
  public:

    /**
     * Constructor for QgsExternalResourceWidgetFactory, where \a name is a human-readable
     * name for the factory and \a messageBar the message bar used to report messages.
     */
    QgsExternalResourceWidgetFactory( const QString &name, QgsMessageBar *messageBar );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    unsigned int fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const override;

  private:
    QgsMessageBar *mMessageBar = nullptr;
};

#endif // QGSEXTERNALRESOURCEWIDGETFACTORY_H
