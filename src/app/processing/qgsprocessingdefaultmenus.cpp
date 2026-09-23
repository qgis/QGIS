/***************************************************************************
    qgsprocessingdefaultmenus.cpp
    ---------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingdefaultmenus.h"

QMap<Qgis::ProcessingMenu, QStringList> QgsProcessingDefaultMenus::defaultProcessingMenuEntries()
{
  return {
    { Qgis::ProcessingMenu::VectorAnalysis,
      { "native:distancematrix",
        "native:sumlinelengths",
        "native:countpointsinpolygon",
        "native:listuniquevalues",
        "native:basicstatisticsforfields",
        "native:nearestneighbouranalysis",
        "native:meancoordinates",
        "native:lineintersections" } },
    { Qgis::ProcessingMenu::VectorResearch,
      { "native:creategrid",
        "native:randomselection",
        "native:randomselectionwithinsubsets",
        "native:randompointsinextent",
        "qgis:randompointsinlayerbounds",
        "native:randompointsinpolygons",
        "qgis:randompointsinsidepolygons",
        "native:randompointsonlines",
        "qgis:regularpoints",
        "native:selectbylocation",
        "native:selectwithindistance",
        "native:polygonfromlayerextent" } },
    { Qgis::ProcessingMenu::VectorGeoprocessing,
      { "native:buffer", "native:convexhull", "native:intersection", "native:union", "native:symmetricaldifference", "native:clip", "native:difference", "native:dissolve", "qgis:eliminateselectedpolygons" } },
    { Qgis::ProcessingMenu::VectorGeometry,
      { "native:checkvalidity",
        "native:exportaddgeometrycolumns",
        "native:centroids",
        "native:delaunaytriangulation",
        "native:voronoipolygons",
        "native:simplifygeometries",
        "native:densifygeometries",
        "native:multiparttosingleparts",
        "native:collect",
        "native:polygonstolines",
        "qgis:linestopolygons",
        "native:extractvertices" } },
    { Qgis::ProcessingMenu::VectorDataManagement, { "native:reprojectlayer", "native:joinattributesbylocation", "native:splitvectorlayer", "native:mergevectorlayers", "native:createspatialindex" } },
    { Qgis::ProcessingMenu::RasterProjections, { "gdal:warpreproject", "gdal:extractprojection", "gdal:assignprojection" } },
    { Qgis::ProcessingMenu::RasterConversion, { "gdal:rasterize", "gdal:polygonize", "gdal:translate", "gdal:rgbtopct", "gdal:pcttorgb" } },
    { Qgis::ProcessingMenu::RasterExtraction, { "gdal:contour", "gdal:cliprasterbyextent", "gdal:cliprasterbymasklayer" } },
    { Qgis::ProcessingMenu::RasterAnalysis,
      { "gdal:sieve",
        "gdal:nearblack",
        "gdal:fillnodata",
        "gdal:proximity",
        "gdal:griddatametrics",
        "gdal:gridaverage",
        "gdal:gridinversedistance",
        "gdal:gridnearestneighbor",
        "gdal:aspect",
        "gdal:hillshade",
        "gdal:roughness",
        "gdal:slope",
        "gdal:tpitopographicpositionindex",
        "gdal:triterrainruggednessindex" } },
    { Qgis::ProcessingMenu::RasterMiscellaneous, { "gdal:buildvirtualraster", "gdal:merge", "gdal:gdalinfo", "gdal:overviews", "gdal:tileindex" } },
    { Qgis::ProcessingMenu::RasterGeneral, { "native:alignrasters" } }
  };
}
