/****************************************************************************
** $Id: dl_entities.h 2398 2005-06-06 18:12:14Z andrew $
**
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
** This file is part of the dxflib project.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This program is free software; you can redistribute it and/or modify  
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; version 2 of the License
**
** Licensees holding valid dxflib Professional Edition licenses may use
** this file in accordance with the dxflib Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.ribbonsoft.com for further details.
**
** Contact info@ribbonsoft.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DL_ENTITIES_H
#define DL_ENTITIES_H


#include <string>
using std::string;

/**
 * Layer Data.
 *
 * @author Andrew Mustun
 */
struct DL_LayerData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_LayerData( const string& lName,
                int lFlags )
  {
    name = lName;
    flags = lFlags;
  }

  /** Layer name. */
  string name;
  /** Layer flags. (1 = frozen, 2 = frozen by default, 4 = locked) */
  int flags;
};



/**
 * Block Data.
 *
 * @author Andrew Mustun
 */
struct DL_BlockData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_BlockData( const string& bName,
                int bFlags,
                double bbpx, double bbpy, double bbpz )
  {
    name = bName;
    flags = bFlags;
    bpx = bbpx;
    bpy = bbpy;
    bpz = bbpz;
  }

  /** Block name. */
  string name;
  /** Block flags. (not used currently) */
  int flags;
  /** X Coordinate of base point. */
  double bpx;
  /** Y Coordinate of base point. */
  double bpy;
  /** Z Coordinate of base point. */
  double bpz;
};



/**
 * Line Type Data.
 *
 * @author Andrew Mustun
 */
struct DL_LineTypeData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_LineTypeData( const string& lName,
                   int lFlags )
  {
    name = lName;
    flags = lFlags;
  }

  /** Line type name. */
  string name;
  /** Line type flags. */
  int flags;
};



/**
 * Point Data.
 *
 * @author Andrew Mustun
 */
struct DL_PointData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_PointData( double px = 0.0, double py = 0.0, double pz = 0.0 )
  {
    x = px;
    y = py;
    z = pz;
  }

  /*! X Coordinate of the point. */
  double x;
  /*! Y Coordinate of the point. */
  double y;
  /*! Z Coordinate of the point. */
  double z;
};



/**
 * Line Data.
 *
 * @author Andrew Mustun
 */
struct DL_LineData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_LineData( double lx1, double ly1, double lz1,
               double lx2, double ly2, double lz2 )
  {
    x1 = lx1;
    y1 = ly1;
    z1 = lz1;

    x2 = lx2;
    y2 = ly2;
    z2 = lz2;
  }

  /*! X Start coordinate of the point. */
  double x1;
  /*! Y Start coordinate of the point. */
  double y1;
  /*! Z Start coordinate of the point. */
  double z1;

  /*! X End coordinate of the point. */
  double x2;
  /*! Y End coordinate of the point. */
  double y2;
  /*! Z End coordinate of the point. */
  double z2;
};



/**
 * Arc Data.
 *
 * @author Andrew Mustun
 */
struct DL_ArcData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_ArcData( double acx, double acy, double acz,
              double aRadius,
              double aAngle1, double aAngle2 )
  {

    cx = acx;
    cy = acy;
    cz = acz;
    radius = aRadius;
    angle1 = aAngle1;
    angle2 = aAngle2;
  }

  /*! X Coordinate of center point. */
  double cx;
  /*! Y Coordinate of center point. */
  double cy;
  /*! Z Coordinate of center point. */
  double cz;

  /*! Radius of arc. */
  double radius;
  /*! Startangle of arc in degrees. */
  double angle1;
  /*! Endangle of arc in degrees. */
  double angle2;
};



/**
 * Circle Data.
 *
 * @author Andrew Mustun
 */
struct DL_CircleData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_CircleData( double acx, double acy, double acz,
                 double aRadius )
  {

    cx = acx;
    cy = acy;
    cz = acz;
    radius = aRadius;
  }

  /*! X Coordinate of center point. */
  double cx;
  /*! Y Coordinate of center point. */
  double cy;
  /*! Z Coordinate of center point. */
  double cz;

  /*! Radius of arc. */
  double radius;
};



/**
 * Polyline Data.
 *
 * @author Andrew Mustun
 */
struct DL_PolylineData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_PolylineData( int pNumber, int pMVerteces, int pNVerteces, int pFlags )
  {
    number = pNumber;
    m = pMVerteces;
    n = pNVerteces;
    flags = pFlags;
  }

  /*! Number of vertices in this polyline. */
  unsigned int number;

  /*! Number of vertices in m direction if polyline is a polygon mesh. */
  unsigned int m;

  /*! Number of vertices in n direction if polyline is a polygon mesh. */
  unsigned int n;

  /*! Flags */
  int flags;
};



/**
 * Vertex Data.
 *
 * @author Andrew Mustun
 */
struct DL_VertexData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_VertexData( double px = 0.0, double py = 0.0, double pz = 0.0,
                 double pBulge = 0.0 )
  {
    x = px;
    y = py;
    z = pz;
    bulge = pBulge;
  }

  /*! X Coordinate of the vertex. */
  double x;
  /*! Y Coordinate of the vertex. */
  double y;
  /*! Z Coordinate of the vertex. */
  double z;
  /*! Bulge of vertex.
   * (The tangent of 1/4 of the arc angle or 0 for lines) */
  double bulge;
};


/**
 * Trace Data.
 *
 * @author AHM
 */
struct DL_TraceData
{
  double x[4];
  double y[4];
  double z[4];
};


/**
 * Solid Data.
 *
 * @author AHM
 */
typedef DL_TraceData DL_SolidData;


/**
 * Spline Data.
 *
 * @author Andrew Mustun
 */
struct DL_SplineData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_SplineData( int pDegree, int pNKnots, int pNControl, int pFlags )
  {
    degree = pDegree;
    nKnots = pNKnots;
    nControl = pNControl;
    flags = pFlags;
  }

  /*! Degree of the spline curve. */
  unsigned int degree;

  /*! Number of knots. */
  unsigned int nKnots;

  /*! Number of control points. */
  unsigned int nControl;

  /*! Flags */
  int flags;
};



/**
 * Spline knot data.
 *
 * @author Andrew Mustun
 */
struct DL_KnotData
{
  DL_KnotData() {}
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_KnotData( double pk )
  {
    k = pk;
  }

  /*! Knot value. */
  double k;
};



/**
 * Spline control point data.
 *
 * @author Andrew Mustun
 */
struct DL_ControlPointData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_ControlPointData( double px, double py, double pz )
  {
    x = px;
    y = py;
    z = pz;
  }

  /*! X coordinate of the control point. */
  double x;
  /*! Y coordinate of the control point. */
  double y;
  /*! Z coordinate of the control point. */
  double z;
};


/**
 * Ellipse Data.
 *
 * @author Andrew Mustun
 */
struct DL_EllipseData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_EllipseData( double ecx, double ecy, double ecz,
                  double emx, double emy, double emz,
                  double eRatio,
                  double eAngle1, double eAngle2 )
  {

    cx = ecx;
    cy = ecy;
    cz = ecz;
    mx = emx;
    my = emy;
    mz = emz;
    ratio = eRatio;
    angle1 = eAngle1;
    angle2 = eAngle2;
  }

  /*! X Coordinate of center point. */
  double cx;
  /*! Y Coordinate of center point. */
  double cy;
  /*! Z Coordinate of center point. */
  double cz;

  /*! X coordinate of the endpoint of the major axis. */
  double mx;
  /*! Y coordinate of the endpoint of the major axis. */
  double my;
  /*! Z coordinate of the endpoint of the major axis. */
  double mz;

  /*! Ratio of minor axis to major axis.. */
  double ratio;
  /*! Startangle of ellipse in rad. */
  double angle1;
  /*! Endangle of ellipse in rad. */
  double angle2;
};



/**
 * Insert Data.
 *
 * @author Andrew Mustun
 */
struct DL_InsertData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_InsertData( const string& iName,
                 double iipx, double iipy, double iipz,
                 double isx, double isy, double isz,
                 double iAngle,
                 int iCols, int iRows,
                 double iColSp, double iRowSp )
  {
    name = iName;
    ipx = iipx;
    ipy = iipy;
    ipz = iipz;
    sx = isx;
    sy = isy;
    sz = isz;
    angle = iAngle;
    cols = iCols;
    rows = iRows;
    colSp = iColSp;
    rowSp = iRowSp;
  }

  /*! Name of the referred block. */
  string name;
  /*! X Coordinate of insertion point. */
  double ipx;
  /*! Y Coordinate of insertion point. */
  double ipy;
  /*! Z Coordinate of insertion point. */
  double ipz;
  /*! X Scale factor. */
  double sx;
  /*! Y Scale factor. */
  double sy;
  /*! Z Scale factor. */
  double sz;
  /*! Rotation angle in rad. */
  double angle;
  /*! Number of colums if we insert an array of the block or 1. */
  int cols;
  /*! Number of rows if we insert an array of the block or 1. */
  int rows;
  /*! Values for the spacing between cols. */
  double colSp;
  /*! Values for the spacing between rows. */
  double rowSp;
};



/**
 * MText Data.
 *
 * @author Andrew Mustun
 */
struct DL_MTextData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_MTextData( double tipx, double tipy, double tipz,
                double tHeight, double tWidth,
                int tAttachmentPoint,
                int tDrawingDirection,
                int tLineSpacingStyle,
                double tLineSpacingFactor,
                const string& tText,
                const string& tStyle,
                double tAngle )
  {
    ipx = tipx;
    ipy = tipy;
    ipz = tipz;

    height = tHeight;
    width = tWidth;
    attachmentPoint = tAttachmentPoint;
    drawingDirection = tDrawingDirection;
    lineSpacingStyle = tLineSpacingStyle;
    lineSpacingFactor = tLineSpacingFactor;
    text = tText;
    style = tStyle;
    angle = tAngle;
  }

  /*! X Coordinate of insertion point. */
  double ipx;
  /*! Y Coordinate of insertion point. */
  double ipy;
  /*! Z Coordinate of insertion point. */
  double ipz;
  /*! Text height */
  double height;
  /*! Width of the text box. */
  double width;
  /**
   * Attachment point.
   *
   * 1 = Top left, 2 = Top center, 3 = Top right,
   * 4 = Middle left, 5 = Middle center, 6 = Middle right,
   * 7 = Bottom left, 8 = Bottom center, 9 = Bottom right
   */
  int attachmentPoint;
  /**
   * Drawing direction.
   *
   * 1 = left to right, 3 = top to bottom, 5 = by style
   */
  int drawingDirection;
  /**
   * Line spacing style.
   *
   * 1 = at least, 2 = exact
   */
  int lineSpacingStyle;
  /**
   * Line spacing factor. 0.25 .. 4.0
   */
  double lineSpacingFactor;
  /*! Text string. */
  string text;
  /*! Style string. */
  string style;
  /*! Rotation angle. */
  double angle;
};



/**
 * Text Data.
 *
 * @author Andrew Mustun
 */
struct DL_TextData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_TextData( double tipx, double tipy, double tipz,
               double tapx, double tapy, double tapz,
               double tHeight, double tXScaleFactor,
               int tTextGenerationFlags,
               int tHJustification,
               int tVJustification,
               const string& tText,
               const string& tStyle,
               double tAngle )
  {
    ipx = tipx;
    ipy = tipy;
    ipz = tipz;

    apx = tapx;
    apy = tapy;
    apz = tapz;

    height = tHeight;
    xScaleFactor = tXScaleFactor;
    textGenerationFlags = tTextGenerationFlags;
    hJustification = tHJustification;
    vJustification = tVJustification;
    text = tText;
    style = tStyle;
    angle = tAngle;
  }

  /*! X Coordinate of insertion point. */
  double ipx;
  /*! Y Coordinate of insertion point. */
  double ipy;
  /*! Z Coordinate of insertion point. */
  double ipz;

  /*! X Coordinate of alignment point. */
  double apx;
  /*! Y Coordinate of alignment point. */
  double apy;
  /*! Z Coordinate of alignment point. */
  double apz;

  /*! Text height */
  double height;
  /*! Relative X scale factor. */
  double xScaleFactor;
  /*! 0 = default, 2 = Backwards, 4 = Upside down */
  int textGenerationFlags;
  /**
   * Horizontal justification.
   *
   * 0 = Left (default), 1 = Center, 2 = Right,
   * 3 = Aligned, 4 = Middle, 5 = Fit
   * For 3, 4, 5 the vertical alignment has to be 0.
   */
  int hJustification;
  /**
   * Vertical justification.
   *
   * 0 = Baseline (default), 1 = Bottom, 2 = Middle, 3= Top
   */
  int vJustification;
  /*! Text string. */
  string text;
  /*! Style (font). */
  string style;
  /*! Rotation angle of dimension text away from default orientation. */
  double angle;
};



/**
 * Generic Dimension Data.
 *
 * @author Andrew Mustun
 */
struct DL_DimensionData
{
  /**
  * Constructor.
  * Parameters: see member variables.
  */
  DL_DimensionData( double ddpx, double ddpy, double ddpz,
                    double dmpx, double dmpy, double dmpz,
                    int dType,
                    int dAttachmentPoint,
                    int dLineSpacingStyle,
                    double dLineSpacingFactor,
                    const string& dText,
                    const string& dStyle,
                    double dAngle )
  {

    dpx = ddpx;
    dpy = ddpy;
    dpz = ddpz;

    mpx = dmpx;
    mpy = dmpy;
    mpz = dmpz;

    type = dType;

    attachmentPoint = dAttachmentPoint;
    lineSpacingStyle = dLineSpacingStyle;
    lineSpacingFactor = dLineSpacingFactor;
    text = dText;
    style = dStyle;
    angle = dAngle;
  }

  /*! X Coordinate of definition point. */
  double dpx;
  /*! Y Coordinate of definition point. */
  double dpy;
  /*! Z Coordinate of definition point. */
  double dpz;
  /*! X Coordinate of middle point of the text. */
  double mpx;
  /*! Y Coordinate of middle point of the text. */
  double mpy;
  /*! Z Coordinate of middle point of the text. */
  double mpz;
  /**
   * Dimension type.
   *
   * 0   Rotated, horizontal, or vertical
   * 1   Aligned
   * 2   Angular
   * 3   Diametric
   * 4   Radius
   * 5   Angular 3-point
   * 6   Ordinate
   * 64  Ordinate type. This is a bit value (bit 7)
   *     used only with integer value 6. If set,
   *     ordinate is X-type; if not set, ordinate is
   *     Y-type
   * 128 This is a bit value (bit 8) added to the
   *     other group 70 values if the dimension text
   *     has been positioned at a user-defined
   *    location rather than at the default location
   */
  int type;
  /**
   * Attachment point.
   *
   * 1 = Top left, 2 = Top center, 3 = Top right,
   * 4 = Middle left, 5 = Middle center, 6 = Middle right,
   * 7 = Bottom left, 8 = Bottom center, 9 = Bottom right,
   */
  int attachmentPoint;
  /**
   * Line spacing style.
   *
   * 1 = at least, 2 = exact
   */
  int lineSpacingStyle;
  /**
   * Line spacing factor. 0.25 .. 4.0
   */
  double lineSpacingFactor;
  /**
   * Text string.
   *
   * Text string entered explicitly by user or null
   * or "<>" for the actual measurement or " " (one blank space).
   * for supressing the text.
   */
  string text;
  /*! Dimension style (font name). */
  string style;
  /**
  * Rotation angle of dimension text away from
   * default orientation.
  */
  double angle;
};



/**
 * Aligned Dimension Data.
 *
 * @author Andrew Mustun
 */
struct DL_DimAlignedData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_DimAlignedData( double depx1, double depy1, double depz1,
                     double depx2, double depy2, double depz2 )
  {

    epx1 = depx1;
    epy1 = depy1;
    epz1 = depz1;

    epx2 = depx2;
    epy2 = depy2;
    epz2 = depz2;
  }

  /*! X Coordinate of Extension point 1. */
  double epx1;
  /*! Y Coordinate of Extension point 1. */
  double epy1;
  /*! Z Coordinate of Extension point 1. */
  double epz1;

  /*! X Coordinate of Extension point 2. */
  double epx2;
  /*! Y Coordinate of Extension point 2. */
  double epy2;
  /*! Z Coordinate of Extension point 2. */
  double epz2;
};



/**
 * Linear Dimension Data.
 *
 * @author Andrew Mustun
 */
struct DL_DimLinearData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_DimLinearData( double ddpx1, double ddpy1, double ddpz1,
                    double ddpx2, double ddpy2, double ddpz2,
                    double dAngle, double dOblique )
  {

    dpx1 = ddpx1;
    dpy1 = ddpy1;
    dpz1 = ddpz1;

    dpx2 = ddpx2;
    dpy2 = ddpy2;
    dpz2 = ddpz2;

    angle = dAngle;
    oblique = dOblique;
  }

  /*! X Coordinate of Extension point 1. */
  double dpx1;
  /*! Y Coordinate of Extension point 1. */
  double dpy1;
  /*! Z Coordinate of Extension point 1. */
  double dpz1;

  /*! X Coordinate of Extension point 2. */
  double dpx2;
  /*! Y Coordinate of Extension point 2. */
  double dpy2;
  /*! Z Coordinate of Extension point 2. */
  double dpz2;

  /*! Rotation angle (angle of dimension line) in degrees. */
  double angle;
  /*! Oblique angle in degrees. */
  double oblique;
};



/**
 * Radial Dimension Data.
 *
 * @author Andrew Mustun
 */
struct DL_DimRadialData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_DimRadialData( double ddpx, double ddpy, double ddpz, double dleader )
  {
    dpx = ddpx;
    dpy = ddpy;
    dpz = ddpz;

    leader = dleader;
  }

  /*! X Coordinate of definition point. */
  double dpx;
  /*! Y Coordinate of definition point. */
  double dpy;
  /*! Z Coordinate of definition point. */
  double dpz;

  /*! Leader length */
  double leader;
};



/**
 * Diametric Dimension Data.
 *
 * @author Andrew Mustun
 */
struct DL_DimDiametricData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_DimDiametricData( double ddpx, double ddpy, double ddpz, double dleader )
  {
    dpx = ddpx;
    dpy = ddpy;
    dpz = ddpz;

    leader = dleader;
  }

  /*! X Coordinate of definition point. */
  double dpx;
  /*! Y Coordinate of definition point. */
  double dpy;
  /*! Z Coordinate of definition point. */
  double dpz;

  /*! Leader length */
  double leader;
};



/**
 * Angular Dimension Data.
 *
 * @author Andrew Mustun
 */
struct DL_DimAngularData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_DimAngularData( double ddpx1, double ddpy1, double ddpz1,
                     double ddpx2, double ddpy2, double ddpz2,
                     double ddpx3, double ddpy3, double ddpz3,
                     double ddpx4, double ddpy4, double ddpz4 )
  {

    dpx1 = ddpx1;
    dpy1 = ddpy1;
    dpz1 = ddpz1;

    dpx2 = ddpx2;
    dpy2 = ddpy2;
    dpz2 = ddpz2;

    dpx3 = ddpx3;
    dpy3 = ddpy3;
    dpz3 = ddpz3;

    dpx4 = ddpx4;
    dpy4 = ddpy4;
    dpz4 = ddpz4;
  }

  /*! X Coordinate of definition point 1. */
  double dpx1;
  /*! Y Coordinate of definition point 1. */
  double dpy1;
  /*! Z Coordinate of definition point 1. */
  double dpz1;

  /*! X Coordinate of definition point 2. */
  double dpx2;
  /*! Y Coordinate of definition point 2. */
  double dpy2;
  /*! Z Coordinate of definition point 2. */
  double dpz2;

  /*! X Coordinate of definition point 3. */
  double dpx3;
  /*! Y Coordinate of definition point 3. */
  double dpy3;
  /*! Z Coordinate of definition point 3. */
  double dpz3;

  /*! X Coordinate of definition point 4. */
  double dpx4;
  /*! Y Coordinate of definition point 4. */
  double dpy4;
  /*! Z Coordinate of definition point 4. */
  double dpz4;
};


/**
 * Angular Dimension Data (3 points version).
 *
 * @author Andrew Mustun
 */
struct DL_DimAngular3PData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_DimAngular3PData( double ddpx1, double ddpy1, double ddpz1,
                       double ddpx2, double ddpy2, double ddpz2,
                       double ddpx3, double ddpy3, double ddpz3 )
  {

    dpx1 = ddpx1;
    dpy1 = ddpy1;
    dpz1 = ddpz1;

    dpx2 = ddpx2;
    dpy2 = ddpy2;
    dpz2 = ddpz2;

    dpx3 = ddpx3;
    dpy3 = ddpy3;
    dpz3 = ddpz3;
  }

  /*! X Coordinate of definition point 1. */
  double dpx1;
  /*! Y Coordinate of definition point 1. */
  double dpy1;
  /*! Z Coordinate of definition point 1. */
  double dpz1;

  /*! X Coordinate of definition point 2. */
  double dpx2;
  /*! Y Coordinate of definition point 2. */
  double dpy2;
  /*! Z Coordinate of definition point 2. */
  double dpz2;

  /*! X Coordinate of definition point 3. */
  double dpx3;
  /*! Y Coordinate of definition point 3. */
  double dpy3;
  /*! Z Coordinate of definition point 3. */
  double dpz3;
};



/**
 * Leader (arrow).
 *
 * @author Andrew Mustun
 */
struct DL_LeaderData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_LeaderData( int lArrowHeadFlag,
                 int lLeaderPathType,
                 int lLeaderCreationFlag,
                 int lHooklineDirectionFlag,
                 int lHooklineFlag,
                 double lTextAnnotationHeight,
                 double lTextAnnotationWidth,
                 int lNumber )
  {

    arrowHeadFlag = lArrowHeadFlag;
    leaderPathType = lLeaderPathType;
    leaderCreationFlag = lLeaderCreationFlag;
    hooklineDirectionFlag = lHooklineDirectionFlag;
    hooklineFlag = lHooklineFlag;
    textAnnotationHeight = lTextAnnotationHeight;
    textAnnotationWidth = lTextAnnotationWidth;
    number = lNumber;
  }

  /*! Arrow head flag (71). */
  int arrowHeadFlag;
  /*! Leader path type (72). */
  int leaderPathType;
  /*! Leader creation flag (73). */
  int leaderCreationFlag;
  /*! Hookline direction flag (74). */
  int hooklineDirectionFlag;
  /*! Hookline flag (75) */
  int hooklineFlag;
  /*! Text annotation height (40). */
  double textAnnotationHeight;
  /*! Text annotation width (41) */
  double textAnnotationWidth;
  /*! Number of vertices in leader (76). */
  int number;
};



/**
 * Leader Vertex Data.
 *
 * @author Andrew Mustun
 */
struct DL_LeaderVertexData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_LeaderVertexData( double px = 0.0, double py = 0.0, double pz = 0.0 )
  {
    x = px;
    y = py;
    z = pz;
  }

  /*! X Coordinate of the vertex. */
  double x;
  /*! Y Coordinate of the vertex. */
  double y;
  /*! Z Coordinate of the vertex. */
  double z;
};



/**
 * Hatch data.
 */
struct DL_HatchData
{
  /**
   * Default constructor.
   */
  DL_HatchData() {}

  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_HatchData( int hNumLoops,
                bool hSolid,
                double hScale,
                double hAngle,
                const string& hPattern )
  {
    numLoops = hNumLoops;
    solid = hSolid;
    scale = hScale;
    angle = hAngle;
    pattern = hPattern;
  }

  /*! Number of boundary paths (loops). */
  int numLoops;
  /*! Solid fill flag (true=solid, false=pattern). */
  bool solid;
  /*! Pattern scale or spacing */
  double scale;
  /*! Pattern angle */
  double angle;
  /*! Pattern name. */
  string pattern;
};



/**
 * Hatch boundary path (loop) data.
 */
struct DL_HatchLoopData
{
  /**
   * Default constructor.
   */
  DL_HatchLoopData() {}
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_HatchLoopData( int hNumEdges )
  {
    numEdges = hNumEdges;
  }

  /*! Number of edges in this loop. */
  int numEdges;
};



/**
 * Hatch edge data.
 */
struct DL_HatchEdgeData
{
  /**
   * Default constructor.
   */
  DL_HatchEdgeData()
  {
    defined = false;
  }

  /**
   * Constructor for a line edge.
   * Parameters: see member variables.
   */
  DL_HatchEdgeData( double lx1, double ly1,
                    double lx2, double ly2 )
  {
    x1 = lx1;
    y1 = ly1;
    x2 = lx2;
    y2 = ly2;
    type = 1;
    defined = true;
  }

  /**
   * Constructor for an arc edge.
   * Parameters: see member variables.
   */
  DL_HatchEdgeData( double acx, double acy,
                    double aRadius,
                    double aAngle1, double aAngle2,
                    bool aCcw )
  {
    cx = acx;
    cy = acy;
    radius = aRadius;
    angle1 = aAngle1;
    angle2 = aAngle2;
    ccw = aCcw;
    type = 2;
    defined = true;
  }

  /**
   * Edge type. 1=line, 2=arc.
   */
  int type;

  /**
   * Set to true if this edge is fully defined.
   */
  bool defined;

  /*! Start point (X). */
  double x1;
  /*! Start point (Y). */
  double y1;
  /*! End point (X). */
  double x2;
  /*! End point (Y). */
  double y2;
  /*! Center point of arc (X). */
  double cx;
  /*! Center point of arc (Y). */
  double cy;
  /*! Arc radius. */
  double radius;
  /*! Start angle. */
  double angle1;
  /*! End angle. */
  double angle2;
  /*! Counterclockwise flag. */
  bool ccw;
};



/**
 * Image Data.
 *
 * @author Andrew Mustun
 */
struct DL_ImageData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_ImageData( const string& iref,
                double iipx, double iipy, double iipz,
                double iux, double iuy, double iuz,
                double ivx, double ivy, double ivz,
                int iwidth, int iheight,
                int ibrightness, int icontrast, int ifade )
  {
    ref = iref;
    ipx = iipx;
    ipy = iipy;
    ipz = iipz;
    ux = iux;
    uy = iuy;
    uz = iuz;
    vx = ivx;
    vy = ivy;
    vz = ivz;
    width = iwidth;
    height = iheight;
    brightness = ibrightness;
    contrast = icontrast;
    fade = ifade;
  }

  /*! Reference to the image file
   (unique, used to refer to the image def object). */
  string ref;
  /*! X Coordinate of insertion point. */
  double ipx;
  /*! Y Coordinate of insertion point. */
  double ipy;
  /*! Z Coordinate of insertion point. */
  double ipz;
  /*! X Coordinate of u vector along bottom of image. */
  double ux;
  /*! Y Coordinate of u vector along bottom of image. */
  double uy;
  /*! Z Coordinate of u vector along bottom of image. */
  double uz;
  /*! X Coordinate of v vector along left side of image. */
  double vx;
  /*! Y Coordinate of v vector along left side of image. */
  double vy;
  /*! Z Coordinate of v vector along left side of image. */
  double vz;
  /*! Width of image in pixel. */
  int width;
  /*! Height of image in pixel. */
  int height;
  /*! Brightness (0..100, default = 50). */
  int brightness;
  /*! Contrast (0..100, default = 50). */
  int contrast;
  /*! Fade (0..100, default = 0). */
  int fade;
};



/**
 * Image Definition Data.
 *
 * @author Andrew Mustun
 */
struct DL_ImageDefData
{
  /**
   * Constructor.
   * Parameters: see member variables.
   */
  DL_ImageDefData( const string& iref,
                   const string& ifile )
  {
    ref = iref;
    file = ifile;
  }

  /*! Reference to the image file
   (unique, used to refer to the image def object). */
  string ref;

  /*! Image file */
  string file;
};

#endif

// EOF

