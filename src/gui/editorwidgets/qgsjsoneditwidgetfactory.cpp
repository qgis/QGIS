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
#include "qgsjsoneditsearchwidgetwrapper.h"

QgsJsonEditWidgetFactory::QgsJsonEditWidgetFactory( const QString &name )
  : QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper *QgsJsonEditWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsJsonEditWrapper( vl, fieldIdx, editor, parent );
}

QgsSearchWidgetWrapper *QgsJsonEditWidgetFactory::createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsJsonEditSearchWidgetWrapper( vl, fieldIdx, parent );
}

QgsEditorConfigWidget *QgsJsonEditWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsJsonEditConfigDlg( vl, fieldIdx, parent );
}

unsigned int QgsJsonEditWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )
  return 10;
}
