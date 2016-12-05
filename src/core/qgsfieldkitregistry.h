/***************************************************************************
  qgsfieldkitregistry.h - QgsFieldKitRegistry

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

class QgsFieldKit;

/**
 * \ingroup core
 * The QgsFieldKitRegistry manages registered classes of QgsFieldKit.
 * A reference to the QgsFieldKitRegistry can be obtained from
 * QgsApplication::fieldKitRegistry().
 */
class CORE_EXPORT QgsFieldKitRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * You should not normally need to create your own field kit registry.
     *
     * Use the one provided by `QgsApplication::fieldKitRegistry()` instead.
     */
    QgsFieldKitRegistry( QObject* parent = nullptr );
    ~QgsFieldKitRegistry();

    /**
     * They will take precedence in order of adding them.
     * The later they are added, the more weight they have.
     *
     * Ownership is transferred to the registry.
     */
    void addFieldKit( QgsFieldKit* kit );

    /**
     * Remove a field kit from the registry.
     * The field kit will be deleted.
     */
    void removeFieldKit( QgsFieldKit* kit );

    /**
     * Get a field kit by its id. If there is no such id registered,
     * a default QgsFallbackFieldKit with a null id will be returned instead.
     */
    QgsFieldKit* fieldKit( const QString& id ) const;

  signals:

    /**
     * Will be emitted after a new field kit has been added.
     */
    void fieldKitAdded( QgsFieldKit* kit );

    /**
     * Will be emitted just before a field kit is removed and deleted.
     */
    void fieldKitRemoved( QgsFieldKit* kit );

  private:
    QHash<QString, QgsFieldKit*> mFieldKits;
    QgsFieldKit* mFallbackFieldKit;
};

#endif // QGSFIELDKITREGISTRY_H
