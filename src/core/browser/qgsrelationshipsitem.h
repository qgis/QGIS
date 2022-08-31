/***************************************************************************
                             qgsrelationshipsitem.h
                             -------------------
    begin                : 2022-07-28
    copyright            : (C) 2022 Nyall Dawson
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
#ifndef QGSRELATIONSHIPSITEM_H
#define QGSRELATIONSHIPSITEM_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsdataitem.h"
#include "qgsweakrelation.h"

/**
 * \ingroup core
 * \brief Contains a collection of relationship items.
 * \since QGIS 3.28
*/
class CORE_EXPORT QgsRelationshipsItem : public QgsDataItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsRelationshipsItem, with the specified \a parent item.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form,
     * but QgsDataItem items pointing to different logical locations should always use a different item \a path.
     * The \a connectionUri argument is the connection part of the layer URI that it is used internally to create
     * a connection and retrieve fields information.
     * The \a providerKey string can be used to specify the key for the QgsDataItemProvider that created this item.
     *
     * The optional \a schema and \a tableName arguments can be used to restrict the visible relationships to
     * those with a matching parent table.
     */
    QgsRelationshipsItem( QgsDataItem *parent SIP_TRANSFERTHIS,
                          const QString &path,
                          const QString &connectionUri,
                          const QString &providerKey,
                          const QString &schema = QString(),
                          const QString &tableName = QString() );

    ~QgsRelationshipsItem() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsRelationshipsItem: %1>" ).arg( sipCpp->path() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QVector<QgsDataItem *> createChildren() override;

    QIcon icon() override;

    /**
     * Returns the connection URI
     */
    QString connectionUri() const;

    /**
     * Returns the schema for filtering relationships, if set.
     *
     * \see tableName()
     */
    QString schema() const { return mSchema; }

    /**
     * Returns the table name for filtering relationships, if set.
     *
     * \see schema()
     */
    QString tableName() const {return mTableName;}

  private:

    QString mConnectionUri;
    QString mSchema;
    QString mTableName;
    QStringList mRelationshipNames;

};


/**
 * \ingroup core
 * \brief A browser item representing a relationship.
 * \since QGIS 3.28
*/
class CORE_EXPORT QgsRelationshipItem : public QgsDataItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsRelationshipItem, with the specified \a parent item and \a relation.
     *
     * \note parent item must be a QgsRelationshipsItem.
     */
    QgsRelationshipItem( QgsDataItem *parent SIP_TRANSFERTHIS,
                         const QgsWeakRelation &relation );

    ~QgsRelationshipItem() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsRelationshipItem: %1>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QIcon icon() override;

    /**
     * Returns the associated relationship.
     */
    const QgsWeakRelation &relation() const;

  private:

    QgsWeakRelation mRelation;

};

#endif // QGSRELATIONSHIPSITEM_H


