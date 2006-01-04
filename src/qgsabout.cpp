/***************************************************************************
                          qgsabout.cpp  -  description
                             -------------------
    begin                : Sat Aug 10 2002
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
/* $Id:$ */

#include "qgsabout.h"
#include "qgsapplication.h"
#ifdef Q_OS_MACX
#include <ApplicationServices/ApplicationServices.h>
#else
#include <QInputDialog>
#include <Q3Process>
#include <QSettings>
#endif
#include <QFile>
#include <QTextStream>
#include <iostream>

/* Uncomment this block to use preloaded images
#include <map>
std::map<QString, QPixmap> mugs;
*/

QgsAbout::QgsAbout()
: QDialog(NULL, Qt::WindowSystemMenuHint)  // Modeless dialog with close button only
{
  setupUi(this);
  init();
}

QgsAbout::~QgsAbout()
{
}

void QgsAbout::init()
{
  //read the authors file to populate the contributors list
  QStringList lines;

  QFile file(QgsApplication::authorsFilePath());
#ifdef QGISDEBUG
  printf (("Reading authors file " + file.name() +
      ".............................................\n").toLocal8Bit().data());
#endif
  if ( file.open( QIODevice::ReadOnly ) ) {
    QTextStream stream( &file );
    QString line;
    int i = 1;
    while ( !stream.atEnd() ) 
    {
      line = stream.readLine(); // line of text excluding '\n'
      //ignore the line if it starts with a hash....
      if (line.left(1)=="#") continue;
#ifdef QGISDEBUG 
      printf( "Contributor: %3d: %s\n", i++, (const char *)line.toLocal8Bit().data() );
#endif 
      QStringList myTokens = QStringList::split("\t",line);
      //printf ("Added contributor name to listbox: %s ",myTokens[0]);
      lines += myTokens[0];
      // add the image to the map
      /* Uncomment this block to preload the images (takes time at initial startup)
      QString authorName = myTokens[0].replace(" ","_");

      QString myString =QString(appPath + "/images/developers/") + authorName + QString(".jpg");
      printf ("Loading mug: %s\n", myString.toLocal8Bit().data()); 
      QPixmap *pixmap = new QPixmap(myString);
      mugs[myTokens[0]] = *pixmap;
      */
    }
    file.close();
    listBox1->clear();
    listBox1->insertStringList(lines,0);
  }

}

void QgsAbout::setVersion(QString v)
{
  lblVersion->setText(v);
}

void QgsAbout::setURLs(QString urls)
{
  lblUrls->setText(urls);
}

void QgsAbout::setWhatsNew(QString txt)
{
  txtWhatsNew->setText(txt);
}

void QgsAbout::setPluginInfo(QString txt)
{
  txtBrowserPlugins->setText(txt);
}

void QgsAbout::on_buttonCancel_clicked()
{
  reject();
}

void QgsAbout::on_listBox1_currentChanged(Q3ListBoxItem *theItem)
{
  //replace spaces in author name
#ifdef QGISDEBUG 
  printf ("Loading mug: "); 
#endif 
  QString myString = listBox1->currentText();
  myString = myString.replace(" ","_");
#ifdef QGISDEBUG 
  printf ("Loading mug: %s", (const char *)myString.toLocal8Bit().data()); 
#endif 
  myString = QgsApplication::developerPath() + myString + QString(".jpg");
#ifdef QGISDEBUG 
  printf ("Loading mug: %s\n", (const char *)myString.toLocal8Bit().data()); 
#endif 
  QPixmap *pixmap = new QPixmap(myString);
  pixAuthorMug->setPixmap(*pixmap);
  /* Uncomment this block to use preloaded images
  pixAuthorMug->setPixmap(mugs[myString]);
  */
}

void QgsAbout::on_btnQgisUser_clicked()
{
  // find a browser
  QString url = "http://lists.sourceforge.net/lists/listinfo/qgis-user";
  openUrl(url);
}

void QgsAbout::on_btnQgisHome_clicked()
{
  openUrl("http://qgis.org");
}

void QgsAbout::openUrl(QString url)
{
#ifdef Q_OS_MACX
  /* Use Mac OS X Launch Services which uses the user's default browser
   * and will just open a new window if that browser is already running.
   * QProcess creates a new browser process for each invocation and expects a
   * commandline application rather than a bundled application.
   */
  CFURLRef urlRef = CFURLCreateWithBytes(kCFAllocatorDefault,
      reinterpret_cast<const UInt8*>(url.utf8().data()), url.length(),
      kCFStringEncodingUTF8, NULL);
  OSStatus status = LSOpenCFURLRef(urlRef, NULL);
  CFRelease(urlRef);
#else
  QSettings settings;
  QString browser = settings.readEntry("/qgis/browser");
  if (browser.length() == 0)
  {
    // ask user for browser and use it
    bool ok;
    QString text = QInputDialog::getText("QGIS Browser Selection",
        "Enter the name of a web browser to use (eg. konqueror).\n"
        "Enter the full path if the browser is not in your PATH.\n"
        "You can change this option later by selection Options from the Tools menu.",
        QLineEdit::Normal,
        QString::null, &ok, this);
    if (ok && !text.isEmpty())
    {
      // user entered something and pressed OK
      browser = text;
      // save the setting
      settings.writeEntry("/qgis/browser", browser);
    } else
    {
      browser = "";
    }
  }
  if (browser.length() > 0)
  {
    // find the installed location of the help files
    // open index.html using browser
    //XXX for debug on win32    QMessageBox::information(this, "Help opening...", browser + " - " + url);
    Q3Process *helpProcess = new Q3Process(this);
    helpProcess->addArgument(browser);
    helpProcess->addArgument(url);
    helpProcess->start();
  }
#endif
  /*  mHelpViewer = new QgsHelpViewer(this,"helpviewer",false);
      mHelpViewer->showContent(mAppDir +"/share/doc","index.html");
      mHelpViewer->show(); */
}
