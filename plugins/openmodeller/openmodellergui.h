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

//qt includes
#include <qstringlist.h>
#include <qstring.h>
#include <qscrollview.h>
#include <qmap.h> 
#include <qvbox.h>
#ifdef WIN32
  #include <openmodellerguibase.h>
  #include "om.hh"
#else
  #include <openmodellerguibase.uic.h>
  #include <openmodeller/om.hh>
#endif
#include "qlogcallback.h"

/**
@author Tim Sutton
*/
class OpenModellerGui : public OpenModellerGuiBase
{
Q_OBJECT
public:
    OpenModellerGui( QWidget* parent , const char* name , bool modal , WFlags fl  );
    ~OpenModellerGui();

    
  //! Callback function passed to oM to keep track of model creation and map projection progress
  static void progressBarCallback( float progress, void *extra_param );
  
    
  void getAlgorithmList();
  void getParameterList( QString theAlgorithmNameQString );
  void parseAndRun(QString theParametersFileNameQString);
  QString makeConfigReport();
  void makeConfigFile();
  int countSelectedSpecies();
  
  
  //
  // The following methods reimplement methods by the same name of the parent class
  //    
    
    void cboModelAlgorithm_activated(  const QString & );
  /** This method overrides the virtual OpenModellerGuiBase method (slot) of the same name. */
  void formSelected(const QString &thePageNameQString);
  void leLocalitiesFileName_textChanged( const QString &theFileNameQString );
  void setSpeciesList(QString theFileNameQSting);
  QString getTaxonFromLocFile(QString theFileNameQString);
  void pbnSelectOutputDirectory_clicked();
  void pbnRemoveLayerFile_clicked();
  void pbnSelectLayerFile_clicked();
  void pbnSelectLocalitiesFile_clicked();
  void leLocalitiesFileName_returnPressed();
  void accept();
  void reject();
  void cboModelAlgorithm_highlighted( const QString &theModelAlgorithm );
  void leOutputFileName_textChanged( const QString &theOutputFileName);
  void leOutputDirectory_textChanged( const QString &theOutputDirectory);
  void pbnDefaultParameters_clicked();
  //void pbnSelectLayerFolder_clicked();
  //void pbnSelectLayerFolderProj_clicked();
  void pbnRemoveLayerFileProj_clicked();
  void pbnSelectLayerFileProj_clicked();
  void pbnCopyLayers_clicked();
  void pbnOtherInputMask_clicked();
  void pbnOtherOutputMask_clicked();
  void pbnOtherOutputFormat_clicked();
  void pbnAllSpecies_clicked();
  void pbnRemoveLocalitiesFiles_clicked();
  void pbnSelectMultipleLocalitiesFiles_clicked();
  void lstTaxa_selectionChanged();
  void radMultipleFiles_toggled( bool theBool);
  void radSingleFile_toggled( bool theBool);

private:
    OpenModeller * mOpenModeller;
    QString modelNameQString;
    QString localitiesFileNameQString;
    QString coordinateSystemQString;
    QString taxonNameQString;
    QStringList layerNamesQStringList;
    QStringList projLayerNamesQStringList;
    QString maskNameQString;
    QString outputMaskNameQString;
    QStringList extraParametersQStringList;
    QString outputFileNameQString;
    QString outputFormatQString;
    bool useTheseSettingsAgainFlag;


    typedef QMap<QString, QWidget *> ParametersMap;
    //setup QMap object and layout for the frame into which the controls will go
    ParametersMap mMap;
    //for storing default settings for alg widgets - key is widget name, val is default
    typedef QMap<QString, QString> DefaultParametersMap;
    DefaultParametersMap mDefaultParametersMap;
    typedef QMap<QString, QWidget *> ParameterLabels;
    ParameterLabels mLabelsMap;
    typedef QMap<QString,QString> ProjectionWKTMap; //wkt = well known text (see gdal/ogr)
    ProjectionWKTMap mProjectionsMap;
    QScrollView * mParametersScrollView ;
    QVBox * mParametersVBox; //will be placed in the above layout
    QFrame * mParametersFrame; //will be placed in the above 
    QGridLayout* mLayout; //will be placed in the above 
    
    
    //Scroll view within the frame
    QScrollView *mScrollView;
    QGridLayout *mScrollViewLayout;
    QWidget *mLayoutWidget;
    void getProjList();
    //convert the image from tif to pseudocolor png (keeps original size)
    void createModelImage(QString theBaseName);
    // Create a smaller version of the image for the html report
    QString createResizedImage(QString theBaseName);

    bool checkLayersMatch();
    QLogCallback * logCallBack;
signals:
   void drawModelImage(QString);
   void drawRasterLayer(QString);
   void modelDone(QString);
};

#endif
