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
#include <QMatrix4x4>

#include "qgis_3d.h"
#include "qgs3dtypes.h"

#define SIP_NO_FILE

class QgsAbstractMaterialSettings;

/**
 * \brief Manages the data of each object of the scene (positions, normals, texture coordinates ...) since each object
 *
 * \note Not available in Python bindings
 *
 * \ingroup qgis_3d
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
    Qgs3DExportObject( const QString &name )
      : mName( name ) {}

    //! Returns the object name
    QString name() const { return mName; }
    //! Sets the object name
    void setName( const QString &name ) { mName = name; }

    //! Returns the object type
    ObjectType type() const { return mType; }

    //! Returns whether object edges will look smooth
    bool smoothEdges() const { return mSmoothEdges; }
    //! Sets whether triangles edges will look smooth
    void setSmoothEdges( bool smoothEdges ) { mSmoothEdges = smoothEdges; }

    //! sets line indexes and positions coordinates
    void setupLine( const QVector<float> &positionsBuffer );

    //! sets triangle indexes and positions coordinates
    void setupTriangle( const QVector<float> &positionsBuffer, const QVector<uint> &facesIndexes, const QMatrix4x4 &transform );

    //! sets point positions coordinates
    void setupPoint( const QVector<float> &positionsBuffer );

    //! Sets normal coordinates for each vertex
    void setupNormalCoordinates( const QVector<float> &normalsBuffer, const QMatrix4x4 &transform );
    //! Sets texture coordinates for each vertex
    void setupTextureCoordinates( const QVector<float> &texturesBuffer );
    //! Sets the material parameters (diffuse color, shininess...) to be exported in the .mtl file
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
    void objectBounds( float &minX, float &minY, float &minZ, float &maxX, float &maxY, float &maxZ ) const;

    //! Saves the current object to the output stream while scaling the object and centering it to be visible in exported scene
    void saveTo( QTextStream &out, float scale, const QVector3D &center, const Qgs3DTypes::ExportFormat &exportFormat = Qgs3DTypes::ExportFormat::Obj, int precision = 6, const QString &materialName = QString() ) const;
    //! saves the texture of the object and material information
    QString saveMaterial( QTextStream &mtlOut, const QString &folder ) const;

    //! Returns the vertex coordinates
    QVector<float> vertexPosition() const { return mVertexPosition; }

    //! Returns the vertex normal coordinates
    QVector<float> normals() const { return mNormals; }

    //! Returns the vertex texture coordinates
    QVector<float> texturesUV() const { return mTexturesUV; }

    //! Returns the vertex indexes
    QVector<unsigned int> indexes() const { return mIndexes; }

  private:
    //! Sets positions coordinates and does the translation, rotation and scaling
    void setupPositionCoordinates( const QVector<float> &positionsBuffer, const QMatrix4x4 &transform = QMatrix4x4() );

    //! Saves the current object to the output stream in Obj format
    void saveToObj( QTextStream &out, float scale, const QVector3D &center, int precision = 6, const QString &materialName = QString() ) const;

    //! Saves the current object to the output stream in Stl format
    void saveToStl( QTextStream &out, float scale, const QVector3D &center, int precision = 6 ) const;

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
