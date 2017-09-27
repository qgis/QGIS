#ifndef QGSFLATTERRAINGENERATOR_H
#define QGSFLATTERRAINGENERATOR_H

#include "qgis_3d.h"

#include "qgsterraingenerator.h"

#include "qgsrectangle.h"


/** \ingroup 3d
 * Terrain generator that creates a simple square flat area.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsFlatTerrainGenerator : public QgsTerrainGenerator
{
  public:
    QgsFlatTerrainGenerator();

    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;

    virtual QgsTerrainGenerator *clone() const override;
    Type type() const override;
    QgsRectangle extent() const override;
    virtual void rootChunkHeightRange( float &hMin, float &hMax ) const override;
    virtual void writeXml( QDomElement &elem ) const override;
    virtual void readXml( const QDomElement &elem ) override;

    //! Sets extent of the terrain
    void setExtent( const QgsRectangle &extent );

    //! Sets CRS of the terrain
    void setCrs( const QgsCoordinateReferenceSystem &crs );
    //! Returns CRS of the terrain
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

  private:

    void updateTilingScheme();

    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
};




#endif // QGSFLATTERRAINGENERATOR_H
