/***************************************************************************
                             qgsfielddomainsitem.h
                             -------------------
    begin                : 2022-01-27
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
#ifndef QGSFIELDDOMAINSITEM_H
#define QGSFIELDDOMAINSITEM_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsdataitem.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsfield.h"

/**
 * \ingroup core
 * \brief Contains a collection of field domain items.
 * \since QGIS 3.26
*/
class CORE_EXPORT QgsFieldDomainsItem : public QgsDataItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsFieldDomainsItem, with the specified \a parent item.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form,
     * but QgsDataItem items pointing to different logical locations should always use a different item \a path.
     * The \a connectionUri argument is the connection part of the layer URI that it is used internally to create
     * a connection and retrieve fields information.
     * The \a providerKey string can be used to specify the key for the QgsDataItemProvider that created this item.
     */
    QgsFieldDomainsItem( QgsDataItem *parent SIP_TRANSFERTHIS,
                         const QString &path,
                         const QString &connectionUri,
                         const QString &providerKey );

    ~QgsFieldDomainsItem() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsFieldDomainsItem: %1>" ).arg( sipCpp->path() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QVector<QgsDataItem *> createChildren() override;

    QIcon icon() override;

    /**
     * Returns the connection URI
     */
    QString connectionUri() const;

  private:

    QString mConnectionUri;
    QStringList mFieldDomainNames;

};


/**
 * \ingroup core
 * \brief A browser item representing a field domain.
 * \since QGIS 3.26
*/
class CORE_EXPORT QgsFieldDomainItem : public QgsDataItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsFieldDomainItem, with the specified \a parent item and \a domain.
     *
     * Ownership of \a domain is transferred to the item.
     *
     * \note parent item must be a QgsFieldDomainsItem.
     */
    QgsFieldDomainItem( QgsDataItem *parent SIP_TRANSFERTHIS,
                        QgsFieldDomain *domain SIP_TRANSFER );

    ~QgsFieldDomainItem() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsFieldDomainItem: %1>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QIcon icon() override;

    /**
     * Returns the associated field domain.
     */
    const QgsFieldDomain *fieldDomain();

  private:

    std::unique_ptr< QgsFieldDomain > mDomain;

};

#endif // QGSFIELDDOMAINSITEM_H


