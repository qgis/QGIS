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

#include "qgsjsoneditwrapper.h"
#include "qgsjsoneditconfigdlg.h"

QgsJsonEditWidgetFactory::QgsJsonEditWidgetFactory( const QString &name )
  : QgsEditorWidgetFactory( name )
{
}

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
  const QVariant::Type type = vl->fields().field( fieldIdx ).type();

  switch ( type )
  {
    case QVariant::Map:
      return 15;
      break;
    case QVariant::List:
      return 10;
      break;
    default:
      return 5;
      break;
  }
}
