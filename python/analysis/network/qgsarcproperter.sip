%ModuleHeaderCode
// fix to allow compilation with sip 4.7 that for some reason
// doesn't add these includes to the file where the code from
// ConvertToSubClassCode goes.
#include <qgsdistancearcproperter.h>
%End

/**
 * \ingroup networkanalysis
 * \class QgsEdgeProperter
 * \brief QgsEdgeProperter is a strategy pattern.
 * You can use it for customize arc property. For example look at QgsDistanceArcProperter or src/plugins/roadgraph/speedproperter.h
 */
class QgsArcProperter
{
%TypeHeaderCode
#include <qgsarcproperter.h>
%End

%ConvertToSubClassCode
  if ( dynamic_cast< QgsDistanceArcProperter* > ( sipCpp ) != NULL )
    sipClass = sipClass_QgsDistanceArcProperter;
  else
    sipClass = NULL;
%End

  public:
    /**
     * default constructor
     */
    QgsArcProperter();

    /**
     * QgsGraphDirector call this method for fetching attribute from source layer
     * \return required attributes list
     */
    virtual QgsAttributeList requiredAttributes() const;

    /**
     * calculate and return adge property
     */
    virtual QVariant property( double distance, const QgsFeature &f ) const;
};
