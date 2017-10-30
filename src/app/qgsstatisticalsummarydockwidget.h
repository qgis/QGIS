/***************************************************************************
    qgsstatisticalsummarydockwidget.h
    ---------------------------------
    begin                : May 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSTATISTICALSUMMARYDOCKWIDGET_H
#define QGSSTATISTICALSUMMARYDOCKWIDGET_H

#include <QMap>
#include "ui_qgsstatisticalsummarybase.h"

#include "qgsstatisticalsummary.h"
#include "qgsstringstatisticalsummary.h"
#include "qgsdatetimestatisticalsummary.h"
#include "qgsdockwidget.h"
#include "qgis_app.h"

class QMenu;
class QgsBrowserModel;
class QModelIndex;
class QgsDockBrowserTreeView;
class QgsLayerItem;
class QgsDataItem;
class QgsBrowserTreeFilterProxyModel;

/**
 * A dock widget which displays a statistical summary of the values in a field or expression
 */
class APP_EXPORT QgsStatisticalSummaryDockWidget : public QgsDockWidget, private Ui::QgsStatisticalSummaryWidgetBase, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsStatisticalSummaryDockWidget( QWidget *parent = nullptr );

    /**
     * Returns the currently active layer for the widget
     * \since QGIS 2.12
     */
    QgsVectorLayer *layer() const { return mLayer; }

  public slots:

    /**
     * Recalculates the displayed statistics
     */
    void refreshStatistics();

  private slots:

    void layerChanged( QgsMapLayer *layer );
    void statActionTriggered( bool checked );
    void layersRemoved( const QStringList &layers );
    void layerSelectionChanged();

  private:

    //! Enumeration of supported statistics types
    enum DataType
    {
      Numeric,  //!< Numeric fields: int, double, etc
      String,  //!< String fields
      DateTime  //!< Date and DateTime fields
    };

    QgsVectorLayer *mLayer = nullptr;

    QMap< int, QAction * > mStatsActions;
    static QList< QgsStatisticalSummary::Statistic > sDisplayStats;
    static QList< QgsStringStatisticalSummary::Statistic > sDisplayStringStats;
    static QList< QgsDateTimeStatisticalSummary::Statistic > sDisplayDateTimeStats;

    void updateNumericStatistics( bool selectedOnly );
    void updateStringStatistics( bool selectedOnly );
    void updateDateTimeStatistics( bool selectedOnly );
    void addRow( int row, const QString &name, const QString &value, bool showValue );

    QgsExpressionContext createExpressionContext() const override;

    void refreshStatisticsMenu();
    DataType fieldType( const QString &fieldName );

    QMenu *mStatisticsMenu = nullptr;
    DataType mFieldType;
    DataType mPreviousFieldType;
};

#endif // QGSSTATISTICALSUMMARYDOCKWIDGET_H
