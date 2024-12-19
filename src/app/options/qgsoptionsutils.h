/***************************************************************************
    qgsoptionsutils.h
    -------------------------
    begin                : September 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSOPTIONSUTILS_H
#define QGSOPTIONSUTILS_H

class QgsOptionsWidgetFactory;
#include <memory>

/**
 * \ingroup app
 *
 * \brief Manages lifetime of a QgsOptionsWidgetFactory, automatically
 * registering and unregistering it as required.
 *
 * \since QGIS 3.16
 */
class QgsScopedOptionsWidgetFactory
{
  public:
    QgsScopedOptionsWidgetFactory( std::unique_ptr< QgsOptionsWidgetFactory > &&factory );
    QgsScopedOptionsWidgetFactory( QgsScopedOptionsWidgetFactory &&other );

    ~QgsScopedOptionsWidgetFactory();

    void reset( std::unique_ptr< QgsOptionsWidgetFactory > factory = nullptr );

  private:
    std::unique_ptr< QgsOptionsWidgetFactory > mFactory;
};


#endif // QGSOPTIONSUTILS_H
