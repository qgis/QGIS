/***************************************************************************
    qgshiddenwidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshiddenwidgetwrapper.h"

#include <QWidget>

QgsHiddenWidgetWrapper::QgsHiddenWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    :  QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
{
}


QVariant QgsHiddenWidgetWrapper::value()
{
  return mValue;
}

QWidget* QgsHiddenWidgetWrapper::createWidget( QWidget* parent )
{
  QWidget* wdg = new QWidget( parent );
  wdg->setVisible( false );
  return wdg;
}

void QgsHiddenWidgetWrapper::initWidget( QWidget* editor )
{
  editor->setVisible( false );
}

bool QgsHiddenWidgetWrapper::valid()
{
  return true;
}

void QgsHiddenWidgetWrapper::setValue( const QVariant& value )
{
  mValue = value;
}
