/***************************************************************************
  qgsfieldformatterregistry.h - QgsFieldFormatterRegistry

 ---------------------
 begin                : 2.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFIELDKITREGISTRY_H
#define QGSFIELDKITREGISTRY_H

#include <QHash>
#include <QString>
#include <QObject>

#include "qgis_sip.h"
#include "qgis_core.h"

class QgsFieldFormatter;

/**
 * \ingroup core
 * The QgsFieldFormatterRegistry manages registered classes of QgsFieldFormatter.
 * A reference to the QgsFieldFormatterRegistry can be obtained from
 * QgsApplication::fieldFormatterRegistry().
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFieldFormatterRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * You should not normally need to create your own field formatter registry.
     *
     * Use the one provided by `QgsApplication::fieldFormatterRegistry()` instead.
     */
    explicit QgsFieldFormatterRegistry( QObject *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsFieldFormatterRegistry() override;

    /**
     * They will take precedence in order of adding them.
     * The later they are added, the more weight they have.
     *
     * Ownership is transferred to the registry.
     */
    void addFieldFormatter( QgsFieldFormatter *formatter SIP_TRANSFER );

    /**
     * Remove a field formatter from the registry.
     * The field formatter will be deleted.
     */
    void removeFieldFormatter( QgsFieldFormatter *formatter );

    /**
     * Remove the field formatter with the specified id.
     */
    void removeFieldFormatter( const QString &id );

    /**
     * Gets a field formatter by its id. If there is no such id registered,
     * a default QgsFallbackFieldFormatter with a null id will be returned instead.
     */
    QgsFieldFormatter *fieldFormatter( const QString &id ) const;

    /**
     * Returns a basic fallback field formatter which can be used
     * to represent any field in an unspectacular manner.
     */
    QgsFieldFormatter *fallbackFieldFormatter() const;

  signals:

    /**
     * Will be emitted after a new field formatter has been added.
     */
    void fieldFormatterAdded( QgsFieldFormatter *formatter );

    /**
     * Will be emitted just before a field formatter is removed and deleted.
     */
    void fieldFormatterRemoved( QgsFieldFormatter *formatter );

  private:
    QHash<QString, QgsFieldFormatter *> mFieldFormatters;
    QgsFieldFormatter *mFallbackFieldFormatter = nullptr;
};

#endif // QGSFIELDKITREGISTRY_H
