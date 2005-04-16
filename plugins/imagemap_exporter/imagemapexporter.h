#ifndef IMAGEMAPEXPORTER
#define IMAGEMAPEXPORTER

#include <qstring.h>
#include <qtextstream.h>

#include <qgsfeature.h>
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>


class ImageMapExporter {
public:

  bool generateImageMap(QString& templateFile, QString& outputFile,
			int radius, QString& format, int urlField, 
			int altField, QgsVectorLayer* layer,
			QgsMapCanvas* canvas, QgsRasterLayer* raster);

private:  
  /** Polygons in shapefiles are built from one or several "rings", which are
      closed simple curves. If the points in the ring are given in clockwise
      order the ring defines the outer boundary of a polygon, if they are given
      in counterclockwise order they define an inner boundary, a hole. This
      function checks the winding of a ring (clockwise or counterclockwise)
      and returns @c true if it is an inner boundary and @c false if it is an
      outer boundary.
      @param points A pointer to the start of the ring. The ring should be
                    given as an array of doubles with alternating x and y 
		    values.
      @param nPoints The number of points in the ring (including the end point,
                     which should be equal to the first point).
  */
  static bool polygonIsHole(const double* points, int nPoints);
  
  /** This function writes a cluster index. */
  static void writeClusterIndex(const std::vector<QString>& urls,
				const std::vector<QString>& alts,
				QTextStream& stream, int clusterID, 
				bool popup);
  
  /** This is used for internal sorting algorithms. */
  static bool cluster_comp(std::pair<QgsFeature*, int> i, 
			   std::pair<QgsFeature*, int> j) {
    return i.second < j.second;
  }
  

};


#endif
