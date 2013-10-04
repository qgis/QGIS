/***************************************************************************
                             qgsfieldchoosercombo.cpp
                             -------------------------
    begin                : September 2013
    copyright            : (C) 2013 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfieldchoosercombo.h"
#include "qgsfieldchooserwidget.h"

QgsFieldChooserCombo::QgsFieldChooserCombo( QgsLayerChooserWidget* layerChooser, QObject *parent )
    : QgsFieldChooserWidget( layerChooser, parent )
    , mWidget( 0 )
{
}

QgsFieldChooserCombo::QgsFieldChooserCombo( QgsVectorLayer* vl, QObject *parent )
    : QgsFieldChooserWidget( parent )
    , mWidget( 0 )
{
  setLayer( vl );
}

bool QgsFieldChooserCombo::initWidget( QWidget *widget )
{
  mWidget = 0;
  QComboBox* cb = dynamic_cast<QComboBox*>( widget );
  if ( !cb )
    return false;

  connect( cb, SIGNAL( currentIndexChanged( int ) ), this, SLOT( currentIndexChanged( int ) ) );
  mWidget = cb;

  layerChanged();
  unselect();

  return true;
}


void QgsFieldChooserCombo::clearWidget()
{
  if ( !mWidget )
    return;

  mWidget->clear();
}

void QgsFieldChooserCombo::unselect()
{
  if ( !mWidget )
    return;
  mWidget->setCurrentIndex( -1 );
}

void QgsFieldChooserCombo::addField( QString fieldAlias, QString fieldName, DisplayStatus display )
{
  if ( !mWidget )
    return;
  mWidget->addItem( fieldAlias, fieldName );
  if ( display == disabled )
  {
    // dirty trick to disable an item in a combo box
    int i = mWidget->count() - 1;
    QModelIndex j = mWidget->model()->index( i, 0 );
    mWidget->model()->setData( j, 0, Qt::UserRole - 1 );
  }
}

int QgsFieldChooserCombo::getFieldIndex()
{
  QString fieldName = getFieldName();
  return mLayer->fieldNameIndex( fieldName );
}

QString QgsFieldChooserCombo::getFieldName()
{
  if ( !mWidget )
    return "";
  if ( !mLayer )
    return "";

  int idx = mWidget->currentIndex();
  QString fieldName = mWidget->itemData( idx, Qt::UserRole ).toString();

  return fieldName;
}

void QgsFieldChooserCombo::setField( QString fieldName )
{
  if ( !mWidget )
    return;

  int idx = mWidget->findData( QVariant( fieldName ), Qt::UserRole );
  mWidget->setCurrentIndex( idx );
}

void QgsFieldChooserCombo::currentIndexChanged( int idx )
{
  if ( !mWidget )
    return;

  const QgsFields &fields = mLayer->pendingFields();
  if ( idx < 0 || idx >= fields.size() )
  {
    emit fieldChanged( -1 );
    return;
  }
  emit fieldChanged( idx );
}
