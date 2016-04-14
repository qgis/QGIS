class QgsAbstractFeatureIterator
{
%TypeHeaderCode
#include <qgsfeatureiterator.h>
%End
  public:

    //! Status of expression compilation for filter expression requests
    enum CompileStatus
    {
      NoCompilation, /*!< Expression could not be compiled or not attempt was made to compile expression */
      PartiallyCompiled, /*!< Expression was partially compiled, but extra checks need to be applied to features*/
      Compiled, /*!< Expression was fully compiled and delegated to data provider source*/
    };

    //! base class constructor - stores the iteration parameters
    QgsAbstractFeatureIterator( const QgsFeatureRequest& request );

    //! destructor makes sure that the iterator is closed properly
    virtual ~QgsAbstractFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& f );

    //! reset the iterator to the starting position
    virtual bool rewind() = 0;
    //! end of iterating: free the resources / lock
    virtual bool close() = 0;

    /** Returns the status of expression compilation for filter expression requests.
     * @note added in QGIS 2.16
     */
    CompileStatus compileStatus() const;

  protected:
    /**
     * If you write a feature iterator for your provider, this is the method you
     * need to implement!!
     *
     * @param f The feature to write to
     * @return  true if a feature was written to f
     */
    virtual bool fetchFeature( QgsFeature& f ) = 0;

    /**
     * By default, the iterator will fetch all features and check if the feature
     * matches the expression.
     * If you have a more sophisticated metodology (SQL request for the features...)
     * and you check for the expression in your fetchFeature method, you can just
     * redirect this call to fetchFeature so the default check will be omitted.
     *
     * @param f The feature to write to
     * @return  true if a feature was written to f
     */
    virtual bool nextFeatureFilterExpression( QgsFeature &f );

    /**
     * By default, the iterator will fetch all features and check if the id
     * is in the request.
     * If you have a more sophisticated metodology (SQL request for the features...)
     * and you are sure, that any feature you return from fetchFeature will match
     * if the request was FilterFids you can just redirect this call to fetchFeature
     * so the default check will be omitted.
     *
     * @param f The feature to write to
     * @return  true if a feature was written to f
     */
    virtual bool nextFeatureFilterFids( QgsFeature & f );

    void ref(); //!< add reference
    void deref(); //!< remove reference, delete if refs == 0

    //! Setup the simplification of geometries to fetch using the specified simplify method
    virtual bool prepareSimplification( const QgsSimplifyMethod& simplifyMethod );
};


class QgsFeatureIterator
{
%TypeHeaderCode
#include <qgsfeatureiterator.h>
%End

  public:
    QgsFeatureIterator* __iter__();
%MethodCode
  sipRes = sipCpp;
%End

  SIP_PYOBJECT __next__();
%MethodCode
  QgsFeature* f = new QgsFeature;
  if (sipCpp->nextFeature(*f))
    sipRes = sipConvertFromType(f, sipType_QgsFeature, Py_None);
  else
  {
    delete f;
    PyErr_SetString(PyExc_StopIteration,"");
  }
%End

    //! construct invalid iterator
    QgsFeatureIterator();
    //! construct a valid iterator
    // QgsFeatureIterator( QgsAbstractFeatureIterator* iter );
    //! copy constructor copies the iterator, increases ref.count
    QgsFeatureIterator( const QgsFeatureIterator& fi );
    //! destructor deletes the iterator if it has no more references
    ~QgsFeatureIterator();

    // QgsFeatureIterator& operator=(const QgsFeatureIterator& other);

    bool nextFeature( QgsFeature& f );
    bool rewind();
    bool close();

    //! find out whether the iterator is still valid or closed already
    bool isClosed() const;

    /** Returns the status of expression compilation for filter expression requests.
     * @note added in QGIS 2.16
     */
    QgsAbstractFeatureIterator::CompileStatus compileStatus() const;

};
