#ifndef _IMAGEWRITER_H_
#define _IMAGEWRITER_H_

#include <qstring.h>
#include "gdal_priv.h"
/** This class will write a png image given an input gdal compatible datasource.
 * The output file will be colour mapped using a pseudocolor renderer.
 **/
struct BandStats {
	QString bandName;
	int bandNo;
	double minValDouble;
	double maxValDouble;
	//the distance between min & max
	double rangeDouble;
	double meanDouble;
	double sumSqrDevDouble; //used to calculate stddev
	double stdDevDouble;
	double sumDouble;
	int elementCountInt;
	double noDataDouble;
};

class ImageWriter
{

public:

  /** Default constructor */
  ImageWriter();
  /** Destructor  */
   ~ImageWriter();
  /** Constructor taking the name of the file to open. */
  void writeImage(QString theInputFileString, QString theOutputFileString);
private:
  /** Calculate image statistics */
  void calculateStats(BandStats * theBandStats,GDALDataset * gdalDataset);
  //
  //   Private attributes
  //

};

#endif

