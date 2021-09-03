/***************************************************************************
  qgsfeaturelistcombobox.cpp - QgsFeatureListComboBox
 ---------------------
 begin                : 10.3.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfeaturelistcombobox.h"

#include "qgsfeaturefiltermodel.h"
#include "qgsanimatedicon.h"
#include "qgsfilterlineedit.h"
#include "qgslogger.h"
#include "qgsapplication.h"

#include <QCompleter>
#include <QLineEdit>
#include <QKeyEvent>

QgsFeatureListComboBox::QgsFeatureListComboBox( QWidget *parent )
  : QComboBox( parent )
  , mModel( new QgsFeatureFilterModel( this ) )
  , mCompleter( new QCompleter( mModel ) )
{
  mCompleter->setCaseSensitivity( Qt::CaseInsensitive );
  mCompleter->setFilterMode( Qt::MatchContains );
  setEditable( true );
  setCompleter( mCompleter );
  mCompleter->setWidget( this );
  connect( mModel, &QgsFeatureFilterModel::sourceLayerChanged, this, &QgsFeatureListComboBox::sourceLayerChanged );
  connect( mModel, &QgsFeatureFilterModel::displayExpressionChanged, this, &QgsFeatureListComboBox::displayExpressionChanged );
  connect( mModel, &QgsFeatureFilterModel::filterExpressionChanged, this, &QgsFeatureListComboBox::filterExpressionChanged );
  connect( mModel, &QgsFeatureFilterModel::isLoadingChanged, this, &QgsFeatureListComboBox::onLoadingChanged );
  connect( mModel, &QgsFeatureFilterModel::filterJobCompleted, this, &QgsFeatureListComboBox::onFilterUpdateCompleted );
  connect( mModel, &QgsFeatureFilterModel::allowNullChanged, this, &QgsFeatureListComboBox::allowNullChanged );
  connect( mModel, &QgsFeatureFilterModel::extraIdentifierValueChanged, this, &QgsFeatureListComboBox::identifierValueChanged );
  connect( mModel, &QgsFeatureFilterModel::extraIdentifierValueIndexChanged, this, &QgsFeatureListComboBox::setCurrentIndex );
  connect( mModel, &QgsFeatureFilterModel::identifierFieldsChanged, this, &QgsFeatureListComboBox::identifierFieldChanged );
  connect( mCompleter, static_cast<void( QCompleter::* )( const QModelIndex & )>( &QCompleter::highlighted ), this, &QgsFeatureListComboBox::onItemSelected );
  connect( mCompleter, static_cast<void( QCompleter::* )( const QModelIndex & )>( &QCompleter::activated ), this, &QgsFeatureListComboBox::onActivated );
  connect( mModel, &QgsFeatureFilterModel::beginUpdate, this, &QgsFeatureListComboBox::storeLineEditState );
  connect( mModel, &QgsFeatureFilterModel::endUpdate, this, &QgsFeatureListComboBox::restoreLineEditState );
  connect( mModel, &QgsFeatureFilterModel::endUpdate, this, &QgsFeatureListComboBox::modelUpdated );
  connect( mModel, &QgsFeatureFilterModel::dataChanged, this, &QgsFeatureListComboBox::onDataChanged );

  connect( this, static_cast<void( QgsFeatureListComboBox::* )( int )>( &QgsFeatureListComboBox::currentIndexChanged ), this, &QgsFeatureListComboBox::onCurrentIndexChanged );

  mLineEdit = new QgsFilterLineEdit( nullptr, QgsApplication::nullRepresentation() );
  mLineEdit->setSelectOnFocus( true );
  mLineEdit->setShowClearButton( true );

  setLineEdit( mLineEdit );
  setModel( mModel );

  connect( mLineEdit, &QgsFilterLineEdit::textEdited, this, &QgsFeatureListComboBox::onCurrentTextChanged );

  connect( mModel, &QgsFeatureFilterModel::currentFeatureChanged, this, &QgsFeatureListComboBox::currentFeatureChanged );

  setToolTip( tr( "Just start typing what you are looking for." ) );
}

QgsVectorLayer *QgsFeatureListComboBox::sourceLayer() const
{
  return mModel->sourceLayer();
}

void QgsFeatureListComboBox::setSourceLayer( QgsVectorLayer *sourceLayer )
{
  mModel->setSourceLayer( sourceLayer );
}

void QgsFeatureListComboBox::setCurrentFeature( const QgsFeature &feature )
{
  QVariantList values;
  const QStringList fields = mModel->identifierFields();
  for ( const QString &field : fields )
  {
    values << feature.attribute( field );
  }
  setIdentifierValues( values );
}

QString QgsFeatureListComboBox::displayExpression() const
{
  return mModel->displayExpression();
}

void QgsFeatureListComboBox::setDisplayExpression( const QString &expression )
{
  mModel->setDisplayExpression( expression );
}

void QgsFeatureListComboBox::onCurrentTextChanged( const QString &text )
{
  mIsCurrentlyEdited = true;
  mPopupRequested = true;
  mModel->setFilterValue( text );
}

void QgsFeatureListComboBox::onFilterUpdateCompleted()
{
  if ( mPopupRequested )
    mCompleter->complete();

  mPopupRequested = false;
}

void QgsFeatureListComboBox::onLoadingChanged()
{
  mLineEdit->setShowSpinner( mModel->isLoading() );
}

void QgsFeatureListComboBox::onItemSelected( const QModelIndex &index )
{
  setCurrentIndex( index.row() );
}

void QgsFeatureListComboBox::onCurrentIndexChanged( int i )
{
  if ( !mLineEdit->hasStateStored() )
    mIsCurrentlyEdited = false;
  const QModelIndex modelIndex = mModel->index( i, 0, QModelIndex() );
  mModel->setExtraIdentifierValues( mModel->data( modelIndex, QgsFeatureFilterModel::IdentifierValuesRole ).toList() );
  mLineEdit->setText( mModel->data( modelIndex, QgsFeatureFilterModel::ValueRole ).toString() );
  mLineEdit->setFont( mModel->data( modelIndex, Qt::FontRole ).value<QFont>() );
  QPalette palette = mLineEdit->palette();
  palette.setBrush( mLineEdit->foregroundRole(), mModel->data( modelIndex, Qt::ForegroundRole ).value<QBrush>() );
  mLineEdit->setPalette( palette );
}

void QgsFeatureListComboBox::onActivated( QModelIndex modelIndex )
{
  setIdentifierValues( mModel->data( modelIndex, QgsFeatureFilterModel::IdentifierValuesRole ).toList() );
  mLineEdit->setText( mModel->data( modelIndex, QgsFeatureFilterModel::ValueRole ).toString() );
}

void QgsFeatureListComboBox::storeLineEditState()
{
  if ( mIsCurrentlyEdited )
  {
    mLineEdit->storeState( );
  }
}

void QgsFeatureListComboBox::restoreLineEditState()
{
  if ( mIsCurrentlyEdited )
  {
    mLineEdit->restoreState( );
  }
}

int QgsFeatureListComboBox::nullIndex() const
{
  int index = -1;

  if ( allowNull() )
  {
    index = findText( QgsApplication::nullRepresentation( ) );
  }

  return index;
}

void QgsFeatureListComboBox::onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles )
{
  Q_UNUSED( roles )
  if ( !mIsCurrentlyEdited )
  {
    const int currentIndex = mModel->extraIdentifierValueIndex();
    if ( currentIndex >= topLeft.row() && currentIndex <= bottomRight.row() )
    {
      const QModelIndex modelIndex = mModel->index( currentIndex, 0, QModelIndex() );
      mLineEdit->setText( mModel->data( modelIndex, QgsFeatureFilterModel::ValueRole ).toString() );
    }
  }
}

QString QgsFeatureListComboBox::identifierField() const
{
  const QStringList list = mModel->identifierFields();
  if ( list.isEmpty() )
    return QString();
  else
    return list.at( 0 );
}

QStringList QgsFeatureListComboBox::identifierFields() const
{
  return mModel->identifierFields();
}

void QgsFeatureListComboBox::setIdentifierField( const QString &identifierField )
{
  mModel->setIdentifierFields( QStringList() << identifierField );
}

void QgsFeatureListComboBox::setIdentifierFields( const QStringList &identifierFields )
{
  mModel->setIdentifierFields( identifierFields );
}

QModelIndex QgsFeatureListComboBox::currentModelIndex() const
{
  return mModel->index( mModel->extraIdentifierValueIndex(), 0, QModelIndex() );
}

void QgsFeatureListComboBox::focusOutEvent( QFocusEvent *event )
{
  Q_UNUSED( event )
  QComboBox::focusOutEvent( event );
  mLineEdit->setText( mModel->data( currentModelIndex(), QgsFeatureFilterModel::ValueRole ).toString() );
}

void QgsFeatureListComboBox::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape )
  {
    mLineEdit->setText( mModel->data( currentModelIndex(), QgsFeatureFilterModel::ValueRole ).toString() );
  }
  QComboBox::keyReleaseEvent( event );
}

bool QgsFeatureListComboBox::allowNull() const
{
  return mModel->allowNull();
}

void QgsFeatureListComboBox::setAllowNull( bool allowNull )
{
  mModel->setAllowNull( allowNull );
  mLineEdit->setClearMode( allowNull ? QgsFilterLineEdit::ClearToNull : QgsFilterLineEdit::ClearToDefault );
}

QVariant QgsFeatureListComboBox::identifierValue() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mModel->extraIdentifierValues().value( 0 );
  Q_NOWARN_DEPRECATED_POP
}

QVariantList QgsFeatureListComboBox::identifierValues() const
{
  return mModel->extraIdentifierValues();
}

void QgsFeatureListComboBox::setIdentifierValue( const QVariant &identifierValue )
{
  setIdentifierValues( QVariantList() << identifierValue );
}

void QgsFeatureListComboBox::setIdentifierValues( const QVariantList &identifierValues )
{
  mModel->setExtraIdentifierValues( identifierValues );
}

void QgsFeatureListComboBox::setIdentifierValuesToNull()
{
  mModel->setExtraIdentifierValueToNull();
}

QgsFeatureRequest QgsFeatureListComboBox::currentFeatureRequest() const
{
  if ( mModel->extraIdentifierValues().isEmpty() )
  {
    return QgsFeatureRequest().setFilterFids( QgsFeatureIds() ); // NULL: Return a request that's guaranteed to not return anything
  }
  else
  {
    QStringList filtersAttrs;
    const QStringList identifierFields = mModel->identifierFields();
    const QVariantList values = mModel->extraIdentifierValues();
    for ( int i = 0; i < identifierFields.count(); i++ )
    {
      if ( i >= values.count() )
      {
        filtersAttrs << QgsExpression::createFieldEqualityExpression( identifierFields.at( i ), QVariant() );
      }
      else
      {
        filtersAttrs << QgsExpression::createFieldEqualityExpression( identifierFields.at( i ), values.at( i ) );
      }
    }
    const QString expression = filtersAttrs.join( QLatin1String( " AND " ) );
    return QgsFeatureRequest().setFilterExpression( expression );
  }
}

QString QgsFeatureListComboBox::filterExpression() const
{
  return mModel->filterExpression();
}

void QgsFeatureListComboBox::setFilterExpression( const QString &filterExpression )
{
  mModel->setFilterExpression( filterExpression );
}
