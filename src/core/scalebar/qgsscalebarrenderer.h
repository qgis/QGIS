/***************************************************************************
                            qgsscalebarrenderer.h
                            ---------------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco.hugentobler@karto.baug.ethz.ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSCALEBARRENDERER_H
#define QGSSCALEBARRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QRectF>
#include <QList>

class QgsRenderContext;
class QgsScaleBarSettings;

/**
 * \ingroup core
 * \class QgsScaleBarRenderer
 * \brief Abstract base class for scale bar renderers.
 *
 * Scalebar renderer subclasses implement custom drawing logic, with the possibility to implement
 * custom labeling.
 *
*/
class CORE_EXPORT QgsScaleBarRenderer
{
  public:

    /**
     * Flags which control scalebar renderer behavior.
     * \since QGIS 3.14
     */
    enum class Flag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      FlagUsesLineSymbol = 1 << 0, //!< Renderer utilizes the scalebar line symbol (see QgsScaleBarSettings::lineSymbol() )
      FlagUsesFillSymbol = 1 << 1, //!< Renderer utilizes the scalebar fill symbol (see QgsScaleBarSettings::fillSymbol() )
      FlagUsesAlternateFillSymbol = 1 << 2, //!< Renderer utilizes the alternate scalebar fill symbol (see QgsScaleBarSettings::alternateFillSymbol() )
      FlagRespectsUnits = 1 << 3, //!< Renderer respects the QgsScaleBarSettings::units() setting
      FlagRespectsMapUnitsPerScaleBarUnit = 1 << 4, //!< Renderer respects the QgsScaleBarSettings::mapUnitsPerScaleBarUnit() setting
      FlagUsesUnitLabel = 1 << 5, //!< Renderer uses the QgsScaleBarSettings::unitLabel() setting
      FlagUsesSegments = 1 << 6, //!< Renderer uses the scalebar segments
      FlagUsesLabelBarSpace = 1 << 7, //!< Renderer uses the QgsScaleBarSettings::labelBarSpace() setting
      FlagUsesLabelVerticalPlacement = 1 << 8, //!< Renderer uses the QgsScaleBarSettings::labelVerticalPlacement() setting
      FlagUsesLabelHorizontalPlacement = 1 << 8, //!< Renderer uses the QgsScaleBarSettings::labelHorizontalPlacement() setting
      FlagUsesAlignment = 1 << 9, //!< Renderer uses the QgsScaleBarSettings::alignment() setting
      FlagUsesSubdivisions = 1 << 10, //!< Renderer uses the scalebar subdivisions (see QgsScaleBarSettings::numberOfSubdivisions() )
      FlagUsesDivisionSymbol = 1 << 11, //!< Renderer utilizes the scalebar division symbol (see QgsScaleBarSettings::divisionLineSymbol() )
      FlagUsesSubdivisionSymbol = 1 << 12, //!< Renderer utilizes the scalebar subdivision symbol (see QgsScaleBarSettings::subdivisionLineSymbol() )
      FlagUsesSubdivisionsHeight = 1 << 13, //!< Renderer uses the scalebar subdivisions height (see QgsScaleBarSettings::subdivisionsHeight() )
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Contains parameters regarding scalebar calculations.
     * \note The need to attribute the parameters vary depending on the targeted scalebar.
     */
    struct CORE_EXPORT ScaleBarContext
    {

      /**
       * The width, in millimeters, of each individual segment drawn.
       * \note The number of map units per segment needs to be set via QgsScaleBarSettings::setUnitsPerSegment.
       */
      double segmentWidth { 0.0 };

      /**
       * Destination size for scalebar. This is used for scalebars which
       * alter their appearance or alignment based on the desired scalebar
       * size (e.g. correctly aligning text in a numeric scale bar).
       */
      QSizeF size;

      //! Scale denominator
      double scale { 1.0 };

      //! Scalebar renderer flags
      Flags flags;

      /**
       * Returns TRUE if the context has valid settings.
       *
       * \since QGIS 3.40
       */
      bool isValid() const;

    };

    QgsScaleBarRenderer() = default;
    virtual ~QgsScaleBarRenderer() = default;

    /**
     * Returns the unique name for this style.
     * \deprecated QGIS 3.40. Use id() instead.
     */
    Q_DECL_DEPRECATED QString name() const SIP_DEPRECATED { return id(); }

    /**
     * Returns the unique ID for this renderer.
     * \since QGIS 3.14
     */
    virtual QString id() const = 0;

    /**
     * Returns the user friendly, translated name for the renderer.
     * \since QGIS 3.14
     */
    virtual QString visibleName() const = 0;

    /**
     * Returns the scalebar rendering flags, which dictates the renderer's behavior.
     *
     * \since QGIS 3.14
     */
    virtual Flags flags() const;

    /**
     * Returns a sorting key value, where renderers with a lower sort key will be shown earlier in lists.
     *
     * Generally, subclasses should return QgsScaleBarRenderer::sortKey() as their sorting key.
     */
    virtual int sortKey() const;

    /**
     * Returns a clone of the renderer. The caller takes ownership of the returned value.
     */
    virtual QgsScaleBarRenderer *clone() const = 0 SIP_FACTORY;

    /**
     * Draws the scalebar using the specified \a settings and \a scaleContext to a destination render \a context.
     */
    virtual void draw( QgsRenderContext &context,
                       const QgsScaleBarSettings &settings,
                       const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const = 0;

    /**
     * Calculates the required box size (in millimeters) for a scalebar using the specified \a settings and \a scaleContext.
     * \deprecated QGIS 3.40. Use the version with a QgsRenderContext instead.
     */
    Q_DECL_DEPRECATED virtual QSizeF calculateBoxSize( const QgsScaleBarSettings &settings,
        const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const SIP_DEPRECATED;

    /**
     * Calculates the required box size (in millimeters) for a scalebar using the specified \a settings and \a scaleContext.
     *
     * \since QGIS 3.14
     */
    virtual QSizeF calculateBoxSize( QgsRenderContext &context,
                                     const QgsScaleBarSettings &settings,
                                     const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const;

    /**
     * Applies any default settings relating to the scalebar to the passed \a settings object.
     *
     * Returns TRUE if settings were applied.
     *
     * \since QGIS 3.14
     */
    virtual bool applyDefaultSettings( QgsScaleBarSettings &settings ) const;

  protected:

    /**
     * Draws default scalebar labels using the specified \a settings and \a scaleContext to a destination render \a context.
     */
    void drawDefaultLabels( QgsRenderContext &context,
                            const QgsScaleBarSettings &settings,
                            const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const;

    /**
     * Returns the text used for the first label in the scalebar.
     */
    QString firstLabelString( const QgsScaleBarSettings &settings ) const;

    /**
     * Returns the x-offset (in millimeters) used for the first label in the scalebar.
     * \deprecated QGIS 3.40. Use the version with QgsRenderContext instead.
     */
    Q_DECL_DEPRECATED double firstLabelXOffset( const QgsScaleBarSettings &settings ) const SIP_DEPRECATED;

    /**
     * Returns the x-offset (in render context painter units) used for the first label in the scalebar.
     * \since QGIS 3.2
     */
    double firstLabelXOffset( const QgsScaleBarSettings &settings, const QgsRenderContext &context, const ScaleBarContext &scaleContext ) const;

    /**
     * Returns a list of positions for each segment within the scalebar.
     * \deprecated QGIS 3.40. Use the version with a QgsRenderContext instead.
     */
    Q_DECL_DEPRECATED QList<double> segmentPositions( const QgsScaleBarRenderer::ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const SIP_DEPRECATED;

    /**
     * Returns a list of positions for each segment within the scalebar.
     * \since QGIS 3.14
     */
    QList<double> segmentPositions( QgsRenderContext &context, const QgsScaleBarRenderer::ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const;

    /**
     * Returns a list of widths of each segment of the scalebar.
     */
    QList<double> segmentWidths( const QgsScaleBarRenderer::ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsScaleBarRenderer::Flags )

#endif //QGSSCALEBARRENDERER_H
