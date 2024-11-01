/***************************************************************************
  qgslocaleawarenumericlineeditdelegate.cpp - QgsLocaleAwareNumericLineEditDelegate

 ---------------------
 begin                : 5.11.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QLineEdit>

#include "qgslocaleawarenumericlineeditdelegate.h"
#include "moc_qgslocaleawarenumericlineeditdelegate.cpp"
#include "qgsdoublevalidator.h"
#include "qgsguiutils.h"

/// @cond PRIVATE

QgsLocaleAwareNumericLineEditDelegate::QgsLocaleAwareNumericLineEditDelegate( Qgis::DataType dataType, QWidget *parent )
  : QStyledItemDelegate( parent )
  , mDataType( dataType )
{
}

QWidget *QgsLocaleAwareNumericLineEditDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  Q_UNUSED( index )
  auto editor = new QLineEdit { parent };
  editor->setValidator( new QgsDoubleValidator( QgsGuiUtils::significantDigits( mDataType ), editor ) );
  return editor;
}

void QgsLocaleAwareNumericLineEditDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QLineEdit *lineEdit { qobject_cast<QLineEdit *>( editor ) };
  if ( lineEdit )
  {
    const QVariant value = index.data();
    lineEdit->setText( displayText( value, QLocale() ) );
  }
  else
  {
    QStyledItemDelegate::setEditorData( editor, index );
  }
}

void QgsLocaleAwareNumericLineEditDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QLineEdit *lineEdit { qobject_cast<QLineEdit *>( editor ) };
  if ( !editor )
  {
    QStyledItemDelegate::setModelData( editor, model, index );
  }
  const double value { QgsDoubleValidator::toDouble( lineEdit->text() ) };
  model->setData( index, value );
}

QString QgsLocaleAwareNumericLineEditDelegate::displayText( const QVariant &value, const QLocale & ) const
{
  return QgsGuiUtils::displayValueWithMaximumDecimals( mDataType, value.toDouble() );
}

void QgsLocaleAwareNumericLineEditDelegate::setDataType( const Qgis::DataType &dataType )
{
  mDataType = dataType;
}

///@endcond
