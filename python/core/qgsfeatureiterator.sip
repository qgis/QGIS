class QgsAbstractFeatureIterator
{
%TypeHeaderCode
#include <qgsfeatureiterator.h>
%End
  public:
    //! base class constructor - stores the iteration parameters
    QgsAbstractFeatureIterator( const QgsFeatureRequest& request );

    //! destructor makes sure that the iterator is closed properly
    virtual ~QgsAbstractFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& f ) = 0;
    //! reset the iterator to the starting position
    virtual bool rewind() = 0;
    //! end of iterating: free the resources / lock
    virtual bool close() = 0;

  protected:
    void ref(); // add reference
    void deref(); // remove reference, delete if refs == 0
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
    sipRes = sipConvertFromInstance(f, sipClass_QgsFeature, Py_None);
  else
  {
    delete f;
    PyErr_SetString(PyExc_StopIteration,"");
  }
%End


  //! construct invalid iterator
  QgsFeatureIterator();
  //! construct a valid iterator
  //QgsFeatureIterator(QgsAbstractFeatureIterator* iter);
  //! copy constructor copies the iterator, increases ref.count
  QgsFeatureIterator(const QgsFeatureIterator& fi);
  //! destructor deletes the iterator if it has no more references
  ~QgsFeatureIterator();

  //QgsFeatureIterator& operator=(const QgsFeatureIterator& other);

  bool nextFeature(QgsFeature& f);
  bool rewind();
  bool close();

  //! find out whether the iterator is still valid or closed already
  bool isClosed();
};
