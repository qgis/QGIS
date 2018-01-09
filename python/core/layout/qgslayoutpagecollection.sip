/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutpagecollection.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutPageCollection : QObject, QgsLayoutSerializableObject
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
Constructor for :py:class:`QgsLayoutItemPage`, with the specified parent ``layout``.
%End

    ~QgsLayoutPageCollection();

    virtual QString stringType() const;
    virtual QgsLayout *layout();


    QList< QgsLayoutItemPage * > pages();
%Docstring
Returns a list of pages in the collection.

.. seealso:: :py:func:`page`

.. seealso:: :py:func:`pageCount`
%End

    int pageCount() const;
%Docstring
Returns the number of pages in the collection.

.. seealso:: :py:func:`pages`
%End

    QgsLayoutItemPage *page( int pageNumber );
%Docstring
Returns a specific page (by ``pageNumber``) from the collection.
Internal page numbering starts at 0 - so a ``pageNumber`` of 0
corresponds to the first page in the collection.
A None is returned if an invalid page number is specified.

.. seealso:: :py:func:`pages`
%End


    int pageNumber( QgsLayoutItemPage *page ) const;
%Docstring
Returns the page number for the specified ``page``, or -1 if the page
is not contained in the collection.
%End

    QList< QgsLayoutItemPage * > visiblePages( QRectF region ) const;
%Docstring
Returns a list of the pages which are visible within the specified
``region`` (in layout coordinates).

.. seealso:: :py:func:`visiblePageNumbers`
%End

    QList< int > visiblePageNumbers( QRectF region ) const;
%Docstring
Returns a list of the page numbers which are visible within the specified
``region`` (in layout coordinates).

.. seealso:: :py:func:`visiblePages`
%End

    bool pageIsEmpty( int page ) const;
%Docstring
Returns whether a given ``page`` index is empty, ie, it contains no items except for the background
paper item.

.. seealso:: :py:func:`shouldExportPage`
%End

    QList< QgsLayoutItem *> itemsOnPage( int page ) const;
%Docstring
Returns a list of layout items on the specified ``page`` index.
%End


    bool shouldExportPage( int page ) const;
%Docstring
Returns whether the specified ``page`` number should be included in exports of the layouts.

.. seealso:: :py:func:`pageIsEmpty`
%End

    void addPage( QgsLayoutItemPage *page /Transfer/ );
%Docstring
Adds a ``page`` to the collection. Ownership of the ``page`` is transferred
to the collection, and the page will automatically be added to the collection's
layout() (there is no need to manually add the page item to the layout).
The page will be added after all pages currently contained in the collection.

Calling addPage() automatically triggers a reflow() of pages.

.. seealso:: :py:func:`extendByNewPage`

.. seealso:: :py:func:`insertPage`
%End

    QgsLayoutItemPage *extendByNewPage();
%Docstring
Adds a new page to the end of the collection. This page will inherit the
same size as the current final page in the collection.

The newly created page will be returned.

.. seealso:: :py:func:`addPage`

.. seealso:: :py:func:`insertPage`
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

.. seealso:: :py:func:`addPage`
%End

    void deletePage( int pageNumber );
%Docstring
Deletes a page from the collection. The page will automatically be removed
from the collection's layout().

Page numbers in collections begin at 0 - so a ``pageNumber`` of 0 will delete
the first page in the collection.

Calling deletePage() automatically triggers a reflow() of pages.

.. seealso:: :py:func:`clear`
%End

    void deletePage( QgsLayoutItemPage *page );
%Docstring
Deletes a page from the collection. The page will automatically be removed
from the collection's layout().

Calling deletePage() automatically triggers a reflow() of pages.

.. seealso:: :py:func:`clear`
%End

    void clear();
%Docstring
Removes all pages from the collection.

.. seealso:: :py:func:`deletePage`
%End

    QgsLayoutItemPage *takePage( QgsLayoutItemPage *page ) /TransferBack/;
%Docstring
Takes a ``page`` from the collection, returning ownership of the page to the caller.
%End

    void setPageStyleSymbol( QgsFillSymbol *symbol );
%Docstring
Sets the ``symbol`` to use for drawing pages in the collection.

Ownership is not transferred, and a copy of the symbol is created internally.

.. seealso:: :py:func:`pageStyleSymbol`
%End

    const QgsFillSymbol *pageStyleSymbol() const;
%Docstring
Returns the symbol to use for drawing pages in the collection.

.. seealso:: :py:func:`setPageStyleSymbol`
%End

    void beginPageSizeChange();
%Docstring
Should be called before changing any page item sizes, and followed by a call to
endPageSizeChange(). If page size changes are wrapped in these calls, then items
will maintain their same relative position on pages after the page sizes are updated.

.. seealso:: :py:func:`endPageSizeChange`
%End

    void endPageSizeChange();
%Docstring
Should be called after changing any page item sizes, and preceded by a call to
beginPageSizeChange(). If page size changes are wrapped in these calls, then items
will maintain their same relative position on pages after the page sizes are updated.

.. seealso:: :py:func:`beginPageSizeChange`
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

.. seealso:: :py:func:`maximumPageSize`
%End

    QSizeF maximumPageSize() const;
%Docstring
Returns the maximum size of any page in the collection, by area. The returned value
is in layout units.

.. seealso:: :py:func:`maximumPageWidth`
%End

    bool hasUniformPageSizes() const;
%Docstring
Returns true if the layout has uniform page sizes, e.g. all pages are the same size.

This method does not consider differing units as non-uniform sizes, only the actual
physical size of the pages.
%End

    int pageNumberForPoint( QPointF point ) const;
%Docstring
Returns the page number corresponding to a ``point`` in the layout (in layout units).

Page numbers in collections begin at 0 - so a page number of 0 indicates the
first page.

.. note::

   This is a relaxed check, which will always return a page number. For instance,
it does not consider x coordinates and vertical coordinates before the first page or
after the last page will still return the nearest page.

.. seealso:: :py:func:`predicatePageNumberForPoint`

.. seealso:: :py:func:`pageAtPoint`

.. seealso:: :py:func:`positionOnPage`
%End

    int predictPageNumberForPoint( QPointF point ) const;
%Docstring
Returns the theoretical page number corresponding to a ``point`` in the layout (in layout units),
assuming that enough pages exist in the layout to cover that point.

If there are insufficient pages currently in the layout, this method will assume that extra
"imaginary" pages have been added at the end of the layout until that point is reached. These
imaginary pages will inherit the size of the existing final page in the layout.

Page numbers in collections begin at 0 - so a page number of 0 indicates the
first page.

.. seealso:: :py:func:`pageNumberForPoint`

.. seealso:: :py:func:`pageAtPoint`

.. seealso:: :py:func:`positionOnPage`
%End

    QgsLayoutItemPage *pageAtPoint( QPointF point ) const;
%Docstring
Returns the page at a specified ``point`` (in layout coordinates).

If no page exists at ``point``, None will be returned.

.. note::

   Unlike pageNumberForPoint(), this method only returns pages which
directly intersect with the specified point.

.. seealso:: :py:func:`pageNumberForPoint`
%End

    QPointF pagePositionToLayoutPosition( int page, const QgsLayoutPoint &position ) const;
%Docstring
Converts a ``position`` on a ``page`` to an absolute position in layout coordinates.\

.. seealso:: :py:func:`pagePositionToAbsolute`
%End

    QgsLayoutPoint pagePositionToAbsolute( int page, const QgsLayoutPoint &position ) const;
%Docstring
Converts a ``position`` on a ``page`` to an absolute position in (maintaining the units from the input ``position``).

.. seealso:: :py:func:`pagePositionToLayoutPosition`
%End

    QPointF positionOnPage( QPointF point ) const;
%Docstring
Returns the position within a page of a ``point`` in the layout (in layout units).

.. seealso:: :py:func:`pageNumberForPoint`
%End

    double spaceBetweenPages() const;
%Docstring
Returns the space between pages, in layout units.
%End

    double pageShadowWidth() const;
%Docstring
Returns the size of the page shadow, in layout units.
%End

    void resizeToContents( const QgsMargins &margins, QgsUnitTypes::LayoutUnit marginUnits );
%Docstring
Resizes the layout to a single page which fits the current contents of the layout.

Calling this method resets the number of pages to 1, with the size set to the
minimum size required to fit all existing layout items. Items will also be
repositioned so that the new top-left bounds of the layout is at the point
(marginLeft, marginTop). An optional margin can be specified.
%End

    virtual bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

%Docstring
Stores the collection's state in a DOM element. The ``parentElement`` should refer to the parent layout's DOM element.

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &collectionElement, const QDomDocument &document, const QgsReadWriteContext &context );

%Docstring
Sets the collection's state from a DOM element. collectionElement is the DOM node corresponding to the collection.

.. seealso:: :py:func:`writeXml`
%End

    QgsLayoutGuideCollection &guides();
%Docstring
Returns a reference to the collection's guide collection, which manages page snap guides.
%End


  public slots:

    void redraw();
%Docstring
Triggers a redraw for all pages.
%End

  signals:

    void changed();
%Docstring
Emitted when pages are added or removed from the collection.
%End

    void pageAboutToBeRemoved( int pageNumber );
%Docstring
Emitted just before a page is removed from the collection.

Page numbers in collections begin at 0 - so a page number of 0 indicates the
first page.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutpagecollection.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
