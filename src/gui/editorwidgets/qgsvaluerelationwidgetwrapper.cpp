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

QgsValueRelationWidgetWrapper::QgsValueRelationWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )
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

  const int nofColumns = columnCount();

  if ( mTableWidget )
  {
    QStringList selection;
    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < nofColumns; ++i )
      {
        QTableWidgetItem *item = mTableWidget->item( j, i );
        if ( item )
        {
          if ( item->checkState() == Qt::Checked )
            selection << item->data( Qt::UserRole ).toString();
        }
      }
    }

    if ( layer()->fields().at( fieldIdx() ).type() == QVariant::Map )
    {
      QVariantList vl;
      //store as QVariantList because it's json
      for ( const QString &s : qgis::as_const( selection ) )
      {
        vl << s;
      }
      v = vl;
    }
    else
    {
      //store as hstore string
      v = selection.join( ',' ).prepend( '{' ).append( '}' );
    }
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
  QgsAttributeForm *form = qobject_cast<QgsAttributeForm *>( parent );
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
    QStringList checkList;

    if ( layer()->fields().at( fieldIdx() ).type() == QVariant::Map )
    {
      //because of json it's stored as QVariantList
      checkList = value.toStringList();
    }
    else
    {
      checkList = QgsValueRelationFieldFormatter::valueToStringList( value );
    }

    QTableWidgetItem *lastChangedItem = nullptr;

    const int nofColumns = columnCount();

    // This block is needed because item->setCheckState triggers dataChanged gets back to value()
    // and iterate over all items again! This can be extremely slow on large items sets.
    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      auto signalBlockedTableWidget = whileBlocking( mTableWidget );
      Q_UNUSED( signalBlockedTableWidget )

      for ( int i = 0; i < nofColumns; ++i )
      {
        QTableWidgetItem *item = mTableWidget->item( j, i );
        if ( item )
        {
          item->setCheckState( checkList.contains( item->data( Qt::UserRole ).toString() ) ? Qt::Checked : Qt::Unchecked );
          //re-set enabled state because it's lost after reloading items
          item->setFlags( mEnabled ? item->flags() | Qt::ItemIsEnabled : item->flags() & ~Qt::ItemIsEnabled );
          lastChangedItem = item;
        }
      }
    }
    // let's trigger the signal now, once and for all
    if ( lastChangedItem )
      lastChangedItem->setCheckState( checkList.contains( lastChangedItem->data( Qt::UserRole ).toString() ) ? Qt::Checked : Qt::Unchecked );

  }
  else if ( mComboBox )
  {
    // findData fails to tell a 0 from a NULL
    // See: "Value relation, value 0 = NULL" - https://issues.qgis.org/issues/19981
    int idx = -1; // default to not found
    for ( int i = 0; i < mComboBox->count(); i++ )
    {
      QVariant v( mComboBox->itemData( i ) );
      if ( qgsVariantEqual( v, value ) )
      {
        idx = i;
        break;
      }
    }
    mComboBox->setCurrentIndex( idx );
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
    QVariant oldValue( value( ) );
    setFormFeatureAttribute( attribute, newValue );
    // Update combos if the value used in the filter expression has changed
    if ( QgsValueRelationFieldFormatter::expressionRequiresFormScope( mExpression )
         && QgsValueRelationFieldFormatter::expressionFormAttributes( mExpression ).contains( attribute ) )
    {
      populate();
      // Restore value
      setValue( value( ) );
      // If the value has changed as a result of another widget's value change,
      // we need to emit the signal to make sure other dependent widgets are
      // updated.
      if ( oldValue != value() && fieldIdx() < formFeature().fields().count() )
      {
        QString attributeName( formFeature().fields().names().at( fieldIdx() ) );
        setFormFeatureAttribute( attributeName, value( ) );
        emitValueChanged( );
      }
    }
  }
}


void QgsValueRelationWidgetWrapper::setFeature( const QgsFeature &feature )
{
  setFormFeature( feature );
  whileBlocking( this )->populate();
  whileBlocking( this )->setValue( feature.attribute( fieldIdx() ) );
  // A bit of logic to set the default value if AllowNull is false and this is a new feature
  // Note that this needs to be here after the cache has been created/updated by populate()
  // and signals unblocked (we want this to propagate to the feature itself)
  if ( formFeature().isValid()
       && ! formFeature().attribute( fieldIdx() ).isValid()
       && ! mCache.empty()
       && ! config( QStringLiteral( "AllowNull" ) ).toBool( ) )
  {
    // This is deferred because at the time the feature is set in one widget it is not
    // set in the next, which is typically the "down" in a drill-down
    QTimer::singleShot( 0, this, [ this ]
    {
      setValue( mCache.at( 0 ).key );
    } );
  }
}

int QgsValueRelationWidgetWrapper::columnCount() const
{
  return std::max( 1, config( QStringLiteral( "NofColumns" ) ).toInt() );
}

void QgsValueRelationWidgetWrapper::populate( )
{
  // Initialize, note that signals are blocked, to avoid double signals on new features
  if ( QgsValueRelationFieldFormatter::expressionRequiresFormScope( mExpression ) )
  {
    mCache = QgsValueRelationFieldFormatter::createCache( config( ), formFeature() );
  }
  else if ( mCache.empty() )
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
    const int nofColumns = columnCount();

    if ( ! mCache.empty() )
    {
      mTableWidget->setRowCount( ( mCache.size() + nofColumns - 1 ) / nofColumns );
    }
    else
      mTableWidget->setRowCount( 1 );
    mTableWidget->setColumnCount( nofColumns );

    whileBlocking( mTableWidget )->clear();
    int row = 0;
    int column = 0;
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &element : qgis::as_const( mCache ) )
    {
      if ( column == nofColumns )
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
  const int nofColumns = columnCount();

  if ( mTableWidget )
  {
    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < nofColumns; ++i )
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
    auto signalBlockedTableWidget = whileBlocking( mTableWidget );
    Q_UNUSED( signalBlockedTableWidget )

    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < mTableWidget->columnCount(); ++i )
      {
        QTableWidgetItem *item = mTableWidget->item( j, i );
        if ( item )
        {
          item->setFlags( enabled ? item->flags() | Qt::ItemIsEnabled : item->flags() & ~Qt::ItemIsEnabled );
        }
      }
    }
  }
  else
    QgsEditorWidgetWrapper::setEnabled( enabled );
}
