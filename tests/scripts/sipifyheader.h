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

typedef QSet<QgsFeatureId SIP_PYTYPE( qint64 )> QgsFeatureIds;
typedef QMap<QgsFeatureId SIP_PYTYPE( qint64 ), QgsAttributeMap SIP_PYTYPE( 'QMap<int, QVariant>' )> QgsChangedAttributesMap;
typedef QMap<QgsFeatureId, QgsAttributeMap> SIP_PYTYPE( 'QMap<qint64, QMap<int, QVariant> >' ) QgsChangedAttributesMap;
typedef QMap<QgsFeatureId, QPair<QMap<Something, Complex> >>  SIP_PYTYPE( 'QMap<qint64, QMap<int, QVariant>>' ) QgsChangedAttributesMap;

/** \ingroup core
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

/** \ingroup core
 * Documentation goes here
 */
class CORE_EXPORT QgsSipifyHeader : public QtClass<QVariant>, private QgsBaseClass
{
  public:
    //! This is an enum
    enum MyEnum
    {
      Success = 0, //!< Edit operation was successful
      NoSuccess = 1, //!< Edit operation resulted in an empty geometry
    };

    //! A constructor with definition in header
    explicit QgsSipifyHeader()
      : QtClass<QVariant>()
      , QgsBaseClass()
    {}

    /*
     * A classic constructor with arguments
     */
    QgsSipifyHeader( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! A multiline method signature
    void multilineMethod( const QgsPoint &startPoint,
                          QgsFeatureId featureId,
                          QgsVectorLayer *vl,
                          QgsSnapper::SnappingType snap_to ) const;

    // Adding SIP_SKIP at the end of a line will discard this MethodCode
    bool thisShouldBeSkipped() const SIP_SKIP;

    //! Factory annotation
    virtual QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;

    /**
     * My long doc string
     * is not very interesting!
     */
    void LongDocStringMethod();

    bool isOKwithErrMesg( QString &ErrMsg SIP_OUT );

    void InOutParam( bool &ok = true SIP_INOUT );

    void setDiagramRenderer( QgsDiagramRenderer *r SIP_TRANSFER );

    void differentDefaultValue( bool defaultValue = true SIP_PYDEFAULTVALUE( false ), QWidget *parent = nullptr, QString msg = QString() SIP_PYDEFAULTVALUE( "hello" ) );

    void differentType( QList<QgsFeatureId> SIP_PYTYPE( QList<qint64> ) &list );

    //! complex default value and type (i.e. containing commas) should be given as a string with single quotes
    void complexDefaultValueAndType( QList<QPair<QgsFeatureId SIP_PYTYPE( qint64 ), QMap<int, QString>>> list = QList<QPair<QgsFeatureId, QMap<int, QString>>>() SIP_PYDEFAULTVALUE( 'QList<QPair<qint64, QMap<int, QString>>>()' ) );

    inline inlineKeyWordShouldNotAppear();

    QString labelForRange( double lower, double upper ) const SIP_PYNAME( labelForLowerUpper );

    void setComposition( QgsComposition *c SIP_KEEPREFERENCE );

    void removeProxyFactory( QNetworkProxyFactory *factory SIP_TRANSFERBACK );

    bool removeFunctionBody( const QList<int, QString> &list, QgsVectorLayer *vl ) { doSomething; return true; } // some comments

    bool deletedFunction() = delete; // some comments

  protected:
    bool thisShouldBeListed();

  private:
    void privateMethodAreNotShown();
#ifdef SIP_RUN
    void privateMethodSIPRUNareShown();
#endif

  public:
    void FallBackToPublic();

  private:
    void PrivateAgain();

};

class CORE_EXPORT ClassWithPrivateInheritanceOnly : private QgsBaseClass
{
    //! A constructor with definition in header on several lines
    explicit ClassWithPrivateInheritanceOnly()
      : QtClass<QVariant>()
      , QgsBaseClass()
    {
      doWhatYouLike();
      haveFun();
    }

}


#endif
