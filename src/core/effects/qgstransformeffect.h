/***************************************************************************
                             qgstransformeffect.h
                             --------------------
    begin                : March 2015
    copyright            : (C) 2015 Nyall Dawson
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
#ifndef QGSTRANSFORMEFFECT_H
#define QGSTRANSFORMEFFECT_H

#include "qgis_core.h"
#include "qgspainteffect.h"
#include "qgis.h"
#include "qgsmapunitscale.h"
#include "qgsunittypes.h"
#include <QPainter>

/**
 * \ingroup core
 * \class QgsTransformEffect
 * \brief A paint effect which applies transformations (such as move,
 * scale and rotate) to a picture.
 *
 * \since QGIS 2.9
 */

class CORE_EXPORT QgsTransformEffect : public QgsPaintEffect
{

  public:

    /**
     * Creates a new QgsTransformEffect effect from a properties string map.
     * \param map encoded properties string map
     * \returns new QgsTransformEffect
     */
    static QgsPaintEffect *create( const QgsStringMap &map ) SIP_FACTORY;

    /**
     * Constructor for QgsTransformEffect.
     */
    QgsTransformEffect() = default;

    QString type() const override { return QStringLiteral( "transform" ); }
    QgsStringMap properties() const override;
    void readProperties( const QgsStringMap &props ) override;
    QgsTransformEffect *clone() const override SIP_FACTORY;

    /**
     * Sets the transform x translation.
     * \param translateX distance to translate along the x axis
     * \see translateX
     * \see setTranslateY
     * \see setTranslateUnit
     * \see setTranslateMapUnitScale
     */
    void setTranslateX( const double translateX ) { mTranslateX = translateX; }

    /**
     * Returns the transform x translation.
     * \returns X distance translated along the x axis
     * \see setTranslateX
     * \see translateY
     * \see translateUnit
     * \see translateMapUnitScale
     */
    double translateX() const { return mTranslateX; }

    /**
     * Sets the transform y translation.
     * \param translateY distance to translate along the y axis
     * \see translateY
     * \see setTranslateX
     * \see setTranslateUnit
     * \see setTranslateMapUnitScale
     */
    void setTranslateY( const double translateY ) { mTranslateY = translateY; }

    /**
     * Returns the transform y translation.
     * \returns Y distance translated along the y axis
     * \see setTranslateY
     * \see translateX
     * \see translateUnit
     * \see translateMapUnitScale
     */
    double translateY() const { return mTranslateY; }

    /**
     * Sets the units used for the transform translation.
     * \param unit units for translation
     * \see translateUnit
     * \see setTranslateX
     * \see setTranslateY
     * \see setTranslateMapUnitScale
     */
    void setTranslateUnit( const QgsUnitTypes::RenderUnit unit ) { mTranslateUnit = unit; }

    /**
     * Returns the units used for the transform translation.
     * \returns units for translation
     * \see setTranslateUnit
     * \see translateX
     * \see translateY
     * \see translateMapUnitScale
     */
    QgsUnitTypes::RenderUnit translateUnit() const { return mTranslateUnit; }

    /**
     * Sets the map unit scale used for the transform translation.
     * \param scale map unit scale for translation
     * \see translateMapUnitScale
     * \see setTranslateX
     * \see setTranslateY
     * \see setTranslateUnit
     */
    void setTranslateMapUnitScale( const QgsMapUnitScale &scale ) { mTranslateMapUnitScale = scale; }

    /**
     * Returns the map unit scale used for the transform translation.
     * \returns map unit scale for translation
     * \see setTranslateMapUnitScale
     * \see translateX
     * \see translateY
     * \see translateUnit
     */
    const QgsMapUnitScale &translateMapUnitScale() const { return mTranslateMapUnitScale; }

    /**
     * Sets the x axis scaling factor.
     * \param scaleX factor to scale x axis by, where 1.0 = no scaling
     * \see scaleX
     * \see setScaleY
     */
    void setScaleX( const double scaleX ) { mScaleX = scaleX; }

    /**
     * Returns the x axis scaling factor.
     * \returns x axis scaling factor, where 1.0 = no scaling
     * \see setScaleX
     * \see scaleY
     */
    double scaleX() const { return mScaleX; }

    /**
     * Sets the y axis scaling factor.
     * \param scaleY factor to scale y axis by, where 1.0 = no scaling
     * \see scaleX
     */
    void setScaleY( const double scaleY ) { mScaleY = scaleY; }

    /**
     * Returns the y axis scaling factor.
     * \returns y axis scaling factor, where 1.0 = no scaling
     * \see setScaleY
     * \see scaleX
     */
    double scaleY() const { return mScaleY; }

    /**
     * Sets the transform \a rotation, in degrees clockwise.
     * \see rotation()
     */
    void setRotation( const double rotation ) { mRotation = rotation; }

    /**
     * Returns the transform rotation, in degrees clockwise.
     * \see setRotation()
     */
    double rotation() const { return mRotation; }

    /**
     * Sets the x axis shearing factor.
     * \param shearX x axis shearing
     * \see shearX
     * \see setShearY
     */
    void setShearX( const double shearX ) { mShearX = shearX; }

    /**
     * Returns the x axis shearing factor.
     * \returns x axis shearing
     * \see setShearX
     * \see shearY
     */
    double shearX() const { return mShearX; }

    /**
     * Sets the y axis shearing factor.
     * \param shearY y axis shearing
     * \see shearY
     * \see setShearX
     */
    void setShearY( const double shearY ) { mShearY = shearY; }

    /**
     * Returns the y axis shearing factor.
     * \returns y axis shearing
     * \see setShearY
     * \see shearX
     */
    double shearY() const { return mShearY; }

    /**
     * Sets whether to reflect along the x-axis
     * \param reflectX true to reflect horizontally
     * \see reflectX
     * \see setReflectY
     */
    void setReflectX( const bool reflectX ) { mReflectX = reflectX; }

    /**
     * Returns whether transform will be reflected along the x-axis
     * \returns true if transform will reflect horizontally
     * \see setReflectX
     * \see reflectY
     */
    bool reflectX() const { return mReflectX; }

    /**
     * Sets whether to reflect along the y-axis
     * \param reflectY true to reflect horizontally
     * \see reflectY
     * \see setReflectX
     */
    void setReflectY( const bool reflectY ) { mReflectY = reflectY; }

    /**
     * Returns whether transform will be reflected along the y-axis
     * \returns true if transform will reflect horizontally
     * \see setReflectY
     * \see reflectX
     */
    bool reflectY() const { return mReflectY; }

  protected:

    void draw( QgsRenderContext &context ) override;
    QRectF boundingRect( const QRectF &rect, const QgsRenderContext &context ) const override;

  private:

    double mTranslateX = 0.0;
    double mTranslateY = 0.0;
    QgsUnitTypes::RenderUnit mTranslateUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mTranslateMapUnitScale;
    double mScaleX = 1.0;
    double mScaleY = 1.0;
    double mRotation = 0.0;
    double mShearX = 0.0;
    double mShearY = 0.0;
    bool mReflectX = false;
    bool mReflectY = false;

    QTransform createTransform( const QgsRenderContext &context ) const;

};

#endif // QGSTRANSFORMEFFECT_H

