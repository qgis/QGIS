/***************************************************************************
                          qgsdlgvectorlayerproperties.cpp
                   Unified property dialog for vector layers
                             -------------------
    begin                : 2004-01-28
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>
#include <limits>

#include "qgisapp.h"
#include "qgsaddjoindialog.h"
#include "qgsapplication.h"
#include "qgsattributeactiondialog.h"
#include "qgsapplydialog.h"
#include "qgscontexthelp.h"
#include "qgscoordinatetransform.h"
#include "qgsdiagramproperties.h"
#include "qgsdiagramrendererv2.h"
#include "qgsfieldcalculator.h"
#include "qgsfieldsproperties.h"
#include "qgslabeldialog.h"
#include "qgslabelinggui.h"
#include "qgslabel.h"
#include "qgslegenditem.h"
//#include "qgsloadstylefromdbdialog.h"
#include "qgsgenericprojectionselector.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgspluginmetadata.h"
#include "qgspluginregistry.h"
#include "qgsproject.h"
#include "qgssavestyletodbdialog.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerproperties.h"
#include "qgsconfig.h"
#include "qgsvectordataprovider.h"
#include "qgsvectoroverlayplugin.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"

#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QSettings>
#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QColorDialog>

#include "qgsrendererv2propertiesdialog.h"
#include "qgsstylev2.h"
#include "qgssymbologyv2conversion.h"

QgsVectorLayerProperties::QgsVectorLayerProperties(
  QgsVectorLayer *lyr,
  QWidget * parent,
  Qt::WFlags fl
)
    : QDialog( parent, fl )
    , layer( lyr )
    , mMetadataFilled( false )
    , mRendererDialog( 0 )
{
  setupUi( this );

  mMaximumScaleIconLabel->setPixmap( QgsApplication::getThemePixmap( "/mActionZoomIn.png" ) );
  mMinimumScaleIconLabel->setPixmap( QgsApplication::getThemePixmap( "/mActionZoomOut.png" ) );

  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( apply() ) );

  connect( insertFieldButton, SIGNAL( clicked() ), this, SLOT( insertField() ) );
  connect( insertExpressionButton, SIGNAL( clicked() ), this, SLOT( insertExpression() ) );

  QVBoxLayout *layout;

  if ( layer->hasGeometryType() )
  {
    // Create the Labeling dialog tab
    layout = new QVBoxLayout( labelingFrame );
    layout->setMargin( 0 );
    labelingDialog = new QgsLabelingGui( QgisApp::instance()->palLabeling(), layer, QgisApp::instance()->mapCanvas(), labelingFrame );
    layout->addWidget( labelingDialog );
    labelingFrame->setLayout( layout );

    // Create the Labeling (deprecated) dialog tab
    layout = new QVBoxLayout( labelOptionsFrame );
    layout->setMargin( 0 );
    labelDialog = new QgsLabelDialog( layer->label(), labelOptionsFrame );
    layout->addWidget( labelDialog );
    labelOptionsFrame->setLayout( layout );
    connect( labelDialog, SIGNAL( labelSourceSet() ), this, SLOT( setLabelCheckBox() ) );
  }
  else
  {
    labelingDialog = 0;
    labelDialog = 0;
    tabWidget->setTabEnabled( 1, false ); // hide labeling item
    tabWidget->setTabEnabled( 2, false ); // hide labeling (deprecated) item
  }

  // Create the Actions dialog tab
  QVBoxLayout *actionLayout = new QVBoxLayout( actionOptionsFrame );
  actionLayout->setMargin( 0 );
  const QgsFields &fields = layer->pendingFields();
  actionDialog = new QgsAttributeActionDialog( layer->actions(), fields, actionOptionsFrame );
  actionLayout->addWidget( actionDialog );

  // Create the menu for the save style button to choose the output format
  mSaveAsMenu = new QMenu( pbnSaveStyleAs );
  mSaveAsMenu->addAction( tr( "QGIS Layer Style File" ) );
  mSaveAsMenu->addAction( tr( "SLD File" ) );

  //Only if the provider support loading & saving style to db add new choices
  if( layer->dataProvider()->isSavingStyleToDBSupported() )
  {
      //for loading
      mLoadStyleMenu =  new QMenu();
      mLoadStyleMenu->addAction( tr( "Load from file" ) );
      mLoadStyleMenu->addAction( tr( "Load from database" ) );
      pbnLoadStyle->setContextMenuPolicy( Qt::PreventContextMenu );
      pbnLoadStyle->setMenu( mLoadStyleMenu );

      QObject::connect( mLoadStyleMenu, SIGNAL( triggered( QAction * ) ),
                        this, SLOT( loadStyleMenuTriggered( QAction * ) ) ) ;

      //for saving
      mSaveAsMenu->addAction( tr( "Save on database (%1)" ).arg( layer->providerType() ) );
  }

  QObject::connect( mSaveAsMenu, SIGNAL( triggered( QAction * ) ),
                    this, SLOT( saveStyleAsMenuTriggered( QAction * ) ) );

  mFieldsPropertiesDialog = new QgsFieldsProperties( layer, mFieldsFrame );
  mFieldsFrame->setLayout( new QVBoxLayout( mFieldsFrame ) );
  mFieldsFrame->layout()->addWidget( mFieldsPropertiesDialog );

  connect( mFieldsPropertiesDialog, SIGNAL( toggleEditing() ), this, SLOT( toggleEditing() ) );
  connect( this, SIGNAL( toggleEditing( QgsMapLayer* ) ), QgisApp::instance(), SLOT( toggleEditing( QgsMapLayer* ) ) );

  reset();

  if ( layer->dataProvider() )//enable spatial index button group if supported by provider
  {
    int capabilities = layer->dataProvider()->capabilities();
    if ( !( capabilities & QgsVectorDataProvider::CreateSpatialIndex ) )
    {
      pbnIndex->setEnabled( false );
    }

    if ( capabilities & QgsVectorDataProvider::SetEncoding )
    {
      cboProviderEncoding->addItems( QgsVectorDataProvider::availableEncodings() );
      QString enc = layer->dataProvider()->encoding();
      int encindex = cboProviderEncoding->findText( enc );
      if ( encindex < 0 )
      {
        cboProviderEncoding->insertItem( 0, enc );
        encindex = 0;
      }
      cboProviderEncoding->setCurrentIndex( encindex );
    }
    else
    {
      // currently only encoding can be set in this group, so hide it completely
      grpProviderOptions->hide();
    }
  }

  leSpatialRefSys->setText( layer->crs().authid() + " - " + layer->crs().description() );
  leSpatialRefSys->setCursorPosition( 0 );

  //insert existing join info
  const QList< QgsVectorJoinInfo >& joins = layer->vectorJoins();
  for ( int i = 0; i < joins.size(); ++i )
  {
    addJoinToTreeWidget( joins[i] );
  }

  diagramPropertiesDialog = new QgsDiagramProperties( layer, mDiagramFrame );
  mDiagramFrame->setLayout( new QVBoxLayout( mDiagramFrame ) );
  mDiagramFrame->layout()->addWidget( diagramPropertiesDialog );

  //layer title and abstract
  if ( layer )
  {
    mLayerTitleLineEdit->setText( layer->title() );
    mLayerAbstractTextEdit->setPlainText( layer->abstract() );
  }

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/VectorLayerProperties/geometry" ).toByteArray() );
  int tabIndex = settings.value( "/Windows/VectorLayerProperties/row", 0 ).toInt();

  // if the last used tab is not enabled display the first enabled one
  if ( !tabWidget->isTabEnabled( tabIndex ) )
  {
    tabIndex = 0;
    for ( int i = 0; i < tabWidget->count(); i++ )
    {
      if ( tabWidget->isTabEnabled( i ) )
      {
        tabIndex = i;
        break;
      }
    }
  }
  tabWidget->setCurrentIndex( tabIndex );

  setWindowTitle( tr( "Layer Properties - %1" ).arg( layer->name() ) );
} // QgsVectorLayerProperties ctor


QgsVectorLayerProperties::~QgsVectorLayerProperties()
{
  if ( layer->hasGeometryType() )
  {
    disconnect( labelDialog, SIGNAL( labelSourceSet() ), this, SLOT( setLabelCheckBox() ) );
  }

  QSettings settings;
  settings.setValue( "/Windows/VectorLayerProperties/geometry", saveGeometry() );
  settings.setValue( "/Windows/VectorLayerProperties/row", tabWidget->currentIndex() );
}

void QgsVectorLayerProperties::toggleEditing()
{
  emit toggleEditing( layer );

  pbnQueryBuilder->setEnabled( layer && layer->dataProvider() && layer->dataProvider()->supportsSubsetString() &&
                               !layer->isEditable() && layer->vectorJoins().size() < 1 );
  if ( layer->isEditable() )
  {
    pbnQueryBuilder->setToolTip( tr( "Stop editing mode to enable this." ) );
  }
}

void QgsVectorLayerProperties::setLabelCheckBox()
{
  labelCheckBox->setCheckState( Qt::Checked );
}

void QgsVectorLayerProperties::insertField()
{
  // Convert the selected field to an expression and
  // insert it into the action at the cursor position

  if ( !fieldComboBox->currentText().isNull() )
  {
    QString field = "[% \"";
    field += fieldComboBox->currentText();
    field += "\" %]";
    htmlMapTip->insertPlainText( field );
  }
}

void QgsVectorLayerProperties::insertExpression()
{
  QString selText = htmlMapTip->textCursor().selectedText();

  // edit the selected expression if there's one
  if ( selText.startsWith( "[%" ) && selText.endsWith( "%]" ) )
    selText = selText.mid( 2, selText.size() - 4 );

  // display the expression builder
  QgsExpressionBuilderDialog dlg( layer , selText, this );
  dlg.setWindowTitle( tr( "Insert expression" ) );
  if ( dlg.exec() == QDialog::Accepted )
  {
    QString expression =  dlg.expressionBuilder()->expressionText();
    //Only add the expression if the user has entered some text.
    if ( !expression.isEmpty() )
    {
      htmlMapTip->insertPlainText( "[%" + expression + "%]" );
    }
  }
}

void QgsVectorLayerProperties::setDisplayField( QString name )
{
  int idx = displayFieldComboBox->findText( name );
  if ( idx == -1 )
  {
    htmlRadio->setChecked( true );
    htmlMapTip->setPlainText( name );
  }
  else
  {
    fieldComboRadio->setChecked( true );
    displayFieldComboBox->setCurrentIndex( idx );
  }
}

//! @note in raster props, this method is called sync()
void QgsVectorLayerProperties::reset( void )
{
  // populate the general information
  mLayerOrigNameLineEdit->setText( layer->originalName() );
  txtDisplayName->setText( layer->name() );
  pbnQueryBuilder->setWhatsThis( tr( "This button opens the query "
                                     "builder and allows you to create a subset of features to display on "
                                     "the map canvas rather than displaying all features in the layer" ) );
  txtSubsetSQL->setWhatsThis( tr( "The query used to limit the features in the "
                                  "layer is shown here. To enter or modify the query, click on the Query Builder button" ) );

  //see if we are dealing with a pg layer here
  grpSubset->setEnabled( true );
  txtSubsetSQL->setText( layer->subsetString() );
  // if the user is allowed to type an adhoc query, the app will crash if the query
  // is bad. For this reason, the sql box is disabled and the query must be built
  // using the query builder, either by typing it in by hand or using the buttons, etc
  // on the builder. If the ability to enter a query directly into the box is required,
  // a mechanism to check it must be implemented.
  txtSubsetSQL->setEnabled( false );
  pbnQueryBuilder->setEnabled( layer && layer->dataProvider() && layer->dataProvider()->supportsSubsetString() &&
                               !layer->isEditable() && layer->vectorJoins().size() < 1 );
  if ( layer->isEditable() )
  {
    pbnQueryBuilder->setToolTip( tr( "Stop editing mode to enable this." ) );
  }

  //get field list for display field combo
  const QgsFields& myFields = layer->pendingFields();
  for ( int idx = 0; idx < myFields.count(); ++idx )
  {
    displayFieldComboBox->addItem( myFields[idx].name() );
    fieldComboBox->addItem( myFields[idx].name() );
  }

  setDisplayField( layer-> displayField() );

  // set up the scale based layer visibility stuff....
  chkUseScaleDependentRendering->setChecked( layer->hasScaleBasedVisibility() );
  bool projectScales = QgsProject::instance()->readBoolEntry( "Scales", "/useProjectScales" );
  if ( projectScales )
  {
    QStringList scalesList = QgsProject::instance()->readListEntry( "Scales", "/ScalesList" );
    cbMinimumScale->updateScales( scalesList );
    cbMaximumScale->updateScales( scalesList );
  }
  cbMinimumScale->setScale( 1.0 / layer->minimumScale() );
  cbMaximumScale->setScale( 1.0 / layer->maximumScale() );

  // load appropriate symbology page (V1 or V2)
  updateSymbologyPage();

  // reset fields in label dialog
  layer->label()->setFields( layer->pendingFields() );

  actionDialog->init();

  if ( layer->hasGeometryType() )
  {
    labelDialog->init();
  }
  labelCheckBox->setChecked( layer->hasLabelsEnabled() );
  labelOptionsFrame->setEnabled( layer->hasLabelsEnabled() );

  mFieldsPropertiesDialog->init();

  QObject::connect( labelCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( enableLabelOptions( bool ) ) );
} // reset()



void QgsVectorLayerProperties::apply()
{
  if ( labelingDialog )
  {
    labelingDialog->writeSettingsToLayer();
  }

  //
  // Set up sql subset query if applicable
  //
  grpSubset->setEnabled( true );

  if ( txtSubsetSQL->toPlainText() != layer->subsetString() )
  {
    // set the subset sql for the layer
    layer->setSubsetString( txtSubsetSQL->toPlainText() );
    mMetadataFilled = false;
  }

  // set up the scale based layer visibility stuff....
  layer->toggleScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  layer->setMinimumScale( 1.0 / cbMinimumScale->scale() );
  layer->setMaximumScale( 1.0 / cbMaximumScale->scale() );

  // provider-specific options
  if ( layer->dataProvider() )
  {
    if ( layer->dataProvider()->capabilities() & QgsVectorDataProvider::SetEncoding )
    {
      layer->setProviderEncoding( cboProviderEncoding->currentText() );
    }
  }

  // update the display field
  if ( htmlRadio->isChecked() )
  {
    layer->setDisplayField( htmlMapTip->toPlainText() );
  }

  if ( fieldComboRadio->isChecked() )
  {
    layer->setDisplayField( displayFieldComboBox->currentText() );
  }

  actionDialog->apply();

  if ( labelDialog )
    labelDialog->apply();
  layer->enableLabels( labelCheckBox->isChecked() );
  layer->setLayerName( mLayerOrigNameLineEdit->text() );

  // Apply fields settings
  mFieldsPropertiesDialog->apply();

  if ( layer->rendererV2() )
  {
    QgsRendererV2PropertiesDialog* dlg = static_cast<QgsRendererV2PropertiesDialog*>( widgetStackRenderers->currentWidget() );
    dlg->apply();
  }

  //apply diagram settings
  diagramPropertiesDialog->apply();

  //apply overlay dialogs
  for ( QList<QgsApplyDialog*>::iterator it = mOverlayDialogs.begin(); it != mOverlayDialogs.end(); ++it )
  {
    ( *it )->apply();
  }

  //layer title and abstract
  layer->setTitle( mLayerTitleLineEdit->text() );
  layer->setAbstract( mLayerAbstractTextEdit->toPlainText() );

  // update symbology
  emit refreshLegend( layer->id(), QgsLegendItem::DontChange );

  //no need to delete the old one, maplayer will do it if needed
  layer->setCacheImage( 0 );

  layer->triggerRepaint();
  // notify the project we've made a change
  QgsProject::instance()->dirty( true );
}

void QgsVectorLayerProperties::on_pbnQueryBuilder_clicked()
{
  // launch the query builder
  QgsQueryBuilder *qb = new QgsQueryBuilder( layer, this );

  // Set the sql in the query builder to the same in the prop dialog
  // (in case the user has already changed it)
  qb->setSql( txtSubsetSQL->toPlainText() );
  // Open the query builder
  if ( qb->exec() )
  {
    // if the sql is changed, update it in the prop subset text box
    txtSubsetSQL->setText( qb->sql() );
    //TODO If the sql is changed in the prop dialog, the layer extent should be recalculated

    // The datasource for the layer needs to be updated with the new sql since this gets
    // saved to the project file. This should happen at the map layer level...

  }
  // delete the query builder object
  delete qb;
}

void QgsVectorLayerProperties::on_pbnIndex_clicked()
{
  QgsVectorDataProvider* pr = layer->dataProvider();
  if ( pr )
  {
    setCursor( Qt::WaitCursor );
    bool errval = pr->createSpatialIndex();
    setCursor( Qt::ArrowCursor );
    if ( errval )
    {
      QMessageBox::information( this, tr( "Spatial Index" ), tr( "Creation of spatial index successful" ) );
    }
    else
    {
      // TODO: Remind the user to use OGR >= 1.2.6 and Shapefile
      QMessageBox::information( this, tr( "Spatial Index" ), tr( "Creation of spatial index failed" ) );
    }
  }
}

QString QgsVectorLayerProperties::metadata()
{
  return layer->metadata();
}

void QgsVectorLayerProperties::on_mLayerOrigNameLineEdit_textEdited( const QString& text )
{
  txtDisplayName->setText( layer->capitaliseLayerName( text ) );
}

void QgsVectorLayerProperties::on_pbnChangeSpatialRefSys_clicked()
{
  QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector( this );
  mySelector->setMessage();
  mySelector->setSelectedCrsId( layer->crs().srsid() );
  if ( mySelector->exec() )
  {
    QgsCoordinateReferenceSystem srs( mySelector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );
    layer->setCrs( srs );
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }
  delete mySelector;

  leSpatialRefSys->setText( layer->crs().authid() + " - " + layer->crs().description() );
  leSpatialRefSys->setCursorPosition( 0 );
}

void QgsVectorLayerProperties::on_pbnLoadDefaultStyle_clicked()
{
  bool defaultLoadedFlag = false;
  QString myMessage = layer->loadDefaultStyle( defaultLoadedFlag );
  //reset if the default style was loaded ok only
  if ( defaultLoadedFlag )
  {
    // all worked ok so no need to inform user
    reset();
  }
  else
  {
    //something went wrong - let them know why
    QMessageBox::information( this, tr( "Default Style" ), myMessage );
  }
}

void QgsVectorLayerProperties::on_pbnSaveDefaultStyle_clicked()
{
  apply(); // make sure the qml to save is uptodate

  // a flag passed by reference
  bool defaultSavedFlag = false;
  // after calling this the above flag will be set true for success
  // or false if the save operation failed
  QString myMessage = layer->saveDefaultStyle( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    //only raise the message if something went wrong
    QMessageBox::information( this, tr( "Default Style" ), myMessage );
  }
}


void QgsVectorLayerProperties::on_pbnLoadStyle_clicked()
{
  QSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( "style/lastStyleDir", "." ).toString();

  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load layer properties from style file" ), myLastUsedDir,
                       tr( "QGIS Layer Style File" ) + " (*.qml);;" + tr( "SLD File" ) + " (*.sld)" );
  if ( myFileName.isNull() )
  {
    return;
  }

  QString myMessage;
  bool defaultLoadedFlag = false;

  if ( myFileName.endsWith( ".sld", Qt::CaseInsensitive ) )
  {
    // load from SLD
    myMessage = layer->loadSldStyle( myFileName, defaultLoadedFlag );
  }
  else
  {
    myMessage = layer->loadNamedStyle( myFileName, defaultLoadedFlag );
  }
  //reset if the default style was loaded ok only
  if ( defaultLoadedFlag )
  {
    reset();
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::information( this, tr( "Load Style" ), myMessage );
  }

  QFileInfo myFI( myFileName );
  QString myPath = myFI.path();
  myQSettings.setValue( "style/lastStyleDir", myPath );
}


void QgsVectorLayerProperties::on_pbnSaveStyleAs_clicked()
{
  saveStyleAs( QML );
}

void QgsVectorLayerProperties::saveStyleAsMenuTriggered( QAction *action )
{
  QMenu *menu = qobject_cast<QMenu *>( sender() );
  if ( !menu )
    return;

  int index = mSaveAsMenu->actions().indexOf( action );
  if ( index < 0 )
    return;

  saveStyleAs(( StyleType ) index );
}

void QgsVectorLayerProperties::saveStyleAs( StyleType styleType )
{
  QSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( "style/lastStyleDir", "." ).toString();

  if( styleType == DB )
  {
         QString infoWindowTitle = QObject::tr( "Save style to DB (%1)" ).arg( layer->providerType() );
         QString msgError;

         QgsSaveStyleToDbDialog askToUser;

         if( askToUser.exec() == QDialog::Accepted )
         {
             QString styleName = askToUser.getName();
             QString styleDesc = askToUser.getDescription();
             QString uiFileContent = askToUser.getUIFileContent();
             bool isDefault = askToUser.isDefault();

             apply();

             layer->saveStyleToDatabase( styleName, styleDesc, isDefault, uiFileContent, msgError );
             if( !msgError.isNull() )
             {
                 QMessageBox::warning( this, infoWindowTitle, msgError );
             }
             else
             {
                 QMessageBox::information( this, infoWindowTitle, tr( "Style saved" ) );
             }
         }
         else
         {
             return;
         }
  }
  else
  {

      QString format, extension;
      if ( styleType == SLD )
      {
        format = tr( "SLD File" ) + " (*.sld)";
        extension = ".sld";
      }
      else
      {
        format =  tr( "QGIS Layer Style File" ) + " (*.qml)";
        extension = ".qml";
      }

      QString myOutputFileName = QFileDialog::getSaveFileName( this, tr( "Save layer properties as style file" ),
                                 myLastUsedDir, format );
      if ( myOutputFileName.isNull() ) //dialog canceled
      {
        return;
      }

      apply(); // make sure the style to save is uptodate

      QString myMessage;
      bool defaultLoadedFlag = false;

      //ensure the user never omitted the extension from the file name
      if ( !myOutputFileName.endsWith( extension, Qt::CaseInsensitive ) )
      {
        myOutputFileName += extension;
      }

      if ( styleType == SLD )
      {
        // convert to SLD
        myMessage = layer->saveSldStyle( myOutputFileName, defaultLoadedFlag );
      }
      else
      {
        myMessage = layer->saveNamedStyle( myOutputFileName, defaultLoadedFlag );
      }

      //reset if the default style was loaded ok only
      if ( defaultLoadedFlag )
      {
        reset();
      }
      else
      {
        //let the user know what went wrong
        QMessageBox::information( this, tr( "Saved Style" ), myMessage );
      }

      QFileInfo myFI( myOutputFileName );
      QString myPath = myFI.path();
      // Persist last used dir
      myQSettings.setValue( "style/lastStyleDir", myPath );
  }
}

void QgsVectorLayerProperties::loadStyleMenuTriggered( QAction *action )
{
  QMenu *menu = qobject_cast<QMenu *>( sender() );
  if ( !menu )
    return;

  int index = mLoadStyleMenu->actions().indexOf( action );

  if ( index == 0 ) //Load from filesystem
  {
     this->on_pbnLoadStyle_clicked();
  }
  else if( index == 1 ) //Load from database
  {
     this->showListOfStylesFromDatabase();
  }

}

void QgsVectorLayerProperties::showListOfStylesFromDatabase()
{
//    QgsLoadStyleFromDBDialog dialog;
}

QList<QgsVectorOverlayPlugin*> QgsVectorLayerProperties::overlayPlugins() const
{
  QList<QgsVectorOverlayPlugin*> pluginList;

  QgisPlugin* thePlugin = 0;
  QgsVectorOverlayPlugin* theOverlayPlugin = 0;

  QList<QgsPluginMetadata*> pluginData = QgsPluginRegistry::instance()->pluginData();
  for ( QList<QgsPluginMetadata*>::iterator it = pluginData.begin(); it != pluginData.end(); ++it )
  {
    if ( *it )
    {
      thePlugin = ( *it )->plugin();
      if ( thePlugin && thePlugin->type() == QgisPlugin::VECTOR_OVERLAY )
      {
        theOverlayPlugin = dynamic_cast<QgsVectorOverlayPlugin *>( thePlugin );
        if ( theOverlayPlugin )
        {
          pluginList.push_back( theOverlayPlugin );
        }
      }
    }
  }

  return pluginList;
}


void QgsVectorLayerProperties::on_mButtonAddJoin_clicked()
{
  QgsAddJoinDialog d( layer );
  if ( d.exec() == QDialog::Accepted )
  {
    QgsVectorJoinInfo info;
    info.targetFieldName = d.targetFieldName();
    info.joinLayerId = d.joinedLayerId();
    info.joinFieldName = d.joinFieldName();
    info.memoryCache = d.cacheInMemory();
    if ( layer )
    {
      //create attribute index if possible
      if ( d.createAttributeIndex() )
      {
        QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( info.joinLayerId ) );
        if ( joinLayer )
        {
          joinLayer->dataProvider()->createAttributeIndex( joinLayer->pendingFields().indexFromName( info.joinFieldName ) );
        }
      }

      layer->addJoin( info );
      addJoinToTreeWidget( info );
      pbnQueryBuilder->setEnabled( layer && layer->dataProvider() && layer->dataProvider()->supportsSubsetString() &&
                                   !layer->isEditable() && layer->vectorJoins().size() < 1 );
    }
  }
}

void QgsVectorLayerProperties::addJoinToTreeWidget( const QgsVectorJoinInfo& join )
{
  QTreeWidgetItem* joinItem = new QTreeWidgetItem();

  QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( join.joinLayerId ) );
  if ( !layer || !joinLayer )
  {
    return;
  }

  joinItem->setText( 0, joinLayer->name() );
  joinItem->setData( 0, Qt::UserRole, join.joinLayerId );

  if ( join.joinFieldName.isEmpty() && join.joinFieldIndex >= 0 && join.joinFieldIndex < joinLayer->pendingFields().count() )
    joinItem->setText( 1, joinLayer->pendingFields().field( join.joinFieldIndex ).name() );   //for compatibility with 1.x
  else
    joinItem->setText( 1, join.joinFieldName );

  if ( join.targetFieldName.isEmpty() && join.targetFieldIndex >= 0 && join.targetFieldIndex < layer->pendingFields().count() )
    joinItem->setText( 2, layer->pendingFields().field( join.targetFieldIndex ).name() );   //for compatibility with 1.x
  else
    joinItem->setText( 2, join.targetFieldName );

  mJoinTreeWidget->addTopLevelItem( joinItem );
}

void QgsVectorLayerProperties::on_mButtonRemoveJoin_clicked()
{
  QTreeWidgetItem* currentJoinItem = mJoinTreeWidget->currentItem();
  if ( !layer || !currentJoinItem )
  {
    return;
  }

  layer->removeJoin( currentJoinItem->data( 0, Qt::UserRole ).toString() );
  mJoinTreeWidget->takeTopLevelItem( mJoinTreeWidget->indexOfTopLevelItem( currentJoinItem ) );
  pbnQueryBuilder->setEnabled( layer && layer->dataProvider() && layer->dataProvider()->supportsSubsetString() &&
                               !layer->isEditable() && layer->vectorJoins().size() < 1 );
}


void QgsVectorLayerProperties::updateSymbologyPage()
{

  //find out the type of renderer in the vectorlayer, create a dialog with these settings and add it to the form
  delete mRendererDialog;
  mRendererDialog = 0;

  if ( layer->rendererV2() )
  {
    mRendererDialog = new QgsRendererV2PropertiesDialog( layer, QgsStyleV2::defaultStyle(), true );

    // display the menu to choose the output format (fix #5136)
    pbnSaveStyleAs->setText( tr( "Save Style" ) );
    pbnSaveStyleAs->setMenu( mSaveAsMenu );
    QObject::disconnect( pbnSaveStyleAs, SIGNAL( clicked() ), this, SLOT( on_pbnSaveStyleAs_clicked() ) );
  }
  else
  {
    tabWidget->setTabEnabled( 0, false ); // hide symbology item
  }

  if ( mRendererDialog )
  {
    widgetStackRenderers->addWidget( mRendererDialog );
    widgetStackRenderers->setCurrentWidget( mRendererDialog );
  }
}

void QgsVectorLayerProperties::on_pbnUpdateExtents_clicked()
{
  layer->updateExtents();
  mMetadataFilled = false;
}

void QgsVectorLayerProperties::on_tabWidget_currentChanged( int index )
{
  if ( index != 6 || mMetadataFilled )
    return;

  //set the metadata contents (which can be expensive)
  QString myStyle = QgsApplication::reportStyleSheet();
  teMetadata->clear();
  teMetadata->document()->setDefaultStyleSheet( myStyle );
  teMetadata->setHtml( metadata() );
  mMetadataFilled = true;
}

void QgsVectorLayerProperties::enableLabelOptions( bool theFlag )
{
  labelOptionsFrame->setEnabled( theFlag );
}

void QgsVectorLayerProperties::on_mMinimumScaleSetCurrentPushButton_clicked()
{
  cbMinimumScale->setScale( 1.0 / QgisApp::instance()->mapCanvas()->mapRenderer()->scale() );
}

void QgsVectorLayerProperties::on_mMaximumScaleSetCurrentPushButton_clicked()
{
  cbMaximumScale->setScale( 1.0 / QgisApp::instance()->mapCanvas()->mapRenderer()->scale() );
}
