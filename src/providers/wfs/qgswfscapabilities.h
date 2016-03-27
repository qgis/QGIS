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
      QString name;
      QString title;
      QString abstract;
      QList<QString> crslist; // first is default
      QgsRectangle bboxLongLat;
      bool insertCap;
      bool updateCap;
      bool deleteCap;
    };

    //! parsed get capabilities document
    struct Capabilities
    {
      Capabilities();
      void clear();

      QString version;
      bool supportsHits;
      bool supportsPaging;
      int maxFeatures;
      QList<FeatureType> featureTypes;
    };

    //! return parsed capabilities - requestCapabilities() must be called before
    const Capabilities& capabilities() const { return mCaps; }

  signals:
    //! emitted when the capabilities have been fully parsed, or an error occured */
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

    static QString NormalizeSRSName( QString crsName );
};

#endif // QGSWFSCAPABILITIES_H
