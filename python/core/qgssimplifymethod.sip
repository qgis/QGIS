
/** This class contains information about how to simplify geometries fetched from a QgsFeatureIterator
 * @note added in 2.2
 */
class QgsSimplifyMethod
{
%TypeHeaderCode
#include "qgssimplifymethod.h"
%End

  public:
    enum MethodType
    {
      NoSimplification,     //!< No simplification is applied
      OptimizeForRendering, //!< Simplify using the map2pixel data to optimize the rendering of geometries
      PreserveTopology      //!< Simplify using the Douglas-Peucker algorithm ensuring that the result is a valid geometry
    };

    //! construct a default method
    QgsSimplifyMethod();
    //! copy constructor
    QgsSimplifyMethod( const QgsSimplifyMethod& rh );

    //! Sets the simplification type
    void setMethodType( MethodType methodType );
    //! Gets the simplification type
    MethodType methodType() const;

    //! Sets the tolerance of simplification. Represents the maximum distance between two coordinates which can be considered equal
    void setTolerance( double tolerance );
    //! Gets the tolerance of simplification
    double tolerance() const;

    //! Sets whether the simplification executes after fetch the geometries from provider, otherwise it executes, when supported, in provider before fetch the geometries
    void setForceLocalOptimization( bool localOptimization );
    //! Gets whether the simplification executes after fetch the geometries from provider, otherwise it executes, when supported, in provider before fetch the geometries
    bool forceLocalOptimization() const;

    //! Creates a geometry simplifier according to specified method
    static QgsAbstractGeometrySimplifier* createGeometrySimplifier( const QgsSimplifyMethod& simplifyMethod );
};
