/***************************************************************************
                             qgsfieldsitem.h
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFIELDSITEM_H
#define QGSFIELDSITEM_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsdataitem.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsfield.h"

/**
 * \ingroup core
 * \brief A collection of field items with some internal logic to retrieve
 * the fields and a the vector layer instance from a connection URI,
 * the schema and the table name.
 * \since QGIS 3.16
*/
class CORE_EXPORT QgsFieldsItem : public QgsDataItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsFieldsItem, with the specified \a parent item.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form,
     * but QgsDataItem items pointing to different logical locations should always use a different item \a path.
     * The \a connectionUri argument is the connection part of the layer URI that it is used internally to create
     * a connection and retrieve fields information.
     * The \a providerKey string can be used to specify the key for the QgsDataItemProvider that created this item.
     * The \a schema and \a tableName are used to retrieve the layer and field information from the \a connectionUri.
     */
    QgsFieldsItem( QgsDataItem *parent SIP_TRANSFERTHIS,
                   const QString &path,
                   const QString &connectionUri,
                   const QString &providerKey,
                   const QString &schema,
                   const QString &tableName );

    ~QgsFieldsItem() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsFieldsItem: %1>" ).arg( sipCpp->path() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QVector<QgsDataItem *> createChildren() override;

    QIcon icon() override;

    /**
     * Returns the schema name
     */
    QString schema() const;

    /**
     * Returns the table name
     */
    QString tableName() const;

    /**
     * Returns the connection URI
     */
    QString connectionUri() const;

    /**
     * Creates and returns a (possibly NULL) layer from the connection URI and schema/table information
     */
    QgsVectorLayer *layer() SIP_FACTORY;

    /**
     * Returns the (possibly NULL) properties of the table this fields belong to.
     * \since QGIS 3.16
     */
    QgsAbstractDatabaseProviderConnection::TableProperty *tableProperty() const;

  private:

    QString mSchema;
    QString mTableName;
    QString mConnectionUri;
    std::unique_ptr<QgsAbstractDatabaseProviderConnection::TableProperty> mTableProperty;

};


/**
 * \ingroup core
 * \brief A layer field item, information about the connection URI, the schema and the
 * table as well as the layer instance the field belongs to can be retrieved
 * from the parent QgsFieldsItem object.
 * \since QGIS 3.16
*/
class CORE_EXPORT QgsFieldItem : public QgsDataItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsFieldItem, with the specified \a parent item and \a field.
     * \note parent item must be a QgsFieldsItem
     */
    QgsFieldItem( QgsDataItem *parent SIP_TRANSFERTHIS,
                  const QgsField &field );

    ~QgsFieldItem() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsFieldItem: %1>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QIcon icon() override;

    /**
     * Returns the field definition.
     *
     * \since QGIS 3.26
     */
    QgsField field() const { return mField; }

    bool equal( const QgsDataItem *other ) override;

  private:

    const QgsField mField;

};

#endif // QGSFIELDSITEM_H


