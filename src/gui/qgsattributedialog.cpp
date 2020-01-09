/***************************************************************************
                         qgsattributedialog.cpp  -  description
                             -------------------
    begin                : October 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributedialog.h"

#include "qgsattributeform.h"
#include "qgshighlight.h"
#include "qgsapplication.h"
#include "qgssettings.h"

QgsAttributeDialog::QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, QWidget *parent, bool showDialogButtons, const QgsAttributeEditorContext &context )
  : QDialog( parent )
  , mOwnedFeature( featureOwner ? thepFeature : nullptr )
{
  init( vl, thepFeature, context, showDialogButtons );
}

QgsAttributeDialog::~QgsAttributeDialog()
{
  if ( mHighlight )
  {
    mHighlight->hide();
    delete mHighlight;
  }

  delete mOwnedFeature;

  saveGeometry();
}

void QgsAttributeDialog::saveGeometry()
{
  // WARNING!!!! Don't use QgsGui::enableAutoGeometryRestore for this dialog -- the object name
  // is dynamic and is set to match the layer/feature combination.
  QgsSettings().setValue( QStringLiteral( "Windows/AttributeDialog/geometry" ), QDialog::saveGeometry() );
}

void QgsAttributeDialog::restoreGeometry()
{
  // WARNING!!!! Don't use QgsGui::enableAutoGeometryRestore for this dialog -- the object name
  // is dynamic and is set to match the layer/feature combination.
  QDialog::restoreGeometry( QgsSettings().value( QStringLiteral( "Windows/AttributeDialog/geometry" ) ).toByteArray() );
}

void QgsAttributeDialog::setHighlight( QgsHighlight *h )
{
  delete mHighlight;

  mHighlight = h;
}

void QgsAttributeDialog::accept()
{
  mAttributeForm->save();
  QDialog::accept();
}

void QgsAttributeDialog::show()
{
  QDialog::show();
  raise();
  activateWindow();
}

void QgsAttributeDialog::reject()
{
  // Delete any actions on other layers that may have been triggered from this dialog
  if ( mAttributeForm->mode() == QgsAttributeEditorContext::AddFeatureMode )
    mTrackedVectorLayerTools.rollback();

  QDialog::reject();
}

void QgsAttributeDialog::init( QgsVectorLayer *layer, QgsFeature *feature, const QgsAttributeEditorContext &context, bool showDialogButtons )
{
  QgsAttributeEditorContext trackedContext = context;
  setWindowTitle( tr( "%1 - Feature Attributes" ).arg( layer->name() ) );
  setLayout( new QGridLayout() );
  layout()->setMargin( 0 );
  mTrackedVectorLayerTools.setVectorLayerTools( trackedContext.vectorLayerTools() );
  trackedContext.setVectorLayerTools( &mTrackedVectorLayerTools );
  if ( showDialogButtons )
    trackedContext.setFormMode( QgsAttributeEditorContext::StandaloneDialog );

  mAttributeForm = new QgsAttributeForm( layer, *feature, trackedContext, this );
  mAttributeForm->disconnectButtonBox();
  layout()->addWidget( mAttributeForm );
  QDialogButtonBox *buttonBox = mAttributeForm->findChild<QDialogButtonBox *>();
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsAttributeDialog::reject );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsAttributeDialog::accept );
  connect( layer, &QObject::destroyed, this, &QWidget::close );

  mMenu = new QgsActionMenu( layer, mAttributeForm->feature(), QStringLiteral( "Feature" ), this );
  if ( !mMenu->actions().isEmpty() )
  {
    QMenuBar *menuBar = new QMenuBar( this );
    menuBar->addMenu( mMenu );
    layout()->setMenuBar( menuBar );
  }

  restoreGeometry();
  focusNextChild();
}

void QgsAttributeDialog::setMode( QgsAttributeEditorContext::Mode mode )
{
  mAttributeForm->setMode( mode );
  mMenu->setMode( mode );
}

bool QgsAttributeDialog::event( QEvent *e )
{
  if ( e->type() == QEvent::WindowActivate && mHighlight )
    mHighlight->show();
  else if ( e->type() == QEvent::WindowDeactivate && mHighlight )
    mHighlight->hide();

  return QDialog::event( e );
}
