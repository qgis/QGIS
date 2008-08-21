/***************************************************************************
 *Copyright (C) 2008 Paolo L. Scala, Barbara Rita Barricelli, Marco Padula *
 * CNR, Milan Unit (Information Technology),                               *
 * Construction Technologies Institute.\n";                                *
 *                                                                         *
 * email : Paolo L. Scala <scala@itc.cnr.it>                               *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "dxf2shpconvertergui.h"
#include "qgscontexthelp.h"

#include "builder.h"
#include "getInsertions.h"
#include "dxflib/src/dl_dxf.h"

//qt includes
#include <qmessagebox.h>
#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QDir>

dxf2shpConverterGui::dxf2shpConverterGui(QWidget *parent, Qt::WFlags fl):
  QDialog(parent, fl)
{
  setupUi(this);
}

dxf2shpConverterGui::~dxf2shpConverterGui(){}

void dxf2shpConverterGui::on_buttonBox_accepted()
{

  QString inf = nomein->text();
  QString outd = dirout->text();

  if (inf.size() > 1)
  {

    int type = 1;
    bool convtexts;

    if(convertTextCheck->checkState())
      convtexts = true;
    else
      convtexts = false;

    if (zero->isChecked())
      type = 0;

    if (uno->isChecked())
      type = 2;

    if (due->isChecked())
      type = 1;

    InsertRetrClass * insertRetr = new InsertRetrClass();

    DL_Dxf * dxf_inserts = new DL_Dxf();

    if (!dxf_inserts->in((string)(inf.toLatin1()), insertRetr)) { // if file open failed

      cout << "Aborting: The input file could not be opened.\n";

      return ;

    }

    Builder * parserClass = new Builder();
    parserClass->initBuilder((string)(outd.toLatin1()), type, insertRetr->XVals, insertRetr->YVals, insertRetr->Names, 
        insertRetr->countInserts, convtexts);

    cout << "Finished getting insertions. Count: " << insertRetr->countInserts <<"\n";

    DL_Dxf * dxf_Main = new DL_Dxf();

    if (!dxf_Main->in((string)(inf.toLatin1()), parserClass)) { // if file open failed

      cout << "Aborting: The input file could not be opened.\n";

      return ;

    }

    delete insertRetr;
    delete dxf_inserts;
    delete dxf_Main;
    parserClass->print_shpObjects();
    bool textsPresent;
    if(parserClass->ret_textObjectsSize() > 0)
      textsPresent = true;
    else 
      textsPresent = false;


    QString mystring = QString((parserClass->ret_outputshp()).c_str());

    emit(createLayer(mystring));

    if((convertTextCheck->checkState()) && (textsPresent)) {
      mystring =  QString((parserClass->ret_outputtshp()).c_str());
      emit(createLayer(mystring));
    }

    delete parserClass; 
  }
  else
  {
    QMessageBox::information(this, "Warning", "Please select a file to convert");
  }

  accept();
}

void dxf2shpConverterGui::on_buttonBox_rejected()
{
  reject();
}

void dxf2shpConverterGui::on_buttonBox_helpRequested()
{
  QString s = "Fields description:\n"
  "* Input DXF file: path to the DXF file to be converted\n" 
  "* Output Shp file: desired filename of the ShapeFile to be created\n"
  "* Shp output file type: specifies the type of the output shapefile\n"
  "* Export text labels checkbox: if checked, an additional shp points layer will be created, "
  "  and the associated dbf table will contain informations about the \"TEXT\" fields found"
   " in the dxf file, and the text strings themselves\n\n"
  "---\n"
  "Developed by Paolo L. Scala, Barbara Rita Barricelli, Marco Padula\n"
  "CNR, Milan Unit (Information Technology), Construction Technologies Institute.\n"
  "For support send a mail to scala@itc.cnr.it\n";

  QMessageBox::information(this, "Help", s);
}


void dxf2shpConverterGui::on_btnBrowseForFile_clicked()
{
  getInputFileName();
}

void dxf2shpConverterGui::on_btnBrowseOutputDir_clicked()
{
  getOutputDir();
}


void dxf2shpConverterGui::getInputFileName()
{
  QSettings settings;

  QString s = QFileDialog::getOpenFileName(this, tr(
        "Choose a delimited text file to open"), settings.value(
          "/Plugin-DelimitedText/text_path", "./").toString(), "Files DXF (*.dxf)");

  nomein->setText(s);
}

void dxf2shpConverterGui::getOutputDir()
{
  QSettings settings;

  QString s = QFileDialog::getSaveFileName(this, 
      "Choose a filename to save under", "output.shp", "Shapefile (*.shp)");


  dirout->setText(s);
}
