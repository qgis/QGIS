/***************************************************************************
                          qgsstoredexpressionmanager.h

 These classes store and control the management and execution of actions
 associated with a particular Qgis layer. Actions are defined to be
 external programs that are run with user-specified inputs that can
 depend on the contents of layer attributes.

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

struct CORE_EXPORT QgsStoredExpression
{

  /* leads to trouble with sip...
  QgsStoredExpression() = default;

  QgsStoredExpression( QString name, QString expression )
    : id( QUuid::createUuid() ),
      name( name ),
      expression( expression )
  {}
  */

  QUuid id;
  QString name;
  QString expression;
};

class CORE_EXPORT QgsStoredExpressionManager : public QObject
{
    Q_OBJECT

  public:

    /**
     * Mode for different use cases
     * FilterExpression for stored expressions to filter attribute table
     * DefaultValueExpression for stored expressions used for default values could be an extension
     */
    enum Mode
    {
      FilterExpression //!< Expressions to filter features
    };

    QgsStoredExpressionManager( Mode mode = FilterExpression )
      : mMode( mode )
    {}

    /**
    * Adds an expression to the list
    *
    *  \param name              optional name of the expression
    *  \param expression        the expression content
    *  \param tag               some content, maybe scope where to be shown
    *  \returns generated id as QUuid
    */
    QUuid addStoredExpression( const QString &name, const QString &expression, const QString &tag = QString() );

    /**
    * Removes an expression to the list
    *
    *  \param id                id of the expression as identification
    *  \param tag               some content, maybe scope where to be shown
    */
    void removeStoredExpression( const QUuid &id, const QString &tag = QString() );

    /**
    * Updates an expression by id
    *
    *  \param id                id of the expression as identification
    *  \param name              new name of the expression
    *  \param expression        new expression content
    *  \param tag               some content, maybe scope where to be shown
    */
    void updateStoredExpression( const QUuid &id, const QString &name, const QString &expression, const QString &tag = QString() );

    /**
    * Appends a list of expressions to the existing list
    *
    *  \param storedExpressions list of expressions and the optional name
    *  \param tag               some content, maybe scope where to be shown
    */
    void addStoredExpressions( QList< QgsStoredExpression > storedExpressions, const QString &tag = QString() );

    /**
    * Returns the list of named expressions
    *
    *  \param tag               some content, maybe scope where to be shown
    */
    QList< QgsStoredExpression > storedExpressions( const QString &tag = QString() );


    /**
    * Returns an expression according to the id
    *
    *  \param id               id of the expression as identification
    *  \param tag               some content, maybe scope where to be shown
    */
    QgsStoredExpression storedExpression( const QUuid &id, const QString &tag = QString() );

    /**
    * Returns an expression according to the expression text
    *
    *  \param expression        id of the expression as identification
    *  \param tag               some content, maybe scope where to be shown
    */
    QgsStoredExpression findStoredExpressionByExpression( const QString &expression, const QString &tag = QString() );

    //! clears list of stored expressions
    void clearStoredExpressions();

    //! Writes the stored expressions out in XML format
    bool writeXml( QDomNode &layer_node ) const;

    //! Reads the  stored expressions in in XML format
    bool readXml( const QDomNode &layer_node );

  signals:

  public slots:

  private:
    Mode mMode = FilterExpression;
    QList< QgsStoredExpression > mStoredExpressions;
};

#endif // QGSSTOREDEXPRESSIONMANAGER_H
