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

    /** Types of distance units
     */
    enum DistanceUnitType
    {
      Standard,    /*!< unit is a standard measurement unit */
      Geographic,  /*!< unit is a geographic (eg degree based) unit */
      UnknownType, /*!< unknown unit type */
    };

    //! Units of area
    enum AreaUnit
    {
      SquareMeters, /*!< square meters */
      SquareKilometers, /*!< square kilometers */
      SquareFeet, /*!< square feet */
      SquareYards, /*!< square yards */
      SquareMiles, /*!< square miles */
      Hectares, /*!< hectares */
      Acres, /*!< acres */
      SquareNauticalMiles, /*!< square nautical miles */
      SquareDegrees, /*!< square degrees, for planar geographic CRS area measurements */
      UnknownAreaUnit, /*!< unknown areal unit */
    };

    //! Units of angles
    enum AngleUnit
    {
      AngleDegrees = 0, /*!< degrees */
      Radians, /*!< square kilometers */
      Gon, /*!< gon/gradian */
      MinutesOfArc, /*!< minutes of arc */
      SecondsOfArc, /*!< seconds of arc */
      Turn, /*!< turn/revolutions */
      UnknownAngleUnit, /*!< unknown angle unit */
    };

    /** Returns the type for a distance unit.
     */
    static DistanceUnitType unitType( QGis::UnitType unit );

    /** Encodes a distance unit to a string.
     * @param unit unit to encode
     * @returns encoded string
     * @see decodeDistanceUnit()
     */
    static QString encodeUnit( QGis::UnitType unit );

    /** Decodes a distance unit from a string.
     * @param string string to decode
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @returns decoded units
     * @see encodeUnit()
     */
    static QGis::UnitType decodeDistanceUnit( const QString& string, bool *ok = 0 );

    /** Returns a translated string representing a distance unit.
     * @param unit unit to convert to string
     * @see stringToDistanceUnit()
     */
    static QString toString( QGis::UnitType unit );

    /** Converts a translated string to a distance unit.
     * @param string string representing a distance unit
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @see toString()
     */
    static QGis::UnitType stringToDistanceUnit( const QString& string, bool *ok = 0 );

    /** Returns the conversion factor between the specified distance units.
     * @param fromUnit distance unit to convert from
     * @param toUnit distance unit to convert to
     * @returns multiplication factor to convert between units
     */
    static double fromUnitToUnitFactor( QGis::UnitType fromUnit, QGis::UnitType toUnit );

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
    static AreaUnit decodeAreaUnit( const QString& string, bool *ok = 0 );

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
    static AreaUnit stringToAreaUnit( const QString& string, bool *ok = 0 );

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
    static AreaUnit distanceToAreaUnit( QGis::UnitType distanceUnit );

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
    static AngleUnit decodeAngleUnit( const QString& string, bool *ok = 0 );

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
};

