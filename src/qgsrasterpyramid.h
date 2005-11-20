/** \brief This struct is used to store pyramid info for the raster layer. */
class QgsRasterPyramid
{
  public:
  /** \brief The pyramid level as implemented in gdal (level 2 is half orignal raster size etc) */
  int levelInt;
  /** \brief XDimension for this pyramid layer */
  int xDimInt;
  /** \brief YDimension for this pyramid layer */
  int yDimInt;
  /** \brief Whether the pyramid layer has been built yet */
  bool existsFlag;

};

