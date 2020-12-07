/***************************************************************************
                          qgsstoredexpressionmanager.h
                             -------------------
    begin                : August 2019
    copyright            : (C) 2019 David Signer
    email                : david at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTOREDEXPRESSIONMANAGER_H
#define QGSSTOREDEXPRESSIONMANAGER_H

#include "qgis_core.h"
#include <QString>
#include <QObject>
#include <QUuid>


#ifdef SIP_RUN
% ModuleHeaderCode
#include <qgsstoredexpressionmanager.h>
% End
#endif

class QDomNode;
class QDomDocument;

/**
 * \ingroup core
 * Stored expression containing name, content (expression text) and a category tag.
 * \since QGIS 3.10
 */
struct CORE_EXPORT QgsStoredExpression
{

  /**
   * Categories of use cases
   * FilterExpression for stored expressions to filter attribute table
   * DefaultValueExpression for stored expressions to use for default values (not yet used)
   */
  enum Category
  {
    FilterExpression        = 1 << 0, //!< Expressions to filter features
    DefaultValueExpression  = 1 << 1,  //!< Expressions to determine default values (not yet used)
    All                     = FilterExpression | DefaultValueExpression
  };

#ifndef SIP_RUN

  /**
   * Constructor for QgsStoredExpression
   */
  QgsStoredExpression() = default;

  /**
   * Create a new QgsStoredExpression with a generated uuid as id
   *
   *  \param name           descriptive name of the expression
   *  \param expression     expression text
   *  \param tag            category of the expression use case - default FilterExpression
   */
  QgsStoredExpression( QString name, QString expression, Category tag = Category::FilterExpression )
    : id( QUuid::createUuid().toString() ),
      name( name ),
      expression( expression ),
      tag( tag )
  {}
#endif

  //! generated uuid used for identification
  QString id;
  //! descriptive name of the expression
  QString name;
  //! expression text
  QString expression;
  //! category of the expression use case
  Category tag;
};

/**
 * \ingroup core
 * Manages stored expressions regarding creation, modification and storing in the project
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsStoredExpressionManager : public QObject
{
    Q_OBJECT

  public:

    /**
    * Constructor for QgsStoredExpressionManager
    */
    QgsStoredExpressionManager() = default;

    /**
    * Adds an expression to the list
    *
    *  \param name              optional name of the expression
    *  \param expression        expression text
    *  \param tag               category of the expression use case - default FilterExpression
    *  \returns generated id
    */
    QString addStoredExpression( const QString &name, const QString &expression, const QgsStoredExpression::Category &tag =  QgsStoredExpression::Category::FilterExpression );

    /**
    * Removes an expression to the list
    *
    *  \param id                id of the expression as identification
    */
    void removeStoredExpression( const QString &id );

    /**
    * Updates an expression by \a id.
    *
    *  \param id                id of the expression as identification
    *  \param name              new name of the expression
    *  \param expression        new expression text
    *  \param tag               new category of the expression use case
    */
    void updateStoredExpression( const QString &id, const QString &name, const QString &expression, const  QgsStoredExpression::Category &tag );

    /**
    * Appends a list of expressions to the existing list
    *
    *  \param storedExpressions list of expressions and the optional name
    */
    void addStoredExpressions( const QList< QgsStoredExpression > &storedExpressions );

    /**
    * Returns the list of named expressions
    *
    *  \param tag               category of the expression use case - default all
    */
    QList< QgsStoredExpression > storedExpressions( const  QgsStoredExpression::Category &tag = QgsStoredExpression::Category::All );


    /**
    * Returns an expression according to the \a id
    *
    *  \param id                id of the expression as identification
    */
    QgsStoredExpression storedExpression( const QString &id ) const;

    /**
    * Returns an expression according to the \a expression text
    *
    *  \param expression        id of the expression as identification
    *  \param tag               category of the expression use case - default all
    */
    QgsStoredExpression findStoredExpressionByExpression( const QString &expression, const  QgsStoredExpression::Category &tag = QgsStoredExpression::Category::All ) const;

    //! Clears list of stored expressions
    void clearStoredExpressions();

    //! Writes the stored expressions out in XML format
    bool writeXml( QDomNode &layerNode ) const;

    //! Reads the  stored expressions in in XML format
    bool readXml( const QDomNode &layerNode );

  signals:

  public slots:

  private:
    QList< QgsStoredExpression > mStoredExpressions;
};

#endif // QGSSTOREDEXPRESSIONMANAGER_H
