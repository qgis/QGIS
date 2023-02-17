/***************************************************************************
    qgsnetworkloggerpanelwidget.h
    -------------------------
    begin                : March 2020
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
#ifndef QGSNETWORKLOGGERPANELWIDGET_H
#define QGSNETWORKLOGGERPANELWIDGET_H

#include "qgsdevtoolwidget.h"
#include "ui_qgsnetworkloggerpanelbase.h"
#include <QTreeView>

class QgsNetworkLogger;
class QgsNetworkLoggerProxyModel;

/**
 * \ingroup app
 * \class QgsNetworkLoggerTreeView
 * \brief A custom QTreeView subclass for showing logged network requests.
 *
 * \since QGIS 3.14
 */
class QgsNetworkLoggerTreeView: public QTreeView
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsNetworkLoggerTreeView, attached to the specified \a logger.
     */
    QgsNetworkLoggerTreeView( QgsNetworkLogger *logger, QWidget *parent = nullptr );

  public slots:

    /**
     * Sets a filter \a string to apply to request URLs.
     */
    void setFilterString( const QString &string );

    /**
     * Sets whether successful requests should be shown.
     */
    void setShowSuccessful( bool show );

    /**
     * Sets whether timed out requests should be shown.
     */
    void setShowTimeouts( bool show );

    /**
     * Sets whether requests served directly from cache are shown
     */
    void setShowCached( bool show );

  private slots:
    void itemExpanded( const QModelIndex &index );
    void contextMenu( QPoint point );

  private:

    void expandChildren( const QModelIndex &index );
    QMenu *mMenu = nullptr;
    QgsNetworkLogger *mLogger = nullptr;
    QgsNetworkLoggerProxyModel *mProxyModel = nullptr;
    bool mAutoScroll = true;
};


/**
 * \ingroup app
 * \class QgsNetworkLoggerPanelWidget
 * \brief A panel widget showing logged network requests.
 *
 * \since QGIS 3.14
 */
class QgsNetworkLoggerPanelWidget : public QgsDevToolWidget, private Ui::QgsNetworkLoggerPanelBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsNetworkLoggerPanelWidget, linked with the specified \a logger.
     */
    QgsNetworkLoggerPanelWidget( QgsNetworkLogger *logger, QWidget *parent );

  private:

    QgsNetworkLoggerTreeView *mTreeView = nullptr;
    QgsNetworkLogger *mLogger = nullptr;
};


#endif // QGSNETWORKLOGGERPANELWIDGET_H
