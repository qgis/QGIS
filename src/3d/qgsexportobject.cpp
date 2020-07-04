#include "qgsexportobject.h"

#include <QVector3D>

QgsExportObject::QgsExportObject(const QString& name, const QString& parentName, QObject* parent)
  : QObject(parent)
  , mName(name)
  , mParentName(parentName)
  , mSmoothEdges(true)
{
}

void QgsExportObject::setupPositionCoordinates( const QVector<float>& positionsBuffer, float scale, const QVector3D translation ) {
  for ( int i = 0; i < positionsBuffer.size(); i += 3 )
  {
    for ( int j = 0; j < 3; ++j )
    {
      mVertxPosition << positionsBuffer[i + j] * scale + translation[j];
    }
  }

  for ( int i = 0; i < positionsBuffer.size() / 3; ++i)
  {
    mIndexes << i + 1;
  }
}

void QgsExportObject::setupPositionCoordinates( const QVector<float>& positionsBuffer, const QVector<unsigned int>& faceIndex, float scale, const QVector3D translation )
{
  // TODO: delete vertices that are not used
  for ( int i = 0; i < positionsBuffer.size(); i += 3 )
  {
    for ( int j = 0; j < 3; ++j )
    {
      mVertxPosition << positionsBuffer[i + j] * scale + translation[j];
    }
  }

  for ( int i = 0; i < faceIndex.size(); i += 3 )
  {
    if (faceIndex[i] == faceIndex[i + 1] && faceIndex[i + 1] == faceIndex[i + 2]) continue;
    for (int j = 0; j < 3; ++j) mIndexes << faceIndex[i + j] + 1;
  }
}

void QgsExportObject::objectBounds(float& minX, float& minY, float& minZ, float& maxX, float& maxY, float& maxZ) {
  for (int vertice : mIndexes) {
    int heightIndex = (vertice - 1) * 3 + 1;
    minX = std::min(minX, mVertxPosition[heightIndex - 1]);
    maxX = std::max(maxX, mVertxPosition[heightIndex - 1]);
    minY = std::min(minY, mVertxPosition[heightIndex]);
    maxY = std::max(maxY, mVertxPosition[heightIndex]);
    minZ = std::min(minZ, mVertxPosition[heightIndex + 1]);
    maxZ = std::max(maxZ, mVertxPosition[heightIndex + 1]);
  }
}

void QgsExportObject::saveTo(QTextStream& out, int scale, const QVector3D& center) {

  // Set object name
  out << "o " << mName << "\n";

  // Set groups
  // turns out grouping doest work as expected in blender

  // smoothen edges
  if ( mSmoothEdges ) out << "s on\n";
  else out << "s off\n";

  // Construct vertices
  for ( int i = 0; i < mVertxPosition.size(); i += 3 )
  {
    // for now just ignore wrong vertex positions
    out << "v ";
    out << ( mVertxPosition[i] - center.x() ) / scale << " ";
    out << ( mVertxPosition[i + 1] - center.y() ) / scale << " ";
    out << ( mVertxPosition[i + 2] - center.z() ) / scale << "\n";
  }

  // Construct faces
  int verticesCount = mVertxPosition.size() / 3;
  for ( int i = 0; i < mIndexes.size(); i += 3 )
  {
    if (mIndexes[i] == mIndexes[i + 1] && mIndexes[i + 1] == mIndexes[i + 2]) continue;
    out << "f " << -1 - (verticesCount - mIndexes[i]) << " " << -1 - (verticesCount - mIndexes[i + 1]) << " " << -1 - (verticesCount - mIndexes[i + 2]) << "\n";
  }
}
