/***************************************************************************
     qgsgmlschema.h
     --------------------------------------
    Date                 : Sun Sep 16 12:19:55 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGMLSCHEMA_H
#define QGSGMLSCHEMA_H

#include "qgis_core.h"
#include <expat.h>
#include "qgis_sip.h"
#include "qgserror.h"
#include "qgsfields.h"
#include <list>
#include <set>
#include <stack>
#include <QPair>
#include <QByteArray>
#include <QDomElement>
#include <QStringList>
#include <QStack>

class QgsRectangle;
class QgsCoordinateReferenceSystem;
class QgsFeature;

/**
 * \ingroup core
 * \brief Description of feature class in GML
*/
class CORE_EXPORT QgsGmlFeatureClass
{
  public:

    QgsGmlFeatureClass() = default;
    QgsGmlFeatureClass( const QString &name, const QString &path );

    QList<QgsField> &fields() { return  mFields; }

    int fieldIndex( const QString &name );

    QString path() const { return mPath; }

    QStringList &geometryAttributes() { return mGeometryAttributes; }

  private:

    /**
     * Feature class name:
     *
     * - element name without NS or known prefix/suffix (_feature)
     * - typeName attribute name
    */
    QString mName;

    //QString mElementName;

    //! Dot separated path to element including the name
    QString mPath;

    /* Fields */
    // Do not use QMap to keep original fields order. If it gets to performance,
    // add a field index map
    QList<QgsField> mFields;

    /* Geometry attribute */
    QStringList mGeometryAttributes;
};

/**
 * \ingroup core
 * \class QgsGmlSchema
 */
class CORE_EXPORT QgsGmlSchema : public QObject
{
    Q_OBJECT
  public:
    QgsGmlSchema();

    //! Gets fields info from XSD
    bool parseXSD( const QByteArray &xml );

    /**
     * Guess GML schema from data if XSD does not exist.
      * Currently only recognizes UMN Mapserver GetFeatureInfo GML response.
      * Supports only UTF-8, UTF-16, ISO-8859-1, US-ASCII XML encodings.
      * \param data GML data
      * \returns TRUE in case of success
    */
    bool guessSchema( const QByteArray &data );

    //! Gets list of dot separated paths to feature classes parsed from GML or XSD
    QStringList typeNames() const;

    //! Gets fields for type/class name parsed from GML or XSD
    QList<QgsField> fields( const QString &typeName );

    //! Gets list of geometry attributes for type/class name
    QStringList geometryAttributes( const QString &typeName );

    //! Gets error if parseXSD() or guessSchema() failed
    QgsError error() const { return mError; }

  private:

    enum ParseMode
    {
      None,
      BoundingBox,
      FeatureMembers, // gml:featureMembers
      FeatureMember, // gml:featureMember
      Feature,  // feature element containing attrs and geo (inside gml:featureMember)
      Attribute,
      Geometry
    };

    //! XML handler methods
    void startElement( const XML_Char *el, const XML_Char **attr );
    void endElement( const XML_Char *el );
    void characters( const XML_Char *chars, int len );
    static void start( void *data, const XML_Char *el, const XML_Char **attr )
    {
      static_cast<QgsGmlSchema *>( data )->startElement( el, attr );
    }
    static void end( void *data, const XML_Char *el )
    {
      static_cast<QgsGmlSchema *>( data )->endElement( el );
    }
    static void chars( void *data, const XML_Char *chars, int len )
    {
      static_cast<QgsGmlSchema *>( data )->characters( chars, len );
    }
    // Add attribute or reset its type according to value of current feature
    void addAttribute( const QString &name, const QString &value );

    //helper routines

    /**
     * Reads attribute as string
     * \returns attribute value or an empty string if no such attribute
    */
    QString readAttribute( const QString &attributeName, const XML_Char **attr ) const;

    //! Returns pointer to main window or 0 if it does not exist
    QWidget *findMainWindow() const;

    //! Gets dom elements by path
    QList<QDomElement> domElements( const QDomElement &element, const QString &path );

    //! Gets dom element by path
    QDomElement domElement( const QDomElement &element, const QString &path );

    //! Filter list of elements by attribute value
    QList<QDomElement> domElements( QList<QDomElement> &elements, const QString &attr, const QString &attrVal );

    //! Gets dom element by path and attribute value
    QDomElement domElement( const QDomElement &element, const QString &path, const QString &attr, const QString &attrVal );

    //! Strip namespace from element name
    QString stripNS( const QString &name );

    /**
     * Find GML base type for complex type of given name
     * \param element input element
     * \param name complex type name
     * \returns name of GML base type without NS, e.g. AbstractFeatureType or empty string if not passed on GML type
     */
    QString xsdComplexTypeGmlBaseType( const QDomElement &element, const QString &name );

    //! Gets feature class information from complex type recursively
    bool xsdFeatureClass( const QDomElement &element, const QString &typeName, QgsGmlFeatureClass &featureClass );


    //! Gets safely (if empty) top from mode stack
    ParseMode modeStackTop() { return mParseModeStack.isEmpty() ? None : mParseModeStack.top(); }

    //! Safely (if empty) pop from mode stack
    ParseMode modeStackPop() { return mParseModeStack.isEmpty() ? None : mParseModeStack.pop(); }

    //! Keep track about the most important nested elements
    //std::stack<ParseMode> mParseModeStack;
    QStack<ParseMode> mParseModeStack;
    //! This contains the character data if an important element has been encountered
    QString mStringCash;
    QgsFeature *mCurrentFeature = nullptr;
    QString mCurrentFeatureId;
    int mFeatureCount = 0;
    QString mAttributeName;
    //! Coordinate separator for coordinate strings. Usually ","
    QString mCoordinateSeparator;
    //! Tuple separator for coordinate strings. Usually " "
    QString mTupleSeparator;

    /* Schema information guessed/parsed from GML in getSchema() */

    //! Depth level, root element is 0
    int mLevel = 0;

    //! Skip all levels under this
    int mSkipLevel;

    //! Path to current level
    QStringList mParsePathStack;

    QString mCurrentFeatureName;

    // List of know geometries (Point, Multipoint,...)
    QStringList mGeometryTypes;

    /* Feature classes map with element paths as keys */
    QMap<QString, QgsGmlFeatureClass> mFeatureClassMap;

    /* Error set if something failed */
    QgsError mError;
};

#endif
