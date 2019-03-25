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
  : public QObject
  , public QgsProjectBadLayerHandler
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
  : public QDialog
  , public Ui::QgsHandleBadLayersBase
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

  private:
    QPushButton *mBrowseButton = nullptr;
    QPushButton *mApplyButton = nullptr;
    const QList<QDomNode> &mLayers;
    QList<int> mRows;
    QString mVectorFileFilter;
    QString mRasterFileFilter;
    QHash <QString, QList<QString> > mFileBase;

    QString filename( int row );
    void setFilename( int row, const QString &filename );
};

#endif
