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

#include "qgsvaluemapsearchwidgetwrapper.h"
#include "qgstexteditconfigdlg.h"

#include "qgsfield.h"
#include "qgsfieldvalidator.h"

#include <QSettings>
#include <QSizePolicy>

QgsValueMapSearchWidgetWrapper::QgsValueMapSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsSearchWidgetWrapper( vl, fieldIdx, parent )
    , mComboBox( nullptr )
{
}

QWidget* QgsValueMapSearchWidgetWrapper::createWidget( QWidget* parent )
{
  return new QComboBox( parent );
}

void QgsValueMapSearchWidgetWrapper::comboBoxIndexChanged( int idx )
{
  if ( mComboBox )
  {
    if ( idx == 0 )
    {
      clearExpression();
    }
    else
    {
      setExpression( mComboBox->itemData( idx ).toString() );
    }
    emit expressionChanged( mExpression );
  }
}

bool QgsValueMapSearchWidgetWrapper::applyDirectly()
{
  return true;
}

QString QgsValueMapSearchWidgetWrapper::expression()
{
  return mExpression;
}

bool QgsValueMapSearchWidgetWrapper::valid() const
{
  return true;
}

void QgsValueMapSearchWidgetWrapper::initWidget( QWidget* editor )
{
  mComboBox = qobject_cast<QComboBox*>( editor );

  if ( mComboBox )
  {
    const QgsEditorWidgetConfig cfg = config();
    QgsEditorWidgetConfig::ConstIterator it = cfg.constBegin();
    mComboBox->addItem( tr( "Please select" ), "" );

    while ( it != cfg.constEnd() )
    {
      mComboBox->addItem( it.key(), it.value() );
      ++it;
    }
    connect( mComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( comboBoxIndexChanged( int ) ) );
  }
}

void QgsValueMapSearchWidgetWrapper::setExpression( QString exp )
{
  QString fieldName = layer()->fields().at( mFieldIdx ).name();
  QString str;

  str = QString( "%1 = '%2'" )
        .arg( QgsExpression::quotedColumnRef( fieldName ),
              exp.replace( '\'', "''" ) );

  mExpression = str;
}

