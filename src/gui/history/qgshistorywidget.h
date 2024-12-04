/***************************************************************************
                             qgshistorywidget.h
                             ------------------
    Date                 : April 2023
    Copyright            : (C) 2023 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHISTORYWIDGET_H
#define QGSHISTORYWIDGET_H

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgshistorywidgetbase.h"
#include "qgspanelwidget.h"
#include "qgshistorywidgetcontext.h"

#include <QSortFilterProxyModel>

class QgsHistoryProviderRegistry;
class QgsHistoryEntryModel;
class QgsMessageBar;

#ifndef SIP_RUN

///@cond PRIVATE
class GUI_EXPORT QgsHistoryEntryProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    QgsHistoryEntryProxyModel( QObject *parent = nullptr );

    void setFilter( const QString &filter );
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:
    QString mFilter;
};
///@endcond PRIVATE
#endif

/**
 * A widget showing entries from a QgsHistoryProviderRegistry.
 * \ingroup gui
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsHistoryWidget : public QgsPanelWidget, private Ui::QgsHistoryWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsHistoryWidget, with the specified \a parent widget.
     *
     * If \a providerId is specified then the widget will contain only items from the matching
     * history provider.
     * If \a backends is specified then the widget will be filtered to only matching backends.
     *
     * If no \a registry is specified then the singleton QgsHistoryProviderRegistry from QgsGui::historyProviderRegistry()
     * will be used.
     */
    QgsHistoryWidget( const QString &providerId = QString(), Qgis::HistoryProviderBackends backends = Qgis::HistoryProviderBackend::LocalProfile, QgsHistoryProviderRegistry *registry = nullptr, const QgsHistoryWidgetContext &context = QgsHistoryWidgetContext(), QWidget *parent = nullptr );

  private slots:

    void currentItemChanged( const QModelIndex &selected, const QModelIndex &previous );
    void nodeDoubleClicked( const QModelIndex &index );
    void showNodeContextMenu( const QPoint &pos );
    void urlClicked( const QUrl &url );

  private:
    QgsHistoryEntryModel *mModel = nullptr;
    QgsHistoryEntryProxyModel *mProxyModel = nullptr;
    QgsHistoryWidgetContext mContext;
};

#endif // QGSHISTORYWIDGET_H
