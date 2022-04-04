/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * tests/code_layout/sipify/sipifyheader.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
class QgsSettingsEntryEnumFlag_settingsDigitizingDefaultSnapType
{
%TypeHeaderCode
#include "sipifyheader.h"
#include "qgssettingsentry.h"
typedef QgsSettingsEntryEnumFlag<Qgis::SnappingType> QgsSettingsEntryEnumFlag_settingsDigitizingDefaultSnapType;
%End
  public:
    QgsSettingsEntryEnumFlag_settingsDigitizingDefaultSnapType( const QString &key, QgsSettings::Section section, const Qgis::SnappingType &defaultValue, const QString &description = QString() );
    QString key( const QString &dynamicKeyPart = QString() ) const;
    Qgis::SnappingType value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, const Qgis::SnappingType &defaultValueOverride = Qgis::SnappingType() ) const;
};



%ModuleHeaderCode
#include <qgsnetworkspeedstrategy.h>
#include <qgsnetworkdistancestrategy.h>
%End








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


typedef QtClass<QVariant> QtClassQVariantBase;

class QgsSipifyHeader : QtClassQVariantBase
{
%Docstring(signature="appended")
Documentation goes here

Here's some comment mentioning another class :py:class:`QgsAutoAwesomemater`.makeAwesome.

.. note::

   some other note

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "sipifyheader.h"
#include "qtclass.h"
typedef QtClass<QVariant> QtClassQVariantBase;
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
      PythonName /PyName=DifferentName/,
      MonkeyName,
    };
    typedef QFlags<QgsSipifyHeader::MyEnum> Flags;


    enum OneLiner { Success, NoSuccess };

  private:
    void makePrivate( int a );

  public:
    void publicMethodBetween1();

  private:
    void makePrivateMultiline( int a,
                               int b );
  public:
    void publicMethodBetween2();

  private:
    bool makePrivateMultilineImpl( int a,
                                   int b );
  public:
    const QgsSettingsEntryEnumFlag_settingsDigitizingDefaultSnapType settingsDigitizingDefaultSnapType;

    struct Data
    {
      Data( QgsMapLayer *layer, Qstring name );

      QString mName;
      int mCount;
      QgsMapLayer *mLayer;
      QList<QAction *> contextMenuActions;
    };

    static const int MONTHS;

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

    Constructor();

    virtual ~Destructor();

    Constructor( const QString &name,
                 bool optional = false,
                 const QVariant &defaultValue = QVariant() );

    QgsSipifyHeader();
%Docstring
Default constructor
%End


    bool operator==( const QgsSipifyHeader other );

    void multilineMethod( const QgsPointXY &startPoint,
                          QgsFeatureId featureId,
                          QgsVectorLayer *vl,
                          Qgis::SnappingType snap_to ) const;
%Docstring
A multiline method signature
%End


    void nonAnnotatedMethodFollowingSkip();


    virtual QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) /Factory/;
%Docstring
Factory annotation
%End

    SomeObject *createAnother() /Factory/;

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
Here's some comment mentioning another class :py:class:`QgsAutoAwesomemater`.makeLessAwesome.
%End

    MyPointer *pointerReturnValue();
%Docstring
I return a pointer. If something bad happens, I return ``None``.

:return: pointer to something cool
%End

    bool isOKwithErrMesg( QString &ErrMsg /Out/ );

    void InOutParam( bool &ok = true /In,Out/ );

    void setDiagramRenderer( QgsDiagramRenderer *r /Transfer/ );

    void differentDefaultValue( bool defaultValue = false , QWidget *parent = 0, QString msg = "hello"  );

    void differentType( QList<qint64>  & list );

    void complexDefaultValueAndType( QList<QPair<qint64 , QMap<int, QString>>> list = QList<QPair<qint64, QMap<int, QString>>> );
%Docstring
complex default value and type (i.e. containing commas) should be given as a string with single quotes
%End

    int inlineKeyWordShouldNotAppear();

    QString labelForRange( double lower, double upper ) const /PyName=labelForLowerUpper/;

    void setComposition( QgsComposition *c /KeepReference/ );

    void removeProxyFactory( QNetworkProxyFactory *factory /TransferBack/ );

    bool removeFunctionBody( const QList<int, QString> &list, QgsVectorLayer *vl, Some::Thing _part = -1 /*default =-1*/ );


    void multilineBodyAndDefinition( const QList<int,
                                     QString> &list,
                                     QgsVectorLayer *vl,
                                     Some::Thing _part = -1 /*default =-1*/ );

    QgsRaster::RasterBuildPyramids buildPyramidsFlag() const;
%Docstring
Removing function body with namespaced return value
%End

    virtual const QgsLayerMetadata &metadata() const;
%Docstring
Removing function body with virtual const reference
%End

    bool myMultiLineBody();
%Docstring
Mulitline body
%End


    virtual int overriddenProperty();

    virtual int overrideWithoutVirtual();


     virtual void overrideWithoutVirtualMultLine( const QList<int, QString> &list1,
                                         const QList<int, QString> &list2 );

    QString returnTypeString() const;

    double returnTypeDouble() const;

    QList< QgsAnnotation * > returnTypeList();

    QVector< QgsAnnotation > returnTypeVector();

    QStringList returnTypeStringList();

    QSet<QgsActionScope> returnTypeSet();

    This<Member> shouldBeIncluded;

    static QString invokableMethod();

    bool initializedMember;

    struct PublicStruct
    {
      explicit PublicStruct( int _part = -1, int _ring = -1, int _vertex = -1, VertexType _type = SegmentVertex );

      bool isValid( const QgsAbstractGeometry *geom ) const;

      int part;
      int ring;
      int vertex;
      VertexType type;
    }

    void combinedAnnotations() /Factory,PyName=otherName/;
    void multiAnnotationArg( SomeClass **object /Out,TransferBack/, int &another /Out/ );

    void simple();
%Docstring
remove argument
%End
    void method();
    void test();
    void avoidIntersections( const QList<QgsVectorLayer *> &avoidIntersectionsLayers );

    void position();
    void position(  bool keep );
    void position( bool keep, bool keep );
    void position( bool keep );

    static SIP_PYLIST  changeReturnType( QVector<int> *resultTree = 0, QVector<double> &resultCost = 0 );

    Whatever &operator[]( int i ) /Factory/;
%MethodCode
    ....
%End

    void ZshouldBeShown();

    void methodCodeWithMultiLineDef();
%MethodCode
    if ( QgsWkbTypes::flatType( a0 ) != QgsWkbTypes::Point )
    {
      multiLineDef( PyExc_ValueError,
                    QString( "%1 is not nice" ).arg( QgsWkbTypes::displayString( a0 ) ).toUtf8().constData() );
    }
    else
    {
      sipCpp = new sipQgsPoint( a0, a1, a2, a3, a4 );
    }
%End

  protected:
    bool thisShouldBeListed();


  private:
    void privateMethodSIPRUNareShown();
  public:
    void FallBackToPublic();

  private:
    void ShowThisPrivateOne();
};

class ClassWithPrivateInheritanceOnly /Abstract/
{
%Docstring(signature="appended")
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
%Docstring(signature="appended")
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



typedef QgsTemplate<Something> QgsTemplateSomethingBase;

class TemplateInheritance1 : QgsTemplateSomethingBase
{

%TypeHeaderCode
#include "sipifyheader.h"
#include "qgstemplate.h"
typedef QgsTemplate<Something> QgsTemplateSomethingBase;
%End
}
class TemplateInheritance2
{

%TypeHeaderCode
#include "sipifyheader.h"
%End
}

typedef QgsTemplate<Something> QgsTemplateSomethingBase;

class TemplateInheritance3 : QgsTemplateSomethingBase, SomethingElse
{

%TypeHeaderCode
#include "sipifyheader.h"
#include "qgstemplate.h"
typedef QgsTemplate<Something> QgsTemplateSomethingBase;
%End
}
class TemplateInheritance4 : SomethingElse1, SomethingElse2
{

%TypeHeaderCode
#include "sipifyheader.h"
%End
}
class TemplateInheritance5 : SomethingElse
{

%TypeHeaderCode
#include "sipifyheader.h"
%End
}


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * tests/code_layout/sipify/sipifyheader.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
