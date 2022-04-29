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
     * Initializes the default random style color scheme for the user.
     * \since QGIS 3.2
     */
    void initStyleScheme();

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
     * \returns TRUE if scheme was found and removed
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
    QList<QgsColorScheme *> schemes( QgsColorScheme::SchemeFlag flag ) const;


    /**
     * Returns color schemes of a specific type
     * \param schemeList destination list for matching schemes
     * \note not available in Python bindings
     */
#ifndef SIP_RUN
    template<class T> void schemes( QList<T *> &schemeList ) const
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

    /**
     * Sets the color \a scheme to use when fetching random colors to use for symbol styles.
     *
     * \a scheme should match a color scheme which is already present in the registry.
     *
     * Note that calling this method takes a snapshot of the colors from the scheme's
     * QgsColorScheme::fetchColors() list. Accordingly, any future changes to the colors
     * in \a scheme are not automatically reflected by calls to fetchRandomStyleColor().
     * If \a scheme is updated, then another call to setRandomStyleColorScheme() must
     * be made in order to update the cached list of available style colors.
     *
     * \see randomStyleColorScheme()
     * \see fetchRandomStyleColor()
     *
     * \since QGIS 3.2
     */
    void setRandomStyleColorScheme( QgsColorScheme *scheme );

    /**
     * Returns the color scheme used when fetching random colors to use for symbol styles.
     *
     * This may be NULLPTR, in which case totally random colors are used for styles.
     *
     * \see setRandomStyleColorScheme()
     * \see fetchRandomStyleColor()
     *
     * \since QGIS 3.2
     */
    QgsColorScheme *randomStyleColorScheme();

    /**
     * Returns a random color for use with a new symbol style (e.g. for a newly created
     * map layer).
     *
     * If a randomStyleColorScheme() is set then this color will be randomly taken from that
     * color scheme. If no randomStyleColorScheme() is set then a totally random color
     * will be generated.
     *
     * Note that calling setRandomStyleColorScheme() takes a snapshot of the colors from the scheme's
     * QgsColorScheme::fetchColors() list. Accordingly, any future changes to the colors
     * in the scheme are not automatically reflected by calls to fetchRandomStyleColor().
     * If the scheme is updated, then another call to setRandomStyleColorScheme() must
     * be made in order to update the cached list of available style colors from which
     * fetchRandomStyleColor() selects colors.
     *
     * This method is thread safe.
     *
     * \see randomStyleColorScheme()
     * \see setRandomStyleColorScheme()
     * \since QGIS 3.2
     */
    QColor fetchRandomStyleColor() const;

  private:

    QList< QgsColorScheme * > mColorSchemeList;

    QgsColorScheme *mRandomStyleColorScheme = nullptr;
    QgsNamedColorList mRandomStyleColors;

    mutable int mNextRandomStyleColorIndex = 0;

    int mNextRandomStyleColorDirection = 1;

};



#endif
