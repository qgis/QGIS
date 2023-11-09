/***************************************************************************
  qgsfeaturepickerwidget.cpp - QgsFeaturePickerWidget
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
#include <QToolButton>
#include <QKeyEvent>

#include "qgsfeaturepickerwidget.h"
#include "qgsfilterlineedit.h"
#include "qgsfeaturepickermodel.h"

QgsFeaturePickerWidget::QgsFeaturePickerWidget( QWidget *parent )
  : QWidget( parent )
  , mModel( new QgsFeaturePickerModel( this ) )
  , mCompleter( new QCompleter( mModel ) )
{
  QHBoxLayout *layout = new QHBoxLayout();
  mComboBox = new QComboBox( this );
  mComboBox->setEditable( true );
  layout->addWidget( mComboBox );

  mPreviousButton = new QToolButton( this );
  mPreviousButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionArrowLeft.svg" ) ) );
  mPreviousButton->setEnabled( false );
  mPreviousButton->setVisible( mShowBrowserButtons );
  layout->addWidget( mPreviousButton );

  mNextButton = new QToolButton( this );
  mNextButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionArrowRight.svg" ) ) );
  mNextButton->setEnabled( false );
  mNextButton->setVisible( mShowBrowserButtons );
  layout->addWidget( mNextButton );
  layout->setContentsMargins( 0, 0, 0, 0 );

  setLayout( layout );

  mCompleter->setCaseSensitivity( Qt::CaseInsensitive );
  mCompleter->setFilterMode( Qt::MatchContains );
  mComboBox->setCompleter( mCompleter );
  mCompleter->setWidget( mComboBox );
  connect( mModel, &QgsFeaturePickerModel::sourceLayerChanged, this, &QgsFeaturePickerWidget::layerChanged );
  connect( mModel, &QgsFeaturePickerModel::displayExpressionChanged, this, &QgsFeaturePickerWidget::displayExpressionChanged );
  connect( mModel, &QgsFeaturePickerModel::filterExpressionChanged, this, &QgsFeaturePickerWidget::filterExpressionChanged );
  connect( mModel, &QgsFeaturePickerModel::isLoadingChanged, this, &QgsFeaturePickerWidget::onLoadingChanged );
  connect( mModel, &QgsFeaturePickerModel::filterJobCompleted, this, &QgsFeaturePickerWidget::onFilterUpdateCompleted );
  connect( mModel, &QgsFeaturePickerModel::allowNullChanged, this, &QgsFeaturePickerWidget::allowNullChanged );
  connect( mModel, &QgsFeaturePickerModel::fetchGeometryChanged, this, &QgsFeaturePickerWidget::fetchGeometryChanged );
  connect( mModel, &QgsFeaturePickerModel::fetchLimitChanged, this, &QgsFeaturePickerWidget::fetchLimitChanged );
  connect( mModel, &QgsFeaturePickerModel::extraIdentifierValueIndexChanged, mComboBox, &QComboBox::setCurrentIndex );
  connect( mModel, &QgsFeaturePickerModel::featureChanged, this, &QgsFeaturePickerWidget::featureChanged );
  connect( mCompleter, static_cast<void( QCompleter::* )( const QModelIndex & )>( &QCompleter::highlighted ), this, &QgsFeaturePickerWidget::onItemSelected );
  connect( mCompleter, static_cast<void( QCompleter::* )( const QModelIndex & )>( &QCompleter::activated ), this, &QgsFeaturePickerWidget::onActivated );
  connect( mModel, &QgsFeaturePickerModel::beginUpdate, this, &QgsFeaturePickerWidget::storeLineEditState );
  connect( mModel, &QgsFeaturePickerModel::endUpdate, this, &QgsFeaturePickerWidget::restoreLineEditState );
  connect( mModel, &QgsFeaturePickerModel::endUpdate, this, &QgsFeaturePickerWidget::modelUpdated );
  connect( mModel, &QgsFeaturePickerModel::dataChanged, this, &QgsFeaturePickerWidget::onDataChanged );

  connect( mComboBox, static_cast<void( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsFeaturePickerWidget::onCurrentIndexChanged );

  connect( mPreviousButton, &QToolButton::clicked, this, [ = ]() {browseFeatures( -1 );} );
  connect( mNextButton, &QToolButton::clicked, this, [ = ]() {browseFeatures( 1 );} );

  mLineEdit = new QgsFilterLineEdit( nullptr, QgsApplication::nullRepresentation() );
  mLineEdit->setSelectOnFocus( true );
  mLineEdit->setShowClearButton( allowNull() );

  mComboBox->setEditable( true );
  mComboBox->setLineEdit( mLineEdit );
  mComboBox->setModel( mModel );

  connect( mLineEdit, &QgsFilterLineEdit::textEdited, this, &QgsFeaturePickerWidget::onCurrentTextChanged );
}

QgsVectorLayer *QgsFeaturePickerWidget::layer() const
{
  return mModel->sourceLayer();
}

void QgsFeaturePickerWidget::setLayer( QgsVectorLayer *sourceLayer )
{
  mModel->setSourceLayer( sourceLayer );
}

void QgsFeaturePickerWidget::setFeature( QgsFeatureId featureId )
{
  mModel->setFeature( featureId );
}

QgsFeature QgsFeaturePickerWidget::feature() const
{
  return mModel->feature();
}

QString QgsFeaturePickerWidget::displayExpression() const
{
  return mModel->displayExpression();
}

void QgsFeaturePickerWidget::setDisplayExpression( const QString &expression )
{
  mModel->setDisplayExpression( expression );
}

void QgsFeaturePickerWidget::onCurrentTextChanged( const QString &text )
{
  mIsCurrentlyEdited = true;
  mPopupRequested = true;
  mModel->setFilterValue( text );
}

void QgsFeaturePickerWidget::onFilterUpdateCompleted()
{
  if ( mPopupRequested )
    mCompleter->complete();

  mPopupRequested = false;
}

void QgsFeaturePickerWidget::onLoadingChanged()
{
  mLineEdit->setShowSpinner( mModel->isLoading() );
}

void QgsFeaturePickerWidget::onItemSelected( const QModelIndex &index )
{
  mComboBox->setCurrentIndex( index.row() );
}

void QgsFeaturePickerWidget::onCurrentIndexChanged( int i )
{
  if ( !mLineEdit->hasStateStored() )
    mIsCurrentlyEdited = false;

  mPreviousButton->setEnabled( i > 0 );
  mNextButton->setEnabled( i < mComboBox->model()->rowCount() - 1 );

  if ( i < 0 )
    return;

  const QModelIndex modelIndex = mModel->index( i, 0, QModelIndex() );
  mModel->setFeature( mModel->data( modelIndex, QgsFeaturePickerModel::FeatureIdRole ).value<QgsFeatureId>() );
  mLineEdit->setText( mModel->data( modelIndex, QgsFeaturePickerModel::ValueRole ).toString() );
  mLineEdit->setFont( mModel->data( modelIndex, Qt::FontRole ).value<QFont>() );
  QPalette palette = mLineEdit->palette();
  palette.setBrush( mLineEdit->foregroundRole(), mModel->data( modelIndex, Qt::ForegroundRole ).value<QBrush>() );
  mLineEdit->setPalette( palette );
}

void QgsFeaturePickerWidget::onActivated( QModelIndex modelIndex )
{
  setFeature( mModel->data( modelIndex, QgsFeaturePickerModel::FeatureIdRole ).value<QgsFeatureId>() );
  mLineEdit->setText( mModel->data( modelIndex, QgsFeaturePickerModel::ValueRole ).toString() );
}

void QgsFeaturePickerWidget::storeLineEditState()
{
  if ( mIsCurrentlyEdited )
  {
    mLineEdit->storeState( );
  }
}

void QgsFeaturePickerWidget::restoreLineEditState()
{
  if ( mIsCurrentlyEdited )
  {
    mLineEdit->restoreState( );
  }
}

int QgsFeaturePickerWidget::nullIndex() const
{
  int index = -1;

  if ( allowNull() )
  {
    index = mComboBox->findText( QgsApplication::nullRepresentation( ) );
  }

  return index;
}

void QgsFeaturePickerWidget::onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles )
{
  Q_UNUSED( roles )
  if ( !mIsCurrentlyEdited )
  {
    const int currentIndex = mModel->extraIdentifierValueIndex();
    if ( currentIndex >= topLeft.row() && currentIndex <= bottomRight.row() )
    {
      const QModelIndex modelIndex = mModel->index( currentIndex, 0, QModelIndex() );
      mLineEdit->setText( mModel->data( modelIndex, QgsFeaturePickerModel::ValueRole ).toString() );
    }
  }
}

void QgsFeaturePickerWidget::browseFeatures( int direction )
{
  const int newIndex = std::min( std::max( 0, mComboBox->currentIndex() + direction ), mComboBox->model()->rowCount() - 1 );
  mComboBox->setCurrentIndex( newIndex );
}

QModelIndex QgsFeaturePickerWidget::currentModelIndex() const
{
  return mModel->index( mModel->extraIdentifierValueIndex(), 0, QModelIndex() );
}

void QgsFeaturePickerWidget::focusOutEvent( QFocusEvent *event )
{
  Q_UNUSED( event )
  QWidget::focusOutEvent( event );
  mLineEdit->setText( mModel->data( currentModelIndex(), QgsFeaturePickerModel::ValueRole ).toString() );
}

void QgsFeaturePickerWidget::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape )
  {
    mLineEdit->setText( mModel->data( currentModelIndex(), QgsFeaturePickerModel::ValueRole ).toString() );
  }
  QWidget::keyReleaseEvent( event );
}

bool QgsFeaturePickerWidget::allowNull() const
{
  return mModel->allowNull();
}

void QgsFeaturePickerWidget::setAllowNull( bool allowNull )
{
  mModel->setAllowNull( allowNull );
  mLineEdit->setClearMode( allowNull ? QgsFilterLineEdit::ClearToNull : QgsFilterLineEdit::ClearToDefault );
}

QString QgsFeaturePickerWidget::filterExpression() const
{
  return mModel->filterExpression();
}

void QgsFeaturePickerWidget::setFilterExpression( const QString &filterExpression )
{
  mModel->setFilterExpression( filterExpression );
}

bool QgsFeaturePickerWidget::fetchGeometry() const
{
  return mModel->fetchGeometry();
}

void QgsFeaturePickerWidget::setFetchGeometry( bool fetchGeometry )
{
  mModel->setFetchGeometry( fetchGeometry );
}

int QgsFeaturePickerWidget::fetchLimit() const
{
  return mModel->fetchLimit();
}

void QgsFeaturePickerWidget::setFetchLimit( int fetchLimit )
{
  mModel->setFetchLimit( fetchLimit );
}

bool QgsFeaturePickerWidget::showBrowserButtons() const
{
  return mShowBrowserButtons;
}

void QgsFeaturePickerWidget::setShowBrowserButtons( bool showBrowserButtons )
{
  if ( showBrowserButtons == mShowBrowserButtons )
    return;

  mShowBrowserButtons = showBrowserButtons;
  mPreviousButton->setVisible( mShowBrowserButtons );
  mNextButton->setVisible( mShowBrowserButtons );
  emit showBrowserButtonsChanged();
}
