/***************************************************************************
    qgsrangewidgetfactory.h
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

#ifndef QGSRANGEWIDGETFACTORY_H
#define QGSRANGEWIDGETFACTORY_H

#include "qgseditorwidgetfactory.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsRangeWidgetFactory
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsRangeWidgetFactory : public QgsEditorWidgetFactory
{
  public:
    QgsRangeWidgetFactory( const QString &name );

    // QgsEditorWidgetFactory interface
  public:
    virtual QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    virtual QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    virtual QHash<const char *, int> supportedWidgetTypes() override;

  private:
    virtual unsigned int fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const override;
};

#endif // QGSRANGEWIDGETFACTORY_H
