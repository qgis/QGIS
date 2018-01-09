/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscustomdrophandler.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsCustomDropHandler : QObject
{
%Docstring
Abstract base class that may be implemented to handle new types of data to be dropped in QGIS.

Implementations have three approaches they can use to handle drops.

1. The simplest approach is to implement handeFileDrop() when they need to handle
dropped files (i.e. with mime type "text/uri-list").

2. Reimplement handleCustomUriDrop() when they want to handle dropped custom
QgsMimeDataUtils.Uri entries, for instance handling dropping custom entries
from the browser tree (with mime type "application/x-vnd.qgis.qgis.uri"). In
this case the implementation's customUriProviderKey() must match the uri
entry's providerKey.

3. Reimplement handleMimeData() to directly handle dropped QMimeData.
Subclasses should take care when overriding this method. When a drop event
occurs, Qt will lock the source application of the drag for the duration
of the drop event handling via handleMimeData() (e.g. dragging files from
explorer to QGIS will lock the explorer window until the drop handling has
been complete). Accordingly handleMimeData() implementations must return
quickly and defer any intensive or slow processing.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscustomdrophandler.h"
%End
  public:

    virtual QString customUriProviderKey() const;
%Docstring
Type of custom URI recognized by the handler. This must match
the URI entry's providerKey in order for handleCustomUriDrop()
to be called.

.. seealso:: :py:func:`handleCustomUriDrop`
%End

    virtual void handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const;
%Docstring
Called from QGIS after a drop event with custom URI known by the handler.

In order for handleCustomUriDrop() to be called, subclasses must
also implement customUriProviderKey() to indicate the providerKey
value which the handler accepts.

.. seealso:: :py:func:`customUriProviderKey`
%End

    virtual void handleMimeData( const QMimeData *data );
%Docstring
Called when the specified mime ``data`` has been dropped onto QGIS.

The base class implementation does nothing.

Subclasses should take care when overriding this method. When a drop event
occurs, Qt will lock the source application of the drag for the duration
of the drop event handling (e.g. dragging files from explorer to QGIS will
lock the explorer window until the drop handling has been complete).

Accordingly, only implementations must be lightweight and return ASAP.
(For instance by copying the relevant parts of ``data`` and then handling
the data after a short timeout).
%End

    virtual bool handleFileDrop( const QString &file );
%Docstring
Called when the specified ``file`` has been dropped onto QGIS. If true
is returned, then the handler has accepted this file and it should not
be further processed (e.g. by other QgsCustomDropHandlers).

The base class implementation does nothing.

This method is not called directly while drop handling is occurring,
so the limitations described in handleMimeData() about returning
quickly do not apply.
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscustomdrophandler.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
