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

#include "qgsdockwidget.h"
#include "qgsfeatureiterator.h"
#include "qgstaskmanager.h"
#include "qgsexpressioncontextgenerator.h"

#include "qgis_app.h"

class QMenu;
class QgsBrowserGuiModel;
class QModelIndex;
class QgsDockBrowserTreeView;
class QgsLayerItem;
class QgsDataItem;
class QgsVectorLayer;
class QgsStatisticalSummary;
class QgsStringStatisticalSummary;
class QgsDateTimeStatisticalSummary;

//! Enumeration of supported statistics types
enum DataType
{
  Numeric,  //!< Numeric fields: int, double, etc
  String,  //!< String fields
  DateTime  //!< Date and DateTime fields
};

/**
 * \class QgsStatisticsValueGatherer
* Calculated raster stats for paletted renderer in a thread
*/
class QgsStatisticsValueGatherer : public QgsTask
{
    Q_OBJECT

  public:
    QgsStatisticsValueGatherer(
      QgsVectorLayer *layer,
      const QgsFeatureIterator &fit,
      long featureCount,
      const QString &sourceFieldExp,
      DataType fieldType,
      Qgis::Statistics statsToCalculate,
      Qgis::StringStatistics stringStatsToCalculate,
      Qgis::DateTimeStatistics dateTimeStatsToCalculate
    );
    ~QgsStatisticsValueGatherer() override;

    bool run() override;

    const QgsStatisticalSummary *statsSummary();
    const QgsStringStatisticalSummary *stringStatsSummary();
    const QgsDateTimeStatisticalSummary *dateTimeStatsSummary();

  private:

    QgsFeatureIterator mFeatureIterator;
    long mFeatureCount = 0;
    QString mFieldExpression;
    int mFieldIndex = -1;
    DataType mFieldType;
    Qgis::Statistics mStatsToCalculate;
    Qgis::StringStatistics mStringStatsToCalculate;
    Qgis::DateTimeStatistics mDateTimeStatsToCalculate;

    std::unique_ptr<QgsExpression> mExpression;
    QgsExpressionContext mContext;

    std::unique_ptr< QgsStatisticalSummary > mStatsSummary;
    std::unique_ptr< QgsStringStatisticalSummary > mStringStatsSummary;
    std::unique_ptr< QgsDateTimeStatisticalSummary > mDateTimeStatsSummary;
};

/**
 * A dock widget which displays a statistical summary of the values in a field or expression
 */
class APP_EXPORT QgsStatisticalSummaryDockWidget : public QgsDockWidget, private Ui::QgsStatisticalSummaryWidgetBase, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsStatisticalSummaryDockWidget( QWidget *parent = nullptr );
    ~QgsStatisticalSummaryDockWidget() override;

    /**
     * Returns the currently active layer for the widget
     */
    QgsVectorLayer *layer() const { return mLayer; }

  public slots:

    /**
     * Copy the displayed statistics to the clipboard
     */
    void copyStatistics();

    /**
     * Recalculates the displayed statistics
     */
    void refreshStatistics();

  private slots:

    void layerChanged( QgsMapLayer *layer );
    void fieldChanged();
    void statActionTriggered( bool checked );
    void layersRemoved( const QStringList &layers );
    void layerSelectionChanged();
    void gathererFinished();

  private:

    QgsVectorLayer *mLayer = nullptr;

    QMap< int, QAction * > mStatsActions;
    QMap< QString, QString > mLastExpression;
    QAction *mSyncAction;

    void updateNumericStatistics();
    void updateStringStatistics();
    void updateDateTimeStatistics();
    void addRow( int row, const QString &name, const QString &value, bool showValue );

    QgsExpressionContext createExpressionContext() const override;

    void refreshStatisticsMenu();
    void manageSyncLayer( bool checked );
    DataType fieldType( const QString &fieldName );

    QMenu *mStatisticsMenu = nullptr;
    DataType mFieldType;
    DataType mPreviousFieldType;

    QString mExpression;

    QPointer< QgsStatisticsValueGatherer > mGatherer;

    bool mPendingCalculate = false;
};

#endif // QGSSTATISTICALSUMMARYDOCKWIDGET_H
