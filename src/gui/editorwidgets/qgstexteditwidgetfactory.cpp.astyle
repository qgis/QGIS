/***************************************************************************
    qgstexteditwidgetfactory.cpp
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

#include "qgstexteditwidgetfactory.h"

#include "qgstexteditwrapper.h"
#include "qgstexteditconfigdlg.h"
#include "qgstexteditsearchwidgetwrapper.h"

QgsTextEditWidgetFactory::QgsTextEditWidgetFactory( const QString &name )
  : QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper *QgsTextEditWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsTextEditWrapper( vl, fieldIdx, editor, parent );
}

QgsSearchWidgetWrapper *QgsTextEditWidgetFactory::createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsTextEditSearchWidgetWrapper( vl, fieldIdx, parent );
}

QgsEditorConfigWidget *QgsTextEditWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsTextEditConfigDlg( vl, fieldIdx, parent );
}

unsigned int QgsTextEditWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )
  return 10;
}
