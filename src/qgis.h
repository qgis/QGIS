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
static const char *qgisVersion = "0.0.5-alpha build 20020820";
}
#endif
