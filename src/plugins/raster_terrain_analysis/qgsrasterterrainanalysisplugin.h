/***************************************************************************
                          qgsrasterterrainanalysisplugin.h  -  description
                             -------------------
    begin                : August 6th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERTERRAINANALYSISPLUGIN_H
#define QGSRASTERTERRAINANALYSISPLUGIN_H

#include "qgisplugin.h"
#include <QObject>

class QgsInterface;
class QAction;

/**A plugin for raster based terrain analysis (e.g. slope, aspect, ruggedness)*/
class QgsRasterTerrainAnalysisPlugin: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:
    QgsRasterTerrainAnalysisPlugin( QgisInterface* iface );
    ~QgsRasterTerrainAnalysisPlugin();

    /**initialize connection to GUI*/
    void initGui();
    /**Unload the plugin and cleanup the GUI*/
    void unload();

  private slots:
    /**Select input file, output file, format and analysis method*/
    void run();

  private:
    QgisInterface* mIface;
    QAction* mAction;
};

#endif // QGSRASTERTERRAINANALYSISPLUGIN_H
