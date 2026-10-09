/***************************************************************************
    qgsjsoneditwidgetfactory.cpp
     --------------------------------------
    Date                 : 3.5.2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsjsoneditwidgetfactory.h"

#include "qgsjsoneditconfigdlg.h"
#include "qgsjsoneditwrapper.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsJsonEditWidgetFactory::QgsJsonEditWidgetFactory( const QString &name, const QIcon &icon )
  : QgsEditorWidgetFactory( name, icon )
{}

QgsEditorWidgetWrapper *QgsJsonEditWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsJsonEditWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsJsonEditWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsJsonEditConfigDlg( vl, fieldIdx, parent );
}

unsigned int QgsJsonEditWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  const QgsField field = vl->fields().field( fieldIdx );
  // Handle the json field
  if ( field.typeName().compare( u"json"_s, Qt::CaseInsensitive ) == 0 || field.typeName().compare( u"jsonb"_s, Qt::CaseInsensitive ) == 0 )
  {
    return 15;
  }
  return 5;
}

bool QgsJsonEditWidgetFactory::isReadOnly() const
{
  return true;
}
