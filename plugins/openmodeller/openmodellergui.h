/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   Gyps - Species Distribution Modelling Toolkit                         *
 *   This toolkit provides data transformation and visualisation           *
 *   tools for use in species distribution modelling tools such as GARP,   *
 *   CSM, Bioclim etc.                                                     *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef OPENMODELLERGUI_H
#define OPENMODELLERGUI_H

#include <openmodellerguibase.h>
#include <qstringlist.h>
/**
@author Tim Sutton
*/
class OpenModellerGui : public OpenModellerGuiBase
{
Q_OBJECT
public:
    OpenModellerGui();
    OpenModellerGui( QWidget* parent , const char* name , bool modal , WFlags fl  );
    ~OpenModellerGui();
    
  void getAlgorithmList();
  void OpenModellerGui::parseAndRun(QString theParametersFileNameQString);
  void makeConfigFile();
  
  //
  // The following methods reimplement methods by the same name of the parent class
  //    
    
    void cboModelAlgorithm_activated(  const QString & );
  /** This method overrides the virtual OpenModellerGuiBase method (slot) of the same name. */
  void formSelected(const QString &thePageNameQString);
  void leLocalitiesFileName_textChanged( const QString &theFileNameQString );
  void setSpeciesList(QString theFileNameQSting);
  void pbnSelectOutputFile_clicked();
  void pbnRemoveParameter_clicked();
  void pbnAddParameter_clicked();
  void pbnRemoveLayerFile_clicked();
  void pbnSelectLayerFile_clicked();
  void pbnSelectLocalitiesFile_clicked();
  void leLocalitiesFileName_returnPressed();
  void accept();

private:
    QString modelNameQString;
    QString localitiesFileNameQString;
    QString coordinateSystemQString;
    QString taxonNameQString;
    QStringList layerNamesQStringList;
    QString maskNameQString;
    QStringList extraParametersQStringList;
    QString outputFileNameQString;
    bool useTheseSettingsAgainFlag;
signals:
   void drawRasterLayer(QString);
};

#endif
