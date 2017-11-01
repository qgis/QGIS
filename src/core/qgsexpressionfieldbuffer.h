/***************************************************************************
                          qgsexpressionfieldbuffer.h
                          ---------------------------
    begin                : May 27, 2014
    copyright            : (C) 2014 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONFIELDBUFFER_H
#define QGSEXPRESSIONFIELDBUFFER_H

#include "qgis_core.h"
#include <QString>
#include <QList>
#include <QDomNode>

#include "qgsfields.h"
#include "qgsexpression.h"

/**
 * \ingroup core
 * Buffers information about expression fields for a vector layer.
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsExpressionFieldBuffer
{
  public:
    struct ExpressionField
    {
      ExpressionField( const QString &exp, const QgsField &fld )
        : cachedExpression( exp )
        , field( fld )
      {}

      QgsExpression cachedExpression;
      QgsField field;
    };

    /**
     * Constructor for QgsExpressionFieldBuffer.
     */
    QgsExpressionFieldBuffer() = default;

    /**
     * Add an expression to the buffer
     *
     * \param exp expression to add
     * \param fld field to add
     */
    void addExpression( const QString &exp, const QgsField &fld );

    /**
     * Remove an expression from the buffer
     *
     * \param index index of expression to remove
     */
    void removeExpression( int index );

    /**
     * Renames an expression field at a given index
     *
     * \param index The index of the expression to change
     * \param name   New name for field
     *
     * \since QGIS 3.0
     */
    void renameExpression( int index, const QString &name );

    /**
     * Changes the expression at a given index
     *
     * \param index The index of the expression to change
     * \param exp   The new expression to set
     *
     * \since QGIS 2.9
     */
    void updateExpression( int index, const QString &exp );

    /**
     * Saves expressions to xml under the layer node
     */
    void writeXml( QDomNode &layer_node, QDomDocument &document ) const;

    /**
     * Reads expressions from project file
     */
    void readXml( const QDomNode &layer_node );

    /**
     * Adds fields with the expressions buffered in this object to a QgsFields object
     *
     * \param flds The fields to be updated
     */
    void updateFields( QgsFields &flds );

    QList<QgsExpressionFieldBuffer::ExpressionField> expressions() const { return mExpressions; }

  private:
    QList<ExpressionField> mExpressions;
};

#endif // QGSEXPRESSIONFIELDBUFFER_H
