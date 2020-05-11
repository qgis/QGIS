/***************************************************************************
    qgsmeshdatasetgroupprovidedwidget.h
    -------------------------------
    begin                : May 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHDATASETGROUPPROVIDEDWIDGET_H
#define QGSMESHDATASETGROUPPROVIDEDWIDGET_H

#include "ui_qgsmeshdatasetgroupprovidedwidgetbase.h"

class QgsMeshLayer;

class APP_EXPORT QgsMeshDatasetGroupProvidedWidget: public QWidget, private Ui::QgsMeshDatasetGroupProvidedWidgetBase
{
    Q_OBJECT
  public:
    //!Constructor
    QgsMeshDatasetGroupProvidedWidget( QWidget *parent = nullptr );

    //! Synchronize widgets state with associated mesh layer
    void syncToLayer( QgsMeshLayer *meshLayer );

    //! Returns group states
    QMap<int, QgsMeshDatasetGroupState> datasetGroupStates() const;

  signals:
    void datasetGroupAdded();

  private slots:
    void addDataset();

  private:
    QgsMeshLayer *mMeshLayer;
};

#endif // QGSMESHDATASETGROUPPROVIDEDWIDGET_H
