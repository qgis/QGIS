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
    };
    typedef QFlags<QgsSipifyHeader::MyEnum> Flags;

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
%End

    void multilineMethod( const QgsPoint &startPoint,
                          QgsFeatureId featureId,
                          QgsVectorLayer *vl,
                          QgsSnapper::SnappingType snap_to ) const;
%Docstring
A multiline method signature
%End


    virtual QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) /Factory/;
%Docstring
Factory annotation
%End

    SomeObject *createAnother() /Factory/;

    void LongDocStringMethod();
%Docstring
 My long doc string
 is not very interesting!
%End

    void LongDocStringMethodWithBrief();
%Docstring
 some brief
 My long doc string
 is not very interesting!
 Here's some comment mentioning another class QgsAutoAwesomemater.makeLessAwesome.
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

    inlineKeyWordShouldNotAppear();

    QString labelForRange( double lower, double upper ) const /PyName=labelForLowerUpper/;

    void setComposition( QgsComposition *c /KeepReference/ );

    void removeProxyFactory( QNetworkProxyFactory *factory /TransferBack/ );

    bool removeFunctionBody( const QList<int, QString> &list, QgsVectorLayer *vl );
    QgsRaster::RasterBuildPyramids buildPyramidsFlag() const;
%Docstring
Removing function body with namespaced return value
%End


    virtual int overriddenProperty();
  protected:
    bool thisShouldBeListed();


  private:
    void privateMethodSIPRUNareShown();
  public:
    void FallBackToPublic();

};

class ClassWithPrivateInheritanceOnly
{
%Docstring
 Documentation goes here
%End

%TypeHeaderCode
#include "sipifyheader.h"
%End
    explicit ClassWithPrivateInheritanceOnly();
%Docstring
A constructor with definition in header on several lines
%End

}

QFlags<QgsSipifyHeader::MyEnum> operator|(QgsSipifyHeader::MyEnum f1, QFlags<QgsSipifyHeader::MyEnum> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * tests/scripts/sipifyheader.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
