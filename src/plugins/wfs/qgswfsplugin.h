/***************************************************************************
                              qgswfsplugin.h
                              -------------------
  begin                : July 25, 2006
  copyright            : (C) 2006 by Marco Hugentobler
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

#ifndef QGSWFSPLUGIN_H
#define QGSWFSPLUGIN_H

#include "qgisplugin.h"
#include <QObject>

class QgisInterface;
class QAction;

/**A plugin for adding vector layers with the WFS provider*/
class QgsWFSPlugin: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:
    QgsWFSPlugin( QgisInterface* iface );
    ~QgsWFSPlugin();
    /**initialize connection to GUI*/
    void initGui();
    /**Unload the plugin and cleanup the GUI*/
    void unload();

  private:
    QgisInterface* mIface;
    QAction* mWfsDialogAction;

  private slots:
    void showSourceDialog();
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( QString theThemeName );
};

#endif
