
class QgsTolerance
{
%TypeHeaderCode
#include <qgstolerance.h>
%End

  public:
    /** Type of unit of tolerance value from settings.
     * MapUnits is slightly confusing, because it actually refers to layer units (historically).
     * For map (project) units, use ProjectUnits. Try to avoid using MapUnits value and use LayerUnits instead. */
    enum UnitType
    {
      /** Layer unit value. @note deprecated: use LayerUnits */
      MapUnits,
      /** Layer unit value */
      LayerUnits,
      /** Pixels unit of tolerance*/
      Pixels,
      /** Map (project) units. Added in 2.8 */
      ProjectUnits
    };

    /**
     * Static function to get vertex tolerance value.
     * The value is read from settings and transformed if necessary.
     * @return value of vertex tolerance in map units (not layer units)
     * @note added in 2.8
     */
    static double vertexSearchRadius( const QgsMapSettings& mapSettings );

    /**
    * Static function to get vertex tolerance value for a layer.
    * The value is read from settings and transformed if necessary.
    * @return value of vertex tolerance in layer units
    */
    static double vertexSearchRadius( QgsMapLayer* layer, const QgsMapSettings& mapSettings );

    /**
    * Static function to get vertex tolerance value for a layer.
    * The value is read from settings and transformed if necessary.
    * @return value of vertex tolerance in layer units
    */
    //! @deprecated since 2.4 - use override with QgsMapSettings
    static double vertexSearchRadius( QgsMapLayer* layer, QgsMapRenderer* renderer ) /Deprecated/;

    /**
    * Static function to get default tolerance value for a layer.
    * The value is read from settings and transformed if necessary.
    * @return value of vertex tolerance in layer units
    */
    static double defaultTolerance( QgsMapLayer* layer, const QgsMapSettings& mapSettings );

    /**
    * Static function to get default tolerance value for a layer.
    * The value is read from settings and transformed if necessary.
    * @return value of vertex tolerance in layer units
    */
    //! @deprecated since 2.4 - use override with QgsMapSettings
    static double defaultTolerance( QgsMapLayer* layer, QgsMapRenderer* renderer ) /Deprecated/;

    /**
    * Static function to translate tolerance value into map units
    * @param tolerance tolerance value to be translated
    * @param layer source layer necessary in case tolerance is in layer units
    * @param mapSettings settings of the map
    * @param units type of units to be translated
    * @return value of tolerance in map units
    * @note added in 2.8
    */
    static double toleranceInProjectUnits( double tolerance, QgsMapLayer* layer, const QgsMapSettings& mapSettings, QgsTolerance::UnitType units );

    /**
    * Static function to translate tolerance value into layer units
    * @param tolerance tolerance value to be translated
    * @param layer reference layer
    * @param mapSettings settings of the map
    * @param units type of units to be translated
    * @return value of tolerance in layer units
    */
    static double toleranceInMapUnits( double tolerance, QgsMapLayer* layer, const QgsMapSettings& mapSettings, UnitType units = LayerUnits );

    /**
    * Static function to translate tolerance value into layer units
    * @param tolerance tolerance value to be translated
    * @param layer reference layer
    * @param renderer renderer
    * @param units type of units to be translated
    * @return value of tolerance in layer units
    */
    //! @deprecated since 2.4 - use the override with QgsMapSettings
    static double toleranceInMapUnits( double tolerance, QgsMapLayer* layer, QgsMapRenderer* renderer, UnitType units = LayerUnits ) /Deprecated/;
};
