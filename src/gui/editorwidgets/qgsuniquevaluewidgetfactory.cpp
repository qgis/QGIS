/***************************************************************************
    qgsuniquevaluewidgetfactory.cpp
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

#include "qgsuniquevaluewidgetfactory.h"

#include "qgsuniquevaluewidgetwrapper.h"
#include "qgsuniquevaluesconfigdlg.h"

QgsUniqueValueWidgetFactory::QgsUniqueValueWidgetFactory( const QString &name, const QIcon &icon )
  : QgsEditorWidgetFactory( name, icon )
{
}


QgsEditorWidgetWrapper *QgsUniqueValueWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsUniqueValuesWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsUniqueValueWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsUniqueValuesConfigDlg( vl, fieldIdx, parent );
}
