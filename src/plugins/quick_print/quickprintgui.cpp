/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "quickprintgui.h"
#include <qgsquickprint.h>
#include "qgscontexthelp.h"
#include "qgsapplication.h"

//qt includes
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>


//standard includes

QuickPrintGui::QuickPrintGui( QgsMapCanvas * thepMapCanvas, 
    QWidget* parent,  Qt::WFlags fl )
: mpMapCanvas ( thepMapCanvas ), QDialog ( parent, fl ) 
{
  setupUi(this);
  readSettings();
  grpOuput->hide();  //until properly implemented
}  

QuickPrintGui::~QuickPrintGui()
{
}
void QuickPrintGui::readSettings()
{
  QSettings mySettings;
  leMapTitle->setText(mySettings.value("quickprint/mapTitle", "Quantum GIS").toString());
  leMapName->setText(mySettings.value("quickprint/mapName", "Quick Print").toString());
  teCopyright->setText(mySettings.value("quickprint/mapCopyright", "(c) QGIS 2008").toString());
  bool myIncrementLastFileFlag = mySettings.value("quickprint/incrementLastFile", false).toBool();
  radUseIncrementedFileName->setChecked(myIncrementLastFileFlag);

}

void QuickPrintGui::writeSettings()
{
  QSettings mySettings;
  mySettings.setValue("quickprint/mapTitle", leMapTitle->text());
  mySettings.setValue("quickprint/mapName", leMapName->text());
  mySettings.setValue("quickprint/mapCopyright", teCopyright->text());
  mySettings.setValue("quickprint/incrementLastFile", radUseIncrementedFileName->isChecked());
}


void QuickPrintGui::on_buttonBox_accepted()
{
  writeSettings();
  QSettings mySettings;  // where we keep last used filter in persistant state
  QString myLastUsedDir = mySettings.value ( "quickprint/lastSaveAsPdfDir", "." ).toString();

  //create a file dialog
  std::auto_ptr < QFileDialog > myFileDialog
  (
    new QFileDialog (
      this,
      QFileDialog::tr ( "Save experiment report to portable document format (.pdf)" ),
      myLastUsedDir,
      tr ( "Portable Document Format (*.pdf)" )
    )
  );
  myFileDialog->setFileMode ( QFileDialog::AnyFile );
  myFileDialog->setAcceptMode ( QFileDialog::AcceptSave );

  //prompt the user for a filename
  QString myOutputFileName;
  if ( myFileDialog->exec() == QDialog::Accepted )
  {
    QStringList myFiles = myFileDialog->selectedFiles();
    if ( !myFiles.isEmpty() )
    {
      myOutputFileName = myFiles[0];
    }
  }

  if ( !myOutputFileName.isEmpty() )
  {
    if ( myFileDialog->selectedFilter() == tr ( "Portable Document Format (*.pdf)" ) )
    {
      //ensure the user never ommitted the extension from the filename
      if ( !myOutputFileName.toUpper().endsWith ( ".PDF" ) )
      {
        myOutputFileName += ".pdf";
      }

      // call plugin print method here
      QString myNorthArrowFile = QgsApplication::pkgDataPath() + "/svg/north_arrows/default.svg";
      QgsQuickPrint myQuickPrint;
      myQuickPrint.setMapCanvas(mpMapCanvas);
      myQuickPrint.setTitle(leMapTitle->text());
      myQuickPrint.setName(leMapName->text());
      myQuickPrint.setCopyright(teCopyright->text());
      myQuickPrint.setLogo1(QgsApplication::iconsPath() + "/qgis-icon.png");
      myQuickPrint.setNorthArrow(myNorthArrowFile);
      myQuickPrint.setOutputPdf(myOutputFileName);
      myQuickPrint.print();
    }
    else
    {
      QMessageBox::warning ( this, tr ( "quickprint" ), tr ( "Unknown format: " ) +
            myFileDialog->selectedFilter() );
    }
    mySettings.setValue ( "quickprint/lastSaveAsPdfDir", myFileDialog->directory().absolutePath() );
  }

  //close the dialog
  accept();
} 

void QuickPrintGui::on_buttonBox_rejected()
{
  reject();
}

void QuickPrintGui::on_buttonBox_helpRequested()
{
  QgsContextHelp::run(context_id);
}

