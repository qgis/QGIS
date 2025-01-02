/***************************************************************************
    qgsbinarywidgetfactory.cpp
     -------------------------
    Date                 : November 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbinarywidgetfactory.h"

#include "qgsbinarywidgetwrapper.h"
#include "qgsdummyconfigdlg.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

QgsBinaryWidgetFactory::QgsBinaryWidgetFactory( const QString &name, QgsMessageBar *messageBar )
  : QgsEditorWidgetFactory( name )
  , mMessageBar( messageBar )
{
}

QgsEditorWidgetWrapper *QgsBinaryWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsBinaryWidgetWrapper( vl, fieldIdx, editor, parent, mMessageBar );
}

QgsEditorConfigWidget *QgsBinaryWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "A widget for interacting with binary (BLOB) fields." ) );
}

unsigned int QgsBinaryWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  const QgsField field = vl->fields().field( fieldIdx );
  const QMetaType::Type type = field.type();
  // ByteArray fields only
  return type == QMetaType::Type::QByteArray ? 20 : 0;
}
