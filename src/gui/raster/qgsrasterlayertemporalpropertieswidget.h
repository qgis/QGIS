/***************************************************************************
                         qgsrasterlayertemporalpropertieswidget.h
                         ------------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERLAYERTEMPORALPROPERTIESWIDGET_H
#define QGSRASTERLAYERTEMPORALPROPERTIESWIDGET_H

#include "ui_qgsrasterlayertemporalpropertieswidgetbase.h"
#include "qgis_gui.h"
#include "qgsrange.h"
#include <QStyledItemDelegate>

class QgsRasterLayer;
class QgsMapLayerConfigWidget;
class QgsExpressionContext;

#ifndef SIP_RUN
///@cond PRIVATE
class QgsRasterBandFixedTemporalRangeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    QgsRasterBandFixedTemporalRangeModel( QObject *parent );
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

    void setLayerData( QgsRasterLayer *layer, const QMap<int, QgsDateTimeRange > &ranges );
    QMap<int, QgsDateTimeRange > rangeData() const { return mRanges; }

  private:

    int mBandCount = 0;
    QMap<int, QString > mBandNames;
    QMap<int, QgsDateTimeRange > mRanges;
};

class QgsFixedTemporalRangeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:

    QgsFixedTemporalRangeDelegate( QObject *parent );

  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

};
///@endcond PRIVATE
#endif

/**
 * \ingroup gui
 * \class QgsRasterLayerTemporalPropertiesWidget
 * \brief A widget for configuring the temporal properties for a raster layer.
 *
 * \since QGIS 3.14
 */

class GUI_EXPORT QgsRasterLayerTemporalPropertiesWidget : public QWidget, private Ui::QgsRasterLayerTemporalPropertiesWidgetBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsRasterLayerTemporalPropertiesWidget.
     */
    QgsRasterLayerTemporalPropertiesWidget( QWidget *parent = nullptr, QgsRasterLayer *layer = nullptr );

    /**
     * Save widget temporal properties inputs.
     */
    void saveTemporalProperties();

    /**
     * Updates the widget state to match the current layer state.
     */
    void syncToLayer();

    /**
     * Adds a child \a widget to the properties widget.
     *
     * \since QGIS 3.20
     */
    void addWidget( QgsMapLayerConfigWidget *widget SIP_TRANSFER );

  private slots:
    void temporalGroupBoxChecked( bool checked );
    void modeChanged();
    void calculateRangeByExpression( bool isUpper );

  private:
    QgsExpressionContext createExpressionContextForBand( int band ) const;

    /**
     * The corresponding map layer with temporal attributes
     */
    QgsRasterLayer *mLayer = nullptr;
    QVBoxLayout *mExtraWidgetLayout = nullptr;

    QList< QgsMapLayerConfigWidget * > mExtraWidgets;

    QgsRasterBandFixedTemporalRangeModel *mFixedRangePerBandModel = nullptr;
    QString mFixedRangeLowerExpression;
    QString mFixedRangeUpperExpression;
};
#endif // QGSRASTERLAYERTEMPORALPROPERTIESWIDGET_H
