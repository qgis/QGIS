#ifndef QGIS_H
#define QGIS_H	
namespace QGis  {
enum MapTools {
	ZoomIn,
	ZoomOut,
	Pan,
	Distance,
	Identify
};
//! Used for symbology operations
enum WKBTYPE{
	WKBPoint=1,
	WKBLineString,
	WKBPolygon,
	WKBMultiPoint,
	WKBMultiLineString,
	WKBMultiPolygon
    };	

}
#endif
