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

    virtual void handleDrop( const QgsMimeDataUtils::Uri &uri ) const = 0;
%Docstring
Method called from QGIS after a drop event with custom URI known by the handler
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscustomdrophandler.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
