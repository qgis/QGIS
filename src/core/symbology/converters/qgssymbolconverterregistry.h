/***************************************************************************
                             qgssymbolconverterregistry.h
                             -----------------
    begin                : February 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#ifndef QGSSYMBOLCONVERTERREGISTRY_H
#define QGSSYMBOLCONVERTERREGISTRY_H

#include "qgis.h"
#include "qgis_core.h"

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

class QgsAbstractSymbolConverter;

/**
 * \class QgsSymbolConverterRegistry
 * \ingroup core
 * \brief A registry of known symbol converters.
 *
 * QgsSymbolConverterRegistry is not usually directly created, but rather accessed through
 * QgsApplication::symbolConverterRegistry().
 *
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsSymbolConverterRegistry : public QObject
{
    Q_OBJECT

  public:
    /**
     * Creates a new empty symbol converter registry.
     *
     * QgsSymbolConverterRegistry is not usually directly created, but rather accessed through
     * QgsApplication::symbolConverterRegistry().
     */
    QgsSymbolConverterRegistry( QObject *parent = nullptr );

    ~QgsSymbolConverterRegistry() override;

    /**
     * Adds the default symbol converters to the registry.
     * \note Not available through Python bindings.
     */
    void populate() SIP_SKIP;

    /**
     * Adds a \a converter to the registry.
     *
     * Ownership of the converter is transferred to the registry.
     *
     * \returns TRUE if the converter was successfully added.
     */
    bool addConverter( QgsAbstractSymbolConverter *converter SIP_TRANSFER );

    /**
     * Returns the converter with matching \a name, or NULLPTR if no matching
     * converter is registered.
     */
    QgsAbstractSymbolConverter *converter( const QString &name ) const;

    /**
     * Removes the converter with matching \a name.
     *
     * The converter will be deleted.
     *
     * \returns TRUE if the converter was successfully removed.
     */
    bool removeConverter( const QString &name );

    /**
     * Returns a list of the registered converter names (IDs).
     */
    QStringList converterNames() const;

  private:
    QMap<QString, QgsAbstractSymbolConverter *> mConverters;
};

#endif //QGSSYMBOLCONVERTERREGISTRY_H
