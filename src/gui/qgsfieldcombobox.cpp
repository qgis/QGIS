/***************************************************************************
   qgsfieldcombobox.cpp
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsfieldcombobox.h"
#include "qgsfieldmodel.h"
#include "qgsmaplayer.h"

QgsFieldComboBox::QgsFieldComboBox( QWidget *parent ) :
    QComboBox( parent )
{
  mFieldModel = new QgsFieldModel( this );
  setModel( mFieldModel );

  connect( this, SIGNAL( currentIndexChanged( int ) ), this, SLOT( indexChanged( int ) ) );
}

void QgsFieldComboBox::setLayer( QgsMapLayer *layer )
{
  mFieldModel->setLayer( layer );
}

QgsVectorLayer *QgsFieldComboBox::layer()
{
  QgsMapLayer* layer = mFieldModel->layer();
  QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer );
  if ( vl )
    return vl;
  else
    return 0;
}

void QgsFieldComboBox::setField( QString fieldName )
{
  QModelIndex idx = mFieldModel->indexFromName( fieldName );
  if ( idx.isValid() )
  {
    setCurrentIndex( idx.row() );
    return;
  }

  if ( mAllowExpression )
  {
    mFieldModel->setExpression( fieldName );
    setCurrentIndex( findText( fieldName ) );
    return;
  }
  setCurrentIndex( -1 );
}

QString QgsFieldComboBox::currentField( bool *isExpression )
{
  if ( isExpression )
  {
    *isExpression = false;
  }

  int i = currentIndex();

  const QModelIndex index = mFieldModel->index( i, 0 );
  if ( !index.isValid() )
  {
    return "";
  }

  QString fieldName = mFieldModel->data( index, QgsFieldModel::FieldNameRole ).toString();
  if ( !fieldName.isEmpty() )
  {
    return fieldName;
  }

  if ( mAllowExpression )
  {
    QString expression = mFieldModel->data( index, QgsFieldModel::ExpressionRole ).toString();
    if ( !expression.isEmpty() )
    {
      if ( isExpression )
      {
        *isExpression = true;
      }
      return expression;
    }
  }

  return "";
}

void QgsFieldComboBox::setAllowExpression( bool allowExpression )
{
  mFieldModel->setAllowExpression( allowExpression );
}

void QgsFieldComboBox::indexChanged( int i )
{
  Q_UNUSED( i );
  QString name = currentField();
  emit fieldChanged( name );
}
