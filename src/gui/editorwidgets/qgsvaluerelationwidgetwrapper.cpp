/***************************************************************************
    qgsvaluerelationwidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvaluerelationwidgetwrapper.h"

#include "qgis.h"
#include "qgsfields.h"
#include "qgsproject.h"
#include "qgsvaluerelationwidgetfactory.h"
#include "qgsvectorlayer.h"
#include "qgsfilterlineedit.h"
#include "qgsfeatureiterator.h"
#include "qgsvaluerelationfieldformatter.h"

#include <QHeaderView>
#include <QComboBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QStringListModel>
#include <QCompleter>

QgsValueRelationWidgetWrapper::QgsValueRelationWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
{
}


QVariant QgsValueRelationWidgetWrapper::value() const
{
  QVariant v;

  if ( mComboBox )
  {
    int cbxIdx = mComboBox->currentIndex();
    if ( cbxIdx > -1 )
    {
      v = mComboBox->currentData();
    }
  }

  if ( mTableWidget )
  {
    QStringList selection;
    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < config( QStringLiteral( "NofColumns" ) ).toInt(); ++i )
      {
        QTableWidgetItem *item = mTableWidget->item( j, i );
        if ( item )
        {
          if ( item->checkState() == Qt::Checked )
            selection << item->data( Qt::UserRole ).toString();
        }
      }
    }
    v = selection.join( QStringLiteral( "," ) ).prepend( '{' ).append( '}' );
  }

  if ( mLineEdit )
  {
    Q_FOREACH ( const QgsValueRelationFieldFormatter::ValueRelationItem &item, mCache )
    {
      if ( item.value == mLineEdit->text() )
      {
        v = item.key;
        break;
      }
    }
  }

  return v;
}

QWidget *QgsValueRelationWidgetWrapper::createWidget( QWidget *parent )
{
  if ( config( QStringLiteral( "AllowMulti" ) ).toBool() )
  {
    return new QTableWidget( parent );
  }
  else if ( config( QStringLiteral( "UseCompleter" ) ).toBool() )
  {
    return new QgsFilterLineEdit( parent );
  }
  {
    return new QComboBox( parent );
  }
}

void QgsValueRelationWidgetWrapper::initWidget( QWidget *editor )
{
  mCache = QgsValueRelationFieldFormatter::createCache( config() );

  mComboBox = qobject_cast<QComboBox *>( editor );
  mTableWidget = qobject_cast<QTableWidget *>( editor );
  mLineEdit = qobject_cast<QLineEdit *>( editor );

  if ( mComboBox )
  {
    if ( config( QStringLiteral( "AllowNull" ) ).toBool() )
    {
      mComboBox->addItem( tr( "(no selection)" ), QVariant( field().type() ) );
    }

    Q_FOREACH ( const QgsValueRelationFieldFormatter::ValueRelationItem &element, mCache )
    {
      mComboBox->addItem( element.value, element.key );
    }

    connect( mComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
             this, static_cast<void ( QgsEditorWidgetWrapper::* )()>( &QgsEditorWidgetWrapper::emitValueChanged ) );
  }
  else if ( mTableWidget )
  {
    mTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
    mTableWidget->horizontalHeader()->setVisible( false );
    mTableWidget->verticalHeader()->setResizeMode( QHeaderView::Stretch );
    mTableWidget->verticalHeader()->setVisible( false );
    mTableWidget->setShowGrid( false );
    mTableWidget->setEditTriggers( QAbstractItemView::NoEditTriggers );
    mTableWidget->setSelectionMode( QAbstractItemView::NoSelection );
    if ( mCache.size() > 0 )
      mTableWidget->setRowCount( ( mCache.size() + config( QStringLiteral( "NofColumns" ) ).toInt() - 1 ) / config( QStringLiteral( "NofColumns" ) ).toInt() );
    else
      mTableWidget->setRowCount( 1 );
    if ( config( QStringLiteral( "NofColumns" ) ).toInt() > 0 )
      mTableWidget->setColumnCount( config( QStringLiteral( "NofColumns" ) ).toInt() );
    else
      mTableWidget->setColumnCount( 1 );

    int row = 0, column = 0;
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &element : qgis::as_const( mCache ) )
    {
      if ( column == config( QStringLiteral( "NofColumns" ) ).toInt() )
      {
        row++;
        column = 0;
      }
      QTableWidgetItem *item = nullptr;
      item = new QTableWidgetItem( element.value );
      item->setData( Qt::UserRole, element.key );
      mTableWidget->setItem( row, column, item );
      column++;
    }
    connect( mTableWidget, &QTableWidget::itemChanged, this, static_cast<void ( QgsEditorWidgetWrapper::* )()>( &QgsEditorWidgetWrapper::emitValueChanged ) );
  }
  else if ( mLineEdit )
  {
    QStringList values;
    values.reserve( mCache.size() );
    Q_FOREACH ( const QgsValueRelationFieldFormatter::ValueRelationItem &i,  mCache )
    {
      values << i.value;
    }

    QStringListModel *m = new QStringListModel( values, mLineEdit );
    QCompleter *completer = new QCompleter( m, mLineEdit );
    completer->setCaseSensitivity( Qt::CaseInsensitive );
    mLineEdit->setCompleter( completer );

    connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & value ) { emit valueChanged( value ); } );
  }
}

bool QgsValueRelationWidgetWrapper::valid() const
{
  return mTableWidget || mLineEdit || mComboBox;
}

void QgsValueRelationWidgetWrapper::setValue( const QVariant &value )
{
  if ( mTableWidget )
  {
    QStringList checkList( QgsValueRelationFieldFormatter::valueToStringList( value ) );

    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < config( QStringLiteral( "NofColumns" ) ).toInt() ; ++i )
      {
        QTableWidgetItem *item = mTableWidget->item( j, i );
        if ( item )
        {
          item->setCheckState( checkList.contains( item->data( Qt::UserRole ).toString() ) ? Qt::Checked : Qt::Unchecked );
        }
      }
    }
  }
  else if ( mComboBox )
  {
    mComboBox->setCurrentIndex( mComboBox->findData( value ) );
  }
  else if ( mLineEdit )
  {
    Q_FOREACH ( QgsValueRelationFieldFormatter::ValueRelationItem i, mCache )
    {
      if ( i.key == value )
      {
        mLineEdit->setText( i.value );
        break;
      }
    }
  }
}

void QgsValueRelationWidgetWrapper::showIndeterminateState()
{
  if ( mTableWidget )
  {
    mTableWidget->blockSignals( true );
    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < config( QStringLiteral( "NofColumns" ) ).toInt(); ++i )
      {
        mTableWidget->item( j, i )->setCheckState( Qt::PartiallyChecked );
      }
    }
    mTableWidget->blockSignals( false );
  }
  else if ( mComboBox )
  {
    whileBlocking( mComboBox )->setCurrentIndex( -1 );
  }
  else if ( mLineEdit )
  {
    whileBlocking( mLineEdit )->clear();
  }
}

void QgsValueRelationWidgetWrapper::setEnabled( bool enabled )
{
  if ( mEnabled == enabled )
    return;

  mEnabled = enabled;

  if ( mTableWidget )
  {
    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < mTableWidget->columnCount(); ++i )
      {
        QTableWidgetItem *item = mTableWidget->item( j, i );
        if ( item )
        {
          if ( enabled )
            item->setFlags( item->flags() | Qt::ItemIsEnabled );
          else
            item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
        }
      }
    }
  }
  else
    QgsEditorWidgetWrapper::setEnabled( enabled );
}
