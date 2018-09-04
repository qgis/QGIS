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
#include "qgsfeatureiterator.h"
#include "qgsfeedback.h"
#include "qgsvectorlayerutils.h"
#include "qgstaskmanager.h"
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
class QgsStatisticsValueGatherer : public QgsTask
{
    Q_OBJECT

  public:
    QgsStatisticsValueGatherer( QgsVectorLayer *layer, const QgsFeatureIterator &fit, long featureCount, const QString &sourceFieldExp )
      : QgsTask( tr( "Fetching statistic values" ) )
      , mFeatureIterator( fit )
      , mFeatureCount( featureCount )
      , mFieldExpression( sourceFieldExp )
    {
      mFieldIndex = layer->fields().lookupField( mFieldExpression );
      if ( mFieldIndex == -1 )
      {
        // use expression, already validated
        mExpression.reset( new QgsExpression( mFieldExpression ) );
        mContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
      }
    }

    bool run() override
    {
      QgsFeature f;
      int current = 0;
      while ( mFeatureIterator.nextFeature( f ) )
      {
        if ( mExpression )
        {
          mContext.setFeature( f );
          QVariant v = mExpression->evaluate( &mContext );
          mValues << v;
        }
        else
        {
          mValues << f.attribute( mFieldIndex );
        }

        if ( isCanceled() )
        {
          return false;
        }

        current++;
        if ( mFeatureCount > 0 )
        {
          setProgress( 100.0 * static_cast< double >( current ) / mFeatureCount );
        }
      }
      return true;
    }

    QList<QVariant> values() const { return mValues; }

  private:

    QgsFeatureIterator mFeatureIterator;
    long mFeatureCount = 0;
    QString mFieldExpression;
    int mFieldIndex = -1;
    QList<QVariant> mValues;

    std::unique_ptr<QgsExpression> mExpression;
    QgsExpressionContext mContext;
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
