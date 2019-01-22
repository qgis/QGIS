/***************************************************************************
  qgsfieldlistcombobox.cpp - QgsFieldListComboBox

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
  connect( mModel, &QgsFeatureFilterModel::identifierFieldChanged, this, &QgsFeatureListComboBox::identifierFieldChanged );
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

  setEditable( true );
  setLineEdit( mLineEdit );
  setModel( mModel );

  connect( mLineEdit, &QgsFilterLineEdit::textEdited, this, &QgsFeatureListComboBox::onCurrentTextChanged );

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
  if ( !mHasStoredEditState )
    mIsCurrentlyEdited = false;
  QModelIndex modelIndex = mModel->index( i, 0, QModelIndex() );
  mModel->setExtraIdentifierValue( mModel->data( modelIndex, QgsFeatureFilterModel::IdentifierValueRole ) );
  mLineEdit->setText( mModel->data( modelIndex, QgsFeatureFilterModel::ValueRole ).toString() );
  mLineEdit->setFont( mModel->data( modelIndex, Qt::FontRole ).value<QFont>() );
  QPalette palette = mLineEdit->palette();
  palette.setBrush( mLineEdit->foregroundRole(), mModel->data( modelIndex, Qt::ForegroundRole ).value<QBrush>() );
  mLineEdit->setPalette( palette );
}

void QgsFeatureListComboBox::onActivated( QModelIndex modelIndex )
{
  setIdentifierValue( mModel->data( modelIndex, QgsFeatureFilterModel::IdentifierValueRole ) );
  mLineEdit->setText( mModel->data( modelIndex, QgsFeatureFilterModel::ValueRole ).toString() );
}

void QgsFeatureListComboBox::storeLineEditState()
{
  if ( mIsCurrentlyEdited )
  {
    mHasStoredEditState = true;
    mLineEditState.store( mLineEdit );
  }
}

void QgsFeatureListComboBox::restoreLineEditState()
{
  if ( mIsCurrentlyEdited )
  {
    mHasStoredEditState = false;
    mLineEditState.restore( mLineEdit );
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
      QModelIndex modelIndex = mModel->index( currentIndex, 0, QModelIndex() );
      mLineEdit->setText( mModel->data( modelIndex, QgsFeatureFilterModel::ValueRole ).toString() );
    }
  }
}

QString QgsFeatureListComboBox::identifierField() const
{
  return mModel->identifierField();
}

void QgsFeatureListComboBox::setIdentifierField( const QString &identifierField )
{
  mModel->setIdentifierField( identifierField );
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
}

QVariant QgsFeatureListComboBox::identifierValue() const
{
  return mModel->extraIdentifierValue();
}

void QgsFeatureListComboBox::setIdentifierValue( const QVariant &identifierValue )
{
  mModel->setExtraIdentifierValue( identifierValue );
}

QgsFeatureRequest QgsFeatureListComboBox::currentFeatureRequest() const
{
  if ( mModel->extraIdentifierValue().isNull() )
    return QgsFeatureRequest().setFilterFids( QgsFeatureIds() ); // NULL: Return a request that's guaranteed to not return anything
  else
    return QgsFeatureRequest().setFilterExpression( QStringLiteral( "%1 = %2" ).arg( QgsExpression::quotedColumnRef( mModel->identifierField() ), QgsExpression::quotedValue( mModel->extraIdentifierValue() ) ) );
}

QString QgsFeatureListComboBox::filterExpression() const
{
  return mModel->filterExpression();
}

void QgsFeatureListComboBox::setFilterExpression( const QString &filterExpression )
{
  mModel->setFilterExpression( filterExpression );
}

void QgsFeatureListComboBox::LineEditState::store( QLineEdit *lineEdit )
{
  text = lineEdit->text();
  selectionStart = lineEdit->selectionStart();
  selectionLength = lineEdit->selectedText().length();
  cursorPosition = lineEdit->cursorPosition();

}

void QgsFeatureListComboBox::LineEditState::restore( QLineEdit *lineEdit ) const
{
  lineEdit->setText( text );
  lineEdit->setCursorPosition( cursorPosition );
  if ( selectionStart > -1 )
    lineEdit->setSelection( selectionStart, selectionLength );
}
