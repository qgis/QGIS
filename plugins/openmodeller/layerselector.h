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
#ifndef LAYERSELECTOR_H
#define LAYERSELECTOR_H

//qt includes
#ifdef WIN32
  #include <layerselectorbase.h>
#else
  #include <layerselectorbase.uic.h>
#endif

#include <qwidget.h>
#include <qstringlist.h> 

/**
@author Tim Sutton
*/
class LayerSelector : public LayerSelectorBase
{
Q_OBJECT
public:
    LayerSelector( QString theBaseDir, QWidget* parent , const char* name , bool modal , WFlags fl  );
    ~LayerSelector() {};
    
public slots:
    void pbnDirectorySelector_clicked();
    void pbnOK_clicked();
    void pbnCancel_clicked();
    QStringList getSelectedLayers() {return selectedLayersList;};
	QString getBaseDir() {return baseDirString;};
    static bool isValidGdalFile(const QString theFilename);
    static bool isValidGdalProj(const QString theFilename); 
private:
    
    void traverseDirectories(const QString& dirname, QListViewItem* theListViewItem);
    QListViewItem * listParent; 
    QString baseDirString;
    QStringList selectedLayersList;
signals:
};

#endif
