/***************************************************************************
    qgsfixattributedialog.cpp
    ---------------------
    begin                : January 2020
    copyright            : (C) 2020 by David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfixattributedialog.h"

#include "qgsattributeform.h"
#include "qgsapplication.h"
#include "qgisapp.h"

#include <QtWidgets/QPushButton>


QgsFixAttributeDialog::QgsFixAttributeDialog( QgsVectorLayer *vl, QgsFeatureList &features, QWidget *parent, const QgsAttributeEditorContext &context )
  : QDialog( parent )
  , mFeatures( features )
{
  init( vl, context );
}

void QgsFixAttributeDialog::init( QgsVectorLayer *layer, const QgsAttributeEditorContext &context )
{
  setWindowTitle( tr( "%1 - Fix Pasted Features" ).arg( layer->name() ) );
  setLayout( new QGridLayout() );
  layout()->setContentsMargins( 0, 0, 0, 0 );

  mUnfixedFeatures = mFeatures;
  mCurrentFeature = mFeatures.begin();

  QGridLayout *infoLayout = new QGridLayout();
  QWidget *infoBox = new QWidget();
  infoBox->setLayout( infoLayout );
  layout()->addWidget( infoBox );

  mDescription = new QLabel( descriptionText() );
  mDescription->setVisible( mFeatures.count() > 1 );
  infoLayout->addWidget( mDescription );
  mProgressBar = new QProgressBar();
  mProgressBar->setOrientation( Qt::Horizontal );
  mProgressBar->setRange( 0, mFeatures.count() );
  mProgressBar->setVisible( mFeatures.count() > 1 );
  infoLayout->addWidget( mProgressBar );
  const QgsFeature feature;
  mAttributeForm = new QgsAttributeForm( layer, *mCurrentFeature, context, this );
  mAttributeForm->setMode( QgsAttributeEditorContext::FixAttributeMode );
  mAttributeForm->disconnectButtonBox();
  layout()->addWidget( mAttributeForm );

  QDialogButtonBox *buttonBox = mAttributeForm->findChild<QDialogButtonBox *>();
  QPushButton *cancelAllBtn = new QPushButton( tr( "Discard All" ) );
  QPushButton *cancelAllInvalidBtn = new QPushButton( tr( "Discard All Invalid" ) );
  QPushButton *storeAllInvalidBtn = new QPushButton( tr( "Paste All (Including Invalid)" ) );
  if ( mFeatures.count() > 1 )
  {
    buttonBox->addButton( cancelAllBtn, QDialogButtonBox::ActionRole );
    buttonBox->addButton( cancelAllInvalidBtn, QDialogButtonBox::ActionRole );
    connect( cancelAllBtn, &QAbstractButton::clicked, this, [ = ]()
    {
      done( DiscardAll );
    } );
    connect( cancelAllInvalidBtn, &QAbstractButton::clicked, this, [ = ]()
    {
      done( PasteValid );
    } );
    buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "Skip" ) );
  }
  else
  {
    storeAllInvalidBtn->setText( tr( "Paste Anyway" ) );
  }
  buttonBox->addButton( storeAllInvalidBtn, QDialogButtonBox::ActionRole );
  connect( storeAllInvalidBtn, &QAbstractButton::clicked, this, [ = ]()
  {
    done( PasteAll );
  } );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsFixAttributeDialog::reject );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsFixAttributeDialog::accept );
  connect( layer, &QObject::destroyed, this, &QWidget::close );

  focusNextChild();
}

QString QgsFixAttributeDialog::descriptionText()
{
  return tr( "%1 of %2 features processed (%3 fixed, %4 skipped)" ).arg( mCurrentFeature - mFeatures.begin() ).arg( mFeatures.count() ).arg( mFixedFeatures.count() ).arg( mCurrentFeature - mFeatures.begin() - mFixedFeatures.count() );
}

void QgsFixAttributeDialog::accept()
{
  mAttributeForm->save();
  mFixedFeatures << mAttributeForm->feature();
  mUnfixedFeatures.removeOne( *mCurrentFeature );

  //next feature
  ++mCurrentFeature;
  if ( mCurrentFeature != mFeatures.end() )
  {
    mAttributeForm->setFeature( *mCurrentFeature );
  }
  else
  {
    done( PasteValid );
  }

  mProgressBar->setValue( mCurrentFeature - mFeatures.begin() );
  mDescription->setText( descriptionText() );
}

void QgsFixAttributeDialog::reject()
{
  //next feature
  ++mCurrentFeature;
  if ( mCurrentFeature != mFeatures.end() )
  {
    mAttributeForm->setFeature( *mCurrentFeature );
  }
  else
  {
    done( PasteValid );
  }

  mProgressBar->setValue( mCurrentFeature - mFeatures.begin() );
  mDescription->setText( descriptionText() );
}
