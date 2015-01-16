/***************************************************************************
    offline_editing.h

    Offline Editing Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 08-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_OFFLINE_EDITING_PLUGIN_H
#define QGS_OFFLINE_EDITING_PLUGIN_H

#include "../qgisplugin.h"
#include <qgsofflineediting.h>
#include <QObject>

class QAction;
class QgisInterface;
class QgsOfflineEditingProgressDialog;

class QgsOfflineEditingPlugin : public QObject, public QgisPlugin
{
    Q_OBJECT

  public:
    QgsOfflineEditingPlugin( QgisInterface* theQgisInterface );
    virtual ~QgsOfflineEditingPlugin();

  public slots:
    //! init the gui
    virtual void initGui() override;
    //! actions
    void convertProject();
    void synchronize();
    //! unload the plugin
    void unload() override;
    //! show the help document
    void help();

  private:
    int mPluginType;
    //! Pointer to the QGIS interface object
    QgisInterface* mQGisIface;
    //!pointer to the qaction for this plugin
    QAction* mActionConvertProject;
    QAction* mActionSynchronize;

    QgsOfflineEditing* mOfflineEditing;
    QgsOfflineEditingProgressDialog* mProgressDialog;

  private slots:
    void updateActions();

    //! update progress dialog
    void showProgress();
    void setLayerProgress( int layer, int numLayers );
    void setProgressMode( QgsOfflineEditing::ProgressMode mode, int maximum );
    void updateProgress( int progress );
    void hideProgress();
};

#endif // QGS_OFFLINE_EDITING_PLUGIN_H
