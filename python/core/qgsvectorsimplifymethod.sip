
/** This class contains information how to simplify geometries fetched from a vector layer */
class QgsVectorSimplifyMethod
{
%TypeHeaderCode
#include "qgsvectorsimplifymethod.h"
%End

  public:
    //! construct a default object
    QgsVectorSimplifyMethod();
    //! copy constructor
    QgsVectorSimplifyMethod( const QgsVectorSimplifyMethod& rh );

    /** Sets the simplification hints of the vector layer managed */
    void setSimplifyHints( int simplifyHints );
    /** Gets the simplification hints of the vector layer managed */
    int simplifyHints() const;

    /** Sets the simplification threshold of the vector layer managed */
    void setThreshold( float threshold );
    /** Gets the simplification threshold of the vector layer managed */
    float threshold() const;

    /** Sets where the simplification executes, after fetch the geometries from provider, or when supported, in provider before fetch the geometries */
    void setForceLocalOptimization( bool localOptimization );
    /** Gets where the simplification executes, after fetch the geometries from provider, or when supported, in provider before fetch the geometries */
    bool forceLocalOptimization();
};
