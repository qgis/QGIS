/***************************************************************************
    qgsgeometrywidgetfactory.cpp
     -----------------------
    Date                 : February 2023
    Copyright            : (C) 2023 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrywidgetfactory.h"

#include "qgsgeometrywidgetwrapper.h"
#include "qgsdummyconfigdlg.h"
#include "qgsvectorlayer.h"

QgsGeometryWidgetFactory::QgsGeometryWidgetFactory( const QString &name, QgsMessageBar *messageBar )
  : QgsEditorWidgetFactory( name )
  , mMessageBar( messageBar )
{
}

QgsEditorWidgetWrapper *QgsGeometryWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsGeometryWidgetWrapper( vl, fieldIdx, editor, parent, mMessageBar );
}

QgsEditorConfigWidget *QgsGeometryWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "A widget for interacting with geometry fields." ) );
}

unsigned int QgsGeometryWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  const QgsField field = vl->fields().field( fieldIdx );
  const QMetaType::Type type = field.type();
  // Geometry fields only
  return ( type == QMetaType::Type::User && field.typeName().compare( QLatin1String( "geometry" ), Qt::CaseInsensitive ) == 0 ) ? 20 : 0;
}
