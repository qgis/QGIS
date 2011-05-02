
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
    * Static function to get vertex tolerance value for a layer.
    * The value is read from settings and transformed if necessary.
    * @return value of vertex tolerance in map units
    */
    static double vertexSearchRadius( QgsMapLayer* layer, QgsMapRenderer* renderer );

    /**
    * Static function to get default tolerance value for a layer.
    * The value is read from settings and transformed if necessary.
    * @return value of default tolerance in map units
    */
    static double defaultTolerance( QgsMapLayer* layer, QgsMapRenderer* renderer );

    /**
    * Static function to translate tolerance value into current map unit value
    * @param tolerace tolerance value to be translated
    * @param units type of units to be translated
    * @return value of tolerance in map units
    */
    static double toleranceInMapUnits( double tolerance, QgsMapLayer* layer, QgsMapRenderer* renderer, UnitType units = MapUnits );

};

