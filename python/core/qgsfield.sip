
/**
  \class QgsField
  \brief Class to encapsulate a field in an attribute table or data source.

  QgsField stores metadata about an attribute field, including name, type
  length, and if applicable, precision.
 */

class QgsField
{

%TypeHeaderCode
#include <qgsfield.h>
%End

public:
    /** Constructor. Constructs a new QgsField object.
     * @param nam Field name
     * @param typ Field type (eg. char, varchar, text, int, serial, double). 
     Field types are usually unique to the source and are stored exactly
     as returned from the data store.
     * @param len Field length
     * @param prec Field precision. Usually decimal places but may also be
     * used in conjunction with other fields types (eg. variable character fields)
     * @param num Has to be true if field contains numeric values.
     */
  QgsField(QString nam = "", QString typ = "", int len = 0, int prec = 0, bool num = false);

  //! Destructor
   ~QgsField();

   bool operator==(const QgsField other) const;
   bool operator!=(const QgsField other) const;

  //! Gets the name of the field
  const QString & name() const;


    /** 
      Gets the field type. Field types vary depending on the data source. Examples
      are char, int, double, blob, geometry, etc. The type is stored exactly as
      the data store reports it, with no attenpt to standardize the value.
      @return QString containing the field type
     */
  const QString & type() const;


    /**
      Gets the length of the field.
      @return int containing the length of the field
     */
  int length() const;


    /**
      Gets the precision of the field. Not all field types have a related precision.
      @return int containing the precision or zero if not applicable to the field type.
     */
  int precision() const;

    /**
     Returns true if field contains numeric values. This information is set by provider.
     */
  bool isNumeric() const;


    /**
      Set the field name.
      @param nam Name of the field
     */
  void setName(const QString & nam);

    /**
      Set the field type.
      @param typ Field type
     */
  void setType(const QString & typ);

    /**
      Set the field length.
      @param len Length of the field
     */
  void setLength(int len);

    /**
      Set the field precision.
      @param prec Precision of the field
     */
  void setPrecision(int prec);

    /**
      Set whether field is numeric
      */
  void setNumeric(bool num);

}; // class QgsField

