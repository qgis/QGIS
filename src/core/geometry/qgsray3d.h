#ifndef QGSRAY3D_H
#define QGSRAY3D_H

#include "qgsbox3d.h"

#include <QVector3D>

class CORE_EXPORT QgsRay3D
{
  public:
    QgsRay3D( const QVector3D &origin, const QVector3D &direction );

    QVector3D origin() const { return mOrigin; }
    QVector3D direction() const { return mDirection; }

    void setOrigin( const QVector3D &origin );
    void setDirection( const QVector3D direction );

    bool intersectsWith( const QgsBox3d &box ) const;
    bool isInFront( const QVector3D &point ) const;
    double angleToPoint( const QVector3D &point ) const;
    double distanceTo( const  QgsBox3d &box ) const;

  private:
    QVector3D mOrigin;
    QVector3D mDirection;
};

#endif // QGSRAY3D_H
