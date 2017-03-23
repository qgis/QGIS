/***************************************************************************
                              qgscheckablecombobox.cpp
                              ------------------------
  begin                : March 21, 2017
  copyright            : (C) 2017 by Alexander Bruy
  email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscheckablecombobox.h"

#include <QLineEdit>
#include <QAbstractItemView>


QgsCheckableItemModel::QgsCheckableItemModel( QObject *parent )
  : QStandardItemModel( 0, 1, parent )
{
}

Qt::ItemFlags QgsCheckableItemModel::flags( const QModelIndex &index ) const
{
  return QStandardItemModel::flags( index ) | Qt::ItemIsUserCheckable;
}

QVariant QgsCheckableItemModel::data( const QModelIndex &index, int role ) const
{
  QVariant value = QStandardItemModel::data( index, role );

  if ( index.isValid() && role == Qt::CheckStateRole && !value.isValid() )
  {
    value = Qt::Unchecked;
  }

  return value;
}

bool QgsCheckableItemModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  bool ok = QStandardItemModel::setData( index, value, role );

  if ( ok && role == Qt::CheckStateRole )
  {
    emit dataChanged( index, index );
    emit itemCheckStateChanged();
  }

  return ok;
}


QgsCheckBoxDelegate::QgsCheckBoxDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

void QgsCheckBoxDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QStyleOptionViewItem &nonConstOpt = const_cast<QStyleOptionViewItem &>( option );
  nonConstOpt.showDecorationSelected = false;
  QStyledItemDelegate::paint( painter, nonConstOpt, index );
}


QgsCheckableComboBox::QgsCheckableComboBox( QWidget *parent )
  : QComboBox( parent )
  , mSeparator( ", " )
{
  setModel( new QgsCheckableItemModel( this ) );
  setItemDelegate( new QgsCheckBoxDelegate( this ) );

  QLineEdit *lineEdit = new QLineEdit( this );
  lineEdit->setReadOnly( true );
  setLineEdit( lineEdit );

  QgsCheckableItemModel *myModel = qobject_cast<QgsCheckableItemModel *>( model() );
  connect( myModel, &QgsCheckableItemModel::itemCheckStateChanged, this, &QgsCheckableComboBox::updateCheckedItems );
  connect( model(), &QStandardItemModel::rowsInserted, this, [ = ]( const QModelIndex &, int, int ) { updateCheckedItems(); } );
  connect( model(), &QStandardItemModel::rowsRemoved, this, [ = ]( const QModelIndex &, int, int ) { updateCheckedItems(); } );

  connect( this, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::activated ), this, &QgsCheckableComboBox::toggleItemCheckState );
}

QString QgsCheckableComboBox::separator() const
{
  return mSeparator;
}

void QgsCheckableComboBox::setSeparator( const QString &separator )
{
  if ( mSeparator != separator )
  {
    mSeparator = separator;
    updateDisplayText();
  }
}

QString QgsCheckableComboBox::defaultText() const
{
  return mDefaultText;
}

void QgsCheckableComboBox::setDefaultText( const QString &text )
{
  if ( mDefaultText != text )
  {
    mDefaultText = text;
    updateDisplayText();
  }
}

QStringList QgsCheckableComboBox::checkedItems() const
{
  QStringList items;

  if ( model() )
  {
    QModelIndex index = model()->index( 0, modelColumn(), rootModelIndex() );
    QModelIndexList indexes = model()->match( index, Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly );
    Q_FOREACH ( const QModelIndex &index, indexes )
    {
      items += index.data().toString();
    }
  }

  return items;
}

Qt::CheckState QgsCheckableComboBox::itemCheckState( int index ) const
{
  return static_cast<Qt::CheckState>( itemData( index, Qt::CheckStateRole ).toInt() );
}

void QgsCheckableComboBox::setItemCheckState( int index, Qt::CheckState state )
{
  setItemData( index, state, Qt::CheckStateRole );
}

void QgsCheckableComboBox::toggleItemCheckState( int index )
{
  QVariant value = itemData( index, Qt::CheckStateRole );
  if ( value.isValid() )
  {
    Qt::CheckState state = static_cast<Qt::CheckState>( value.toInt() );
    setItemData( index, ( state == Qt::Unchecked ? Qt::Checked : Qt::Unchecked ), Qt::CheckStateRole );
  }
}

void QgsCheckableComboBox::hidePopup()
{
  if ( !view()->underMouse() )
  {
    QComboBox::hidePopup();
  }
}

void QgsCheckableComboBox::setCheckedItems( const QStringList &items )
{
  Q_FOREACH ( const QString &text, items )
  {
    const int index = findText( text );
    setItemCheckState( index, index != -1 ? Qt::Checked : Qt::Unchecked );
  }
}

void QgsCheckableComboBox::resizeEvent( QResizeEvent *event )
{
  QComboBox::resizeEvent( event );
  updateDisplayText();
}

void QgsCheckableComboBox::updateCheckedItems()
{
  QStringList items = checkedItems();
  updateDisplayText();
  emit checkedItemsChanged( items );
}

void QgsCheckableComboBox::updateDisplayText()
{
  QString text;
  QStringList items = checkedItems();
  if ( items.isEmpty() )
  {
    text = mDefaultText;
  }
  else
  {
    text = items.join( mSeparator );
  }

  QRect rect = lineEdit()->rect();
  QFontMetrics fontMetrics( font() );
  text = fontMetrics.elidedText( text, Qt::ElideRight, rect.width() );
  setEditText( text );
}
