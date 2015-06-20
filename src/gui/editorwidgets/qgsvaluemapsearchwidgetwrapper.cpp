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
    : QgsDefaultSearchWidgetWrapper( vl, fieldIdx, parent ),
      mComboBox( NULL )
{
}

QWidget* QgsValueMapSearchWidgetWrapper::createWidget( QWidget* parent )
{
  return new QComboBox( parent );
}

void QgsValueMapSearchWidgetWrapper::comboBoxIndexChanged(int)
{
  if ( mComboBox )
    setExpression(mComboBox->itemData( mComboBox->currentIndex()).toString());
}

bool QgsValueMapSearchWidgetWrapper::applyDirectly() 
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
    mComboBox->addItem( tr( "Please select" ), "");

    while ( it != cfg.constEnd() )
    {
      mComboBox->addItem( it.key(), it.value() );
      ++it;
    }
    connect( mComboBox, SIGNAL( currentIndexChanged(int) ), this, SLOT( comboBoxIndexChanged(int) ) );
  }
}

