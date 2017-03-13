%ModuleHeaderCode
#include <qgsnetworkspeedstrategy.h>
#include <qgsnetworkdistancestrategy.h>
%End

/**
 * \ingroup analysis
 * \class QgsNetworkStrategy
 * \brief QgsNetworkStrategy defines strategy used for calculation of the edge cost. For example it can
 * take into account travel distance, amount of time or money. Currently there are two strategies
 * implemented in the analysis library: QgsNetworkDistanceStrategy and QgsNetworkSpeedStrategy.
 * QgsNetworkStrategy implemented using "strategy" design pattern.
 */
class QgsNetworkStrategy
{
%TypeHeaderCode
#include <qgsnetworkstrategy.h>
%End

%ConvertToSubClassCode
  if ( dynamic_cast< QgsNetworkDistanceStrategy* > ( sipCpp ) != NULL )
    sipType = sipType_QgsNetworkDistanceStrategy;
  else if ( dynamic_cast< QgsNetworkSpeedStrategy* > ( sipCpp ) != NULL )
    sipType = sipType_QgsNetworkSpeedStrategy;
  else
    sipType = NULL;
%End


  public:

    /**
     * Default constructor
     */
    QgsNetworkStrategy();

    virtual ~QgsNetworkStrategy();

    /**
     * Returns list of the source layer attributes needed for cost calculation.
     * This method called by QgsGraphDirector.
     * \return list of required attributes
     */
    virtual QgsAttributeList requiredAttributes() const;

    /**
     * Returns edge cost
     */
    virtual QVariant cost( double distance, const QgsFeature &f ) const = 0;
};
