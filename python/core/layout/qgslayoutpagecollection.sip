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

    ~QgsLayoutPageCollection();

    QgsLayout *layout() const;
%Docstring
 Returns the layout this collection belongs to.
 :rtype: QgsLayout
%End

    QList< QgsLayoutItemPage * > pages();
%Docstring
 Returns a list of pages in the collection.
.. seealso:: page()
.. seealso:: pageCount()
 :rtype: list of QgsLayoutItemPage
%End

    int pageCount() const;
%Docstring
 Returns the number of pages in the collection.
.. seealso:: pages()
 :rtype: int
%End

    QgsLayoutItemPage *page( int pageNumber );
%Docstring
 Returns a specific page (by ``pageNumber``) from the collection.
 Internal page numbering starts at 0 - so a ``pageNumber`` of 0
 corresponds to the first page in the collection.
 A None is returned if an invalid page number is specified.
.. seealso:: pages()
 :rtype: QgsLayoutItemPage
%End

    void addPage( QgsLayoutItemPage *page /Transfer/ );
%Docstring
 Adds a ``page`` to the collection. Ownership of the ``page`` is transferred
 to the collection, and the page will automatically be added to the collection's
 layout() (there is no need to manually add the page item to the layout).
 The page will be added after all pages currently contained in the collection.

 Calling addPage() automatically triggers a reflow() of pages.

.. seealso:: insertPage()
%End

    void insertPage( QgsLayoutItemPage *page /Transfer/, int beforePage );
%Docstring
 Inserts a ``page`` into a specific position in the collection.

 Ownership of the ``page`` is transferred
 to the collection, and the page will automatically be added to the collection's
 layout() (there is no need to manually add the page item to the layout).

 The page will be added after before the page number specified by ``beforePage``.
 (Page numbers in collections begin at 0 - so a ``beforePage`` of 0 will insert
 the page before all existing pages).

 Calling insertPage() automatically triggers a reflow() of pages.

.. seealso:: addPage()
%End

    void deletePage( int pageNumber );
%Docstring
 Deletes a page from the collection. The page will automatically be removed
 from the collection's layout().

 Page numbers in collections begin at 0 - so a ``pageNumber`` of 0 will delete
 the first page in the collection.

 Calling deletePage() automatically triggers a reflow() of pages.
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

    void reflow();
%Docstring
 Forces the page collection to reflow the arrangement of pages, e.g. to account
 for page size/orientation change.
%End

    double maximumPageWidth() const;
%Docstring
 Returns the maximum width of pages in the collection. The returned value is
 in layout units.
 :rtype: float
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutpagecollection.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
