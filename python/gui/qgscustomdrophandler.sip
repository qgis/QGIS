/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscustomdrophandler.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsCustomDropHandler
{
%Docstring
 Abstract base class that may be implemented to handle new types of data to be dropped in QGIS.
 Implementations will be used when a QgsMimeDataUtils.Uri has layerType equal to "custom",
 and the providerKey is equal to key() returned by the implementation.

 Alternatively, implementations can override the handleMimeData() or handleFileDrop()
 methods to handle QMimeData and file drops directly. Reimplementation of these methods
 does not rely on key() matching.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscustomdrophandler.h"
%End
  public:
    virtual ~QgsCustomDropHandler();

    virtual QString key() const = 0;
%Docstring
Type of custom URI recognized by the handler
 :rtype: str
%End

    virtual void handleDrop( const QgsMimeDataUtils::Uri &uri ) const;
%Docstring
Method called from QGIS after a drop event with custom URI known by the handler
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
 :rtype: bool
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscustomdrophandler.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
