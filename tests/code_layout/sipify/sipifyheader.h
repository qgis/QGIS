/***************************************************************************
                      sipifyheader.h - Demo for sipify.pl
                     --------------------------------------
Date                 : 28.03.2017
Copyright            : (C) 2017 Denis Rouzaud
email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SIPIFYHEADER_H
#define SIPIFYHEADER_H

#include "qgis_core.h"
#include <QtClass>

#ifdef SIP_RUN
% ModuleHeaderCode
#include <qgsnetworkspeedstrategy.h>
#include <qgsnetworkdistancestrategy.h>
% End
#endif

#include "sipifyheader.h"

// one shall include qgis.h to use SIP annotations
#include "qgis.h"

class QgsForwardDeclaration;




/***************************************************************************
 * This is some random block comment that will not be displayed.
 * Block comments shall will placed upon class, method, enum without
 * any blank lines in between.
 ****************************************************************************/

// typedef have no Docstring, so commenting here will not be used
#ifdef SIP_RUN
typedef qint64 QgsFeatureId;
#else
typedef WhatEver ShouldNotBeDisplayed;
#endif

typedef QSet<QgsFeatureId SIP_PYALTERNATIVETYPE( qint64 )> QgsFeatureIds;
typedef QMap<QgsFeatureId SIP_PYALTERNATIVETYPE( qint64 ), QgsAttributeMap SIP_PYALTERNATIVETYPE( 'QMap<int, QVariant>' )> QgsChangedAttributesMap;
typedef QMap<QgsFeatureId, QgsAttributeMap> SIP_PYALTERNATIVETYPE( 'QMap<qint64, QMap<int, QVariant> >' ) QgsChangedAttributesMap;
typedef QMap<QgsFeatureId, QPair<QMap<Something, Complex> >>  SIP_PYALTERNATIVETYPE( 'QMap<qint64, QMap<int, QVariant>>' ) QgsChangedAttributesMap;

/**
 * \ingroup core
 * A super QGIS class
 */
#ifndef SIP_RUN // following will be hidden
class CORE_EXPORT QgsSuperClass : public QtClass<QVariant>
{
  public:
    //! A constructor with definition in header
    QgsSuperClass()
      : QtClass<QVariant>()
    {}
};
#else // following will be displayed in generated file
typedef QVector<QVariant> QgsSuperClass;

% MappedType QgsSuperClass
{
  // The annotations are modified by astyle (these will be fixed by sipify.pl)
  % TypeHeaderCode
#include <qgssipifyheader.h>
  % End

  % ConvertFromTypeCode
  // Create the list.
  PyObject *l;
  return l;
  % End
}
#endif

/**
 * \ingroup core
 * Documentation goes here
 *
 * Here's some comment mentioning another class QgsAutoAwesomemater::makeAwesome.
 * \note some other note
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsSipifyHeader : public QtClass<QVariant>, private Ui::QgsBaseClass
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->headerType() == QgsSipifyHeader::Special )
      sipType = sipType_QgsSpecialSipifyHeader;
    else
      sipType = sipType_QgsStandardSipifyHeader;
    SIP_END
#endif

  public:
    //! This is an enum
    enum MyEnum
    {
      Success = 0, //!< Edit operation was successful
      NoSuccess = 1, //!< Edit operation resulted in an empty geometry
      ImaginarySuccess = 1 << 3, //!< Edit operation resulted in an imaginary geometry
      RecursiveSuccess = 1 << 4, //!< Edit operation resulted in an n-dimensional wormhole
      SuccessCombination = Success | ImaginarySuccess, //!< Holy Graal
      PythonName SIP_PYNAME( DifferentName ), //!< Different python name
      MonkeyName SIP_MONKEYPATCH_COMPAT_NAME( MonkeyPatchName ), //!< Monkey patched compatibility name
    };
    Q_DECLARE_FLAGS( Flags, MyEnum )

    enum OneLiner { Success, NoSuccess };

    void makePrivate( int a ) SIP_MAKE_PRIVATE;

    void publicMethodBetween1();

    void makePrivateMultiline( int a,
                               int b ) SIP_MAKE_PRIVATE;

    void publicMethodBetween2();

    bool makePrivateMultilineImpl( int a,
                                   int b ) SIP_MAKE_PRIVATE
    {return false}

    static const inline QgsSettingsEntryEnumFlag<Qgis::SnappingType> settingsDigitizingDefaultSnapType = QgsSettingsEntryEnumFlag<Qgis::SnappingType>( QStringLiteral( "/qgis/digitizing/default_snap_type" ), Qgis::SnappingType::VertexFlag );

    /**
     * Docstring headers for structs are not supported by sip (as of 4.18) and
     * therefore this docstring must not to be copied to the sipfile.
     */
    struct Data
    {
      Data( QgsMapLayer *layer, Qstring name )
        : mLayer( layer )
        , mName( name )
      {}

      QString mName;
      int mCount = 100;
      QgsMapLayer *mLayer = nullptr;
      QList<QAction *> contextMenuActions = QList<QAction *>();
    };

    static const int MONTHS = 60 * 60 * 24 * 30; // something

    //! A constructor with definition in header
    explicit QgsSipifyHeader()
      : QtClass<QVariant>()
      , QgsBaseClass()
    {}

    /*
     * A classic constructor with arguments
     */
    QgsSipifyHeader( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! A constructor with no empty `()`
    QgsSipifyHeader( bool a = true )
      : mMember( nullptr )
    {}

    //! A constructor with some special character types
    QgsSipifyHeader( QList<Point> a, const Issues &b = Issues::weDontHaveIssues(), QgsClass *b = nullptr )
      : mMember( nullptr )
    {}

    Constructor()
      : mHasNamedNodes( false )
    {}

    virtual ~Destructor() { qDeleteAll( mList ); }

    Constructor( const QString &name,
                 bool optional = false,
                 const QVariant &defaultValue = QVariant() )
      : mName( name )
      , mOptional( optional )
      , mDefaultValue( defaultValue )
    {}

    //! Default constructor
    QgsSipifyHeader() = default;

    //! Assignment operator should be removed in sip
    QgsSipifyHeader &operator=( const QgsSipifyHeader other );

    //! Comparison operator should be kept
    bool operator==( const QgsSipifyHeader other );

    //! A multiline method signature
    void multilineMethod( const QgsPointXY &startPoint,
                          QgsFeatureId featureId,
                          QgsVectorLayer *vl,
                          Qgis::SnappingType snap_to ) const;

    // Adding SIP_SKIP at the end of a line will discard this line
    bool thisShouldBeSkipped() const SIP_SKIP;

    void nonAnnotatedMethodFollowingSkip();

    bool myMultiLineSkipped( const QList<int, QString> &list1,
                             const QList<int, QString> &list2,
                             const QList<int, QString> &list3 ) SIP_SKIP;

    //! Factory annotation
    virtual QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;

    SomeObject *createAnother() SIP_FACTORY { return something; }

    SomeObject *createAnother2() override SIP_FACTORY { return something; }

    /**
     * My long doc string
     * is not very interesting!
     */
    void LongDocStringMethod();

    /**
     * \brief some brief
     * My long doc \a string
     * is not very interesting!
     * Here's some comment mentioning another class QgsAutoAwesomemater::makeLessAwesome.
     */
    void LongDocStringMethodWithBrief();

    /**
     * I return a pointer. If something bad happens, I return NULLPTR.
    * \returns pointer to something cool
     */
    MyPointer *pointerReturnValue();

    bool isOKwithErrMesg( QString &ErrMsg SIP_OUT );

    void InOutParam( bool &ok = true SIP_INOUT );

    void setDiagramRenderer( QgsDiagramRenderer *r SIP_TRANSFER );

    void differentDefaultValue( bool defaultValue = true SIP_PYARGDEFAULT( false ), QWidget *parent = nullptr, QString msg = QString() SIP_PYARGDEFAULT( "hello" ) );

    void differentType( QList<QgsFeatureId> SIP_PYALTERNATIVETYPE( QList<qint64> ) & list );

    //! complex default value and type (i.e. containing commas) should be given as a string with single quotes
    void complexDefaultValueAndType( QList<QPair<QgsFeatureId SIP_PYALTERNATIVETYPE( qint64 ), QMap<int, QString>>> list = QList<QPair<QgsFeatureId, QMap<int, QString>>>() SIP_PYARGDEFAULT( 'QList<QPair<qint64, QMap<int, QString>>>()' ) );

    inline int inlineKeyWordShouldNotAppear();

    QString labelForRange( double lower, double upper ) const SIP_PYNAME( labelForLowerUpper );

    void setComposition( QgsComposition *c SIP_KEEPREFERENCE );

    void removeProxyFactory( QNetworkProxyFactory *factory SIP_TRANSFERBACK );

    bool removeFunctionBody( const QList<int, QString> &list, QgsVectorLayer *vl, Some::Thing _part = -1 /*default =-1*/ ) { doSomething; return true; }   // some comments

    static inline QgsMapLayer *skippedMethodWithBody() SIP_SKIP
    {
      OhNoYouShouldnotHaveReadThis();
      if ( ThisIsTrue() )
      {
        return false;
      }
    }

    void multilineBodyAndDefinition( const QList<int,
                                     QString> &list,
                                     QgsVectorLayer *vl,
                                     Some::Thing _part = -1 /*default =-1*/ )
    {
      doSomething;
      return true;
    }

    //! Removing function body with namespaced return value
    QgsRaster::RasterBuildPyramids buildPyramidsFlag() const { return mBuildPyramidsFlag; }

    //! Removing function body with virtual const reference
    virtual const QgsLayerMetadata &metadata() const { return mMetadata; }

    //! Mulitline body
    bool myMultiLineBody()
    {
      if ( isTrue() )
      {
        return false;
      }
      else
      {
        return true;
      }
    }

    bool deletedFunction() = delete; // some comments

    virtual int overriddenProperty() override { return 42; } // if in doubt, comment it out

    int overrideWithoutVirtual() override;

    void overrideWithoutVirtualMultLine( const QList<int, QString> &list1,
                                         const QList<int, QString> &list2 ) override;

    QString returnTypeString() const;

    double returnTypeDouble() const;

    QList< QgsAnnotation * > returnTypeList();

    QVector< QgsAnnotation > returnTypeVector();

    QStringList returnTypeStringList();

    QSet<QgsActionScope> returnTypeSet();

    This<Member> shouldBeIncluded;

    Q_INVOKABLE static QString invokableMethod();

    bool initializedMember{ false };

    struct CORE_EXPORT PublicStruct
    {
      explicit PublicStruct( int _part = -1, int _ring = -1, int _vertex = -1, VertexType _type = SegmentVertex )
        : part( _part )
        , ring( _ring )
        , vertex( _vertex )
        , type( _type )
      {}

      bool isValid( const QgsAbstractGeometry *geom ) const
      {
        return ( part >= 0 && part < geom->partCount() ) &&
               ( ring < geom->ringCount( part ) ) &&
               ( vertex < 0 || vertex < geom->vertexCount( part, ring ) );
      }

      int part;
      int ring;
      int vertex;
      VertexType type;
    }

    void combinedAnnotations() SIP_FACTORY SIP_PYNAME( otherName );
    void multiAnnotationArg( SomeClass **object SIP_OUT SIP_TRANSFERBACK, int &another SIP_OUT );

    //! remove argument
    void simple( bool test SIP_PYARGREMOVE );
    void method( bool myArg SIP_PYARGREMOVE = test );
    void test( QgsMapLayer *vl SIP_PYARGREMOVE = nullptr );
    void avoidIntersections( const QList<QgsVectorLayer *> &avoidIntersectionsLayers,
                             const QHash<QgsVectorLayer *, QSet<QgsFeatureId> > &ignoreFeatures SIP_PYARGREMOVE = ( QHash<QgsVectorLayer *, QSet<QgsFeatureId> >() ) );
    void position( bool single_remove SIP_PYARGREMOVE );
    void position( bool first_remove SIP_PYARGREMOVE, bool keep );
    void position( bool keep, bool middle_remove SIP_PYARGREMOVE, bool keep );
    void position( bool keep, bool last_remove SIP_PYARGREMOVE );

    static void SIP_PYALTERNATIVETYPE( SIP_PYLIST ) changeReturnType( QVector<int> *resultTree = 0, QVector<double> &resultCost = 0 );

    //! Some comment
    Whatever &operator[]( int i ) SIP_FACTORY;
#ifdef SIP_RUN
    % MethodCode
    ....
    % End
#endif

#if 0
#if Whatever
    void X();
#else
    void Y();
#endif
#else
    void ZshouldBeShown();
#endif

    void methodCodeWithMultiLineDef();
#ifdef SIP_RUN
    % MethodCode
    if ( QgsWkbTypes::flatType( a0 ) != QgsWkbTypes::Point )
    {
      multiLineDef( PyExc_ValueError,
                    QString( "%1 is not nice" ).arg( QgsWkbTypes::displayString( a0 ) ).toUtf8().constData() );
    }
    else
    {
      sipCpp = new sipQgsPoint( a0, a1, a2, a3, a4 );
    }
    % End
#endif

  protected:
    bool thisShouldBeListed();

    Whatever skipMember;
    Whatever::Something *alsoSkipMember = nullptr;
    mutable Whatever alsoSkipThis;
    Some<Other> memberToSkip;
    QList<QgsMapLayer *> list2skip;
    QMap<QString, Qt::CheckState> map2skip;
    FilterType mFilter = FilterNone;
    QgsFeatureId mFilterFid = -1;
    QgsFeatureIds mFilterFids;
    std::unique_ptr< QgsExpression > mFilterExpression;
    long mLimit = -1;
    InvalidGeometryCheck mInvalidGeometryFilter = GeometryNoCheck;
    std::function< void( const QgsFeature & ) > mInvalidGeometryCallback;
    static QHash<QString, Help> sFunctionHelpTexts;
    friend class QgsOgcUtils;
    template<typename> friend class QgsAbstractFeatureIteratorFromSource;
    const QgsAbstractGeometry *mGeometry = 0;
    mutable unsigned char *mP;

  private:
    void privateMethodAreNotShown();
#ifdef SIP_RUN
    void privateMethodSIPRUNareShown();
#endif
  public:
    void FallBackToPublic();

  private:
    void PrivateAgain();
    /* Single line block comments shouldn't break the parser */

    void ShowThisPrivateOne() SIP_FORCE;

    struct ProcessFeatureWrapper
    {
      QgsGeometrySnapper *instance = nullptr;
      double snapTolerance;
      SnapMode mode;
      explicit ProcessFeatureWrapper( QgsGeometrySnapper *_instance, double snapTolerance, SnapMode mode )
        : instance( _instance )
        , snapTolerance( snapTolerance )
        , mode( mode )
      {}
      void operator()( QgsFeature &feature ) { return instance->processFeature( feature, snapTolerance, mode ); }
    };

    enum PointFlag { SnappedToRefNode, SnappedToRefSegment, Unsnapped };
};

/**
 * \class ClassWithPrivateInheritanceOnly
 * \ingroup core
 * Documentation goes here
 */
class CORE_EXPORT ClassWithPrivateInheritanceOnly : private QgsBaseClass SIP_ABSTRACT
{
  public:
    //! A constructor with definition in header on several lines
    explicit ClassWithPrivateInheritanceOnly()
      : QtClass<QVariant>()
      , QgsBaseClass()
    {
      doWhatYouLike();
      haveFun();
    }
};

/**
 * \class AbstractClass
 * \ingroup core
 * Documentation goes here
 */

class CORE_EXPORT AbstractClass SIP_ABSTRACT
{
  public:
    //! A constructor
    explicit AbstractClass()
    {
    }

  private:

    /**
     * This method should be overridden by subclasses but not exposed to the public
     * or protected API.
     */
    virtual QString reason() = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsSipifyHeader::Flags )

class CORE_EXPORT TemplateInheritance1 : public QgsTemplate<Something>
{
}
class CORE_EXPORT TemplateInheritance2 : public QList<Something>, private SomethingElse
{
}
class CORE_EXPORT TemplateInheritance3 : public QgsTemplate<Something>, public SomethingElse
{
}
class CORE_EXPORT TemplateInheritance4 : public SomethingElse1, public QList<Something>, public SomethingElse2
{
}
class CORE_EXPORT TemplateInheritance5 : public SomethingElse, public QList<Something>
{
}


#endif
