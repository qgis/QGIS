%ModuleHeaderCode
#include <qgsspeedstrategy.h>
#include <qgsdistancestrategy.h>
%End

/**
 * \ingroup analysis
 * \class QgsStrategy
 * \brief QgsStrategy defines strategy used for calculation of the edge cost. For example it can
 * take into account travel distance, amount of time or money. Currently there are two strategies
 * implemented in the analysis library: QgsDistanceStrategy and QgsSpeedStrategy.
 * QgsStrategy implemented with strategy design pattern.
 */
class QgsStrategy
{
%TypeHeaderCode
#include <qgsstrategy.h>
%End

%ConvertToSubClassCode
  if ( dynamic_cast< QgsDistanceStrategy* > ( sipCpp ) != NULL )
    sipType = sipType_QgsStrategy;
  else if ( dynamic_cast< QgsSpeedStrategy* > ( sipCpp ) != NULL )
    sipType = sipType_QgsSpeedStrategy;
  else
    sipType = NULL;
%End


  public:

    /**
     * Default constructor
     */
    QgsStrategy();

    virtual ~QgsStrategy();

    /**
     * Returns list of the source layer attributes needed for cost calculation.
     * This method called by QgsGraphDirector.
     * \return list of required attributes
     */
    virtual QgsAttributeList requiredAttributes() const;

    /**
     * Return edge cost
     */
    virtual QVariant cost( double distance, const QgsFeature &f ) const = 0;
};
