/***************************************************************************
                         qgslocatormodelbridge.h
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

#ifndef QGSLOCATORMODELBRIDGE_H
#define QGSLOCATORMODELBRIDGE_H

#include <QObject>

#include "qgis_core.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsrectangle.h"

class QItemSelection;

class QgsLocatorResult;
class QgsLocator;
class QgsLocatorContext;
class QgsLocatorModel;
class QgsLocatorProxyModel;


/**
 * \ingroup core
 * \brief The QgsLocatorModelBridge class provides the core functionality
 * to be used in a locator widget.
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsLocatorModelBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool isRunning READ isRunning NOTIFY isRunningChanged )
  public:
    //! Constructor of QgsLocatorModelBridge
    explicit QgsLocatorModelBridge( QObject *parent = nullptr );
    virtual ~QgsLocatorModelBridge() = default;

    //! Perform a search
    Q_INVOKABLE void performSearch( const QString &text );

    //! Returns the locator
    QgsLocator *locator() const;

    //! Returns the proxy model
    Q_INVOKABLE QgsLocatorProxyModel *proxyModel() const;

    //! Returns TRUE if some text to be search is pending in the queue
    bool hasQueueRequested() const;

    //! Returns TRUE if the a search is currently running
    bool isRunning() const;

    //! Triggers the result at given \a index and with optional \a actionId if an additional action was triggered
    void triggerResult( const QModelIndex &index,  const int actionId = -1 );

    /**
     * Returns the coordinate transform context, which should be used whenever the
     * locator constructs a coordinate transform.
     *
     * \see setTransformContext()
     * \since QGIS 3.18
     */
    QgsCoordinateTransformContext transformContext() const { return mTransformContext; }

    /**
     * Sets the coordinate transform \a context, which should be used whenever the
     * locator constructs a coordinate transform.
     *
     * \see transformContext()
     * \since QGIS 3.18
     */
    void setTransformContext( const QgsCoordinateTransformContext &context ) { mTransformContext = context; }

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

    /**
     * This will call filters implementation of selection/deselection of results
     * \since QGIS 3.40
     */
    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

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
    QgsCoordinateTransformContext mTransformContext;
};

#endif // QGSLOCATORMODELBRIDGE_H
