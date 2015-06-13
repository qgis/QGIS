/***************************************************************************
    qgsdefaultsearchwidgettwrapper.cpp
     --------------------------------------
    Date                 : 31.5.2015
    Copyright            : (C) 2015 Karolina Alexiou (carolinux)
    Email                : carolinegr at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdefaultsearchwidgetwrapper.h"

#include "qgsfield.h"
#include "qgsfieldvalidator.h"

#include <QSettings>
#include <QSizePolicy>

QgsDefaultSearchWidgetWrapper::QgsDefaultSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsSearchWidgetWrapper( vl, fieldIdx, parent )
    , mLineEdit( NULL )
{
}

void QgsDefaultSearchWidgetWrapper::setFeature( const QgsFeature& feature )
{
}

QString QgsDefaultSearchWidgetWrapper::expression()
{
  //FIXME clearly not
  return "foooo";//mLineEdit->text();
}

void QgsDefaultSearchWidgetWrapper::setExpression(QString exp)
{

}

QWidget* QgsDefaultSearchWidgetWrapper::createWidget( QWidget* parent )
{
  return new QgsFilterLineEdit( parent );
}

void QgsDefaultSearchWidgetWrapper::initWidget( QWidget* widget )
{
  mLineEdit = qobject_cast<QgsFilterLineEdit*>( widget );
  //mLineEdit->setSizePolicy( QSizePolicy ::Expanding , QSizePolicy ::Fixed );
  connect( widget, SIGNAL( textChanged( QString ) ), this, SLOT( expressionChanged( QString ) ) );
}
