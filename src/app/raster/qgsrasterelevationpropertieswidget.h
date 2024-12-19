/***************************************************************************
    qgsrasterelevationpropertieswidget.h
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

#ifndef QGSRASTERELEVATIONPROPERTIESWIDGET_H
#define QGSRASTERELEVATIONPROPERTIESWIDGET_H

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgsexpressioncontextgenerator.h"
#include "ui_qgsrasterelevationpropertieswidgetbase.h"

#include <QAbstractItemModel>
#include <QStyledItemDelegate>

class QgsRasterLayer;

class QgsRasterBandFixedElevationRangeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    QgsRasterBandFixedElevationRangeModel( QObject *parent );
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

    void setLayerData( QgsRasterLayer *layer, const QMap<int, QgsDoubleRange > &ranges );
    QMap<int, QgsDoubleRange > rangeData() const { return mRanges; }

  private:

    int mBandCount = 0;
    QMap<int, QString > mBandNames;
    QMap<int, QgsDoubleRange > mRanges;
};

class QgsRasterBandDynamicElevationRangeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    QgsRasterBandDynamicElevationRangeModel( QObject *parent );
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    void setLayer( QgsRasterLayer *layer );
    void setLowerExpression( const QString &expression );
    void setUpperExpression( const QString &expression );
  private:

    QPointer< QgsRasterLayer > mLayer;
    QString mLowerExpression;
    QString mUpperExpression;
};

class QgsFixedElevationRangeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:

    QgsFixedElevationRangeDelegate( QObject *parent );

  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

};

class QgsRasterElevationPropertiesWidget : public QgsMapLayerConfigWidget, public QgsExpressionContextGenerator, private Ui::QgsRasterElevationPropertiesWidgetBase
{
    Q_OBJECT
  public:

    QgsRasterElevationPropertiesWidget( QgsRasterLayer *layer, QgsMapCanvas *canvas, QWidget *parent );

    void syncToLayer( QgsMapLayer *layer ) final;
    QgsExpressionContext createExpressionContext() const final;

  public slots:
    void apply() override;

  private slots:

    void modeChanged();
    void onChanged();
    void calculateRangeByExpression( bool isUpper );

  private:

    QgsExpressionContext createExpressionContextForBand( int band ) const;

    QgsRasterLayer *mLayer = nullptr;
    bool mBlockUpdates = false;
    QgsRasterBandFixedElevationRangeModel *mFixedRangePerBandModel = nullptr;
    QgsRasterBandDynamicElevationRangeModel *mDynamicRangePerBandModel = nullptr;
    QString mFixedRangeLowerExpression = QStringLiteral( "@band" );
    QString mFixedRangeUpperExpression = QStringLiteral( "@band" );

};


class QgsRasterElevationPropertiesWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsRasterElevationPropertiesWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsStyleDock() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    QString layerPropertiesPagePositionHint() const override;
};



#endif // QGSRASTERELEVATIONPROPERTIESWIDGET_H
