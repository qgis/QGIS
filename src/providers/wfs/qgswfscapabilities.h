/***************************************************************************
    qgswfscapabilities.h
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSCAPABILITIES_H
#define QGSWFSCAPABILITIES_H

#include <QDomElement>
#include <QSet>
#include "qgsrectangle.h"

//! Encapsultes WFS Capabilities
class QgsWfsCapabilities
{
  public:
    //! description of a vector layer
    struct FeatureType
    {
        //! Default constructor
        FeatureType() = default;

        QString name;
        QString nameSpace; // for some Deegree servers that requires a NAMESPACES parameter for GetFeature
        QString title;
        QString abstract;
        QList<QString> crslist; // first is default
        QgsRectangle bbox;
        bool bboxSRSIsWGS84 = false; // if false, the bbox is expressed in crslist[0] CRS
        bool insertCap = false;
        bool updateCap = false;
        bool deleteCap = false;
    };

    //! argument of a function
    struct Argument
    {
        //! name
        QString name;
        //! type, or empty if unknown
        QString type;

        //! constructor
        Argument( const QString &nameIn = QString(), const QString &typeIn = QString() )
          : name( nameIn ), type( typeIn ) {}
    };

    //! description of server functions
    struct Function
    {
        //! name
        QString name;
        //! Returns type, or empty if unknown
        QString returnType;
        //! minimum number of argument (or -1 if unknown)
        int minArgs = -1;
        //! maximum number of argument (or -1 if unknown)
        int maxArgs = -1;
        //! list of arguments. May be empty despite minArgs > 0
        QList<Argument> argumentList;

        //! constructor with name and fixed number of arguments
        Function( const QString &nameIn, int args )
          : name( nameIn ), minArgs( args ), maxArgs( args ) {}
        //! constructor with name and min,max number of arguments
        Function( const QString &nameIn, int minArgs, int maxArgsIn )
          : name( nameIn ), minArgs( minArgs ), maxArgs( maxArgsIn ) {}
        //! default constructor
        Function() = default;
    };

    QgsWfsCapabilities();

    QString version;
    bool supportsHits;
    bool supportsPaging;
    bool supportsJoins;
    long long maxFeatures;
    QList<FeatureType> featureTypes;
    QList<Function> spatialPredicatesList;
    QList<Function> functionList;
    bool useEPSGColumnFormat; // whether to use EPSG:XXXX srsname
    QList<QString> outputFormats;
    QgsStringMap operationGetEndpoints;
    QgsStringMap operationPostEndpoints;

    QSet<QString> setAllTypenames;
    QMap<QString, QString> mapUnprefixedTypenameToPrefixedTypename;
    QSet<QString> setAmbiguousUnprefixedTypename;

    void clear();
    QString addPrefixIfNeeded( const QString &name ) const;
    QString getNamespaceForTypename( const QString &name ) const;
    QString getNamespaceParameterValue( const QString &WFSVersion, const QString &typeName ) const;

    //! Returns whether the server supports IsPoint, IsCurve and IsSurface functions
    bool supportsGeometryTypeFilters() const;
};

#endif // QGSWFSCAPABILITIES_H
