/***************************************************************************
    qgsprofilerpanelwidget.h
    -------------------------
    begin                : May 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROFILERPANELWIDGET_H
#define QGSPROFILERPANELWIDGET_H

#include "qgsdevtoolwidget.h"
#include "ui_qgsprofilerpanelbase.h"
#include <QTreeView>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>

class QgsRuntimeProfiler;

class QgsProfilerProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    QgsProfilerProxyModel( QgsRuntimeProfiler *profiler, QObject *parent );

    void setGroup( const QString &group );

  protected:
    bool filterAcceptsRow( int row, const QModelIndex &source_parent ) const override;

  private:
    QString mGroup;
};

/**
 * \ingroup app
 * \class QgsProfilerPanelWidget
 * \brief A panel widget showing profiled startup times for debugging.
 *
 * \since QGIS 3.14
 */
class QgsProfilerPanelWidget : public QgsDevToolWidget, private Ui::QgsProfilerPanelBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProfilerPanelWidget.
     */
    QgsProfilerPanelWidget( QgsRuntimeProfiler *profiler, QWidget *parent );

  private:
    QgsRuntimeProfiler *mProfiler = nullptr;
    QgsProfilerProxyModel *mProxyModel = nullptr;
};

// adapted from KDAB's "hotspot"

class CostDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:
    explicit CostDelegate( quint32 sortRole, quint32 totalCostRole, QObject *parent = nullptr );
    ~CostDelegate();

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

  private:
    quint32 m_sortRole;
    quint32 m_totalCostRole;
};


#endif // QGSPROFILERPANELWIDGET_H
