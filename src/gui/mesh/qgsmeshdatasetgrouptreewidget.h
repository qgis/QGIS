/***************************************************************************
    qgsmeshdatasetgrouptreewidget.h
    -------------------------------
    begin                : May 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHDATASETGROUPTREEWIDGET_H
#define QGSMESHDATASETGROUPTREEWIDGET_H

#include "qgis_gui.h"
#include "ui_qgsmeshdatasetgrouptreewidgetbase.h"

#include "qgsmeshdataset.h"

SIP_NO_FILE

class QgsMeshLayer;

class GUI_EXPORT QgsMeshDatasetGroupTreeWidget: public QWidget, private Ui::QgsMeshDatasetGroupTreeWidgetBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsMeshDatasetGroupTreeWidget( QWidget *parent = nullptr );

    //! Synchronizes widgets state with associated mesh layer
    void syncToLayer( QgsMeshLayer *meshLayer );

  public slots:
    //! Apply the dataset group tree item to the layer
    void apply();

  signals:
    void datasetGroupAdded();

  private slots:
    void addDataset();

  private:
    QgsMeshLayer *mMeshLayer;
};

#endif // QGSMESHDATASETGROUPTREEWIDGET_H
