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

    /**
     * Returns the layout this collection belongs to.
     */
    QgsLayout *layout() const;

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

    void createDefaultPageStyleSymbol();
};

#endif //QGSLAYOUTPAGECOLLECTION_H
