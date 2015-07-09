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

#ifndef DL_CREATIONADAPTER_H
#define DL_CREATIONADAPTER_H

#include "dl_global.h"

#include "dl_creationinterface.h"

/**
 * An abstract adapter class for receiving DXF events when a DXF file is being read. 
 * The methods in this class are empty. This class exists as convenience for creating 
 * listener objects.
 *
 * @author Andrew Mustun
 */
class DXFLIB_EXPORT DL_CreationAdapter : public DL_CreationInterface {
public:
    DL_CreationAdapter() {}
    virtual ~DL_CreationAdapter() {}
    virtual void processCodeValuePair(unsigned int, const std::string&) {}
    virtual void endSection() {}
    virtual void addLayer(const DL_LayerData&) {}
    virtual void addLinetype(const DL_LinetypeData&) {}
    virtual void addLinetypeDash(double) {}
    virtual void addBlock(const DL_BlockData&) {}
    virtual void endBlock() {}
    virtual void addTextStyle(const DL_StyleData&) {}
    virtual void addPoint(const DL_PointData&) {}
    virtual void addLine(const DL_LineData&) {}
    virtual void addXLine(const DL_XLineData&) {}
    virtual void addRay(const DL_RayData&) {}

    virtual void addArc(const DL_ArcData&) {}
    virtual void addCircle(const DL_CircleData&) {}
    virtual void addEllipse(const DL_EllipseData&) {}
    
    virtual void addPolyline(const DL_PolylineData&) {}
    virtual void addVertex(const DL_VertexData&) {}
    
    virtual void addSpline(const DL_SplineData&) {}
    virtual void addControlPoint(const DL_ControlPointData&) {}
    virtual void addFitPoint(const DL_FitPointData&) {}
    virtual void addKnot(const DL_KnotData&) {}
    
    virtual void addInsert(const DL_InsertData&) {}
    
    virtual void addMText(const DL_MTextData&) {}
    virtual void addMTextChunk(const std::string&) {}
    virtual void addText(const DL_TextData&) {}
    virtual void addAttribute(const DL_AttributeData&) {}
    
    virtual void addDimAlign(const DL_DimensionData&,
                             const DL_DimAlignedData&) {}
    virtual void addDimLinear(const DL_DimensionData&,
                              const DL_DimLinearData&) {}
    virtual void addDimRadial(const DL_DimensionData&,
                              const DL_DimRadialData&) {}
    virtual void addDimDiametric(const DL_DimensionData&,
                              const DL_DimDiametricData&) {}
    virtual void addDimAngular(const DL_DimensionData&,
                              const DL_DimAngularData&) {}
    virtual void addDimAngular3P(const DL_DimensionData&,
                              const DL_DimAngular3PData&) {}
    virtual void addDimOrdinate(const DL_DimensionData&,
                             const DL_DimOrdinateData&) {}
    virtual void addLeader(const DL_LeaderData&) {}
    virtual void addLeaderVertex(const DL_LeaderVertexData&) {}
    
    virtual void addHatch(const DL_HatchData&) {}

    virtual void addTrace(const DL_TraceData&) {}
    virtual void add3dFace(const DL_3dFaceData&) {}
    virtual void addSolid(const DL_SolidData&) {}
    
    virtual void addImage(const DL_ImageData&) {}
    virtual void linkImage(const DL_ImageDefData&) {}
    virtual void addHatchLoop(const DL_HatchLoopData&) {}
    virtual void addHatchEdge(const DL_HatchEdgeData&) {}

    virtual void addXRecord(const std::string&) {}
    virtual void addXRecordString(int, const std::string&) {}
    virtual void addXRecordReal(int, double) {}
    virtual void addXRecordInt(int, int) {}
    virtual void addXRecordBool(int, bool) {}

    virtual void addXDataApp(const std::string&) {}
    virtual void addXDataString(int, const std::string&) {}
    virtual void addXDataReal(int, double) {}
    virtual void addXDataInt(int, int) {}

    virtual void addDictionary(const DL_DictionaryData&) {}
    virtual void addDictionaryEntry(const DL_DictionaryEntryData&) {}

    virtual void endEntity() {}

    virtual void addComment(const std::string&) {}

    virtual void setVariableVector(const std::string&,  double, double, double, int) {}
    virtual void setVariableString(const std::string&, const std::string&, int) {}
    virtual void setVariableInt(const std::string&, int, int) {}
    virtual void setVariableDouble(const std::string&, double, int) {}
#ifdef DL_COMPAT
    virtual void setVariableVector(const char*,  double, double, double, int) {}
    virtual void setVariableString(const char*, const char*, int) {}
    virtual void setVariableInt(const char*, int, int) {}
    virtual void setVariableDouble(const char*, double, int) {}
    virtual void processCodeValuePair(unsigned int, char*) {}
    virtual void addComment(const char*) {}
    virtual void addMTextChunk(const char*) {}
#endif
    virtual void endSequence() {}
};

#endif
