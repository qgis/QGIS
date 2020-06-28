#ifndef QGS3DSCENEEXPORTER_H
#define QGS3DSCENEEXPORTER_H

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>

class QgsTessellatedPolygonGeometry;
class Qgs3DMapSettings;

class Qgs3DSceneExporter
{

public:
  Qgs3DSceneExporter( );

  void parseEntity(Qt3DCore::QEntity* entity);
  void saveToFile(const QString& filePath);
private:
  void processAttribute(Qt3DRender::QAttribute* attribute);
  void processGeometry(QgsTessellatedPolygonGeometry* geom);
private:
  QVector<float> mVertices;
};

#endif // QGS3DSCENEEXPORTER_H
