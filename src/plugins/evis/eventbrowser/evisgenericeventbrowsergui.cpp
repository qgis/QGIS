/*
** File: evisgenericeventbrowsergui.cpp
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-03-08
**
** Copyright ( c ) 2007, American Museum of Natural History. All rights reserved.
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This library/program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** This work was made possible through a grant by the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/
#include "evisgenericeventbrowsergui.h"

#include "qgsapplication.h"
#include "qgsfeatureiterator.h"
#include "qgsmaptopixel.h"
#include "qgsmapcanvas.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgspointxy.h"
#include "qgsfields.h"
#include "qgsrectangle.h"
#include "qgsvectorlayer.h"

#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QGraphicsScene>
#include <QSettings>
#include <QPainter>
#include <QProcess>
#include <QFileDialog>

/**
* Constructor called when browser is launched from the application plugin tool bar
* \param parent - Pointer the to parent QWidget for modality
* \param interface - Pointer to the application interface
* \param fl - Window flags
*/
eVisGenericEventBrowserGui::eVisGenericEventBrowserGui( QWidget *parent, QgisInterface *interface, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mInterface( interface )
{
  setupUi( this );
  connect( buttonboxOptions, &QDialogButtonBox::clicked, this, &eVisGenericEventBrowserGui::buttonboxOptions_clicked );
  connect( chkboxApplyPathRulesToDocs, &QCheckBox::stateChanged, this, &eVisGenericEventBrowserGui::chkboxApplyPathRulesToDocs_stateChanged );
  connect( cboxEventImagePathField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &eVisGenericEventBrowserGui::cboxEventImagePathField_currentIndexChanged );
  connect( cboxCompassBearingField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &eVisGenericEventBrowserGui::cboxCompassBearingField_currentIndexChanged );
  connect( cboxCompassOffsetField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &eVisGenericEventBrowserGui::cboxCompassOffsetField_currentIndexChanged );
  connect( chkboxDisplayCompassBearing, &QCheckBox::stateChanged, this, &eVisGenericEventBrowserGui::chkboxDisplayCompassBearing_stateChanged );
  connect( chkboxEventImagePathRelative, &QCheckBox::stateChanged, this, &eVisGenericEventBrowserGui::chkboxEventImagePathRelative_stateChanged );
  connect( chkboxUseOnlyFilename, &QCheckBox::stateChanged, this, &eVisGenericEventBrowserGui::chkboxUseOnlyFilename_stateChanged );
  connect( displayArea, &QTabWidget::currentChanged, this, &eVisGenericEventBrowserGui::displayArea_currentChanged );
  connect( dsboxCompassOffset, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &eVisGenericEventBrowserGui::dsboxCompassOffset_valueChanged );
  connect( leBasePath, &QLineEdit::textChanged, this, &eVisGenericEventBrowserGui::leBasePath_textChanged );
  connect( pbtnAddFileType, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnAddFileType_clicked );
  connect( pbtnDeleteFileType, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnDeleteFileType_clicked );
  connect( pbtnNext, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnNext_clicked );
  connect( pbtnPrevious, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnPrevious_clicked );
  connect( pbtnResetApplyPathRulesToDocs, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnResetApplyPathRulesToDocs_clicked );
  connect( pbtnResetBasePathData, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnResetBasePathData_clicked );
  connect( pbtnResetCompassBearingData, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnResetCompassBearingData_clicked );
  connect( pbtnResetCompassOffsetData, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnResetCompassOffsetData_clicked );
  connect( pbtnResetEventImagePathData, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnResetEventImagePathData_clicked );
  connect( pbtnResetUseOnlyFilenameData, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnResetUseOnlyFilenameData_clicked );
  connect( rbtnManualCompassOffset, &QRadioButton::toggled, this, &eVisGenericEventBrowserGui::rbtnManualCompassOffset_toggled );
  connect( tableFileTypeAssociations, &QTableWidget::cellDoubleClicked, this, &eVisGenericEventBrowserGui::tableFileTypeAssociations_cellDoubleClicked );

  QSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "eVis/browser-geometry" ) ).toByteArray() );

  if ( initBrowser() )
  {
    loadRecord();
    show();
  }
  else
  {
    close();
  }
}

/**
* Constructor called when browser is launched by the eVisEventIdTool
* \param parent - Pointer to the parent QWidget for modality
* \param canvas - Pointer to the map canvas
* \param fl - Window flags
*/
eVisGenericEventBrowserGui::eVisGenericEventBrowserGui( QWidget *parent, QgsMapCanvas *canvas, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mCanvas( canvas )
{
  setupUi( this );
  connect( buttonboxOptions, &QDialogButtonBox::clicked, this, &eVisGenericEventBrowserGui::buttonboxOptions_clicked );
  connect( chkboxApplyPathRulesToDocs, &QCheckBox::stateChanged, this, &eVisGenericEventBrowserGui::chkboxApplyPathRulesToDocs_stateChanged );
  connect( cboxEventImagePathField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &eVisGenericEventBrowserGui::cboxEventImagePathField_currentIndexChanged );
  connect( cboxCompassBearingField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &eVisGenericEventBrowserGui::cboxCompassBearingField_currentIndexChanged );
  connect( cboxCompassOffsetField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &eVisGenericEventBrowserGui::cboxCompassOffsetField_currentIndexChanged );
  connect( chkboxDisplayCompassBearing, &QCheckBox::stateChanged, this, &eVisGenericEventBrowserGui::chkboxDisplayCompassBearing_stateChanged );
  connect( chkboxEventImagePathRelative, &QCheckBox::stateChanged, this, &eVisGenericEventBrowserGui::chkboxEventImagePathRelative_stateChanged );
  connect( chkboxUseOnlyFilename, &QCheckBox::stateChanged, this, &eVisGenericEventBrowserGui::chkboxUseOnlyFilename_stateChanged );
  connect( displayArea, &QTabWidget::currentChanged, this, &eVisGenericEventBrowserGui::displayArea_currentChanged );
  connect( dsboxCompassOffset, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &eVisGenericEventBrowserGui::dsboxCompassOffset_valueChanged );
  connect( leBasePath, &QLineEdit::textChanged, this, &eVisGenericEventBrowserGui::leBasePath_textChanged );
  connect( pbtnAddFileType, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnAddFileType_clicked );
  connect( pbtnDeleteFileType, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnDeleteFileType_clicked );
  connect( pbtnNext, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnNext_clicked );
  connect( pbtnPrevious, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnPrevious_clicked );
  connect( pbtnResetApplyPathRulesToDocs, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnResetApplyPathRulesToDocs_clicked );
  connect( pbtnResetBasePathData, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnResetBasePathData_clicked );
  connect( pbtnResetCompassBearingData, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnResetCompassBearingData_clicked );
  connect( pbtnResetCompassOffsetData, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnResetCompassOffsetData_clicked );
  connect( pbtnResetEventImagePathData, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnResetEventImagePathData_clicked );
  connect( pbtnResetUseOnlyFilenameData, &QPushButton::clicked, this, &eVisGenericEventBrowserGui::pbtnResetUseOnlyFilenameData_clicked );
  connect( rbtnManualCompassOffset, &QRadioButton::toggled, this, &eVisGenericEventBrowserGui::rbtnManualCompassOffset_toggled );
  connect( tableFileTypeAssociations, &QTableWidget::cellDoubleClicked, this, &eVisGenericEventBrowserGui::tableFileTypeAssociations_cellDoubleClicked );

  if ( initBrowser() )
  {
    loadRecord();
    show();
  }
  else
  {
    close();
  }
}


/**
 * Basic destructor
 */
eVisGenericEventBrowserGui::~eVisGenericEventBrowserGui()
{
  QSettings settings;
  settings.setValue( QStringLiteral( "eVis/browser-geometry" ), saveGeometry() );

  //Clean up, disconnect the highlighting routine and refresh the canvas to clear highlighting symbol
  if ( mCanvas )
  {
    disconnect( mCanvas, &QgsMapCanvas::renderComplete, this, &eVisGenericEventBrowserGui::renderSymbol );
    mCanvas->refresh();
  }

  //On close, clear selected feature
  if ( mVectorLayer )
  {
    mVectorLayer->removeSelection();
  }
}

/**
 * This method is an extension of the constructor. It was implemented to reduce the amount of code duplicated between the constructors.
 */
bool eVisGenericEventBrowserGui::initBrowser()
{

  //setup gui
  setWindowTitle( tr( "Generic Event Browser" ) );

  connect( treeEventData, &QTreeWidget::itemDoubleClicked, this, &eVisGenericEventBrowserGui::launchExternalApplication );

  mHighlightSymbol.load( QStringLiteral( ":/evis/eVisHighlightSymbol.png" ) );
  mPointerSymbol.load( QStringLiteral( ":/evis/eVisPointerSymbol.png" ) );
  mCompassOffset = 0.0;

  //Flag to let us know if the browser fully loaded
  mBrowserInitialized = false;

  //Initialize some class variables
  mDefaultEventImagePathField = 0;
  mDefaultCompassBearingField = 0;
  mDefaultCompassOffsetField = 0;

  //initialize Display tab GUI elements
  pbtnNext->setEnabled( false );
  pbtnPrevious->setEnabled( false );


  //Set up Attribute display
  treeEventData->setColumnCount( 2 );
  QStringList treeHeaders;
  treeHeaders << tr( "Field" ) << tr( "Value" );
  treeEventData->setHeaderLabels( treeHeaders );

  //Initialize Options tab GUI elements
  cboxEventImagePathField->setEnabled( true );
  chkboxEventImagePathRelative->setChecked( false );

  chkboxDisplayCompassBearing->setChecked( false );
  cboxCompassBearingField->setEnabled( true );

  rbtnManualCompassOffset->setChecked( false );
  dsboxCompassOffset->setEnabled( true );
  dsboxCompassOffset->setValue( 0.0 );
  rbtnAttributeCompassOffset->setChecked( false );
  cboxCompassOffsetField->setEnabled( true );


  chkboxUseOnlyFilename->setChecked( false );


  chkboxSaveEventImagePathData->setChecked( false );
  chkboxSaveCompassBearingData->setChecked( false );
  chkboxSaveCompassOffsetData->setChecked( false );
  chkboxSaveBasePathData->setChecked( false );
  chkboxSaveUseOnlyFilenameData->setChecked( false );


  //Check to for interface, not null when launched from plugin toolbar, otherwise expect map canvas
  if ( mInterface )
  {
    //check for active layer
    if ( mInterface->activeLayer() )
    {
      //verify that the active layer is a vector layer
      if ( QgsMapLayer::VectorLayer == mInterface->activeLayer()->type() )
      {
        mVectorLayer = qobject_cast< QgsVectorLayer * >( mInterface->activeLayer() );
        mCanvas = mInterface->mapCanvas();
      }
      else
      {
        QMessageBox::warning( this, tr( "Generic Event Browser" ), tr( "This tool only supports vector data." ) );
        return false;
      }
    }
    else
    {
      QMessageBox::warning( this, tr( "Generic Event Browser" ), tr( "No active layers found." ) );
      return false;
    }
  }
  //check for map canvas, if map canvas is null, throw error
  else if ( mCanvas )
  {
    //check for active layer
    if ( mCanvas->currentLayer() )
    {
      //verify that the active layer is a vector layer
      if ( QgsMapLayer::VectorLayer == mCanvas->currentLayer()->type() )
      {
        mVectorLayer = qobject_cast< QgsVectorLayer * >( mCanvas->currentLayer() );
      }
      else
      {
        QMessageBox::warning( this, tr( "Generic Event Browser" ), tr( "This tool only supports vector data." ) );
        return false;
      }
    }
    else
    {
      QMessageBox::warning( this, tr( "Generic Event Browser" ), tr( "No active layers found." ) );
      return false;
    }
  }
  else
  {
    QMessageBox::warning( this, tr( "Generic Event Browser" ), tr( "Unable to connect to either the map canvas or application interface." ) );
    return false;
  }

  //Connect rendering routine for highlighting symbols and load symbols
  connect( mCanvas, &QgsMapCanvas::renderComplete, this, &eVisGenericEventBrowserGui::renderSymbol );

  mDataProvider = mVectorLayer->dataProvider();

  /*
   * A list of the selected feature ids is made so that we can move forward and backward through
   * the list. The data providers only have the ability to get one feature at a time or
   * sequentially move forward through the selected features
   */
  if ( 0 == mVectorLayer->selectedFeatureCount() ) //if nothing is selected select everything
  {
    mVectorLayer->invertSelection();
    mFeatureIds = mVectorLayer->selectedFeatureIds().toList();
  }
  else //use selected features
  {
    mFeatureIds = mVectorLayer->selectedFeatureIds().toList();
  }

  if ( 0 == mFeatureIds.size() )
    return false;

  //get the first feature in the list so we can set the field in the pull-down menus
  QgsFeature *myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );
  if ( !myFeature )
  {
    QMessageBox::warning( this, tr( "Generic Event Browser" ), tr( "An invalid feature was received during initialization." ) );
    return false;
  }

  QgsFields myFields = mVectorLayer->fields();
  mIgnoreEvent = true; //Ignore indexChanged event when adding items to combo boxes
  for ( int x = 0; x < myFields.count(); x++ )
  {
    QString name = myFields.at( x ).name();
    cboxEventImagePathField->addItem( name );
    cboxCompassBearingField->addItem( name );
    cboxCompassOffsetField->addItem( name );
    if ( myFeature->attribute( x ).toString().contains( QRegExp( "(jpg|jpeg|tif|tiff|gif)", Qt::CaseInsensitive ) ) )
    {
      mDefaultEventImagePathField = x;
    }

    if ( name.contains( QRegExp( "(comp|bear)", Qt::CaseInsensitive ) ) )
    {
      mDefaultCompassBearingField = x;
    }

    if ( name.contains( QRegExp( "(offset|declination)", Qt::CaseInsensitive ) ) )
    {
      mDefaultCompassOffsetField = x;
    }
  }
  mIgnoreEvent = false;

  //Set Display tab gui items
  if ( mFeatureIds.size() > 1 )
  {
    pbtnNext->setEnabled( true );
  }

  setWindowTitle( tr( "Event Browser - Displaying Records 01 of %1" ).arg( mFeatureIds.size(), 2, 10, QChar( '0' ) ) );

  //Set Options tab gui items
  initOptionsTab();

  //Load file associations into Configure External Applications tab gui items
  QSettings myQSettings;
  myQSettings.beginWriteArray( QStringLiteral( "/eVis/filetypeassociations" ) );
  int myTotalAssociations = myQSettings.childGroups().count();
  int myIterator = 0;
  while ( myIterator < myTotalAssociations )
  {
    myQSettings.setArrayIndex( myIterator );
    tableFileTypeAssociations->insertRow( tableFileTypeAssociations->rowCount() );
    tableFileTypeAssociations->setItem( myIterator, 0, new QTableWidgetItem( myQSettings.value( QStringLiteral( "extension" ), "" ).toString() ) );
    tableFileTypeAssociations->setItem( myIterator, 1, new QTableWidgetItem( myQSettings.value( QStringLiteral( "application" ), "" ).toString() ) );
    myIterator++;
  }
  myQSettings.endArray();

  mBrowserInitialized = true;

  return true;
}

/**
 * This method is an extension of the constructor. It was implemented so that it could be called by the GUI at anytime.
 */
void eVisGenericEventBrowserGui::initOptionsTab()
{
  //The base path has to be set first. If not if/when cboxEventImagePathRelative state change slot
  //will all ways over write the base path with the path to the data source
  //TODO: Find some better logic to prevent this from happening.
  leBasePath->setText( mConfiguration.basePath() );
  chkboxUseOnlyFilename->setChecked( mConfiguration.isUseOnlyFilenameSet() );

  //Set Options tab gui items
  int myIndex = cboxEventImagePathField->findText( mConfiguration.eventImagePathField(), Qt::MatchExactly );
  if ( -1 != myIndex )
  {
    cboxEventImagePathField->setCurrentIndex( myIndex );
  }
  else
  {
    cboxEventImagePathField->setCurrentIndex( mDefaultEventImagePathField );
  }

  chkboxEventImagePathRelative->setChecked( mConfiguration.isEventImagePathRelative() );

  myIndex = cboxCompassBearingField->findText( mConfiguration.compassBearingField(), Qt::MatchExactly );
  if ( -1 != myIndex )
  {
    cboxCompassBearingField->setCurrentIndex( myIndex );
  }
  else
  {
    cboxCompassBearingField->setCurrentIndex( mDefaultCompassBearingField );
  }

  chkboxDisplayCompassBearing->setChecked( mConfiguration.isDisplayCompassBearingSet() );

  if ( !mConfiguration.isDisplayCompassBearingSet() )
  {
    cboxCompassBearingField->setEnabled( false );
  }

  dsboxCompassOffset->setValue( mConfiguration.compassOffset() );
  myIndex = cboxCompassOffsetField->findText( mConfiguration.compassOffsetField(), Qt::MatchExactly );
  if ( -1 != myIndex )
  {
    cboxCompassOffsetField->setCurrentIndex( myIndex );
  }
  else
  {
    loadRecord();
    cboxCompassOffsetField->setCurrentIndex( mDefaultCompassOffsetField );
  }

  if ( mConfiguration.isManualCompassOffsetSet() )
  {
    rbtnManualCompassOffset->setChecked( true );
    rbtnAttributeCompassOffset->setChecked( false );
  }
  else if ( !mConfiguration.compassOffsetField().isEmpty() )
  {
    rbtnManualCompassOffset->setChecked( false );
    rbtnAttributeCompassOffset->setChecked( true );
  }
  else
  {
    rbtnManualCompassOffset->setChecked( false );
    rbtnAttributeCompassOffset->setChecked( false );
    dsboxCompassOffset->setEnabled( false );
    cboxCompassOffsetField->setEnabled( false );
  }

  chkboxApplyPathRulesToDocs->setChecked( mConfiguration.isApplyPathRulesToDocsSet() );

}

void eVisGenericEventBrowserGui::closeEvent( QCloseEvent *event )
{
  if ( mBrowserInitialized )
  {
    accept();
    event->accept();
  }
}

void eVisGenericEventBrowserGui::accept()
{
  QSettings myQSettings;

  if ( chkboxSaveEventImagePathData->isChecked() )
  {
    myQSettings.setValue( QStringLiteral( "eVis/eventimagepathfield" ), cboxEventImagePathField->currentText() );
    myQSettings.setValue( QStringLiteral( "eVis/eventimagepathrelative" ), chkboxEventImagePathRelative->isChecked() );
  }

  if ( chkboxSaveCompassBearingData->isChecked() )
  {
    myQSettings.setValue( QStringLiteral( "eVis/compassbearingfield" ), cboxCompassBearingField->currentText() );
    myQSettings.setValue( QStringLiteral( "eVis/displaycompassbearing" ), chkboxDisplayCompassBearing->isChecked() );
  }

  if ( chkboxSaveCompassOffsetData->isChecked() )
  {
    myQSettings.setValue( QStringLiteral( "eVis/manualcompassoffset" ), rbtnManualCompassOffset->isChecked() );
    myQSettings.setValue( QStringLiteral( "eVis/compassoffset" ), dsboxCompassOffset->value() );
    myQSettings.setValue( QStringLiteral( "eVis/attributecompassoffset" ), rbtnAttributeCompassOffset->isChecked() );
    myQSettings.setValue( QStringLiteral( "eVis/compassoffsetfield" ), cboxCompassOffsetField->currentText() );
  }

  if ( chkboxSaveBasePathData->isChecked() )
  {
    myQSettings.setValue( QStringLiteral( "eVis/basepath" ), leBasePath->text() );
  }

  if ( chkboxSaveUseOnlyFilenameData->isChecked() )
  {
    myQSettings.setValue( QStringLiteral( "eVis/useonlyfilename" ), chkboxUseOnlyFilename->isChecked() );
  }

  if ( chkboxSaveApplyPathRulesToDocs->isChecked() )
  {
    myQSettings.setValue( QStringLiteral( "eVis/applypathrulestodocs" ), chkboxApplyPathRulesToDocs->isChecked() );
  }

  myQSettings.remove( QStringLiteral( "/eVis/filetypeassociations" ) );
  myQSettings.beginWriteArray( QStringLiteral( "/eVis/filetypeassociations" ) );
  int myIterator = 0;
  int myIndex = 0;
  while ( myIterator < tableFileTypeAssociations->rowCount() )
  {
    myQSettings.setArrayIndex( myIndex );
    if ( tableFileTypeAssociations->item( myIterator, 0 ) && tableFileTypeAssociations->item( myIterator, 1 ) )
    {
      myQSettings.setValue( QStringLiteral( "extension" ), tableFileTypeAssociations->item( myIterator, 0 )->text() );
      myQSettings.setValue( QStringLiteral( "application" ), tableFileTypeAssociations->item( myIterator, 1 )->text() );
      myIndex++;
    }
    myIterator++;
  }
  myQSettings.endArray();
}

/**
 * Modifies the Event Image Path according to the local and global settings
 */
void eVisGenericEventBrowserGui::buildEventImagePath()
{
  //This if statement is a bit of a hack, have to track down where the 0 is coming from on initialization
  if ( "0" != mEventImagePath )
  {
    int myImageNameMarker = 0;

    if ( mEventImagePath.contains( '/' ) )
    {
      myImageNameMarker = mEventImagePath.lastIndexOf( '/' );
    }
    else
    {
      myImageNameMarker = mEventImagePath.lastIndexOf( '\\' );
    }

    QString myImageName = mEventImagePath;
    myImageName.remove( 0, myImageNameMarker + 1 );
    if ( mConfiguration.isUseOnlyFilenameSet() )
    {
      mEventImagePath = mConfiguration.basePath() + myImageName;
    }
    else
    {
      if ( mConfiguration.isEventImagePathRelative() )
      {
        mEventImagePath = mConfiguration.basePath() + mEventImagePath;
      }
    }
  }
}

/**
 * Chooses which image loading method to use and centers the map canvas on the current feature
 */
void eVisGenericEventBrowserGui::displayImage()
{
  //This if statement is a bit of a hack, have to track down where the 0 is coming from on initialization
  if ( "0" != mEventImagePath && 0 == displayArea->currentIndex() )
  {
    if ( mEventImagePath.startsWith( QLatin1String( "http://" ), Qt::CaseInsensitive ) )
    {
      imageDisplayArea->displayUrlImage( mEventImagePath );
    }
    else
    {
      imageDisplayArea->displayImage( mEventImagePath );
    }

    //clear any selection that may be present
    mVectorLayer->removeSelection();
    if ( !mFeatureIds.isEmpty() )
    {
      //select the current feature in the layer
      mVectorLayer->select( mFeatureIds.at( mCurrentFeatureIndex ) );
      //get a copy of the feature
      QgsFeature *myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );

      if ( !myFeature )
        return;

      QgsPointXY myPoint = myFeature->geometry().asPoint();
      myPoint = mCanvas->mapSettings().layerToMapCoordinates( mVectorLayer, myPoint );
      //keep the extent the same just center the map canvas in the display so our feature is in the middle
      QgsRectangle myRect( myPoint.x() - ( mCanvas->extent().width() / 2 ), myPoint.y() - ( mCanvas->extent().height() / 2 ), myPoint.x() + ( mCanvas->extent().width() / 2 ), myPoint.y() + ( mCanvas->extent().height() / 2 ) );

      // only change the extents if the point is beyond the current extents to minimize repaints
      if ( !mCanvas->extent().contains( myPoint ) )
      {
        mCanvas->setExtent( myRect );
      }
      mCanvas->refresh();
    }
  }
}

/**
 * Returns a pointer to the requested feature with a given featureid
 * \param id - FeatureId of the feature to find/select
 */
QgsFeature *eVisGenericEventBrowserGui::featureAtId( QgsFeatureId id )
{
  //This method was originally necessary because delimited text data provider did not support featureAtId()
  //It has mostly been stripped down now
  if ( mVectorLayer && mFeatureIds.size() != 0 )
  {
    if ( !mVectorLayer->getFeatures( QgsFeatureRequest().setFilterFid( id ) ).nextFeature( mFeature ) )
    {
      return nullptr;
    }
  }

  return &mFeature;
}

/**
 * Display the attrbiutes for the current feature and load the image
 */
void eVisGenericEventBrowserGui::loadRecord()
{
  treeEventData->clear();

  //Get a pointer to the current feature
  QgsFeature *myFeature = nullptr;
  myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );

  if ( !myFeature )
    return;

  QString myCompassBearingField = cboxCompassBearingField->currentText();
  QString myCompassOffsetField = cboxCompassOffsetField->currentText();
  QString myEventImagePathField = cboxEventImagePathField->currentText();
  QgsFields myFields = mVectorLayer->fields();
  QgsAttributes myAttrs = myFeature->attributes();
  //loop through the attributes and display their contents
  for ( int i = 0; i < myAttrs.count(); ++i )
  {
    QStringList myValues;
    QString fieldName = myFields.at( i ).name();
    myValues << fieldName << myAttrs.at( i ).toString();
    QTreeWidgetItem *myItem = new QTreeWidgetItem( myValues );
    if ( fieldName == myEventImagePathField )
    {
      mEventImagePath = myAttrs.at( i ).toString();
    }

    if ( fieldName == myCompassBearingField )
    {
      mCompassBearing = myAttrs.at( i ).toDouble();
    }

    if ( mConfiguration.isAttributeCompassOffsetSet() )
    {
      if ( fieldName == myCompassOffsetField )
      {
        mCompassOffset = myAttrs.at( i ).toDouble();
      }
    }
    else
    {
      mCompassOffset = 0.0;
    }

    //Check to see if the attribute is a know file type
    int myIterator = 0;
    while ( myIterator < tableFileTypeAssociations->rowCount() )
    {
      if ( tableFileTypeAssociations->item( myIterator, 0 ) && ( myAttrs.at( i ).toString().startsWith( tableFileTypeAssociations->item( myIterator, 0 )->text() + ':', Qt::CaseInsensitive ) || myAttrs.at( i ).toString().endsWith( tableFileTypeAssociations->item( myIterator, 0 )->text(), Qt::CaseInsensitive ) ) )
      {
        myItem->setBackground( 1, QBrush( QColor( 183, 216, 125, 255 ) ) );
        break;
      }
      else
        myIterator++;
    }
    treeEventData->addTopLevelItem( myItem );
  }
  //Modify EventImagePath as needed
  buildEventImagePath();

  //Request the image to be displayed in the browser
  displayImage();
}

/**
 * Restore the default configuration options
 */
void eVisGenericEventBrowserGui::restoreDefaultOptions()
{
  chkboxEventImagePathRelative->setChecked( false );
  cboxEventImagePathField->setCurrentIndex( mDefaultEventImagePathField );

  cboxCompassBearingField->setEnabled( true );
  cboxCompassBearingField->setCurrentIndex( mDefaultCompassBearingField );
  cboxCompassBearingField->setEnabled( false );
  chkboxDisplayCompassBearing->setChecked( false );

  cboxCompassOffsetField->setEnabled( true );
  cboxCompassOffsetField->setCurrentIndex( mDefaultCompassOffsetField );
  cboxCompassOffsetField->setEnabled( false );
  rbtnManualCompassOffset->setChecked( true );
  dsboxCompassOffset->setValue( 0.0 );

  leBasePath->clear();
  chkboxUseOnlyFilename->setChecked( false );

  chkboxSaveEventImagePathData->setChecked( false );
  chkboxSaveCompassBearingData->setChecked( false );
  chkboxSaveCompassOffsetData->setChecked( false );
  chkboxSaveBasePathData->setChecked( false );
  chkboxSaveUseOnlyFilenameData->setChecked( false );
  chkboxApplyPathRulesToDocs->setChecked( false );
}

/**
 * Sets the base path to the path of the data source
 */
void eVisGenericEventBrowserGui::setBasePathToDataSource()
{
  int myPathMarker = 0;

  QString mySourceUri = mDataProvider->dataSourceUri();
  //Check to see which way the directory symbol goes, I think this is actually unnecessary in qt
  if ( mySourceUri.contains( '/' ) )
  {
    myPathMarker = mySourceUri.lastIndexOf( '/' );
  }
  else
  {
    myPathMarker = mySourceUri.lastIndexOf( '\\' );
  }

  //Strip off the actual filename so we just have path
  mySourceUri.truncate( myPathMarker + 1 );

  //check for duplicate directory symbols when concatinating the two strings
#ifdef Q_OS_WIN
  mySourceUri.replace( "\\\\", "\\" );
#else
  if ( mySourceUri.startsWith( QLatin1String( "http://" ), Qt::CaseInsensitive ) )
  {
    mySourceUri.replace( QLatin1String( "//" ), QLatin1String( "/" ) );
    mySourceUri.replace( QLatin1String( "http:/" ), QLatin1String( "http://" ), Qt::CaseInsensitive );
  }
  else
  {
    mySourceUri.replace( QLatin1String( "//" ), QLatin1String( "/" ) );
  }
#endif

  leBasePath->setText( mySourceUri );
}

/*
 *
 * Public and Private Slots
 *
 */

/**
 * Slot called when a column is clicked in the tree displaying the attribute data
 * \param item - The tree widget item click
 * \param column - The column that was clicked
 */
void eVisGenericEventBrowserGui::launchExternalApplication( QTreeWidgetItem *item, int column )
{
  // At this point there is only attribute data with no children, ignore clicks on field name
  if ( 1 == column )
  {
    int myIterator = 0;
    bool startsWithExtension = false;
    while ( myIterator < tableFileTypeAssociations->rowCount() )
    {
      if ( item->text( column ).startsWith( tableFileTypeAssociations->item( myIterator, 0 )->text() + ':', Qt::CaseInsensitive ) )
      {
        startsWithExtension = true;
        break;
      }
      else if ( item->text( column ).endsWith( tableFileTypeAssociations->item( myIterator, 0 )->text(), Qt::CaseInsensitive ) )
      {
        startsWithExtension = false;
        break;
      }
      else
        myIterator++;
    }

    if ( myIterator != tableFileTypeAssociations->rowCount() )
    {
      QProcess *myProcess = new QProcess();
      QString myApplication = tableFileTypeAssociations->item( myIterator, 1 )->text();
      QString myDocument = item->text( column );
      if ( startsWithExtension )
      {
        myDocument = item->text( column ).remove( tableFileTypeAssociations->item( myIterator, 0 )->text() + ':', Qt::CaseInsensitive );
      }

      if ( "" != myApplication )
      {
        if ( mConfiguration.isApplyPathRulesToDocsSet() )
        {
          int myDocumentNameMarker = 0;

          if ( myDocument.contains( '/' ) )
          {
            myDocumentNameMarker = myDocument.lastIndexOf( '/' );
          }
          else
          {
            myDocumentNameMarker = myDocument.lastIndexOf( '\\' );
          }

          QString myDocumentName = myDocument;
          myDocumentName.remove( 0, myDocumentNameMarker + 1 );
          if ( mConfiguration.isUseOnlyFilenameSet() )
          {
            myDocument = mConfiguration.basePath() + myDocumentName;
          }
          else
          {
            if ( mConfiguration.isEventImagePathRelative() )
            {
              myDocument = mConfiguration.basePath() + myDocument;
            }
          }
        }

        myProcess->start( myApplication, QStringList() << myDocument );
      }
    }
    else
    {
      QMessageBox::information( this, tr( "Attribute Contents" ), item->text( column ) );
    }
  }
}

/**
 * Slot called when the restore or save button is click on the options panel
 * \param state - The new state of the checkbox
 */
void eVisGenericEventBrowserGui::buttonboxOptions_clicked( QAbstractButton *button )
{
  if ( QDialogButtonBox::ResetRole == buttonboxOptions->buttonRole( button ) )
  {
    restoreDefaultOptions();
  }
  else if ( QDialogButtonBox::AcceptRole == buttonboxOptions->buttonRole( button ) )
  {
    accept();
  }
}

/**
 * Slot called when the state changes for the chkboxApplyPathRulesToDocs checkbox.
 * \param state - The new state of the checkbox
 */
void eVisGenericEventBrowserGui::chkboxApplyPathRulesToDocs_stateChanged( int state )
{
  Q_UNUSED( state );
  mConfiguration.setApplyPathRulesToDocs( chkboxApplyPathRulesToDocs->isChecked() );
}

/**
 * Slot called when the index changes for the cboxEventImagePathField combo box.
 * \param index - The index of the new selected item
 */
void eVisGenericEventBrowserGui::cboxEventImagePathField_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( !mIgnoreEvent )
  {
    mConfiguration.setEventImagePathField( cboxEventImagePathField->currentText() );

    QgsFields myFields = mVectorLayer->fields();
    QgsFeature *myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );

    if ( !myFeature )
      return;

    QgsAttributes myAttrs = myFeature->attributes();
    for ( int i = 0; i < myAttrs.count(); ++i )
    {
      if ( myFields.at( i ).name() == cboxEventImagePathField->currentText() )
      {
        mEventImagePath = myAttrs.at( i ).toString();
      }
    }
  }
}

/**
 * Slot called when the index changes for the cboxCompassBearingField combo box.
 * \param index - The index of the new selected item
 */
void eVisGenericEventBrowserGui::cboxCompassBearingField_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( !mIgnoreEvent )
  {
    mConfiguration.setCompassBearingField( cboxCompassBearingField->currentText() );

    QgsFields myFields = mVectorLayer->fields();
    QgsFeature *myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );

    if ( !myFeature )
      return;

    QgsAttributes myAttrs = myFeature->attributes();
    for ( int i = 0; i < myAttrs.count(); ++i )
    {
      if ( myFields.at( i ).name() == cboxCompassBearingField->currentText() )
      {
        mCompassBearing = myAttrs.at( i ).toDouble();
      }
    }
  }
}

/**
 * Slot called when the index changes for the cboxCompassBearingField combo box.
 * \param index - The index of the new selected item
 */
void eVisGenericEventBrowserGui::cboxCompassOffsetField_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( !mIgnoreEvent )
  {
    mConfiguration.setCompassOffsetField( cboxCompassOffsetField->currentText() );

    QgsFields myFields = mVectorLayer->fields();
    QgsFeature *myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );

    if ( !myFeature )
      return;

    QgsAttributes myAttrs = myFeature->attributes();
    for ( int i = 0; i < myAttrs.count(); ++i )
    {
      if ( myFields.at( i ).name() == cboxCompassOffsetField->currentText() )
      {
        mCompassOffset = myAttrs.at( i ).toDouble();
      }
    }
  }
}

/**
 * Slot called when the chkDisplayCompassBearing radio button is toggled
 * \param state - The current selection state of the radio button
 */
void eVisGenericEventBrowserGui::chkboxDisplayCompassBearing_stateChanged( int state )
{
  Q_UNUSED( state );
  mConfiguration.setDisplayCompassBearing( chkboxDisplayCompassBearing->isChecked() );
  cboxCompassBearingField->setEnabled( chkboxDisplayCompassBearing->isChecked() );
}

/**
 * Slot called when the state changes for the chkboxEventImagePathRelative checkbox.
 * \param state - The new state of the checkbox
 */
void eVisGenericEventBrowserGui::chkboxEventImagePathRelative_stateChanged( int state )
{
  Q_UNUSED( state );
  mConfiguration.setEventImagePathRelative( chkboxEventImagePathRelative->isChecked() );

  if ( chkboxEventImagePathRelative->isChecked() && "" == leBasePath->text() )
  {
    setBasePathToDataSource();
  }

}

/**
 * Slot called when the state changes for the chkboxUseOnlyFilename checkbox.
 * \param state - The new state of the checkbox
 */
void eVisGenericEventBrowserGui::chkboxUseOnlyFilename_stateChanged( int state )
{
  Q_UNUSED( state );
  mConfiguration.setUseOnlyFilename( chkboxUseOnlyFilename->isChecked() );
}

/**
 * Slot called when the tabs in the tabWidget are selected
 * \param currentTabIndex - The index of the currently selected tab
 */
void eVisGenericEventBrowserGui::displayArea_currentChanged( int currentTabIndex )
{
  //Force redraw when we switching back to the Display tab
  if ( 0 == currentTabIndex )
  {
    loadRecord();
  }
}

/**
 * Slot called when a manual compass offset is entered
 * \param value - The new compass offset
 */
void eVisGenericEventBrowserGui::dsboxCompassOffset_valueChanged( double value )
{
  mConfiguration.setCompassOffset( value );
}

/**
 * Slot called the text in leBasePath is set or changed
 * \param text - The new base path
 */
void eVisGenericEventBrowserGui::leBasePath_textChanged( const QString &text )
{
  mConfiguration.setBasePath( text );
}

/**
 * Slot called when the pbtnAddFileType button is clicked - adds a new row to the file associations table
 */
void eVisGenericEventBrowserGui::pbtnAddFileType_clicked()
{
  tableFileTypeAssociations->insertRow( tableFileTypeAssociations->rowCount() );
}

/**
 * Slot called when the pbtnDeleteFileType button is clicked - removes arow from the file associations table
 */
void eVisGenericEventBrowserGui::pbtnDeleteFileType_clicked()
{
  if ( 1 <= tableFileTypeAssociations->rowCount() )
  {
    tableFileTypeAssociations->removeRow( tableFileTypeAssociations->currentRow() );
  }

}

/**
 * Slot called when the pbtnNext button is pressed
 */
void eVisGenericEventBrowserGui::pbtnNext_clicked()
{
  if ( mCurrentFeatureIndex != mFeatureIds.size() - 1 )
  {
    pbtnPrevious->setEnabled( true );
    mCurrentFeatureIndex++;

    setWindowTitle( tr( "Event Browser - Displaying Records %1 of %2" )
                    .arg( mCurrentFeatureIndex + 1, 2, 10, QChar( '0' ) ).arg( mFeatureIds.size(), 2, 10, QChar( '0' ) ) );

    loadRecord();
  }

  if ( mCurrentFeatureIndex == mFeatureIds.size() - 1 )
  {
    pbtnNext->setEnabled( false );
  }
}

/**
 * Slot called when the pbtnPrevious button is pressed
 */
void eVisGenericEventBrowserGui::pbtnPrevious_clicked()
{
  if ( mCurrentFeatureIndex > 0 )
  {
    pbtnNext->setEnabled( true );
    mCurrentFeatureIndex--;

    setWindowTitle( tr( "Event Browser - Displaying Records %1 of %2" )
                    .arg( mCurrentFeatureIndex + 1, 2, 10, QChar( '0' ) ).arg( mFeatureIds.size(), 2, 10, QChar( '0' ) ) );

    loadRecord();
  }

  if ( mCurrentFeatureIndex == 0 )
  {
    pbtnPrevious->setEnabled( false );
  }

}

void eVisGenericEventBrowserGui::pbtnResetApplyPathRulesToDocs_clicked()
{
  chkboxApplyPathRulesToDocs->setChecked( false );
}

void eVisGenericEventBrowserGui::pbtnResetBasePathData_clicked()
{
  leBasePath->clear();
  if ( chkboxEventImagePathRelative->isChecked() )
  {
    setBasePathToDataSource();
  }
}

void eVisGenericEventBrowserGui::pbtnResetCompassBearingData_clicked()
{
  cboxCompassBearingField->setEnabled( true );
  cboxCompassBearingField->setCurrentIndex( mDefaultCompassBearingField );
  cboxCompassBearingField->setEnabled( false );
  chkboxDisplayCompassBearing->setChecked( false );
}

void eVisGenericEventBrowserGui::pbtnResetCompassOffsetData_clicked()
{
  cboxCompassOffsetField->setEnabled( true );
  cboxCompassOffsetField->setCurrentIndex( mDefaultCompassOffsetField );
  cboxCompassOffsetField->setEnabled( false );
  rbtnManualCompassOffset->setChecked( true );
  dsboxCompassOffset->setValue( 0.0 );
}

void eVisGenericEventBrowserGui::pbtnResetEventImagePathData_clicked()
{
  chkboxEventImagePathRelative->setChecked( false );
  cboxEventImagePathField->setCurrentIndex( mDefaultEventImagePathField );
}

void eVisGenericEventBrowserGui::pbtnResetUseOnlyFilenameData_clicked()
{
  chkboxUseOnlyFilename->setChecked( false );
}

void eVisGenericEventBrowserGui::rbtnManualCompassOffset_toggled( bool state )
{
  mConfiguration.setManualCompassOffset( state );
  mConfiguration.setAttributeCompassOffset( !state );

  dsboxCompassOffset->setEnabled( state );
  cboxCompassOffsetField->setEnabled( !state );
}

/**
 * Slot called when an entry in the file associations table is clicked
 * \param row - the row that was clicked
 * \param column - the column that was clicked
 */
void eVisGenericEventBrowserGui::tableFileTypeAssociations_cellDoubleClicked( int row, int column )
{
  if ( 1 == column )
  {
    QString myApplication = QFileDialog::getOpenFileName( this, tr( "Select Application" ), QDir::homePath(), tr( "All ( * )" ) );
    if ( "" != myApplication )
    {
      tableFileTypeAssociations->setItem( row, column, new QTableWidgetItem( myApplication ) );
    }

  }
}

/**
 * This slot is coonnected to the map canvas. When the canvas is done drawing the slot is fired to display thee highlighting symbol
 * \param painter - Pointer to the QPainter object
 */
void eVisGenericEventBrowserGui::renderSymbol( QPainter *painter )
{

  if ( !mFeatureIds.isEmpty() && mVectorLayer )
  {
    //Get a pointer to the current feature
    QgsFeature *myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );

    if ( !myFeature )
      return;

    QgsPointXY myPoint = myFeature->geometry().asPoint();
    myPoint = mCanvas->mapSettings().layerToMapCoordinates( mVectorLayer, myPoint );

    mCanvas->getCoordinateTransform()->transform( &myPoint );

    if ( mConfiguration.isDisplayCompassBearingSet() )
    {
      //Make a copy of the pointersymbol and rotate it based on the values in the attribute field
      QPixmap myTempPixmap( mPointerSymbol.height(), mPointerSymbol.height() );
      myTempPixmap.fill( QColor( 255, 255, 255, 0 ) );
      QPainter p( &myTempPixmap );
      QMatrix wm;
      wm.translate( myTempPixmap.width() / 2, myTempPixmap.height() / 2 ); // really center

      double myBearing = mCompassBearing;
      if ( mConfiguration.isManualCompassOffsetSet() )
      {
        myBearing = mCompassBearing + mConfiguration.compassOffset();
      }
      else
      {
        myBearing = mCompassBearing + mCompassOffset;
      }

      if ( myBearing < 0.0 )
      {
        while ( myBearing < 0.0 )
          myBearing = 360.0 + myBearing;
      }
      else if ( myBearing >= 360.0 )
      {
        while ( myBearing >= 360.0 )
          myBearing = myBearing - 360.0;
      }

      wm.rotate( myBearing );

      p.setWorldMatrix( wm );
      p.drawPixmap( -mPointerSymbol.width() / 2, -mPointerSymbol.height() / 2, mPointerSymbol );

      int xShift = static_cast<int>( myPoint.x() ) - ( myTempPixmap.width() / 2 );
      int yShift = static_cast<int>( myPoint.y() ) - ( myTempPixmap.height() / 2 );
      painter->drawPixmap( xShift, yShift, myTempPixmap );
    }
    else
    {
      int xShift = static_cast<int>( myPoint.x() ) - ( mHighlightSymbol.width() / 2 );
      int yShift = static_cast<int>( myPoint.y() ) - ( mHighlightSymbol.height() / 2 );
      painter->drawPixmap( xShift, yShift, mHighlightSymbol );
    }
  }
}
