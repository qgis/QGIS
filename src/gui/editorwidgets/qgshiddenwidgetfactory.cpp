/***************************************************************************
    qgshiddenwidgetfactory.cpp
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

#include "qgshiddenwidgetfactory.h"

#include "qgsdummyconfigdlg.h"
#include "qgshiddenwidgetwrapper.h"

QgsHiddenWidgetFactory::QgsHiddenWidgetFactory( const QString &name, const QIcon &icon )
  : QgsEditorWidgetFactory( name, icon )
{
}

QgsEditorWidgetWrapper *QgsHiddenWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsHiddenWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsHiddenWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "A hidden field will be invisible - the user is not able to see its contents." ) );
}

bool QgsHiddenWidgetFactory::isReadOnly() const
{
  return true;
}
