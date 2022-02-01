/***************************************************************************
                          qgsgenericprojectionselector.cpp
                    Set user defined CRS using projection selector widget
                             -------------------
    begin                : May 28, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgssettings.h"

#include "qgsprojectionselectiondialog.h"
#include "qgshelp.h"
#include <QDialogButtonBox>
#include <QApplication>
#include "qgsgui.h"
#include <QPushButton>


//
// QgsCrsSelectionWidget
//
QgsCrsSelectionWidget::QgsCrsSelectionWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  //we will show this only when a message is set
  textEdit->hide();

  mNotSetText = tr( "No CRS (or unknown/non-Earth projection)" );
  mLabelNoCrs->setText( tr( "Use this option to treat all coordinates as Cartesian coordinates in an unknown reference system." ) );

  mComboCrsType->addItem( tr( "Predefined CRS" ), static_cast< int >( CrsType::Predefined ) );
  mComboCrsType->addItem( tr( "Custom CRS" ), static_cast< int >( CrsType::Custom ) );

  mStackedWidget->setCurrentWidget( mPageDatabase );
  mComboCrsType->setCurrentIndex( mComboCrsType->findData( static_cast< int >( CrsType::Predefined ) ) );

  connect( mComboCrsType, qOverload< int >( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    if ( !mComboCrsType->currentData().isValid() )
      mStackedWidget->setCurrentWidget( mPageNoCrs );
    else
    {
      switch ( static_cast< CrsType >( mComboCrsType->currentData().toInt() ) )
      {
        case QgsCrsSelectionWidget::CrsType::Predefined:
          mStackedWidget->setCurrentWidget( mPageDatabase );
          break;
        case QgsCrsSelectionWidget::CrsType::Custom:
          mStackedWidget->setCurrentWidget( mPageCustom );
          break;
      }
    }

    if ( !mBlockSignals )
    {
      emit crsChanged();
      emit hasValidSelectionChanged( hasValidSelection() );
    }
  } );

  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::projectionDoubleClicked, this, [ = ]
  {
    emit crsDoubleClicked( projectionSelector->crs() );
  } );

  connect( mCrsDefinitionWidget, &QgsCrsDefinitionWidget::crsChanged, this, [ = ]()
  {
    if ( !mBlockSignals )
    {
      emit crsChanged();
      emit hasValidSelectionChanged( hasValidSelection() );
    }
  } );

  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::crsSelected, this, [ = ]()
  {
    if ( !mBlockSignals )
    {
      emit crsChanged();
      emit hasValidSelectionChanged( hasValidSelection() );
    }
  } );

  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::hasValidSelectionChanged, this, [ = ]()
  {
    if ( !mBlockSignals )
    {
      emit crsChanged();
      emit hasValidSelectionChanged( hasValidSelection() );
    }
  } );

  const QgsSettings settings;
  mSplitter->restoreState( settings.value( QStringLiteral( "Windows/ProjectionSelectorDialog/splitterState" ) ).toByteArray() );
}

QgsCrsSelectionWidget::~QgsCrsSelectionWidget()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/ProjectionSelectorDialog/splitterState" ), mSplitter->saveState() );
}

void QgsCrsSelectionWidget::setMessage( const QString &message )
{
  textEdit->setHtml( QStringLiteral( "<head><style>%1</style></head><body>%2</body>" ).arg( QgsApplication::reportStyleSheet(),
                     message ) );
  textEdit->show();
}

void QgsCrsSelectionWidget::setShowNoCrs( bool show )
{
  if ( mShowNoCrsOption == show )
    return;

  mBlockSignals++;
  mShowNoCrsOption = show;
  if ( mShowNoCrsOption )
  {
    mComboCrsType->insertItem( 0, mNotSetText );
  }
  else
  {
    mComboCrsType->removeItem( 0 );
  }

  if ( show && mDeferredInvalidCrsSet )
  {
    mComboCrsType->setCurrentIndex( 0 );
  }

  mBlockSignals--;

  if ( mDeferredInvalidCrsSet )
    emit crsChanged();

  mDeferredInvalidCrsSet = false;

  emit hasValidSelectionChanged( hasValidSelection() );
}

bool QgsCrsSelectionWidget::showNoCrs() const
{
  return mShowNoCrsOption;
}

void QgsCrsSelectionWidget::setNotSetText( const QString &text, const QString &description )
{
  mNotSetText = text;

  if ( mShowNoCrsOption )
  {
    mComboCrsType->setItemText( 0, mNotSetText );
  }

  mLabelNoCrs->setText( description.isEmpty() ? text : description );
}

bool QgsCrsSelectionWidget::hasValidSelection() const
{
  if ( !mComboCrsType->currentData().isValid() )
    return true;
  else if ( mDeferredInvalidCrsSet )
    return false;
  else
  {
    switch ( static_cast< CrsType >( mComboCrsType->currentData().toInt() ) )
    {
      case QgsCrsSelectionWidget::CrsType::Predefined:
        return projectionSelector->hasValidSelection();
      case QgsCrsSelectionWidget::CrsType::Custom:
        return mCrsDefinitionWidget->crs().isValid();
    }
    BUILTIN_UNREACHABLE
  }
}

QgsCoordinateReferenceSystem QgsCrsSelectionWidget::crs() const
{
  if ( !mComboCrsType->currentData().isValid() )
    return QgsCoordinateReferenceSystem();
  else
  {
    switch ( static_cast< CrsType >( mComboCrsType->currentData().toInt() ) )
    {
      case QgsCrsSelectionWidget::CrsType::Predefined:
        return projectionSelector->crs();
      case QgsCrsSelectionWidget::CrsType::Custom:
        return mCrsDefinitionWidget->crs();
    }
    BUILTIN_UNREACHABLE
  }
}

void QgsCrsSelectionWidget::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mBlockSignals++;
  if ( !crs.isValid() )
  {
    if ( mShowNoCrsOption )
      mComboCrsType->setCurrentIndex( 0 );
    else
      mDeferredInvalidCrsSet = true;
  }
  else
  {
    projectionSelector->setCrs( crs );
    mCrsDefinitionWidget->setCrs( crs );
    if ( crs.isValid() && crs.authid().isEmpty() )
    {
      mComboCrsType->setCurrentIndex( mComboCrsType->findData( static_cast< int>( CrsType::Custom ) ) );
      mStackedWidget->setCurrentWidget( mPageCustom );
    }
    else
    {
      mComboCrsType->setCurrentIndex( mComboCrsType->findData( static_cast< int>( CrsType::Predefined ) ) );
      mStackedWidget->setCurrentWidget( mPageDatabase );
    }
  }
  mBlockSignals--;

  emit crsChanged();
  emit hasValidSelectionChanged( hasValidSelection() );
}

void QgsCrsSelectionWidget::setOgcWmsCrsFilter( const QSet<QString> &crsFilter )
{
  projectionSelector->setOgcWmsCrsFilter( crsFilter );
}



//
// QgsProjectionSelectionDialog
//

QgsProjectionSelectionDialog::QgsProjectionSelectionDialog( QWidget *parent,
    Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  QVBoxLayout *vlayout = new QVBoxLayout();

  mCrsWidget = new QgsCrsSelectionWidget();
  vlayout->addWidget( mCrsWidget, 1 );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsProjectionSelectionDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsProjectionSelectionDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsProjectionSelectionDialog::showHelp );

  vlayout->addWidget( mButtonBox );

  setLayout( vlayout );

  QgsGui::enableAutoGeometryRestore( this );

  //apply selected projection upon double-click on item
  connect( mCrsWidget, &QgsCrsSelectionWidget::crsDoubleClicked, this, &QgsProjectionSelectionDialog::accept );
}

void QgsProjectionSelectionDialog::setMessage( const QString &message )
{
  mCrsWidget->setMessage( message );
}

void QgsProjectionSelectionDialog::showNoCrsForLayerMessage()
{
  setMessage( tr( "This layer appears to have no projection specification." )
              + ' '
              + tr( "By default, this layer will now have its projection set to that of the project, "
                    "but you may override this by selecting a different projection below." ) );
}

void QgsProjectionSelectionDialog::setShowNoProjection( bool show )
{
  mCrsWidget->setShowNoCrs( show );
}

bool QgsProjectionSelectionDialog::showNoProjection() const
{
  return mCrsWidget->showNoCrs();
}

void QgsProjectionSelectionDialog::setNotSetText( const QString &text, const QString &description )
{
  mCrsWidget->setNotSetText( text, description );
}

void QgsProjectionSelectionDialog::setRequireValidSelection()
{
  mRequireValidSelection = true;
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( hasValidSelection() );

  connect( mCrsWidget, &QgsCrsSelectionWidget::hasValidSelectionChanged, this, [ = ]( bool isValid )
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( isValid );
  } );
}

bool QgsProjectionSelectionDialog::hasValidSelection() const
{
  return mCrsWidget->hasValidSelection();
}

QgsCoordinateReferenceSystem QgsProjectionSelectionDialog::crs() const
{
  return mCrsWidget->crs();
}

void QgsProjectionSelectionDialog::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrsWidget->setCrs( crs );

  if ( mRequireValidSelection )
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( hasValidSelection() );
}

void QgsProjectionSelectionDialog::setOgcWmsCrsFilter( const QSet<QString> &crsFilter )
{
  mCrsWidget->setOgcWmsCrsFilter( crsFilter );
}

void QgsProjectionSelectionDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_projections/working_with_projections.html" ) );
}
