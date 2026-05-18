/***************************************************************************
  qgscolorgradingsettings.h
  --------------------------------------
  Date                 : May 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORGRADINGSETTINGS_H
#define QGSCOLORGRADINGSETTINGS_H

#include "qgis.h"
#include "qgis_3d.h"

#include <QMap>
#include <QString>

#define SIP_NO_FILE

class QgsReadWriteContext;
class QDomElement;


/**
 * \brief Contains the configuration of the scene's color grading settings, such as exposure and tone mapping.
 * \ingroup qgis_3d
 * \note Not available in Python bindings
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsColorGradingSettings
{
  public:
    QgsColorGradingSettings() = default;
    QgsColorGradingSettings( const QgsColorGradingSettings &other );
    QgsColorGradingSettings &operator=( QgsColorGradingSettings const &rhs );

    /**
     * Reads settings from a DOM \a element.
     *
     * \see writeXml()
     */
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Writes settings to a DOM \a element.
     *
     * \see readXml()
     */
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const;

    /**
     * Returns the exposure adjustment value.
     *
     * Exposure adjustment is applied to the linear color values before bloom and tone mapping are evaluated.
     *
     * A value of 0.0 represents no adjustment. Positive values will brighten the scene,
     * while negative values will darken it.
     *
     * \see setExposureAdjustment()
     */
    double exposureAdjustment() const { return mExposureAdjustment; }

    /**
     * Sets the exposure \a adjustment value.
     *
     * \see exposureAdjustment()
     */
    void setExposureAdjustment( double adjustment ) { mExposureAdjustment = adjustment; }

    /**
     * Returns the tone mapping method applied to the scene.
     *
     * Tone mapping handles the transformation of High Dynamic Range (HDR) colors into
     * the Low Dynamic Range (LDR) range. It is evaluated after exposure and bloom effects
     * have been applied.
     *
     * \see setToneMapping()
     */
    Qgis::ToneMappingMethod toneMapping() const { return mToneMapping; }

    /**
     * Sets the tone \a mapping method.
     * \see toneMapping()
     */
    void setToneMapping( Qgis::ToneMappingMethod mapping ) { mToneMapping = mapping; }

  private:
    double mExposureAdjustment = 0.0;
    Qgis::ToneMappingMethod mToneMapping = Qgis::ToneMappingMethod::Clamp;
};

#endif // QGSCOLORGRADINGSETTINGS_H
