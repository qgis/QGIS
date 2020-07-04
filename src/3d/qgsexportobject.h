/***************************************************************************
  qgsexportobject.h
  --------------------------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPORTOBJECT_H
#define QGSEXPORTOBJECT_H

#include <QObject>
#include <QTextStream>
#include <QVector>
#include <QVector3D>

/**
 * @brief The QgsExportObject class
 * Manages the data of each object of the scene (positions, normals, texture coordinates ...) since each object
 * \ingroup 3d
 * \since QGIS 3.16
 */
class QgsExportObject : public QObject
{
    Q_OBJECT
  public:

    /**
     * @brief QgsExportObject
     * Constructs an export object that will be filled with coordinates later
     * @param name
     * The name of the object (the user will be able to select each object individually using its name in blender)
     * @param parentName
     * The name of the parent (Will be useful to define scene hierarchie)
     * @param parent
     * The parent QObject (we use this to delete the QgsExportObject instance once the exporter instance is deallocated)
     */
    QgsExportObject( const QString &name, const QString &parentName = QString(), QObject *parent = nullptr );

    //! Returns whether object edges will look smooth
    bool smoothEdges() { return mSmoothEdges; }
    //! Sets whether triangles edges will look smooth
    void setSmoothEdges( bool smoothEdges ) { mSmoothEdges = smoothEdges; }

    //! Sets positions coordinates from just one positions buffer (generates faces automatically) and does the translation and scaling
    void setupPositionCoordinates( const QVector<float> &positionsBuffer, float scale = 1.0f, const QVector3D translation = QVector3D( 0, 0, 0 ) );
    //! Sets positions coordinates from just one positions buffer and indexes buffer and does the translation and scaling
    void setupPositionCoordinates( const QVector<float> &positionsBuffer, const QVector<unsigned int> &facesIndexes, float scale = 1.0f, const QVector3D translation = QVector3D( 0, 0, 0 ) );
    //! Updates the box bounds explained with the current object bounds
    //! This expands the bounding box if the current object outside the bounds of the already established bounds
    void objectBounds( float &minX, float &minY, float &minZ, float &maxX, float &maxY, float &maxZ );

    //! Saves the current object to the output stream while scaling the object and centering it to be visible in exported scene
    void saveTo( QTextStream &out, int scale, const QVector3D &center );
  private:
    QString mName;
    QString mParentName;
    QVector<float> mVertxPosition;
    QVector<int> mIndexes;

    bool mSmoothEdges;
};

#endif // QGSEXPORTOBJECT_H
