/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/
#include <qsettings.h>
#include <qprocess.h>
#include <qinputdialog.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qapplication.h>
#include <qstringlist.h>
#include <map>
  std::map<QString, QPixmap> mugs;
void QgsAbout::init()
{
  //read the authors file to populate the contributors list
  QStringList lines;

#if defined(Q_OS_MACX) || defined(WIN32)
  QString appPath = qApp->applicationDirPath() + "/share/qgis";
#else
  QString appPath = QString(PKGDATAPATH);
#endif

  QFile file(appPath + "/doc/AUTHORS" );
#ifdef QGISDEBUG
  printf ("Readng authors file " + file.name() + ".............................................\n");
#endif
  if ( file.open( IO_ReadOnly ) ) {
    QTextStream stream( &file );
    QString line;
    int i = 1;
    while ( !stream.atEnd() ) 
    {
      line = stream.readLine(); // line of text excluding '\n'
      //ignore the line if it starts with a hash....
      if (line.left(1)=="#") continue;
#ifdef QGISDEBUG 
      printf( "Contributor: %3d: %s\n", i++, line.latin1() );
#endif 
      QStringList myTokens = QStringList::split("\t",line);
      //printf ("Added contributor name to listbox: %s ",myTokens[0]);
      lines += myTokens[0];
      // add the image to the map
      /* Uncomment this block to preload the images (takes time at initial startup
      QString authorName = myTokens[0].replace(" ","_");

      QString myString =QString(appPath + "/images/developers/") + authorName + QString(".jpg");
      printf ("Loading mug: %s\n", myString.ascii()); 
      QPixmap *pixmap = new QPixmap(myString);
      mugs[myTokens[0]] = *pixmap;
      */
    }
    file.close();
    listBox1->clear();
    listBox1->insertStringList(lines,0);
  }

}
void QgsAbout::setVersion(QString v){
  lblVersion->setText(v) ;
}
void QgsAbout::setURLs(QString urls){
  lblUrls->setText(urls);
}
void QgsAbout::setWhatsNew(QString txt){
  txtWhatsNew->setText(txt);
}
void QgsAbout::setPluginInfo(QString txt){
  txtBrowserPlugins->setText(txt);
}


void QgsAbout::showAuthorPic( QListBoxItem * theItem)
{
  //replace spaces in author name
#ifdef QGISDEBUG 
  printf ("Loading mug: "); 
#endif 
#if defined(Q_OS_MACX) || defined(WIN32)
  QString appPath = qApp->applicationDirPath() + "/share/qgis";
#else
  QString appPath = QString(PKGDATAPATH);
#endif

  QString myString = listBox1->currentText();
  myString = myString.replace(" ","_");
#ifdef QGISDEBUG 
  printf ("Loading mug: %s", myString.ascii()); 
#endif 
  myString =QString(appPath + "/images/developers/") + myString + QString(".jpg");
#ifdef QGISDEBUG 
  printf ("Loading mug: %s\n", myString.ascii()); 
#endif 
  QPixmap *pixmap = new QPixmap(myString);
  pixAuthorMug->setPixmap(*pixmap);
  /* Uncomment this block to use preloaded images
  pixAuthorMug->setPixmap(mugs[myString]);
  */
}


void QgsAbout::qgisUserMailingList()
{
  // find a browser
  QString url = "http://lists.sourceforge.net/lists/listinfo/qgis-user";
  openUrl(url);
}
void QgsAbout::qgisHomePage()
{
  openUrl("http://qgis.org");
}
void QgsAbout::openUrl(QString url)
{
  QSettings settings;
  QString browser = settings.readEntry("/qgis/browser");
  if (browser.length() == 0)
  {
    // ask user for browser and use it
    bool ok;
    QString text = QInputDialog::getText("QGIS Browser Selection",
        "Enter the name of a web browser to use (eg. konqueror).\nEnter the full path if the browser is not in your PATH.\nYou can change this option later by selection Options from the Tools menu.",
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
    //XXX for debug on win32      QMessageBox::information(this,"Help opening...", browser + " - " + url);
    QProcess *helpProcess = new QProcess(this);
    helpProcess->addArgument(browser);
    helpProcess->addArgument(url);
    helpProcess->start();
  }
  /*  mHelpViewer = new QgsHelpViewer(this,"helpviewer",false);
      mHelpViewer->showContent(mAppDir +"/share/doc","index.html");
      mHelpViewer->show(); */

}


