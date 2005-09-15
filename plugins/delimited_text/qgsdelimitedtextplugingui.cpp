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
/* $Id$ */
#include <iostream>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qfile.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qmessagebox.h>
#include "qgsdelimitedtextplugingui.h"
#include "../../src/qgisiface.h"

QgsDelimitedTextPluginGui::QgsDelimitedTextPluginGui() : QgsDelimitedTextPluginGuiBase()
{

}

QgsDelimitedTextPluginGui::QgsDelimitedTextPluginGui( QgisIface * _qI, QWidget* parent , const char* name , bool modal , WFlags fl  ) : QgsDelimitedTextPluginGuiBase( parent, name, modal, fl ), qI(_qI)
{
  // at startup, fetch the last used delimiter and directory from
  // settings
  QSettings settings;
  QString key = "/Qgis/delimited_text_plugin";
  txtDelimiter->setText(settings.readEntry(key + "/delimiter"));

}  
QgsDelimitedTextPluginGui::~QgsDelimitedTextPluginGui()
{
}

void QgsDelimitedTextPluginGui::pbnOK_clicked()
{
  if(txtLayerName->text().length() > 0)
  {
    //Build the delimited text URI from the user provided information
    QString uri = QString("%1?delimiter=%2&xField=%3&yField=%4")
      .arg(txtFilePath->text())
      .arg(txtDelimiter->text())
      .arg(cmbXField->currentText())
      .arg(cmbYField->currentText());
    std::cerr << "Adding layer using " << uri.local8Bit() << std::endl; 
    // add the layer to the map
    emit drawVectorLayer(uri,txtLayerName->text(),"delimitedtext");
    // store the settings

    QSettings settings;
    QString key = "/Qgis/delimited_text_plugin";
    settings.writeEntry(key + "/delimiter", txtDelimiter->text());
    QFileInfo fi(txtFilePath->text());
    settings.writeEntry(key + "/text_path", fi.dirPath());
  }
  else
  {
    QMessageBox::warning(this, tr("No layer name"), tr("Please enter a layer name before adding the layer to the map"));
  }
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
      if(txtDelimiter->text().length() > 0)
      {
#ifdef QGISDEBUG
        std::cerr << "Attempting to split the input line: " << line.local8Bit() <<
          " using delimiter " << txtDelimiter->text().local8Bit() << std::endl;
#endif

        QStringList fieldList = QStringList::split(QRegExp(txtDelimiter->text()), line);

#ifdef QGISDEBUG
        std::cerr << "Split line into " << fieldList.size() << " parts" << std::endl; 
#endif
        //
        // We don't know anything about a text based field other
        // than its name. All fields are assumed to be text
        int fieldPos = 0;
        for ( QStringList::Iterator it = fieldList.begin(); it != fieldList.end(); ++it ) {
          // add item to both drop-downs (X field and Y field)
          if((*it).length() > 0)
          {
            cmbXField->insertItem(*it);
            cmbYField->insertItem(*it);
          }
        }           
        // enable the buttons
        enableButtons();
      }
      else
      {
        QMessageBox::warning(this, tr("No delimiter"),tr("Please specify a delimiter prior to parsing the file"));
      }
      // clear the sample text box
      txtSample->clear();
      // put the header row in the sample box
      txtSample->insert(line + "\n"); 
      // put a few more lines into the sample box
      int counter = 0;
      while((line=stream.readLine()) && (counter < 20))
      {
        txtSample->insert(line + "\n");
        counter++;
      }
      // close the file
      file->close();
    }

  }

}
void QgsDelimitedTextPluginGui::getOpenFileName()
{
  // Get a file to process, starting at the current directory
  // Set inital dir to last used
  QSettings settings;

  QString s = QFileDialog::getOpenFileName(
      settings.readEntry("/Qgis/delimited_text_plugin/text_path","./"),
      "Text files (*.txt)",
      0,
      "open file dialog",
      tr("Choose a delimited text file to open") );

  // set path
  txtFilePath->setText(s);
  // update the field drop-down boxes by parsing and splitting
  // the header row
  updateFieldLists();
}
void QgsDelimitedTextPluginGui::enableButtons()
{
  pbnParse->setEnabled(txtDelimiter->text().length() > 0 && txtFilePath->text().length() > 0);
  pbnOK->setEnabled(txtDelimiter->text().length() > 0 && txtFilePath->text().length() > 0);
}
void QgsDelimitedTextPluginGui::help()
{
  qI->openURL("plugins/delimited_text/index.html",true);
}
