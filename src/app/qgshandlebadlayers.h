/***************************************************************************
                          qgshandlebadlayers.h  -  description
                             -------------------
    begin                : Sat 05 Mar 2011
    copyright            : (C) 2011 by Juergen E. Fischer, norBIT GmbH
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSHANDLEBADLAYERS_H
#define QGSHANDLEBADLAYERS_H

#include "ui_qgshandlebadlayersbase.h"
#include "qgsprojectbadlayerhandler.h"
#include "qgis_app.h"

class APP_EXPORT QgsHandleBadLayersHandler
  : public QObject,
    public QgsProjectBadLayerHandler
{
    Q_OBJECT

  public:
    QgsHandleBadLayersHandler() = default;

    //! Implementation of the handler
    void handleBadLayers( const QList<QDomNode> &layers ) override;

  signals:

    /**
     * Emitted when layers have changed
     * \since QGIS 3.6
     */
    void layersChanged();
};

class QPushButton;

class APP_EXPORT QgsHandleBadLayers
  : public QDialog,
    public Ui::QgsHandleBadLayersBase
{
    Q_OBJECT

  public:
    QgsHandleBadLayers( const QList<QDomNode> &layers );

    int layerCount();

  private slots:
    void selectionChanged();
    void browseClicked();
    void editAuthCfg();
    void apply();
    void accept() override;

    /**
     *  Will search for selected (if any) or all files.
     * Found files will be highlighted in green of approval, otherwise in red.
     * \since QGIS 3.12
     */
    void autoFind();

  private:
    enum class CustomRoles : int
    {
      Index = Qt::UserRole,
      LayerType,
      Provider,
      ProviderIsFileBased,
      DataSourceWasAutoRepaired,
      LayerId,
    };

    QPushButton *mBrowseButton = nullptr;
    QPushButton *mApplyButton = nullptr;
    QPushButton *mAutoFindButton = nullptr;
    const QList<QDomNode> &mLayers;
    // Registry of the original paths associated with a file as a backup
    QHash<QString, QString> mOriginalFileBase;
    // Keeps a registry of valid alternatives for a basepath
    QHash<QString, QStringList> mAlternativeBasepaths;

    QString filename( int row );
    void setFilename( int row, const QString &filename );

    /**
     * Checks if \a newPath for the provided \a layerId is valid.
     * Otherwise all other know viable alternative for the original basepath will be tested.
     * \since QGIS 3.12
     */
    QString checkBasepath( const QString &layerId, const QString &newPath, const QString &fileName, bool &foundPath );

    /**
     * Returns a list of all rows associated with file-based providers.
     *
     * If \a selectedOnly is true then only currently selected rows will be considered.
     */
    QList<int> fileBasedRows( bool selectedOnly );
};

#endif
