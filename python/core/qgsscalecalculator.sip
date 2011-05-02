
/**
 * \class QgsScaleCalculator
 * \brief Calculates scale for a given combination of canvas size, map extent,
 * and monitor dpi.
 */
class QgsScaleCalculator
{
%TypeHeaderCode
#include <qgsscalecalculator.h>
%End
  
  public:

    /**
     * Constructor
     * @param dpi Monitor resolution in dots per inch
     * @param mapUnits Units of the data on the map. Must match a value from the
     * QGis::UnitType enum (Meters, Feet, Degrees)
     */
    QgsScaleCalculator(int dpi = 0, 
                       QGis::UnitType mapUnits = QGis::Meters);

    //! Destructor
    ~QgsScaleCalculator();

    /**
     * Set the dpi to be used in scale calculations
     * @param dpi Dots per inch of monitor resolution
     */
    void setDpi(int dpi);

    /**
     * Accessor for dpi used in scale calculations
     * @return int the dpi used for scale calculations.
     */
    int dpi();
    
    /**
     * Set the map units
     * @param mapUnits Units of the data on the map. Must match a value from the
     */
    void setMapUnits(QGis::UnitType mapUnits);

    /** Returns current map units */
    QGis::UnitType mapUnits() const;
    
    /**
     * Calculate the scale
     * @param mapExtent QgsRectangle containing the current map extent
     * @param canvasWidth Width of the map canvas in pixel (physical) units
     * @return scale of current map view
     */
    double calculate(QgsRectangle &mapExtent, int canvasWidth);

    /**
     * Calculate the distance between to points in geographic coordinates.
     * Used to calculate scale for map views with geographic (decimal degree)
     * data.
     * @param mapExtent QgsRectangle containing the current map extent
     */
    double calculateGeographicDistance(QgsRectangle &mapExtent);

};

