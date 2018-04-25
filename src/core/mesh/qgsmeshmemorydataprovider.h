/***************************************************************************
                         qgsmeshmemorydataprovider.h
                         ---------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHMEMORYDATAPROVIDER_H
#define QGSMESHMEMORYDATAPROVIDER_H

#define SIP_NO_FILE

///@cond PRIVATE

#include <QString>

#include "qgis_core.h"
#include "qgis.h"
#include "qgsmeshdataprovider.h"
#include "qgsrectangle.h"

/**
 * \ingroup core
 * Provides data stored in-memory for QgsMeshLayer. Useful for plugins or tests.
 * \since QGIS 3.2
 */
class QgsMeshMemoryDataProvider: public QgsMeshDataProvider
{
    Q_OBJECT

  public:

    /**
     * Construct a mesh in-memory data provider from data string
     *
     * Data string constains simple definition of vertices and faces
     * Each entry is separated by "\n" sign and section deliminer "---"
     * vertex is x and y coordinate separated by comma
     * face is list of vertex indexes, numbered from 0
     * For example:
     *
     *  \code
     *    QString uri(
     *      "1.0, 2.0 \n" \
     *      "2.0, 2.0 \n" \
     *      "3.0, 2.0 \n" \
     *      "2.0, 3.0 \n" \
     *      "1.0, 3.0 \n" \
     *      "---"
     *      "0, 1, 3, 4 \n" \
     *      "1, 2, 3 \n"
     *    );
     * \endcode
     */
    QgsMeshMemoryDataProvider( const QString &uri = QString() );
    ~QgsMeshMemoryDataProvider();

    bool isValid() const override;
    QString name() const override;
    QString description() const override;
    QgsCoordinateReferenceSystem crs() const override;

    int vertexCount() const override;
    int faceCount() const override;
    QgsMeshVertex vertex( int index ) const override;
    QgsMeshFace face( int index ) const override;

    //! Returns the memory provider key
    static QString providerKey();
    //! Returns the memory provider description
    static QString providerDescription();
    //! Provider factory
    static QgsMeshMemoryDataProvider *createProvider( const QString &uri );
  private:
    bool splitSections( const QString &uri );

    bool addVertices( const QString &def );
    bool addFaces( const QString &def );

    QVector<QgsMeshVertex> mVertices;
    QVector<QgsMeshFace> mFaces;

    bool mIsValid = false;
};

///@endcond

#endif // QGSMESHMEMORYDATAPROVIDER_H
