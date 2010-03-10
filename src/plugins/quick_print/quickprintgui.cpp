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
#include "qgslogger.h"

//qt includes
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QPrinter>

//standard includes

QuickPrintGui::QuickPrintGui( QgsMapCanvas * thepMapCanvas,
                              QWidget* parent,  Qt::WFlags fl ) :
    QDialog( parent, fl ), mpMapCanvas( thepMapCanvas )
{
  setupUi( this );
  grpOuput->hide();  //until properly implemented
  cboPageSize->addItem( "A0 (841 x 1189 mm)", "QPrinter::A0" );
  cboPageSize->addItem( "A1 (594 x 841 mm)", "QPrinter::A1" );
  cboPageSize->addItem( "A2 (420 x 594 mm)", "QPrinter::A2" );
  cboPageSize->addItem( "A3 (297 x 420 mm)", "QPrinter::A3" );
  cboPageSize->addItem( "A4 (210 x 297 mm, 8.26 x 11.69 inches)", "QPrinter::A4" );
  cboPageSize->addItem( "A5 (148 x 210 mm)", "QPrinter::A5" );
  cboPageSize->addItem( "A6 (105 x 148 mm)", "QPrinter::A6" );
  cboPageSize->addItem( "A7 (74 x 105 mm)", "QPrinter::A7" );
  cboPageSize->addItem( "A8 (52 x 74 mm)", "QPrinter::A8" );
  cboPageSize->addItem( "A9 (37 x 52 mm)", "QPrinter::A9" );
  cboPageSize->addItem( "B0 (1030 x 1456 mm)", "QPrinter::B0" );
  cboPageSize->addItem( "B1 (728 x 1030 mm)", "QPrinter::B1" );
  cboPageSize->addItem( "B10 (32 x 45 mm)", "QPrinter::B10" );
  cboPageSize->addItem( "B2 (515 x 728 mm)", "QPrinter::B2" );
  cboPageSize->addItem( "B3 (364 x 515 mm)", "QPrinter::B3" );
  cboPageSize->addItem( "B4 (257 x 364 mm)", "QPrinter::B4" );
  cboPageSize->addItem( "B5 (182 x 257 mm, 7.17 x 10.13 inches)", "QPrinter::B5" );
  cboPageSize->addItem( "B6 (128 x 182 mm)", "QPrinter::B6" );
  cboPageSize->addItem( "B7 (91 x 128 mm)", "QPrinter::B7" );
  cboPageSize->addItem( "B8 (64 x 91 mm)", "QPrinter::B8" );
  cboPageSize->addItem( "B9 (45 x 64 mm)", "QPrinter::B9" );
  cboPageSize->addItem( "C5E (163 x 229 mm)", "QPrinter::C5E" );
  cboPageSize->addItem( "Comm10E (105 x 241 mm, U.S. Common 10 Envelope)", "QPrinter::Comm10E" );
  cboPageSize->addItem( "DLE (110 x 220 mm)", "QPrinter::DLE" );
  cboPageSize->addItem( "Executive (7.5 x 10 inches, 191 x 254 mm)", "QPrinter::Executive" );
  cboPageSize->addItem( "Folio (210 x 330 mm)", "QPrinter::Folio" );
  cboPageSize->addItem( "Ledger (432 x 279 mm)", "QPrinter::Ledger" );
  cboPageSize->addItem( "Legal (8.5 x 14 inches, 216 x 356 mm)", "QPrinter::Legal" );
  cboPageSize->addItem( "Letter (8.5 x 11 inches, 216 x 279 mm)", "QPrinter::Letter" );
  readSettings();
}

QuickPrintGui::~QuickPrintGui()
{
}
void QuickPrintGui::readSettings()
{
  QSettings mySettings;
  leMapTitle->setText( mySettings.value( "quickprint/mapTitle", "Quantum GIS" ).toString() );
  leMapName->setText( mySettings.value( "quickprint/mapName", "Quick Print" ).toString() );
  teCopyright->setText( mySettings.value( "quickprint/mapCopyright", "(c) QGIS 2008" ).toString() );
  bool myIncrementLastFileFlag = mySettings.value( "quickprint/incrementLastFile", false ).toBool();
  radUseIncrementedFileName->setChecked( myIncrementLastFileFlag );

  QString myPageSize = mySettings.value( "quickprint/pageSize",
                                         "QPrinter::A4" ).toString();
  cboPageSize->setCurrentIndex( cboPageSize->findData( myPageSize ) );
}

void QuickPrintGui::writeSettings()
{
  QSettings mySettings;
  mySettings.setValue( "quickprint/mapTitle", leMapTitle->text() );
  mySettings.setValue( "quickprint/mapName", leMapName->text() );
  mySettings.setValue( "quickprint/mapCopyright", teCopyright->toPlainText() );
  mySettings.setValue( "quickprint/incrementLastFile", radUseIncrementedFileName->isChecked() );
  mySettings.setValue( "quickprint/pageSize",
                       cboPageSize->itemData( cboPageSize->currentIndex() ) );
}


void QuickPrintGui::on_buttonBox_accepted()
{
  writeSettings();
  QSettings mySettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = mySettings.value( "quickprint/lastSaveAsPdfDir", "." ).toString();

  //create a file dialog
  std::auto_ptr < QFileDialog > myFileDialog
  (
    new QFileDialog(
      this,
      QFileDialog::tr( "Save experiment report to portable document format (.pdf)" ),
      myLastUsedDir,
      tr( "Portable Document Format (*.pdf)" )
    )
  );
  myFileDialog->setFileMode( QFileDialog::AnyFile );
  myFileDialog->setAcceptMode( QFileDialog::AcceptSave );

  //prompt the user for a file name
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
    if ( myFileDialog->selectedFilter() == tr( "Portable Document Format (*.pdf)" ) )
    {
      //ensure the user never omitted the extension from the file name
      if ( !myOutputFileName.toUpper().endsWith( ".PDF" ) )
      {
        myOutputFileName += ".pdf";
      }

      // call plugin print method here
      QString myNorthArrowFile = QgsApplication::pkgDataPath() + "/svg/north_arrows/default.svg";
      QgsQuickPrint myQuickPrint;
      myQuickPrint.setMapCanvas( mpMapCanvas );
      myQuickPrint.setTitle( leMapTitle->text() );
      myQuickPrint.setName( leMapName->text() );
      myQuickPrint.setCopyright( teCopyright->toPlainText() );
      myQuickPrint.setLogo1( QgsApplication::iconsPath() + "/qgis-icon.png" );
      myQuickPrint.setNorthArrow( myNorthArrowFile );
      myQuickPrint.setOutputPdf( myOutputFileName );
      QString myPageSizeString = cboPageSize->itemData( cboPageSize->currentIndex() ).toString();
      myQuickPrint.setPageSize( QgsQuickPrint::stringToPageSize( myPageSizeString ) );
      QgsDebugMsg( QString( "Page size : %1" ).arg( myPageSizeString ) );
      myQuickPrint.printMap();
    }
    else
    {
      QMessageBox::warning( this, tr( "quickprint" ),
                            tr( "Unknown format: %1" ).arg( myFileDialog->selectedFilter() ) );
    }
    mySettings.setValue( "quickprint/lastSaveAsPdfDir", myFileDialog->directory().absolutePath() );
  }

  //close the dialog
  accept();
}

void QuickPrintGui::on_buttonBox_rejected()
{
  reject();
}
