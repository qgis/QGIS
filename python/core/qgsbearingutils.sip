/**
 * \class QgsBearingUtils
 * \ingroup core
 * Utilities for calculating bearings and directions.
 * \note Added in version 2.18
*/
class QgsBearingUtils
{
%TypeHeaderCode
#include <qgsbearingutils.h>
%End
  public:

    /**
     * Returns the direction to true north from a specified point and for a specified
     * coordinate reference system. The returned value is in degrees clockwise from
     * vertical. An exception will be thrown if the bearing could not be calculated.
     */
    static double bearingTrueNorth( const QgsCoordinateReferenceSystem& crs,
                                    const QgsPoint& point );
};
