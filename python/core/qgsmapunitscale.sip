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

    /** Constructor for QgsMapUnitScale
     * @param minScale minimum allowed scale, or 0.0 if no minimum scale set
     * @param maxScale maximum allowed scale, or 0.0 if no maximum scale set
     */
    QgsMapUnitScale( double minScale = 0.0, double maxScale = 0.0 );

    /** The minimum scale, or 0.0 if unset */
    double minScale;
    /** The maximum scale, or 0.0 if unset */
    double maxScale;

    /** Whether the minimum size in mm should be respected */
    bool minSizeMMEnabled;
    /** The minimum size in millimeters, or 0.0 if unset */
    double minSizeMM;
    /** Whether the maximum size in mm should be respected */
    bool maxSizeMMEnabled;
    /** The maximum size in millimeters, or 0.0 if unset */
    double maxSizeMM;

    /** Computes a map units per pixel scaling factor, respecting the minimum and maximum scales
     * set for the object.
     * @param c render context
     * @returns map units per pixel, limited between minimum and maximum scales
     */
    double computeMapUnitsPerPixel( const QgsRenderContext& c ) const;

    bool operator==( const QgsMapUnitScale& other ) const;
    bool operator!=( const QgsMapUnitScale& other ) const;
};
