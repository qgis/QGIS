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

#include <QSettings>
#include <QGridLayout>


QgsAttributeDialog::QgsAttributeDialog( QgsVectorLayer* vl, QgsFeature* thepFeature, bool featureOwner, QgsDistanceArea myDa, QWidget* parent, bool showDialogButtons )
    : QObject( parent )
    , mHighlight( 0 )
{
  QgsAttributeEditorContext context;
  context.setDistanceArea( myDa );
  init( vl, thepFeature, context, parent );

  if ( !showDialogButtons )
    mAttributeForm->hideButtonBox();

  if ( featureOwner )
    delete thepFeature;
}

QgsAttributeDialog::QgsAttributeDialog( QgsVectorLayer* vl, QgsFeature* thepFeature, bool featureOwner, QWidget* parent, bool showDialogButtons, QgsAttributeEditorContext context )
    : QObject( parent )
    , mHighlight( 0 )
{
  init( vl, thepFeature, context, parent );

  if ( !showDialogButtons )
    mAttributeForm->hideButtonBox();

  if ( featureOwner )
    delete thepFeature;
}

QgsAttributeDialog::~QgsAttributeDialog()
{
  if ( mHighlight )
  {
    mHighlight->hide();
    delete mHighlight;
  }

  delete mDialog;
}

void QgsAttributeDialog::saveGeometry()
{
  if ( mDialog )
  {
    QSettings settings;
    settings.setValue( mSettingsPath + "geometry", mDialog->saveGeometry() );
  }
}

void QgsAttributeDialog::restoreGeometry()
{
  if ( mDialog )
  {
    QSettings settings;
    mDialog->restoreGeometry( settings.value( mSettingsPath + "geometry" ).toByteArray() );
  }
}

void QgsAttributeDialog::setHighlight( QgsHighlight* h )
{
  delete mHighlight;

  mHighlight = h;
}

void QgsAttributeDialog::accept()
{
  mAttributeForm->save();
}

int QgsAttributeDialog::exec()
{
  if ( mDialog )
  {
    return mDialog->exec();
  }
  else
  {
    QgsDebugMsg( "No dialog" );
    return QDialog::Accepted;
  }
}

void QgsAttributeDialog::show()
{
  if ( mDialog )
  {
    mDialog->setAttribute( Qt::WA_DeleteOnClose );
    mDialog->show();
    mDialog->raise();
    mDialog->activateWindow();
    mDialog->installEventFilter( this );
    setParent( mDialog );
  }
}

bool QgsAttributeDialog::eventFilter( QObject* obj, QEvent* e )
{
  if ( mHighlight && obj == mDialog )
  {
    switch ( e->type() )
    {
      case QEvent::WindowActivate:
        mHighlight->show();
        break;

      case QEvent::WindowDeactivate:
        mHighlight->hide();
        break;

      default:
        break;
    }
  }

  return false;
}

void QgsAttributeDialog::onDialogFinished( int result )
{
  Q_UNUSED( result )
  saveGeometry();
}

void QgsAttributeDialog::init( QgsVectorLayer* layer, QgsFeature* feature, QgsAttributeEditorContext& context, QWidget* parent )
{
  mDialog = new QDialog( parent );
  mDialog->setWindowTitle( tr( "Feature Attributes" ) );
  mDialog->setLayout( new QGridLayout() );
  mDialog->layout()->setMargin( 0 );
  mAttributeForm = new QgsAttributeForm( layer, *feature, context, parent );
  mDialog->layout()->addWidget( mAttributeForm );
  QDialogButtonBox* buttonBox = mAttributeForm->findChild<QDialogButtonBox*>();
  connect( buttonBox, SIGNAL( rejected() ), mDialog, SLOT( reject() ) );
  connect( buttonBox, SIGNAL( accepted() ), mDialog, SLOT( accept() ) );
  connect( mDialog, SIGNAL( finished( int ) ), this, SLOT( onDialogFinished( int ) ) );
  restoreGeometry();
}
