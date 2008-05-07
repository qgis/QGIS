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
/* $Id$ */

#include "qgsabout.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#ifdef Q_WS_MAC
#include <ApplicationServices/ApplicationServices.h>
#else
#include <QInputDialog>
#include <QProcess>
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

  // Set up text in dialog.
  QString format("<p align=center>%1</p><p align=center><a href=%2>%3</a></p>");
  QString sentence1 = tr("Quantum GIS is licensed under the GNU General Public License");
  QString link("http://www.gnu.org/licenses");
  lblUrls->setHtml(format.arg(sentence1).arg(link).arg(link));
  QgsDebugMsg(format.arg(sentence1).arg(link).arg(link));

  // set the 60x60 icon pixmap
  QPixmap icon(QgsApplication::iconsPath() + "qgis-icon-60x60.png");
  qgisIcon->setPixmap(icon);
  //read the authors file to populate the contributors list
  QStringList lines;

  QFile file(QgsApplication::authorsFilePath());
#ifdef QGISDEBUG
  printf (("Reading authors file " + file.name() +
      ".............................................\n").toLocal8Bit().data());
#endif
  if ( file.open( QIODevice::ReadOnly ) ) {
    QTextStream stream( &file );
    // Always use UTF-8
    stream.setCodec("UTF-8");
    QString line;
#ifdef QGISDEBUG 
    int i = 1; 
#endif 

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
    listBox1->insertItems(0, lines);

    // Load in the image for the first author
    if (listBox1->count() > 0)
      listBox1->setCurrentRow(0);
  }

  // read the SPONSORS file and populate the text widget
    QFile sponsorFile(QgsApplication::sponsorsFilePath());
  #ifdef QGISDEBUG
    printf (("Reading sponsors file " + sponsorFile.name() +
        ".............................................\n").toLocal8Bit().data());
  #endif
    if ( sponsorFile.open( QIODevice::ReadOnly ) ) {
      QString sponsorHTML = "<html><body><h2>"
        + tr("QGIS Sponsors") + "</h2><center>"

        + tr("The following have sponsored QGIS by contributing money to fund development and other project costs")
        + "</center><hr>"
        + "<table border='1' cellpadding='1' width='100%'>"
        + "<tr><th>" + tr("Name") + "</th><th>" + tr("Website") + "</th>"
        + "<th></th>"
        + "</th><th>" + tr("Name") + "</th><th>" + tr("Website") + "</th></tr>";
      QString website;
      QTextStream sponsorStream( &sponsorFile );
      // Always use UTF-8
      sponsorStream.setCodec("UTF-8");
      QString sline;
      int count = 0;
      while ( !sponsorStream.atEnd() ) 
      {
        sline = sponsorStream.readLine(); // line of text excluding '\n'
        //ignore the line if it starts with a hash....
        if (sline.left(1)=="#") continue;
        count++;
        QStringList myTokens = QStringList::split("|",sline);
        if(myTokens.size() > 1)
        {
          website = myTokens[1];
        }
        else
        { 
          website = "&nbsp;";
        }
        if(count == 1){
          sponsorHTML += "<tr>";
        }
        if(count == 2)
        {
          // a spacer between donator columns
          sponsorHTML += "<td></td>";
        }
        sponsorHTML += "<td>" + myTokens[0] + "</td><td>" + website + "</td>";
        if(count == 2){
          // close the row 
          sponsorHTML += "</tr>";
          count = 0;
        }
        
      }
      sponsorHTML += "</table></body></html>";

      txtSponsors->setAcceptRichText(true);
      txtSponsors->setHtml(sponsorHTML);
      #ifdef QGISDEBUG
       std::cout << "sponsorHTML:" << sponsorHTML.toAscii().constData() << std::endl;
       std::cout << "txtSponsors:" << txtSponsors->toHtml().toAscii().constData() << std::endl;
      #endif
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

void QgsAbout::on_listBox1_currentItemChanged(QListWidgetItem *theItem)
{
  //replace spaces in author name
#ifdef QGISDEBUG 
  printf ("Loading mug: "); 
#endif 
  QString myString = listBox1->currentItem()->text();
  myString = myString.replace(" ","_");
  myString = QgsAbout::fileSystemSafe(myString);
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
  QString url = "http://lists.qgis.org/cgi-bin/mailman/listinfo/qgis-user";
  openUrl(url);
}

void QgsAbout::on_btnQgisHome_clicked()
{
  openUrl("http://qgis.org");
}

void QgsAbout::openUrl(QString url)
{
#ifdef Q_WS_MAC
  /* Use Mac OS X Launch Services which uses the user's default browser
   * and will just open a new window if that browser is already running.
   * QProcess creates a new browser process for each invocation and expects a
   * commandline application rather than a bundled application.
   */
  CFURLRef urlRef = CFURLCreateWithBytes(kCFAllocatorDefault,
      reinterpret_cast<const UInt8*>(url.utf8().data()), url.length(),
      kCFStringEncodingUTF8, NULL);
  OSStatus status = LSOpenCFURLRef(urlRef, NULL);
  status = 0; // avoid compiler warnings
  CFRelease(urlRef);
#else
  QSettings settings;
  QString browser = settings.readEntry("/qgis/browser");
  if (browser.length() == 0)
  {
    // ask user for browser and use it
    bool ok;
    QString text = QInputDialog::getText(tr("QGIS Browser Selection"),
	 tr("Enter the name of a web browser to use (eg. konqueror).\n"
	    "Enter the full path if the browser is not in your PATH.\n"
            "You can change this option later by selection Options from"
	    " the Settings menu (Help Browser tab)."),
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
    QProcess *helpProcess = new QProcess(this);
    helpProcess->start(browser, QStringList() << url);
  }
#endif
  /*  mHelpViewer = new QgsHelpViewer(this,"helpviewer",false);
      mHelpViewer->showContent(mAppDir +"/share/doc","index.html");
      mHelpViewer->show(); */
}

/*
 * The function below makes a name safe for using in most file system
 * Step 1: Code QString as UTF-8
 * Step 2: Replace all bytes of the UTF-8 above 0x7f with the hexcode in lower case.
 * Step 2: Replace all non [a-z][a-Z][0-9] with underscore (backward compatibility)
 */
QString QgsAbout::fileSystemSafe(QString filename)
{
  QString result;
  QByteArray utf8 = filename.toUtf8();

  for (int i = 0; i < utf8.size(); i++)
  {
     uchar c = utf8[i];

     if (c > 0x7f)
     {
       result = result + QString("%1").arg(c, 2, 16, QChar('0'));
     }
     else
     {
       result = result + QString(c);
     }
   }
  result.replace(QRegExp("[^a-z0-9A-Z]"), "_");
  QgsDebugMsg(result);

  return result;
}
