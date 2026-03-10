/***************************************************************************
                             qgssymbolconverter.cpp
                             -----------------
    begin                : February 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#include "qgssymbolconverter.h"

QgsAbstractSymbolConverter::~QgsAbstractSymbolConverter() = default;

QgsSymbolConverterContext::QgsSymbolConverterContext( QgsReadWriteContext &rwContext )
  : mRwContext( rwContext )
{}

QgsReadWriteContext &QgsSymbolConverterContext::readWriteContext()
{
  return mRwContext;
}
