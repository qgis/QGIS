/***************************************************************************
  labeling.h
  Smart labeling for vector layers
  -------------------
         begin                : June 2009
         copyright            : (C) Martin Dobias
         email                : wonder.sk at gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef Labeling_H
#define Labeling_H

//QT4 includes
#include <QObject>

//QGIS includes
#include "../qgisplugin.h"

#include "qgsmaplayer.h" // for MOC

//forward declarations
class QAction;
class QPainter;
class QToolBar;

class QgisInterface;

class QgsPalLabeling;
class LabelingTool;

class Labeling: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:

    /**
    * Constructor for a plugin. The QgisInterface pointer is passed by
    * QGIS when it attempts to instantiate the plugin.
    * @param theInterface Pointer to the QgisInterface object.
     */
    Labeling( QgisInterface * theInterface );
    //! Destructor
    virtual ~Labeling();

  public slots:
    //! init the gui
    virtual void initGui();
    //! Show the dialog box
    void run();
    //! unload the plugin
    void unload();

    //! start labeling map tool
    void setTool();

  private:

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
    //! Pointer to the qaction for this plugin
    QAction * mQActionPointer;
    QAction * mActionTool;

    QgsPalLabeling* mLBL;

    LabelingTool* mTool;
};

#endif //Labeling_H
