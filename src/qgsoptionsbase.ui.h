/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <qsettings.h>
#include <qfiledialog.h>

void QgsOptionsBase::init()
{
    // read the current browser and set it
    QSettings settings;
    QString browser = settings.readEntry("/qgis/browser");
    cmbBrowser->setCurrentText(browser);
    int identifyValue = settings.readNumEntry("/qgis/map/identifyRadius");
    spinBoxIdentifyValue->setValue(identifyValue);
    bool hideSplashFlag = false;
    if (settings.readEntry("/qgis/hideSplash")=="true")
    {
      hideSplashFlag =true;
    }
    cbxHideSplash->setChecked(hideSplashFlag);
    cmbTheme->setCurrentText(settings.readEntry("/qgis/theme"));
}
void QgsOptionsBase::saveOptions()
{
 QSettings settings;
 settings.writeEntry("/qgis/browser", cmbBrowser->currentText());
 settings.writeEntry("/qgis/map/identifyRadius", spinBoxIdentifyValue->value());
 settings.writeEntry("/qgis/hideSplash",cbxHideSplash->isChecked());
 settings.writeEntry("/qgis/theme",cmbTheme->currentText());
 settings.writeEntry("/qgis/map/updateThreshold", spinBoxUpdateThreshold->value());
 accept();
}


void QgsOptionsBase::cbxHideSplash_toggled( bool )
{

}
void QgsOptionsBase::addTheme(QString item)
{
  cmbTheme->insertItem(item);
}


void QgsOptionsBase::themeChanged(const QString & )
{

}


void QgsOptionsBase::findBrowser()
{
    QString filter;
#ifdef WIN32
    filter = "Applications (*.exe)";
#else
    filter = "All Files (*)";
#endif
cmbBrowser->setCurrentText(QFileDialog::getOpenFileName(
                    "./",
                    filter, 
                    this,
                    "open file dialog",
                    "Choose a browser" ));
}
