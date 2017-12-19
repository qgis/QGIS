/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsabstractlayoutiterator.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsAbstractLayoutIterator
{
%Docstring
*************************************************************************
*
This program is free software; you can redistribute it and/or modify  *
it under the terms of the GNU General Public License as published by  *
the Free Software Foundation; either version 2 of the License, or     *
(at your option) any later version.                                   *
*
**************************************************************************
%End

%TypeHeaderCode
#include "qgsabstractlayoutiterator.h"
%End
  public:

    virtual ~QgsAbstractLayoutIterator();

    virtual bool beginRender() = 0;
%Docstring
Called when rendering begins, before iteration commences. Returns true if successful, false if no iteration
is available or required.

.. seealso:: :py:func:`endRender()`
%End

    virtual bool endRender() = 0;
%Docstring
Ends the render, performing any required cleanup tasks.
%End

    virtual int count() const = 0;
%Docstring
Returns the number of features to iterate over.
%End

    virtual bool next() = 0;
%Docstring
Iterates to next feature, returning false if no more features exist to iterate over.

.. seealso:: :py:func:`previous()`

.. seealso:: :py:func:`last()`

.. seealso:: :py:func:`first()`
%End

    virtual bool previous() = 0;
%Docstring
Iterates to the previous feature, returning false if no previous feature exists.

.. seealso:: :py:func:`next()`

.. seealso:: :py:func:`last()`

.. seealso:: :py:func:`first()`
%End

    virtual bool last() = 0;
%Docstring
Seeks to the last feature, returning false if no feature was found.

.. seealso:: :py:func:`next()`

.. seealso:: :py:func:`previous()`

.. seealso:: :py:func:`first()`
%End

    virtual bool first() = 0;
%Docstring
Seeks to the first feature, returning false if no feature was found.

.. seealso:: :py:func:`next()`

.. seealso:: :py:func:`previous()`

.. seealso:: :py:func:`last()`
%End
};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsabstractlayoutiterator.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
