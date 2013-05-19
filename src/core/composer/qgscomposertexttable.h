/***************************************************************************
                         qgscomposertexttable.h
                         ----------------------
    begin                : April 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERTEXTTABLE_H
#define QGSCOMPOSERTEXTTABLE_H

#include "qgscomposertable.h"

/**A text table item that reads text from string lists*/
class CORE_EXPORT QgsComposerTextTable: public QgsComposerTable
{
  public:
    QgsComposerTextTable( QgsComposition* c );
    ~QgsComposerTextTable();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const { return ComposerTextTable; }

    void setHeaderLabels( const QStringList& l ) { mHeaderLabels = l; }
    void addRow( const QStringList& row ) { mRowText.append( row ); }

    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

  protected:
    //! @note not available in python bindings
    bool getFeatureAttributes( QList<QgsAttributes>& attributes );
    QMap<int, QString> getHeaderLabels() const;

  private:
    /**Column titles*/
    QStringList mHeaderLabels;
    /**One stringlist per row*/
    QList< QStringList > mRowText;
};

#endif // QGSCOMPOSERTEXTTABLE_H
