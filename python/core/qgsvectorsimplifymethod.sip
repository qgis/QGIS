
/** This class contains information how to simplify geometries fetched from a vector layer
 * @note added in 2.2
 */
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

    /** Simplification flags for fast rendering of features */
    enum SimplifyHint
    {
      NoSimplification,           //!< No simplification can be applied
      GeometrySimplification,     //!< The geometries can be simplified using the current map2pixel context state
      AntialiasingSimplification, //!< The geometries can be rendered with 'AntiAliasing' disabled because of it is '1-pixel size'
      FullSimplification,         //!< All simplification hints can be applied ( Geometry + AA-disabling )
    };
    typedef QFlags<QgsVectorSimplifyMethod::SimplifyHint> SimplifyHints;

    /** Sets the simplification hints of the vector layer managed */
    void setSimplifyHints( QFlags<QgsVectorSimplifyMethod::SimplifyHint> simplifyHints );
    /** Gets the simplification hints of the vector layer managed */
    QFlags<QgsVectorSimplifyMethod::SimplifyHint> simplifyHints() const;

    /** Sets the simplification threshold of the vector layer managed */
    void setThreshold( float threshold );
    /** Gets the simplification threshold of the vector layer managed */
    float threshold() const;

    /** Sets where the simplification executes, after fetch the geometries from provider, or when supported, in provider before fetch the geometries */
    void setForceLocalOptimization( bool localOptimization );
    /** Gets where the simplification executes, after fetch the geometries from provider, or when supported, in provider before fetch the geometries */
    bool forceLocalOptimization() const;

    /** Sets the maximum scale at which the layer should be simplified */
    void setMaximumScale( float maximumScale );
    /** Gets the maximum scale at which the layer should be simplified */
    float maximumScale() const;
};

QFlags<QgsVectorSimplifyMethod::SimplifyHint> operator|( QgsVectorSimplifyMethod::SimplifyHint f1, QFlags<QgsVectorSimplifyMethod::SimplifyHint> f2 );
