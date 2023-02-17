/***************************************************************************
  Qgs3DExportObject.h
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

#ifndef Qgs3DExportObject_H
#define Qgs3DExportObject_H

#include <QObject>
#include <QTextStream>
#include <QVector>
#include <QVector3D>
#include <QImage>
#include <QMap>

#include "qgis_3d.h"

#define SIP_NO_FILE

class QgsAbstractMaterialSettings;

/**
 * \brief Manages the data of each object of the scene (positions, normals, texture coordinates ...) since each object
 *
 * \note Not available in Python bindings
 *
 * \ingroup 3d
 * \since QGIS 3.16
 */
class _3D_EXPORT Qgs3DExportObject
{
  public:
    //! The type of exported geometry
    enum ObjectType
    {
      TriangularFaces,
      LineStrip,
      Points
    };

    /**
     * \brief Qgs3DExportObject
     * Constructs an export object that will be filled with coordinates later
     * \param name The name of the parent (Will be useful to define scene hierarchy)
     */
    Qgs3DExportObject( const QString &name ) : mName( name ) { }

    //! Returns the object name
    QString name() const { return mName; }
    //! Sets the object name
    void setName( const QString &name ) { mName = name; }

    //! Returns the object type
    ObjectType type() const { return mType; }
    //! Sets the object type
    void setType( ObjectType type ) { mType = type; }

    //! Returns whether object edges will look smooth
    bool smoothEdges() const { return mSmoothEdges; }
    //! Sets whether triangles edges will look smooth
    void setSmoothEdges( bool smoothEdges ) { mSmoothEdges = smoothEdges; }

    //! Sets positions coordinates and does the translation and scaling
    void setupPositionCoordinates( const QVector<float> &positionsBuffer, float scale = 1.0f, const QVector3D &translation = QVector3D( 0, 0, 0 ) );
    //! Sets the faces in facesIndexes to the faces in the object
    void setupFaces( const QVector<uint> &facesIndexes );
    //! sets line vertex indexes
    void setupLine( const QVector<uint> &facesIndexes );

    //! Sets normal coordinates for each vertex
    void setupNormalCoordinates( const QVector<float> &normalsBuffer );
    //! Sets texture coordinates for each vertex
    void setupTextureCoordinates( const QVector<float> &texturesBuffer );
    //! Sets the material parameters (diffuse color, shininess...) from phong material
    void setupMaterial( QgsAbstractMaterialSettings *material );

    //! Sets the texture image used by the object
    void setTextureImage( const QImage &image ) { this->mTextureImage = image; };
    //! Returns the texture image used by the object
    QImage textureImage() const { return mTextureImage; }

    /**
     *
     * Updates the box bounds explained with the current object bounds
     * This expands the bounding box if the current object outside the bounds of the already established bounds
     */
    void objectBounds( float &minX, float &minY, float &minZ, float &maxX, float &maxY, float &maxZ );

    //! Sets a material parameter to be exported in the .mtl file
    void setMaterialParameter( const QString &parameter, const QString &value ) { mMaterialParameters[parameter] = value; }

    //! Saves the current object to the output stream while scaling the object and centering it to be visible in exported scene
    void saveTo( QTextStream &out, float scale, const QVector3D &center );
    //! saves the texture of the object and material information
    QString saveMaterial( QTextStream &mtlOut, const QString &folder );

  private:
    QString mName;
    ObjectType mType = ObjectType::TriangularFaces;
    QString mParentName;
    QVector<float> mVertexPosition;
    QVector<float> mNormals;
    QVector<float> mTexturesUV;
    QVector<unsigned int> mIndexes;
    QMap<QString, QString> mMaterialParameters;

    QImage mTextureImage;

    bool mSmoothEdges = false;
};

#endif // Qgs3DExportObject_H
