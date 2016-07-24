class QgsPainting
{
%TypeHeaderCode
#include <qgspainting.h>
%End

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
    static QPainter::CompositionMode getCompositionMode( QgsPainting::BlendMode blendMode );
    //! Returns a BlendMode corresponding to a QPainter::CompositionMode
    static QgsPainting::BlendMode getBlendModeEnum( QPainter::CompositionMode blendMode );

};
