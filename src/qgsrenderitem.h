/***************************************************************************
                         qgsrenderer.h  -  description
                             -------------------
    begin                : Sat Jan 4 2003
    copyright            : (C) 2003 by Gary E.Sherman
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
#ifndef QGSRENDERITEM_H
#define QGSRENDERITEM_H
#include <map>
#include "qgssymbol.h"

class QgsSymbol;
class QString;


/**A renderitem contains a symbol together with the attribute value for which it is valid and a label*/
class QgsRenderItem {
 protected:
    /**Symbol to use in rendering the class*/
    QgsSymbol* mSymbol;
    /**Value of the field*/
    QString mValue;
    /**Label to use when rendering (may be same as value of field)*/
    QString mLabel;
 public:
    /**Default Constructor*/
    QgsRenderItem();
     /** Constructor
    * @param symbol Symbol to use for rendering matching features
    * @param _value Value of the field
    * @param _label Label to use in the legend
    */
    QgsRenderItem(QgsSymbol symbol, QString _value, QString _label);
    /**Destructor*/
    ~QgsRenderItem();
    /** Gets the symbol associated with this render item
     * @return QgsSymbol pointer
     */
    QgsSymbol* getSymbol();
    /** Sets the label for the item
     * @param label the string used as label
     */
    void setLabel(QString label);
    /** Sets the symbol associated with this render item. The symbol is copied.
     * @param s Symbol
     */
    void setSymbol(QgsSymbol s);
    /** Sets a symbol object, which is allocated on the heap. QgsRenderItem automatically deletes it*/
    void setSymbol(QgsSymbol* s);
    void setValue(QString value);
    /**Returns the label*/
    const QString& label() const;
    /**Returns the value of the field*/
    const QString& value() const; 

};

inline QgsSymbol* QgsRenderItem::getSymbol()
{
    return mSymbol;
}

inline const QString& QgsRenderItem::value() const
{
    return mValue;
}

#endif // QGSRENDERITEM_H

