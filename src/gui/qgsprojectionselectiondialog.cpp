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
#include <QApplication>
#include "qgsgui.h"
#include <QPushButton>

QgsProjectionSelectionDialog::QgsProjectionSelectionDialog( QWidget *parent,
    Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsProjectionSelectionDialog::showHelp );

  //we will show this only when a message is set
  textEdit->hide();

  tabWidget->setCurrentWidget( mTabDatabase );

  mCheckBoxNoProjection->setHidden( true );
  mCheckBoxNoProjection->setEnabled( false );
  connect( mCheckBoxNoProjection, &QCheckBox::toggled, this, [ = ]
  {
#if 0
    if ( !mBlockSignals )
    {
      emit crsSelected();
      emit hasValidSelectionChanged( hasValidSelection() );
    }
#endif
  } );
  connect( mCheckBoxNoProjection, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    if ( mCheckBoxNoProjection->isEnabled() )
    {
      tabWidget->setDisabled( checked );
    }
  } );


  //apply selected projection upon double-click on item
  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::projectionDoubleClicked, this, &QgsProjectionSelectionDialog::accept );

  const QgsSettings settings;
  mSplitter->restoreState( settings.value( QStringLiteral( "Windows/ProjectionSelectorDialog/splitterState" ) ).toByteArray() );
}

QgsProjectionSelectionDialog::~QgsProjectionSelectionDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/ProjectionSelectorDialog/splitterState" ), mSplitter->saveState() );
}

void QgsProjectionSelectionDialog::setMessage( const QString &message )
{
  textEdit->setHtml( QStringLiteral( "<head><style>%1</style></head><body>%2</body>" ).arg( QgsApplication::reportStyleSheet(),
                     message ) );
  textEdit->show();
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
  mCheckBoxNoProjection->setVisible( show );
  mCheckBoxNoProjection->setEnabled( show );
  if ( show )
  {
    tabWidget->setDisabled( mCheckBoxNoProjection->isChecked() );
  }

  if ( mRequireValidSelection )
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( hasValidSelection() );
}

bool QgsProjectionSelectionDialog::showNoProjection() const
{
  return !mCheckBoxNoProjection->isHidden();
}

void QgsProjectionSelectionDialog::setNotSetText( const QString &text )
{
  mCheckBoxNoProjection->setText( text );
}

void QgsProjectionSelectionDialog::setRequireValidSelection()
{
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( hasValidSelection() );

  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::hasValidSelectionChanged, this, [ = ]( bool )
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( hasValidSelection() );
  } );

  connect( mCheckBoxNoProjection, &QCheckBox::toggled, this, [ = ]( bool )
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( hasValidSelection() );
  } );

  connect( mCrsDefinitionWidget, &QgsCrsDefinitionWidget::crsChanged, this, [ = ]()
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( hasValidSelection() );
  } );

  connect( tabWidget, &QTabWidget::currentChanged, this, [ = ]()
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( hasValidSelection() );
  } );
}

bool QgsProjectionSelectionDialog::hasValidSelection() const
{
  if ( mCheckBoxNoProjection->isChecked() )
    return true;

  if ( tabWidget->currentWidget() == mTabCustom )
    return mCrsDefinitionWidget->crs().isValid();
  else
    return projectionSelector->hasValidSelection();
}

QgsCoordinateReferenceSystem QgsProjectionSelectionDialog::crs() const
{
  if ( mCheckBoxNoProjection->isEnabled() && mCheckBoxNoProjection->isChecked() )
    return QgsCoordinateReferenceSystem();

  if ( tabWidget->currentWidget() == mTabCustom )
    return mCrsDefinitionWidget->crs();
  else
    return projectionSelector->crs();
}

void QgsProjectionSelectionDialog::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( !crs.isValid() )
  {
    mCheckBoxNoProjection->setChecked( true );
  }
  else
  {
    mCheckBoxNoProjection->setChecked( false );

    projectionSelector->setCrs( crs );
    mCrsDefinitionWidget->setCrs( crs );
    if ( crs.isValid() && crs.authid().isEmpty() )
      tabWidget->setCurrentWidget( mTabCustom );
    else
      tabWidget->setCurrentWidget( mTabDatabase );
  }

  if ( mRequireValidSelection )
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( hasValidSelection() );
}

void QgsProjectionSelectionDialog::setOgcWmsCrsFilter( const QSet<QString> &crsFilter )
{
  projectionSelector->setOgcWmsCrsFilter( crsFilter );
}

void QgsProjectionSelectionDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_projections/working_with_projections.html" ) );
}
