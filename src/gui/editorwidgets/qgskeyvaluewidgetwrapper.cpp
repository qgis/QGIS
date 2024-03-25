/***************************************************************************
    qgskeyvaluewidgetwrapper.cpp
     --------------------------------------
    Date                 : 08.2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick.valsecchi@camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgskeyvaluewidgetwrapper.h"
#include "qgskeyvaluewidget.h"
#include "qgsattributeform.h"

QgsKeyValueWidgetWrapper::QgsKeyValueWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent ):
  QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )
{
}

QVariant QgsKeyValueWidgetWrapper::value() const
{
  if ( !mWidget ) return QVariant( QVariant::Map );
  return mWidget->map();
}

void QgsKeyValueWidgetWrapper::showIndeterminateState()
{
  mWidget->setMap( QVariantMap() );
}

void QgsKeyValueWidgetWrapper::setEnabled( bool enabled )
{
  if ( mWidget )
  {
    mWidget->setReadOnly( !enabled );
  }
}

QWidget *QgsKeyValueWidgetWrapper::createWidget( QWidget *parent )
{
  if ( isInTable( parent ) )
  {
    // if to be put in a table, draw a border and set a decent size
    QFrame *ret = new QFrame( parent );
    ret->setFrameShape( QFrame::StyledPanel );
    QHBoxLayout *layout = new QHBoxLayout( ret );
    layout->addWidget( new QgsKeyValueWidget( ret ) );
    ret->setMinimumSize( QSize( 320, 110 ) );
    return ret;
  }
  else
  {
    return new QgsKeyValueWidget( parent );
  }
}

void QgsKeyValueWidgetWrapper::initWidget( QWidget *editor )
{
  mWidget = qobject_cast<QgsKeyValueWidget *>( editor );
  if ( !mWidget )
  {
    mWidget = editor->findChild<QgsKeyValueWidget *>();
  }

  connect( mWidget, &QgsKeyValueWidget::valueChanged, this, &QgsKeyValueWidgetWrapper::onValueChanged );
}

bool QgsKeyValueWidgetWrapper::valid() const
{
  return true;
}

void QgsKeyValueWidgetWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  mWidget->setMap( value.toMap() );
}

void QgsKeyValueWidgetWrapper::updateConstraintWidgetStatus()
{
  // Nothing
}

void QgsKeyValueWidgetWrapper::onValueChanged()
{
  emitValueChanged();
}
