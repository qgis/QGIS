
/***************************************************************************
    qgsappdevtoolutils.h
    -------------------------
    begin                : March 2020
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
#ifndef QGSAPPDEVTOOLUTILS_H
#define QGSAPPDEVTOOLUTILS_H

class QgsDevToolWidgetFactory;
#include <memory>

/**
 * \ingroup app
 *
 * \brief Manages lifetime of a QgsDevToolWidgetFactory, automatically
 * registering and unregistering it as required.
 *
 * \since QGIS 3.14
 */
class QgsScopedDevToolWidgetFactory
{
  public:
    QgsScopedDevToolWidgetFactory();
    ~QgsScopedDevToolWidgetFactory();

    void reset( std::unique_ptr<QgsDevToolWidgetFactory> factory = nullptr );

  private:
    std::unique_ptr<QgsDevToolWidgetFactory> mFactory;
};


#endif // QGSAPPDEVTOOLUTILS_H
