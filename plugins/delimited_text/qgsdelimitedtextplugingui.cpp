/***************************************************************************
 *   Copyright (C) 2004 by Gary Sherman                                    *
 *   sherman at mrcc.com                                                   *
 *                                                                         *
 *   GUI for loading a delimited text file as a layer in QGIS              *
 *   This plugin works in conjuction with the delimited text data          *
 *   provider plugin                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include <iostream>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qfile.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include "qgsdelimitedtextplugingui.h"
#include "../../src/qgisiface.h"

QgsDelimitedTextPluginGui::QgsDelimitedTextPluginGui() : QgsDelimitedTextPluginGuiBase()
{

}

QgsDelimitedTextPluginGui::QgsDelimitedTextPluginGui( QgisIface * _qI, QWidget* parent , const char* name , bool modal , WFlags fl  ) : QgsDelimitedTextPluginGuiBase( parent, name, modal, fl ), qI(_qI)
{

}  
QgsDelimitedTextPluginGui::~QgsDelimitedTextPluginGui()
{
}

void QgsDelimitedTextPluginGui::pbnOK_clicked()
{
  //Build the delimited text URI from the user provided information
  QString uri = QString("%1?delimiter=%2&xField=%3&yField=%4")
    .arg(txtFilePath->text())
    .arg(txtDelimiter->text())
    .arg(cmbXField->currentText())
    .arg(cmbYField->currentText());
  std::cerr << "Adding layer using " << uri << std::endl; 
  // add the layer to the map
  emit drawVectorLayer(uri,QString("layername"),"delimitedtext");
} 

void QgsDelimitedTextPluginGui::updateFieldLists()
{
  // Update the x and y field dropdown boxes
#ifdef QGISDEBUG  
  std::cerr << "Updating field lists" << std::endl;
#endif
  // open the file

  if(QFile::exists(txtFilePath->text())){
    QFile *file = new QFile(txtFilePath->text());
    if ( file->open( IO_ReadOnly ) ) {
      // clear the field lists
      cmbXField->clear();
      cmbYField->clear();
      QTextStream stream( file );
      QString line;
      line = stream.readLine(); // line of text excluding '\n'
#ifdef QGISDEBUG
      std::cerr << "Attempting to split the input line: " << line <<
        " using delimiter " << txtDelimiter->text() << std::endl;
#endif

      QStringList fieldList = QStringList::split(txtDelimiter->text(), line);

#ifdef QGISDEBUG
      std::cerr << "Split line into " << fieldList.size() << " parts" << std::endl; 
#endif
      //
      // We don't know anything about a text based field other
      // than its name. All fields are assumed to be text
      int fieldPos = 0;
      for ( QStringList::Iterator it = fieldList.begin(); it != fieldList.end(); ++it ) {
        // add item to both drop-downs (X field and Y field)
        cmbXField->insertItem(*it);
        cmbYField->insertItem(*it);
      }           
      // close the file
      file->close();
    }

  }

}
void QgsDelimitedTextPluginGui::getOpenFileName()
{
  // Get a file to process, starting at the current directory
  QString s = QFileDialog::getOpenFileName(
      "./",
      "Text files (*.txt)",
      0,
      "open file dialog",
      "Choose a delimited text file to open" );

  // set path
  txtFilePath->setText(s);
  // update the field drop-down boxes by parsing and splitting
  // the header row
  updateFieldLists();
}
void QgsDelimitedTextPluginGui::enableBrowseButton(const QString &delimiter)
{
  // If a delimiter has entered, enable the browse button,
  // otherwise disable it.
  btnBrowseForFile->setEnabled(delimiter.length() > 0);
}
void QgsDelimitedTextPluginGui::help()
{
   qI->openURL("plugins/delimited_text/index.html",true);
}
