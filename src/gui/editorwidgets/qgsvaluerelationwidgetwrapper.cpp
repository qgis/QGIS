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
#include "qgsattributeform.h"
#include "qgsattributes.h"

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
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &item : qgis::as_const( mCache ) )
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
  QgsAttributeForm *form = dynamic_cast<QgsAttributeForm *>( parent );
  if ( form )
    connect( form, &QgsAttributeForm::widgetValueChanged, this, &QgsValueRelationWidgetWrapper::widgetValueChanged );

  mExpression = config().value( QStringLiteral( "FilterExpression" ) ).toString();

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

  mComboBox = qobject_cast<QComboBox *>( editor );
  mTableWidget = qobject_cast<QTableWidget *>( editor );
  mLineEdit = qobject_cast<QLineEdit *>( editor );

  // Read current initial form values from the editor context
  setFeature( context().formFeature() );

  if ( mComboBox )
  {
    connect( mComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
             this, static_cast<void ( QgsEditorWidgetWrapper::* )()>( &QgsEditorWidgetWrapper::emitValueChanged ), Qt::UniqueConnection );
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
    connect( mTableWidget, &QTableWidget::itemChanged, this, static_cast<void ( QgsEditorWidgetWrapper::* )()>( &QgsEditorWidgetWrapper::emitValueChanged ), Qt::UniqueConnection );
  }
  else if ( mLineEdit )
  {
    connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & value ) { emit valueChanged( value ); }, Qt::UniqueConnection );
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

    QTableWidgetItem *lastChangedItem = nullptr;

    // This block is needed because item->setCheckState triggers dataChanged gets back to value()
    // and iterate over all items again! This can be extremely slow on large items sets.
    mTableWidget->blockSignals( true );
    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < config( QStringLiteral( "NofColumns" ) ).toInt() ; ++i )
      {
        QTableWidgetItem *item = mTableWidget->item( j, i );
        if ( item )
        {
          item->setCheckState( checkList.contains( item->data( Qt::UserRole ).toString() ) ? Qt::Checked : Qt::Unchecked );
          lastChangedItem = item;
        }
      }
    }
    mTableWidget->blockSignals( false );
    // let's trigger the signal now, once and for all
    if ( lastChangedItem )
      lastChangedItem->setCheckState( checkList.contains( lastChangedItem->data( Qt::UserRole ).toString() ) ? Qt::Checked : Qt::Unchecked );

  }
  else if ( mComboBox )
  {
    mComboBox->setCurrentIndex( mComboBox->findData( value ) );
  }
  else if ( mLineEdit )
  {
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &i : qgis::as_const( mCache ) )
    {
      if ( i.key == value )
      {
        mLineEdit->setText( i.value );
        break;
      }
    }
  }
}

void QgsValueRelationWidgetWrapper::widgetValueChanged( const QString &attribute, const QVariant &newValue, bool attributeChanged )
{

  // Do nothing if the value has not changed
  if ( attributeChanged )
  {
    mFeature.setAttribute( attribute, newValue );
    // Update combos if the value used in the filter expression has changed
    if ( QgsValueRelationFieldFormatter::expressionRequiresFormScope( mExpression )
         && QgsValueRelationFieldFormatter::expressionFormAttributes( mExpression ).contains( attribute ) )
    {
      populate();
      // Restore value
      setValue( value( ) );
    }
  }
}


void QgsValueRelationWidgetWrapper::setFeature( const QgsFeature &feature )
{
  mFeature = feature;
  whileBlocking( this )->populate();
  whileBlocking( this )->setValue( feature.attribute( mFieldIdx ) );
  // A bit of logic to set the default value if AllowNull is false and this is a new feature
  // Note that this needs to be here after the cache has been created/updated by populate()
  // and signals unblocked (we want this to propagate to the feature itself)
  if ( mFeature.isValid()
       && ! mFeature.attribute( mFieldIdx ).isValid()
       && mCache.size() > 0
       && ! config( QStringLiteral( "AllowNull" ) ).toBool( ) )
  {
    // This is deferred because at the time the feature is set in one widget it is not
    // set in the next, which is typically the "down" in a drill-down
    QTimer::singleShot( 0, [ = ]
    {
      setValue( mCache.at( 0 ).key );
    } );
  }
}


void QgsValueRelationWidgetWrapper::populate( )
{
  // Initialize, note that signals are blocked, to avoid double signals on new features
  if ( QgsValueRelationFieldFormatter::expressionRequiresFormScope( mExpression ) )
  {
    mCache = QgsValueRelationFieldFormatter::createCache( config( ), mFeature );
  }
  else if ( mCache.isEmpty() )
  {
    mCache = QgsValueRelationFieldFormatter::createCache( config( ) );
  }

  if ( mComboBox )
  {
    mComboBox->clear();
    if ( config( QStringLiteral( "AllowNull" ) ).toBool( ) )
    {
      whileBlocking( mComboBox )->addItem( tr( "(no selection)" ), QVariant( field().type( ) ) );
    }

    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &element : qgis::as_const( mCache ) )
    {
      whileBlocking( mComboBox )->addItem( element.value, element.key );
    }
  }
  else if ( mTableWidget )
  {
    if ( mCache.size() > 0 )
      mTableWidget->setRowCount( ( mCache.size() + config( QStringLiteral( "NofColumns" ) ).toInt() - 1 ) / config( QStringLiteral( "NofColumns" ) ).toInt() );
    else
      mTableWidget->setRowCount( 1 );
    if ( config( QStringLiteral( "NofColumns" ) ).toInt() > 0 )
      mTableWidget->setColumnCount( config( QStringLiteral( "NofColumns" ) ).toInt() );
    else
      mTableWidget->setColumnCount( 1 );

    whileBlocking( mTableWidget )->clear();
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
      whileBlocking( mTableWidget )->setItem( row, column, item );
      column++;
    }
  }
  else if ( mLineEdit )
  {
    QStringList values;
    values.reserve( mCache.size() );
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &i : qgis::as_const( mCache ) )
    {
      values << i.value;
    }
    QStringListModel *m = new QStringListModel( values, mLineEdit );
    QCompleter *completer = new QCompleter( m, mLineEdit );
    completer->setCaseSensitivity( Qt::CaseInsensitive );
    mLineEdit->setCompleter( completer );
  }
}

void QgsValueRelationWidgetWrapper::showIndeterminateState()
{
  if ( mTableWidget )
  {
    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < config( QStringLiteral( "NofColumns" ) ).toInt(); ++i )
      {
        whileBlocking( mTableWidget )->item( j, i )->setCheckState( Qt::PartiallyChecked );
      }
    }
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
    mTableWidget->blockSignals( true );
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
    mTableWidget->blockSignals( false );
  }
  else
    QgsEditorWidgetWrapper::setEnabled( enabled );
}
