/***************************************************************************
    qgsrecentstylehandler.h
    ------------------------
    begin                : September 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRECENTSTYLEHANDLER_H
#define QGSRECENTSTYLEHANDLER_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbol.h"
#include <unordered_map>
#include <memory>

class QgsSymbol;

/**
 * \ingroup core
 * \class QgsRecentStyleHandler
 * \brief Handles and tracks style items recently used in the QGIS GUI.
 *
 * QgsRecentStyleHandler is not usually directly created, but rather accessed through
 * QgsApplication::recentStyleHandler().
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsRecentStyleHandler
{
  public:

    /**
     * Creates a new recent style handler.
     *
    * QgsRecentStyleHandler is not usually directly created, but rather accessed through
    * QgsApplication::recentStyleHandler().
    */
    QgsRecentStyleHandler();

    //! QgsRecentStyleHandler cannot be copied
    QgsRecentStyleHandler( const QgsRecentStyleHandler &other ) = delete;
    //! QgsRecentStyleHandler cannot be copied
    QgsRecentStyleHandler &operator=( const QgsRecentStyleHandler &other ) = delete;

    ~QgsRecentStyleHandler();

    /**
     * Pushes a recently used \a symbol with the specified \a identifier.
     *
     * Ownership of \a symbol is transferred.
     *
     * ### Example
     *
     * \code{.py}
     *   # create a new simple fill symbol
     *   my_fill_symbol = QgsFillSymbol.createSimple( { 'color': '#ff0000' } )
     *
     *   # push this symbol to the recent style handler, using a custom identifier "fill_symbol_for_new_rectangles"
     *   QgsApplication.recentStyleHandler().pushRecentSymbol( 'fill_symbol_for_new_rectangles', my_fill_symbol )
     *
     *   # ... later in the same QGIS session, retrieve a copy of this symbol so that we can use it for a newly created rectangle
     *   new_symbol = QgsApplication.recentStyleHandler().recentSymbol( 'fill_symbol_for_new_rectangles' )
     * \endcode
     *
     * \see recentSymbol()
     */
    void pushRecentSymbol( const QString &identifier, QgsSymbol *symbol SIP_TRANSFER );

    /**
     * Returns a copy of the recently used symbol with the specified \a identifier, or NULLPTR if no symbol
     * with the identifier exists.
     *
     * Caller takes ownership of the returned object.
     *
     * \see pushRecentSymbol()
     */
    QgsSymbol *recentSymbol( const QString &identifier ) const SIP_FACTORY;

    /**
     * Returns a copy of the recently used symbol with the specified \a identifier, or NULLPTR if no symbol
     * with the identifier exists, and casts it to a particular symbol type.
     *
     * \note not available in Python bindings
     * \see pushRecentSymbol()
     */
    template <class SymbolType> std::unique_ptr< SymbolType > recentSymbol( const QString &identifier ) const SIP_SKIP
    {
      std::unique_ptr< QgsSymbol > tmpSymbol( recentSymbol( identifier ) );
      if ( SymbolType *symbolCastToType = dynamic_cast<SymbolType *>( tmpSymbol.get() ) )
      {
        return std::unique_ptr< SymbolType >( dynamic_cast<SymbolType *>( tmpSymbol.release() ) );
      }
      else
      {
        //could not cast
        return nullptr;
      }
    }

  private:

#ifdef SIP_RUN
    QgsRecentStyleHandler( const QgsRecentStyleHandler &other );
#endif

    std::unordered_map< QString, std::unique_ptr< QgsSymbol > > mRecentSymbols;

};

#endif // QGSRECENTSTYLEHANDLER_H
