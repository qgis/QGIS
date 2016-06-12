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

#include <QObject>
#include <QDomElement>

#include "qgsrectangle.h"
#include "qgswfsrequest.h"

/** Manages the GetCapabilities request */
class QgsWFSCapabilities : public QgsWFSRequest
{
    Q_OBJECT
  public:
    explicit QgsWFSCapabilities( const QString& theUri );
    virtual ~QgsWFSCapabilities();

    //! start network connection to get capabilities
    bool requestCapabilities( bool synchronous );

    //! description of a vector layer
    struct FeatureType
    {
      //! Default constructor
      FeatureType() : bboxSRSIsWGS84( false ), insertCap( false ), updateCap( false ), deleteCap( false ) {}

      QString name;
      QString title;
      QString abstract;
      QList<QString> crslist; // first is default
      QgsRectangle bbox;
      bool bboxSRSIsWGS84; // if false, the bbox is expressed in crslist[0] CRS
      bool insertCap;
      bool updateCap;
      bool deleteCap;
    };

    //! argument of a function
    struct Argument
    {
      //! name
      QString name;
      //! type, or empty if unknown
      QString type;

      //! constructor
      Argument( const QString& nameIn = QString(), const QString& typeIn = QString() ) : name( nameIn ), type( typeIn ) {}
    };

    //! description of server functions
    struct Function
    {
      //! name
      QString name;
      //! return type, or empty if unknown
      QString returnType;
      //! minimum number of argument (or -1 if unknown)
      int minArgs;
      //! maximum number of argument (or -1 if unknown)
      int maxArgs;
      //! list of arguments. May be empty despite minArgs > 0
      QList<Argument> argumentList;

      //! constructor with name and fixed number of arguments
      Function( const QString& nameIn, int args ) : name( nameIn ), minArgs( args ), maxArgs( args ) {}
      //! constructor with name and min,max number of arguments
      Function( const QString& nameIn, int minArgs, int maxArgsIn ) : name( nameIn ), minArgs( minArgs ), maxArgs( maxArgsIn ) {}
      //! default constructor
      Function() : minArgs( -1 ), maxArgs( -1 ) {}
    };

    //! parsed get capabilities document
    struct Capabilities
    {
      Capabilities();

      QString version;
      bool supportsHits;
      bool supportsPaging;
      bool supportsJoins;
      int maxFeatures;
      QList<FeatureType> featureTypes;
      QList<Function> spatialPredicatesList;
      QList<Function> functionList;
      bool useEPSGColumnFormat; // whether to use EPSG:XXXX srsname

      QSet< QString > setAllTypenames;
      QMap< QString, QString> mapUnprefixedTypenameToPrefixedTypename;
      QSet< QString > setAmbiguousUnprefixedTypename;

      void clear();
      QString addPrefixIfNeeded( const QString& name ) const;
    };

    //! return parsed capabilities - requestCapabilities() must be called before
    const Capabilities& capabilities() const { return mCaps; }

  signals:
    //! emitted when the capabilities have been fully parsed, or an error occurred */
    void gotCapabilities();

  private slots:
    void capabilitiesReplyFinished();

  protected:
    virtual QString errorMessageWithReason( const QString& reason ) override;
    virtual int defaultExpirationInSec() override;

  private:
    Capabilities mCaps;

    /** Takes <Operations> element and updates the capabilities*/
    void parseSupportedOperations( const QDomElement& operationsElem,
                                   bool& insertCap,
                                   bool& updateCap,
                                   bool& deleteCap );

    void parseFilterCapabilities( const QDomElement& filterCapabilitiesElem );

    static QString NormalizeSRSName( QString crsName );
};

#endif // QGSWFSCAPABILITIES_H
