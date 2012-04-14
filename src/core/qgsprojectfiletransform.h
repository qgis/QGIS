/***************************************************************************
                          qgsprojectfiletransform.h  -  description
                             -------------------
    begin                : Sun 15 dec 2007
    copyright            : (C) 2007 by Magnus Homann
    email                : magnus at homann.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** \ingroup core
 * Class to convert from older project file versions to newer.
 * This class provides possibility to store a project file as a QDomDocument,
 * and provides the ability to specify version of the project file, and
 * perform upgrades to a more recent version
 */


#ifndef QGSPROJECTFILETRANSFORM_H
#define QGSPROJECTFILETRANSFORM_H

#include <QString>
#include <QDomDocument>
#include <vector>
#include "qgsprojectversion.h"

class QgsRasterLayer;

class QgsProjectFileTransform
{
  public:
    //Default constructor
    //QgsProjectfiletransform() {}
    ~QgsProjectFileTransform() {}

    /*! Create an instance from a Dom and a supplied version
     * @param domDocument The Dom document to use as content
     * @param version Version number
     */
    QgsProjectFileTransform( QDomDocument & domDocument,
                             QgsProjectVersion version )
    {
      mDom = domDocument;
      mCurrentVersion = version;
    }


    bool updateRevision( QgsProjectVersion version );

    /*! Prints the contents via QgsDebugMsg()
     */
    void dump();


  private:

    typedef struct
    {
      QgsProjectVersion from;
      QgsProjectVersion to;
      void ( QgsProjectFileTransform::* transformFunc )();
    } transform;

    static transform transformers[];

    QDomDocument mDom;
    QgsProjectVersion mCurrentVersion;

    // Transformer functions below. Declare functions here,
    // define them in qgsprojectfiletransform.cpp and add them
    // to the transformArray with proper version number
    void transformNull() {}; // Do absolutely nothing
    void transform081to090();
    void transform091to0100();
    void transform0100to0110();
    void transform0110to1000();
    void transform1100to1200();
    void transform1400to1500();
    void transform1800to1900();

    //helper functions
    int rasterBandNumber( const QDomElement& rasterPropertiesElem, const QString bandName, QgsRasterLayer* rlayer );
    void transformContrastEnhancement( QDomDocument& doc, const QDomElement& rasterproperties, QDomElement& rendererElem );
    void transformRasterTransparency( QDomDocument& doc, const QDomElement& orig, QDomElement& rendererElem );
};


#endif //QGSPROJECTFILETRANSFORM_H

