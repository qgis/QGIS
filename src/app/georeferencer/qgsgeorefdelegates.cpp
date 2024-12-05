/***************************************************************************
     qgsgeorefdelegates.cpp
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <limits>

#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionValidator>

#include "qgsgeorefvalidators.h"
#include "qgsgeorefdelegates.h"
#include "moc_qgsgeorefdelegates.cpp"
#include <cmath>

// ------------------------- QgsDmsAndDdDelegate --------------------------- //
QgsDmsAndDdDelegate::QgsDmsAndDdDelegate( QWidget *parent )
  : QStyledItemDelegate( parent )
{
}

QWidget *QgsDmsAndDdDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/ ) const
{
  QLineEdit *editor = new QLineEdit( parent );
  QgsDMSAndDDValidator *validator = new QgsDMSAndDDValidator( editor );
  editor->setValidator( validator );

  return editor;
}

void QgsDmsAndDdDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  const QString value = index.model()->data( index, Qt::EditRole ).toString();

  QLineEdit *lineEdit = static_cast<QLineEdit *>( editor );
  lineEdit->setText( value );
}

void QgsDmsAndDdDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QLineEdit *lineEdit = static_cast<QLineEdit *>( editor );
  const QString stringValue = lineEdit->text();
  double value = 0;
  if ( stringValue.contains( ' ' ) )
    value = dmsToDD( stringValue );
  else
    value = stringValue.toDouble();

  model->setData( index, value, Qt::EditRole );
}

void QgsDmsAndDdDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /*index*/ ) const
{
  editor->setGeometry( option.rect );
}

double QgsDmsAndDdDelegate::dmsToDD( const QString &dms ) const
{
  const QStringList list = dms.split( ' ' );
  QString tmpStr = list.at( 0 );
  double res = std::fabs( tmpStr.toDouble() );

  tmpStr = list.value( 1 );
  if ( !tmpStr.isEmpty() )
    res += tmpStr.toDouble() / 60;

  tmpStr = list.value( 2 );
  if ( !tmpStr.isEmpty() )
    res += tmpStr.toDouble() / 3600;

  return dms.startsWith( '-' ) ? -res : res;
}

// ---------------------------- QgsCoordDelegate --------------------------- //
QgsCoordDelegate::QgsCoordDelegate( QWidget *parent )
  : QStyledItemDelegate( parent )
{
}

QWidget *QgsCoordDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/ ) const
{
  QLineEdit *editor = new QLineEdit( parent );
  const thread_local QRegularExpression re( QStringLiteral( "-?\\d*(\\.\\d+)?" ) );
  QRegularExpressionValidator *validator = new QRegularExpressionValidator( re, editor );
  editor->setValidator( validator );

  return editor;
}

void QgsCoordDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  const QString value = index.model()->data( index, Qt::EditRole ).toString();

  QLineEdit *lineEdit = static_cast<QLineEdit *>( editor );
  lineEdit->setText( value );
}

void QgsCoordDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QLineEdit *lineEdit = static_cast<QLineEdit *>( editor );
  const QString stringValue = lineEdit->text();
  const double value = stringValue.toDouble();
  model->setData( index, value, Qt::EditRole );
}

void QgsCoordDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index )
  editor->setGeometry( option.rect );
}
