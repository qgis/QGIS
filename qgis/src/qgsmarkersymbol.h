/***************************************************************************
                          qgsmarkersymbol.h  -  description
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

#ifndef QGSMARKERSYMBOL_H
#define QGSMARKERSYMBOL_H
#include "qgssymbol.h"
class QString;

/*! \class QgsMarkerSymbol
 * \brief Symbol for displaying markers (points)
 */
class QgsMarkerSymbol : public QgsSymbol{
 public:
    //! Constructor
    QgsMarkerSymbol();
    //! Destructor
    ~QgsMarkerSymbol();
          /*! Comparison operator
      @return True if symbols are equal
    */
    bool operator==(const QgsMarkerSymbol &r1);
    /*! Assignment operator
     * @param r1 QgsMarkerSymbol to assign from
     */
    QgsMarkerSymbol & operator=(const QgsMarkerSymbol &r1);
 private:
  
};
#endif // QGSMARKERSYMBOL_H
