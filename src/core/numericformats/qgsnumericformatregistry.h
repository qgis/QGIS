/***************************************************************************
                             qgsnumericformatregistry.h
                             --------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNUMERICFORMATREGISTRY_H
#define QGSNUMERICFORMATREGISTRY_H

#include <QHash>
#include <QString>
#include <QObject>

#include "qgis_sip.h"
#include "qgis_core.h"

class QgsNumericFormat;
class QDomElement;
class QgsReadWriteContext;

/**
 * \ingroup core
 * \brief The QgsNumericFormatRegistry manages registered classes of QgsNumericFormat.
 *
 * A reference to the QgsFieldFormatterRegistry can be obtained from
 * QgsApplication::numericFormatRegistry().
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsNumericFormatRegistry
{

  public:

    /**
     * You should not normally need to create your own numeric format registry.
     *
     * Use the one provided by `QgsApplication::numericFormatRegistry()` instead.
     */
    explicit QgsNumericFormatRegistry();
    ~QgsNumericFormatRegistry();

    /**
     * Returns a list of the format IDs currently contained in the registry.
     */
    QStringList formats() const;

    /**
     * Adds a new \a format to the registry.
     *
     * Ownership is transferred to the registry.
     */
    void addFormat( QgsNumericFormat *format SIP_TRANSFER );

    /**
     * Removes the format with matching \a id from the registry.
     */
    void removeFormat( const QString &id );

    /**
     * Creates a new numeric format by \a id. If there is no such \a id registered,
     * a default QgsFallbackNumericFormat with a null id will be returned instead.
     *
     * The caller takes ownership of the returned object.
     */
    QgsNumericFormat *format( const QString &id ) const SIP_TRANSFERBACK;

    /**
     * Creates a new numeric format by \a id, using the supplied \a configuration. If there is no such \a id registered,
     * a default QgsFallbackNumericFormat with a null id will be returned instead.
     *
     * The caller takes ownership of the returned object.
     */
    QgsNumericFormat *create( const QString &id, const QVariantMap &configuration, const QgsReadWriteContext &context ) const SIP_TRANSFERBACK;

    /**
     * Creates a new numeric format from an XML \a element. If there is no matching format ID registered,
     * a default QgsFallbackNumericFormat will be returned instead.
     *
     * The caller takes ownership of the returned object.
     */
    QgsNumericFormat *createFromXml( const QDomElement &element, const QgsReadWriteContext &context ) const SIP_TRANSFERBACK;

    /**
     * Returns a basic numeric formatter which can be used
     * to represent any number in an default manner.
     *
     * The caller takes ownership of the returned object.
     */
    QgsNumericFormat *fallbackFormat() const SIP_FACTORY;

    /**
     * Returns the translated, user-visible name for the format with matching \a id.
     */
    QString visibleName( const QString &id ) const;

    /**
     * Returns the sorting key for the format with matching \a id.
     */
    int sortKey( const QString &id ) const;

  private:
    QHash<QString, QgsNumericFormat *> mFormats;
};

#endif // QGSNUMERICFORMATREGISTRY_H
