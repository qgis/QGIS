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
** This work was made possible through a grant by the the John D. and
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
#include "qgsmaprenderer.h"
#include "qgsmaptopixel.h"
#include "qgsmapcanvas.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgspoint.h"
#include "qgsfield.h"
#include "qgsrectangle.h"

#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QGraphicsScene>
#include <QSettings>
#include <QPainter>
#include <QProcess>
#include <QFileDialog>

/**
* Constructor called when browser is launched from the application plugin tool bar
* @param parent - Pointer the to parent QWidget for modality
* @param interface - Pointer the the application interface
* @param fl - Window flags
*/
eVisGenericEventBrowserGui::eVisGenericEventBrowserGui( QWidget* parent, QgisInterface* interface, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  restoreState();

  mCurrentFeatureIndex = 0;
  mInterface = interface;
  mDataProvider = 0;
  mVectorLayer = 0;
  mCanvas = 0;

  mIgnoreEvent = false;

  if ( initBrowser( ) )
  {
    loadRecord( );
    show( );
  }
  else
  {
    close( );
  }
}

/**
* Constructor called when browser is launched by the eVisEventIdTool
* @param parent - Pointer to the parent QWidget for modality
* @param canvas - Pointer to the map canvas
* @param fl - Window flags
*/
eVisGenericEventBrowserGui::eVisGenericEventBrowserGui( QWidget* parent, QgsMapCanvas* canvas, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  mCurrentFeatureIndex = 0;
  mInterface = 0;
  mDataProvider = 0;
  mVectorLayer = 0;
  mCanvas = canvas;

  mIgnoreEvent = false;

  if ( initBrowser( ) )
  {
    loadRecord( );
    show( );
  }
  else
  {
    close( );
  }
}


/**
 * Basic descructor
 */
eVisGenericEventBrowserGui::~eVisGenericEventBrowserGui( )
{
  //Clean up, disconnect the highlighting routine and refesh the canvase to clear highlighting symbol
  if ( 0 != mCanvas )
  {
    disconnect( mCanvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( renderSymbol( QPainter * ) ) );
    mCanvas->refresh( );
  }

  //On close, clear selected feature
  if ( 0 != mVectorLayer )
  {
    mVectorLayer->removeSelection( false );
  }
}

/**
 * This method is an extension of the constructor. It was implemented to reduce the amount of code duplicated between the constuctors.
 */
bool eVisGenericEventBrowserGui::initBrowser( )
{

  //setup gui
  setWindowTitle( tr( "Generic Event Browser" ) );

  connect( treeEventData, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ), this, SLOT( launchExternalApplication( QTreeWidgetItem *, int ) ) );

  mHighlightSymbol.load( ":/evis/eVisHighlightSymbol.png" );
  mPointerSymbol.load( ":/evis/eVisPointerSymbol.png" );
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

  QString myThemePath = QgsApplication::activeThemePath( );
  pbtnResetEventImagePathData->setIcon( QIcon( QPixmap( myThemePath + "/mActionDraw.png" ) ) );
  pbtnResetCompassBearingData->setIcon( QIcon( QPixmap( myThemePath + "/mActionDraw.png" ) ) );
  pbtnResetCompassOffsetData->setIcon( QIcon( QPixmap( myThemePath + "/mActionDraw.png" ) ) );
  pbtnResetBasePathData->setIcon( QIcon( QPixmap( myThemePath + "/mActionDraw.png" ) ) );
  pbtnResetUseOnlyFilenameData->setIcon( QIcon( QPixmap( myThemePath + "/mActionDraw.png" ) ) );
  pbtnResetApplyPathRulesToDocs->setIcon( QIcon( QPixmap( myThemePath + "/mActionDraw.png" ) ) );

  chkboxSaveEventImagePathData->setChecked( false );
  chkboxSaveCompassBearingData->setChecked( false );
  chkboxSaveCompassOffsetData->setChecked( false );
  chkboxSaveBasePathData->setChecked( false );
  chkboxSaveUseOnlyFilenameData->setChecked( false );

  //Set up Configure External Application buttons
  pbtnAddFileType->setIcon( QIcon( QPixmap( myThemePath + "/mActionNewAttribute.png" ) ) );
  pbtnDeleteFileType->setIcon( QIcon( QPixmap( myThemePath + "/mActionDeleteAttribute.png" ) ) );

  //Check to for interface, not null when launched from plugin toolbar, otherwise expect map canvas
  if ( 0 != mInterface )
  {
    //check for active layer
    if ( mInterface->activeLayer( ) )
    {
      //verify that the active layer is a vector layer
      if ( QgsMapLayer::VectorLayer == mInterface->activeLayer( )->type( ) )
      {
        mVectorLayer = ( QgsVectorLayer* )mInterface->activeLayer( );
        mCanvas = mInterface->mapCanvas( );
      }
      else
      {
        QMessageBox::warning( this, tr( "Warning" ), tr( "This tool only supports vector data" ) );
        return false;
      }
    }
    else
    {
      QMessageBox::warning( this, tr( "Warning" ), tr( "No active layers found" ) );
      return false;
    }
  }
  //check for map canvas, if map canvas is null, throw error
  else if ( 0 != mCanvas )
  {
    //check for active layer
    if ( mCanvas->currentLayer( ) )
    {
      //verify that the active layer is a vector layer
      if ( QgsMapLayer::VectorLayer == mCanvas->currentLayer( )->type( ) )
      {
        mVectorLayer = ( QgsVectorLayer* )mCanvas->currentLayer( );
      }
      else
      {
        QMessageBox::warning( this, tr( "Warning" ), tr( "This tool only supports vector data" ) );
        return false;
      }
    }
    else
    {
      QMessageBox::warning( this, tr( "Warning" ), tr( "No active layers found" ) );
      return false;
    }
  }
  else
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "Unable to connect to either the map canvas or application interface" ) );
    return false;
  }

  //Connect rendering routine for highlighting symbols and load symbols
  connect( mCanvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( renderSymbol( QPainter * ) ) );

  mDataProvider = mVectorLayer->dataProvider( );

  /*
   * A list of the selected feature ids is made so that we can move forward and backward through
   * the list. The data providers only have the ability to get one feature at a time or
   * sequentially move forward through the selected features
   */
  if ( 0 == mVectorLayer->selectedFeatureCount( ) ) //if nothing is selected select everything
  {
    mVectorLayer->invertSelection();
    mFeatureIds = mVectorLayer->selectedFeaturesIds().toList();
  }
  else //use selected features
  {
    mFeatureIds = mVectorLayer->selectedFeaturesIds().toList();
  }

  if ( 0 == mFeatureIds.size() )
    return false;

  //get the first feature in the list so we can set the field in the pulldown menues
  QgsFeature* myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );
  if ( !myFeature )
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "An invalid feature was received during initialization" ) );
    return false;
  }

  QgsFieldMap myFieldMap = mDataProvider->fields( );
  QgsAttributeMap myAttributeMap = myFeature->attributeMap( );
  mIgnoreEvent = true; //Ignore indexChanged event when adding items to combo boxes
  for ( int x = 0; x < myFieldMap.size( ); x++ )
  {
    cboxEventImagePathField->addItem( myFieldMap[x].name( ) );
    cboxCompassBearingField->addItem( myFieldMap[x].name( ) );
    cboxCompassOffsetField->addItem( myFieldMap[x].name( ) );
    if ( myAttributeMap[x].toString( ).contains( QRegExp( "(jpg|jpeg|tif|tiff|gif)", Qt::CaseInsensitive ) ) )
    {
      mDefaultEventImagePathField = x;
    }

    if ( myFieldMap[x].name( ).contains( QRegExp( "(comp|bear)", Qt::CaseInsensitive ) ) )
    {
      mDefaultCompassBearingField = x;
    }

    if ( myFieldMap[x].name( ).contains( QRegExp( "(offset|declination)", Qt::CaseInsensitive ) ) )
    {
      mDefaultCompassOffsetField = x;
    }
  }
  mIgnoreEvent = false;

  //Set Display tab gui items
  if ( mFeatureIds.size( ) > 1 )
  {
    pbtnNext->setEnabled( true );
  }

  setWindowTitle( tr( "Event Browser - Displaying records 01 of %1" ).arg( mFeatureIds.size(), 2, 10, QChar( '0' ) ) );

  //Set Options tab gui items
  initOptionsTab( );

  //Load file associations into Configure External Applications tab gui items
  QSettings myQSettings;
  myQSettings.beginWriteArray( "/eVis/filetypeassociations" );
  int myTotalAssociations = myQSettings.childGroups( ).count( );
  int myIterator = 0;
  while ( myIterator < myTotalAssociations )
  {
    myQSettings.setArrayIndex( myIterator );
    tableFileTypeAssociations->insertRow( tableFileTypeAssociations->rowCount( ) );
    tableFileTypeAssociations->setItem( myIterator, 0, new QTableWidgetItem( myQSettings.value( "extension", "" ).toString( ) ) );
    tableFileTypeAssociations->setItem( myIterator, 1, new QTableWidgetItem( myQSettings.value( "application", "" ).toString( ) ) );
    myIterator++;
  }
  myQSettings.endArray( );

  mBrowserInitialized = true;

  return true;
}

/**
 * This method is an extension of the constructor. It was implemented so that it could be called by the GUI at anytime.
 */
void eVisGenericEventBrowserGui::initOptionsTab( )
{
  //The base path has to be set first. If not if/when cboxEventImagePathRelative state change slot
  //will all ways over write the base path with the path to the data source
  //TODO: Find some better logic to prevent this from happening.
  leBasePath->setText( mConfiguration.basePath( ) );
  chkboxUseOnlyFilename->setChecked( mConfiguration.isUseOnlyFilenameSet( ) );

  //Set Options tab gui items
  int myIndex = cboxEventImagePathField->findText( mConfiguration.eventImagePathField( ), Qt::MatchExactly );
  if ( -1 != myIndex )
  {
    cboxEventImagePathField->setCurrentIndex( myIndex );
  }
  else
  {
    cboxEventImagePathField->setCurrentIndex( mDefaultEventImagePathField );
  }

  chkboxEventImagePathRelative->setChecked( mConfiguration.isEventImagePathRelative( ) );

  myIndex = cboxCompassBearingField->findText( mConfiguration.compassBearingField( ), Qt::MatchExactly );
  if ( -1 != myIndex )
  {
    cboxCompassBearingField->setCurrentIndex( myIndex );
  }
  else
  {
    cboxCompassBearingField->setCurrentIndex( mDefaultCompassBearingField );
  }

  chkboxDisplayCompassBearing->setChecked( mConfiguration.isDisplayCompassBearingSet( ) );

  if ( !mConfiguration.isDisplayCompassBearingSet( ) )
  {
    cboxCompassBearingField->setEnabled( false );
  }

  dsboxCompassOffset->setValue( mConfiguration.compassOffset( ) );
  myIndex = cboxCompassOffsetField->findText( mConfiguration.compassOffsetField( ), Qt::MatchExactly );
  if ( -1 != myIndex )
  {
    cboxCompassOffsetField->setCurrentIndex( myIndex );
  }
  else
  {
    loadRecord( );
    cboxCompassOffsetField->setCurrentIndex( mDefaultCompassOffsetField );
  }

  if ( mConfiguration.isManualCompassOffsetSet( ) )
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

  chkboxApplyPathRulesToDocs->setChecked( mConfiguration.isApplyPathRulesToDocsSet( ) );

}

void eVisGenericEventBrowserGui::closeEvent( QCloseEvent *event )
{
  if ( mBrowserInitialized )
  {
    accept( );
    event->accept( );
  }
}

void eVisGenericEventBrowserGui::accept( )
{
  QSettings myQSettings;

  if ( chkboxSaveEventImagePathData->isChecked( ) )
  {
    myQSettings.setValue( "/eVis/eventimagepathfield", cboxEventImagePathField->currentText( ) );
    myQSettings.setValue( "/eVis/eventimagepathrelative", chkboxEventImagePathRelative->isChecked( ) );
  }

  if ( chkboxSaveCompassBearingData->isChecked( ) )
  {
    myQSettings.setValue( "/eVis/compassbearingfield", cboxCompassBearingField->currentText( ) );
    myQSettings.setValue( "/eVis/displaycompassbearing", chkboxDisplayCompassBearing->isChecked( ) );
  }

  if ( chkboxSaveCompassOffsetData->isChecked( ) )
  {
    myQSettings.setValue( "/eVis/manualcompassoffset", rbtnManualCompassOffset->isChecked( ) );
    myQSettings.setValue( "/eVis/compassoffset", dsboxCompassOffset->value( ) );
    myQSettings.setValue( "/eVis/attributecompassoffset", rbtnAttributeCompassOffset->isChecked( ) );
    myQSettings.setValue( "/eVis/compassoffsetfield", cboxCompassOffsetField->currentText( ) );
  }

  if ( chkboxSaveBasePathData->isChecked( ) )
  {
    myQSettings.setValue( "/eVis/basepath", leBasePath->text( ) );
  }

  if ( chkboxSaveUseOnlyFilenameData->isChecked( ) )
  {
    myQSettings.setValue( "/eVis/useonlyfilename", chkboxUseOnlyFilename->isChecked( ) );
  }

  if ( chkboxSaveApplyPathRulesToDocs->isChecked( ) )
  {
    myQSettings.setValue( "/eVis/applypathrulestodocs", chkboxApplyPathRulesToDocs->isChecked( ) );
  }

  saveState();

  myQSettings.remove( "/eVis/filetypeassociations" );
  myQSettings.beginWriteArray( "/eVis/filetypeassociations" );
  int myIterator = 0;
  int myIndex = 0;
  while ( myIterator < tableFileTypeAssociations->rowCount( ) )
  {
    myQSettings.setArrayIndex( myIndex );
    if ( 0 != tableFileTypeAssociations->item( myIterator, 0 ) && 0 != tableFileTypeAssociations->item( myIterator, 1 ) )
    {
      myQSettings.setValue( "extension", tableFileTypeAssociations->item( myIterator, 0 )->text( ) );
      myQSettings.setValue( "application", tableFileTypeAssociations->item( myIterator, 1 )->text( ) );
      myIndex++;
    }
    myIterator++;
  }
  myQSettings.endArray( );
}

/**
 * Modifies the Event Image Path according to the local and global settings
 */
void eVisGenericEventBrowserGui::buildEventImagePath( )
{
  //This if statement is a bit of a hack, have to track down where the 0 is comming from on initalization
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
    if ( mConfiguration.isUseOnlyFilenameSet( ) )
    {
      mEventImagePath = mConfiguration.basePath( ) + myImageName;
    }
    else
    {
      if ( mConfiguration.isEventImagePathRelative( ) )
      {
        mEventImagePath = mConfiguration.basePath( ) + mEventImagePath;
      }
    }
  }
}

/**
 * Chooses which image loading method to use and centers the map canvas on the current feature
 */
void eVisGenericEventBrowserGui::displayImage( )
{
  //This if statement is a bit of a hack, have to track down where the 0 is comming from on initalization
  if ( "0" != mEventImagePath && 0 == displayArea->currentIndex( ) )
  {
    if ( mEventImagePath.startsWith( "http://", Qt::CaseInsensitive ) )
    {
      imageDisplayArea->displayUrlImage( mEventImagePath );
    }
    else
    {
      imageDisplayArea->displayImage( mEventImagePath );
    }

    //clear any selection that may be present
    mVectorLayer->removeSelection( false );
    if ( mFeatureIds.size( ) > 0 )
    {
      //select the current feature in the layer
      mVectorLayer->select( mFeatureIds.at( mCurrentFeatureIndex ), true );
      //get a copy of the feature
      QgsFeature* myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );

      if ( 0 == myFeature )
        return;

      QgsPoint myPoint = myFeature->geometry( )->asPoint( );
      myPoint = mCanvas->mapRenderer( )->layerToMapCoordinates( mVectorLayer, myPoint );
      //keep the extent the same just center the map canvas in the display so our feature is in the middle
      QgsRectangle myRect( myPoint.x( ) - ( mCanvas->extent( ).width( ) / 2 ), myPoint.y( ) - ( mCanvas->extent( ).height( ) / 2 ), myPoint.x( ) + ( mCanvas->extent( ).width( ) / 2 ), myPoint.y( ) + ( mCanvas->extent( ).height( ) / 2 ) );

      // only change the extents if the point is beyond the current extents to minimise repaints
      if ( !mCanvas->extent().contains( myPoint ) )
      {
        mCanvas->setExtent( myRect );
      }
      mCanvas->refresh( );
    }
  }
}

/**
 * Returns a pointer to the reqested feature with a given featureid
 * @param id - FeatureId of the feature to find/select
 */
QgsFeature* eVisGenericEventBrowserGui::featureAtId( QgsFeatureId id )
{
  //This method was originally necessary because delimited text data provider did not support featureAtId( )
  //It has mostly been stripped down now
  if ( mDataProvider && mFeatureIds.size( ) != 0 )
  {
    if ( !mVectorLayer->featureAtId( id, mFeature, true, true ) )
    {
      return 0;
    }
  }

  return &mFeature;
}

/**
 * Display the attrbiutes for the current feature and load the image
 */
void eVisGenericEventBrowserGui::loadRecord( )
{
  treeEventData->clear();

  //Get a pointer to the current feature
  QgsFeature* myFeature;
  myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );

  if ( 0 == myFeature )
    return;

  QString myCompassBearingField = cboxCompassBearingField->currentText( );
  QString myCompassOffsetField = cboxCompassOffsetField->currentText( );
  QString myEventImagePathField = cboxEventImagePathField->currentText( );
  QgsFieldMap myFieldMap = mDataProvider->fields( );
  QgsAttributeMap myAttributeMap = myFeature->attributeMap( );
  //loop through the attributes and display their contents
  for ( QgsAttributeMap::const_iterator it = myAttributeMap.begin( ); it != myAttributeMap.end( ); ++it )
  {
    QStringList myValues;
    myValues << myFieldMap[it.key( )].name( ) << it->toString( );
    QTreeWidgetItem* myItem = new QTreeWidgetItem( myValues );
    if ( myFieldMap[it.key( )].name( ) == myEventImagePathField )
    {
      mEventImagePath = it->toString( );
    }

    if ( myFieldMap[it.key( )].name( ) == myCompassBearingField )
    {
      mCompassBearing = it->toDouble( );
    }

    if ( mConfiguration.isAttributeCompassOffsetSet( ) )
    {
      if ( myFieldMap[it.key( )].name( ) == myCompassOffsetField )
      {
        mCompassOffset = it->toDouble( );
      }
    }
    else
    {
      mCompassOffset = 0.0;
    }

    //Check to see if the attribute is a know file type
    int myIterator = 0;
    while ( myIterator < tableFileTypeAssociations->rowCount( ) )
    {
      if ( tableFileTypeAssociations->item( myIterator, 0 ) && ( it->toString( ).startsWith( tableFileTypeAssociations->item( myIterator, 0 )->text( ) + ":", Qt::CaseInsensitive ) || it->toString( ).endsWith( tableFileTypeAssociations->item( myIterator, 0 )->text( ), Qt::CaseInsensitive ) ) )
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
  buildEventImagePath( );

  //Request the image to be displayed in the browser
  displayImage( );
}

/**
 * Restore the default configuration options
 */
void eVisGenericEventBrowserGui::restoreDefaultOptions( )
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

  leBasePath->setText( "" );
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
void eVisGenericEventBrowserGui::setBasePathToDataSource( )
{
  //Noticed some strangeness here while cleaning up for migration to the QGIS trunk - PJE 2009-07-01
  //TODO: The check for windows paths not longer does anything, remove or fix

  int myPathMarker = 0;
  bool isWindows = false;
  QString mySourceUri = mDataProvider->dataSourceUri( );
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
  if ( isWindows )
  {
    mySourceUri.replace( "\\\\", "\\" );
  }
  else
  {
    if ( mySourceUri.startsWith( "http://", Qt::CaseInsensitive ) )
    {
      mySourceUri.replace( "//", "/" );
      mySourceUri.replace( "http:/", "http://", Qt::CaseInsensitive );
    }
    else
    {
      mySourceUri.replace( "//", "/" );
    }
  }

  leBasePath->setText( mySourceUri );
}

/*
 *
 * Public and Private Slots
 *
 */

/**
 * Slot called when a column is clicked in the tree displaying the attribute data
 * @param theItem - The tree widget item click
 * @param theColumn - The column that was clicked
 */
void eVisGenericEventBrowserGui::launchExternalApplication( QTreeWidgetItem * theItem, int theColumn )
{
  // At this point there is only attribute data with no children, ignore clicks on field name
  if ( 1 == theColumn )
  {
    int myIterator = 0;
    bool startsWithExtension = false;
    while ( myIterator < tableFileTypeAssociations->rowCount( ) )
    {
      if ( theItem->text( theColumn ).startsWith( tableFileTypeAssociations->item( myIterator, 0 )->text( ) + ":", Qt::CaseInsensitive ) )
      {
        startsWithExtension = true;
        break;
      }
      else if ( theItem->text( theColumn ).endsWith( tableFileTypeAssociations->item( myIterator, 0 )->text( ), Qt::CaseInsensitive ) )
      {
        startsWithExtension = false;
        break;
      }
      else
        myIterator++;
    }

    if ( myIterator != tableFileTypeAssociations->rowCount( ) )
    {
      QProcess *myProcess = new QProcess( );
      QString myApplication = tableFileTypeAssociations->item( myIterator, 1 )->text( );
      QString myDocument = theItem->text( theColumn );
      if ( startsWithExtension )
      {
        myDocument = theItem->text( theColumn ).remove( tableFileTypeAssociations->item( myIterator, 0 )->text( ) + ":", Qt::CaseInsensitive );
      }

      if ( "" != myApplication )
      {
        if ( mConfiguration.isApplyPathRulesToDocsSet( ) )
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
          if ( mConfiguration.isUseOnlyFilenameSet( ) )
          {
            myDocument = mConfiguration.basePath( ) + myDocumentName;
          }
          else
          {
            if ( mConfiguration.isEventImagePathRelative( ) )
            {
              myDocument = mConfiguration.basePath( ) + myDocument;
            }
          }
        }

        myProcess->start( myApplication, QStringList( ) << myDocument );
      }
    }
    else
    {
      QMessageBox::information( this, tr( "Attribute Contents" ), theItem->text( theColumn ) );
    }
  }
}

/**
 * Slot called when the restore or save button is click on the options panel
 * @param state - The new state of the checkbox
 */
void eVisGenericEventBrowserGui::on_buttonboxOptions_clicked( QAbstractButton* theButton )
{
  if ( QDialogButtonBox::ResetRole == buttonboxOptions->buttonRole( theButton ) )
  {
    restoreDefaultOptions( );
  }
  else if ( QDialogButtonBox::AcceptRole == buttonboxOptions->buttonRole( theButton ) )
  {
    accept( );
  }
}

/**
 * Slot called when the state changes for the chkboxApplyPathRulesToDocs check box.
 * @param theState - The new state of the checkbox
 */
void eVisGenericEventBrowserGui::on_chkboxApplyPathRulesToDocs_stateChanged( int theState )
{
  Q_UNUSED( theState );
  mConfiguration.setApplyPathRulesToDocs( chkboxApplyPathRulesToDocs->isChecked( ) );
}

/**
 * Slot called when the index changes for the cboxEventImagePathField combo box.
 * @param theIndex - The index of the new selected item
 */
void eVisGenericEventBrowserGui::on_cboxEventImagePathField_currentIndexChanged( int theIndex )
{
  Q_UNUSED( theIndex );
  if ( !mIgnoreEvent )
  {
    mConfiguration.setEventImagePathField( cboxEventImagePathField->currentText( ) );

    QgsFieldMap myFieldMap = mDataProvider->fields( );
    QgsFeature* myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );

    if ( 0 == myFeature )
      return;

    QgsAttributeMap myAttributeMap = myFeature->attributeMap( );
    for ( QgsAttributeMap::const_iterator it = myAttributeMap.begin( ); it != myAttributeMap.end( ); ++it )
    {
      if ( myFieldMap[it.key( )].name( ) == cboxEventImagePathField->currentText( ) )
      {
        mEventImagePath = it->toString( );
      }
    }
  }
}

/**
 * Slot called when the index changes for the cboxCompassBearingField combo box.
 * @param theIndex - The index of the new selected item
 */
void eVisGenericEventBrowserGui::on_cboxCompassBearingField_currentIndexChanged( int theIndex )
{
  Q_UNUSED( theIndex );
  if ( !mIgnoreEvent )
  {
    mConfiguration.setCompassBearingField( cboxCompassBearingField->currentText( ) );

    QgsFieldMap myFieldMap = mDataProvider->fields( );
    QgsFeature* myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );

    if ( 0 == myFeature )
      return;

    QgsAttributeMap myAttributeMap = myFeature->attributeMap( );
    for ( QgsAttributeMap::const_iterator it = myAttributeMap.begin( ); it != myAttributeMap.end( ); ++it )
    {
      if ( myFieldMap[it.key( )].name( ) == cboxCompassBearingField->currentText( ) )
      {
        mCompassBearing = it->toDouble( );
      }
    }
  }
}

/**
 * Slot called when the index changes for the cboxCompassBearingField combo box.
 * @param theIndex - The index of the new selected item
 */
void eVisGenericEventBrowserGui::on_cboxCompassOffsetField_currentIndexChanged( int theIndex )
{
  Q_UNUSED( theIndex );
  if ( !mIgnoreEvent )
  {
    mConfiguration.setCompassOffsetField( cboxCompassOffsetField->currentText( ) );

    QgsFieldMap myFieldMap = mDataProvider->fields( );
    QgsFeature* myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );

    if ( 0 == myFeature )
      return;

    QgsAttributeMap myAttributeMap = myFeature->attributeMap( );
    for ( QgsAttributeMap::const_iterator it = myAttributeMap.begin( ); it != myAttributeMap.end( ); ++it )
    {
      if ( myFieldMap[it.key( )].name( ) == cboxCompassOffsetField->currentText( ) )
      {
        mCompassOffset = it->toDouble( );
      }
    }
  }
}

/**
 * Slot called when the chkDisplayCompassBearing radio button is toggled
 * @param theState - The current selection state of the radio button
 */
void eVisGenericEventBrowserGui::on_chkboxDisplayCompassBearing_stateChanged( int theState )
{
  Q_UNUSED( theState );
  mConfiguration.setDisplayCompassBearing( chkboxDisplayCompassBearing->isChecked( ) );
  cboxCompassBearingField->setEnabled( chkboxDisplayCompassBearing->isChecked( ) );
}

/**
 * Slot called when the state changes for the chkboxEventImagePathRelative check box.
 * @param theState - The new state of the checkbox
 */
void eVisGenericEventBrowserGui::on_chkboxEventImagePathRelative_stateChanged( int theState )
{
  Q_UNUSED( theState );
  mConfiguration.setEventImagePathRelative( chkboxEventImagePathRelative->isChecked( ) );

  if ( chkboxEventImagePathRelative->isChecked( ) && "" == leBasePath->text( ) )
  {
    setBasePathToDataSource( );
  }

}

/**
 * Slot called when the state changes for the chkboxUseOnlyFilename check box.
 * @param theState - The new state of the checkbox
 */
void eVisGenericEventBrowserGui::on_chkboxUseOnlyFilename_stateChanged( int theState )
{
  Q_UNUSED( theState );
  mConfiguration.setUseOnlyFilename( chkboxUseOnlyFilename->isChecked( ) );
}

/**
 * Slot called when the tabs in the tabWidget are selected
 * @param theCurrentTabIndex - The index of the currently selected tab
 */
void eVisGenericEventBrowserGui::on_displayArea_currentChanged( int theCurrentTabIndex )
{
  //Force redraw when we switching back to the Display tab
  if ( 0 == theCurrentTabIndex )
  {
    loadRecord( );
  }
}

/**
 * Slot called when a manual compass offset is entered
 * @param theValue - The new compass offset
 */
void eVisGenericEventBrowserGui::on_dsboxCompassOffset_valueChanged( double theValue )
{
  mConfiguration.setCompassOffset( theValue );
}

/**
 * Slot called the text in leBasePath is set or changed
 * @param theText - The new base path
 */
void eVisGenericEventBrowserGui::on_leBasePath_textChanged( QString theText )
{
  mConfiguration.setBasePath( theText );
}

/**
 * Slot called when the pbtnAddFileType button is clicked - adds a new row to the file associations table
 */
void eVisGenericEventBrowserGui::on_pbtnAddFileType_clicked( )
{
  tableFileTypeAssociations->insertRow( tableFileTypeAssociations->rowCount( ) );
}

/**
 * Slot called when the pbtnDeleteFileType button is clicked - removes arow from the file associations table
 */
void eVisGenericEventBrowserGui::on_pbtnDeleteFileType_clicked( )
{
  if ( 1 <= tableFileTypeAssociations->rowCount( ) )
  {
    tableFileTypeAssociations->removeRow( tableFileTypeAssociations->currentRow( ) );
  }

}

/**
 * Slot called when the pbtnNext button is pressed
 */
void eVisGenericEventBrowserGui::on_pbtnNext_clicked( )
{
  if ( mCurrentFeatureIndex != mFeatureIds.size( ) - 1 )
  {
    pbtnPrevious->setEnabled( true );
    mCurrentFeatureIndex++;

    setWindowTitle( tr( "Event Browser - Displaying records %1 of %2" )
                    .arg( mCurrentFeatureIndex + 1, 2, 10, QChar( '0' ) ).arg( mFeatureIds.size(), 2, 10, QChar( '0' ) ) );

    loadRecord( );
  }

  if ( mCurrentFeatureIndex == mFeatureIds.size( ) - 1 )
  {
    pbtnNext->setEnabled( false );
  }
}

/**
 * Slot called when the pbtnPrevious button is pressed
 */
void eVisGenericEventBrowserGui::on_pbtnPrevious_clicked( )
{
  if ( mCurrentFeatureIndex > 0 )
  {
    pbtnNext->setEnabled( true );
    mCurrentFeatureIndex--;

    setWindowTitle( tr( "Event Browser - Displaying records %1 of %2" )
                    .arg( mCurrentFeatureIndex + 1, 2, 10, QChar( '0' ) ).arg( mFeatureIds.size(), 2, 10, QChar( '0' ) ) );

    loadRecord( );
  }

  if ( mCurrentFeatureIndex == 0 )
  {
    pbtnPrevious->setEnabled( false );
  }

}

void eVisGenericEventBrowserGui::on_pbtnResetApplyPathRulesToDocs_clicked( )
{
  chkboxApplyPathRulesToDocs->setChecked( false );
}

void eVisGenericEventBrowserGui::on_pbtnResetBasePathData_clicked( )
{
  leBasePath->setText( "" );
  if ( chkboxEventImagePathRelative->isChecked( ) )
  {
    setBasePathToDataSource( );
  }
}

void eVisGenericEventBrowserGui::on_pbtnResetCompassBearingData_clicked( )
{
  cboxCompassBearingField->setEnabled( true );
  cboxCompassBearingField->setCurrentIndex( mDefaultCompassBearingField );
  cboxCompassBearingField->setEnabled( false );
  chkboxDisplayCompassBearing->setChecked( false );
}

void eVisGenericEventBrowserGui::on_pbtnResetCompassOffsetData_clicked( )
{
  cboxCompassOffsetField->setEnabled( true );
  cboxCompassOffsetField->setCurrentIndex( mDefaultCompassOffsetField );
  cboxCompassOffsetField->setEnabled( false );
  rbtnManualCompassOffset->setChecked( true );
  dsboxCompassOffset->setValue( 0.0 );
}

void eVisGenericEventBrowserGui::on_pbtnResetEventImagePathData_clicked( )
{
  chkboxEventImagePathRelative->setChecked( false );
  cboxEventImagePathField->setCurrentIndex( mDefaultEventImagePathField );
}

void eVisGenericEventBrowserGui::on_pbtnResetUseOnlyFilenameData_clicked( )
{
  chkboxUseOnlyFilename->setChecked( false );
}

void eVisGenericEventBrowserGui::on_rbtnManualCompassOffset_toggled( bool theState )
{
  mConfiguration.setManualCompassOffset( theState );
  mConfiguration.setAttributeCompassOffset( !theState );

  dsboxCompassOffset->setEnabled( theState );
  cboxCompassOffsetField->setEnabled( !theState );
}

/**
 * Slot called when an entry in the file associations table is clicked
 * @param theRow - the row that was clicked
 * @param theColumn - the column that was clicked
 */
void eVisGenericEventBrowserGui::on_tableFileTypeAssociations_cellDoubleClicked( int theRow, int theColumn )
{
  if ( 1 == theColumn )
  {
    QString myApplication = QFileDialog::getOpenFileName( this, tr( "Select Application" ), "", tr( "All ( * )" ) );
    if ( "" != myApplication )
    {
      tableFileTypeAssociations->setItem( theRow, theColumn, new QTableWidgetItem( myApplication ) );
    }

  }
}

/**
 * This slot is coonnected to the map canvas. When the canvas is done drawing the slot is fired to display thee highlighting symbol
 * @param thePainter - Pointer to the QPainter object
 */
void eVisGenericEventBrowserGui::renderSymbol( QPainter* thePainter )
{

  if ( mFeatureIds.size( ) > 0 && mVectorLayer != 0 )
  {
    //Get a pointer to the current feature
    QgsFeature* myFeature = featureAtId( mFeatureIds.at( mCurrentFeatureIndex ) );

    if ( 0 == myFeature )
      return;

    QgsPoint myPoint = myFeature->geometry( )->asPoint( );
    myPoint = mCanvas->mapRenderer( )->layerToMapCoordinates( mVectorLayer, myPoint );

    mCanvas->getCoordinateTransform( )->transform( &myPoint );

    if ( mConfiguration.isDisplayCompassBearingSet( ) )
    {
      //Make a copy of the pointersymbol and rotate it based on the values in the attribute field
      QPixmap myTempPixmap( mPointerSymbol.height( ), mPointerSymbol.height( ) );
      myTempPixmap.fill( QColor( 255, 255, 255, 0 ) );
      QPainter p( &myTempPixmap );
      QMatrix wm;
      wm.translate( myTempPixmap.width( ) / 2, myTempPixmap.height( ) / 2 ); // really center

      double myBearing = mCompassBearing;
      if ( mConfiguration.isManualCompassOffsetSet( ) )
      {
        myBearing = mCompassBearing + mConfiguration.compassOffset( );
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
      p.drawPixmap( -mPointerSymbol.width( ) / 2, -mPointerSymbol.height( ) / 2, mPointerSymbol );

      int xShift = ( int )myPoint.x( ) - ( myTempPixmap.width( ) / 2 );
      int yShift = ( int )myPoint.y( ) - ( myTempPixmap.height( ) / 2 );
      thePainter->drawPixmap( xShift, yShift, myTempPixmap );
    }
    else
    {
      int xShift = ( int )myPoint.x( ) - ( mHighlightSymbol.width( ) / 2 );
      int yShift = ( int )myPoint.y( ) - ( mHighlightSymbol.height( ) / 2 );
      thePainter->drawPixmap( xShift, yShift, mHighlightSymbol );
    }
  }
}

void eVisGenericEventBrowserGui::saveState()
{
  QSettings settings;
  settings.setValue( "/eVis/browser-geometry", saveGeometry() );
}

void eVisGenericEventBrowserGui::restoreState()
{
  QSettings settings;
  restoreGeometry( settings.value( "/eVis/browser-geometry" ).toByteArray() );
}

