/****************************************************************************
** Copyright (C) 2001-2013 RibbonSoft, GmbH. All rights reserved.
**
** This file is part of the dxflib project.
**
** This file is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
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

#include "dl_global.h"

#include <string>
#include <vector>

/**
 * Layer Data.
 */
struct DXFLIB_EXPORT DL_LayerData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_LayerData(const std::string& lName,
                 int lFlags) {
        name = lName;
        flags = lFlags;
    }

    /** Layer name. */
    std::string name;
    /** Layer flags. (1 = frozen, 2 = frozen by default, 4 = locked) */
    int flags;
};



/**
 * Block Data.
 */
struct DXFLIB_EXPORT DL_BlockData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_BlockData(const std::string& bName,
                 int bFlags,
                 double bbpx, double bbpy, double bbpz) {
        name = bName;
        flags = bFlags;
        bpx = bbpx;
        bpy = bbpy;
        bpz = bbpz;
    }

    /** Block name. */
    std::string name;
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
 */
struct DXFLIB_EXPORT DL_LinetypeData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_LinetypeData(
        const std::string& name,
        const std::string& description,
        int flags,
        int numberOfDashes,
        double patternLength,
        double* pattern = NULL
        )
        : name(name),
        description(description),
        flags(flags),
        numberOfDashes(numberOfDashes),
        patternLength(patternLength),
        pattern(pattern)
    {}

    /** Linetype name */
    std::string name;
    /** Linetype description */
    std::string description;
    /** Linetype flags */
    int flags;
    /** Number of dashes */
    int numberOfDashes;
    /** Pattern length */
    double patternLength;
    /** Pattern */
    double* pattern;
};



/**
 * Text style data.
 */
struct DXFLIB_EXPORT DL_StyleData {
    /**
     * Constructor
     * Parameters: see member variables.
     */
    DL_StyleData(
        const std::string& name,
        int flags,
        double fixedTextHeight,
        double widthFactor,
        double obliqueAngle,
        int textGenerationFlags,
        double lastHeightUsed,
        const std::string& primaryFontFile,
        const std::string& bigFontFile
        )
        : name(name),
        flags(flags),
        fixedTextHeight(fixedTextHeight),
        widthFactor(widthFactor),
        obliqueAngle(obliqueAngle),
        textGenerationFlags(textGenerationFlags),
        lastHeightUsed(lastHeightUsed),
        primaryFontFile(primaryFontFile),
        bigFontFile(bigFontFile),
        bold(false),
        italic(false) {
    }

    bool operator==(const DL_StyleData& other) {
        // ignore lastHeightUsed:
        return (name==other.name &&
            flags==other.flags &&
            fixedTextHeight==other.fixedTextHeight &&
            widthFactor==other.widthFactor &&
            obliqueAngle==other.obliqueAngle &&
            textGenerationFlags==other.textGenerationFlags &&
            primaryFontFile==other.primaryFontFile &&
            bigFontFile==other.bigFontFile);
    }

    /** Style name */
    std::string name;
    /** Style flags */
    int flags;
    /** Fixed text height or 0 for not fixed. */
    double fixedTextHeight;
    /** Width factor */
    double widthFactor;
    /** Oblique angle */
    double obliqueAngle;
    /** Text generation flags */
    int textGenerationFlags;
    /** Last height used */
    double lastHeightUsed;
    /** Primary font file name */
    std::string primaryFontFile;
    /** Big font file name */
    std::string bigFontFile;

    bool bold;
    bool italic;
};

/**
 * Point Data.
 */
struct DXFLIB_EXPORT DL_PointData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_PointData(double px=0.0, double py=0.0, double pz=0.0) {
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
 */
struct DXFLIB_EXPORT DL_LineData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_LineData(double lx1, double ly1, double lz1,
                double lx2, double ly2, double lz2) {
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
 * XLine Data.
 */
struct DXFLIB_EXPORT DL_XLineData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_XLineData(double bx, double by, double bz,
                double dx, double dy, double dz) :
        bx(bx), by(by), bz(bz),
        dx(dx), dy(dy), dz(dz) {
    }

    /*! X base point. */
    double bx;
    /*! Y base point. */
    double by;
    /*! Z base point. */
    double bz;

    /*! X direction vector. */
    double dx;
    /*! Y direction vector. */
    double dy;
    /*! Z direction vector. */
    double dz;
};

/**
 * Ray Data.
 */
struct DXFLIB_EXPORT DL_RayData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_RayData(double bx, double by, double bz,
               double dx, double dy, double dz) :
        bx(bx), by(by), bz(bz),
        dx(dx), dy(dy), dz(dz) {
    }

    /*! X base point. */
    double bx;
    /*! Y base point. */
    double by;
    /*! Z base point. */
    double bz;

    /*! X direction vector. */
    double dx;
    /*! Y direction vector. */
    double dy;
    /*! Z direction vector. */
    double dz;
};



/**
 * Arc Data.
 */
struct DXFLIB_EXPORT DL_ArcData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_ArcData(double acx, double acy, double acz,
               double aRadius,
               double aAngle1, double aAngle2) {

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
 */
struct DXFLIB_EXPORT DL_CircleData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_CircleData(double acx, double acy, double acz,
                  double aRadius) {

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
 */
struct DXFLIB_EXPORT DL_PolylineData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_PolylineData(int pNumber, int pMVerteces, int pNVerteces, int pFlags) {
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
 */
struct DXFLIB_EXPORT DL_VertexData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_VertexData(double px=0.0, double py=0.0, double pz=0.0,
                  double pBulge=0.0) {
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
 * Trace Data / solid data / 3d face data.
 */
struct DXFLIB_EXPORT DL_TraceData {
    DL_TraceData() {
        thickness = 0.0;
        for (int i=0; i<4; i++) {
            x[i] = 0.0;
            y[i] = 0.0;
            z[i] = 0.0;
        }
    }
    
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_TraceData(double sx1, double sy1, double sz1,
                double sx2, double sy2, double sz2,
                double sx3, double sy3, double sz3,
                double sx4, double sy4, double sz4,
                double sthickness=0.0) {

        thickness = sthickness;

        x[0] = sx1;
        y[0] = sy1;
        z[0] = sz1;

        x[1] = sx2;
        y[1] = sy2;
        z[1] = sz2;
        
        x[2] = sx3;
        y[2] = sy3;
        z[2] = sz3;
        
        x[3] = sx4;
        y[3] = sy4;
        z[3] = sz4;
    }

    /*! Thickness */
    double thickness;
    
    /*! Points */
    double x[4];
    double y[4];
    double z[4];
};





/**
 * Solid Data.
 */
typedef DL_TraceData DL_SolidData;


/**
 * 3dface Data.
 */
typedef DL_TraceData DL_3dFaceData;


/**
 * Spline Data.
 */
struct DXFLIB_EXPORT DL_SplineData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_SplineData(int degree,
                  int nKnots,
                  int nControl,
                  int nFit,
                  int flags) :
        degree(degree),
        nKnots(nKnots),
        nControl(nControl),
        nFit(nFit),
        flags(flags) {
    }

    /*! Degree of the spline curve. */
    unsigned int degree;

    /*! Number of knots. */
    unsigned int nKnots;

    /*! Number of control points. */
    unsigned int nControl;

    /*! Number of fit points. */
    unsigned int nFit;

    /*! Flags */
    int flags;

    double tangentStartX;
    double tangentStartY;
    double tangentStartZ;
    double tangentEndX;
    double tangentEndY;
    double tangentEndZ;
};



/**
 * Spline knot data.
 */
struct DXFLIB_EXPORT DL_KnotData {
    DL_KnotData() {}
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_KnotData(double pk) {
        k = pk;
    }

    /*! Knot value. */
    double k;
};



/**
 * Spline control point data.
 */
struct DXFLIB_EXPORT DL_ControlPointData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_ControlPointData(double px, double py, double pz, double weight) {
        x = px;
        y = py;
        z = pz;
        w = weight;
    }

    /*! X coordinate of the control point. */
    double x;
    /*! Y coordinate of the control point. */
    double y;
    /*! Z coordinate of the control point. */
    double z;
    /*! Weight of control point. */
    double w;
};



/**
 * Spline fit point data.
 */
struct DXFLIB_EXPORT DL_FitPointData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_FitPointData(double x, double y, double z) : x(x), y(y), z(z) {}

    /*! X coordinate of the fit point. */
    double x;
    /*! Y coordinate of the fit point. */
    double y;
    /*! Z coordinate of the fit point. */
    double z;
};



/**
 * Ellipse Data.
 */
struct DXFLIB_EXPORT DL_EllipseData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_EllipseData(double cx, double cy, double cz,
                   double mx, double my, double mz,
                   double ratio,
                   double angle1, double angle2)
        : cx(cx),
          cy(cy),
          cz(cz),
          mx(mx),
          my(my),
          mz(mz),
          ratio(ratio),
          angle1(angle1),
          angle2(angle2) {
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
 */
struct DXFLIB_EXPORT DL_InsertData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_InsertData(const std::string& name,
                  double ipx, double ipy, double ipz,
                  double sx, double sy, double sz,
                  double angle,
                  int cols, int rows,
                  double colSp, double rowSp) :
          name(name),
          ipx(ipx), ipy(ipy), ipz(ipz),
          sx(sx), sy(sy), sz(sz),
          angle(angle),
          cols(cols), rows(rows),
          colSp(colSp), rowSp(rowSp) {
    }

    /*! Name of the referred block. */
    std::string name;
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
    /*! Rotation angle in degrees. */
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
 */
struct DXFLIB_EXPORT DL_MTextData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_MTextData(double ipx, double ipy, double ipz,
                 double dirx, double diry, double dirz,
                 double height, double width,
                 int attachmentPoint,
                 int drawingDirection,
                 int lineSpacingStyle,
                 double lineSpacingFactor,
                 const std::string& text,
                 const std::string& style,
                 double angle) :
         ipx(ipx), ipy(ipy), ipz(ipz),
         dirx(dirx), diry(diry), dirz(dirz),
         height(height), width(width),
         attachmentPoint(attachmentPoint),
         drawingDirection(drawingDirection),
         lineSpacingStyle(lineSpacingStyle),
         lineSpacingFactor(lineSpacingFactor),
         text(text),
         style(style),
         angle(angle) {

    }

    /*! X Coordinate of insertion point. */
    double ipx;
    /*! Y Coordinate of insertion point. */
    double ipy;
    /*! Z Coordinate of insertion point. */
    double ipz;
    /*! X Coordinate of X direction vector. */
    double dirx;
    /*! Y Coordinate of X direction vector. */
    double diry;
    /*! Z Coordinate of X direction vector. */
    double dirz;
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
    std::string text;
    /*! Style string. */
    std::string style;
    /*! Rotation angle. */
    double angle;
};



/**
 * Text Data.
 */
struct DXFLIB_EXPORT DL_TextData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_TextData(double ipx, double ipy, double ipz,
                double apx, double apy, double apz,
                double height, double xScaleFactor,
                int textGenerationFlags,
                int hJustification,
                int vJustification,
                const std::string& text,
                const std::string& style,
                double angle)
        : ipx(ipx), ipy(ipy), ipz(ipz),
          apx(apx), apy(apy), apz(apz),
          height(height), xScaleFactor(xScaleFactor),
          textGenerationFlags(textGenerationFlags),
          hJustification(hJustification),
          vJustification(vJustification),
          text(text),
          style(style),
          angle(angle) {
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
    std::string text;
    /*! Style (font). */
    std::string style;
    /*! Rotation angle of dimension text away from default orientation. */
    double angle;
};


/**
 * Block attribute data.
 */
struct DXFLIB_EXPORT DL_AttributeData : public DL_TextData {
    DL_AttributeData(const DL_TextData& tData, const std::string& tag)
        : DL_TextData(tData), tag(tag) {

    }

    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_AttributeData(double ipx, double ipy, double ipz,
                double apx, double apy, double apz,
                double height, double xScaleFactor,
                int textGenerationFlags,
                int hJustification,
                int vJustification,
                const std::string& tag,
                const std::string& text,
                const std::string& style,
                double angle)
        : DL_TextData(ipx, ipy, ipz,
                apx, apy, apz,
                height, xScaleFactor,
                textGenerationFlags,
                hJustification,
                vJustification,
                text,
                style,
                angle),
          tag(tag) {
    }

    /*! Tag. */
    std::string tag;
};


/**
 * Generic Dimension Data.
 */
struct DXFLIB_EXPORT DL_DimensionData {
    /**
    * Constructor.
    * Parameters: see member variables.
    */
    DL_DimensionData(double dpx, double dpy, double dpz,
                     double mpx, double mpy, double mpz,
                     int type,
                     int attachmentPoint,
                     int lineSpacingStyle,
                     double lineSpacingFactor,
                     const std::string& text,
                     const std::string& style,
                     double angle,
                     double linearFactor = 1.0) :
        dpx(dpx), dpy(dpy), dpz(dpz),
        mpx(mpx), mpy(mpy), mpz(mpz),
        type(type),
        attachmentPoint(attachmentPoint),
        lineSpacingStyle(lineSpacingStyle),
        lineSpacingFactor(lineSpacingFactor),
        text(text),
        style(style),
        angle(angle),
        linearFactor(linearFactor) {

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
    std::string text;
    /*! Dimension style (font name). */
    std::string style;
    /**
     * Rotation angle of dimension text away from
     * default orientation.
     */
    double angle;
    /**
     * Linear factor style override.
     */
    double linearFactor;
};



/**
 * Aligned Dimension Data.
 */
struct DXFLIB_EXPORT DL_DimAlignedData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_DimAlignedData(double depx1, double depy1, double depz1,
                      double depx2, double depy2, double depz2) {

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
 * Linear (rotated) Dimension Data.
 */
struct DXFLIB_EXPORT DL_DimLinearData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_DimLinearData(double ddpx1, double ddpy1, double ddpz1,
                     double ddpx2, double ddpy2, double ddpz2,
                     double dAngle, double dOblique) {

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
 */
struct DXFLIB_EXPORT DL_DimRadialData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_DimRadialData(double ddpx, double ddpy, double ddpz, double dleader) {
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
 */
struct DXFLIB_EXPORT DL_DimDiametricData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_DimDiametricData(double ddpx, double ddpy, double ddpz, double dleader) {
        dpx = ddpx;
        dpy = ddpy;
        dpz = ddpz;

        leader = dleader;
    }

    /*! X Coordinate of definition point (DXF 15). */
    double dpx;
    /*! Y Coordinate of definition point (DXF 25). */
    double dpy;
    /*! Z Coordinate of definition point (DXF 35). */
    double dpz;

    /*! Leader length */
    double leader;
};



/**
 * Angular Dimension Data.
 */
struct DXFLIB_EXPORT DL_DimAngularData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_DimAngularData(double ddpx1, double ddpy1, double ddpz1,
                      double ddpx2, double ddpy2, double ddpz2,
                      double ddpx3, double ddpy3, double ddpz3,
                      double ddpx4, double ddpy4, double ddpz4) {

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
 */
struct DXFLIB_EXPORT DL_DimAngular3PData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_DimAngular3PData(double ddpx1, double ddpy1, double ddpz1,
                        double ddpx2, double ddpy2, double ddpz2,
                        double ddpx3, double ddpy3, double ddpz3) {

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
 * Ordinate Dimension Data.
 */
struct DXFLIB_EXPORT DL_DimOrdinateData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_DimOrdinateData(double ddpx1, double ddpy1, double ddpz1,
                      double ddpx2, double ddpy2, double ddpz2,
                      bool dxtype) {

        dpx1 = ddpx1;
        dpy1 = ddpy1;
        dpz1 = ddpz1;

        dpx2 = ddpx2;
        dpy2 = ddpy2;
        dpz2 = ddpz2;

        xtype = dxtype;
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

    /*! True if the dimension indicates the X-value, false for Y-value */
    bool xtype;
};



/**
 * Leader (arrow).
 */
struct DXFLIB_EXPORT DL_LeaderData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_LeaderData(int lArrowHeadFlag,
                  int lLeaderPathType,
                  int lLeaderCreationFlag,
                  int lHooklineDirectionFlag,
                  int lHooklineFlag,
                  double lTextAnnotationHeight,
                  double lTextAnnotationWidth,
                  int lNumber) {

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
 */
struct DXFLIB_EXPORT DL_LeaderVertexData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_LeaderVertexData(double px=0.0, double py=0.0, double pz=0.0) {
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
struct DXFLIB_EXPORT DL_HatchData {
    /**
     * Default constructor.
     */
    DL_HatchData() {}

    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_HatchData(int numLoops,
                 bool solid,
                 double scale,
                 double angle,
                 const std::string& pattern,
                 double originX = 0.0,
                 double originY = 0.0) :
        numLoops(numLoops),
        solid(solid),
        scale(scale),
        angle(angle),
        pattern(pattern),
        originX(originX),
        originY(originY) {

    }

    /*! Number of boundary paths (loops). */
    int numLoops;
    /*! Solid fill flag (true=solid, false=pattern). */
    bool solid;
    /*! Pattern scale or spacing */
    double scale;
    /*! Pattern angle in degrees */
    double angle;
    /*! Pattern name. */
    std::string pattern;
    /*! Pattern origin */
    double originX;
    double originY;
};



/**
 * Hatch boundary path (loop) data.
 */
struct DXFLIB_EXPORT DL_HatchLoopData {
    /**
     * Default constructor.
     */
    DL_HatchLoopData() {}
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_HatchLoopData(int hNumEdges) {
        numEdges = hNumEdges;
    }

    /*! Number of edges in this loop. */
    int numEdges;
};



/**
 * Hatch edge data.
 */
struct DXFLIB_EXPORT DL_HatchEdgeData {
    /**
     * Default constructor.
     */
    DL_HatchEdgeData() : defined(false), x1(0.0), y1(0.0), x2(0.0), y2(0.0) {
    }

    /**
     * Constructor for a line edge.
     * Parameters: see member variables.
     */
    DL_HatchEdgeData(double x1, double y1,
                     double x2, double y2) :
        defined(true),
        type(1),
        x1(x1),
        y1(y1),
        x2(x2),
        y2(y2) {
    }

    /**
     * Constructor for an arc edge.
     * Parameters: see member variables.
     */
    DL_HatchEdgeData(double cx, double cy,
                     double radius,
                     double angle1, double angle2,
                     bool ccw) :
        defined(true),
        type(2),
        cx(cx),
        cy(cy),
        radius(radius),
        angle1(angle1),
        angle2(angle2),
        ccw(ccw) {
    }

    /**
     * Constructor for an ellipse arc edge.
     * Parameters: see member variables.
     */
    DL_HatchEdgeData(double cx, double cy,
                     double mx, double my,
                     double ratio,
                     double angle1, double angle2,
                     bool ccw) :
        defined(true),
        type(3),
        cx(cx),
        cy(cy),
        angle1(angle1),
        angle2(angle2),
        ccw(ccw),
        mx(mx),
        my(my),
        ratio(ratio) {
    }

    /**
     * Constructor for a spline edge.
     * Parameters: see member variables.
     */
    DL_HatchEdgeData(unsigned int degree,
                     bool rational,
                     bool periodic,
                     unsigned int nKnots,
                     unsigned int nControl,
                     unsigned int nFit,
                     const std::vector<double>& knots,
                     const std::vector<std::vector<double> >& controlPoints,
                     const std::vector<std::vector<double> >& fitPoints,
                     const std::vector<double>& weights,
                     double startTangentX,
                     double startTangentY,
                     double endTangentX,
                     double endTangentY) :
        defined(true),
        type(4),
        degree(degree),
        rational(rational),
        periodic(periodic),
        nKnots(nKnots),
        nControl(nControl),
        nFit(nFit),
        controlPoints(controlPoints),
        knots(knots),
        weights(weights),
        fitPoints(fitPoints),
        startTangentX(startTangentX),
        startTangentY(startTangentY),
        endTangentX(endTangentX),
        endTangentY(endTangentY) {
    }

    /**
     * Set to true if this edge is fully defined.
     */
    bool defined;

    /**
     * Edge type. 1=line, 2=arc, 3=elliptic arc, 4=spline.
     */
    int type;

    // line edges:

    /*! Start point (X). */
    double x1;
    /*! Start point (Y). */
    double y1;
    /*! End point (X). */
    double x2;
    /*! End point (Y). */
    double y2;

    /*! Center point of arc or ellipse arc (X). */
    double cx;
    /*! Center point of arc or ellipse arc (Y). */
    double cy;
    /*! Arc radius. */
    double radius;
    /*! Start angle of arc or ellipse arc. */
    double angle1;
    /*! End angle of arc or ellipse arc. */
    double angle2;
    /*! Counterclockwise flag for arc or ellipse arc. */
    bool ccw;

    /*! Major axis end point (X). */
    double mx;
    /*! Major axis end point (Y). */
    double my;
    /*! Axis ratio */
    double ratio;


    /*! Spline degree */
    unsigned int degree;
    bool rational;
    bool periodic;
    /*! Number of knots. */
    unsigned int nKnots;
    /*! Number of control points. */
    unsigned int nControl;
    /*! Number of fit points. */
    unsigned int nFit;

    std::vector<std::vector<double> > controlPoints;
    std::vector<double> knots;
    std::vector<double> weights;
    std::vector<std::vector<double> > fitPoints;

    double startTangentX;
    double startTangentY;

    double endTangentX;
    double endTangentY;

    /** Polyline boundary vertices (x y [bulge])*/
    std::vector<std::vector<double> > vertices;
    //bool closed;
};



/**
 * Image Data.
 */
struct DXFLIB_EXPORT DL_ImageData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_ImageData(const std::string& iref,
                  double iipx, double iipy, double iipz,
                  double iux, double iuy, double iuz,
                  double ivx, double ivy, double ivz,
                  int iwidth, int iheight,
                  int ibrightness, int icontrast, int ifade) {
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
    std::string ref;
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
 */
struct DXFLIB_EXPORT DL_ImageDefData {
    /**
     * Constructor.
     * Parameters: see member variables.
     */
    DL_ImageDefData(const std::string& iref,
                 const std::string& ifile) {
        ref = iref;
        file = ifile;
    }

    /*! Reference to the image file 
        (unique, used to refer to the image def object). */
    std::string ref;

    /*! Image file */
    std::string file;
};



/**
 * Dictionary data.
 */
struct DXFLIB_EXPORT DL_DictionaryData {
    DL_DictionaryData(const std::string& handle) : handle(handle) {}
    std::string handle;
};



/**
 * Dictionary entry data.
 */
struct DXFLIB_EXPORT DL_DictionaryEntryData {
    DL_DictionaryEntryData(const std::string& name, const std::string& handle) :
        name(name), handle(handle) {}

    std::string name;
    std::string handle;
};

#endif

// EOF
