
class QgsTolerance
{
%TypeHeaderCode
#include <qgstolerance.h>
%End

public:
    /**Type of unit of tolerance value from settings*/
    enum UnitType 
    { 
      /**Map unit value*/
      MapUnits,
      /**Pixels unit of tolerance*/
      Pixels
    };

    /**
    * Static function to get vertex tolerance value from settings
    * @param mapUnitsPerPixel number of map units per pixel
    * @return value of vertex tolerance in map units
    */
    static double vertexSearchRadius( double mapUnitsPerPixel );

    /**
    * Static function to get default tolerance value from settings
    * @param mapUnitsPerPixel number of map units per pixel
    * @return value of default tolerance in map units
    */
    static double defaultTolerance( double mapUnitsPerPixel );

    /** 
    * Static function to translate tolerance value into current map unit value
    * @param tolerace tolerance value to be translated
    * @param mapUnitsPerPixel number of map units per pixel
    * @param units type of units to be translated
    * @return value of tolerance in map units
    */
    static double toleranceInMapUnits(double tolerance, double mapUnitsPerPixel, UnitType units = MapUnits);

};

