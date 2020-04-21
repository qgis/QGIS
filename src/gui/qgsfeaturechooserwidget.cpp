/***************************************************************************
  qgsfeaturechooserwidget.cpp - QgsFeatureChooserWidget
 ---------------------
 begin                : 03.04.2020
 copyright            : (C) 2020 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QHBoxLayout>
#include <QKeyEvent>

#include "qgsfeaturechooserwidget.h"
#include "qgsfilterlineedit.h"
#include "qgsfeaturechoosermodel.h"

QgsFeatureChooserWidget::QgsFeatureChooserWidget( QWidget *parent )
  : QWidget( parent )
  , mModel( new QgsFeatureChooserModel( this ) )
  , mCompleter( new QCompleter( mModel ) )
{
  QHBoxLayout *layout = new QHBoxLayout();
  mComboBox = new QComboBox( this );
  mComboBox->setEditable( true );
  layout->addWidget( mComboBox );
  setLayout( layout );

  mCompleter->setCaseSensitivity( Qt::CaseInsensitive );
  mCompleter->setFilterMode( Qt::MatchContains );
  mComboBox->setCompleter( mCompleter );
  mCompleter->setWidget( mComboBox );
  connect( mModel, &QgsFeatureChooserModel::sourceLayerChanged, this, &QgsFeatureChooserWidget::layerChanged );
  connect( mModel, &QgsFeatureChooserModel::displayExpressionChanged, this, &QgsFeatureChooserWidget::displayExpressionChanged );
  connect( mModel, &QgsFeatureChooserModel::filterExpressionChanged, this, &QgsFeatureChooserWidget::filterExpressionChanged );
  connect( mModel, &QgsFeatureChooserModel::isLoadingChanged, this, &QgsFeatureChooserWidget::onLoadingChanged );
  connect( mModel, &QgsFeatureChooserModel::filterJobCompleted, this, &QgsFeatureChooserWidget::onFilterUpdateCompleted );
  connect( mModel, &QgsFeatureChooserModel::allowNullChanged, this, &QgsFeatureChooserWidget::allowNullChanged );
  connect( mModel, &QgsFeatureChooserModel::currentIndexChanged, mComboBox, &QComboBox::setCurrentIndex );
  connect( mModel, &QgsFeatureChooserModel::currentFeatureChanged, this, &QgsFeatureChooserWidget::currentFeatureChanged );
  connect( mCompleter, static_cast<void( QCompleter::* )( const QModelIndex & )>( &QCompleter::highlighted ), this, &QgsFeatureChooserWidget::onItemSelected );
  connect( mCompleter, static_cast<void( QCompleter::* )( const QModelIndex & )>( &QCompleter::activated ), this, &QgsFeatureChooserWidget::onActivated );
  connect( mModel, &QgsFeatureChooserModel::beginUpdate, this, &QgsFeatureChooserWidget::storeLineEditState );
  connect( mModel, &QgsFeatureChooserModel::endUpdate, this, &QgsFeatureChooserWidget::restoreLineEditState );
  connect( mModel, &QgsFeatureChooserModel::endUpdate, this, &QgsFeatureChooserWidget::modelUpdated );
  connect( mModel, &QgsFeatureChooserModel::dataChanged, this, &QgsFeatureChooserWidget::onDataChanged );

  connect( mComboBox, static_cast<void( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsFeatureChooserWidget::onCurrentIndexChanged );

  mLineEdit = new QgsFilterLineEdit( nullptr, QgsApplication::nullRepresentation() );
  mLineEdit->setSelectOnFocus( true );
  mLineEdit->setShowClearButton( allowNull() );

  mComboBox->setEditable( true );
  mComboBox->setLineEdit( mLineEdit );
  mComboBox->setModel( mModel );

  connect( mLineEdit, &QgsFilterLineEdit::textEdited, this, &QgsFeatureChooserWidget::onCurrentTextChanged );

  setToolTip( tr( "Just start typing what you are looking for." ) );
}

QgsVectorLayer *QgsFeatureChooserWidget::layer() const
{
  return mModel->sourceLayer();
}

void QgsFeatureChooserWidget::setLayer( QgsVectorLayer *sourceLayer )
{
  mModel->setSourceLayer( sourceLayer );
}

void QgsFeatureChooserWidget::setCurrentFeature(QgsFeatureId featureId )
{
  mModel->setCurrentFeature( featureId );
}

QString QgsFeatureChooserWidget::displayExpression() const
{
  return mModel->displayExpression();
}

void QgsFeatureChooserWidget::setDisplayExpression( const QString &expression )
{
  mModel->setDisplayExpression( expression );
}

void QgsFeatureChooserWidget::onCurrentTextChanged( const QString &text )
{
  mIsCurrentlyEdited = true;
  mPopupRequested = true;
  mModel->setFilterValue( text );
}

void QgsFeatureChooserWidget::onFilterUpdateCompleted()
{
  if ( mPopupRequested )
    mCompleter->complete();

  mPopupRequested = false;
}

void QgsFeatureChooserWidget::onLoadingChanged()
{
  mLineEdit->setShowSpinner( mModel->isLoading() );
}

void QgsFeatureChooserWidget::onItemSelected( const QModelIndex &index )
{
  mComboBox->setCurrentIndex( index.row() );
}

void QgsFeatureChooserWidget::onCurrentIndexChanged( int i )
{
  if ( !mHasStoredEditState )
    mIsCurrentlyEdited = false;
  QModelIndex modelIndex = mModel->index( i, 0, QModelIndex() );
  mModel->setCurrentFeature( mModel->data( modelIndex, QgsFeatureChooserModel::FeatureIdRole ).value<QgsFeatureId>() );
  mLineEdit->setText( mModel->data( modelIndex, QgsFeatureChooserModel::ValueRole ).toString() );
  mLineEdit->setFont( mModel->data( modelIndex, Qt::FontRole ).value<QFont>() );
  QPalette palette = mLineEdit->palette();
  palette.setBrush( mLineEdit->foregroundRole(), mModel->data( modelIndex, Qt::ForegroundRole ).value<QBrush>() );
  mLineEdit->setPalette( palette );
}

void QgsFeatureChooserWidget::onActivated( QModelIndex modelIndex )
{
  setCurrentFeature( mModel->data( modelIndex, QgsFeatureChooserModel::FeatureIdRole ).value<QgsFeatureId>() );
  mLineEdit->setText( mModel->data( modelIndex, QgsFeatureChooserModel::ValueRole ).toString() );
}

void QgsFeatureChooserWidget::storeLineEditState()
{
  if ( mIsCurrentlyEdited )
  {
    mHasStoredEditState = true;
    mLineEditState.store( mLineEdit );
  }
}

void QgsFeatureChooserWidget::restoreLineEditState()
{
  if ( mIsCurrentlyEdited )
  {
    mHasStoredEditState = false;
    mLineEditState.restore( mLineEdit );
  }
}

int QgsFeatureChooserWidget::nullIndex() const
{
  int index = -1;

  if ( allowNull() )
  {
    index = mComboBox->findText( QgsApplication::nullRepresentation( ) );
  }

  return index;
}

void QgsFeatureChooserWidget::onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles )
{
  Q_UNUSED( roles )
  if ( !mIsCurrentlyEdited )
  {
    const int currentIndex = mModel->currentIndex();
    if ( currentIndex >= topLeft.row() && currentIndex <= bottomRight.row() )
    {
      QModelIndex modelIndex = mModel->index( currentIndex, 0, QModelIndex() );
      mLineEdit->setText( mModel->data( modelIndex, QgsFeatureChooserModel::ValueRole ).toString() );
    }
  }
}

QModelIndex QgsFeatureChooserWidget::currentModelIndex() const
{
  return mModel->index( mModel->currentIndex(), 0, QModelIndex() );
}

void QgsFeatureChooserWidget::focusOutEvent( QFocusEvent *event )
{
  Q_UNUSED( event )
  QWidget::focusOutEvent( event );
  mLineEdit->setText( mModel->data( currentModelIndex(), QgsFeatureChooserModel::ValueRole ).toString() );
}

void QgsFeatureChooserWidget::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape )
  {
    mLineEdit->setText( mModel->data( currentModelIndex(), QgsFeatureChooserModel::ValueRole ).toString() );
  }
  QWidget::keyReleaseEvent( event );
}

bool QgsFeatureChooserWidget::allowNull() const
{
  return mModel->allowNull();
}

void QgsFeatureChooserWidget::setAllowNull( bool allowNull )
{
  mModel->setAllowNull( allowNull );
  mLineEdit->setClearMode( allowNull ? QgsFilterLineEdit::ClearToNull : QgsFilterLineEdit::ClearToDefault );
}

QString QgsFeatureChooserWidget::filterExpression() const
{
  return mModel->filterExpression();
}

void QgsFeatureChooserWidget::setFilterExpression( const QString &filterExpression )
{
  mModel->setFilterExpression( filterExpression );
}

void QgsFeatureChooserWidget::LineEditState::store( QLineEdit *lineEdit )
{
  text = lineEdit->text();
  selectionStart = lineEdit->selectionStart();
  selectionLength = lineEdit->selectedText().length();
  cursorPosition = lineEdit->cursorPosition();

}

void QgsFeatureChooserWidget::LineEditState::restore( QLineEdit *lineEdit ) const
{
  lineEdit->setText( text );
  lineEdit->setCursorPosition( cursorPosition );
  if ( selectionStart > -1 )
    lineEdit->setSelection( selectionStart, selectionLength );
}
