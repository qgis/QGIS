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
class QMenu;

/**A plugin for raster based terrain analysis (e.g. slope, aspect, ruggedness)*/
class QgsRasterTerrainAnalysisPlugin: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:
    QgsRasterTerrainAnalysisPlugin( QgisInterface* iface );
    ~QgsRasterTerrainAnalysisPlugin();

    /**initialize connection to GUI*/
    void initGui() override;
    /**Unload the plugin and cleanup the GUI*/
    void unload() override;

  private slots:
    void hillshade();
    void relief();
    void slope();
    void aspect();
    void ruggedness();

  private:
    QgisInterface* mIface;
    QMenu* mTerrainAnalysisMenu;
};

#endif // QGSRASTERTERRAINANALYSISPLUGIN_H
