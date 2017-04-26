/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * tests/scripts/sipifyheader.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








typedef qint64 QgsFeatureId;

typedef QSet<qint64 > QgsFeatureIds;
typedef QMap<qint64 , QMap<int, QVariant>> QgsChangedAttributesMap;
typedef QMap<qint64, QMap<int, QVariant> > QgsChangedAttributesMap;
typedef QMap<qint64, QMap<int, QVariant>> QgsChangedAttributesMap;

typedef QVector<QVariant> QgsSuperClass;

%MappedType QgsSuperClass
{
  // The annotations are modified by astyle (these will be fixed by sipify.pl)
%TypeHeaderCode
#include <qgssipifyheader.h>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;
  return l;
%End
}

class QgsSipifyHeader : QtClass<QVariant>
{
%Docstring
 Documentation goes here

 Here's some comment mentioning another class QgsAutoAwesomemater.makeAwesome.
.. versionadded:: 3.0
.. note::

   some other note
%End

%TypeHeaderCode
#include "sipifyheader.h"
%End

%ConvertToSubClassCode
    if ( sipCpp->headerType() == QgsSipifyHeader::Special )
      sipType = sipType_QgsSpecialSipifyHeader;
    else
      sipType = sipType_QgsStandardSipifyHeader;
%End
  public:
    enum MyEnum
    {
      Success,
      NoSuccess,
      ImaginarySuccess,
      RecursiveSuccess,
      SuccessCombination,
    };
    typedef QFlags<QgsSipifyHeader::MyEnum> Flags;


    enum OneLiner { Success, NoSuccess };

    struct Data
    {
      Data( QgsMapLayer *layer, Qstring name );
      QString mName;
      int mCount;
      QgsMapLayer *mLayer;
    };

    explicit QgsSipifyHeader();
%Docstring
A constructor with definition in header
%End
    QgsSipifyHeader( QWidget *parent /TransferThis/ = 0 );
%Docstring
 A classic constructor with arguments
%End

    QgsSipifyHeader( bool a = true );
%Docstring
A constructor with no empty `()`
%End
    QgsSipifyHeader( QList<Point> a, const Issues &b = Issues::weDontHaveIssues(), QgsClass *b = 0 );
%Docstring
A constructor with some special character types
%End
    {}

    QgsSipifyHeader();
%Docstring
Default constructor
%End

//     QgsSipifyHeader &operator=( const QgsSipifyHeader other );
    bool operator==( const QgsSipifyHeader other );
%Docstring
Comparison operator should be kept
 :rtype: bool
%End

    void multilineMethod( const QgsPoint &startPoint,
                          QgsFeatureId featureId,
                          QgsVectorLayer *vl,
                          QgsSnappingResult::SnappingType snap_to ) const;
%Docstring
A multiline method signature
%End


    void nonAnnotatedMethodFollowingSkip();


    virtual QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) /Factory/;
%Docstring
Factory annotation
 :rtype: QgsMapLayerRenderer
%End

    SomeObject *createAnother() /Factory/;
%Docstring
 :rtype: SomeObject
%End

    virtual SomeObject *createAnother2() /Factory/;

    void LongDocStringMethod();
%Docstring
 My long doc string
 is not very interesting!
%End

    void LongDocStringMethodWithBrief();
%Docstring
 some brief
 My long doc ``string``
 is not very interesting!
 Here's some comment mentioning another class QgsAutoAwesomemater.makeLessAwesome.
%End

    MyPointer *pointerReturnValue();
%Docstring
 I return a pointer. If something bad happens, I return None.
 :return: pointer to something cool
 :rtype: MyPointer
%End

    bool isOKwithErrMesg( QString &ErrMsg /Out/ );
%Docstring
 :rtype: bool
%End

    void InOutParam( bool &ok = true /In,Out/ );

    void setDiagramRenderer( QgsDiagramRenderer *r /Transfer/ );

    void differentDefaultValue( bool defaultValue = false , QWidget *parent = 0, QString msg = "hello"  );

    void differentType( QList<qint64>  & list );

    void complexDefaultValueAndType( QList<QPair<qint64 , QMap<int, QString>>> list = QList<QPair<qint64, QMap<int, QString>>> );
%Docstring
complex default value and type (i.e. containing commas) should be given as a string with single quotes
%End

    int inlineKeyWordShouldNotAppear();
%Docstring
 :rtype: int
%End

    QString labelForRange( double lower, double upper ) const /PyName=labelForLowerUpper/;
%Docstring
 :rtype: str
%End

    void setComposition( QgsComposition *c /KeepReference/ );

    void removeProxyFactory( QNetworkProxyFactory *factory /TransferBack/ );

    bool removeFunctionBody( const QList<int, QString> &list, QgsVectorLayer *vl );
%Docstring
 :rtype: bool
%End

    QgsRaster::RasterBuildPyramids buildPyramidsFlag() const;
%Docstring
Removing function body with namespaced return value
 :rtype: QgsRaster.RasterBuildPyramids
%End

    virtual const QgsLayerMetadata &metadata() const;
%Docstring
Removing function body with virtual const reference
 :rtype: QgsLayerMetadata
%End

    bool myMultiLineBody();
%Docstring
Mulitline body
 :rtype: bool
%End


    virtual int overriddenProperty();
    virtual int overrideWithoutVirtual();


     virtual void overrideWithoutVirtualMultLine( const QList<int, QString> &list1,
                                         const QList<int, QString> &list2 );

    QString returnTypeString() const;
%Docstring
 :rtype: str
%End

    double returnTypeDouble() const;
%Docstring
 :rtype: float
%End

    QList< QgsAnnotation * > returnTypeList();
%Docstring
 :rtype: list of QgsAnnotation
%End

    QVector< QgsAnnotation > returnTypeVector();
%Docstring
 :rtype: list of QgsAnnotation
%End

    QStringList returnTypeStringList();
%Docstring
 :rtype: list of str
%End

    QSet<QgsActionScope> returnTypeSet();
%Docstring
 :rtype: set of QgsActionScope
%End

    This<Member> shouldBeIncluded;

    static QString invokableMethod();
%Docstring
 :rtype: str
%End

    bool initializedMember;

    struct PublicStruct
    {
      int var;
    }

    void ZshouldBeShown();


  protected:
    bool thisShouldBeListed();
%Docstring
 :rtype: bool
%End


  private:
    void privateMethodSIPRUNareShown();
  public:
    void FallBackToPublic();

  private:
    void ShowThisPrivateOne() ;
};

class ClassWithPrivateInheritanceOnly
{
%Docstring
 Documentation goes here
%End

%TypeHeaderCode
#include "sipifyheader.h"
%End
  public:
    explicit ClassWithPrivateInheritanceOnly();
%Docstring
A constructor with definition in header on several lines
%End
};


class AbstractClass /Abstract/
{
%Docstring
 Documentation goes here
%End

%TypeHeaderCode
#include "sipifyheader.h"
%End
  public:
    explicit AbstractClass();
%Docstring
A constructor
%End
};

QFlags<QgsSipifyHeader::MyEnum> operator|(QgsSipifyHeader::MyEnum f1, QFlags<QgsSipifyHeader::MyEnum> f2);



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * tests/scripts/sipifyheader.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
