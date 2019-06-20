/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/locator/qgslocatormodelbridge.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsLocatorModelBridge : QObject
{
%Docstring
The QgsLocatorModelBridge class provides the core functionality
to be used in a locator widget.

.. versionadded:: 3.6
%End

%TypeHeaderCode
#include "qgslocatormodelbridge.h"
%End
  public:
    explicit QgsLocatorModelBridge( QObject *parent = 0 );
%Docstring
Constructor of QgsLocatorModelBridge
%End
    virtual ~QgsLocatorModelBridge();

    void performSearch( const QString &text );
%Docstring
Perform a search
%End

    QgsLocator *locator() const;
%Docstring
Returns the locator
%End

    QgsLocatorProxyModel *proxyModel() const;
%Docstring
Returns the proxy model
%End

    bool hasQueueRequested() const;
%Docstring
Returns ``True`` if some text to be search is pending in the queue
%End

    bool isRunning() const;
%Docstring
Returns ``True`` if the a search is currently running
%End

    void triggerResult( const QModelIndex &index,  const int actionId = -1 );
%Docstring
Triggers the result at given ``index`` and with optional ``actionId`` if an additional action was triggered
%End

  signals:
    void resultAdded();
%Docstring
Emitted when a result is added
%End

    void isRunningChanged();
%Docstring
Emitted when the running status changes
%End

    void resultsCleared();
%Docstring
Emitted when the results are cleared
%End

  public slots:
    void invalidateResults();
%Docstring
This will invalidate current search results
%End

    void updateCanvasExtent( const QgsRectangle &extent );
%Docstring
Update the canvas extent used to create search context
%End

    void updateCanvasCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Update the canvas CRS used to create search context
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/locator/qgslocatormodelbridge.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
