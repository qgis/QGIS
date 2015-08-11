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
#include "qgsproject.h"

class APP_EXPORT QgsHandleBadLayersHandler
      : public QObject
      , public QgsProjectBadLayerHandler
{
    Q_OBJECT

  public:
    QgsHandleBadLayersHandler();

    /** Implementation of the handler */
    virtual void handleBadLayers( QList<QDomNode> layers, QDomDocument projectDom ) override;
};


class QPushButton;

class APP_EXPORT QgsHandleBadLayers
      : public QDialog
      , private Ui::QgsHandleBadLayersBase
{
    Q_OBJECT

  public:
    QgsHandleBadLayers( const QList<QDomNode> &layers, const QDomDocument &dom );
    ~QgsHandleBadLayers();

    int layerCount();

  private slots:
    void selectionChanged();
    void browseClicked();
    void apply();
    void accept() override;
    void rejected();

  private:
    QPushButton *mBrowseButton;
    const QList<QDomNode> &mLayers;
    QList<int> mRows;
    QString mVectorFileFilter;
    QString mRasterFileFilter;

    QString filename( int row );
    void setFilename( int row, QString filename );
};

#endif
