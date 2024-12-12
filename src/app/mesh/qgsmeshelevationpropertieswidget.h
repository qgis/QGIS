/***************************************************************************
    qgsmeshelevationpropertieswidget.h
    ---------------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHELEVATIONPROPERTIESWIDGET_H
#define QGSMESHELEVATIONPROPERTIESWIDGET_H

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"

#include <QStyledItemDelegate>

#include "ui_qgsmeshelevationpropertieswidgetbase.h"

class QgsMeshLayer;

class QgsMeshGroupFixedElevationRangeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    QgsMeshGroupFixedElevationRangeModel( QObject *parent );
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

    void setLayerData( QgsMeshLayer *layer, const QMap<int, QgsDoubleRange> &ranges );
    QMap<int, QgsDoubleRange> rangeData() const { return mRanges; }

  private:
    int mGroupCount = 0;
    QMap<int, QString> mGroupNames;
    QMap<int, QgsDoubleRange> mRanges;
};


class QgsMeshFixedElevationRangeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    QgsMeshFixedElevationRangeDelegate( QObject *parent );

  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};

class QgsMeshElevationPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsMeshElevationPropertiesWidgetBase
{
    Q_OBJECT
  public:
    QgsMeshElevationPropertiesWidget( QgsMeshLayer *layer, QgsMapCanvas *canvas, QWidget *parent );

    void syncToLayer( QgsMapLayer *layer ) final;

  public slots:
    void apply() override;

  private slots:
    void modeChanged();
    void onChanged();
    void calculateRangeByExpression( bool isUpper );

  private:
    QgsExpressionContext createExpressionContextForGroup( int group ) const;

    QgsMeshLayer *mLayer = nullptr;
    bool mBlockUpdates = false;
    QgsMeshGroupFixedElevationRangeModel *mFixedRangePerGroupModel = nullptr;
    QString mFixedRangeLowerExpression = QStringLiteral( "@group" );
    QString mFixedRangeUpperExpression = QStringLiteral( "@group" );
};


class QgsMeshElevationPropertiesWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsMeshElevationPropertiesWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsStyleDock() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    QString layerPropertiesPagePositionHint() const override;
};


#endif // QGSMESHELEVATIONPROPERTIESWIDGET_H
