/***************************************************************************
    qgsoapifcql2textexpressioncompiler.h
    ------------------------------------
    begin                : April 2023
    copyright            : (C) 2023 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOAPIFCQL2TEXTEXPRESSIONCOMPILER_H
#define QGSOAPIFCQL2TEXTEXPRESSIONCOMPILER_H

#include "qgis_core.h"
#include "qgsfields.h"
#include "qgsexpressionnodeimpl.h"

#include "qgsoapifqueryablesrequest.h"

class QgsExpression;
class QgsExpressionNode;

/**
 * Compiles a QgsExpression to a CQL2-Text expression:
 * cf https://docs.ogc.org/DRAFTS/21-065.html
 */
class QgsOapifCql2TextExpressionCompiler
{
  public:
    QgsOapifCql2TextExpressionCompiler(
      const QMap<QString, QgsOapifQueryablesRequest::Queryable> &queryables,
      bool supportsLikeBetweenIn,
      bool supportsCaseI,
      bool supportsBasicSpatialOperators,
      bool invertAxisOrientation
    );

    //! Possible results from expression compilation
    enum Result
    {
      Complete, //!< Expression was successfully compiled and can be completely delegated to provider
      Partial,  //!< Expression was partially compiled, but provider will return extra records and results must be double-checked using QGIS' expression engine
      Fail      //!< Provider cannot handle expression
    };

    /**
     * Compiles an expression and returns the result of the compilation.
     */
    Result compile( const QgsExpression *exp );

    /**
     * Returns the compiled expression string for use by the provider.
     */
    QString result() const { return mResult; }

    /**
     * Returns whether a geometry literal has been used (which might require filter-crs to be emitted)
     */
    bool geometryLiteralUsed() const { return mGeometryLiteralUsed; }

  private:
    Result compileNode( const QgsExpressionNode *node, QString &result );

    Result compileNodeFunction( const QgsExpressionNodeFunction *n, QString &result );

    QString literalValue( const QVariant &value ) const;

    QString quotedIdentifier( const QString &identifier ) const;

    // Input
    const QMap<QString, QgsOapifQueryablesRequest::Queryable> &mQueryables;
    const bool mSupportsLikeBetweenIn;
    const bool mSupportsCaseI;
    const bool mSupportsBasicSpatialOperators;
    const bool mInvertAxisOrientation;

    // Output
    QString mResult;
    bool mGeometryLiteralUsed = false;
};

#endif // QGSOAPIFCQL2TEXTEXPRESSIONCOMPILER_H
