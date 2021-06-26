/***************************************************************************
    qgstexteditwidgetfactory.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
