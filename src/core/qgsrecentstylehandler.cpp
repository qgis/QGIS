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

#include "qgsrecentstylehandler.h"
#include "qgssymbol.h"

QgsRecentStyleHandler::QgsRecentStyleHandler() = default;

QgsRecentStyleHandler::~QgsRecentStyleHandler() = default;


void QgsRecentStyleHandler::pushRecentSymbol( const QString &identifier, QgsSymbol *symbol )
{
  mRecentSymbols[ identifier ] = std::unique_ptr< QgsSymbol >( symbol );
}

QgsSymbol *QgsRecentStyleHandler::recentSymbol( const QString &identifier ) const
{
  auto it = mRecentSymbols.find( identifier );
  if ( it != mRecentSymbols.end() )
    return it->second->clone();
  else
    return nullptr;
}
