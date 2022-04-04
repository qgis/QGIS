/***************************************************************************
  qgsvectortilebasiclabelingwidget.h
  --------------------------------------
  Date                 : May 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILEBASICLABELINGWIDGET_H
#define QGSVECTORTILEBASICLABELINGWIDGET_H

#include "qgsmaplayerconfigwidget.h"

#include "ui_qgsvectortilebasiclabelingwidget.h"

#include "qgswkbtypes.h"

#include <memory>
#include <QSortFilterProxyModel>

///@cond PRIVATE
#define SIP_NO_FILE

class QgsVectorTileBasicLabeling;
class QgsVectorTileBasicLabelingListModel;
class QgsVectorTileLayer;
class QgsMapCanvas;
class QgsMessageBar;
class QgsVectorTileBasicLabelingProxyModel;

/**
 * \ingroup gui
 * \brief Styling widget for basic labling of vector tile layer
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsVectorTileBasicLabelingWidget : public QgsMapLayerConfigWidget, private Ui::QgsVectorTileBasicLabelingWidget
{
    Q_OBJECT
  public:
    QgsVectorTileBasicLabelingWidget( QgsVectorTileLayer *layer, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent = nullptr );
    ~QgsVectorTileBasicLabelingWidget() override;

    void setLayer( QgsVectorTileLayer *layer );

  public slots:
    //! Applies the settings made in the dialog
    void apply() override;

  private slots:
    void addStyle( QgsWkbTypes::GeometryType geomType );
    //void addStyle();
    void editStyle();
    void editStyleAtIndex( const QModelIndex &index );
    void removeStyle();

    void updateLabelingFromWidget();

  private:
    QPointer< QgsVectorTileLayer > mVTLayer;
    std::unique_ptr<QgsVectorTileBasicLabeling> mLabeling;
    QgsVectorTileBasicLabelingListModel *mModel = nullptr;
    QgsVectorTileBasicLabelingProxyModel *mProxyModel = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
};


class QgsPalLayerSettings;
class QgsVectorLayer;
class QgsSymbolWidgetContext;
class QgsLabelingGui;

/**
 * \ingroup gui
 * \brief Helper widget class that wraps QgsLabelingGui into a QgsPanelWidget
 *
 * \since QGIS 3.14
 */
class QgsLabelingPanelWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:
    QgsLabelingPanelWidget( const QgsPalLayerSettings &labelSettings, QgsVectorLayer *vectorLayer, QgsMapCanvas *mapCanvas, QWidget *parent = nullptr );

    void setDockMode( bool dockMode ) override;

    void setContext( const QgsSymbolWidgetContext &context );
    QgsPalLayerSettings labelSettings();

  private:
    QgsLabelingGui *mLabelingGui = nullptr;
};


class QgsVectorTileBasicLabelingStyle;

class QgsVectorTileBasicLabelingListModel : public QAbstractListModel
{
    Q_OBJECT
  public:

    enum Role
    {
      MinZoom = Qt::UserRole + 1,
      MaxZoom,
      Label,
      Layer,
      Filter
    };

    QgsVectorTileBasicLabelingListModel( QgsVectorTileBasicLabeling *r, QObject *parent = nullptr );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    void insertStyle( int row, const QgsVectorTileBasicLabelingStyle &style );

    // drag'n'drop support
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

  private:
    QgsVectorTileBasicLabeling *mLabeling = nullptr;
};

class QgsVectorTileBasicLabelingProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    QgsVectorTileBasicLabelingProxyModel( QgsVectorTileBasicLabelingListModel *source, QObject *parent = nullptr );

    void setCurrentZoom( int zoom );
    void setFilterVisible( bool enabled );
    void setFilterString( const QString &string );

    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:

    bool mFilterVisible = false;
    QString mFilterString;
    int mCurrentZoom = -1;
};


///@endcond

#endif // QGSVECTORTILEBASICLABELINGWIDGET_H
