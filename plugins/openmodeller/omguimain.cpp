//
// C++ Implementation: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "omguimain.h"
#include <qpixmap.h>
#include <qlabel.h>
#include <qstring.h>
#include "openmodellergui.h"
#include <qsettings.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qscrollview.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qframe.h>
//om includes
#include <openmodeller/om.hh>
OmGuiMain::OmGuiMain()
  : OmGuiMainBase()
{
  //set up a scrollviewdesigner
  QGridLayout *myLayout = new QGridLayout(frameImage,1,1);
  QScrollView *myScrollView = new QScrollView(frameImage);
  
  myLayout->addWidget(myScrollView,0,0,0);
  QVBox * myVBox = new QVBox (myScrollView->viewport());
  myScrollView->addChild(myVBox);
  mPictureWidget = new QLabel(myVBox);
  myScrollView->setResizePolicy(QScrollView::AutoOneFit);
  //show();
  //
  // Note this function searches for om plugins - and should only 
  // ever be run once in the life of the qgis session (the om 
  // plugin registry is a singleton), so we do it here
  // rather than in the ctor of the plugin.
  //
  AlgorithmFactory::searchDefaultDirs();
  runWizard();
}


OmGuiMain::~OmGuiMain()
{
}

void OmGuiMain::fileExit()
{
  close();
}
void OmGuiMain::runWizard()
{
  OpenModellerGui * myOpenModellerGui = new OpenModellerGui(this,"openModeller Wizard",true,0);
  connect(myOpenModellerGui, SIGNAL(drawModelImage(QString)), this, SLOT(drawModelImage(QString)));
  connect(myOpenModellerGui, SIGNAL(modelDone(QString)), this, SLOT(modelDone(QString)));

  myOpenModellerGui->exec();
  show();
}

/**

  Convenience function for readily creating file filters.

  Given a long name for a file filter and a regular expression, return
  a file filter string suitable for use in a QFileDialog::OpenFiles()
  call.  The regular express, glob, will have both all lower and upper
  case versions added.

*/
static QString createFileFilter_(QString const &longName, QString const &glob)
{
    return longName + " (" + glob.lower() + " " + glob.upper() + ");;";
}                               // createFileFilter_



void OmGuiMain::drawModelImage(QString theFileName)
{
   //set the image label on the calculating variables screen to show the last
  //variable calculated
  std::cout << "drawModelImage Called" << std::endl;
  QPixmap myPixmap(theFileName);
  mPictureWidget->setPixmap(myPixmap); 
  mPictureWidget->show();
  //make sure the main gui windows shows (its off when app starts!)
  show();
}


void OmGuiMain::saveMapAsImage()
{
    //create a map to hold the QImageIO names and the filter names
    //the QImageIO name must be passed to the pixmap saveas image function
    typedef QMap<QString, QString> FilterMap;
    FilterMap myFilterMap;

    //find out the last used filter
    QSettings myQSettings;  // where we keep last used filter in persistant state
    QString myLastUsedFilter = myQSettings.readEntry("/openmodeller/saveAsImageFilter");
    QString myLastUsedDir = myQSettings.readEntry("/openmodeller/lastSaveAsImageDir",".");


    // get a list of supported output image types
    int myCounterInt=0;
    QString myFilters;
    for ( ; myCounterInt < QImageIO::outputFormats().count(); myCounterInt++ )
    {
        QString myFormat=QString(QImageIO::outputFormats().at( myCounterInt ));
        QString myFilter = createFileFilter_(myFormat + " format", "*."+myFormat);
        myFilters += myFilter;
        myFilterMap[myFilter] = myFormat;
    }
    
    std::cout << "Available Filters Map: " << std::endl;
    FilterMap::Iterator myIterator;
    for ( myIterator = myFilterMap.begin(); myIterator != myFilterMap.end(); ++myIterator )
    {
        std::cout << myIterator.key() << "  :  " << myIterator.data() << std::endl;
    }

    //create a file dialog using the filter list generated above
    std::auto_ptr < QFileDialog > myQFileDialog(
        new QFileDialog(
            myLastUsedDir,
            myFilters,
            0,
            QFileDialog::tr("Save file dialog"),
            tr("Choose a filename to save the map image as")
        )
    );


    // allow for selection of more than one file
    myQFileDialog->setMode(QFileDialog::AnyFile);

    if (myLastUsedFilter!=QString::null)       // set the filter to the last one used
    {
        myQFileDialog->setSelectedFilter(myLastUsedFilter);
    }


    //prompt the user for a filename
    QString myOutputFileNameQString; // = myQFileDialog->getSaveFileName(); //delete this
    if (myQFileDialog->exec() == QDialog::Accepted)
    {
        myOutputFileNameQString = myQFileDialog->selectedFile();
    }

    QString myFilterString = myQFileDialog->selectedFilter()+";;";

    std::cout << "Selected filter: " << myFilterString << std::endl;
    std::cout << "Image type to be passed to pixmap saver: " << myFilterMap[myFilterString] << std::endl;

    myQSettings.writeEntry("/openmodeller/lastSaveAsImageFilter" , myFilterString);
    myQSettings.writeEntry("/openmodeller/lastSaveAsImageDir", myQFileDialog->dirPath());

    if (!myOutputFileNameQString.isEmpty())
    {
      mPictureWidget->pixmap()->save( myOutputFileNameQString, myFilterMap[myFilterString], -1); 
    }

} 

void OmGuiMain::modelDone(QString theText)
{
  // clean up the loggin stuff
  g_log.setCallback( 0 );
  // remove the GUI
  txtbLog->setText(theText);
}
