/***************************************************************************
  qgsdistancearea.cpp - Distance and area calculations on the ellipsoid
 ---------------------------------------------------------------------------
  Date                 : September 2005
  Copyright            : (C) 2005 by Martin Dobias
  email                : won.der at centrum.sk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <math.h>
#include <sqlite3.h>
#include <qdir.h>
#include <qsettings.h>

#include "qgis.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgscoordinatetransform.h"
#include "qgsspatialrefsys.h"
#include "qgsgeometry.h"
#include "qgsdistancearea.h"

#define DEG2RAD(x)    ((x)*M_PI/180)


QgsDistanceArea::QgsDistanceArea()
{
  // init with default settings
  mCoordTransform = new QgsCoordinateTransform;
  setDefaultEllipsoid();
  setProjectAsSourceSRS();
}


QgsDistanceArea::~QgsDistanceArea()
{
  delete mCoordTransform;
}


void QgsDistanceArea::setSourceSRS(long srsid)
{
  QgsSpatialRefSys srcSRS;
  srcSRS.createFromSrsId(srsid);
  mCoordTransform->setSourceSRS(srcSRS);
}


void QgsDistanceArea::setProjectAsSourceSRS()
{
  int projEnabled = QgsProject::instance()->readNumEntry("SpatialRefSys","/ProjectionsEnabled",0);
  long srsid = GEOSRS_ID;
  if (projEnabled)
  {
    srsid = QgsProject::instance()->readNumEntry("SpatialRefSys","/ProjectSRSID",GEOSRS_ID);
  }
  setSourceSRS(srsid);
}


bool QgsDistanceArea::setEllipsoid(const QString& ellipsoid)
{
  QString radius, parameter2;
  //
  // SQLITE3 stuff - get parameters for selected ellipsoid
  //
  QString mQGisSettingsDir = QDir::homeDirPath () + "/.qgis/";
  sqlite3      *myDatabase;
  char         *myErrorMessage = 0;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(QString(mQGisSettingsDir+"qgis.db").latin1(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    return false;
  }
  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  QString mySql = "select radius, parameter2 from tbl_ellipsoid where acronym='" + ellipsoid + "'";
  myResult = sqlite3_prepare(myDatabase, (const char *)mySql, mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    if (sqlite3_step(myPreparedStatement) == SQLITE_ROW)
    {
      radius = QString((char *)sqlite3_column_text(myPreparedStatement,0));
      parameter2 = QString((char *)sqlite3_column_text(myPreparedStatement,1));
    }
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  
  // row for this ellipsoid wasn't found?
  if (radius.isEmpty() || parameter2.isEmpty())
  {
#ifdef QGISDEBUG
    std::cout << "setEllipsoid: no row in tbl_ellipsoid for acronym '" << ellipsoid.local8Bit() << "'" << std::endl;
#endif
    return false;
  }
  
  // get major semiaxis
  if (radius.left(2) == "a=")
    mSemiMajor = radius.mid(2).toDouble();
  else
  {
#ifdef QGISDEBUG
    std::cout << "setEllipsoid: wrong format of radius field: '" << radius.local8Bit() << "'" << std::endl;
#endif
    return false;
  }
  
  // get second parameter
  // one of values 'b' or 'f' is in field parameter2
  // second one must be computed using formula: invf = a/(a-b)
  if (parameter2.left(2) == "b=")
  {
    mSemiMinor = parameter2.mid(2).toDouble();
    mInvFlattening = mSemiMajor / (mSemiMajor - mSemiMinor);
  }
  else if (parameter2.left(3) == "rf=")
  {
    mInvFlattening = parameter2.mid(3).toDouble();
    mSemiMinor = mSemiMajor - (mInvFlattening / mSemiMajor);
  }
  else
  {
#ifdef QGISDEBUG
    std::cout << "setEllipsoid: wrong format of parameter2 field: '" << parameter2.local8Bit() << "'" << std::endl;
#endif
    return false;
  }
  
#ifdef QGISDEBUG
    std::cout << "setEllipsoid: a=" << mSemiMajor << ", b=" << mSemiMinor << ", 1/f=" << mInvFlattening << std::endl;
#endif


  // get spatial ref system for ellipsoid
  QString proj4 = "+proj=longlat +ellps=";
  proj4 += ellipsoid;
  QgsSpatialRefSys destSRS;
  destSRS.createFromProj4(proj4);
  
  // set transformation from project SRS to ellipsoid coordinates
  mCoordTransform->setDestSRS(destSRS);

  // precalculate some values for area calculations
  computeAreaInit();
  
  mEllipsoid = ellipsoid;
  return true;
}


bool QgsDistanceArea::setDefaultEllipsoid()
{
  QSettings settings;
  QString ellipsoid = settings.readEntry("/qgis/measure/ellipsoid", "WGS84");
  return setEllipsoid(ellipsoid);
}


double QgsDistanceArea::measure(QgsGeometry* geometry)
{
  size_t size = geometry->wkbSize();
  unsigned char* wkb = geometry->wkbBuffer();
  unsigned char* ptr;
  unsigned int wkbType;
  double res, resTotal = 0;
  int count, i;
  
  memcpy(&wkbType, (wkb+1), sizeof(wkbType));

  // measure distance or area based on what is the type of geometry
  switch (wkbType)
  {
    case QGis::WKBLineString:
      measureLine(wkb, &res);
      return res;
      
    case QGis::WKBMultiLineString:
      count = *((int*)(wkb+5));
      ptr = wkb+9;
      for (i = 0; i < count; i++)
      {
        ptr = measureLine(ptr, &res);
        resTotal += res;
      }
      return resTotal;
      
    case QGis::WKBPolygon:
      measurePolygon(wkb, &res);
      return res;
      
    case QGis::WKBMultiPolygon:
      count = *((int*)(wkb+5));
      ptr = wkb+9;
      for (i = 0; i < count; i++)
      {
        ptr = measurePolygon(ptr, &res);
        resTotal += res;
      }
      return resTotal;
      
    default:
      std::cout << "measure: unexpected geometry type: " << wkbType << std::endl;
      return 0;
  }
}


unsigned char* QgsDistanceArea::measureLine(unsigned char* feature, double* area)
{
  unsigned char *ptr = feature + 5;
  unsigned int nPoints = *((int*)ptr);
  ptr = feature + 9;
  
  std::vector<QgsPoint> points(nPoints);

  // Extract the points from the WKB format into the vector
  for (unsigned int i = 0; i < nPoints; ++i)
  {
    QgsPoint& p = points[i];
    p.setX(*((double *) ptr));
    ptr += sizeof(double);
    p.setY(*((double *) ptr));
    ptr += sizeof(double);
  }
  
  *area = measureLine(points);
  return ptr;
}

double QgsDistanceArea::measureLine(const std::vector<QgsPoint>& points)
{
  if (points.size() < 2)
    return 0;
  
  double total = 0;
  QgsPoint p1, p2;
  p1 = mCoordTransform->transform(points[0]);
  
  for (int i = 1; i < points.size(); i++)
  {
    p2 = mCoordTransform->transform(points[i]);
    total = computeDistanceBearing(p1,p2);
    p1 = p2;
  }
  return total;
}

double QgsDistanceArea::measureLine(const QgsPoint& p1, const QgsPoint& p2)
{
  QgsPoint pp1 = mCoordTransform->transform(p1);
  QgsPoint pp2 = mCoordTransform->transform(p2);
  return computeDistanceBearing(pp1, pp2);
}


unsigned char* QgsDistanceArea::measurePolygon(unsigned char* feature, double* area)
{
  // get number of rings in the polygon
  unsigned int numRings = *((int*)(feature + 1 + sizeof(int)));
  
  if (numRings == 0)
    return 0;
  
  // Set pointer to the first ring
  unsigned char* ptr = feature + 1 + 2 * sizeof(int); 
  
  std::vector<QgsPoint> points;
  double x,y, areaTmp;
  *area = 0;

  for (unsigned int idx = 0; idx < numRings; idx++)
  {
    int nPoints = *((int*)ptr);
    points.resize(nPoints);
    ptr += 4;

    // Extract the points from the WKB and store in a pair of
    // vectors.
    for (unsigned int jdx = 0; jdx < nPoints; jdx++)
    {
      x = *((double *) ptr);
      ptr += sizeof(double);
      y = *((double *) ptr);
      ptr += sizeof(double);
    
      points[jdx] = mCoordTransform->transform(QgsPoint(x,y));
    }
  
    if (points.size() > 2)
    {
      bool clockWise = true;
      areaTmp = computePolygonArea(points);
      if (idx == 0)
        *area += areaTmp; // exterior ring
      else
        *area -= areaTmp; // interior rings
    }
  }
  
  return ptr;
}


double QgsDistanceArea::measurePolygon(const std::vector<QgsPoint>& points)
{
  std::vector<QgsPoint> pts(points.size());
  for (int i = 0; i < points.size(); i++)
  {
    pts[i] = mCoordTransform->transform(points[i]);
  }
  return computePolygonArea(pts);
}



///////////////////////////////////////////////////////////
// distance calculation

double QgsDistanceArea::computeDistanceBearing(
                  const QgsPoint& p1, const QgsPoint& p2,
                  double* course1, double* course2)
{
  if (p1.x() == p2.x() && p1.y() == p2.y())
    return 0;
  
  // ellipsoid
  double a = mSemiMajor;
  double b = mSemiMinor;
  double f = 1 / mInvFlattening;
  
  double p1_lat = DEG2RAD(p1.y()), p1_lon = DEG2RAD(p1.x());
  double p2_lat = DEG2RAD(p2.y()), p2_lon = DEG2RAD(p2.x());
  
  double L = p2_lon - p1_lon;
  double U1 = atan((1-f) * tan(p1_lat));
  double U2 = atan((1-f) * tan(p2_lat));
  double sinU1 = sin(U1), cosU1 = cos(U1);
  double sinU2 = sin(U2), cosU2 = cos(U2);
  double lambda = L;
  double lambdaP = 2*M_PI;
  
  double sinLambda, cosLambda, sinSigma, cosSigma;
  double sigma, alpha, cosSqAlpha, cos2SigmaM, C;
  double tu1, tu2;
  
  int iterLimit = 20;
  while (fabs(lambda-lambdaP) > 1e-12 && --iterLimit>0)
  {
    sinLambda = sin(lambda);
    cosLambda = cos(lambda);
    tu1 = (cosU2*sinLambda);
    tu2 = (cosU1*sinU2-sinU1*cosU2*cosLambda);
    sinSigma = sqrt(tu1 * tu1 + tu2 * tu2);
    cosSigma = sinU1*sinU2 + cosU1*cosU2*cosLambda;
    sigma = atan2(sinSigma, cosSigma);
    alpha = asin(cosU1 * cosU2 * sinLambda / sinSigma);
    cosSqAlpha = cos(alpha) * cos(alpha);
    cos2SigmaM = cosSigma - 2*sinU1*sinU2/cosSqAlpha;
    C = f/16*cosSqAlpha*(4+f*(4-3*cosSqAlpha));
    lambdaP = lambda;
    lambda = L + (1-C) * f * sin(alpha) *
        (sigma + C*sinSigma*(cos2SigmaM+C*cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)));
  }
  
  if (iterLimit == 0)
    return -1;  // formula failed to converge
  
  double uSq = cosSqAlpha*(a*a-b*b)/(b*b);
  double A = 1 + uSq/16384*(4096+uSq*(-768+uSq*(320-175*uSq)));
  double B = uSq/1024 * (256+uSq*(-128+uSq*(74-47*uSq)));
  double deltaSigma = B*sinSigma*(cos2SigmaM+B/4*(cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)-
      B/6*cos2SigmaM*(-3+4*sinSigma*sinSigma)*(-3+4*cos2SigmaM*cos2SigmaM)));
  double s = b*A*(sigma-deltaSigma);
  
  if (course1)
  {
    *course1 = atan2(tu1, tu2);
  }
  if (course2)
  {
    // PI is added to return azimuth from P2 to P1
    *course2 = atan2(cosU1*sinLambda, -sinU1*cosU2 + cosU1*sinU2*cosLambda) + M_PI;
  }
  
  return s;
}



///////////////////////////////////////////////////////////
// stuff for measuring areas - copied from GRASS
// don't know how does it work, but it's working .)
// see G_begin_ellipsoid_polygon_area() in area_poly1.c

double QgsDistanceArea::getQ(double x)
{
  double sinx, sinx2;

  sinx = sin(x);
  sinx2 = sinx * sinx;

  return sinx * (1 + sinx2 * (m_QA + sinx2 * (m_QB + sinx2 * m_QC)));
}


double QgsDistanceArea::getQbar(double x)
{
  double cosx, cosx2;

  cosx = cos(x);
  cosx2 = cosx * cosx;

  return cosx * (m_QbarA + cosx2 * (m_QbarB + cosx2 * (m_QbarC + cosx2 * m_QbarD)));
}


void QgsDistanceArea::computeAreaInit()
{
  double a2 = (mSemiMajor*mSemiMajor);
  double e2 = 1 - (a2/(mSemiMinor*mSemiMinor));
  double e4, e6;

  m_TwoPI = M_PI+M_PI;

  e4 = e2 * e2;
  e6 = e4 * e2;

  m_AE = a2 * (1 - e2);

  m_QA = (2.0/3.0)*e2;
  m_QB = (3.0/5.0)*e4;
  m_QC = (4.0/7.0)*e6;

  m_QbarA = -1.0 - (2.0/3.0)*e2 - (3.0/5.0)*e4  -  (4.0/7.0)*e6;
  m_QbarB =        (2.0/9.0)*e2 + (2.0/5.0)*e4  +  (4.0/7.0)*e6;
  m_QbarC =                     - (3.0/25.0)*e4 - (12.0/35.0)*e6;
  m_QbarD =                                        (4.0/49.0)*e6;

  m_Qp = getQ(M_PI/2);
  m_E  = 4 * M_PI * m_Qp * m_AE;
  if (m_E < 0.0) m_E = -m_E;
}


double QgsDistanceArea::computePolygonArea(const std::vector<QgsPoint>& points)
{
  double x1,y1,x2,y2,dx,dy;
  double Qbar1, Qbar2;
  double area;

  int n = points.size();
  x2 = DEG2RAD(points[n-1].x());
  y2 = DEG2RAD(points[n-1].y());
  Qbar2 = getQbar(y2);

  area = 0.0;

  for (int i = 0; i < n; i++)
  {
    x1 = x2;
    y1 = y2;
    Qbar1 = Qbar2;

    x2 = DEG2RAD(points[i].x());
    y2 = DEG2RAD(points[i].y());
    Qbar2 = getQbar(y2);

    if (x1 > x2)
      while (x1 - x2 > M_PI)
        x2 += m_TwoPI;
    else if (x2 > x1)
      while (x2 - x1 > M_PI)
        x1 += m_TwoPI;

    dx = x2 - x1;
    area += dx * (m_Qp - getQ(y2));

    if ((dy = y2 - y1) != 0.0)
      area += dx * getQ(y2) - (dx/dy)*(Qbar2-Qbar1);
  }
  if((area *= m_AE) < 0.0)
    area = -area;

    /* kludge - if polygon circles the south pole the area will be
  * computed as if it cirlced the north pole. The correction is
  * the difference between total surface area of the earth and
  * the "north pole" area.
    */
  if (area > m_E) area = m_E;
  if (area > m_E/2) area = m_E - area;

  return area;
}

