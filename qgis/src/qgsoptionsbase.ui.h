/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <qsettings.h>

void QgsOptionsBase::init()
{
    // read the current browser and set it
    QSettings settings;
    QString browser = settings.readEntry("/qgis/browser");
    cmbBrowser->setCurrentText(browser);
}
void QgsOptionsBase::saveOptions()
{
 QSettings settings;
 settings.writeEntry("/qgis/browser", cmbBrowser->currentText());
 accept();
}
