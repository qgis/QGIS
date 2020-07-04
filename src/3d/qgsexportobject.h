#ifndef QGSEXPORTOBJECT_H
#define QGSEXPORTOBJECT_H

#include <QObject>
#include <QTextStream>
#include <QVector>
#include <QVector3D>

// Manages the data of each object of the scene (positions, normals, texture coordinates ...)
class QgsExportObject : public QObject {
    Q_OBJECT
  public:
    QgsExportObject(const QString& name, const QString& parentName = QString(), QObject* parent = nullptr);

    bool smoothEdges() { return mSmoothEdges; }
    void setSmoothEdges(bool smoothEdges) { mSmoothEdges = smoothEdges; }

    void setupPositionCoordinates( const QVector<float>& positionsBuffer, float scale=1.0f, const QVector3D translation=QVector3D(0, 0, 0) );
    void setupPositionCoordinates( const QVector<float>& positionsBuffer, const QVector<unsigned int>& facesIndexes, float scale=1.0f, const QVector3D translation=QVector3D(0, 0, 0) );
    void objectBounds(float& minX, float& minY, float& minZ, float& maxX, float& maxY, float& maxZ);

    void saveTo(QTextStream& out, int scale, const QVector3D& center);
  private:
    QString mName;
    QString mParentName;
    QVector<float> mVertxPosition;
    QVector<int> mIndexes;

    bool mSmoothEdges;
};

#endif // QGSEXPORTOBJECT_H
