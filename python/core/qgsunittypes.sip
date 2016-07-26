/** \ingroup core
 * \class QgsUnitTypes
 * \brief Helper functions for various unit types.
 * \note Added in version 2.14
 */

class QgsUnitTypes
{
%TypeHeaderCode
#include <qgsunittypes.h>
%End

  public:

    //! Units of distance
    enum DistanceUnit
    {
      DistanceMeters, /*!< meters */
      DistanceKilometers, /*!< kilometers */
      DistanceFeet, /*!< imperial feet */
      DistanceNauticalMiles, /*!< nautical miles */
      DistanceYards, /*!< imperial yards */
      DistanceMiles, /*!< terrestial miles */
      DistanceDegrees, /*!< degrees, for planar geographic CRS distance measurements */
      DistanceUnknownUnit, /*!< unknown distance unit */
    };

    /** Types of distance units
     */
    enum DistanceUnitType
    {
      Standard, /*!< unit is a standard measurement unit */
      Geographic,   /*!< unit is a geographic (eg degree based) unit */
      UnknownType,  /*!< unknown unit type */
    };

    //! Units of area
    enum AreaUnit
    {
      AreaSquareMeters, /*!< square meters */
      AreaSquareKilometers, /*!< square kilometers */
      AreaSquareFeet, /*!< square feet */
      AreaSquareYards, /*!< square yards */
      AreaSquareMiles, /*!< square miles */
      AreaHectares, /*!< hectares */
      AreaAcres, /*!< acres */
      AreaSquareNauticalMiles, /*!< square nautical miles */
      AreaSquareDegrees, /*!< square degrees, for planar geographic CRS area measurements */
      AreaUnknownUnit, /*!< unknown areal unit */
    };

    //! Units of angles
    enum AngleUnit
    {
      AngleDegrees, /*!< degrees */
      AngleRadians, /*!< square kilometers */
      AngleGon, /*!< gon/gradian */
      AngleMinutesOfArc, /*!< minutes of arc */
      AngleSecondsOfArc, /*!< seconds of arc */
      AngleTurn, /*!< turn/revolutions */
      AngleUnknownUnit, /*!< unknown angle unit */
    };

    //! Rendering size units
    enum RenderUnit
    {
      RenderMillimeters, //!< millimeters
      RenderMapUnits, //!< map units
      RenderPixels, //!< pixels
      RenderPercentage, //!< percentage of another measurement (eg canvas size, feature size)
      RenderUnknownUnit, //!< mixed or unknown units
    };

    //! List of render units
    typedef QList<QgsUnitTypes::RenderUnit> RenderUnitList;

    /** Returns the type for a distance unit.
     */
    static DistanceUnitType unitType( QgsUnitTypes::DistanceUnit unit );

    /** Encodes a distance unit to a string.
     * @param unit unit to encode
     * @returns encoded string
     * @see decodeDistanceUnit()
     */
    static QString encodeUnit( QgsUnitTypes::DistanceUnit unit );

    /** Decodes a distance unit from a string.
     * @param string string to decode
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @returns decoded units
     * @see encodeUnit()
     */
    static QgsUnitTypes::DistanceUnit decodeDistanceUnit( const QString& string, bool *ok /Out/ = 0 );

    /** Returns a translated string representing a distance unit.
     * @param unit unit to convert to string
     * @see stringToDistanceUnit()
     */
    static QString toString( QgsUnitTypes::DistanceUnit unit );

    /** Converts a translated string to a distance unit.
     * @param string string representing a distance unit
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @see toString()
     */
    static QgsUnitTypes::DistanceUnit stringToDistanceUnit( const QString& string, bool *ok /Out/ = 0 );

    /** Returns the conversion factor between the specified distance units.
     * @param fromUnit distance unit to convert from
     * @param toUnit distance unit to convert to
     * @returns multiplication factor to convert between units
     */
    static double fromUnitToUnitFactor( QgsUnitTypes::DistanceUnit fromUnit, QgsUnitTypes::DistanceUnit toUnit );

    /** Returns the type for an areal unit.
    */
    static DistanceUnitType unitType( AreaUnit unit );

    /** Encodes an areal unit to a string.
     * @param unit unit to encode
     * @returns encoded string
     * @see decodeAreaUnit()
    */
    static QString encodeUnit( AreaUnit unit );

    /** Decodes an areal unit from a string.
     * @param string string to decode
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @returns decoded units
     * @see encodeUnit()
    */
    static AreaUnit decodeAreaUnit( const QString& string, bool *ok /Out/ = 0 );

    /** Returns a translated string representing an areal unit.
     * @param unit unit to convert to string
     * @see stringToAreaUnit()
     */
    static QString toString( AreaUnit unit );

    /** Converts a translated string to an areal unit.
     * @param string string representing an areal unit
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @see toString()
     */
    static AreaUnit stringToAreaUnit( const QString& string, bool *ok /Out/ = 0 );

    /** Returns the conversion factor between the specified areal units.
     * @param fromUnit area unit to convert from
     * @param toUnit area unit to convert to
     * @returns multiplication factor to convert between units
     */
    static double fromUnitToUnitFactor( AreaUnit fromUnit, AreaUnit toUnit );

    /** Converts a distance unit to its corresponding area unit, eg meters to square meters
     * @param distanceUnit distance unit to convert
     * @return matching areal unit
     */
    static AreaUnit distanceToAreaUnit( QgsUnitTypes::DistanceUnit distanceUnit );

    // ANGULAR UNITS

    /** Encodes an angular unit to a string.
     * @param unit unit to encode
     * @returns encoded string
     * @see decodeAngleUnit()
    */
    static QString encodeUnit( AngleUnit unit );

    /** Decodes an angular unit from a string.
     * @param string string to decode
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @returns decoded units
     * @see encodeUnit()
    */
    static AngleUnit decodeAngleUnit( const QString& string, bool *ok /Out/ = 0 );

    /** Returns a translated string representing an angular unit.
     * @param unit unit to convert to string
     */
    static QString toString( AngleUnit unit );

    /** Returns the conversion factor between the specified angular units.
     * @param fromUnit angle unit to convert from
     * @param toUnit angle unit to convert to
     * @returns multiplication factor to convert between units
     */
    static double fromUnitToUnitFactor( AngleUnit fromUnit, AngleUnit toUnit );

    /** Returns an angle formatted as a friendly string.
     * @param angle angle to format
     * @param decimals number of decimal places to show
     * @param unit unit of angle
     * @returns formatted angle string
     */
    static QString formatAngle( double angle, int decimals, AngleUnit unit );

    // RENDER UNITS

    /** Encodes a render unit to a string.
     * @param unit unit to encode
     * @returns encoded string
     * @see decodeRenderUnit()
     */
    static QString encodeUnit( RenderUnit unit );

    /** Decodes a render unit from a string.
     * @param string string to decode
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @returns decoded units
     * @see encodeUnit()
     */
    static RenderUnit decodeRenderUnit( const QString& string, bool *ok /Out/ = 0 );

};

