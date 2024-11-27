/***************************************************************************
                             qgsdatabaseschemaitem.h
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
#ifndef QGSDATABASESCHEMAITEM_H
#define QGSDATABASESCHEMAITEM_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsdatacollectionitem.h"

/**
 * \ingroup core
 * \brief A Collection that represents a database schema item
 * \since QGIS 3.16
*/
class CORE_EXPORT QgsDatabaseSchemaItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsDatabaseSchemaItem, with the specified \a parent item.
     *
     * The \a name argument specifies the text to show in the model for the item. A translated string should
     * be used wherever appropriate.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form,
     * but QgsSchemaItem items pointing to different logical locations should always use a different item \a path.
     *
     * The optional \a providerKey string can be used to specify the key for the QgsDataItemProvider that created this item.
     */
    QgsDatabaseSchemaItem( QgsDataItem *parent SIP_TRANSFERTHIS, const QString &name, const QString &path = QString(), const QString &providerKey = QString() );

    ~QgsDatabaseSchemaItem() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsDatabaseSchemaItem: \"%1\" %2>" ).arg( sipCpp->name(), sipCpp->path() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QgsAbstractDatabaseProviderConnection *databaseConnection() const override;

};

#endif // QGSDATABASESCHEMAITEM_H


