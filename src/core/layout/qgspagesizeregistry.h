/***************************************************************************
                         qgspagesizeregistry.h
                         --------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSPAGESIZEREGISTRY_H
#define QGSPAGESIZEREGISTRY_H

#include "qgslayoutsize.h"
#include <QString>
#include <QSizeF>
#include <QList>

/**
 * \ingroup core
 * \class QgsPageSize
 * \brief A named page size for layouts.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsPageSize
{

  public:

    QgsPageSize();

    /**
     * Constructor for QgsPageSize, accepting the \a name of the page size and
     * page \a size.
    */
    QgsPageSize( const QString &name, const QgsLayoutSize &size );

    /**
     * Constructor for QgsPageSize, accepting a page \a size.
    */
    QgsPageSize( const QgsLayoutSize &size );

    //! Name of page size
    QString name;

    //! Page size
    QgsLayoutSize size;

    bool operator==( const QgsPageSize &other ) const;
    bool operator!=( const QgsPageSize &other ) const;
};

/**
 * \ingroup core
 * \class QgsPageSizeRegistry
 * \brief A registry for known page sizes.
 *
 * QgsPageSizeRegistry is not usually directly created, but rather accessed through
 * QgsApplication::pageSizeRegistry().
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsPageSizeRegistry
{
  public:

    /**
     * Creates a registry and populates it with known sizes
     */
    QgsPageSizeRegistry();

    /**
     * Adds a page \a size to the registry.
     */
    void add( const QgsPageSize &size );

    /**
     * Returns a list of page sizes in the registry.
     */
    QList< QgsPageSize > entries() const;

    /**
     * Finds matching page sizes from the registry, using a case insensitive match
     * on the page size \a name.
     */
    QList< QgsPageSize > find( const QString &name ) const;

    /**
     * Decodes a \a string representing a preset page size.
     * The decoded page size will be stored in the \a size argument.
     * \returns true if string was successfully decoded
    */
    bool decodePageSize( const QString &string, QgsPageSize &size );

  private:

    QList< QgsPageSize > mPageSizes;
};

#endif //QGSPAGESIZEREGISTRY_H
