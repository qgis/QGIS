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
#include "qgseditorwidgetwrapper.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsrelationeditor.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
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

int QgsAttributeDialog::sFormCounter = 0;
QString QgsAttributeDialog::sSettingsPath = "/Windows/AttributeDialog/";

QgsAttributeDialog::QgsAttributeDialog( QgsVectorLayer* vl, QgsFeature* thepFeature, bool featureOwner, QWidget* parent, bool showDialogButtons, QgsAttributeEditorContext context )
    : QObject( parent )
    , mDialog( 0 )
    , mContext( context )
    , mLayer( vl )
    , mFeature( thepFeature )
    , mFeatureOwner( featureOwner )
    , mHighlight( 0 )
    , mFormNr( sFormCounter++ )
    , mShowDialogButtons( showDialogButtons )
{
  mContext.adjustForLayer( mLayer );
  init();
}

QgsAttributeDialog::QgsAttributeDialog( QgsVectorLayer* vl, QgsFeature* thepFeature, bool featureOwner, QgsDistanceArea myDa, QWidget* parent, bool showDialogButtons )
    : QObject( parent )
    , mDialog( 0 )
    , mContext( )
    , mLayer( vl )
    , mFeature( thepFeature )
    , mFeatureOwner( featureOwner )
    , mHighlight( 0 )
    , mFormNr( sFormCounter++ )
    , mShowDialogButtons( showDialogButtons )
{
  mContext.setDistanceArea( myDa );
  mContext.adjustForLayer( mLayer );
  init();
}

void QgsAttributeDialog::init()
{
  if ( !mFeature || !mLayer->dataProvider() )
    return;

  const QgsFields &theFields = mLayer->pendingFields();
  if ( theFields.isEmpty() )
    return;

  QDialogButtonBox *buttonBox = NULL;

  if ( mLayer->editorLayout() == QgsVectorLayer::UiFileLayout && !mLayer->editForm().isEmpty() )
  {
    // UI-File defined layout
    QFile file( mLayer->editForm() );

    if ( file.open( QFile::ReadOnly ) )
    {
      QUiLoader loader;

      QFileInfo fi( mLayer->editForm() );
      loader.setWorkingDirectory( fi.dir() );
      QWidget *myWidget = loader.load( &file, qobject_cast<QWidget*>( parent() ) );
      file.close();

      mDialog = qobject_cast<QDialog*>( myWidget );
      buttonBox = myWidget->findChild<QDialogButtonBox*>();
    }
  }
  else if ( mLayer->editorLayout() == QgsVectorLayer::TabLayout )
  {
    // Tab display
    mDialog = new QDialog( qobject_cast<QWidget*>( parent() ) );

    QGridLayout *gridLayout;
    QTabWidget *tabWidget;

    mDialog->resize( 447, 343 );
    gridLayout = new QGridLayout( mDialog );
    gridLayout->setObjectName( QString::fromUtf8( "gridLayout" ) );

    tabWidget = new QTabWidget( mDialog );
    gridLayout->addWidget( tabWidget );

    foreach ( const QgsAttributeEditorElement *widgDef, mLayer->attributeEditorElements() )
    {
      QWidget* tabPage = new QWidget( tabWidget );

      tabWidget->addTab( tabPage, widgDef->name() );
      QGridLayout *tabPageLayout = new QGridLayout( tabPage );

      if ( widgDef->type() == QgsAttributeEditorElement::AeTypeContainer )
      {
        QString dummy1;
        bool dummy2;
        tabPageLayout->addWidget( QgsAttributeEditor::createWidgetFromDef( widgDef, tabPage, mLayer, *mFeature, mContext, dummy1, dummy2 ) );
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

  // Still no dialog: create the default generated dialog
  if ( !mDialog )
  {
    mDialog = new QDialog( qobject_cast<QWidget*>( parent() ) );

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
      QString myFieldName = mLayer->attributeDisplayName( fldIdx );
      // by default (until user defined alias) append date format
      // (validator does not allow to enter a value in wrong format)
      const QgsField &myField = theFields[fldIdx];
      if ( myField.type() == QVariant::Date && mLayer->attributeAlias( fldIdx ).isEmpty() )
      {
        myFieldName += " (" + mLayer->dateFormat( fldIdx ) + ")";
      }

      QWidget *myWidget = QgsAttributeEditor::createAttributeEditor( mDialog, 0, mLayer, fldIdx, mFeature->attribute( fldIdx ), mContext );
      if ( !myWidget )
        continue;

      QLabel *mypLabel = new QLabel( myFieldName, mypInnerFrame );

      if ( mLayer->editType( fldIdx ) != QgsVectorLayer::Immutable )
      {
        if ( mLayer->isEditable() && mLayer->fieldEditable( fldIdx ) )
        {
          myWidget->setEnabled( true );
        }
        else if ( mLayer->editType( fldIdx ) == QgsVectorLayer::Photo )
        {
          foreach ( QWidget *w, myWidget->findChildren<QWidget *>() )
          {
            w->setEnabled( qobject_cast<QLabel *>( w ) ? true : false );
          }
        }
        else if ( mLayer->editType( fldIdx ) == QgsVectorLayer::WebView )
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
        else if ( mLayer->editType( fldIdx ) == QgsVectorLayer::EditorWidgetV2 )
        {
          QgsEditorWidgetWrapper* ww = QgsEditorWidgetWrapper::fromWidget( myWidget );
          if ( ww )
          {
            ww->setEnabled( false );
          }
        }
        else
        {
          myWidget->setEnabled( false );
        }
      }

      if ( mLayer->labelOnTop( fldIdx ) )
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

    QList<QgsRelation> relations = QgsProject::instance()->relationManager()->referencedRelations( mLayer );

    foreach ( const QgsRelation& relation, relations )
    {
      QgsRelationEditorWidget *myWidget = QgsRelationEditorWidget::createRelationEditor( relation, *mFeature, mContext, mDialog );
      if ( !myWidget )
        continue;

      myWidget->setProperty( "qgisRelation", relation.id() );
      myWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
      mypInnerLayout->addWidget( myWidget, index, 0, 1, 2 );
      mypInnerLayout->setRowStretch( index, 2 );
      ++index;
    }

    // Set focus to first widget in list, to help entering data without moving the mouse.
    if ( mypInnerLayout->rowCount() > 0 )
    {
      QWidget* widget = mypInnerLayout->itemAtPosition( 0, 1 )->widget();
      if ( widget )
        widget->setFocus( Qt::OtherFocusReason );
    }

    QSpacerItem *mypSpacer = new QSpacerItem( 0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding );
    mypInnerLayout->addItem( mypSpacer, mypInnerLayout->rowCount() + 1, 0 );
  }
  else
  {
#if 0
    QgsDistanceArea myDa;

    myDa.setSourceCrs( vl->crs().srsid() );
    myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapRenderer()->hasCrsTransformEnabled() );
    myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
#endif

    // Get all widgets on the dialog
    QList<QWidget *> myWidgets = mDialog->findChildren<QWidget*>();
    Q_FOREACH( QWidget* myWidget, myWidgets )
    {
      // Check the widget's properties for a relation definition
      QVariant vRel = myWidget->property( "qgisRelation" );
      if ( vRel.isValid() )
      {
        QgsRelationManager* relMgr = QgsProject::instance()->relationManager();
        QgsRelation relation = relMgr->relation( vRel.toString() );
        if ( relation.isValid() )
        {
          QgsRelationEditorWidget *relWdg = QgsRelationEditorWidget::createRelationEditor( relation, *mFeature, mContext, myWidget );
          if ( !myWidget->layout() )
          {
            myWidget->setLayout( new QHBoxLayout() );
          }
          myWidget->layout()->addWidget( relWdg );
        }
      }
      else
      {
        // No widget definition properties defined, check if the widget's
        // objectName matches a field name
        for ( int fldIdx = 0; fldIdx < theFields.count(); ++fldIdx )
        {
          if ( myWidget->objectName() == theFields[fldIdx].name() )
          {
            QgsAttributeEditor::createAttributeEditor( mDialog, myWidget, mLayer, fldIdx, mFeature->attribute( fldIdx ), mContext );

            if ( mLayer->editType( fldIdx ) != QgsVectorLayer::Immutable )
            {
              if ( mLayer->isEditable() && mLayer->fieldEditable( fldIdx ) )
              {
                myWidget->setEnabled( true );
              }
              else if ( mLayer->editType( fldIdx ) == QgsVectorLayer::Photo )
              {
                foreach ( QWidget *w, myWidget->findChildren<QWidget *>() )
                {
                  w->setEnabled( qobject_cast<QLabel *>( w ) ? true : false );
                }
              }
              else if ( mLayer->editType( fldIdx ) == QgsVectorLayer::WebView )
              {
                foreach ( QWidget *w, myWidget->findChildren<QWidget *>() )
                {
                  w->setEnabled( qobject_cast<QWebView *>( w ) ? true : false );
                }
              }
              else if ( mLayer->editType( fldIdx ) == QgsVectorLayer::EditorWidgetV2 )
              {
                QgsEditorWidgetWrapper* ww = QgsEditorWidgetWrapper::fromWidget( myWidget );
                if ( ww )
                {
                  ww->setEnabled( false );
                }
              }
              else
              {
                myWidget->setEnabled( false );
              }
            }
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
        if ( mLayer->getFeatures( QgsFeatureRequest().setFilterFid( mFeature->id() ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) && f.geometry() )
        {
          mFeature->setGeometry( *f.geometry() );
        }
      }

      exp.setGeomCalculator( mContext.distanceArea() );

      QVariant value = exp.evaluate( mFeature, mLayer->pendingFields() );

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

  mEditable = mLayer->isEditable();

  if ( mDialog )
  {
    if ( mDialog->objectName().isEmpty() )
      mDialog->setObjectName( "QgsAttributeDialogBase" );

    if ( mDialog->windowTitle().isEmpty() )
      mDialog->setWindowTitle( tr( "Attributes - %1" ).arg( mLayer->name() ) );
  }

  if ( mShowDialogButtons )
  {
    if ( buttonBox )
    {
      buttonBox->clear();

      if ( mLayer->isEditable() )
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
      // Add dummy buttons
      if ( mLayer->isEditable() )
      {
        buttonBox->clear();

        buttonBox->setStandardButtons( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
        connect( buttonBox, SIGNAL( accepted() ), mDialog, SLOT( accept() ) );
        connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
      }

      buttonBox->setVisible( false );
    }
  }

  QMetaObject::connectSlotsByName( mDialog );

  connect( mDialog, SIGNAL( destroyed() ), this, SLOT( dialogDestroyed() ) );

  if ( !mLayer->editFormInit().isEmpty() )
  {
#if 0
    // would be nice if only PyQt's QVariant.toPyObject() wouldn't take ownership
    vl->setProperty( "featureForm.dialog", QVariant::fromValue( qobject_cast<QObject*>( mDialog ) ) );
    vl->setProperty( "featureForm.id", QVariant( mpFeature->id() ) );
#endif

    QString module = mLayer->editFormInit();
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

    QString form =  QString( "_qgis_featureform_%1 = sip.wrapinstance( %2, QtGui.QDialog )" )
                    .arg( mFormNr )
                    .arg(( unsigned long ) mDialog );

    QString layer = QString( "_qgis_layer_%1 = sip.wrapinstance( %2, qgis.core.QgsVectorLayer )" )
                    .arg( mLayer->id() )
                    .arg(( unsigned long ) mLayer );

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

    mReturnvarname = QString( "_qgis_feature_form_%1" ).arg( dt.toString( "yyyyMMddhhmmsszzz" ) );
    QString expr = QString( "%5 = %1(_qgis_featureform_%2, _qgis_layer_%3, %4)" )
                   .arg( mLayer->editFormInit() )
                   .arg( mFormNr )
                   .arg( mLayer->id() )
                   .arg( featurevarname )
                   .arg( mReturnvarname );

    QgsDebugMsg( QString( "running featureForm init: %1" ).arg( expr ) );
    QgsPythonRunner::run( expr );
  }

  // Only restore the geometry of the dialog if it's not a custom one.
  if ( mLayer->editorLayout() != QgsVectorLayer::UiFileLayout )
  {
    restoreGeometry();
  }
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

  // Only save the geometry of the dialog if it's not a custom one.
  if ( mLayer->editorLayout() != QgsVectorLayer::UiFileLayout )
  {
    saveGeometry();
  }

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

    if ( QgsAttributeEditor::retrieveValue( mContext.proxyWidget( mLayer, idx ), mLayer, idx, value ) )
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

  if ( !mReturnvarname.isEmpty() )
  {
    QString expr = QString( "if locals().has_key('%1'): del %1\n" ).arg( mReturnvarname );
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
