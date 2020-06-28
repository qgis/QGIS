#include "qgs3dsceneexporter.h"

#include <QDebug>
#include <QVector>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QComponent>
#include <Qt3DCore/QNode>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometryRenderer>
#include <qgstessellatedpolygongeometry.h>
#include <qgs3dmapscene.h>
#include <QByteArray>
#include <QFile>
#include <QTextStream>
#include <QtMath>

Qgs3DSceneExporter::Qgs3DSceneExporter( )
  : mVertices(QVector<float>())
{

}


void Qgs3DSceneExporter::parseEntity(Qt3DCore::QEntity* entity) {
  if (entity == nullptr) return;
  for (Qt3DCore::QComponent *c : entity->components()) {
    Qt3DRender::QGeometryRenderer* comp = qobject_cast<Qt3DRender::QGeometryRenderer*>(c);
    if (comp == nullptr) continue;
    Qt3DRender::QGeometry* geom = comp->geometry();
    QgsTessellatedPolygonGeometry* polygonGeometry = qobject_cast<QgsTessellatedPolygonGeometry*>(geom);
    if (geom == nullptr) continue;
    if (polygonGeometry != nullptr) processGeometry(polygonGeometry);
//    for (Qt3DRender::QAttribute* attribute : geom->attributes()) {
//      processAttribute(attribute);
//    }
  }
  for (QObject* child : entity->children()) {
    Qt3DCore::QEntity* childEntity = qobject_cast<Qt3DCore::QEntity*>( child );
    if (childEntity != nullptr) parseEntity(childEntity);
  }
}

void Qgs3DSceneExporter::processAttribute(Qt3DRender::QAttribute* attribute) {
  qDebug() << __PRETTY_FUNCTION__;
  Qt3DRender::QBuffer* buffer = attribute->buffer();
  QByteArray bufferData = buffer->data();
  uint bytesOffset = attribute->byteOffset();
  uint bytesStride = attribute->byteStride();
  uint verticesCount = attribute->count();
  uint vertexSize = attribute->vertexSize();

  QDataStream stream(bufferData);

  for (int offset = bytesOffset; offset < vertexSize * verticesCount * bytesStride; offset += bytesStride) {
    QVector<float> verticeData;
    for (int i = 0; i < vertexSize; ++i) {
      float v;
      stream >> v;
      if (qIsInf(v)) break;
      verticeData.push_back(v);
    }
    if (verticeData.size() == vertexSize) {
      mVertices.append(verticeData);
      qDebug() << "v" << verticeData[0] << verticeData[1] << verticeData[2];
    }
  }
}

void Qgs3DSceneExporter::saveToFile(const QString& filePath) {
  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;

  qDebug() << filePath;
  QTextStream out(&file);

  // Construct vertices
  for (int i = 0; i < mVertices.size(); i += 3) {
    out << "v " << mVertices[i] << " " << mVertices[i + 1] << " " << mVertices[i + 2] << "\n";
//    qDebug() << "v " << mVertices[i] << " " << mVertices[i + 1] << " " << mVertices[i + 2] << "\n";

  }
  // Construct faces
  for (int i = 0; i < mVertices.size() / 9; ++i) {
    out << "f " << 3 * i + 1 << " " << 3 * i + 2 << " " << 3 * i + 3 << "\n";
//    qDebug() << "f " << 3 * i << " " << 3 * i + 1 << " " << 3 * i + 2 << "\n";
  }
}

void Qgs3DSceneExporter::processGeometry(QgsTessellatedPolygonGeometry* geom) {
  QByteArray data = geom->mVertexBuffer->data();
  Qt3DRender::QAttribute* attribute = geom->mPositionAttribute;
  uint bytesOffset = attribute->byteOffset();
  uint bytesStride = attribute->byteStride();
  uint verticesCount = attribute->count();
  uint vertexSize = attribute->vertexSize();

  QVector<float> floatData;
  QDataStream dataStream(data);
  while (!dataStream.atEnd()) {
    float f;
    dataStream >> f;
    floatData.push_back(f);
  }

  for (int i = 0; i < floatData.size(); i += bytesStride / sizeof(float)) {
    mVertices << floatData[i] << floatData[i + 1] << floatData[i + 2];
  }
}
