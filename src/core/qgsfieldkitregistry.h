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

#include <QVector>
class QgsFieldKit;

class QgsFieldKitRegistry
{
  public:

    /**
     * You should not normally need to create your own field kit registry.
     *
     * Use the one provided by `QgsApplication::fieldKitRegistry()` instead.
     */
    QgsFieldKitRegistry();
    ~QgsFieldKitRegistry();

    /**
     * They will take precedence in order of adding them.
     * The later they are added, the more weight they have.
     *
     * Ownership is transferred to the registry.
     */
    void addFieldKit( QgsFieldKit* kit );

    void removeFieldKit( QgsFieldKit* kit );

  private:
    QVector<QgsFieldKit*> mFieldKits;
};

#endif // QGSFIELDKITREGISTRY_H
