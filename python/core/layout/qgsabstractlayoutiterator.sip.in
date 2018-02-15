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
 An abstract base class for QgsLayout based classes which can be exported by QgsLayoutExporter.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsabstractlayoutiterator.h"
%End
  public:

    virtual ~QgsAbstractLayoutIterator();

    virtual QgsLayout *layout() = 0;
%Docstring
Returns the layout associated with the iterator.
%End

    virtual bool beginRender() = 0;
%Docstring
Called when rendering begins, before iteration commences. Returns true if successful, false if no iteration
is available or required.

.. seealso:: :py:func:`endRender`
%End

    virtual bool endRender() = 0;
%Docstring
Ends the render, performing any required cleanup tasks.
%End

    virtual int count() = 0;
%Docstring
Returns the number of features to iterate over.
%End

    virtual bool next() = 0;
%Docstring
Iterates to next feature, returning false if no more features exist to iterate over.
%End

    virtual QString filePath( const QString &baseFilePath, const QString &extension ) = 0;
%Docstring
Returns the file path for the current feature, based on a
specified base file path and extension.
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsabstractlayoutiterator.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
