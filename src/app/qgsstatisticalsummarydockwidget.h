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
#include "qgsfeedback.h"
#include "qgsvectorlayer.h"
#include "qgis_app.h"

class QMenu;
class QgsBrowserModel;
class QModelIndex;
class QgsDockBrowserTreeView;
class QgsLayerItem;
class QgsDataItem;
class QgsBrowserTreeFilterProxyModel;


/**
 * \class QgsStatisticsValueGatherer
* Calculated raster stats for paletted renderer in a thread
*/
class QgsStatisticsValueGatherer: public QThread
{
    Q_OBJECT

  public:
    QgsStatisticsValueGatherer( QgsVectorLayer *layer, QString sourceFieldExp, bool selectedOnly )
      : mLayer( layer )
      , mExpression( sourceFieldExp )
      , mSelectedOnly( selectedOnly )
      , mWasCanceled( false )
    {}

    void run() override
    {
      mWasCanceled = false;

      // allow responsive cancelation
      mFeedback = new QgsFeedback();
      connect( mFeedback, &QgsFeedback::progressChanged, this, &QgsStatisticsValueGatherer::progressChanged );

      bool ok;
      mValues = mLayer->getValues( mExpression, ok, mSelectedOnly, mFeedback );
      if ( !ok )
      {
        mWasCanceled = true;
      }

      // be overly cautious - it's *possible* stop() might be called between deleting mFeedback and nulling it
      mFeedbackMutex.lock();
      delete mFeedback;
      mFeedback = nullptr;
      mFeedbackMutex.unlock();

      emit gatheredValues();
    }

    //! Informs the gatherer to immediately stop collecting values
    void stop()
    {
      // be cautious, in case gatherer stops naturally just as we are canceling it and mFeedback gets deleted
      mFeedbackMutex.lock();
      if ( mFeedback )
        mFeedback->cancel();
      mFeedbackMutex.unlock();

      mWasCanceled = true;
    }

    //! Returns true if collection was canceled before completion
    bool wasCanceled() const { return mWasCanceled; }

    QList<QVariant> values() const { return mValues; }

  signals:

    /**
     * Emitted when values have been collected
     */
    void gatheredValues();

  signals:
    //! Internal routines can connect to this signal if they use event loop
    void canceled();

    void progressChanged( double progress );

  private:

    QgsVectorLayer *mLayer = nullptr;
    QString mExpression;
    bool mSelectedOnly = false;
    QList<QVariant> mValues;
    int mMissingValues;
    QgsFeedback *mFeedback = nullptr;
    QMutex mFeedbackMutex;
    bool mWasCanceled;
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
    void fieldChanged();
    void statActionTriggered( bool checked );
    void layersRemoved( const QStringList &layers );
    void layerSelectionChanged();
    void gathererThreadFinished();

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

    void updateNumericStatistics();
    void updateStringStatistics();
    void updateDateTimeStatistics();
    void addRow( int row, const QString &name, const QString &value, bool showValue );

    QgsExpressionContext createExpressionContext() const override;

    void refreshStatisticsMenu();
    DataType fieldType( const QString &fieldName );

    QMenu *mStatisticsMenu = nullptr;
    DataType mFieldType;
    DataType mPreviousFieldType;

    QString mExpression;

    QgsStatisticsValueGatherer *mGatherer = nullptr;
};

#endif // QGSSTATISTICALSUMMARYDOCKWIDGET_H
