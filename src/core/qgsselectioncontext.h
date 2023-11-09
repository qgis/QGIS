/***************************************************************************
                              qgsselectioncontext.h
                             --------------------------
    begin                : June 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#ifndef QGSSELECTIONCONTEXT_H
#define QGSSELECTIONCONTEXT_H

#include "qgis_core.h"

/**
 * \class QgsSelectionContext
 * \ingroup core
 *
 * \brief Encapsulates the context of a layer selection operation.
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsSelectionContext
{
  public:

    /**
     * Returns the map scale at which the selection should occur.
     *
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     *
     * \see setScale()
     */
    double scale() const;

    /**
     * Sets the map \a scale at which the selection should occur.
     *
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     *
     * \see scale()
     */
    void setScale( double scale );

  private:

    double mScale = 0;
};

#endif //QGSELECTIONCONTEXT_H



