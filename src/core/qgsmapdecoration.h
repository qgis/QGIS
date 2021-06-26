/***************************************************************************
                          qgsmapdecoration.h
                          ----------------
    begin                : April 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPDECORATION_H
#define QGSMAPDECORATION_H

#include "qgis_core.h"
#include "qgsmapsettings.h"
#include "qgsrendercontext.h"

/**
 * \ingroup core
 * \class QgsMapDecoration
 * \brief Interface for map decorations.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsMapDecoration
{

  public:

    /**
     * Constructor for QgsMapDecoration.
     */
    QgsMapDecoration() = default;

    virtual ~QgsMapDecoration() = default;

    /**
     * Renders a map decoration.
     */
    virtual void render( const QgsMapSettings &mapSettings, QgsRenderContext &context ) = 0;

    /**
     * Returns the map decoration display name.
     * \since QGIS 3.14
     */
    const QString displayName() const { return mDisplayName; }

  protected:

    /**
     * Sets the map decoration display \a name.
     * \since QGIS 3.14
     */
    void setDisplayName( const QString &name ) { mDisplayName = name; }

  private:

    QString mDisplayName;

};

#endif //QGSMAPDECORATION_H
