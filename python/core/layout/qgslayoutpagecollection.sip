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
 Constructor for QgsLayoutItemPage, with the specified parent ``layout``.
%End

    ~QgsLayoutPageCollection();

    virtual QString stringType() const;
    virtual QgsLayout *layout();


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

    int pageNumber( QgsLayoutItemPage *page ) const;
%Docstring
 Returns the page number for the specified ``page``, or -1 if the page
 is not contained in the collection.
 :rtype: int
%End

    QList< QgsLayoutItemPage * > visiblePages( QRectF region ) const;
%Docstring
 Returns a list of the pages which are visible within the specified
 ``region`` (in layout coordinates).
.. seealso:: visiblePageNumbers()
 :rtype: list of QgsLayoutItemPage
%End

    QList< int > visiblePageNumbers( QRectF region ) const;
%Docstring
 Returns a list of the page numbers which are visible within the specified
 ``region`` (in layout coordinates).
.. seealso:: visiblePages()
 :rtype: list of int
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

    void deletePage( QgsLayoutItemPage *page );
%Docstring
 Deletes a page from the collection. The page will automatically be removed
 from the collection's layout().

 Calling deletePage() automatically triggers a reflow() of pages.
%End

    QgsLayoutItemPage *takePage( QgsLayoutItemPage *page ) /TransferBack/;
%Docstring
 Takes a ``page`` from the collection, returning ownership of the page to the caller.
 :rtype: QgsLayoutItemPage
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

    int pageNumberForPoint( QPointF point ) const;
%Docstring
 Returns the page number corresponding to a ``point`` in the layout (in layout units).

 Page numbers in collections begin at 0 - so a page number of 0 indicates the
 first page.

.. note::

   This is a relaxed check, which will always return a page number. For instance,
 it does not consider x coordinates and vertical coordinates before the first page or
 after the last page will still return the nearest page.

.. seealso:: pageAtPoint()
.. seealso:: positionOnPage()
 :rtype: int
%End

    QgsLayoutItemPage *pageAtPoint( QPointF point ) const;
%Docstring
 Returns the page at a specified ``point`` (in layout coordinates).

 If no page exists at ``point``, None will be returned.

.. note::

   Unlike pageNumberForPoint(), this method only returns pages which
 directly intersect with the specified point.

.. seealso:: pageNumberForPoint()
 :rtype: QgsLayoutItemPage
%End

    QPointF positionOnPage( QPointF point ) const;
%Docstring
 Returns the position within a page of a ``point`` in the layout (in layout units).

.. seealso:: pageNumberForPoint()
 :rtype: QPointF
%End

    double spaceBetweenPages() const;
%Docstring
 Returns the space between pages, in layout units.
 :rtype: float
%End

    double pageShadowWidth() const;
%Docstring
 Returns the size of the page shadow, in layout units.
 :rtype: float
%End

    virtual bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

%Docstring
 Stores the collection's state in a DOM element. The ``parentElement`` should refer to the parent layout's DOM element.
.. seealso:: readXml()
 :rtype: bool
%End

    virtual bool readXml( const QDomElement &collectionElement, const QDomDocument &document, const QgsReadWriteContext &context );

%Docstring
 Sets the collection's state from a DOM element. collectionElement is the DOM node corresponding to the collection.
.. seealso:: writeXml()
 :rtype: bool
%End

    QgsLayoutGuideCollection &guides();
%Docstring
 Returns a reference to the collection's guide collection, which manages page snap guides.
 :rtype: QgsLayoutGuideCollection
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
