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

#include <QDockWidget>
#include <QMap>
#include "ui_qgsstatisticalsummarybase.h"

#include "qgsstatisticalsummary.h"

class QgsBrowserModel;
class QModelIndex;
class QgsDockBrowserTreeView;
class QgsLayerItem;
class QgsDataItem;
class QgsBrowserTreeFilterProxyModel;

/**A dock widget which displays a statistical summary of the values in a field or expression
 */
class APP_EXPORT QgsStatisticalSummaryDockWidget : public QDockWidget, private Ui::QgsStatisticalSummaryWidgetBase
{
    Q_OBJECT

  public:
    QgsStatisticalSummaryDockWidget( QWidget *parent = 0 );
    ~QgsStatisticalSummaryDockWidget();

  public slots:

    /**Recalculates the displayed statistics
     */
    void refreshStatistics();

  private slots:

    void layerChanged( QgsMapLayer* layer );
    void statActionTriggered( bool checked );

  private:

    QgsVectorLayer* mLayer;

    QMap< int, QAction* > mStatsActions;
    static QList< QgsStatisticalSummary::Statistic > mDisplayStats;
};

#endif // QGSSTATISTICALSUMMARYDOCKWIDGET_H
