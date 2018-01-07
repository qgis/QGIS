/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspainting.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsPainting
{
%Docstring
Misc painting enums and functions.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgspainting.h"
%End
  public:

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

    static QPainter::CompositionMode getCompositionMode( QgsPainting::BlendMode blendMode );
%Docstring
Returns a QPainter.CompositionMode corresponding to a BlendMode
%End
    static QgsPainting::BlendMode getBlendModeEnum( QPainter::CompositionMode blendMode );
%Docstring
Returns a BlendMode corresponding to a QPainter.CompositionMode
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspainting.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
