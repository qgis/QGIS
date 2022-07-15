/***************************************************************************
  qgsfeature3dhandler_p.h
  --------------------------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATURE3DHANDLER_P_H
#define QGSFEATURE3DHANDLER_P_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <Qt3DCore/QEntity>

class QgsFeature;


#include "qgsexpressioncontext.h"

class Qgs3DMapSettings;

#define SIP_NO_FILE


/**
 * \ingroup 3d
 * \brief Rendering context for preparation of 3D entities.
 *
 * \note Not available in Python bindings
 */
class Qgs3DRenderContext
{
  public:
    Qgs3DRenderContext( const Qgs3DMapSettings &map ) : mMap( map ) {}

    const Qgs3DMapSettings &map() const { return mMap; }

    /**
     * Sets the expression context. This context is used for all expression evaluation
     * associated with this render context.
     * \see expressionContext()
     */
    void setExpressionContext( const QgsExpressionContext &context ) { mExpressionContext = context; }

    /**
     * Gets the expression context. This context should be used for all expression evaluation
     * associated with this render context.
     * \see setExpressionContext()
     */
    QgsExpressionContext &expressionContext() { return mExpressionContext; }

    /**
     * Gets the expression context (const version). This context should be used for all expression evaluation
     * associated with this render context.
     * \see setExpressionContext()
     * \note not available in Python bindings
     */
    const QgsExpressionContext &expressionContext() const { return mExpressionContext; } SIP_SKIP

  private:
    const Qgs3DMapSettings &mMap;
    //! Expression context
    QgsExpressionContext mExpressionContext;

};


/**
 * \ingroup 3d
 * \brief Interface to be implemented by 3D symbol implementations in order to generate 3D entities.
 */
class QgsFeature3DHandler
{
  public:
    virtual ~QgsFeature3DHandler() = default;

    /**
     * Called before feature iteration starts to initialize, get required attributes.
     * \returns TRUE on success (on FALSE the handler failed to initialize and processFeature() / finalize() should not be called
     */
    virtual bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) = 0;

    /**
     * Called for every feature to extract information out of it into some
     * temporary variables in the derived handler class.
     */
    virtual void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) = 0;

    /**
     * When feature iteration has finished, finalize() is called to turn the extracted data
     * to a 3D entity object(s) attached to the given parent.
     */
    virtual void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) = 0;

    /**
     * Returns minimal Z value of the data (in world coordinates).
     * \note this method should not be called before call to finalize() - it may not be initialized
     */
    float zMinimum() const { return mZMin; }

    /**
     * Returns maximal Z value of the data (in world coordinates).
     * \note this method should not be called before call to finalize() - it may not be initialized
     */
    float zMaximum() const { return mZMax; }

    /**
     * Returns the number of features processed by the handler.
     */
    int featureCount() const { return mFeatureCount; }

  protected:
    //! updates zMinimum, zMaximum from the vector of positions in 3D world coordinates
    void updateZRangeFromPositions( const QVector<QVector3D> &positions );

  protected:
    float mZMin = std::numeric_limits<float>::max();
    float mZMax = std::numeric_limits<float>::lowest();
    int mFeatureCount = 0;
};


class Qgs3DMapSettings;
class QgsVectorLayer;

namespace Qgs3DSymbolImpl
{
  //! generic method to iterate over a layer, handle features with handler and create an entity out of it
  Qt3DCore::QEntity *entityFromHandler( QgsFeature3DHandler *handler, const Qgs3DMapSettings &map, QgsVectorLayer *layer );
}


/// @endcond

#endif // QGSFEATURE3DHANDLER_P_H
