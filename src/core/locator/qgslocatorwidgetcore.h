/***************************************************************************
                         qgslocatorwidgetcore.h
                         ------------------
    begin                : November 2018
    copyright            : (C) 2018 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLOCATORWIDGETCORE_H
#define QGSLOCATORWIDGETCORE_H

#include <QObject>

#include "qgis_core.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"

class QgsLocatorResult;
class QgsLocator;
class QgsLocatorContext;
class QgsLocatorModel;
class QgsLocatorProxyModel;


/**
 * \ingroup core
 * The QgsLocatorWidgetCore class provides the core functionality
 * to be used in a locator widget.
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsLocatorWidgetCore : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool isRunning READ isRunning NOTIFY isRunningChanged )
  public:
    //! Constructor of QgsLocatorWidgetCore
    explicit QgsLocatorWidgetCore( QObject *parent = nullptr );

    //! Perform a search
    Q_INVOKABLE void performSearch( const QString &text );

    //! Returns the locator
    QgsLocator *locator() const;

    //! Returns the proxy model
    Q_INVOKABLE QgsLocatorProxyModel *proxyModel() const;

    //! Returns true if some text to be search is pending in the queue
    bool hasQueueRequested() const;

    //! Returns true if the a search is currently running
    bool isRunning() const;

  signals:
    //! Emitted when a result is added
    void resultAdded();

    //! Emitted when the running status changes
    void isRunningChanged();

    //! Emitted when the results are cleared
    void resultsCleared();

  public slots:
    //! This will invalidate current search results
    void invalidateResults();

    //! Update the canvas extent used to create search context
    void updateCanvasExtent( const QgsRectangle &extent );

    //! Update the canvas CRS used to create search context
    void updateCanvasCrs( const QgsCoordinateReferenceSystem &crs );

  private slots:
    void searchFinished();
    void addResult( const QgsLocatorResult &result );

  private:
    QgsLocatorContext createContext();
    void setIsRunning( bool isRunning );

    QgsLocator *mLocator = nullptr;
    QgsLocatorModel *mLocatorModel = nullptr;
    QgsLocatorProxyModel *mProxyModel = nullptr;

    QString mNextRequestedString;
    bool mHasQueuedRequest = false;
    bool mIsRunning = false;

    // keep track of map canvas extent and CRS
    // if much if needed, it would be possible to add
    // a QgsMapCanvasController in core to achieve this
    // see discussion in https://github.com/qgis/QGIS/pull/8404
    QgsRectangle mCanvasExtent;
    QgsCoordinateReferenceSystem mCanvasCrs;
};

#endif // QGSLOCATORWIDGETCORE_H
