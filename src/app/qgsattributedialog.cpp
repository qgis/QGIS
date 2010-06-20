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
/* $Id$ */
#include "qgsattributedialog.h"
#include "qgsfield.h"
#include "qgslogger.h"

#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsuniquevaluerenderer.h"
#include "qgssymbol.h"
#include "qgsattributeeditor.h"
#include "qgsrubberband.h"

#include "qgisapp.h"

#include <QTableWidgetItem>
#include <QSettings>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDialogButtonBox>
#include <QUiLoader>
#include <QDialog>
#include <QVBoxLayout>
#include <QLineEdit>

QgsAttributeDialog::QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature )
    : mDialog( 0 )
    , mSettingsPath( "/Windows/AttributeDialog/" )
    , mLayer( vl )
    , mpFeature( thepFeature )
    , mRubberBand( 0 )
{
  if ( mpFeature == NULL || vl->dataProvider() == NULL )
    return;

  const QgsFieldMap &theFieldMap = vl->pendingFields();
  if ( theFieldMap.isEmpty() )
    return;

  QgsAttributeMap myAttributes = mpFeature->attributeMap();

  QDialogButtonBox *buttonBox = NULL;

  if ( !vl->editForm().isEmpty() )
  {
    QFile file( vl->editForm() );

    if ( file.open( QFile::ReadOnly ) )
    {
      QUiLoader loader;

      QFileInfo fi( vl->editForm() );
      loader.setWorkingDirectory( fi.dir() );
      QWidget *myWidget = loader.load( &file, QgisApp::instance() );
      file.close();

      mDialog = qobject_cast<QDialog*>( myWidget );
      buttonBox = myWidget->findChild<QDialogButtonBox*>();
    }
  }

  if ( !mDialog )
  {
    mDialog = new QDialog( QgisApp::instance() );

    QGridLayout *gridLayout;
    QFrame *mFrame;

    mDialog->resize( 447, 343 );
    gridLayout = new QGridLayout( mDialog );
    gridLayout->setSpacing( 6 );
    gridLayout->setMargin( 11 );
    gridLayout->setObjectName( QString::fromUtf8( "gridLayout" ) );
    mFrame = new QFrame( mDialog );
    mFrame->setObjectName( QString::fromUtf8( "mFrame" ) );
    mFrame->setFrameShape( QFrame::StyledPanel );
    mFrame->setFrameShadow( QFrame::Raised );

    gridLayout->addWidget( mFrame, 0, 0, 1, 1 );

    buttonBox = new QDialogButtonBox( mDialog );
    buttonBox->setObjectName( QString::fromUtf8( "buttonBox" ) );
    gridLayout->addWidget( buttonBox, 2, 0, 1, 1 );

    //
    //Set up dynamic inside a scroll box
    //
    QVBoxLayout *mypOuterLayout = new QVBoxLayout();
    mypOuterLayout->setContentsMargins( 0, 0, 0, 0 );
    //transfers layout ownership so no need to call delete

    mFrame->setLayout( mypOuterLayout );
    QScrollArea *mypScrollArea = new QScrollArea();
    //transfers scroll area ownership so no need to call delete
    mypOuterLayout->addWidget( mypScrollArea );
    QFrame *mypInnerFrame = new QFrame();
    mypInnerFrame->setFrameShape( QFrame::NoFrame );
    mypInnerFrame->setFrameShadow( QFrame::Plain );
    //transfers frame ownership so no need to call delete
    mypScrollArea->setWidget( mypInnerFrame );
    mypScrollArea->setWidgetResizable( true );
    QGridLayout *mypInnerLayout = new QGridLayout( mypInnerFrame );

    int index = 0;
    for ( QgsAttributeMap::const_iterator it = myAttributes.begin(); it != myAttributes.end(); ++it )
    {
      const QgsField &field = theFieldMap[it.key()];

      //show attribute alias if available
      QString myFieldName = vl->attributeDisplayName( it.key() );
      int myFieldType = field.type();

      QWidget *myWidget = QgsAttributeEditor::createAttributeEditor( 0, 0, vl, it.key(), it.value() );
      if ( !myWidget )
        continue;

      QLabel * mypLabel = new QLabel();
      mypInnerLayout->addWidget( mypLabel, index, 0 );
      if ( myFieldType == QVariant::Int )
      {
        mypLabel->setText( myFieldName + tr( " (int)" ) );
      }
      else if ( myFieldType == QVariant::Double )
      {
        mypLabel->setText( myFieldName + tr( " (dbl)" ) );
      }
      else //string
      {
        //any special behaviour for string goes here
        mypLabel->setText( myFieldName + tr( " (txt)" ) );
      }

      myWidget->setEnabled( vl->isEditable() );

      mypInnerLayout->addWidget( myWidget, index, 1 );
      mpIndizes << it.key();
      mpWidgets << myWidget;
      ++index;
    }
    // Set focus to first widget in list, to help entering data without moving the mouse.
    if ( mpWidgets.size() > 0 )
    {
      mpWidgets.first()->setFocus( Qt::OtherFocusReason );
    }
  }
  else
  {
    for ( QgsAttributeMap::const_iterator it = myAttributes.begin(); it != myAttributes.end(); ++it )
    {
      const QgsField &field = theFieldMap[it.key()];

      QWidget *myWidget = mDialog->findChild<QWidget*>( field.name() );
      if ( !myWidget )
        continue;

      QgsAttributeEditor::createAttributeEditor( mDialog, myWidget, vl, it.key(), it.value() );

      myWidget->setEnabled( vl->isEditable() );

      mpIndizes << it.key();
      mpWidgets << myWidget;
    }
  }

  if ( mDialog )
  {
    if ( mDialog->objectName().isEmpty() )
      mDialog->setObjectName( "QgsAttributeDialogBase" );

    if ( mDialog->windowTitle().isEmpty() )
      mDialog->setWindowTitle( tr( "Attributes - %1" ).arg( vl->name() ) );
  }

  if ( buttonBox )
  {
    buttonBox->clear();

    if ( vl->isEditable() )
    {
      buttonBox->setStandardButtons( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
      connect( buttonBox, SIGNAL( accepted() ), mDialog, SLOT( accept() ) );
      connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    }
    else
    {
      buttonBox->setStandardButtons( QDialogButtonBox::Cancel );
    }

    connect( buttonBox, SIGNAL( rejected() ), mDialog, SLOT( reject() ) );
  }

  QMetaObject::connectSlotsByName( mDialog );

  connect( mDialog, SIGNAL( destroyed() ), this, SLOT( dialogDestroyed() ) );

  if ( !vl->editFormInit().isEmpty() )
  {
#if 0
    // would be nice if only PyQt's QVariant.toPyObject() wouldn't take ownership
    vl->setProperty( "featureForm.dialog", QVariant::fromValue( qobject_cast<QObject*>( mDialog ) ) );
    vl->setProperty( "featureForm.id", QVariant( mpFeature->id() ) );
#endif

    QString module = vl->editFormInit();
    int pos = module.lastIndexOf( "." );
    if ( pos >= 0 )
    {
      QgisApp::instance()->runPythonString( QString( "import %1" ).arg( module.left( pos ) ) );
    }

    QgisApp::instance()->runPythonString( QString( "_qgis_featureform_%1 = wrapinstance( %2, QtGui.QDialog )" ).arg( mLayer->getLayerID() ).arg(( unsigned long ) mDialog ) );

    QString expr = QString( "%1(_qgis_featureform_%2,'%2',%3)" ).arg( vl->editFormInit() ).arg( vl->getLayerID() ).arg( mpFeature->id() );
    QgsDebugMsg( QString( "running featureForm init: %1" ).arg( expr ) );
    QgisApp::instance()->runPythonString( expr );
  }

  restoreGeometry();
}


QgsAttributeDialog::~QgsAttributeDialog()
{
  if ( mRubberBand )
  {
    mRubberBand->hide();
    delete mRubberBand;
  }

  saveGeometry();

  if ( mDialog )
  {
    delete mDialog;
  }
}

void QgsAttributeDialog::accept()
{
  if ( !mLayer->isEditable() )
    return;

  //write the new values back to the feature
  QgsAttributeMap myAttributes = mpFeature->attributeMap();
  int myIndex = 0;
  for ( QgsAttributeMap::const_iterator it = myAttributes.begin(); it != myAttributes.end(); ++it )
  {
    QVariant value;

    int idx = mpIndizes.value( myIndex );
    if ( QgsAttributeEditor::retrieveValue( mpWidgets.value( myIndex ), mLayer, idx, value ) )
      mpFeature->changeAttribute( idx, value );

    ++myIndex;
  }
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
  mDialog->setAttribute( Qt::WA_DeleteOnClose );
  mDialog->show();
  mDialog->raise();
  mDialog->activateWindow();
  mDialog->installEventFilter( this );
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

void QgsAttributeDialog::setHighlight( QgsRubberBand *rb )
{
  if ( mRubberBand )
  {
    delete mRubberBand;
  }

  mRubberBand = rb;
}


void QgsAttributeDialog::dialogDestroyed()
{
#if 0
  mLayer->setProperty( "featureForm.dialog", QVariant() );
  mLayer->setProperty( "featureForm.id", QVariant() );
#endif
  QString expr = QString( "if locals().has_key('_qgis_featureform_%1'): del _qgis_featureform_%1\n" ).arg( mLayer->getLayerID() );
  QgisApp::instance()->runPythonString( expr );

  mDialog = NULL;
  deleteLater();
}

bool QgsAttributeDialog::eventFilter( QObject *obj, QEvent *e )
{
  if ( mRubberBand && obj == mDialog )
  {
    switch ( e->type() )
    {
      case QEvent::WindowActivate:
        mRubberBand->show();
        break;
      case QEvent::WindowDeactivate:
        mRubberBand->hide();
        break;
      default:
        break;
    }
  }

  return false;
}
