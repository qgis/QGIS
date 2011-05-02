/***************************************************************************
                              qgsdisplacementplugin.h
                              -----------------------
  begin                : January 26, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDISPLACEMENTPLUGIN_H
#define QGSDISPLACEMENTPLUGIN_H

#include "qgisplugin.h"

class QgisInterface;

/**A plugin that adds a point displacement renderer to the symbol registry*/
class QgsDisplacementPlugin: public QgisPlugin
{
  public:
    QgsDisplacementPlugin( QgisInterface* iface );
    ~QgsDisplacementPlugin();
    /**Adds renderer to the registry*/
    void initGui();
    /**Removes renderer from the registry*/
    void unload();

  private:
    QgisInterface* mIface;
};

#endif // QGSDISPLACEMENTPLUGIN_H
