
/** \class QgsFeatureAttribute - Feature attribute class.
 * \brief Encapsulates a single feature attribute.
  *@author Gary E.Sherman
  */

class QgsFeatureAttribute
{
%TypeHeaderCode
#include <qgsfeatureattribute.h>
%End

public:

  //! Constructor
  QgsFeatureAttribute(const QString & field = 0, const QString & value = 0, bool num = false);

  //! Destructor
  ~QgsFeatureAttribute();

  /**
	* Get the field name for this feature attribute
	* @return Field name
	*/
  const QString & fieldName() const;


  /**
	* Get the field value for this feature attribute
	* @return Field value
	*/
  const QString & fieldValue() const;

  /**
  * Returns whether value is numeric or not
  * @return Numeric flag
  */
  bool isNumeric() const;

  void setFieldName(QString name);

  void setFieldValue(QString thevalue);

  void setNumeric(bool num = true);

  SIP_PYOBJECT __repr__();
%MethodCode
    QString str = sipCpp->fieldName() + " = " + sipCpp->fieldValue();
    sipRes = PyString_FromString(str.toLocal8Bit().data());
%End

}; // class QgsFeatureAttribute

