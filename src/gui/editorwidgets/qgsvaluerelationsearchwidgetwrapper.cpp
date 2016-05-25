/***************************************************************************
    qgsvaluerelationsearchwidgetwrapper.cpp
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

#include "qgsvaluerelationsearchwidgetwrapper.h"

#include "qgsfield.h"
#include "qgsmaplayerregistry.h"
#include "qgsvaluerelationwidgetfactory.h"
#include "qgsvectorlayer.h"
#include "qgsfilterlineedit.h"

#include <QSettings>
#include <QStringListModel>
#include <QCompleter>

QgsValueRelationSearchWidgetWrapper::QgsValueRelationSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsSearchWidgetWrapper( vl, fieldIdx, parent )
    , mComboBox( nullptr )
    , mListWidget( nullptr )
    , mLineEdit( nullptr )
    , mLayer( nullptr )
{
}

bool QgsValueRelationSearchWidgetWrapper::applyDirectly()
{
  if ( mLineEdit )
  {
    return false;
  }
  return true;
}

QString QgsValueRelationSearchWidgetWrapper::expression()
{
  return mExpression;
}

QVariant QgsValueRelationSearchWidgetWrapper::value() const
{
  QVariant v;

  if ( mComboBox )
  {
    int cbxIdx = mComboBox->currentIndex();
    if ( cbxIdx > -1 )
    {
      v = mComboBox->itemData( mComboBox->currentIndex() );
    }
  }

  if ( mListWidget )
  {
    QStringList selection;
    for ( int i = 0; i < mListWidget->count(); ++i )
    {
      QListWidgetItem* item = mListWidget->item( i );
      if ( item->checkState() == Qt::Checked )
        selection << item->data( Qt::UserRole ).toString();
    }

    v = selection.join( "," ).prepend( '{' ).append( '}' );
  }

  if ( mLineEdit )
  {
    Q_FOREACH ( const ValueRelationItem& i , mCache )
    {
      if ( i.second == mLineEdit->text() )
      {
        v = i.first;
        break;
      }
    }
  }

  return v;
}

QgsSearchWidgetWrapper::FilterFlags QgsValueRelationSearchWidgetWrapper::supportedFlags() const
{
  return EqualTo | NotEqualTo | IsNull | IsNotNull;
}

QgsSearchWidgetWrapper::FilterFlags QgsValueRelationSearchWidgetWrapper::defaultFlags() const
{
  return EqualTo;
}

QString QgsValueRelationSearchWidgetWrapper::createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const
{
  QString fieldName = QgsExpression::quotedColumnRef( layer()->fields().at( mFieldIdx ).name() );

  //clear any unsupported flags
  flags &= supportedFlags();
  if ( flags & IsNull )
    return fieldName + " IS NULL";
  if ( flags & IsNotNull )
    return fieldName + " IS NOT NULL";

  QVariant v = value();
  if ( !v.isValid() )
    return QString();

  switch ( v.type() )
  {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    {
      if ( flags & EqualTo )
        return fieldName + '=' + v.toString();
      else if ( flags & NotEqualTo )
        return fieldName + "<>" + v.toString();
      break;
    }

    default:
    {
      if ( flags & EqualTo )
        return fieldName + "='" + v.toString() + '\'';
      else if ( flags & NotEqualTo )
        return fieldName + "<>'" + v.toString() + '\'';
      break;
    }
  }

  return QString();
}

void QgsValueRelationSearchWidgetWrapper::clearWidget()
{
  if ( mComboBox )
  {
    mComboBox->setCurrentIndex( 0 );
  }
  if ( mListWidget )
  {
    mListWidget->clearSelection();
  }
  if ( mLineEdit )
  {
    mLineEdit->setText( QString() );
  }
}

void QgsValueRelationSearchWidgetWrapper::setEnabled( bool enabled )
{
  if ( mComboBox )
  {
    mComboBox->setEnabled( enabled );
  }
  if ( mListWidget )
  {
    mListWidget->setEnabled( enabled );
  }
  if ( mLineEdit )
  {
    mLineEdit->setEnabled( enabled );
  }
}

bool QgsValueRelationSearchWidgetWrapper::valid() const
{
  return true;
}

void QgsValueRelationSearchWidgetWrapper::onValueChanged()
{
  QVariant vl = value();
  if ( !vl.isValid() )
  {
    clearExpression();
    emit valueCleared();
  }
  else
  {
    QSettings settings;
    setExpression( vl.isNull() ? settings.value( "qgis/nullValue", "NULL" ).toString() : vl.toString() );
    emit valueChanged();
  }
  emit expressionChanged( mExpression );
}

void QgsValueRelationSearchWidgetWrapper::setExpression( QString exp )
{
  QSettings settings;
  QString nullValue = settings.value( "qgis/nullValue", "NULL" ).toString();
  QString fieldName = layer()->fields().at( mFieldIdx ).name();

  QString str;
  if ( exp == nullValue )
  {
    str = QString( "%1 IS NULL" ).arg( QgsExpression::quotedColumnRef( fieldName ) );
  }
  else
  {
    str = QString( "%1 = '%3'" )
          .arg( QgsExpression::quotedColumnRef( fieldName ),
                exp.replace( '\'', "''" )
              );
  }
  mExpression = str;
}

QWidget* QgsValueRelationSearchWidgetWrapper::createWidget( QWidget* parent )
{
  if ( config( "AllowMulti" ).toBool() )
  {
    return new QgsFilterLineEdit( parent );
  }
  else if ( config( "UseCompleter" ).toBool() )
  {
    return new QgsFilterLineEdit( parent );
  }
  else
  {
    return new QComboBox( parent );
  }
}

void QgsValueRelationSearchWidgetWrapper::initWidget( QWidget* editor )
{
  mCache = QgsValueRelationWidgetWrapper::createCache( config() );

  mComboBox = qobject_cast<QComboBox*>( editor );
  mListWidget = qobject_cast<QListWidget*>( editor );
  mLineEdit = qobject_cast<QLineEdit*>( editor );

  if ( mComboBox )
  {
    mComboBox->addItem( tr( "Please select" ), QVariant() ); // creates an invalid to allow selecting all features
    if ( config( "AllowNull" ).toBool() )
    {
      mComboBox->addItem( tr( "(no selection)" ), QVariant( layer()->fields().at( mFieldIdx ).type() ) );
    }

    Q_FOREACH ( const ValueRelationItem& element, mCache )
    {
      mComboBox->addItem( element.second, element.first );
    }

    connect( mComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onValueChanged() ) );
  }
  else if ( mListWidget )
  {
    Q_FOREACH ( const ValueRelationItem& element, mCache )
    {
      QListWidgetItem *item;
      item = new QListWidgetItem( element.second );
      item->setData( Qt::UserRole, element.first );

      mListWidget->addItem( item );
    }
    connect( mListWidget, SIGNAL( itemChanged( QListWidgetItem* ) ), this, SLOT( onValueChanged() ) );
  }
  else if ( mLineEdit )
  {
    QStringList values;
    Q_FOREACH ( const ValueRelationItem& i,  mCache )
    {
      values << i.second;
    }

    QStringListModel* m = new QStringListModel( values, mLineEdit );
    QCompleter* completer = new QCompleter( m, mLineEdit );
    completer->setCaseSensitivity( Qt::CaseInsensitive );
    mLineEdit->setCompleter( completer );
    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( onValueChanged() ) );
  }
}


