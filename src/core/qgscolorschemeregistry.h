/***************************************************************************
                             qgscolorschemeregistry.h
                             ------------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#ifndef QGSCOLORSCHEMEREGISTRY_H
#define QGSCOLORSCHEMEREGISTRY_H

#include "qgis_core.h"
#include "qgscolorscheme.h"
#include <QList>

/**
 * \ingroup core
 * \class QgsColorSchemeRegistry
 * \brief Registry of color schemes
 *
 * A registry of QgsColorScheme color schemes. This class can be created directly, or
 * accessed via a QgsApplication::colorSchemeRegistry().
 * \since QGIS 2.5
 */
class CORE_EXPORT QgsColorSchemeRegistry
{

  public:

    /**
     * Constructor for an empty color scheme registry
     */
    QgsColorSchemeRegistry() = default;

    virtual ~QgsColorSchemeRegistry();

    /**
     * Adds all color schemes from the global instance to this color scheme.
     * \see addDefaultSchemes
     * \see addColorScheme
     */
    void populateFromInstance();

    /**
     * Adds all default color schemes to this color scheme.
     * \see populateFromInstance
     * \see addColorScheme
     * \see addUserSchemes
     */
    void addDefaultSchemes();

    /**
     * Creates schemes for all gpl palettes in the user's palettes folder.
     * \see populateFromInstance
     * \see addDefaultSchemes
     * \see addColorScheme
     */
    void addUserSchemes();

    /**
     * Adds a color scheme to the registry. Ownership of the scheme is transferred
     * to the registry.
     * \param scheme color scheme to add
     * \see populateFromInstance
     * \see removeColorScheme
     */
    void addColorScheme( QgsColorScheme *scheme SIP_TRANSFER );

    /**
     * Removes all matching color schemes from the registry
     * \param scheme color scheme to remove
     * \returns true if scheme was found and removed
     * \see addColorScheme
     */
    bool removeColorScheme( QgsColorScheme *scheme );

    /**
     * Returns all color schemes in the registry
     * \returns list of color schemes
     */
    QList<QgsColorScheme *> schemes() const;

    /**
     * Returns all color schemes in the registry which have a specified flag set
     * \param flag flag to match
     * \returns list of color schemes with flag set
     */
    QList<QgsColorScheme *> schemes( const QgsColorScheme::SchemeFlag flag ) const;


    /**
     * Return color schemes of a specific type
     * \param schemeList destination list for matching schemes
     * \note not available in Python bindings
     */
#ifndef SIP_RUN
    template<class T> void schemes( QList<T *> &schemeList )
    {
      schemeList.clear();
      QList<QgsColorScheme *> schemeInstanceList = schemes();
      QList<QgsColorScheme *>::iterator schemeIt = schemeInstanceList.begin();
      for ( ; schemeIt != schemeInstanceList.end(); ++schemeIt )
      {
        T *scheme = dynamic_cast<T *>( *schemeIt );
        if ( scheme )
        {
          schemeList.push_back( scheme );
        }
      }
    }
#endif

  private:

    QList< QgsColorScheme * > mColorSchemeList;

};



#endif
