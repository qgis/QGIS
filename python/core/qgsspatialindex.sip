
class QgsSpatialIndex
{
%TypeHeaderCode
#include "qgsspatialindex.h"
%End

public:

  /* creation of spatial index */

  /** create new spatial index that will be stored in memory */
  //static QgsSpatialIndex* createMemoryIndex();

  /** create new spatial index that stores its data on disk */
  //static QgsSpatialIndex* createDiskIndex(QString fileName);

  /** constructor - creates R-tree */
  QgsSpatialIndex();
  
  /** destructor finalizes work with spatial index */
  ~QgsSpatialIndex();


  /* operations */

  /** add feature to index */
  bool insertFeature(QgsFeature& f);

  /** remove feature from index */
  bool deleteFeature(QgsFeature& f);


  /* queries */

  /** returns features that intersect the specified rectangle */
  QList<qint64> intersects(QgsRectangle rect);

  /** returns nearest neighbors (their count is specified by second parameter) */
  QList<qint64> nearestNeighbor(QgsPoint point, int neighbors);


};

