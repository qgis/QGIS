/***************************************************************************
                              qgslayoutpagecollection.h
                             --------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTPAGECOLLECTION_H
#define QGSLAYOUTPAGECOLLECTION_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssymbol.h"
#include "qgslayout.h"
#include "qgslayoutitempage.h"
#include "qgslayoutitem.h"
#include "qgslayoutserializableobject.h"
#include "qgslayoutpoint.h"
#include <QObject>
#include <memory>

class QgsLayout;
class QgsLayoutGuideCollection;

/**
 * \ingroup core
 * \class QgsLayoutPageCollection
 * \brief A manager for a collection of pages in a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutPageCollection : public QObject, public QgsLayoutSerializableObject
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemPage, with the specified parent \a layout.
     */
    explicit QgsLayoutPageCollection( QgsLayout *layout SIP_TRANSFERTHIS );

    ~QgsLayoutPageCollection() override;

    QString stringType() const override { return QStringLiteral( "LayoutPageCollection" ); }
    QgsLayout *layout() override;

    /**
     * Returns a list of pages in the collection.
     * \see page()
     * \see pageCount()
     */
    QList< QgsLayoutItemPage * > pages();

    /**
     * Returns the number of pages in the collection.
     * \see pages()
     */
    int pageCount() const;

    /**
     * Returns a specific page (by \a pageNumber) from the collection.
     * Internal page numbering starts at 0 - so a \a pageNumber of 0
     * corresponds to the first page in the collection.
     * A NULLPTR is returned if an invalid page number is specified.
     * \see pages()
     */
    QgsLayoutItemPage *page( int pageNumber );

    /**
     * Returns a specific page (by \a pageNumber) from the collection.
     * Internal page numbering starts at 0 - so a \a pageNumber of 0
     * corresponds to the first page in the collection.
     * A NULLPTR is returned if an invalid page number is specified.
     * \see pages()
     * \note Not available in Python bindings.
     */
    const QgsLayoutItemPage *page( int pageNumber ) const SIP_SKIP;

    /**
     * Returns the page number for the specified \a page, or -1 if the page
     * is not contained in the collection.
     */
    int pageNumber( QgsLayoutItemPage *page ) const;

    /**
     * Returns a list of the pages which are visible within the specified
     * \a region (in layout coordinates).
     * \see visiblePageNumbers()
     */
    QList< QgsLayoutItemPage * > visiblePages( const QRectF &region ) const;

    /**
     * Returns a list of the page numbers which are visible within the specified
     * \a region (in layout coordinates).
     * \see visiblePages()
     */
    QList< int > visiblePageNumbers( const QRectF &region ) const;

    /**
     * Returns whether a given \a page index is empty, ie, it contains no items except for the background
     * paper item.
     * \see shouldExportPage()
     */
    bool pageIsEmpty( int page ) const;

    /**
     * Returns a list of layout items on the specified \a page index.
     */
    QList< QgsLayoutItem *> itemsOnPage( int page ) const;

    /**
     * Returns layout items of a specific type on a specified \a page.
     * \note not available in Python bindings.
     */
    template<class T> void itemsOnPage( QList<T *> &itemList, int page ) const SIP_SKIP
    {
      itemList.clear();
      const QList<QGraphicsItem *> graphicsItemList = mLayout->items();
      for ( QGraphicsItem *graphicsItem : graphicsItemList )
      {
        T *item = dynamic_cast<T *>( graphicsItem );
        if ( item && item->page() == page )
        {
          itemList.push_back( item );
        }
      }
    }

    /**
     * Returns whether the specified \a page number should be included in exports of the layouts.
     *
     * \warning This will always return TRUE unless the layout is being currently exported -- it cannot
     * be used in advance to determine whether a given page will be exported!
     *
     * \see pageIsEmpty()
     */
    bool shouldExportPage( int page ) const;

    /**
     * Adds a \a page to the collection. Ownership of the \a page is transferred
     * to the collection, and the page will automatically be added to the collection's
     * layout() (there is no need to manually add the page item to the layout).
     * The page will be added after all pages currently contained in the collection.
     *
     * Calling addPage() automatically triggers a reflow() of pages.
     *
     * \see extendByNewPage()
     * \see insertPage()
     */
    void addPage( QgsLayoutItemPage *page SIP_TRANSFER );

    /**
     * Adds a new page to the end of the collection. This page will inherit the
     * same size as the current final page in the collection.
     *
     * The newly created page will be returned.
     *
     * \see addPage()
     * \see insertPage()
     */
    QgsLayoutItemPage *extendByNewPage();

    /**
     * Inserts a \a page into a specific position in the collection.
     *
     * Ownership of the \a page is transferred
     * to the collection, and the page will automatically be added to the collection's
     * layout() (there is no need to manually add the page item to the layout).
     *
     * The page will be added after before the page number specified by \a beforePage.
     * (Page numbers in collections begin at 0 - so a \a beforePage of 0 will insert
     * the page before all existing pages).
     *
     * Calling insertPage() automatically triggers a reflow() of pages.
     *
     * \see addPage()
     */
    void insertPage( QgsLayoutItemPage *page SIP_TRANSFER, int beforePage );

    /**
     * Deletes a page from the collection. The page will automatically be removed
     * from the collection's layout().
     *
     * Page numbers in collections begin at 0 - so a \a pageNumber of 0 will delete
     * the first page in the collection.
     *
     * Calling deletePage() automatically triggers a reflow() of pages.
     *
     * \see clear()
     */
    void deletePage( int pageNumber );

    /**
     * Deletes a page from the collection. The page will automatically be removed
     * from the collection's layout().
     *
     * Calling deletePage() automatically triggers a reflow() of pages.
     *
     * \see clear()
     */
    void deletePage( QgsLayoutItemPage *page );

    /**
     * Removes all pages from the collection.
     * \see deletePage()
     */
    void clear();

    /**
     * Takes a \a page from the collection, returning ownership of the page to the caller.
     */
    QgsLayoutItemPage *takePage( QgsLayoutItemPage *page ) SIP_TRANSFERBACK;

    /**
     * Sets the \a symbol to use for drawing pages in the collection.
     *
     * Ownership is not transferred, and a copy of the symbol is created internally.
     * \see pageStyleSymbol()
     */
    void setPageStyleSymbol( QgsFillSymbol *symbol );

    /**
     * Returns the symbol to use for drawing pages in the collection.
     * \see setPageStyleSymbol()
     */
    const QgsFillSymbol *pageStyleSymbol() const { return mPageStyleSymbol.get(); }

    /**
     * Should be called before changing any page item sizes, and followed by a call to
     * endPageSizeChange(). If page size changes are wrapped in these calls, then items
     * will maintain their same relative position on pages after the page sizes are updated.
     * \see endPageSizeChange()
     */
    void beginPageSizeChange();

    /**
     * Should be called after changing any page item sizes, and preceded by a call to
     * beginPageSizeChange(). If page size changes are wrapped in these calls, then items
     * will maintain their same relative position on pages after the page sizes are updated.
     * \see beginPageSizeChange()
     */
    void endPageSizeChange();

    /**
     * Forces the page collection to reflow the arrangement of pages, e.g. to account
     * for page size/orientation change.
     */
    void reflow();

    /**
     * Returns the maximum width of pages in the collection. The returned value is
     * in layout units.
     *
     * \see maximumPageSize()
     */
    double maximumPageWidth() const;

    /**
     * Returns the maximum size of any page in the collection, by area. The returned value
     * is in layout units.
     *
     * \see maximumPageWidth()
     */
    QSizeF maximumPageSize() const;

    /**
     * Returns TRUE if the layout has uniform page sizes, e.g. all pages are the same size.
     *
     * This method does not consider differing units as non-uniform sizes, only the actual
     * physical size of the pages.
     */
    bool hasUniformPageSizes() const;

    /**
     * Returns the page number corresponding to a \a point in the layout (in layout units).
     *
     * Page numbers in collections begin at 0 - so a page number of 0 indicates the
     * first page.
     *
     * \note This is a relaxed check, which will always return a page number. For instance,
     * it does not consider x coordinates and vertical coordinates before the first page or
     * after the last page will still return the nearest page.
     *
     * \see predictPageNumberForPoint()
     * \see pageAtPoint()
     * \see positionOnPage()
     */
    int pageNumberForPoint( QPointF point ) const;

    /**
     * Returns the theoretical page number corresponding to a \a point in the layout (in layout units),
     * assuming that enough pages exist in the layout to cover that point.
     *
     * If there are insufficient pages currently in the layout, this method will assume that extra
     * "imaginary" pages have been added at the end of the layout until that point is reached. These
     * imaginary pages will inherit the size of the existing final page in the layout.
     *
     * Page numbers in collections begin at 0 - so a page number of 0 indicates the
     * first page.
     *
     * \see pageNumberForPoint()
     * \see pageAtPoint()
     * \see positionOnPage()
     */
    int predictPageNumberForPoint( QPointF point ) const;

    /**
     * Returns the page at a specified \a point (in layout coordinates).
     *
     * If no page exists at \a point, NULLPTR will be returned.
     *
     * \note Unlike pageNumberForPoint(), this method only returns pages which
     * directly intersect with the specified point.
     *
     * \see pageNumberForPoint()
     */
    QgsLayoutItemPage *pageAtPoint( QPointF point ) const;

    /**
     * Converts a \a position on a \a page to an absolute position in layout coordinates.\
     * \see pagePositionToAbsolute()
     */
    QPointF pagePositionToLayoutPosition( int page, const QgsLayoutPoint &position ) const;

    /**
     * Converts a \a position on a \a page to an absolute position in (maintaining the units from the input \a position).
     * \see pagePositionToLayoutPosition()
     */
    QgsLayoutPoint pagePositionToAbsolute( int page, const QgsLayoutPoint &position ) const;

    /**
     * Returns the position within a page of a \a point in the layout (in layout units).
     *
     * \see pageNumberForPoint()
     */
    QPointF positionOnPage( QPointF point ) const;

    /**
     * Returns the space between pages, in layout units.
     */
    double spaceBetweenPages() const;

    /**
     * Returns the size of the page shadow, in layout units.
     */
    double pageShadowWidth() const;

    /**
     * Resizes the layout to a single page which fits the current contents of the layout.
     *
     * Calling this method resets the number of pages to 1, with the size set to the
     * minimum size required to fit all existing layout items. Items will also be
     * repositioned so that the new top-left bounds of the layout is at the point
     * (marginLeft, marginTop). An optional margin can be specified.
     */
    void resizeToContents( const QgsMargins &margins, QgsUnitTypes::LayoutUnit marginUnits );

    /**
     * Stores the collection's state in a DOM element. The \a parentElement should refer to the parent layout's DOM element.
     * \see readXml()
     */
    bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const override;

    /**
     * Sets the collection's state from a DOM element. collectionElement is the DOM node corresponding to the collection.
     * \see writeXml()
     */
    bool readXml( const QDomElement &collectionElement, const QDomDocument &document, const QgsReadWriteContext &context ) override;

    /**
     * Returns a reference to the collection's guide collection, which manages page snap guides.
     */
    QgsLayoutGuideCollection &guides();

    /**
     * Returns a reference to the collection's guide collection, which manages page snap guides.
     */
    SIP_SKIP const QgsLayoutGuideCollection &guides() const;

  public slots:

    /**
     * Triggers a redraw for all pages.
     */
    void redraw();

  signals:

    /**
     * Emitted when pages are added or removed from the collection.
     */
    void changed();

    /**
     * Emitted just before a page is removed from the collection.
     *
     * Page numbers in collections begin at 0 - so a page number of 0 indicates the
     * first page.
     */
    void pageAboutToBeRemoved( int pageNumber );

  private:

    QgsLayout *mLayout = nullptr;

    std::unique_ptr< QgsLayoutGuideCollection > mGuideCollection;

    //! Symbol for drawing pages
    std::unique_ptr< QgsFillSymbol > mPageStyleSymbol;

    QList< QgsLayoutItemPage * > mPages;

    bool mBlockUndoCommands = false;

    QMap< QString, QPair< int, QgsLayoutPoint > > mPreviousItemPositions;

    void createDefaultPageStyleSymbol();

    friend class QgsLayoutPageCollectionUndoCommand;
};

#endif //QGSLAYOUTPAGECOLLECTION_H
