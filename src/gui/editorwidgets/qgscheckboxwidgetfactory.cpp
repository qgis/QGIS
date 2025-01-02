/***************************************************************************
    qgscheckboxwidgetfactory.cpp
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

#include "qgscheckboxwidgetfactory.h"
#include "qgscheckboxsearchwidgetwrapper.h"
#include "qgscheckboxwidgetwrapper.h"
#include "qgscheckboxconfigdlg.h"

QgsCheckboxWidgetFactory::QgsCheckboxWidgetFactory( const QString &name )
  : QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper *QgsCheckboxWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsCheckboxWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsSearchWidgetWrapper *QgsCheckboxWidgetFactory::createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsCheckboxSearchWidgetWrapper( vl, fieldIdx, parent );
}

QgsEditorConfigWidget *QgsCheckboxWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsCheckBoxConfigDlg( vl, fieldIdx, parent );
}

QHash<const char *, int> QgsCheckboxWidgetFactory::supportedWidgetTypes()
{
  QHash<const char *, int> map = QHash<const char *, int>();
  map.insert( QCheckBox::staticMetaObject.className(), 10 );
  map.insert( QGroupBox::staticMetaObject.className(), 10 );
  return map;
}

unsigned int QgsCheckboxWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  const QMetaType::Type type = vl->fields().field( fieldIdx ).type();
  return type == QMetaType::Type::Bool ? 20 : 5;
}
