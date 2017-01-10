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
#include "qgsactionmenu.h"

#include <QSettings>
#include <QMessageBox>

QgsAttributeDialog::QgsAttributeDialog( QgsVectorLayer* vl, QgsFeature* thepFeature, bool featureOwner, QWidget* parent, bool showDialogButtons, const QgsAttributeEditorContext &context )
    : QDialog( parent )
    , mHighlight( nullptr )
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

  if ( mOwnedFeature )
    delete mOwnedFeature;

  saveGeometry();
}

void QgsAttributeDialog::saveGeometry()
{
  QSettings().setValue( mSettingsPath + "geometry", QDialog::saveGeometry() );
}

void QgsAttributeDialog::restoreGeometry()
{
  QDialog::restoreGeometry( QSettings().value( mSettingsPath + "geometry" ).toByteArray() );
}

void QgsAttributeDialog::setHighlight( QgsHighlight* h )
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

void QgsAttributeDialog::onLayerModifiedCheck()
{
  if ( mAttributeForm->isChanged() )
  {
    // Bring this window to the top
    raise();

    QgsExpression displayExpression = mAttributeForm->layer()->displayExpression();
    QgsExpressionContext context = mAttributeForm->layer()->createExpressionContext();
    context.setFeature( mAttributeForm->feature() );

    QString featureTitle = displayExpression.evaluate( &context ).toString();

    QMessageBox messageBox( this );
    messageBox.setWindowTitle( tr( "Apply changes to feature?" ) );
    messageBox.setText( tr( "The attributes of feature <b>%1</b> have been edited but not yet saved to the edit buffer.<p>Do you want to save the changes now?</p>" ).arg( featureTitle ) );
    messageBox.setStandardButtons( QMessageBox::Save );
    messageBox.addButton( tr( "Reset changes" ), QMessageBox::ResetRole );
    messageBox.setDefaultButton( QMessageBox::Save );

    switch ( messageBox.exec() )
    {
      case QMessageBox::Save:
        mAttributeForm->save();
        break;

      case QMessageBox::Reset:
        mAttributeForm->resetValues();
        break;

      default:
        Q_ASSERT( false ); // Ok, seriously, how did you get here?
        break;
    }
  }
}

void QgsAttributeDialog::reject()
{
  // Delete any actions on other layers that may have been triggered from this dialog
  if ( mAttributeForm->mode() == QgsAttributeForm::AddFeatureMode )
    mTrackedVectorLayerTools.rollback();

  QDialog::reject();
}

void QgsAttributeDialog::init( QgsVectorLayer* layer, QgsFeature* feature, const QgsAttributeEditorContext& context, bool showDialogButtons )
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
  QDialogButtonBox* buttonBox = mAttributeForm->findChild<QDialogButtonBox*>();
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsAttributeDialog::reject );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsAttributeDialog::accept );
  connect( layer, &QObject::destroyed, this, &QgsAttributeDialog::close );
  connect( layer, &QgsVectorLayer::beforeModifiedCheck, this, &QgsAttributeDialog::onLayerModifiedCheck );

  QgsActionMenu* menu = new QgsActionMenu( layer, mAttributeForm->feature(), QStringLiteral( "AttributeTableRow" ), this );
  if ( !menu->actions().isEmpty() )
  {
    QMenuBar* menuBar = new QMenuBar( this );
    menuBar->addMenu( menu );
    layout()->setMenuBar( menuBar );
  }
  else
  {
    delete menu;
  }

  restoreGeometry();
  focusNextChild();
}

bool QgsAttributeDialog::event( QEvent* e )
{
  if ( e->type() == QEvent::WindowActivate && mHighlight )
    mHighlight->show();
  else if ( e->type() == QEvent::WindowDeactivate && mHighlight )
    mHighlight->hide();

  return QDialog::event( e );
}
