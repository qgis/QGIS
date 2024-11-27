/***************************************************************************
    qgshiddenwidgetwrapper.cpp
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

#include "qgshiddenwidgetwrapper.h"
#include "moc_qgshiddenwidgetwrapper.cpp"

#include <QWidget>

QgsHiddenWidgetWrapper::QgsHiddenWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )
{
}


QVariant QgsHiddenWidgetWrapper::value() const
{
  return mValue;
}

QWidget *QgsHiddenWidgetWrapper::createWidget( QWidget *parent )
{
  QWidget *wdg = new QWidget( parent );
  wdg->setVisible( false );
  return wdg;
}

void QgsHiddenWidgetWrapper::initWidget( QWidget *editor )
{
  editor->setVisible( false );
}

bool QgsHiddenWidgetWrapper::valid() const
{
  return true;
}

void QgsHiddenWidgetWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  mValue = value;
}
