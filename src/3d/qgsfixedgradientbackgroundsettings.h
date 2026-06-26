/***************************************************************************
  qgsfixedgradientbackgroundsettings.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Dominik Cindrić
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFIXEDGRADIENTBACKGROUNDSETTINGS_H
#define QGSFIXEDGRADIENTBACKGROUNDSETTINGS_H

#include "qgis_3d.h"
#include "qgsabstract3dmapbackgroundsettings.h"

#include <QColor>
#include <QString>

/**
 * \ingroup qgis_3d
 * \brief Background settings for a two-color vertical gradient rendered behind the 3D scene.
 *
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsFixedGradientBackgroundSettings : public QgsAbstract3DMapBackgroundSettings
{
  public:
    QgsFixedGradientBackgroundSettings() = default;

    Qgis::Map3DBackgroundType type() const override { return Qgis::Map3DBackgroundType::FixedGradientBackground; }

    QgsFixedGradientBackgroundSettings *clone() const override SIP_FACTORY;
    void readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const override;

    /**
     * Returns the color at the top of the gradient.
     *
     * \see setTopColor()
     */
    QColor topColor() const { return mTopColor; }

    /**
     * Sets the \a color at the top of the gradient.
     *
     * \see topColor()
     */
    void setTopColor( const QColor &color ) { mTopColor = color; }

    /**
     * Returns the color at the bottom of the gradient.
     *
     * \see setBottomColor()
     */
    QColor bottomColor() const { return mBottomColor; }

    /**
     * Sets the \a color at the bottom of the gradient.
     *
     * \see bottomColor()
     */
    void setBottomColor( const QColor &color ) { mBottomColor = color; }

  private:
    QColor mTopColor = QColor( 0, 128, 255 );
    QColor mBottomColor = Qt::black;
};

#endif // QGSFIXEDGRADIENTBACKGROUNDSETTINGS_H
