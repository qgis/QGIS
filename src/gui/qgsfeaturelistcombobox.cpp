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
  connect( mModel, &QgsFeatureFilterModel::extraIdentifierValueChanged, this, &QgsFeatureListComboBox::identifierValueChanged );
  connect( mModel, &QgsFeatureFilterModel::extraIdentifierValueIndexChanged, this, &QgsFeatureListComboBox::setCurrentIndex );
  connect( mModel, &QgsFeatureFilterModel::identifierFieldChanged, this, &QgsFeatureListComboBox::identifierFieldChanged );
  connect( mCompleter, static_cast<void( QCompleter::* )( const QModelIndex & )>( &QCompleter::highlighted ), this, &QgsFeatureListComboBox::onItemSelected );
  connect( mCompleter, static_cast<void( QCompleter::* )( const QModelIndex & )>( &QCompleter::activated ), this, &QgsFeatureListComboBox::onActivated );
  connect( mModel, &QgsFeatureFilterModel::beginUpdate, this, &QgsFeatureListComboBox::storeLineEditState );
  connect( mModel, &QgsFeatureFilterModel::endUpdate, this, &QgsFeatureListComboBox::restoreLineEditState );

  connect( this, static_cast<void( QgsFeatureListComboBox::* )( int )>( &QgsFeatureListComboBox::currentIndexChanged ), this, &QgsFeatureListComboBox::onCurrentIndexChanged );

  mLineEdit = new QgsFilterLineEdit();
  setEditable( true );
  setLineEdit( mLineEdit );
  setModel( mModel );

  connect( mLineEdit, &QgsFilterLineEdit::textEdited, this, &QgsFeatureListComboBox::onCurrentTextChanged );

  connect( mLineEdit, &QgsFilterLineEdit::textChanged, this, []( const QString & text )
  {
    QgsDebugMsg( QStringLiteral( "Edit text changed to %1" ).arg( text ) );
  } );
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
  QModelIndex modelIndex = mModel->index( i, 0, QModelIndex() );
  mModel->setExtraIdentifierValue( mModel->data( modelIndex, QgsFeatureFilterModel::IdentifierValueRole ) );
  mLineEdit->setText( mModel->data( modelIndex, QgsFeatureFilterModel::ValueRole ).toString() );
}

void QgsFeatureListComboBox::onActivated( QModelIndex modelIndex )
{
  setIdentifierValue( mModel->data( modelIndex, QgsFeatureFilterModel::IdentifierValueRole ) );
  QgsDebugMsg( QStringLiteral( "Activated index" ) );
  QgsDebugMsg( QStringLiteral( "%1 %2" ).arg( QString::number( modelIndex.row() ), mModel->data( modelIndex, QgsFeatureFilterModel::ValueRole ).toString() ) );
  mLineEdit->setText( mModel->data( modelIndex, QgsFeatureFilterModel::ValueRole ).toString() );
}

void QgsFeatureListComboBox::storeLineEditState()
{
  mLineEditState.store( mLineEdit );
}

void QgsFeatureListComboBox::restoreLineEditState()
{
  mLineEditState.restore( mLineEdit );
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
  return mAllowNull;
}

void QgsFeatureListComboBox::setAllowNull( bool allowNull )
{
  if ( mAllowNull == allowNull )
    return;

  mAllowNull = allowNull;
  emit allowNullChanged();
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
  lineEdit->setSelection( selectionStart, selectionLength );
}
