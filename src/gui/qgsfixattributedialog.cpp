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

#include <QtWidgets/QPushButton>

QgsFixAttributeDialog::QgsFixAttributeDialog( QgsVectorLayer *vl, QgsFeatureList &features, QWidget *parent )
  : QDialog( parent )
  , mFeatures( features )
{
  init( vl );
}

void QgsFixAttributeDialog::init( QgsVectorLayer *layer )
{
  QgsAttributeEditorContext context;
  setWindowTitle( tr( "%1 - Fix Feature Attributes" ).arg( layer->name() ) );
  setLayout( new QGridLayout() );
  layout()->setMargin( 0 );
  context.setFormMode( QgsAttributeEditorContext::StandaloneDialog );

  mUnfixedFeatures = mFeatures;
  mCurrentFeature = mFeatures.begin();

  QGridLayout *infoLayout = new QGridLayout();
  QWidget *infoBox = new QWidget();
  infoBox->setLayout( infoLayout );
  layout()->addWidget( infoBox );

  mDescription = new QLabel( descriptionText() );
  infoLayout->addWidget( mDescription );
  mProgressBar = new QProgressBar();
  mProgressBar->setOrientation( Qt::Horizontal );
  mProgressBar->setRange( 0, mFeatures.count() );
  infoLayout->addWidget( mProgressBar );
  QgsFeature feature;
  mAttributeForm = new QgsAttributeForm( layer, *mCurrentFeature, context, this );
  mAttributeForm->setMode( QgsAttributeEditorContext::SingleEditMode );
  mAttributeForm->disconnectButtonBox();
  layout()->addWidget( mAttributeForm );

  QDialogButtonBox *buttonBox = mAttributeForm->findChild<QDialogButtonBox *>();
  QPushButton *cancelAllBtn = new QPushButton( tr( "Cancel all" ) );
  QPushButton *cancelAllInvalidBtn = new QPushButton( tr( "Cancel all invalid" ) );
  QPushButton *storeAllInvalidBtn = new QPushButton( tr( "Store all (even invalid)" ) );
  buttonBox->addButton( cancelAllBtn, QDialogButtonBox::ActionRole );
  buttonBox->addButton( cancelAllInvalidBtn, QDialogButtonBox::ActionRole );
  buttonBox->addButton( storeAllInvalidBtn, QDialogButtonBox::ActionRole );
  connect( cancelAllBtn, &QAbstractButton::clicked, this, [ = ]()
  {
    done( VanishAll );
  } );
  connect( cancelAllInvalidBtn, &QAbstractButton::clicked, this, [ = ]()
  {
    done( CopyValid );
  } );
  connect( storeAllInvalidBtn, &QAbstractButton::clicked, this, [ = ]()
  {
    done( CopyAll );
  } );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsFixAttributeDialog::reject );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsFixAttributeDialog::accept );
  connect( layer, &QObject::destroyed, this, &QWidget::close );

  focusNextChild();
}

QString QgsFixAttributeDialog::descriptionText()
{
  return tr( "%1 of %2 features fixed\n%3 of %4 features canceled" ).arg( fixedFeatures().count() ).arg( mFeatures.count() ).arg( mCurrentFeature - mFeatures.begin() - fixedFeatures().count() ).arg( mFeatures.count() );
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
    done( CopyValid );
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
    done( CopyValid );
  }

  mProgressBar->setValue( mCurrentFeature - mFeatures.begin() );
  mDescription->setText( descriptionText() );
}
