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
#include "qgsexception.h"
#include "qgsproject.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QMatrix>
#include <QMessageBox>
#include <Q3PaintDeviceMetrics>
#include <QPainter>
#include <Q3Picture>
#include <QPrinter>
#include <QSettings>
#include <QIcon>
#include <QPixmap>
#include <QToolBar>
#include <iostream>


QgsComposer::QgsComposer( QgisApp *qgis): QMainWindow()
{
  setupUi(this);
  setupTheme();

  mQgis = qgis;
  mFirstTime = true;

  std::cout << "QgsComposer::QgsComposer" << std::endl;

  mView = new QgsComposerView ( this, mViewFrame);
  mPrinter = 0;

  QGridLayout *l = new QGridLayout(mViewFrame, 1, 1 );
  l->addWidget( mView, 0, 0 );

  mCompositionOptionsLayout = new QGridLayout( mCompositionOptionsFrame, 1, 1 );
  mItemOptionsLayout = new QGridLayout( mItemOptionsFrame, 1, 1 );

  mCompositionNameComboBox->insertItem( "Map 1" );

  mComposition  = new QgsComposition( this, 1 );
  mComposition->setActive ( true );

  if ( ! connect( mQgis, SIGNAL( projectRead() ), this, SLOT( projectRead()) ) ) {
    qDebug( "unable to connect to projectRead" );
  } 
  if ( ! connect( mQgis, SIGNAL( newProject() ), this, SLOT(newProject()) ) ) {
    qDebug( "unable to connect to newProject" );
  }

  // Doesn't work, there is not such signal I think (copy from QgisApp)
  if ( ! connect(mQgis, SIGNAL(aboutToQuit()), this, SLOT(saveWindowState()) ) ) { 
    qDebug( "unable to connect to aboutToQuit" );
  }
  restoreWindowState();

  selectItem(); // Set selection tool
}

QgsComposer::~QgsComposer()
{
}

void QgsComposer::setupTheme()
{
  //calculate the active theme path
  QString myThemePath= QgsApplication::themePath();
  

  //now set all the icons
  mActionOpenTemplate->setIconSet(QIcon(QPixmap(myThemePath + "/mActionFileOpen.png")));
  mActionSaveTemplateAs->setIconSet(QIcon(QPixmap(myThemePath + "/mActionFileSaveAs.png")));
  mActionExportAsImage->setIconSet(QIcon(QPixmap(myThemePath + "/mActionExportMapServer.png")));
  mActionExportAsSVG->setIconSet(QIcon(QPixmap(myThemePath + "/mActionSaveAsSVG.png")));
  mActionPrint->setIconSet(QIcon(QPixmap(myThemePath + "/mActionFilePrint.png")));
  mActionZoomAll->setIconSet(QIcon(QPixmap(myThemePath + "/mActionZoomFullExtent.png")));
  mActionZoomIn->setIconSet(QIcon(QPixmap(myThemePath + "/mActionZoomIn.png")));
  mActionZoomOut->setIconSet(QIcon(QPixmap(myThemePath + "/mActionZoomOut.png")));
  mActionRefreshView->setIconSet(QIcon(QPixmap(myThemePath + "/mActionDraw.png")));
  mActionAddImage->setIconSet(QIcon(QPixmap(myThemePath + "/mActionSaveMapAsImage.png")));
  mActionAddNewMap->setIconSet(QIcon(QPixmap(myThemePath + "/mActionAddRasterLayer.png")));
  mActionAddNewLabel->setIconSet(QIcon(QPixmap(myThemePath + "/mActionLabel.png")));
  mActionAddNewVectLegend->setIconSet(QIcon(QPixmap(myThemePath + "/mActionAddLegend.png")));
  mActionAddNewScalebar->setIconSet(QIcon(QPixmap(myThemePath + "/mActionScaleBar.png")));
  mActionSelectMoveItem->setIconSet(QIcon(QPixmap(myThemePath + "/mActionPan.png")));
}

void QgsComposer::open ( void )
{
  if ( mFirstTime ) {
    mComposition->createDefault();
    mFirstTime = false;
  }

  show();
}

void QgsComposer::removeWidgetChildren ( QWidget *w )
{
  std::cout << "QgsComposer::removeWidgetChildren" << std::endl;

#if QT_VERSION < 0x040000
  const QObjectList *ol = mItemOptionsFrame->children();
  if ( ol ) {
    QObjectListIt olit( *ol );
    QObject *ob;
    while( (ob = olit.current()) ) {
      ++olit;
      if( ob->isWidgetType() ) {
        QWidget *ow = (QWidget *) ob;
        w->removeChild ( ob );
        ow->hide ();
      }
    }
  }
#else
  const QObjectList ol = mItemOptionsFrame->children();
  if ( !ol.isEmpty() ) 
  {
    QListIterator<QObject*> olit( ol );
    QObject *ob;
    while( olit.hasNext() )
    {
      ob = olit.next();
      if( ob->isWidgetType() ) 
      {
        QWidget *ow = (QWidget *) ob;
        w->removeChild ( ob );
        ow->hide ();
      }
    }
  }
#endif
}

void QgsComposer::showCompositionOptions ( QWidget *w ) {
  std::cout << "QgsComposer::showCompositionOptions" << std::endl;

  removeWidgetChildren ( mCompositionOptionsFrame );

  if ( w ) { 
    w->reparent ( mCompositionOptionsFrame, QPoint(0,0), TRUE );
    mCompositionOptionsLayout->addWidget( w, 0, 0 );
  }
}

void QgsComposer::showItemOptions ( QWidget *w )
{
  std::cout << "QgsComposer::showItemOptions" << std::endl;

  removeWidgetChildren ( mItemOptionsFrame );

  // NOTE: It is better to leave there the tab with item options if w is NULL

  if ( w ) {
    w->reparent ( mItemOptionsFrame, QPoint(0,0), TRUE );

    mItemOptionsLayout->addWidget( w, 0, 0 );
    mOptionsTabWidget->setCurrentPage (1);
  }
}

QgsMapCanvas *QgsComposer::mapCanvas(void)
{
  return mQgis->getMapCanvas();
}

QgsComposerView *QgsComposer::view(void)
{
  return mView;
}

QgsComposition *QgsComposer::composition(void)
{
  return mComposition;
}

void QgsComposer::zoomFull(void)
{
  QMatrix m;

  // scale
  double xscale = 1.0 * (mView->width()-10) / mComposition->canvas()->width();
  double yscale = 1.0 * (mView->height()-10) / mComposition->canvas()->height();
  double scale = ( xscale < yscale ? xscale : yscale );

  // translate
  double dx = ( mView->width() - scale * mComposition->canvas()->width() ) / 2;
  double dy = ( mView->height() - scale * mComposition->canvas()->height() ) / 2;

  m.translate ( dx, dy );
  m.scale( scale, scale );

  mView->setWorldMatrix( m );
  mView->repaintContents();
}

void QgsComposer::on_mActionZoomAll_activated(void)
{
  zoomFull();
}

void QgsComposer::on_mActionZoomIn_activated(void)
{
  QMatrix m = mView->worldMatrix();
  m.scale( 2.0, 2.0 );
  mView->setWorldMatrix( m );
  mView->repaintContents();
}

void QgsComposer::on_mActionZoomOut_activated(void)
{
  QMatrix m = mView->worldMatrix();
  m.scale( 0.5, 0.5 );
  mView->setWorldMatrix( m );
  mView->repaintContents();
}

void QgsComposer::on_mActionRefreshView_activated(void)
{
  mComposition->refresh();
  mView->repaintContents();
}

void QgsComposer::on_mActionPrint_activated(void)
{
  /* Uff!!! It is impossible to set a custom page size for QPrinter.
   * Only the sizes hardcoded in Qt library can be used.
   * 'Fortunately', it seems that everything is written to postscript output file,
   * regardless the pages size ->
   *
   * 1) outputToFile == false: If the output page size doesn't match the user defined size 
   *                           (in QgsComposer) the output is rescaled and centered so that 
   *                           it fit to the select output page size. 
   *                           a warning is displayed, that the map was rescaled.
   *                           
   * 2) outputToFile == true:  the output postscript file is written (page size is not
   *                           important but bigger is better because it lefts enough space
   *                           in BoundingBox definition), then the file is reopened, 
   *                           and the BoundingBox is redefined.
   */ 

  // NOTE: QT 3.2 has QPrinter::setOptionEnabled but only for three options 
  if ( !mPrinter ) {

    mPrinter = new QPrinter ( QPrinter::PrinterResolution );
    //mPrinter = new QPrinter ( QPrinter::HighResolution );
    //mPrinter = new QPrinter ( QPrinter::ScreenResolution );
    mPrinter->setFullPage ( true );
#ifndef Q_OS_MACX
    // For Qt/Mac 3, don't set outputToFile to true before calling setup 
    // because it wiil suppress the Print dialog and output to file without
    // giving the user a chance to select a printer instead.
    // The Mac Print dialog provides an option to create a pdf which is
    // intended to be invisible to the application. If an eps is desired,
    // a custom Mac Print dialog is needed.
    mPrinter->setOutputToFile (true ) ;
    mPrinter->setOutputFileName ( QDir::convertSeparators ( QDir::home().path() + "/" + "qgis.eps") );
#endif

    if ( mComposition->paperOrientation() == QgsComposition::Portrait ) {
      mPrinter->setOrientation ( QPrinter::Portrait );
    } else {
      mPrinter->setOrientation ( QPrinter::Landscape );
    }
    mPrinter->setColorMode ( QPrinter::Color );
    mPrinter->setPageSize ( QPrinter::A4 );
  }

  mPrinter->setResolution ( mComposition->resolution() );

  if ( mPrinter->setup(this) ) {
    // TODO: mPrinter->setup() moves the composer under Qgisapp, get it to foreground somehow
    //       raise() for now, is it something better?
    raise ();

    // TODO: Qt does not add pagesize to output file, it can cause problems if ps2pdf is used 
    // or if default page on printer is different.
    // We should add somewhere in output file:
    // << /PageSize [ %d %d ] >> setpagedevice
    // %d %d is width and height in points
    
    // WARNING: If QCanvasView recieves repaint signal during the printing
    // (e.g. covered by QPrinter::setup dialog) it breaks somehow drawing of QCanvas items 
    // (for example not all features in the map are drawn.
    // I don't know how to stop temporarily updating, (I don't want to reimplement 
    // repaint in QCanvasView, so I unset the view, print and reset.
    mView->setCanvas(0);

    int resolution = mPrinter->resolution();

    std::cout << "Resolution = " << resolution << std::endl;

    double scale = resolution / 25.4 / mComposition->scale();

    mComposition->setPlotStyle ( QgsComposition::Postscript );

    if ( mPrinter->outputToFile() ) {
      try {
      std::cout << "Print to file" << std::endl;

#ifdef Q_WS_X11
      // NOTE: On UNIX setPageSize after setup() works, but setOrientation does not
      //   -> the BoundingBox must follow the orientation 

      QPrinter::PageSize psize = mPrinter->pageSize();
      // B0 ( 1000x1414mm = 2835x4008pt ) is the biggest defined in Qt, a map can be bigger 
      // but probably not bigger than 9999x9999pt = 3527x3527mm 
      mPrinter->setPageSize ( QPrinter::B0 );
#endif

      QPainter p(mPrinter);
      p.scale ( scale, scale); 

      mComposition->canvas()->drawArea ( QRect(0,0, 
            (int) (mComposition->paperWidth() * mComposition->scale()),
            (int) (mComposition->paperHeight() * mComposition->scale()) ), 
            &p, FALSE );

      p.end();

#ifdef Q_WS_X11
      // reset the page
      mPrinter->setPageSize ( psize );

      QFile f(mPrinter->outputFileName());

      // Overwrite the bounding box
      if (!f.open( QIODevice::ReadWrite )) {
        throw QgsIOException(tr("Couldn't open " + f.name() + tr(" for read/write")));
      }
      Q_LONG offset = 0;
      Q_LONG size;
      bool found = false;
      QString s;
      char buf[101];
      while ( !f.atEnd() ) {
        size = f.readLine ( buf, 100 );
        s = QString(buf);
        if ( s.find ("%%BoundingBox:") == 0 ) {
          found = true;
          break;
        }
        offset += size;
      }

      if ( found ) {
        int w,h;

        w = (int) ( 72 * mComposition->paperWidth() / 25.4 );
        h = (int) ( 72 * mComposition->paperHeight() / 25.4 );
        if ( mPrinter->orientation() == QPrinter::Landscape ) { 
          int tmp = w; w = h; h = tmp;
        }
        s.sprintf( "%%%%BoundingBox: 0 0 %d %d", w, h );

        if ( s.length() > size ) {
          QMessageBox::warning(this,"Error in Print", "Cannot format BoundingBox");
        } else {
          if ( ! f.at(offset) ) {
            QMessageBox::warning(this,"Error in Print", "Cannot seek");
          } else {
            /* Write spaces (for case the size > s.length() ) */
            QString es;
            es.fill(' ', size-1 );
            f.flush();
            if ( f.writeBlock ( es.toLocal8Bit().data(), size-1 ) < size-1 ) {
              QMessageBox::warning(this,"Error in Print", "Cannot overwrite BoundingBox");
            }
            f.flush();
            f.at(offset);
            f.flush();
            if ( f.writeBlock ( s.toLocal8Bit().data(), s.length() ) <  s.length()-1 ) {
              QMessageBox::warning(this,"Error in Print", "Cannot overwrite BoundingBox");
            }
            f.flush();
          }
        }
      } else {
        QMessageBox::warning(this,"Error in Print", "Cannot find BoundingBox");
      }
      f.close();

      // Overwrite translate
      if ( mPrinter->orientation() == QPrinter::Portrait ) { 
        if (!f.open( QIODevice::ReadWrite )) {
          throw QgsIOException(tr("Couldn't open " + f.name() + tr(" for read/write")));
        }
        offset = 0;
        found = false;

        //Example:
        //0 4008 translate 1 -1 scale/defM ...
        QRegExp rx ( "^0 [^ ]+ translate ([^ ]+ [^ ]+) scale/defM matrix CM d \\} d" );

        while ( !f.atEnd() ) {
          size = f.readLine ( buf, 100 );
          s = QString(buf);
          if ( rx.search( s ) != -1 ) {
            found = true;
            break;
          }
          offset += size;
        }

        if ( found ) {
          int trans;

          trans = (int) ( 72 * mComposition->paperHeight() / 25.4 );
          s.sprintf( "0 %d translate %s scale/defM matrix CM d } d", trans, (const char *)rx.cap(1).toLocal8Bit().data() );

          if ( s.length() > size ) {
            QMessageBox::warning(this,"Error in Print", "Cannot format translate");
          } else {
            if ( ! f.at(offset) ) {
              QMessageBox::warning(this,"Error in Print", "Cannot seek");
            } else {
              /* Write spaces (for case the size > s.length() ) */
              QString es;
              es.fill(' ', size-1 );
              f.flush();
              if ( f.writeBlock ( es.toLocal8Bit().data(), size-1 ) < size-1 ) {
                QMessageBox::warning(this,"Error in Print", "Cannot overwrite translate");
              }
              f.flush();
              f.at(offset);
              f.flush();
              if ( f.writeBlock ( s.toLocal8Bit().data(), s.length() ) <  s.length()-1 ) {
                QMessageBox::warning(this,"Error in Print", "Cannot overwrite translate");
              }
              f.flush();
            }
          }
        } else {
          QMessageBox::warning(this,"Error in Print", "Cannot find translate");
        }
        f.close();
      }
#endif
      } catch (QgsIOException e) {
        QMessageBox::warning(this,"File IO Error", e.what());
      }
    } else {  // print to printer
	bool print = true;

	// Check size 
	Q3PaintDeviceMetrics pm(mPrinter);

	std::cout << "Paper: " << pm.widthMM() << " x " << pm.heightMM() << std::endl;
	if ( mComposition->paperWidth() != pm.widthMM() || 
	    mComposition->paperHeight() != pm.heightMM() )
	{
	  int answer = QMessageBox::warning ( 0, "Paper does not match", 
	      "The selected paper size does not match the composition size",
	      QMessageBox::Ok,  QMessageBox::Abort );

	  if ( answer == QMessageBox::Abort )
	    print = false;

	}

	if ( print ) {
	  std::cout << "Printing ... " << std::endl;
	  QPainter p(mPrinter);
	  p.scale ( scale, scale); 
	  mComposition->canvas()->drawArea ( QRect(0,0, 
		(int) (mComposition->paperWidth() * mComposition->scale()),
		(int) (mComposition->paperHeight() * mComposition->scale()) ), 
	      &p, FALSE );
	  p.end();
	  std::cout << "... printing finished" << std::endl;
	}
      }

      mComposition->setPlotStyle ( QgsComposition::Preview );
      mView->setCanvas(mComposition->canvas());
  } 
  else 
  {
      raise ();
  }
}

void QgsComposer::on_mActionExportAsImage_activated(void)
{
  // Image size 
  int width = (int) (mComposition->resolution() * mComposition->paperWidth() / 25.4); 
  int height = (int) (mComposition->resolution() * mComposition->paperHeight() / 25.4); 

  int memuse = width * height * 3 / 1000000;  // pixmap + image
#ifdef QGISDEBUG
  std::cout << "Image " << width << " x " << height << std::endl;
  std::cout << "memuse = " << memuse << std::endl;
#endif

  if ( memuse > 200 ) { // cca 4500 x 4500
    int answer = QMessageBox::warning ( 0, "Big image", 
        "To create image " + QString::number(width) + " x " 
        + QString::number(height) 
        + " requires circa " 
        + QString::number(memuse) + " MB of memory", 
        QMessageBox::Ok,  QMessageBox::Abort );
  
    raise ();
    if ( answer == QMessageBox::Abort ) return;
  }

  // Get file and format (stolen from qgisapp.cpp but modified significantely)

  //create a map to hold the QImageIO names and the filter names
  //the QImageIO name must be passed to the mapcanvas saveas image function
  typedef QMap<QString, QString> FilterMap;
  FilterMap myFilterMap;

  //find out the last used filter
  QSettings myQSettings;  // where we keep last used filter in persistant state
  QString myLastUsedFormat = myQSettings.readEntry("/UI/lastSaveAsImageFormat", "PNG" );
  QString myLastUsedFile = myQSettings.readEntry("/UI/lastSaveAsImageFile","qgis.png");

  // get a list of supported output image types
  int myCounterInt=0;
  QString myFilters;
  QString myLastUsedFilter;
  for ( ; myCounterInt < QPictureIO::outputFormats().count(); myCounterInt++ )
  {
    QString myFormat=QString(QPictureIO::outputFormats().at( myCounterInt ));
    QString myFilter = myFormat + " format (*." + myFormat.lower() + " *." + myFormat.upper() + ")";
    if ( myCounterInt > 0 ) myFilters += ";;";
    myFilters += myFilter;
    myFilterMap[myFilter] = myFormat;
    if ( myFormat == myLastUsedFormat ) 
    { 
      myLastUsedFilter = myFilter;
    }
  }
#ifdef QGISDEBUG
  std::cout << "Available Filters Map: " << std::endl;
  FilterMap::Iterator myIterator;
  for ( myIterator = myFilterMap.begin(); myIterator != myFilterMap.end(); ++myIterator )
  {
    std::cout << myIterator.key().toLocal8Bit().data() << "  :  " << myIterator.data().toLocal8Bit().data() << std::endl;
  }
#endif

  //create a file dialog using the the filter list generated above
  std::auto_ptr < QFileDialog > myQFileDialog(
      new QFileDialog(
        this,
        tr("Choose a filename to save the map image as"),
        "",
        myFilters
        )
      );
  myQFileDialog->selectFile( myLastUsedFile );

  // allow for selection of more than one file
  myQFileDialog->setMode(QFileDialog::AnyFile);

  // set the filter to the last one used
  myQFileDialog->selectFilter(myLastUsedFilter);

  //prompt the user for a filename
  QString myOutputFileNameQString; // = myQFileDialog->getSaveFileName(); //delete this

  int result = myQFileDialog->exec();
  raise ();
  
  if ( result != QDialog::Accepted) return;

  myOutputFileNameQString = myQFileDialog->selectedFile();
  QString myFilterString = myQFileDialog->selectedFilter();
#ifdef QGISDEBUG
  std::cout << "Selected filter: " << myFilterString.toLocal8Bit().data() << std::endl;
  std::cout << "Image type: " << myFilterMap[myFilterString].toLocal8Bit().data() << std::endl;
#endif

  myQSettings.writeEntry("/UI/lastSaveAsImageFormat" , myFilterMap[myFilterString] );
  myQSettings.writeEntry("/UI/lastSaveAsImageFile", myOutputFileNameQString);

  if ( myOutputFileNameQString == "" ) return;

  double scale = (double) (mComposition->resolution() / 25.4 / mComposition->scale());

  mView->setCanvas(0);
  mComposition->setPlotStyle ( QgsComposition::Print );

  QPixmap pixmap ( width, height );
  pixmap.fill ( QColor(255,255,255) ) ;
  QPainter p(&pixmap);
  p.scale ( scale, scale); 
  mComposition->canvas()->drawArea ( QRect(0,0, 
        (int) (mComposition->paperWidth() * mComposition->scale()),
        (int) (mComposition->paperHeight() * mComposition->scale()) ), 
      &p, FALSE );
  p.end();

  mComposition->setPlotStyle ( QgsComposition::Preview );
  mView->setCanvas(mComposition->canvas());

  pixmap.save ( myOutputFileNameQString, myFilterMap[myFilterString].toLocal8Bit().data() );
}

void QgsComposer::on_mActionExportAsSVG_activated(void)
{
  QSettings myQSettings;
  QString myLastUsedFile = myQSettings.readEntry("/UI/lastSaveAsSvgFile","qgis.svg");

  QFileDialog *myQFileDialog = new QFileDialog( this, "Save svg file dialog",
                                                "", "SVG Format (*.svg *SVG)" );
  
  myQFileDialog->setCaption(tr("Choose a filename to save the map as"));

  myQFileDialog->selectFile( myLastUsedFile );
  myQFileDialog->setMode(QFileDialog::AnyFile);

  int result = myQFileDialog->exec();
  raise ();
  
  if ( result != QDialog::Accepted) return;
  QString myOutputFileNameQString = myQFileDialog->selectedFile();

  if ( myOutputFileNameQString == "" ) return;

  myQSettings.writeEntry("/UI/lastSaveAsSvgFile", myOutputFileNameQString);

  mView->setCanvas(0);
  mComposition->setPlotStyle ( QgsComposition::Print );

  Q3Picture pic;
  QPainter p(&pic);
  mComposition->canvas()->drawArea ( QRect(0,0, 
        (int) (mComposition->paperWidth() * mComposition->scale()),
        (int) (mComposition->paperHeight() * mComposition->scale()) ), 
      &p, FALSE );
  p.end();

  mComposition->setPlotStyle ( QgsComposition::Preview );
  mView->setCanvas(mComposition->canvas());

  QRect br = pic.boundingRect();

  pic.save ( myOutputFileNameQString, "svg" );
}

void QgsComposer::setToolActionsOff(void)
{
  mActionOpenTemplate->setOn ( false );
  mActionSaveTemplateAs->setOn ( false );
  mActionExportAsImage->setOn ( false );
  mActionExportAsSVG->setOn ( false );
  mActionPrint->setOn ( false );
  mActionZoomAll->setOn ( false );
  mActionZoomIn->setOn ( false );
  mActionZoomOut->setOn ( false );
  mActionRefreshView->setOn ( false );
  mActionAddNewMap->setOn ( false );
  mActionAddNewLabel->setOn ( false );
  mActionAddNewVectLegend->setOn ( false );
  mActionAddNewScalebar->setOn ( false );
  mActionSelectMoveItem->setOn ( false );
}

void QgsComposer::selectItem(void)
{
  mComposition->setTool ( QgsComposition::Select );
  setToolActionsOff();
  mActionSelectMoveItem->setOn ( true );
}

void QgsComposer::on_mActionSelectMoveItem_activated(void)
{
  selectItem();
}

void QgsComposer::on_mActionAddNewMap_activated(void)
{
  mComposition->setTool ( QgsComposition::AddMap );
  setToolActionsOff();
  mActionAddNewMap->setOn ( true );
}

void QgsComposer::on_mActionAddNewVectLegend_activated(void)
{
  mComposition->setTool ( QgsComposition::AddVectorLegend );
  setToolActionsOff();
  mActionAddNewVectLegend->setOn ( true );
}

void QgsComposer::on_mActionAddNewLabel_activated(void)
{
  mComposition->setTool ( QgsComposition::AddLabel );
  setToolActionsOff();
  mActionAddNewLabel->setOn ( true );
}

void QgsComposer::on_mActionAddNewScalebar_activated(void)
{
  mComposition->setTool ( QgsComposition::AddScalebar );
  setToolActionsOff();
  mActionAddNewScalebar->setOn ( true );
}

void QgsComposer::on_mActionAddImage_activated(void)
{
  mComposition->setTool ( QgsComposition::AddPicture );
  setToolActionsOff();
  mActionAddImage->setOn ( true );
}

void QgsComposer::moveEvent ( QMoveEvent *e ) { saveWindowState(); }
void QgsComposer::resizeEvent ( QResizeEvent *e ) { saveWindowState(); }

void QgsComposer::saveWindowState()
{
  std::cout << "QgsComposer::saveWindowState" << std::endl;
  QSettings settings;

  QPoint p = this->pos();
  QSize s = this->size();

  settings.writeEntry("/Composer/geometry/x", p.x());
  settings.writeEntry("/Composer/geometry/y", p.y());
  settings.writeEntry("/Composer/geometry/w", s.width());
  settings.writeEntry("/Composer/geometry/h", s.height());

  Q3ValueList<int> list = mSplitter->sizes();
  Q3ValueList<int>::Iterator it = list.begin();
  settings.writeEntry("/Composer/geometry/wiev", (int)(*it) );
  it++;
  settings.writeEntry("/Composer/geometry/options", (int)(*it) );
}

void QgsComposer::restoreWindowState()
{
  QSettings settings;

  QDesktopWidget *d = QApplication::desktop();
  int dw = d->width();
  int dh = d->height();
  int w = settings.readNumEntry("/Composer/geometry/w", 600);
  int h = settings.readNumEntry("/Composer/geometry/h", 400);
  int x = settings.readNumEntry("/Composer/geometry/x", (dw - 600) / 2);
  int y = settings.readNumEntry("/Composer/geometry/y", (dh - 400) / 2);
  resize(w, h);
  move(x, y);

  // This doesn't work
  Q3ValueList<int> list;
  w = settings.readNumEntry("/Composer/geometry/view", 300);
  list.push_back( w );
  w = settings.readNumEntry("/Composer/geometry/options", 300);
  list.push_back( w );
  mSplitter->setSizes ( list );
}

void QgsComposer::projectRead(void)
{
  std::cout << "QgsComposer::projectRead" << std::endl;

  if ( mComposition ) delete mComposition;
  mComposition  = new QgsComposition( this, 1 );

  // Read composition if it is defined in project
  QStringList l = QgsProject::instance()->subkeyList ( "Compositions", "" );

  bool found = false;
  for ( QStringList::iterator it = l.begin(); it != l.end(); ++it ) {
    std::cout << "key: " << (*it).toLocal8Bit().data() << std::endl;
    if ( (*it).compare ( "composition_1" ) == 0 ) {
      found = true;
      break;
    }
  }

  if ( found ) {
    mComposition->readSettings ( );
    mFirstTime = false;
  } else { 
    if ( isVisible() ) {
      mComposition->createDefault();
      mFirstTime = false;
    } else {
      mFirstTime = true;
    }
  }

  mComposition->setActive ( true );
}

void QgsComposer::newProject(void)
{
  std::cout << "QgsComposer::newProject" << std::endl;

  if ( mComposition ) delete mComposition;

  mComposition  = new QgsComposition( this, 1 );
  mComposition->setActive ( true );

  // If composer is visible, create default immediately, otherwise wait for the first open()
  if ( isVisible() ) {
    mComposition->createDefault();
    mFirstTime = false;
  } else {
    mFirstTime = true;
  }
}

bool QgsComposer::writeSettings ( void )
{
#ifdef WIN32
  bool ok = true;
#else
  bool ok = false;
#endif
  return ok;
}

bool QgsComposer::readSettings ( void )
{
#ifdef WIN32
  bool ok = true;
#else
  bool ok = false;
#endif
  return ok;
}

bool QgsComposer::writeXML( QDomNode & node, QDomDocument & doc )
{
  std::cout << "QgsComposer::writeXML" << std::endl;

  QDomElement compositionsNode = doc.createElement("compositions");

  node.appendChild( compositionsNode );

  return true;
}

bool QgsComposer::readXML( QDomNode & node )
{
  std::cout << "QgsComposer::readXML" << std::endl;

  return true;
}

