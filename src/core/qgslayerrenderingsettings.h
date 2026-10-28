/***************************************************************************
  qgslayerrenderingsettings.h
  --------------------------------------
  Date                 : July 2026
  Copyright            : (C) 2026 by Damiano Lombardi
  Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/**
 * \ingroup core
 * \brief Stores layer-level rendering settings supplied by data providers.
 *
 * These settings are intended for layer-wide rendering behavior (such as opacity
 * and scale based visibility)
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsLayerRenderingSettings
{
  public:
    /**
     * Sets layer opacity in the range [0, 1].
     */
    void setLayerOpacity( double opacity ) { mLayerOpacity = opacity; }

    /**
     * Returns TRUE if layer opacity is set.
     */
    bool hasLayerOpacity() const { return mLayerOpacity.has_value(); }

    /**
     * Returns layer opacity in the range [0, 1].
     */
    double layerOpacity() const { return mLayerOpacity.value_or( 1.0 ); }

    /**
     * Sets minimum scale denominator for visibility.
     */
    void setMinimumScale( double scale ) { mMinimumScale = scale; }

    /**
     * Returns TRUE if minimum scale is set.
     */
    bool hasMinimumScale() const { return mMinimumScale.has_value(); }

    /**
     * Returns minimum scale denominator.
     */
    double minimumScale() const { return mMinimumScale.value_or( 0 ); }

    /**
     * Sets maximum scale denominator for visibility.
     */
    void setMaximumScale( double scale ) { mMaximumScale = scale; }

    /**
     * Returns TRUE if maximum scale is set.
     */
    bool hasMaximumScale() const { return mMaximumScale.has_value(); }

    /**
     * Returns maximum scale denominator.
     */
    double maximumScale() const { return mMaximumScale.value_or( 0 ); }

  private:
    std::optional<double> mLayerOpacity;
    std::optional<double> mMinimumScale;
    std::optional<double> mMaximumScale;
};
