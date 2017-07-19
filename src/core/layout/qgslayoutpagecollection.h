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
#include <QObject>
#include <memory>

class QgsLayout;

/**
 * \ingroup core
 * \class QgsLayoutPageCollection
 * \brief A manager for a collection of pages in a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutPageCollection : public QObject
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemPage, with the specified parent \a layout.
     */
    explicit QgsLayoutPageCollection( QgsLayout *layout SIP_TRANSFERTHIS );

    ~QgsLayoutPageCollection();

    /**
     * Returns the layout this collection belongs to.
     */
    QgsLayout *layout() const;

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
     * Adds a \a page to the collection. Ownership of the \a page is transferred
     * to the collection, and the page will automatically be added to the collection's
     * layout() (there is no need to manually add the page item to the layout).
     * The page will be added after all pages currently contained in the collection.
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
     * \see addPage()
     */
    void insertPage( QgsLayoutItemPage *page SIP_TRANSFER, int beforePage );

    /**
     * Deletes a page from the collection. The page will automatically be removed
     * from the collection's layout().
     *
     * Page numbers in collections begin at 0 - so a \a pageNumber of 0 will delete
     * the first page in the collection.
     */
    void deletePage( int pageNumber );

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

  private:

    QgsLayout *mLayout = nullptr;

    //! Symbol for drawing pages
    std::unique_ptr< QgsFillSymbol > mPageStyleSymbol;

    QList< QgsLayoutItemPage * > mPages;

    void createDefaultPageStyleSymbol();
};

#endif //QGSLAYOUTPAGECOLLECTION_H
