/***************************************************************************
  main.cpp  -  description
  -------------------
begin                : Fri Jun 21 10:48:28 AKDT 2002
copyright            : (C) 2002 by Gary E.Sherman
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
/* $Id$ */
#include <iostream>
#include <qapplication.h>
#include <qfont.h>
#include <qfile.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <qstyle.h>
#include <qpixmap.h>
#include "qgisapp.h"
#include <qstringlist.h> 
#include <stdio.h>
#include <getopt.h>

int main(int argc, char *argv[])
{

  /////////////////////////////////////////////////////////////////
  // Command line options 'behaviour' flag setup
  ////////////////////////////////////////////////////////////////

  //
  // Parse the command line arguments, looking to see if the user has asked for any 
  // special behaviours. Any remaining non command arguments will be kept aside to
  // be passed as a list of layers and / or a project that should be loaded.
  //
  
  // This behaviour is used to load the app, snapshot the map,
  // save the image to disk and then exit
  QString mySnapshotFileName="";
  // This behaviour will print some help text to console and exit
  // without doing anything further
  int myHelpFlag=false;
  // This behaviour will cause QGIS to autoload a project
  QString myProjectFileName="";
  // This behaviour will allow you to force the use of a translation file
  // which is useful for testing
  QString myTranslationFileName="";
  // This is the 'leftover' arguments collection
  QStringList * myFileList=new QStringList();


  //////////////////////////////////////////////////////////////// 
  // USe the GNU Getopts utility to parse cli arguments
  // Invokes ctor `GetOpt (int argc, char **argv,  char *optstring);'
  ///////////////////////////////////////////////////////////////
  int optionChar;
  while (1)
  {
    static struct option long_options[] =
    {
      /* These options set a flag. */
      {"help", no_argument, &myHelpFlag, 1},
      /* These options don't set a flag.
       *  We distinguish them by their indices. */
      {"snapshot", required_argument, 0, 's'},
      {"lang",     required_argument, 0, 'l'},
      {"project",  required_argument, 0, 'p'},
      {0, 0, 0, 0}
    };
    /* getopt_long stores the option index here. */
    int option_index = 0;

    optionChar = getopt_long (argc, argv, "slp",
            long_options, &option_index);

    /* Detect the end of the options. */
    if (optionChar == -1)
      break;

    switch (optionChar)
    {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
              break;
            printf ("option %s", long_options[option_index].name);
            if (optarg)
              printf (" with arg %s", optarg);
            printf ("\n");
            break;

        case 's':
            mySnapshotFileName=optarg;
            break;

        case 'l':
            myTranslationFileName=optarg;
            break;

        case 'p':
            myProjectFileName=optarg;
            break;

        case '?':
            myHelpFlag=1;
            break;

        default:
            abort ();
    }
  }


  // Add any remaining args to the file list - we will attempt to load them 
  // as layers in the map view further down....
  if (optind < argc)
  {
    while (optind < argc)
      myFileList->append(argv[optind++]);
  }
  
  /////////////////////////////////////////////////////////////////////
  // Now we have the handlers for the different behaviours...
  ////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////
  // Initialise the application and the translation stuff
  /////////////////////////////////////////////////////////////////////
  
  QApplication a(argc, argv);
  // a.setFont(QFont("helvetica", 11));

  QTranslator tor(0);

  // set the location where your .qm files are in load() below as the last parameter instead of "."
  // for development, use "/" to use the english original as
  // .qm files are stored in the base project directory.
  if (myTranslationFileName!="")
  {
    QString translation = "qgis_" + myTranslationFileName;
    tor.load(translation, ".");
  } 
  else
  {
    tor.load(QString("qgis_") + QTextCodec::locale(), ".");
  }
  
  //tor.load("qgis_go", "." );
  a.installTranslator(&tor);
  /* uncomment the following line, if you want a Windows 95 look */
  //a.setStyle("Windows");

  QgisApp *qgis = new QgisApp();
  a.setMainWidget(qgis);


  
  /////////////////////////////////////////////////////////////////////
  // Print help text and exit if so required - program will terminate afterwards
  /////////////////////////////////////////////////////////////////////
  
  if (myHelpFlag)
  {
    std::cout << "Quantum GIS - Version 0.2 'Pumpkin'" << std::endl;
    std::cout << "Quantum GIS (QGIS) is a viewer for spatial data sets, including" << std::endl;
    std::cout << "raster and vector data." << std::endl << std::endl;
    std::cout << "Usage: qgis --help | --snapshot filename | [--project filename.qgs] [--lang iso-language-name] [FILES]" << std::endl ;
    std::cout << "  --help - Display help (this information)" << std::endl ;
    std::cout << "  FILES:" << std::endl ;
    std::cout << "    Files specified on the command line can include rasters, " << std::endl ;
    std::cout << "    vectors, and QGIS project files (.qgs): " << std::endl ;
    std::cout << "     1. Rasters - Supported formats include GeoTiff, DEM " << std::endl ;
    std::cout << "        and others supported by GDAL" << std::endl ;
    return 0;
  }

  
  /////////////////////////////////////////////////////////////////////
  // Load a project file if one was specified
  /////////////////////////////////////////////////////////////////////
    if(myProjectFileName!="")
    {
      qgis->addProject(myProjectFileName);
    }


  /////////////////////////////////////////////////////////////////////
  // autoload any filenames that were passed in on the command line
  /////////////////////////////////////////////////////////////////////
  for ( QStringList::Iterator myIterator = myFileList->begin(); myIterator != myFileList->end(); ++myIterator ) 
  {
    QString myLayerName = *myIterator;
#ifdef QGISDEBUG
    printf("Trying to load file : %s\n", myLayerName);
#endif
    // try to add all these layers - any unsupported file types will
    // be rejected automatically
    // The funky bool ok is so this can be debugged a bit easier...

    //nope - try and load it as raster
    bool ok = qgis->addRasterLayer(myLayerName);
    if(!ok){
      //nope - try and load it as a shape/ogr
      ok = qgis->addLayer(myLayerName);
      //we have no idea what this layer is...
      if(!ok){
        std::cout << "Unable to load " << myLayerName << std::endl;
      }
    }
  }
  
  /////////////////////////////////////////////////////////////////////
  // Take a snapshot of the map view then exit if snapshot mode requested
  /////////////////////////////////////////////////////////////////////
    if(myProjectFileName!="")
    {
      qgis->saveMapAsImage(mySnapshotFileName);
      return 1;
    }

    
  /////////////////////////////////////////////////////////////////////
  // Continue on to interactive gui...
  /////////////////////////////////////////////////////////////////////
  qgis->show();
  a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

  //
  //turn control over to the main application loop...
  //
  int result = a.exec();

  return result;
}
