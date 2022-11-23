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
#include "qgssettings.h"
#include "qgsmessagebar.h"
#include "qgsactionmenu.h"
#include "qgsmaplayeractioncontext.h"

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
  QString error;
  const bool didSave = mAttributeForm->saveWithDetails( &error );
  if ( didSave )
  {
    QDialog::accept();
  }
  else
  {
    if ( error.isEmpty() )
      error = tr( "An unknown error was encountered saving attributes" );

    mMessageBar->pushMessage( QString(),
                              error,
                              Qgis::MessageLevel::Critical );
  }
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
  layout()->setContentsMargins( 0, 0, 0, 0 );
  mMessageBar = new QgsMessageBar( this );
  mMessageBar->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
  layout()->addWidget( mMessageBar );

  setLayout( layout() );

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
  mMenu->setActionContextGenerator( this );
  if ( !mMenu->isEmpty() )
  {
    mMenuBar = new QMenuBar( this );
    mMenuBar->addMenu( mMenu );
    layout()->setMenuBar( mMenuBar );
  }

  restoreGeometry();
  focusNextChild();
}

void QgsAttributeDialog::setMode( QgsAttributeEditorContext::Mode mode )
{
  mAttributeForm->setMode( mode );
  mMenu->setMode( mode );

  if ( !mMenu->isEmpty() && !mMenuBar )
  {
    mMenuBar = new QMenuBar( this );
    mMenuBar->addMenu( mMenu );
    layout()->setMenuBar( mMenuBar );
  }
  else if ( mMenu->isEmpty() && mMenuBar )
  {
    layout()->setMenuBar( nullptr );
    delete mMenuBar;
    mMenuBar = nullptr;
  }
}

bool QgsAttributeDialog::event( QEvent *e )
{
  if ( e->type() == QEvent::WindowActivate && mHighlight )
    mHighlight->show();
  else if ( e->type() == QEvent::WindowDeactivate && mHighlight )
    mHighlight->hide();

  return QDialog::event( e );
}

void QgsAttributeDialog::setExtraContextScope( QgsExpressionContextScope *extraScope )
{
  mAttributeForm->setExtraContextScope( extraScope );
}

QgsMapLayerActionContext QgsAttributeDialog::createActionContext()
{
  QgsMapLayerActionContext context;
  context.setAttributeDialog( this );
  context.setMessageBar( mMessageBar );
  return context;
}

