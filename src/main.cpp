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
#include <qapplication.h>
#include <qfont.h>
#include <qfile.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <qstyle.h>
#include <qpixmap.h>
//#include "qgis.h"
#include "qgisapp.h"

int main(int argc, char *argv[])
{
	QString myArgString;
	QFile myQFile;
	QStringList myFileStringList;
	bool myFileExistsFlag;

	QApplication a(argc, argv);
	// a.setFont(QFont("helvetica", 11));
	QTranslator tor(0);
	// set the location where your .qm files are in load() below as the last parameter instead of "."
	// for development, use "/" to use the english original as
	// .qm files are stored in the base project directory.
        if(argc == 2){
        QString translation = "qgis_" + QString(argv[1]);
        tor.load(translation,".");
        }else{
         tor.load(QString("qgis_") + QTextCodec::locale(), ".");
        }
        //tor.load("qgis_go", "." );
	a.installTranslator(&tor);
	/* uncomment the following line, if you want a Windows 95 look */
	//a.setStyle("Windows");

	QgisApp *qgis = new QgisApp();
	a.setMainWidget(qgis);

	qgis->show();
        // 
        // autoload any filenames that were passed in on the command line
        // 
        for(int myIteratorInt = 1; myIteratorInt < argc; myIteratorInt++) {
#ifdef DEBUG                                
          printf("%d: %s\n", myIteratorInt, argv[myIteratorInt]);
#endif                                
          myQFile.setName(argv[myIteratorInt]);
          myFileExistsFlag = myQFile.open(IO_ReadOnly);
          myQFile.close();
          if(myFileExistsFlag) 
          {
#ifdef DEBUG                                
            printf("OK\n");
#endif                                
            myArgString = argv[myIteratorInt];
#ifdef DEBUG                                
            printf("Layer count: %d\n",myFileStringList.count());
#endif                                
            myFileStringList.append(myArgString);

          }
        }
#ifdef DEBUG                                
	printf("rCount: %d\n",myFileStringList.count());
#endif                                
	if(!myFileStringList.isEmpty()) {
#ifdef DEBUG                                
		printf("Loading vector files...\n");
#endif                                
                //try to add all these layers - any unsupported file types will be refected automatically
		qgis->addRasterLayer(myFileStringList);
		qgis->addLayer(myFileStringList);
	}

	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

        //
        //turn control over to the main application loop...
        //
	int result = a.exec();

	return result;
}
