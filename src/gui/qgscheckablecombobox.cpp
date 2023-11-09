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
#include "qgsapplication.h"

#include <QEvent>
#include <QMouseEvent>
#include <QLineEdit>
#include <QPoint>
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
  const bool ok = QStandardItemModel::setData( index, value, role );

  if ( ok && role == Qt::CheckStateRole )
  {
    emit itemCheckStateChanged( index );
  }

  emit dataChanged( index, index );
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
  , mModel( new QgsCheckableItemModel( this ) )
  , mSeparator( QStringLiteral( ", " ) )
{
  setModel( mModel );
  setItemDelegate( new QgsCheckBoxDelegate( this ) );

  QLineEdit *lineEdit = new QLineEdit( this );
  lineEdit->setReadOnly( true );
  QPalette pal = qApp->palette();
  pal.setBrush( QPalette::Base, pal.button() );
  lineEdit->setPalette( pal );
  setLineEdit( lineEdit );
  lineEdit->installEventFilter( this );
  lineEdit->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( lineEdit, &QAbstractItemView::customContextMenuRequested, this, &QgsCheckableComboBox::showContextMenu );

  mContextMenu = new QMenu( this );
  mSelectAllAction = mContextMenu->addAction( tr( "Select All" ) );
  mDeselectAllAction = mContextMenu->addAction( tr( "Deselect All" ) );
  connect( mSelectAllAction, &QAction::triggered, this, &QgsCheckableComboBox::selectAllOptions );
  connect( mDeselectAllAction, &QAction::triggered, this, &QgsCheckableComboBox::deselectAllOptions );

  view()->viewport()->installEventFilter( this );
  view()->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( view(), &QAbstractItemView::customContextMenuRequested, this, &QgsCheckableComboBox::showContextMenu );

  connect( model(), &QStandardItemModel::rowsInserted, this, [ = ]( const QModelIndex &, int, int ) { updateDisplayText(); } );
  connect( model(), &QStandardItemModel::rowsRemoved, this, [ = ]( const QModelIndex &, int, int ) { updateDisplayText(); } );
  connect( model(), &QStandardItemModel::dataChanged, this, [ = ]( const QModelIndex &, const QModelIndex &, const QVector< int > & ) { updateDisplayText(); } );
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

void QgsCheckableComboBox::addItemWithCheckState( const QString &text, Qt::CheckState state, const QVariant &userData )
{
  QComboBox::addItem( text, userData );
  setItemCheckState( count() - 1, state );
}

QStringList QgsCheckableComboBox::checkedItems() const
{
  QStringList items;

  if ( auto *lModel = model() )
  {
    const QModelIndex index = lModel->index( 0, modelColumn(), rootModelIndex() );
    const QModelIndexList indexes = lModel->match( index, Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly );
    const auto constIndexes = indexes;
    for ( const QModelIndex &index : constIndexes )
    {
      items += index.data().toString();
    }
  }

  return items;
}

QVariantList QgsCheckableComboBox::checkedItemsData() const
{
  QVariantList data;

  if ( auto *lModel = model() )
  {
    const QModelIndex index = lModel->index( 0, modelColumn(), rootModelIndex() );
    const QModelIndexList indexes = lModel->match( index, Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly );
    const auto constIndexes = indexes;
    for ( const QModelIndex &index : constIndexes )
    {
      data += index.data( Qt::UserRole ).toString();
    }
  }

  return data;
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
  const QVariant value = itemData( index, Qt::CheckStateRole );
  if ( value.isValid() )
  {
    const Qt::CheckState state = static_cast<Qt::CheckState>( value.toInt() );
    setItemData( index, ( state == Qt::Unchecked ? Qt::Checked : Qt::Unchecked ), Qt::CheckStateRole );
  }
  updateCheckedItems();
}

void QgsCheckableComboBox::hidePopup()
{
  if ( !mSkipHide )
  {
    QComboBox::hidePopup();
  }
  mSkipHide = false;
}

void QgsCheckableComboBox::showContextMenu( QPoint pos )
{
  Q_UNUSED( pos )

  mContextMenu->exec( QCursor::pos() );
}

void QgsCheckableComboBox::selectAllOptions()
{
  blockSignals( true );
  for ( int i = 0;  i < count(); i++ )
  {
    setItemData( i, Qt::Checked, Qt::CheckStateRole );
  }
  blockSignals( false );
  updateCheckedItems();
}

void QgsCheckableComboBox::deselectAllOptions()
{
  blockSignals( true );
  for ( int i = 0;  i < count(); i++ )
  {
    setItemData( i, Qt::Unchecked, Qt::CheckStateRole );
  }
  blockSignals( false );
  updateCheckedItems();
}

bool QgsCheckableComboBox::eventFilter( QObject *object, QEvent *event )
{
  if ( object == lineEdit() )
  {
    if ( event->type() == QEvent::MouseButtonPress && static_cast<QMouseEvent *>( event )->button() == Qt::LeftButton && object == lineEdit() )
    {
      mSkipHide = true;
      showPopup();
    }
  }
  else if ( ( event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease )
            && object == view()->viewport() )
  {
    mSkipHide = true;

    if ( event->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent *>( event )->button() == Qt::RightButton )
    {
      return true;
    }

    if ( event->type() == QEvent::MouseButtonRelease )
    {
      const QModelIndex index = view()->indexAt( static_cast<QMouseEvent *>( event )->pos() );
      if ( index.isValid() )
      {
        QgsCheckableItemModel *myModel = qobject_cast<QgsCheckableItemModel *>( model() );
        QStandardItem *item = myModel->itemFromIndex( index );
        item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
        updateCheckedItems();
      }
      return true;
    }
  }

  return QComboBox::eventFilter( object, event );
}

void QgsCheckableComboBox::setCheckedItems( const QStringList &items )
{
  const auto constItems = items;
  for ( const QString &text : constItems )
  {
    const int index = findText( text );
    setItemCheckState( index, index != -1 ? Qt::Checked : Qt::Unchecked );
  }
  updateCheckedItems();
}

void QgsCheckableComboBox::resizeEvent( QResizeEvent *event )
{
  QComboBox::resizeEvent( event );
  updateDisplayText();
}

void QgsCheckableComboBox::updateCheckedItems()
{
  const QStringList items = checkedItems();
  updateDisplayText();
  emit checkedItemsChanged( items );
}

void QgsCheckableComboBox::updateDisplayText()
{
  // There is only a line edit if the combobox is in editable state
  if ( !lineEdit() )
    return;

  QString text;
  const QStringList items = checkedItems();
  if ( items.isEmpty() )
  {
    text = mDefaultText;
  }
  else
  {
    text = items.join( mSeparator );
  }

  const QRect rect = lineEdit()->rect();
  const QFontMetrics fontMetrics( font() );
  text = fontMetrics.elidedText( text, Qt::ElideRight, rect.width() );
  setEditText( text );
}

