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
#include "qgslayoutitempage.h"
#include "qgslayoutserializableobject.h"
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

    ~QgsLayoutPageCollection();

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
     * A nullptr is returned if an invalid page number is specified.
     * \see pages()
     */
    QgsLayoutItemPage *page( int pageNumber );

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
    QList< QgsLayoutItemPage * > visiblePages( QRectF region ) const;

    /**
     * Returns a list of the page numbers which are visible within the specified
     * \a region (in layout coordinates).
     * \see visiblePages()
     */
    QList< int > visiblePageNumbers( QRectF region ) const;

    /**
     * Adds a \a page to the collection. Ownership of the \a page is transferred
     * to the collection, and the page will automatically be added to the collection's
     * layout() (there is no need to manually add the page item to the layout).
     * The page will be added after all pages currently contained in the collection.
     *
     * Calling addPage() automatically triggers a reflow() of pages.
     *
     * \see insertPage()
     */
    void addPage( QgsLayoutItemPage *page SIP_TRANSFER );

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
     */
    void deletePage( int pageNumber );

    /**
     * Deletes a page from the collection. The page will automatically be removed
     * from the collection's layout().
     *
     * Calling deletePage() automatically triggers a reflow() of pages.
     */
    void deletePage( QgsLayoutItemPage *page );

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
     * Forces the page collection to reflow the arrangement of pages, e.g. to account
     * for page size/orientation change.
     */
    void reflow();

    /**
     * Returns the maximum width of pages in the collection. The returned value is
     * in layout units.
     */
    double maximumPageWidth() const;

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
     * \see pageAtPoint()
     * \see positionOnPage()
     */
    int pageNumberForPoint( QPointF point ) const;

    /**
     * Returns the page at a specified \a point (in layout coordinates).
     *
     * If no page exists at \a point, nullptr will be returned.
     *
     * \note Unlike pageNumberForPoint(), this method only returns pages which
     * directly intersect with the specified point.
     *
     * \see pageNumberForPoint()
     */
    QgsLayoutItemPage *pageAtPoint( QPointF point ) const;

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

    void createDefaultPageStyleSymbol();

    friend class QgsLayoutPageCollectionUndoCommand;
};

#endif //QGSLAYOUTPAGECOLLECTION_H
