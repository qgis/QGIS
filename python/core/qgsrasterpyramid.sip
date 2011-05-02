/** \brief This struct is used to store pyramid info for the raster layer. */
class QgsRasterPyramid
{
%TypeHeaderCode
#include <qgsrasterpyramid.h>
%End

  public:
  /** \brief The pyramid level as implemented in gdal (level 2 is half orignal raster size etc) */
  int level;
  /** \brief XDimension for this pyramid layer */
  int xDim;
  /** \brief YDimension for this pyramid layer */
  int yDim;
  /** \brief Whether the pyramid layer has been built yet */
  bool exists;
  /** \brief Whether the pyramid should be built */
  bool build;

  QgsRasterPyramid();
};
