#ifndef QGSPAINTING_H
#define QGSPAINTING_H

#include <QPainter>

/**
 * @ingroup core
 * Misc painting enums and functions.
 *
 * @note added in QGIS 3.0
 */
class CORE_EXPORT QgsPainting
{
  public:

    /** Blending modes enum defining the available composition modes that can
     * be used when rendering a layer
     */
    enum BlendMode
    {
      BlendNormal,
      BlendLighten,
      BlendScreen,
      BlendDodge,
      BlendAddition,
      BlendDarken,
      BlendMultiply,
      BlendBurn,
      BlendOverlay,
      BlendSoftLight,
      BlendHardLight,
      BlendDifference,
      BlendSubtract,
      BlendSource,
      BlendDestinationOver,
      BlendClear,
      BlendDestination,
      BlendSourceIn,
      BlendDestinationIn,
      BlendSourceOut,
      BlendDestinationOut,
      BlendSourceAtop,
      BlendDestinationAtop,
      BlendXor,
    };

    //! Returns a QPainter::CompositionMode corresponding to a BlendMode
    static QPainter::CompositionMode getCompositionMode( BlendMode blendMode );
    //! Returns a BlendMode corresponding to a QPainter::CompositionMode
    static BlendMode getBlendModeEnum( QPainter::CompositionMode blendMode );

};

#endif // QGSPAINTING_H
