/***************************************************************************
    qgsmeshstaticdatasetwidget.h
    -------------------------------------
    begin                : March 2020
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
#ifndef QGSMESHSTATICDATASETWIDGET_H
#define QGSMESHSTATICDATASETWIDGET_H

#include <QAbstractListModel>

#include "qgis_gui.h"
#include "ui_qgsmeshstaticdatasetwidgetbase.h"
#include "qgsmeshdataset.h"

SIP_NO_FILE

class QgsMeshLayer;
class QgsMeshDataProvider;

/**
 * \ingroup gui
 * \class QgsMeshDatasetListModel
 *
 * \brief List model for dataset contained in dataset group,
 * used to display by time dataset in widget
 */
class QgsMeshDatasetListModel: public QAbstractListModel
{
    Q_OBJECT
  public:
    //! Constructor
    QgsMeshDatasetListModel( QObject *parent );

    //! Sets the layer
    void setMeshLayer( QgsMeshLayer *layer );
    //! Sets the dataset group
    void setDatasetGroup( int group );

    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;

  private:
    QgsMeshLayer *mLayer = nullptr;
    int mDatasetGroup = -1;
};

/**
 * \ingroup gui
 * \class QgsMeshStaticDatasetWidget
 *
 * \brief A widget for setup of the static dataset of a mesh layer.
 */
class GUI_EXPORT QgsMeshStaticDatasetWidget  : public QWidget, private Ui::QgsMeshStaticDatasetWidget
{
    Q_OBJECT
  public:
    //! Constructor
    QgsMeshStaticDatasetWidget( QWidget *parent = nullptr );

    //! Sets the layer
    void setLayer( QgsMeshLayer *layer );

    //! Synchronizes widgets state with associated mesh layer
    void syncToLayer();

    //! Applies the settings made in the widget
    void apply();

  public slots:
    //! Sets the scalar dataset group
    void setScalarDatasetGroup( int index );
    //! Sets the vector dataset group
    void setVectorDatasetGroup( int index );

  private:
    void setScalarDatasetIndex( int index );
    void setVectorDatasetIndex( int index );

    int mScalarDatasetGroup = -1;
    int mVectorDatasetGroup = -1;

    QgsMeshDatasetListModel *mDatasetScalarModel = nullptr;
    QgsMeshDatasetListModel *mDatasetVectorModel = nullptr;

    QgsMeshLayer *mLayer;
};

#endif // QGSMESHSTATICDATASETWIDGET_H
