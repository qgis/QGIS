/***************************************************************************
    qgsqueryloggerpanelwidget.h
    -------------------------
    begin                : October 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSQUERYLOGGERPANELWIDGET_H
#define QGSQUERYLOGGERPANELWIDGET_H

#include "qgsdevtoolwidget.h"
#include "ui_qgsqueryloggerpanelbase.h"
#include <QTreeView>

class QgsAppQueryLogger;
class QgsDatabaseQueryLoggerProxyModel;

/**
 * \ingroup app
 * \class QgsDatabaseQueryLoggerTreeView
 * \brief A custom QTreeView subclass for showing logged database queries.
 */
class QgsDatabaseQueryLoggerTreeView: public QTreeView
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsDatabaseQueryLoggerTreeView, attached to the specified \a logger.
     */
    QgsDatabaseQueryLoggerTreeView( QgsAppQueryLogger *logger, QWidget *parent = nullptr );

  public slots:

    /**
     * Sets a filter \a string to apply to request URLs.
     */
    void setFilterString( const QString &string );

  private slots:
    void itemExpanded( const QModelIndex &index );
    void contextMenu( QPoint point );

  private:

    void expandChildren( const QModelIndex &index );
    QMenu *mMenu = nullptr;
    QgsAppQueryLogger *mLogger = nullptr;
    QgsDatabaseQueryLoggerProxyModel *mProxyModel = nullptr;
    bool mAutoScroll = true;
};


/**
 * \ingroup app
 * \class QgsDatabaseQueryLoggerPanelWidget
 * \brief A panel widget showing logged network requests.
 */
class QgsDatabaseQueryLoggerPanelWidget : public QgsDevToolWidget, private Ui::QgsDatabaseQueryLoggerPanelBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsDatabaseQueryLoggerPanelWidget, linked with the specified \a logger.
     */
    QgsDatabaseQueryLoggerPanelWidget( QgsAppQueryLogger *logger, QWidget *parent );

  private:

    QgsDatabaseQueryLoggerTreeView *mTreeView = nullptr;
    QgsAppQueryLogger *mLogger = nullptr;
};


#endif // QGSQUERYLOGGERPANELWIDGET_H
