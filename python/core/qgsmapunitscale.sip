/*!
 * \class QgsMapUnitScale
 * \brief Struct for storing maximum and minimum scales for measurements in map units
 *
 * For measurements in map units, a minimum and a maximum scale can be defined.
 * Outside this range, the measurements aren't scaled anymore proportionally to
 * the map scale.
 */

class QgsMapUnitScale
{
%TypeHeaderCode
#include <qgsmapunitscale.h>
%End
  public:
    QgsMapUnitScale();
    QgsMapUnitScale( float _minScale, float _maxScale );

    /** The minimum scale, or 0.0 if unset */
    double minScale;
    /** The maximum scale, or 0.0 if unset */
    double maxScale;

    double computeMapUnitsPerPixel( const QgsRenderContext& c ) const;
};
