/***************************************************************************
                          qgssymbol.h  -  description
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

#ifndef QGSSYMBOL_H
#define QGSSYMBOL_H
#include <qcolor.h>
class QString;
class QColor;

/*! \class QgsSymbol
 * \brief Base class for symbols used in rendering map layers.
 */
class QgsSymbol{
 public:
    //! Constructor
    QgsSymbol(QColor c = QColor(0,0,0));
    QgsSymbol(const QgsSymbol &sym);
    //! Set the color
    void setColor(QColor c);
    //! Get the current color
    QColor color() const;
    //! Get the fill color
    QColor fillColor() const;
    void setFillColor(QColor c);
    int lineWidth() const;
    void setLineWidth(int w);
    //! Destructor
    ~QgsSymbol();
          /*! Comparison operator
      @return True if symbols are equal
    */
    bool operator==(const QgsSymbol &r1);
    /*! Assignment operator
     * @param r1 QgsSymbol to assign from
     */
    QgsSymbol & operator=(const QgsSymbol &r1);
 private:
    QColor m_color;
    QColor m_fillColor;
    int m_lineWidth;
};
#endif // QGSSYMBOL_H
