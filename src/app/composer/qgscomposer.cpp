/***************************************************************************
                         qgscomposer.cpp  -  description
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscomposer.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgscomposerview.h"
#include "qgscomposition.h"
#include "qgscompositionwidget.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlabelwidget.h"
#include "qgscomposerlegend.h"
#include "qgscomposerlegendwidget.h"
#include "qgscomposermap.h"
#include "qgscomposermapwidget.h"
#include "qgscomposerpicture.h"
#include "qgscomposerpicturewidget.h"
#include "qgscomposerscalebar.h"
#include "qgscomposerscalebarwidget.h"
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsmessageviewer.h"
#include "qgscontexthelp.h"
#include "qgscursors.h"

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QMatrix>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>

#include <QPrinter>
#include <QPrintDialog>
#include <QSettings>
#include <QIcon>
#include <QPixmap>
#if QT_VERSION < 0x040300
#include <Q3Picture>
#else
#include <QSvgGenerator>
#endif
#include <QToolBar>
#include <QToolButton>
#include <QImageWriter>
#include <QCheckBox>
#include <QSizeGrip>
#include "qgslogger.h"

QgsComposer::QgsComposer( QgisApp *qgis ): QMainWindow()
{
  setupUi( this );
  setupTheme();

  QToolButton* orderingToolButton = new QToolButton( this );
  orderingToolButton->setPopupMode( QToolButton::InstantPopup );
  orderingToolButton->setAutoRaise( true );
  orderingToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );

  orderingToolButton->addAction( mActionRaiseItems );
  orderingToolButton->addAction( mActionLowerItems );
  orderingToolButton->addAction( mActionMoveItemsToTop );
  orderingToolButton->addAction( mActionMoveItemsToBottom );
  orderingToolButton->setDefaultAction( mActionRaiseItems );
  toolBar->addWidget( orderingToolButton );

  QToolButton* alignToolButton = new QToolButton( this );
  alignToolButton->setPopupMode( QToolButton::InstantPopup );
  alignToolButton->setAutoRaise( true );
  alignToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );

  alignToolButton->addAction( mActionAlignLeft );
  alignToolButton->addAction( mActionAlignHCenter );
  alignToolButton->addAction( mActionAlignRight );
  alignToolButton->addAction( mActionAlignTop );
  alignToolButton->addAction( mActionAlignVCenter );
  alignToolButton->addAction( mActionAlignBottom );
  alignToolButton->setDefaultAction( mActionAlignLeft );
  toolBar->addWidget( alignToolButton );

  QActionGroup* toggleActionGroup = new QActionGroup( this );
  toggleActionGroup->addAction( mActionMoveItemContent );
  toggleActionGroup->addAction( mActionAddNewMap );
  toggleActionGroup->addAction( mActionAddNewLabel );
  toggleActionGroup->addAction( mActionAddNewLegend );
  toggleActionGroup->addAction( mActionAddNewScalebar );
  toggleActionGroup->addAction( mActionAddImage );
  toggleActionGroup->addAction( mActionSelectMoveItem );
  toggleActionGroup->setExclusive( true );

  setWindowTitle( tr( "QGIS - print composer" ) );

  mActionAddNewMap->setCheckable( true );
  mActionAddNewLabel->setCheckable( true );
  mActionAddNewLegend->setCheckable( true );
  mActionSelectMoveItem->setCheckable( true );
  mActionAddNewScalebar->setCheckable( true );
  mActionAddImage->setCheckable( true );
  mActionMoveItemContent->setCheckable( true );

#ifdef Q_WS_MAC
  QMenu *appMenu = menuBar()->addMenu( tr( "QGIS" ) );
  appMenu->addAction( QgisApp::instance()->actionAbout() );
  appMenu->addAction( QgisApp::instance()->actionOptions() );

  QMenu *fileMenu = menuBar()->addMenu( tr( "File" ) );
  QAction *closeAction = fileMenu->addAction( tr( "Close" ), this, SLOT( close() ), tr( "Ctrl+W" ) );
  fileMenu->addAction( mActionExportAsImage );
  fileMenu->addAction( mActionExportAsSVG );
  fileMenu->addSeparator();
  fileMenu->addAction( mActionPrint );

  QMenu *editMenu = menuBar()->addMenu( tr( "Edit" ) );
  QAction *undoAction = editMenu->addAction( tr( "&Undo" ), this, SLOT( undo() ), tr( "Ctrl+Z" ) );
  undoAction->setEnabled( false );
  editMenu->addSeparator();
  QAction *cutAction = editMenu->addAction( tr( "Cu&t" ), this, SLOT( cut() ), tr( "Ctrl+X" ) );
  cutAction->setEnabled( false );
  QAction *copyAction = editMenu->addAction( tr( "&Copy" ), this, SLOT( copy() ), tr( "Ctrl+C" ) );
  copyAction->setEnabled( false );
  QAction *pasteAction = editMenu->addAction( tr( "&Paste" ), this, SLOT( paste() ), tr( "Ctrl+V" ) );
  pasteAction->setEnabled( false );
  QAction *deleteAction = editMenu->addAction( tr( "Delete" ) );
  deleteAction->setEnabled( false );

  QMenu *viewMenu = menuBar()->addMenu( tr( "View" ) );
  viewMenu->addAction( mActionZoomIn );
  viewMenu->addAction( mActionZoomOut );
  viewMenu->addAction( mActionZoomAll );
  viewMenu->addSeparator();
  viewMenu->addAction( mActionRefreshView );

  QMenu *layoutMenu = menuBar()->addMenu( tr( "Layout" ) );
  layoutMenu->addAction( mActionAddNewMap );
  layoutMenu->addAction( mActionAddNewLabel );
  layoutMenu->addAction( mActionAddNewScalebar );
  layoutMenu->addAction( mActionAddNewLegend );
  layoutMenu->addAction( mActionAddImage );
  layoutMenu->addAction( mActionSelectMoveItem );
  layoutMenu->addAction( mActionMoveItemContent );
  layoutMenu->addSeparator();
  layoutMenu->addAction( mActionGroupItems );
  layoutMenu->addAction( mActionUngroupItems );
  layoutMenu->addAction( mActionRaiseItems );
  layoutMenu->addAction( mActionLowerItems );
  layoutMenu->addAction( mActionMoveItemsToTop );
  layoutMenu->addAction( mActionMoveItemsToBottom );

#ifndef Q_WS_MAC64 /* assertion failure in NSMenuItem setSubmenu (Qt 4.5.0-snapshot-20080830) */
  menuBar()->addMenu( QgisApp::instance()->windowMenu() );

  menuBar()->addMenu( QgisApp::instance()->helpMenu() );
#endif

  // Create action to select this window and add it to Window menu
  mWindowAction = new QAction( windowTitle(), this );
  connect( mWindowAction, SIGNAL( triggered() ), this, SLOT( activate() ) );
#endif

  mQgis = qgis;
  mFirstTime = true;

  QgsDebugMsg( "entered." );

  setMouseTracking( true );
  //mSplitter->setMouseTracking(true);
  mViewFrame->setMouseTracking( true );

  //create composer view
  mView = new QgsComposerView( mViewFrame );
  connectSlots();

  mComposition  = new QgsComposition( mQgis->mapCanvas()->mapRenderer() );
  mView->setComposition( mComposition );

  QgsCompositionWidget* compositionWidget = new QgsCompositionWidget( mCompositionOptionsFrame, mComposition );
  compositionWidget->show();

  mCompositionOptionsLayout = new QGridLayout( mCompositionOptionsFrame );
  mCompositionOptionsLayout->setMargin( 0 );
  mCompositionOptionsLayout->addWidget( compositionWidget );

  mPrinter = 0;

  QGridLayout *l = new QGridLayout( mViewFrame );
  l->setMargin( 0 );
  l->addWidget( mView, 0, 0 );

  mCompositionNameComboBox->insertItem( 0, tr( "Map 1" ) );

  //mComposition  = new QgsComposition( this, 1 );
  //mComposition->setActive ( true );

  // Create size grip (needed by Mac OS X for QMainWindow if QStatusBar is not visible)
  mSizeGrip = new QSizeGrip( this );
  mSizeGrip->resize( mSizeGrip->sizeHint() );
  mSizeGrip->move( rect().bottomRight() - mSizeGrip->rect().bottomRight() );

  restoreWindowState();
  setSelectionTool();

  mView->setFocus();

  //connect with signals from QgsProject to read/write project files
  if ( QgsProject::instance() )
  {
    connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument& ) ), this, SLOT( readXML( const QDomDocument& ) ) );
    connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument& ) ), this, SLOT( writeXML( QDomDocument& ) ) );
  }
}

QgsComposer::~QgsComposer()
{
}

void QgsComposer::setupTheme()
{
  //now set all the icons - getThemeIcon will fall back to default theme if its
  //missing from active theme
  mActionLoadFromTemplate->setIcon( QgisApp::getThemeIcon( "/mActionFileOpen.png" ) );
  mActionSaveAsTemplate->setIcon( QgisApp::getThemeIcon( "/mActionFileSaveAs.png" ) );
  mActionExportAsImage->setIcon( QgisApp::getThemeIcon( "/mActionExportMapServer.png" ) );
  mActionExportAsSVG->setIcon( QgisApp::getThemeIcon( "/mActionSaveAsSVG.png" ) );
  mActionExportAsPDF->setIcon( QgisApp::getThemeIcon( "/mActionSaveAsPDF.png" ) );
  mActionPrint->setIcon( QgisApp::getThemeIcon( "/mActionFilePrint.png" ) );
  mActionZoomAll->setIcon( QgisApp::getThemeIcon( "/mActionZoomFullExtent.png" ) );
  mActionZoomIn->setIcon( QgisApp::getThemeIcon( "/mActionZoomIn.png" ) );
  mActionZoomOut->setIcon( QgisApp::getThemeIcon( "/mActionZoomOut.png" ) );
  mActionRefreshView->setIcon( QgisApp::getThemeIcon( "/mActionDraw.png" ) );
  mActionAddImage->setIcon( QgisApp::getThemeIcon( "/mActionSaveMapAsImage.png" ) );
  mActionAddNewMap->setIcon( QgisApp::getThemeIcon( "/mActionAddRasterLayer.png" ) );
  mActionAddNewLabel->setIcon( QgisApp::getThemeIcon( "/mActionLabel.png" ) );
  mActionAddNewLegend->setIcon( QgisApp::getThemeIcon( "/mActionAddLegend.png" ) );
  mActionAddNewScalebar->setIcon( QgisApp::getThemeIcon( "/mActionScaleBar.png" ) );
  mActionSelectMoveItem->setIcon( QgisApp::getThemeIcon( "/mActionSelectPan.png" ) );
  mActionMoveItemContent->setIcon( QgisApp::getThemeIcon( "/mActionMoveItemContent.png" ) );
  mActionGroupItems->setIcon( QgisApp::getThemeIcon( "/mActionGroupItems.png" ) );
  mActionUngroupItems->setIcon( QgisApp::getThemeIcon( "/mActionUngroupItems.png" ) );
  mActionRaiseItems->setIcon( QgisApp::getThemeIcon( "/mActionRaiseItems.png" ) );
  mActionLowerItems->setIcon( QgisApp::getThemeIcon( "/mActionLowerItems.png" ) );
  mActionMoveItemsToTop->setIcon( QgisApp::getThemeIcon( "/mActionMoveItemsToTop.png" ) );
  mActionMoveItemsToBottom->setIcon( QgisApp::getThemeIcon( "/mActionMoveItemsToBottom.png" ) );
  mActionAlignLeft->setIcon( QgisApp::getThemeIcon( "/mActionAlignLeft.png" ) );
  mActionAlignHCenter->setIcon( QgisApp::getThemeIcon( "/mActionAlignHCenter.png" ) );
  mActionAlignRight->setIcon( QgisApp::getThemeIcon( "/mActionAlignRight.png" ) );
  mActionAlignTop->setIcon( QgisApp::getThemeIcon( "/mActionAlignTop.png" ) );
  mActionAlignVCenter->setIcon( QgisApp::getThemeIcon( "/mActionAlignVCenter.png" ) );
  mActionAlignBottom->setIcon( QgisApp::getThemeIcon( "/mActionAlignBottom.png" ) );
}

void QgsComposer::connectSlots()
{
  connect( mView, SIGNAL( selectedItemChanged( const QgsComposerItem* ) ), this, SLOT( showItemOptions( const QgsComposerItem* ) ) );
  connect( mView, SIGNAL( composerLabelAdded( QgsComposerLabel* ) ), this, SLOT( addComposerLabel( QgsComposerLabel* ) ) );
  connect( mView, SIGNAL( composerMapAdded( QgsComposerMap* ) ), this, SLOT( addComposerMap( QgsComposerMap* ) ) );
  connect( mView, SIGNAL( itemRemoved( QgsComposerItem* ) ), this, SLOT( deleteItem( QgsComposerItem* ) ) );
  connect( mView, SIGNAL( composerScaleBarAdded( QgsComposerScaleBar* ) ), this, SLOT( addComposerScaleBar( QgsComposerScaleBar* ) ) );
  connect( mView, SIGNAL( composerLegendAdded( QgsComposerLegend* ) ), this, SLOT( addComposerLegend( QgsComposerLegend* ) ) );
  connect( mView, SIGNAL( composerPictureAdded( QgsComposerPicture* ) ), this, SLOT( addComposerPicture( QgsComposerPicture* ) ) );
  connect( mView, SIGNAL( actionFinished() ), this, SLOT( setSelectionTool() ) );
}

void QgsComposer::open( void )
{
  if ( mFirstTime )
  {
    //mComposition->createDefault();
    mFirstTime = false;
    show();
    zoomFull(); // zoomFull() does not work properly until we have called show()
  }

  else
  {
    show(); //make sure the window is displayed - with a saved project, it's possible to not have already called show()
    //is that a bug?
    activate(); //bring the composer window to the front
  }
}

void QgsComposer::activate()
{
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();
}

#ifdef Q_WS_MAC
void QgsComposer::changeEvent( QEvent* event )
{
  QMainWindow::changeEvent( event );
  switch ( event->type() )
  {
    case QEvent::ActivationChange:
      if ( QApplication::activeWindow() == this )
      {
        mWindowAction->setChecked( true );
      }
      break;

    default:
      break;
  }
}

void QgsComposer::closeEvent( QCloseEvent *event )
{
  QMainWindow::closeEvent( event );
  if ( event->isAccepted() )
  {
    QgisApp::instance()->removeWindow( mWindowAction );
  }
}

void QgsComposer::showEvent( QShowEvent *event )
{
  QMainWindow::showEvent( event );
  // add to menu if (re)opening window (event not due to unminimize)
  if ( !event->spontaneous() )
  {
    QgisApp::instance()->addWindow( mWindowAction );
  }
}
#endif

void QgsComposer::showCompositionOptions( QWidget *w )
{
  QWidget* currentWidget = mItemStackedWidget->currentWidget();
  mItemStackedWidget->removeWidget( currentWidget );
  mItemStackedWidget->addWidget( w );
}

void QgsComposer::showItemOptions( const QgsComposerItem* item )
{
  QWidget* currentWidget = mItemStackedWidget->currentWidget();

  if ( !item )
  {
    mItemStackedWidget->removeWidget( currentWidget );
    mItemStackedWidget->setCurrentWidget( 0 );
    return;
  }

  QMap<QgsComposerItem*, QWidget*>::iterator it = mItemWidgetMap.find( const_cast<QgsComposerItem*>( item ) );
  if ( it == mItemWidgetMap.constEnd() )
  {
    return;
  }

  QWidget* newWidget = it.value();

  if ( !newWidget || newWidget == currentWidget ) //bail out if new widget does not exist or is already there
  {
    return;
  }

  mItemStackedWidget->removeWidget( currentWidget );
  mItemStackedWidget->addWidget( newWidget );
  mItemStackedWidget->setCurrentWidget( newWidget );
  //newWidget->show();
}

QgsMapCanvas *QgsComposer::mapCanvas( void )
{
  return mQgis->mapCanvas();
}

QgsComposerView *QgsComposer::view( void )
{
  return mView;
}

/*QgsComposition *QgsComposer::composition(void)
{
  return mComposition;
  }*/

void QgsComposer::zoomFull( void )
{
  if ( mView )
  {
    mView->fitInView( 0, 0, mComposition->paperWidth() + 1, mComposition->paperHeight() + 1, Qt::KeepAspectRatio );
  }
}

void QgsComposer::on_mActionZoomAll_triggered()
{
  zoomFull();
  mView->update();
  emit zoomLevelChanged();
}

void QgsComposer::on_mActionZoomIn_triggered()
{
  mView->scale( 2, 2 );
  mView->update();
  emit zoomLevelChanged();
}

void QgsComposer::on_mActionZoomOut_triggered()
{
  mView->scale( .5, .5 );
  mView->update();
  emit zoomLevelChanged();
}

void QgsComposer::on_mActionRefreshView_triggered()
{
  if ( mComposition )
  {
    mComposition->update();
  }
}

void QgsComposer::on_mActionExportAsPDF_triggered()
{
  QSettings myQSettings;  // where we keep last used filter in persistant state
  QString myLastUsedFile = myQSettings.value( "/UI/lastSaveAsPdfFile", "qgis.pdf" ).toString();
  QFileInfo file( myLastUsedFile );
  QFileDialog *myQFileDialog = new QFileDialog( this, tr( "Choose a file name to save the map as" ),
      file.path(), tr( "PDF Format" ) + " (*.pdf *PDF)" );
  myQFileDialog->selectFile( file.fileName() );
  myQFileDialog->setFileMode( QFileDialog::AnyFile );
  myQFileDialog->setAcceptMode( QFileDialog::AcceptSave );

  int result = myQFileDialog->exec();
  raise();
  if ( result != QDialog::Accepted ) return;

  QString myOutputFileNameQString = myQFileDialog->selectedFiles().first();
  if ( myOutputFileNameQString == "" ) return;

  myQSettings.setValue( "/UI/lastSaveAsPdfFile", myOutputFileNameQString );

  QPrinter printer;

  printer.setOutputFormat( QPrinter::PdfFormat );
  printer.setOutputFileName( myOutputFileNameQString );

  print( printer );
}

void QgsComposer::on_mActionPrint_triggered()
{
  QPrinter printer;

  QPrintDialog printDialog( &printer );
  if ( printDialog.exec() != QDialog::Accepted )
    return;

  print( printer );
}

void QgsComposer::print( QPrinter &printer )
{
  if( !mComposition )
    return;

  if ( containsWMSLayer() )
  {
    showWMSPrintingWarning();
  }

  //try to set most of the print dialog settings based on composer properties
  if ( mComposition->paperHeight() > mComposition->paperWidth() )
  {
    printer.setOrientation( QPrinter::Portrait );
  }
  else
  {
    printer.setOrientation( QPrinter::Landscape );
  }

  //set resolution based on composer setting
  printer.setFullPage( true );
  printer.setColorMode( QPrinter::Color );

  //set user-defined resolution
  printer.setResolution( mComposition->printResolution() );

  QPainter p( &printer );

  QgsComposition::PlotStyle savedPlotStyle = mComposition->plotStyle();
  mComposition->setPlotStyle( QgsComposition::Print );

  QApplication::setOverrideCursor( Qt::BusyCursor );

  if ( mComposition->printAsRaster() )
  {
    //print out via QImage, code copied from on_mActionExportAsImage_activated
    int width = ( int )( mComposition->printResolution() * mComposition->paperWidth() / 25.4 );
    int height = ( int )( mComposition-> printResolution() * mComposition->paperHeight() / 25.4 );
    QImage image( QSize( width, height ), QImage::Format_ARGB32 );
    image.setDotsPerMeterX( mComposition->printResolution() / 25.4 * 1000 );
    image.setDotsPerMeterY( mComposition->printResolution() / 25.4 * 1000 );
    image.fill( 0 );
    QPainter imagePainter( &image );
    QRectF sourceArea( 0, 0, mComposition->paperWidth(), mComposition->paperHeight() );
    QRectF targetArea( 0, 0, width, height );
    mComposition->render( &imagePainter, targetArea, sourceArea );
    imagePainter.end();
    p.drawImage( targetArea, image, targetArea );
  }
  else
  {
#if QT_VERSION < 0x040400
    QRectF paperRect( 0, 0, mComposition->paperWidth(), mComposition->paperHeight() );
    QRect pageRect = printer.pageRect();
    mComposition->render( &p, pageRect, paperRect );
#else
    //better in case of custom page size, but only possible with Qt>=4.4.0
    QRectF paperRectMM = printer.pageRect( QPrinter::Millimeter );
    QRectF paperRectPixel = printer.pageRect( QPrinter::DevicePixel );
    mComposition->render( &p, paperRectPixel, paperRectMM );
#endif
  }
  mComposition->setPlotStyle( savedPlotStyle );
  QApplication::restoreOverrideCursor();
}

void QgsComposer::on_mActionExportAsImage_triggered()
{
  if ( containsWMSLayer() )
  {
    showWMSPrintingWarning();
  }

  // Image size
  int width = ( int )( mComposition->printResolution() * mComposition->paperWidth() / 25.4 );
  int height = ( int )( mComposition-> printResolution() * mComposition->paperHeight() / 25.4 );

  int memuse = width * height * 3 / 1000000;  // pixmap + image
  QgsDebugMsg( QString( "Image %1 x %2" ).arg( width ).arg( height ) );
  QgsDebugMsg( QString( "memuse = %1" ).arg( memuse ) );

  if ( memuse > 200 )   // cca 4500 x 4500
  {
    int answer = QMessageBox::warning( 0, tr( "Big image" ),
                                       tr( "To create image %1 x %2 requires circa %3 MB of memory" )
                                       .arg( width ).arg( height ).arg( memuse ),
                                       QMessageBox::Ok,  QMessageBox::Abort );

    raise();
    if ( answer == QMessageBox::Abort ) return;
  }

  // Get file and format (stolen from qgisapp.cpp but modified significantely)

  //create a map to hold the QImageIO names and the filter names
  //the QImageIO name must be passed to the mapcanvas saveas image function
  typedef QMap<QString, QString> FilterMap;
  FilterMap myFilterMap;

  //find out the last used filter
  QSettings myQSettings;  // where we keep last used filter in persistant state
  QString myLastUsedFormat = myQSettings.value( "/UI/lastSaveAsImageFormat", "png" ).toString();
  QString myLastUsedFile = myQSettings.value( "/UI/lastSaveAsImageFile", "qgis.png" ).toString();
  QFileInfo file( myLastUsedFile );

  // get a list of supported output image types
  int myCounterInt = 0;
  QString myFilters;
  QString myLastUsedFilter;
  for ( ; myCounterInt < QImageWriter::supportedImageFormats().count(); myCounterInt++ )
  {
    QString myFormat = QString( QImageWriter::supportedImageFormats().at( myCounterInt ) );
    QString myFilter = tr( "%1 format (*.%2 *.%3)" )
                       .arg( myFormat ).arg( myFormat.toLower() ).arg( myFormat.toUpper() );

    if ( myCounterInt > 0 ) myFilters += ";;";
    myFilters += myFilter;
    myFilterMap[myFilter] = myFormat;
    if ( myFormat == myLastUsedFormat )
    {
      myLastUsedFilter = myFilter;
    }
  }
#ifdef QGISDEBUG
  QgsDebugMsg( "Available Filters Map: " );
  FilterMap::Iterator myIterator;
  for ( myIterator = myFilterMap.begin(); myIterator != myFilterMap.end(); ++myIterator )
  {
    QgsDebugMsg( QString( "%1  :  %2" ).arg( myIterator.key() ).arg( myIterator.value() ) );
  }
#endif

  //create a file dialog using the the filter list generated above
  std::auto_ptr < QFileDialog > myQFileDialog(
    new QFileDialog(
      this,
      tr( "Choose a file name to save the map image as" ),
      file.path(),
      myFilters
    )
  );

  myQFileDialog->setFileMode( QFileDialog::AnyFile );

  // set the filter to the last one used
  myQFileDialog->selectFilter( myLastUsedFilter );

  // set the 'Open' button to something that makes more sense
  myQFileDialog->setAcceptMode( QFileDialog::AcceptSave );

  //prompt the user for a file name
  QString myOutputFileNameQString;

  int result = myQFileDialog->exec();
  //raise();

  if ( result != QDialog::Accepted )
  {
    return;
  }

  myOutputFileNameQString = myQFileDialog->selectedFiles().last();
  QgsDebugMsg( myOutputFileNameQString );
  QString myFilterString = myQFileDialog->selectedFilter();
  QgsDebugMsg( QString( "Selected filter: %1" ).arg( myFilterString ) );
  QgsDebugMsg( QString( "Image type: %1" ).arg( myFilterMap[myFilterString] ) );

  myQSettings.setValue( "/UI/lastSaveAsImageFormat", myFilterMap[myFilterString] );
  myQSettings.setValue( "/UI/lastSaveAsImageFile", myOutputFileNameQString );

  if ( myOutputFileNameQString == "" ) return;

  mComposition->setPlotStyle( QgsComposition::Print );
  mView->setScene( 0 );

  QImage image( QSize( width, height ), QImage::Format_ARGB32 );
  image.setDotsPerMeterX( mComposition->printResolution() / 25.4 * 1000 );
  image.setDotsPerMeterY( mComposition->printResolution() / 25.4 * 1000 );
  image.fill( 0 );
  QPainter p( &image );
  QRectF sourceArea( 0, 0, mComposition->paperWidth(), mComposition->paperHeight() );
  QRectF targetArea( 0, 0, width, height );
  mComposition->render( &p, targetArea, sourceArea );
  p.end();

  mComposition->setPlotStyle( QgsComposition::Preview );
  image.save( myOutputFileNameQString, myFilterMap[myFilterString].toLocal8Bit().data() );
  mView->setScene( mComposition );
}


void QgsComposer::on_mActionExportAsSVG_triggered()
{
  if ( containsWMSLayer() )
  {
    showWMSPrintingWarning();
  }

  QString myQSettingsLabel = "/UI/displaySVGWarning";
  QSettings myQSettings;

  bool displaySVGWarning = myQSettings.value( myQSettingsLabel, true ).toBool();

  if ( displaySVGWarning )
  {
    QgsMessageViewer* m = new QgsMessageViewer( this );
    m->setWindowTitle( tr( "SVG warning" ) );
    m->setCheckBoxText( tr( "Don't show this message again" ) );
    m->setCheckBoxState( Qt::Unchecked );
    m->setCheckBoxVisible( true );
    m->setCheckBoxQSettingsLabel( myQSettingsLabel );
    m->setMessageAsHtml( tr( "<p>The SVG export function in Qgis has several "
                             "problems due to bugs and deficiencies in the " )
#if QT_VERSION < 0x040300
                         + tr( "Qt4 svg code. Of note, text does not "
                               "appear in the SVG file and there are problems "
                               "with the map bounding box clipping other items "
                               "such as the legend or scale bar.</p>" )
#else
                         + tr( "Qt4 svg code. In particular, there are problems "
                               "with layers not being clipped to the map "
                               "bounding box.</p>" )
#endif
                         + tr( "If you require a vector-based output file from "
                               "Qgis it is suggested that you try printing "
                               "to PostScript if the SVG output is not "
                               "satisfactory."
                               "</p>" ) );
    m->exec();
  }
  QString myLastUsedFile = myQSettings.value( "/UI/lastSaveAsSvgFile", "qgis.svg" ).toString();
  QFileInfo file( myLastUsedFile );
  QFileDialog *myQFileDialog = new QFileDialog( this, tr( "Choose a file name to save the map as" ),
      file.path(), tr( "SVG Format" ) + " (*.svg *SVG)" );
  myQFileDialog->selectFile( file.fileName() );
  myQFileDialog->setFileMode( QFileDialog::AnyFile );
  myQFileDialog->setAcceptMode( QFileDialog::AcceptSave );

  int result = myQFileDialog->exec();
  raise();
  if ( result != QDialog::Accepted ) return;

  QString myOutputFileNameQString = myQFileDialog->selectedFiles().first();
  if ( myOutputFileNameQString == "" ) return;

  myQSettings.setValue( "/UI/lastSaveAsSvgFile", myOutputFileNameQString );

  //mView->setScene(0);//don't redraw the scene on the display while we render
  mComposition->setPlotStyle( QgsComposition::Print );

#if QT_VERSION < 0x040300
  Q3Picture pic;
  QPainter p( &pic );
  QRectF renderArea( 0, 0, ( mComposition->paperWidth() * mComposition->scale() ), ( mComposition->paperHeight() * mComposition->scale() ) );
#else
  QSvgGenerator generator;
  generator.setFileName( myOutputFileNameQString );
  generator.setSize( QSize(( int )mComposition->paperWidth(), ( int )mComposition->paperHeight() ) );
  generator.setResolution( 25.4 ); //because the rendering is done in mm, convert the dpi

  QPainter p( &generator );
  QRectF renderArea( 0, 0, mComposition->paperWidth(), mComposition->paperHeight() );
#endif
  mComposition->render( &p, renderArea, renderArea );
  p.end();

  mComposition->setPlotStyle( QgsComposition::Preview );
  //mView->setScene(mComposition->canvas()); //now that we're done, set the view to show the scene again

#if QT_VERSION < 0x040300
  QRect br = pic.boundingRect();

  pic.save( myOutputFileNameQString, "svg" );
#endif
}

void QgsComposer::on_mActionSelectMoveItem_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::Select );
  }
}

void QgsComposer::on_mActionAddNewMap_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddMap );
  }
}

void QgsComposer::on_mActionAddNewLegend_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddLegend );
  }
}

void QgsComposer::on_mActionAddNewLabel_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddLabel );
  }
}

void QgsComposer::on_mActionAddNewScalebar_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddScalebar );
  }
}

void QgsComposer::on_mActionAddImage_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddPicture );
  }
}

void QgsComposer::on_mActionSaveAsTemplate_triggered()
{
  //show file dialog
  QSettings settings;
  QString lastSaveDir = settings.value( "UI/lastComposerTemplateDir", "" ).toString();
  QString saveFileName = QFileDialog::getSaveFileName( 0, tr( "save template" ), lastSaveDir, "*.qpt" );
  if ( saveFileName.isEmpty() )
  {
    return;
  }

  QFileInfo saveFileInfo( saveFileName );
  //check if suffix has been added
  if ( saveFileInfo.suffix().isEmpty() )
  {
    QString saveFileNameWithSuffix = saveFileName.append( ".qpt" );
    saveFileInfo = QFileInfo( saveFileNameWithSuffix );
  }
  settings.setValue( "UI/LastComposerTemplateDir", saveFileInfo.absolutePath() );

  QFile templateFile( saveFileName );
  if ( !templateFile.open( QIODevice::WriteOnly ) )
  {
    return;
  }

  QDomDocument saveDocument;
  writeXML( saveDocument, saveDocument );

  if ( templateFile.write( saveDocument.toByteArray() ) == -1 )
  {
    QMessageBox::warning( 0, tr( "Save error" ), tr( "Error, could not save file" ) );
  }
}

void QgsComposer::on_mActionLoadFromTemplate_triggered()
{
  QSettings settings;
  QString openFileDir = settings.value( "UI/lastComposerTemplateDir", "" ).toString();
  QString openFileString = QFileDialog::getOpenFileName( 0, tr( "Load template" ), openFileDir, "*.qpt" );

  if ( openFileString.isEmpty() )
  {
    return; //canceled by the user
  }

  QFileInfo openFileInfo( openFileString );
  settings.setValue( "UI/LastComposerTemplateDir", openFileInfo.absolutePath() );

  QFile templateFile( openFileString );
  if ( !templateFile.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::warning( 0, tr( "Read error" ), tr( "Error, could not read file" ) );
    return;
  }

  QDomDocument templateDocument;
  if ( !templateDocument.setContent( &templateFile, false ) )
  {
    QMessageBox::warning( 0, tr( "Read error" ), tr( "Content of template file is not valid" ) );
    return;
  }

  readXML( templateDocument );

  //clean up after template read (e.g. legend and map extent)
  cleanupAfterTemplateRead();
}

void QgsComposer::on_mActionMoveItemContent_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::MoveItemContent );
  }
}

void QgsComposer::on_mActionGroupItems_triggered()
{
  if ( mView )
  {
    mView->groupItems();
  }
}

void QgsComposer::on_mActionUngroupItems_triggered()
{
  if ( mView )
  {
    mView->ungroupItems();
  }
}

void QgsComposer::on_mActionRaiseItems_triggered()
{
  if ( mComposition )
  {
    mComposition->raiseSelectedItems();
  }
}

void QgsComposer::on_mActionLowerItems_triggered()
{
  if ( mComposition )
  {
    mComposition->lowerSelectedItems();
  }
}

void QgsComposer::on_mActionMoveItemsToTop_triggered()
{
  if ( mComposition )
  {
    mComposition->moveSelectedItemsToTop();
  }
}

void QgsComposer::on_mActionMoveItemsToBottom_triggered()
{
  if ( mComposition )
  {
    mComposition->moveSelectedItemsToBottom();
  }
}

void QgsComposer::on_mActionAlignLeft_triggered()
{
  if ( mComposition )
  {
    mComposition->alignSelectedItemsLeft();
  }
}

void QgsComposer::on_mActionAlignHCenter_triggered()
{
  if ( mComposition )
  {
    mComposition->alignSelectedItemsHCenter();
  }
}

void QgsComposer::on_mActionAlignRight_triggered()
{
  if ( mComposition )
  {
    mComposition->alignSelectedItemsRight();
  }
}

void QgsComposer::on_mActionAlignTop_triggered()
{
  if ( mComposition )
  {
    mComposition->alignSelectedItemsTop();
  }
}

void QgsComposer::on_mActionAlignVCenter_triggered()
{
  if ( mComposition )
  {
    mComposition->alignSelectedItemsVCenter();
  }
}

void QgsComposer::on_mActionAlignBottom_triggered()
{
  if ( mComposition )
  {
    mComposition->alignSelectedItemsBottom();
  }
}

void QgsComposer::moveEvent( QMoveEvent *e ) { saveWindowState(); }

void QgsComposer::resizeEvent( QResizeEvent *e )
{
  // Move size grip when window is resized
  mSizeGrip->move( rect().bottomRight() - mSizeGrip->rect().bottomRight() );

  saveWindowState();
}

void QgsComposer::saveWindowState()
{
  QSettings settings;
  settings.setValue( "/Composer/geometry", saveGeometry() );
  //settings.setValue("/Composer/splitterState", mSplitter->saveState());
}

void QgsComposer::restoreWindowState()
{
  QSettings settings;
  restoreGeometry( settings.value( "/Composer/geometry" ).toByteArray() );
  QVariant splitterState = settings.value( "/Composer/splitterState" );
  if ( splitterState != QVariant::QVariant() )
  {
    //mSplitter->restoreState(settings.value("/Composer/splitterState").toByteArray());
  }
  else
  {
    QList<int> defaultSize;
    defaultSize << 300 << 100; // page display 300 pixels, details pane 100 pixels
    //mSplitter->setSizes(defaultSize);
  }
}

void QgsComposer::on_helpPButton_clicked()
{
  QgsContextHelp::run( context_id );
}

void QgsComposer::on_closePButton_clicked()
{
  close();
}

void  QgsComposer::writeXML( QDomDocument& doc )
{

  QDomNodeList nl = doc.elementsByTagName( "qgis" );
  if ( nl.count() < 1 )
  {
    return;
  }
  QDomElement qgisElem = nl.at( 0 ).toElement();
  if ( qgisElem.isNull() )
  {
    return;
  }

  writeXML( qgisElem, doc );
}

void QgsComposer::writeXML( QDomNode& parentNode, QDomDocument& doc )
{
  QDomElement composerElem = doc.createElement( "Composer" );
  parentNode.appendChild( composerElem );

  //store composer items:
  QMap<QgsComposerItem*, QWidget*>::const_iterator itemIt = mItemWidgetMap.constBegin();
  for ( ; itemIt != mItemWidgetMap.constEnd(); ++itemIt )
  {
    itemIt.key()->writeXML( composerElem, doc );
  }

  //store composer view

  //store composition
  if ( mComposition )
  {
    mComposition->writeXML( composerElem, doc );
  }
}

void QgsComposer::readXML( const QDomDocument& doc )
{
  //look for Composer element
  QDomNodeList nl = doc.elementsByTagName( "Composer" );
  if ( nl.size() < 1 )
  {
    return; //nothing to do...
  }
  QDomElement composerElem = nl.at( 0 ).toElement();

  //look for Composition element
  QDomNodeList cnl = composerElem.elementsByTagName( "Composition" );
  if ( cnl.size() < 1 )
  {
    return; //nothing to do
  }


  //delete composer view and composition
  delete mView;
  mView = 0;
  //delete every child of mViewFrame
  QObjectList viewFrameChildren = mViewFrame->children();
  QObjectList::iterator it = viewFrameChildren.begin();
  for ( ; it != viewFrameChildren.end(); ++it )
  {
    delete( *it );
  }
  //delete composition widget
  QgsCompositionWidget* oldCompositionWidget = dynamic_cast<QgsCompositionWidget*>( mCompositionOptionsFrame->children().at( 0 ) );
  delete oldCompositionWidget;
  delete mCompositionOptionsLayout;
  mCompositionOptionsLayout = 0;

  QDomElement compositionElem = cnl.at( 0 ).toElement();

  //todo: move in function because duplicated code with constructor
  mView = new QgsComposerView( mViewFrame );
  connectSlots();

  mComposition = new QgsComposition( mQgis->mapCanvas()->mapRenderer() );
  mComposition->readXML( compositionElem, doc );

  QGridLayout *l = new QGridLayout( mViewFrame );
  l->setMargin( 0 );
  l->addWidget( mView, 0, 0 );

  //create compositionwidget
  QgsCompositionWidget* compositionWidget = new QgsCompositionWidget( mCompositionOptionsFrame, mComposition );
  compositionWidget->show();

  mCompositionOptionsLayout = new QGridLayout( mCompositionOptionsFrame );
  mCompositionOptionsLayout->setMargin( 0 );
  mCompositionOptionsLayout->addWidget( compositionWidget );

  //read and restore all the items

  //composer labels
  QDomNodeList composerLabelList = composerElem.elementsByTagName( "ComposerLabel" );
  for ( int i = 0; i < composerLabelList.size(); ++i )
  {
    QDomElement currentComposerLabelElem = composerLabelList.at( i ).toElement();
    QgsComposerLabel* newLabel = new QgsComposerLabel( mComposition );
    newLabel->readXML( currentComposerLabelElem, doc );
    addComposerLabel( newLabel );
    mComposition->addItem( newLabel );
    mComposition->update();
    mComposition->clearSelection();
    newLabel->setSelected( true );
    showItemOptions( newLabel );
  }

  //composer maps
  QDomNodeList composerMapList = composerElem.elementsByTagName( "ComposerMap" );
  for ( int i = 0; i < composerMapList.size(); ++i )
  {
    QDomElement currentComposerMapElem = composerMapList.at( i ).toElement();
    QgsComposerMap* newMap = new QgsComposerMap( mComposition );
    newMap->readXML( currentComposerMapElem, doc );
    addComposerMap( newMap );
    mComposition->addItem( newMap );
    mComposition->update();
    mComposition->clearSelection();
    newMap->setSelected( true );
    showItemOptions( newMap );
  }

  //composer scalebars
  QDomNodeList composerScaleBarList = composerElem.elementsByTagName( "ComposerScaleBar" );
  for ( int i = 0; i < composerScaleBarList.size(); ++i )
  {
    QDomElement currentScaleBarElem = composerScaleBarList.at( i ).toElement();
    QgsComposerScaleBar* newScaleBar = new QgsComposerScaleBar( mComposition );
    newScaleBar->readXML( currentScaleBarElem, doc );
    addComposerScaleBar( newScaleBar );
    mComposition->addItem( newScaleBar );
    mComposition->update();
    mComposition->clearSelection();
    newScaleBar->setSelected( true );
    showItemOptions( newScaleBar );
  }

  //composer legends
  QDomNodeList composerLegendList = composerElem.elementsByTagName( "ComposerLegend" );
  for ( int i = 0; i < composerLegendList.size(); ++i )
  {
    QDomElement currentLegendElem = composerLegendList.at( i ).toElement();
    QgsComposerLegend* newLegend = new QgsComposerLegend( mComposition );
    newLegend->readXML( currentLegendElem, doc );
    addComposerLegend( newLegend );
    mComposition->addItem( newLegend );
    mComposition->update();
    mComposition->clearSelection();
    newLegend->setSelected( true );
    showItemOptions( newLegend );
  }

  //composer pictures
  QDomNodeList composerPictureList = composerElem.elementsByTagName( "ComposerPicture" );
  for ( int i = 0; i < composerPictureList.size(); ++i )
  {
    QDomElement currentPictureElem = composerPictureList.at( i ).toElement();
    QgsComposerPicture* newPicture = new QgsComposerPicture( mComposition );
    newPicture->readXML( currentPictureElem, doc );
    addComposerPicture( newPicture );
    mComposition->addItem( newPicture );
    mComposition->update();
    mComposition->clearSelection();
    newPicture->setSelected( true );
    showItemOptions( newPicture );
  }

  mComposition->sortZList();
  mView->setComposition( mComposition );

  setSelectionTool();
}

void QgsComposer::addComposerMap( QgsComposerMap* map )
{
  if ( !map )
  {
    return;
  }

  QgsComposerMapWidget* mapWidget = new QgsComposerMapWidget( map );
  connect( this, SIGNAL( zoomLevelChanged() ), map, SLOT( renderModeUpdateCachedImage() ) );
  mItemWidgetMap.insert( map, mapWidget );
}

void QgsComposer::addComposerLabel( QgsComposerLabel* label )
{
  if ( !label )
  {
    return;
  }

  QgsComposerLabelWidget* labelWidget = new QgsComposerLabelWidget( label );
  mItemWidgetMap.insert( label, labelWidget );
}

void QgsComposer::addComposerScaleBar( QgsComposerScaleBar* scalebar )
{
  if ( !scalebar )
  {
    return;
  }

  QgsComposerScaleBarWidget* sbWidget = new QgsComposerScaleBarWidget( scalebar );
  mItemWidgetMap.insert( scalebar, sbWidget );
}

void QgsComposer::addComposerLegend( QgsComposerLegend* legend )
{
  if ( !legend )
  {
    return;
  }

  QgsComposerLegendWidget* lWidget = new QgsComposerLegendWidget( legend );
  mItemWidgetMap.insert( legend, lWidget );
}

void QgsComposer::addComposerPicture( QgsComposerPicture* picture )
{
  if ( !picture )
  {
    return;
  }

  QgsComposerPictureWidget* pWidget = new QgsComposerPictureWidget( picture );
  mItemWidgetMap.insert( picture, pWidget );
}

void QgsComposer::deleteItem( QgsComposerItem* item )
{
  QMap<QgsComposerItem*, QWidget*>::iterator it = mItemWidgetMap.find( item );

  if ( it == mItemWidgetMap.end() )
  {
    return;
  }

  delete( it.value() );
  mItemWidgetMap.remove( it.key() );
}

void QgsComposer::setSelectionTool()
{
  mActionSelectMoveItem->setChecked( true );
  on_mActionSelectMoveItem_triggered();
}

bool QgsComposer::containsWMSLayer() const
{
  QMap<QgsComposerItem*, QWidget*>::const_iterator item_it = mItemWidgetMap.constBegin();
  QgsComposerItem* currentItem = 0;
  QgsComposerMap* currentMap = 0;

  for ( ; item_it != mItemWidgetMap.constEnd(); ++item_it )
  {
    currentItem = item_it.key();
    currentMap = dynamic_cast<QgsComposerMap*>( currentItem );
    if ( currentMap )
    {
      if ( currentMap->containsWMSLayer() )
      {
        return true;
      }
    }
  }
  return false;
}

void QgsComposer::showWMSPrintingWarning()
{
  QString myQSettingsLabel = "/UI/displayComposerWMSWarning";
  QSettings myQSettings;

  bool displayWMSWarning = myQSettings.value( myQSettingsLabel, true ).toBool();
  if ( displayWMSWarning )
  {
    QgsMessageViewer* m = new QgsMessageViewer( this );
    m->setWindowTitle( tr( "Project contains WMS layers" ) );
    m->setMessage( tr( "Some WMS servers (e.g. UMN mapserver) have a limit for the WIDTH and HEIGHT parameter. Printing layers from such servers may exceed this limit. If this is the case, the WMS layer will not be printed" ), QgsMessageOutput::MessageText );
    m->setCheckBoxText( tr( "Don't show this message again" ) );
    m->setCheckBoxState( Qt::Unchecked );
    m->setCheckBoxVisible( true );
    m->setCheckBoxQSettingsLabel( myQSettingsLabel );
    m->exec();
  }
}

void QgsComposer::cleanupAfterTemplateRead()
{
  QMap<QgsComposerItem*, QWidget*>::const_iterator itemIt = mItemWidgetMap.constBegin();
  for ( ; itemIt != mItemWidgetMap.constEnd(); ++itemIt )
  {
    //update all legends completely
    QgsComposerLegend* legendItem = dynamic_cast<QgsComposerLegend*>( itemIt.key() );
    if ( legendItem )
    {
      legendItem->updateLegend();
      continue;
    }

    //update composer map extent if it does not intersect the full extent of all layers
    QgsComposerMap* mapItem = dynamic_cast<QgsComposerMap*>( itemIt.key() );
    if ( mapItem )
    {
      //test if composer map extent intersects extent of all layers
      bool intersects = false;
      QgsRectangle composerMapExtent = mapItem->extent();
      if ( mQgis )
      {
        QgsMapCanvas* canvas = mQgis->mapCanvas();
        if ( canvas )
        {
          QgsRectangle mapCanvasExtent = mQgis->mapCanvas()->fullExtent();
          if ( composerMapExtent.intersects( mapCanvasExtent ) )
          {
            intersects = true;
          }
        }
      }

      //if not: apply current canvas extent
      if ( !intersects )
      {
        double currentWidth = mapItem->rect().width();
        double currentHeight = mapItem->rect().height();
        if ( currentWidth - 0 > 0.0 ) //don't divide through zero
        {
          QgsRectangle canvasExtent = mapItem->mapRenderer()->extent();
          //adapt min y of extent such that the size of the map item stays the same
          double newCanvasExtentHeight = currentHeight / currentWidth * canvasExtent.width();
          canvasExtent.setYMinimum( canvasExtent.yMaximum() - newCanvasExtentHeight );
          mapItem->setNewExtent( canvasExtent );
        }
      }
    }
  }
}
