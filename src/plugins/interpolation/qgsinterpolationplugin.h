/***************************************************************************
                              qgsinterpolationplugin.h
                              ------------------------
  begin                : March 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
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

#ifndef QGSINTERPOLATIONPLUGIN_H
#define QGSINTERPOLATIONPLUGIN_H

#include "qgisplugin.h"
#include <QObject>

class QgisInterface;
class QAction;

/**A plugin that does interpolation on vertices of
a vector layer*/
class QgsInterpolationPlugin: public QObject, public QgisPlugin
{
    Q_OBJECT

  public:
    QgsInterpolationPlugin( QgisInterface* iface );
    ~QgsInterpolationPlugin();
    /**initialize connection to GUI*/
    void initGui();
    /**Unload the plugin and cleanup the GUI*/
    void unload();

  public slots:
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( QString theThemeName );

  private slots:
    void showInterpolationDialog();

  private:
    QgisInterface* mIface;
    QAction* mInterpolationAction;
};

#endif
