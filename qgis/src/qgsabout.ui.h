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


void QgsAbout::showAuthorPic( QListBoxItem * )
{

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


