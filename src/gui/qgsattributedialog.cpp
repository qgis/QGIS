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
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsattributeeditor.h"
#include "qgshighlight.h"
#include "qgsexpression.h"
#include "qgspythonrunner.h"

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
#include <QWebView>
#include <QPushButton>

int QgsAttributeDialog::smFormCounter = 0;

QgsAttributeDialog::QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, QgsDistanceArea myDa, QWidget* parent, bool showDialogButtons )
    : mDialog( 0 )
    , mSettingsPath( "/Windows/AttributeDialog/" )
    , mLayer( vl )
    , mFeature( thepFeature )
    , mFeatureOwner( featureOwner )
    , mHighlight( 0 )
    , mFormNr( -1 )
    , mShowDialogButtons( showDialogButtons )
{
  if ( !mFeature || !vl->dataProvider() )
    return;

  const QgsFields &theFields = vl->pendingFields();
  if ( theFields.isEmpty() )
    return;

  QgsAttributes myAttributes = mFeature->attributes();

  QDialogButtonBox *buttonBox = NULL;

  if ( vl->editorLayout() == QgsVectorLayer::UiFileLayout && !vl->editForm().isEmpty() )
  {
    // UI-File defined layout
    QFile file( vl->editForm() );

    if ( file.open( QFile::ReadOnly ) )
    {
      QUiLoader loader;

      QFileInfo fi( vl->editForm() );
      loader.setWorkingDirectory( fi.dir() );
      QWidget *myWidget = loader.load( &file, parent );
      file.close();

      mDialog = qobject_cast<QDialog*>( myWidget );
      buttonBox = myWidget->findChild<QDialogButtonBox*>();
    }
  }
  else if ( vl->editorLayout() == QgsVectorLayer::TabLayout )
  {
    // Tab display
    mDialog = new QDialog( parent );

    QGridLayout *gridLayout;
    QTabWidget *tabWidget;

    mDialog->resize( 447, 343 );
    gridLayout = new QGridLayout( mDialog );
    gridLayout->setObjectName( QString::fromUtf8( "gridLayout" ) );

    tabWidget = new QTabWidget( mDialog );
    gridLayout->addWidget( tabWidget );

    foreach ( const QgsAttributeEditorElement *widgDef, vl->attributeEditorElements() )
    {
      QWidget* tabPage = new QWidget( tabWidget );

      tabWidget->addTab( tabPage, widgDef->name() );
      QGridLayout *tabPageLayout = new QGridLayout( tabPage );

      if ( widgDef->type() == QgsAttributeEditorElement::AeTypeContainer )
      {
        tabPageLayout->addWidget( QgsAttributeEditor::createWidgetFromDef( widgDef, tabPage, vl, myAttributes, mProxyWidgets, false ) );
      }
      else
      {
        QgsDebugMsg( "No support for fields in attribute editor on top level" );
      }
    }

    buttonBox = new QDialogButtonBox( mDialog );
    buttonBox->setObjectName( QString::fromUtf8( "buttonBox" ) );
    gridLayout->addWidget( buttonBox );
  }

  if ( !mDialog )
  {
    mDialog = new QDialog( parent );

    QGridLayout *gridLayout;
    QFrame *mFrame;

    mDialog->resize( 447, 343 );
    gridLayout = new QGridLayout( mDialog );
    gridLayout->setSpacing( 6 );
    gridLayout->setMargin( 2 );
    gridLayout->setObjectName( QString::fromUtf8( "gridLayout" ) );
    mFrame = new QFrame( mDialog );
    mFrame->setObjectName( QString::fromUtf8( "mFrame" ) );
    mFrame->setFrameShape( QFrame::NoFrame );
    mFrame->setFrameShadow( QFrame::Plain );

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
    mypScrollArea->setFrameShape( QFrame::NoFrame );
    mypScrollArea->setFrameShadow( QFrame::Plain );

    //transfers scroll area ownership so no need to call delete
    mypOuterLayout->addWidget( mypScrollArea );

    QSpacerItem *mypSpacer = new QSpacerItem( 10, 10, QSizePolicy::Fixed, QSizePolicy::Expanding );
    mypOuterLayout->addItem( mypSpacer );

    QFrame *mypInnerFrame = new QFrame();
    mypInnerFrame->setFrameShape( QFrame::NoFrame );
    mypInnerFrame->setFrameShadow( QFrame::Plain );

    //transfers frame ownership so no need to call delete
    mypScrollArea->setWidget( mypInnerFrame );

    mypScrollArea->setWidgetResizable( true );
    QGridLayout *mypInnerLayout = new QGridLayout( mypInnerFrame );

    int index = 0;
    for ( int fldIdx = 0; fldIdx < theFields.count(); ++fldIdx )
    {
      //show attribute alias if available
      QString myFieldName = vl->attributeDisplayName( fldIdx );
      // by default (until user defined alias) append date format
      // (validator does not allow to enter a value in wrong format)
      const QgsField &myField = theFields[fldIdx];
      if ( myField.type() == QVariant::Date && vl->attributeAlias( fldIdx ).isEmpty() )
      {
        myFieldName += " (" + vl->dateFormat( fldIdx ) + ")";
      }

      QWidget *myWidget = QgsAttributeEditor::createAttributeEditor( 0, 0, vl, fldIdx, myAttributes[fldIdx], mProxyWidgets );
      if ( !myWidget )
        continue;

      QLabel *mypLabel = new QLabel( myFieldName, mypInnerFrame );

      if ( vl->editType( fldIdx ) != QgsVectorLayer::Immutable )
      {
        if ( vl->isEditable() && vl->fieldEditable( fldIdx ) )
        {
          myWidget->setEnabled( true );
        }
        else if ( vl->editType( fldIdx ) == QgsVectorLayer::Photo )
        {
          foreach ( QWidget *w, myWidget->findChildren<QWidget *>() )
          {
            w->setEnabled( qobject_cast<QLabel *>( w ) ? true : false );
          }
        }
        else if ( vl->editType( fldIdx ) == QgsVectorLayer::WebView )
        {
          foreach ( QWidget *w, myWidget->findChildren<QWidget *>() )
          {
            if ( qobject_cast<QWebView *>( w ) )
              w->setEnabled( true );
            else if ( qobject_cast<QPushButton *>( w ) && w->objectName() == "openUrl" )
              w->setEnabled( true );
            else
              w->setEnabled( false );
          }
        }
        else
        {
          myWidget->setEnabled( false );
        }
      }

      if ( vl->labelOnTop( fldIdx ) )
      {
        mypInnerLayout->addWidget( mypLabel, index++, 0, 1, 2 );
        mypInnerLayout->addWidget( myWidget, index++, 0, 1, 2 );
      }
      else
      {
        mypInnerLayout->addWidget( mypLabel, index, 0 );
        mypInnerLayout->addWidget( myWidget, index, 1 );
        ++index;
      }
    }

    // Set focus to first widget in list, to help entering data without moving the mouse.
    if ( mypInnerLayout->rowCount() > 0 )
    {
      QWidget* widget = mypInnerLayout->itemAtPosition( 0, 1 )->widget();
      if ( widget )
        widget->setFocus( Qt::OtherFocusReason );
    }
  }
  else
  {
#if 0
    QgsDistanceArea myDa;

    myDa.setSourceCrs( vl->crs().srsid() );
    myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapRenderer()->hasCrsTransformEnabled() );
    myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
#endif
    for ( int fldIdx = 0; fldIdx < theFields.count(); ++fldIdx )
    {
      QList<QWidget *> myWidgets = mDialog->findChildren<QWidget*>( theFields[fldIdx].name() );
      if ( myWidgets.isEmpty() )
        continue;

      foreach ( QWidget *myWidget, myWidgets )
      {
        QgsAttributeEditor::createAttributeEditor( mDialog, myWidget, vl, fldIdx, myAttributes[fldIdx], mProxyWidgets );

        if ( vl->editType( fldIdx ) != QgsVectorLayer::Immutable )
        {
          if ( vl->isEditable() && vl->fieldEditable( fldIdx ) )
          {
            myWidget->setEnabled( true );
          }
          else if ( vl->editType( fldIdx ) == QgsVectorLayer::Photo )
          {
            foreach ( QWidget *w, myWidget->findChildren<QWidget *>() )
            {
              w->setEnabled( qobject_cast<QLabel *>( w ) ? true : false );
            }
          }
          else if ( vl->editType( fldIdx ) == QgsVectorLayer::WebView )
          {
            foreach ( QWidget *w, myWidget->findChildren<QWidget *>() )
            {
              w->setEnabled( qobject_cast<QWebView *>( w ) ? true : false );
            }
          }
          else
          {
            myWidget->setEnabled( false );
          }
        }
      }
    }

    foreach ( QLineEdit *le, mDialog->findChildren<QLineEdit*>() )
    {
      if ( !le->objectName().startsWith( "expr_" ) )
        continue;

      le->setReadOnly( true );
      QString expr = le->text();
      le->setText( tr( "Error" ) );

      QgsExpression exp( expr );
      if ( exp.hasParserError() )
        continue;


      if ( !mFeature->geometry() && exp.needsGeometry() )
      {
        QgsFeature f;
        if ( vl->getFeatures( QgsFeatureRequest().setFilterFid( mFeature->id() ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) && f.geometry() )
        {
          mFeature->setGeometry( *f.geometry() );
        }
      }

      exp.setGeomCalculator( myDa );

      QVariant value = exp.evaluate( mFeature, vl->pendingFields() );

      if ( !exp.hasEvalError() )
      {
        QString text;
        switch ( value.type() )
        {
          case QVariant::Invalid: text = "NULL"; break;
          case QVariant::Int: text = QString::number( value.toInt() ); break;
          case QVariant::Double: text = QString::number( value.toDouble() ); break;
          case QVariant::String:
          default: text = value.toString();
        }
        le->setText( text );
      }
      else
      {
        le->setText( tr( "Error: %1" ).arg( exp.evalErrorString() ) );
      }
    }
  }

  if ( mDialog )
  {
    if ( mDialog->objectName().isEmpty() )
      mDialog->setObjectName( "QgsAttributeDialogBase" );

    if ( mDialog->windowTitle().isEmpty() )
      mDialog->setWindowTitle( tr( "Attributes - %1" ).arg( vl->name() ) );
  }

  if ( mShowDialogButtons )
  {
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
  }
  else
  {
    if ( buttonBox )
    {
      buttonBox->setVisible( false );
    }
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
      QgsPythonRunner::run( QString( "import %1" ).arg( module.left( pos ) ) );
    }

    /* Reload the module if the DEBUGMODE switch has been set in the module.
     If set to False you have to reload QGIS to reset it to True due to Python
     module caching */
    QString reload = QString( "if hasattr(%1,'DEBUGMODE') and %1.DEBUGMODE:"
                              "    reload(%1)" ).arg( module.left( pos ) );

    QgsPythonRunner::run( reload );

    mFormNr = smFormCounter++;

    QString form =  QString( "_qgis_featureform_%1 = sip.wrapinstance( %2, QtGui.QDialog )" )
                    .arg( mFormNr )
                    .arg(( unsigned long ) mDialog );

    QString layer = QString( "_qgis_layer_%1 = sip.wrapinstance( %2, qgis.core.QgsVectorLayer )" )
                    .arg( vl->id() )
                    .arg(( unsigned long ) vl );

    // Generate the unique ID of this feature.  We used to use feature ID but some providers
    // return a ID that is an invalid python variable when we have new unsaved features.
    QDateTime dt = QDateTime::currentDateTime();
    QString featurevarname = QString( "_qgis_feature_%1" ).arg( dt.toString( "yyyyMMddhhmmsszzz" ) );
    QString feature = QString( "%1 = sip.wrapinstance( %2, qgis.core.QgsFeature )" )
                      .arg( featurevarname )
                      .arg(( unsigned long ) mFeature );

    QgsPythonRunner::run( form );
    QgsPythonRunner::run( feature );
    QgsPythonRunner::run( layer );

    QString expr = QString( "%1(_qgis_featureform_%2, _qgis_layer_%3, %4)" )
                   .arg( vl->editFormInit() )
                   .arg( mFormNr )
                   .arg( vl->id() )
                   .arg( featurevarname );

    QgsDebugMsg( QString( "running featureForm init: %1" ).arg( expr ) );
    QgsPythonRunner::run( expr );
  }

  restoreGeometry();
}


QgsAttributeDialog::~QgsAttributeDialog()
{
  if ( mHighlight )
  {
    mHighlight->hide();
    delete mHighlight;
  }

  if ( mFeatureOwner )
  {
    delete mFeature;
  }

  saveGeometry();

  if ( mDialog )
  {
    delete mDialog;
  }
}

void QgsAttributeDialog::accept()
{
  if ( !mLayer->isEditable() || !mFeature )
    return;

  //write the new values back to the feature
  const QgsFields& fields = mLayer->pendingFields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    QVariant value;

    if ( QgsAttributeEditor::retrieveValue( mProxyWidgets.value( idx ), mLayer, idx, value ) )
      mFeature->setAttribute( idx, value );
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
  if ( mDialog )
  {
    mDialog->setAttribute( Qt::WA_DeleteOnClose );
    mDialog->show();
    mDialog->raise();
    mDialog->activateWindow();
    mDialog->installEventFilter( this );
  }
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

void QgsAttributeDialog::setHighlight( QgsHighlight *h )
{
  if ( mHighlight )
  {
    delete mHighlight;
  }

  mHighlight = h;
}


void QgsAttributeDialog::dialogDestroyed()
{
#if 0
  mLayer->setProperty( "featureForm.dialog", QVariant() );
  mLayer->setProperty( "featureForm.id", QVariant() );
#endif
  if ( -1 < mFormNr )
  {
    QString expr = QString( "if locals().has_key('_qgis_featureform_%1'): del _qgis_featureform_%1\n" ).arg( mFormNr );
    QgsPythonRunner::run( expr );
  }

  mDialog = NULL;
  deleteLater();
}

bool QgsAttributeDialog::eventFilter( QObject *obj, QEvent *e )
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
