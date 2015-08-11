/***************************************************************************
                          qgszonalstatisticsplugin.h  -  description
                             -----------------------
    begin                : August 29th, 2009
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

#ifndef QGSZONALSTATISTICSPLUGIN_H
#define QGSZONALSTATISTICSPLUGIN_H

#include "qgisplugin.h"
#include <QObject>

class QgsInterface;
class QAction;

class QgsZonalStatisticsPlugin: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:
    QgsZonalStatisticsPlugin( QgisInterface* iface );
    ~QgsZonalStatisticsPlugin();

    /** Initialize connection to GUI*/
    void initGui() override;
    /** Unload the plugin and cleanup the GUI*/
    void unload() override;

  private slots:
    /** Select input file, output file, format and analysis method*/
    void run();

  private:
    QgisInterface* mIface;
    QAction* mAction;
};

#endif // QGSZONALSTATISTICSPLUGIN_H
