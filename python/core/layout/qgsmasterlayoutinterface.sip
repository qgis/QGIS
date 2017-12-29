/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsmasterlayoutinterface.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsMasterLayoutInterface
{
%Docstring
 Interface for master layout type objects, such as print layouts and reports.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsmasterlayoutinterface.h"
%End
  public:

    virtual ~QgsMasterLayoutInterface();

    virtual QgsMasterLayoutInterface *clone() const = 0 /Factory/;
%Docstring
Creates a clone of the layout. Ownership of the returned layout
is transferred to the caller.
%End

    virtual QString name() const = 0;
%Docstring
Returns the layout's name.

.. seealso:: :py:func:`setName()`
%End

    virtual QIcon icon() const = 0;
%Docstring
Returns an icon for the layout.
%End

    virtual void setName( const QString &name ) = 0;
%Docstring
Sets the layout's name.

.. seealso:: :py:func:`name()`
%End

    virtual QgsProject *layoutProject() const = 0;
%Docstring
The project associated with the layout. Used to get access to layers, map themes,
relations and various other bits. It is never null.
%End

    virtual QDomElement writeLayoutXml( QDomDocument &document, const QgsReadWriteContext &context ) const = 0;
%Docstring
Returns the layout's state encapsulated in a DOM element.

.. seealso:: :py:func:`readLayoutXml()`
%End

    virtual bool readLayoutXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context ) = 0;
%Docstring
Sets the layout's state from a DOM element. ``layoutElement`` is the DOM node corresponding to the layout.

.. seealso:: :py:func:`writeLayoutXml()`
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsmasterlayoutinterface.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
