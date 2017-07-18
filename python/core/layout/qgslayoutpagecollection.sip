/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutpagecollection.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutPageCollection : QObject
{
%Docstring
 A manager for a collection of pages in a layout.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutpagecollection.h"
%End
  public:

    explicit QgsLayoutPageCollection( QgsLayout *layout /TransferThis/ );
%Docstring
 Constructor for QgsLayoutItemPage, with the specified parent ``layout``.
%End

    QgsLayout *layout() const;
%Docstring
 Returns the layout this collection belongs to.
 :rtype: QgsLayout
%End

    void setPageStyleSymbol( QgsFillSymbol *symbol );
%Docstring
 Sets the ``symbol`` to use for drawing pages in the collection.

 Ownership is not transferred, and a copy of the symbol is created internally.
.. seealso:: pageStyleSymbol()
%End

    const QgsFillSymbol *pageStyleSymbol() const;
%Docstring
 Returns the symbol to use for drawing pages in the collection.
.. seealso:: setPageStyleSymbol()
 :rtype: QgsFillSymbol
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutpagecollection.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
