/***************************************************************************
                          qgspolygonsymbol.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id */

#ifndef QGSPOLYGONSYMBOL_H
#define QGSPOLYGONSYMBOL_H
#include "qgssymbol.h"
class QString;

/*! \class QgsPolygonSymbol
 * \brief Symbol for displaying polygons
 */
class QgsPolygonSymbol : public QgsSymbol{
 public:
    //! Constructor
    QgsPolygonSymbol();
    //! Destructor
    ~QgsPolygonSymbol();
          /*! Comparison operator
      @return True if symbols are equal
    */
    bool operator==(const QgsPolygonSymbol &r1);
    /*! Assignment operator
     * @param r1 QgsPolygonSymbol to assign from
     */
    QgsPolygonSymbol & operator=(const QgsPolygonSymbol &r1);
 private:
  
};
#endif // QGSPOLYGONSYMBOL_H
