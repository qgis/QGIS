/***************************************************************************
    qgsvaluerelationsearchwidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
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

#include <QStringListModel>
#include <QCompleter>

QgsValueRelationSearchWidgetWrapper::QgsValueRelationSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsDefaultSearchWidgetWrapper( vl, fieldIdx, parent )
    , mComboBox( 0 )
    , mListWidget( 0 )
    , mLineEdit( 0 )
    , mLayer( 0 )
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

QVariant QgsValueRelationSearchWidgetWrapper::value()
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

    v = selection.join( "," ).prepend( "{" ).append( "}" );
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

void QgsValueRelationSearchWidgetWrapper::valueChanged()
{
    setExpression(value().toString());
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
    mComboBox->addItem( tr( "Please select" ), QVariant(layer()->pendingFields()[mFieldIdx].type()) );
    if ( config( "AllowNull" ).toBool() )
    {
      mComboBox->addItem( tr( "(no selection)" ), QVariant(layer()->pendingFields()[mFieldIdx].type()) );
    }

    Q_FOREACH ( const ValueRelationItem& element, mCache )
    {
      mComboBox->addItem( element.second, element.first );
    }

    connect( mComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( valueChanged() ) );
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
    connect( mListWidget, SIGNAL( itemChanged( QListWidgetItem* ) ), this, SLOT( valueChanged() ) );
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
    connect( mLineEdit, SIGNAL( textChanged( QListWidgetItem* ) ), this, SLOT( valueChanged() ) );
  }
}


