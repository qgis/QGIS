
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
     * @param name Field name
     * @param type Field variant type, currently supported: String / Int / Double
     * @param typeName Field type (eg. char, varchar, text, int, serial, double).
     Field types are usually unique to the source and are stored exactly
     as returned from the data store.
     * @param len Field length
     * @param prec Field precision. Usually decimal places but may also be
     * used in conjunction with other fields types (eg. variable character fields)
     * @param comment Comment for the field
     */

    QgsField( QString name = QString(),
              QVariant::Type type = QVariant::Invalid,
              QString typeName = QString(),
              int len = 0,
              int prec = 0,
              QString comment = QString() );

    //! Destructor
    ~QgsField();

    bool operator==( const QgsField& other ) const;
    bool operator!=( const QgsField& other ) const;

    //! Gets the name of the field
    const QString & name() const;

    //! Gets variant type of the field as it will be retrieved from data source
    QVariant::Type type() const;

    /**
      Gets the field type. Field types vary depending on the data source. Examples
      are char, int, double, blob, geometry, etc. The type is stored exactly as
      the data store reports it, with no attenpt to standardize the value.
      @return QString containing the field type
     */
    const QString & typeName() const;


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
    Returns the field comment
    */
    const QString & comment() const;

    /**
      Set the field name.
      @param nam Name of the field
     */
    void setName( const QString & nam );

    /**
      Set variant type.
     */
    void setType( QVariant::Type type );

    /**
      Set the field type.
      @param typ Field type
     */
    void setTypeName( const QString & typ );

    /**
      Set the field length.
      @param len Length of the field
     */
    void setLength( int len );

    /**
      Set the field precision.
      @param prec Precision of the field
     */
    void setPrecision( int prec );


    /**
      Set the field comment
      */
    void setComment( const QString & comment );

    /**Formats string for display*/
    QString displayString( const QVariant& v ) const;
}; // class QgsField



class QgsFields
{
%TypeHeaderCode
#include <qgsfield.h>
%End
  public:

    enum FieldOrigin { OriginUnknown, OriginProvider, OriginJoin, OriginEdit };

    void clear();
    void append( const QgsField& field, FieldOrigin origin = OriginProvider, int originIndex = -1 );
    void remove( int fieldIdx );

    bool isEmpty() const;
    // __len__ annotation since sip 4.10.3
    //int count() const /__len__/;
    int count() const;
    int __len__() const;
%MethodCode
  sipRes = sipCpp->count();
%End
    int size() const;
    //const QgsField& operator[](int i) const;
    //QgsField& operator[](int i);
    const QgsField& at(int i) const;
    QList<QgsField> toList() const;

    const QgsField& field( int fieldIdx ) const;
    const QgsField& field( const QString& name ) const;
    FieldOrigin fieldOrigin( int fieldIdx ) const;
    int fieldOriginIndex( int fieldIdx ) const;

    int indexFromName( const QString& name ) const;
    void extend( const QgsFields& other );


    QgsField& operator[](int i) /Factory/;
%MethodCode
  SIP_SSIZE_T idx = sipConvertFromSequenceIndex(a0, sipCpp->count());
  if (idx < 0)
    sipIsErr = 1;
  else
    sipRes = new QgsField(sipCpp->operator[](idx));

%End

/*  SIP_PYOBJECT __getitem__(int key);
%MethodCode
  if (a0 = sipConvertFromSequenceIndex(a0, sipCpp->count()) < 0)
    sipIsErr = 1;
  else
  {
    qDebug("__getitem__ %d", a0);
    QgsField* fld = new QgsField(sipCpp->at(a0));
    sipRes = sipConvertFromInstance(fld, sipClass_QgsField, Py_None);
  }
%End*/

void __setitem__(int key, const QgsField& field);
%MethodCode
  int idx = (int)sipConvertFromSequenceIndex(a0, sipCpp->count());
  if (idx < 0)
    sipIsErr = 1;
  else
    (*sipCpp)[idx] = *a1;
%End

};

