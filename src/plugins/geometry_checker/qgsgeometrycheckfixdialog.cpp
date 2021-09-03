/***************************************************************************
 *  qgsgeometrycheckerfixdialog.cpp                                        *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QGridLayout>

#include "qgsgeometrycheckfixdialog.h"
#include "qgsgeometrycheckerresulttab.h"
#include "qgisinterface.h"
#include "qgsmapcanvas.h"
#include "qgssettings.h"
#include "qgsgeometrycheckerror.h"
#include "qgsgeometrychecker.h"
#include "qgsgeometrycheck.h"

QgsGeometryCheckerFixDialog::QgsGeometryCheckerFixDialog( QgsGeometryChecker *checker,
    const QList<QgsGeometryCheckError *> &errors, QWidget *parent )
  : QDialog( parent )
  , mChecker( checker )
  , mErrors( errors )
{
  setWindowTitle( tr( "Fix Errors" ) );

  QGridLayout *layout = new QGridLayout();
  layout->setContentsMargins( 6, 6, 6, 6 );
  setLayout( layout );

  mResolutionsBox = new QGroupBox();
  mResolutionsBox->setFlat( true );
  mResolutionsBox->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
  layout->addWidget( mResolutionsBox, 0, 0, 1, 2 );
  layout->addItem( new QSpacerItem( 1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding ), 1, 0, 1, 2 );

  mStatusLabel = new QLabel();
  layout->addWidget( mStatusLabel, 2, 0, 1, 2 );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Abort, Qt::Horizontal );
  mNextBtn = mButtonBox->addButton( tr( "Next" ), QDialogButtonBox::ActionRole );
  mFixBtn = mButtonBox->addButton( tr( "Fix" ), QDialogButtonBox::ActionRole );
  mSkipBtn = mButtonBox->addButton( tr( "Skip" ), QDialogButtonBox::ActionRole );
  mNextBtn->setAutoDefault( true );
  mFixBtn->setAutoDefault( true );
  layout->addWidget( mButtonBox, 3, 0, 1, 1 );

  mProgressBar = new QProgressBar();
  mProgressBar->setRange( 0, errors.size() );
  mProgressBar->setValue( 0 );
  layout->addWidget( mProgressBar, 3, 1, 1, 1 );

  mRadioGroup = new QButtonGroup( this );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mNextBtn, &QAbstractButton::clicked, this, &QgsGeometryCheckerFixDialog::setupNextError );
  connect( mFixBtn, &QAbstractButton::clicked, this, &QgsGeometryCheckerFixDialog::fixError );
  connect( mSkipBtn, &QAbstractButton::clicked, this, &QgsGeometryCheckerFixDialog::skipError );

}

void QgsGeometryCheckerFixDialog::showEvent( QShowEvent * )
{
  setupNextError();
}

void QgsGeometryCheckerFixDialog::setupNextError()
{
  mProgressBar->setValue( mProgressBar->maximum() - mErrors.size() );
  mNextBtn->setVisible( false );
  mFixBtn->setVisible( true );
  mFixBtn->setFocus();
  mSkipBtn->setVisible( true );
  mStatusLabel->clear();
  mResolutionsBox->setEnabled( true );

  QgsGeometryCheckError *error = mErrors.at( 0 );
  emit currentErrorChanged( error );

  mResolutionsBox->setTitle( tr( "Select how to fix error \"%1\":" ).arg( error->description() ) );

  delete mRadioGroup;
  mRadioGroup = new QButtonGroup( this );

  delete mResolutionsBox->layout();
  qDeleteAll( mResolutionsBox->children() );
  mResolutionsBox->setLayout( new QVBoxLayout() );
  mResolutionsBox->layout()->setContentsMargins( 0, 0, 0, 4 );

  const int checkedid = QgsSettings().value( QgsGeometryCheckerResultTab::sSettingsGroup + error->check()->id(), QVariant::fromValue<int>( 0 ) ).toInt();
  const QList<QgsGeometryCheckResolutionMethod> resolutionMethods = error->check()->availableResolutionMethods();
  for ( const QgsGeometryCheckResolutionMethod &method : resolutionMethods )
  {
    QRadioButton *radio = new QRadioButton( method.name() );
    radio->setChecked( checkedid == method.id() );
    mResolutionsBox->layout()->addWidget( radio );
    mRadioGroup->addButton( radio, method.id() );
  }
  adjustSize();
}

void QgsGeometryCheckerFixDialog::fixError()
{
  mResolutionsBox->setEnabled( false );
  mFixBtn->setVisible( false );
  mSkipBtn->setVisible( false );

  setCursor( Qt::WaitCursor );

  QgsGeometryCheckError *error = mErrors.at( 0 );
  mChecker->fixError( error, mRadioGroup->checkedId(), true );

  unsetCursor();

  mStatusLabel->setText( error->resolutionMessage() );
  if ( error->status() == QgsGeometryCheckError::StatusFixed )
  {
    mStatusLabel->setText( tr( "<b>Fixed:</b> %1" ).arg( error->resolutionMessage() ) );
  }
  else if ( error->status() == QgsGeometryCheckError::StatusFixFailed )
  {
    mStatusLabel->setText( tr( "<span color=\"red\"><b>Fixed failed:</b> %1</span>" ).arg( error->resolutionMessage() ) );
  }
  else if ( error->status() == QgsGeometryCheckError::StatusObsolete )
  {
    mStatusLabel->setText( tr( "<b>Error is obsolete</b>" ) );
  }
  mErrors.removeFirst();

  while ( !mErrors.isEmpty() && mErrors.at( 0 )->status() >= QgsGeometryCheckError::StatusFixed )
  {
    mErrors.removeFirst();
  }

  mProgressBar->setValue( mProgressBar->maximum() - mErrors.size() );

  if ( mErrors.isEmpty() )
  {
    mButtonBox->addButton( QDialogButtonBox::Close );
    mNextBtn->setVisible( false );
    mFixBtn->setVisible( false );
    mSkipBtn->setVisible( false );
    mButtonBox->button( QDialogButtonBox::Abort )->setVisible( false );
  }
  else
  {
    mNextBtn->setVisible( true );
    mNextBtn->setFocus();
  }
  adjustSize();
  emit currentErrorChanged( error );
}

void QgsGeometryCheckerFixDialog::skipError()
{
  mErrors.removeFirst();

  while ( !mErrors.isEmpty() && mErrors.at( 0 )->status() >= QgsGeometryCheckError::StatusFixed )
  {
    mErrors.removeFirst();
  }
  if ( !mErrors.isEmpty() )
  {
    setupNextError();
  }
  else
  {
    reject();
  }
}
