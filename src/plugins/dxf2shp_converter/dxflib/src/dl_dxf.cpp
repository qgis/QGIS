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

#include "dl_dxf.h"

#include <algorithm>
#include <string>
#include <cstdio>
#include <cassert>
#include <cmath>

#include "dl_attributes.h"
#include "dl_codes.h"
#include "dl_creationadapter.h"
#include "dl_writer_ascii.h"

#include "iostream"

/**
 * Default constructor.
 */
DL_Dxf::DL_Dxf() {
    version = DL_VERSION_2000;

    vertices = nullptr;
    maxVertices = 0;
    vertexIndex = 0;

    knots = nullptr;
    maxKnots = 0;
    knotIndex = 0;

    weights = nullptr;
    weightIndex = 0;

    controlPoints = nullptr;
    maxControlPoints = 0;
    controlPointIndex = 0;

    fitPoints = nullptr;
    maxFitPoints = 0;
    fitPointIndex = 0;

    leaderVertices = nullptr;
    maxLeaderVertices = 0;
    leaderVertexIndex = 0;
}



/**
 * Destructor.
 */
DL_Dxf::~DL_Dxf() {
    if (vertices!=nullptr) {
        delete[] vertices;
    }
    if (knots!=nullptr) {
        delete[] knots;
    }
    if (controlPoints!=nullptr) {
        delete[] controlPoints;
    }
    if (fitPoints!=nullptr) {
        delete[] fitPoints;
    }
    if (weights!=nullptr) {
        delete[] weights;
    }
    if (leaderVertices!=nullptr) {
        delete[] leaderVertices;
    }
}



/**
 * @brief Reads the given file and calls the appropriate functions in
 * the given creation interface for every entity found in the file.
 *
 * @param file Input
 *      Path and name of file to read
 * @param creationInterface
 *      Pointer to the class which takes care of the entities in the file.
 *
 * @retval true If \p file could be opened.
 * @retval false If \p file could not be opened.
 */
bool DL_Dxf::in(const std::string& file, DL_CreationInterface* creationInterface) {
    FILE *fp;
    firstCall = true;
    currentObjectType = DL_UNKNOWN;

    fp = fopen(file.c_str(), "rt");
    if (fp) {
        while (readDxfGroups(fp, creationInterface)) {}
        fclose(fp);
        return true;
    }

    return false;
}



/**
 * Reads a DXF file from an existing stream.
 *
 * @param stream The string stream.
 * @param creationInterface
 *      Pointer to the class which takes care of the entities in the file.
 *
 * @retval true If \p file could be opened.
 * @retval false If \p file could not be opened.
 */
bool DL_Dxf::in(std::stringstream& stream,
                DL_CreationInterface* creationInterface) {
    
    if (stream.good()) {
        firstCall=true;
        currentObjectType = DL_UNKNOWN;
        while (readDxfGroups(stream, creationInterface)) {}
        return true;
    }
    return false;
}



/**
 * @brief Reads a group couplet from a DXF file.  Calls another function
 * to process it.
 *
 * A group couplet consists of two lines that represent a single
 * piece of data.  An integer constant on the first line indicates
 * the type of data.  The value is on the next line.\n
 *
 * This function reads a couplet, determines the type of data, and
 * passes the value to the the appropriate handler function of
 * \p creationInterface.\n
 * 
 * \p fp is advanced so that the next call to \p readDXFGroups() reads
 * the next couplet in the file.
 *
 * @param fp Handle of input file
 * @param creationInterface Handle of class which processes entities
 *      in the file
 *
 * @retval true If EOF not reached.
 * @retval false If EOF reached.
 */
bool DL_Dxf::readDxfGroups(FILE *fp, DL_CreationInterface* creationInterface) {

    static int line = 1;

    // Read one group of the DXF file and strip the lines:
    if (DL_Dxf::getStrippedLine(groupCodeTmp, DL_DXF_MAXLINE, fp) &&
            DL_Dxf::getStrippedLine(groupValue, DL_DXF_MAXLINE, fp) ) {

        groupCode = static_cast<unsigned int>(toInt(groupCodeTmp));

        creationInterface->processCodeValuePair(groupCode, groupValue);
        line+=2;
        processDXFGroup(creationInterface, groupCode, groupValue);
    }

    return !feof(fp);
}



/**
 * Same as above but for stringstreams.
 */
bool DL_Dxf::readDxfGroups(std::stringstream& stream,
                           DL_CreationInterface* creationInterface) {

    static int line = 1;

    // Read one group of the DXF file and chop the lines:
    if (DL_Dxf::getStrippedLine(groupCodeTmp, DL_DXF_MAXLINE, stream) &&
            DL_Dxf::getStrippedLine(groupValue, DL_DXF_MAXLINE, stream) ) {

        groupCode = static_cast<unsigned int>(toInt(groupCodeTmp));

        line+=2;
        processDXFGroup(creationInterface, groupCode, groupValue);
    }
    return !stream.eof();
}



/**
 * @brief Reads line from file & strips whitespace at start and newline 
 * at end.
 *
 * @param s Output\n
 *      Pointer to character array that chopped line will be returned in.
 * @param size Size of \p s.  (Including space for NULL.)
 * @param fp Input\n
 *      Handle of input file.
 *
 * @retval true if line could be read
 * @retval false if \p fp is already at end of file
 *
 * @todo Change function to use safer FreeBSD strl* functions
 * @todo Is it a problem if line is blank (i.e., newline only)?
 *      Then, when function returns, (s==NULL).
 */
bool DL_Dxf::getStrippedLine(std::string& s, unsigned int size, FILE *fp) {
    if (!feof(fp)) {
        // The whole line in the file.  Includes space for NULL.
        char* wholeLine = new char[size];
        // Only the useful part of the line
        char* line;

        line = fgets(wholeLine, size, fp);

        if (line!=nullptr && line[0] != '\0') { // Evaluates to fgets() retval
            // line == wholeLine at this point.
            // Both guaranteed to be NULL terminated.

            // Strip leading whitespace and trailing CR/LF.
            stripWhiteSpace(&line);

            s = line;
            assert(size > s.length());
        }

        delete[] wholeLine; // Done with wholeLine

        return true;
    } else {
        s = "";
        return false;
    }
}



/**
 * Same as above but for stringstreams.
 */
bool DL_Dxf::getStrippedLine(std::string &s, unsigned int size,
                            std::stringstream& stream) {

    if (!stream.eof()) {
        // Only the useful part of the line
        char* line = new char[size+1];
        char* oriLine = line;
        stream.getline(line, size);
        stripWhiteSpace(&line);
        s = line;
        assert(size > s.length());
        delete[] oriLine;
        return true;
    } else {
        s[0] = '\0';
        return false;
    }
}



/**
 * @brief Strips leading whitespace and trailing Carriage Return (CR)
 * and Line Feed (LF) from NULL terminated string.
 *
 * @param s Input and output.
 *      NULL terminates string.
 *
 * @retval true if \p s is non-NULL
 * @retval false if \p s is NULL
 */
bool DL_Dxf::stripWhiteSpace(char** s) {
    // last non-NULL char:
    int lastChar = static_cast<int>( strlen(*s) ) - 1;

    // Is last character CR or LF?
    while ( (lastChar >= 0) &&
            (((*s)[lastChar] == 10) || ((*s)[lastChar] == 13) ||
             ((*s)[lastChar] == ' ' || ((*s)[lastChar] == '\t'))) ) {
        (*s)[lastChar] = '\0';
        lastChar--;
    }

    // Skip whitespace, excluding \n, at beginning of line
    while ((*s)[0]==' ' || (*s)[0]=='\t') {
        ++(*s);
    }
    
    return ((*s) ? true : false);
}



/**
 * Processes a group (pair of group code and value).
 *
 * @param creationInterface Handle to class that creates entities and
 * other CAD data from DXF group codes
 *
 * @param groupCode Constant indicating the data type of the group.
 * @param groupValue The data value.
 *
 * @retval true if done processing current entity and new entity begun
 * @retval false if not done processing current entity
*/
bool DL_Dxf::processDXFGroup(DL_CreationInterface* creationInterface,
                             int groupCode, const std::string& groupValue) {

    //printf("%d\n", groupCode);
    //printf("%s\n", groupValue.c_str());

    // Init values on first call
    if (firstCall) {
        settingValue[0] = '\0';
        firstCall=false;
    }

    // Indicates comment or dxflib version:
    if (groupCode==999) {
        if (!groupValue.empty()) {
            if (groupValue.substr(0, 6)=="dxflib") {
                libVersion = getLibVersion(groupValue.substr(7));
            }
            
            addComment(creationInterface, groupValue);
        }
    }

    // Indicates start of new entity or variable:
    else if (groupCode==0 || groupCode==9) {
        // If new entity is encountered, the last one is complete.
        // Prepare default attributes for next entity:
        std::string layer = getStringValue(8, "0");

        int width;
        // Compatibility with qcad1:
        if (hasValue(39) && !hasValue(370)) {
            width = getIntValue(39, -1);
        }
        // since autocad 2002:
        else if (hasValue(370)) {
            width = getIntValue(370, -1);
        }
        // default to BYLAYER:
        else {
            width = -1;
        }

        int color;
        color = getIntValue(62, 256);
        int color24;
        color24 = getIntValue(420, -1);
        int handle;
        handle = getIntValue(5, -1);

        std::string linetype = getStringValue(6, "BYLAYER");

        attrib = DL_Attributes(layer,                   // layer
                               color,                   // color
                               color24,                 // 24 bit color
                               width,                   // width
                               linetype,                // linetype
                               handle);                 // handle
        attrib.setInPaperSpace( static_cast<bool>(getIntValue(67, 0)));
        attrib.setLinetypeScale(getRealValue(48, 1.0));
        creationInterface->setAttributes(attrib);

        int elevationGroupCode=30;
        if (currentObjectType==DL_ENTITY_LWPOLYLINE ) {
            // see lwpolyline group codes reference
            elevationGroupCode=38;
        }
        else {
            // see polyline group codes reference
            elevationGroupCode=30;
        }

        creationInterface->setExtrusion(getRealValue(210, 0.0),
                                        getRealValue(220, 0.0),
                                        getRealValue(230, 1.0),
                                        getRealValue(elevationGroupCode, 0.0));

        // Add the previously parsed entity via creationInterface
        switch (currentObjectType) {
        case DL_SETTING:
            addSetting(creationInterface);
            break;

        case DL_LAYER:
            addLayer(creationInterface);
            break;

        case DL_LINETYPE:
            addLinetype(creationInterface);
            break;

        case DL_BLOCK:
            addBlock(creationInterface);
            break;

        case DL_ENDBLK:
            endBlock(creationInterface);
            break;

        case DL_STYLE:
            addTextStyle(creationInterface);
            break;

        case DL_ENTITY_POINT:
            addPoint(creationInterface);
            break;

        case DL_ENTITY_LINE:
            addLine(creationInterface);
            break;

        case DL_ENTITY_XLINE:
            addXLine(creationInterface);
            break;

        case DL_ENTITY_RAY:
            addRay(creationInterface);
            break;

        case DL_ENTITY_POLYLINE:
        case DL_ENTITY_LWPOLYLINE:
            addPolyline(creationInterface);
            break;

        case DL_ENTITY_VERTEX:
            addVertex(creationInterface);
            break;

        case DL_ENTITY_SPLINE:
            addSpline(creationInterface);
            break;

        case DL_ENTITY_ARC:
            addArc(creationInterface);
            break;

        case DL_ENTITY_CIRCLE:
            addCircle(creationInterface);
            break;

        case DL_ENTITY_ELLIPSE:
            addEllipse(creationInterface);
            break;

        case DL_ENTITY_INSERT:
            addInsert(creationInterface);
            break;

        case DL_ENTITY_MTEXT:
            addMText(creationInterface);
            break;

        case DL_ENTITY_TEXT:
            addText(creationInterface);
            break;

        case DL_ENTITY_ATTRIB:
            addAttribute(creationInterface);
            break;

        case DL_ENTITY_DIMENSION: {
                int type = (getIntValue(70, 0)&0x07);

                switch (type) {
                case 0:
                    addDimLinear(creationInterface);
                    break;

                case 1:
                    addDimAligned(creationInterface);
                    break;

                case 2:
                    addDimAngular(creationInterface);
                    break;

                case 3:
                    addDimDiametric(creationInterface);
                    break;

                case 4:
                    addDimRadial(creationInterface);
                    break;

                case 5:
                    addDimAngular3P(creationInterface);
                    break;
                
                case 6:
                    addDimOrdinate(creationInterface);
                    break;

                default:
                    break;
                }
            }
            break;

        case DL_ENTITY_LEADER:
            addLeader(creationInterface);
            break;

        case DL_ENTITY_HATCH:
            //addHatch(creationInterface);
            handleHatchData(creationInterface);
            break;

        case DL_ENTITY_IMAGE:
            addImage(creationInterface);
            break;

        case DL_ENTITY_IMAGEDEF:
            addImageDef(creationInterface);
            break;

        case DL_ENTITY_TRACE:
            addTrace(creationInterface);
            break;
        
        case DL_ENTITY_3DFACE:
            add3dFace(creationInterface);
            break;

        case DL_ENTITY_SOLID:
            addSolid(creationInterface);
            break;

        case DL_ENTITY_SEQEND:
            endSequence(creationInterface);
            break;

        default:
            break;
        }

        creationInterface->endSection();

        // reset all values (they are not persistent and only this
        //  way we can set defaults for omitted values)
//        for (int i=0; i<DL_DXF_MAXGROUPCODE; ++i) {
//            values[i][0] = '\0';
//        }
        values.clear();
        settingValue[0] = '\0';
        settingKey = "";
        firstHatchLoop = true;
        //firstHatchEdge = true;
        hatchEdge = DL_HatchEdgeData();
        //xRecordHandle = "";
        xRecordValues = false;

        // Last DXF entity or setting has been handled
        // Now determine what the next entity or setting type is

        int prevEntity = currentObjectType;

        // Read DXF variable:
        if (groupValue[0]=='$') {
            currentObjectType = DL_SETTING;
            settingKey = groupValue;
        }

        // Read Layers:
        else if (groupValue=="LAYER") {
            currentObjectType = DL_LAYER;
        }

        // Read Linetypes:
        else if (groupValue=="LTYPE") {
            currentObjectType = DL_LINETYPE;
        }

        // Read Blocks:
        else if (groupValue=="BLOCK") {
            currentObjectType = DL_BLOCK;
        } else if (groupValue=="ENDBLK") {
            currentObjectType = DL_ENDBLK;
        }

        // Read text styles:
        else if (groupValue=="STYLE") {
            currentObjectType = DL_STYLE;
        }

        // Read entities:
        else if (groupValue=="POINT") {
            currentObjectType = DL_ENTITY_POINT;
        } else if (groupValue=="LINE") {
            currentObjectType = DL_ENTITY_LINE;
        } else if (groupValue=="XLINE") {
            currentObjectType = DL_ENTITY_XLINE;
        } else if (groupValue=="RAY") {
            currentObjectType = DL_ENTITY_RAY;
        } else if (groupValue=="POLYLINE") {
            currentObjectType = DL_ENTITY_POLYLINE;
        } else if (groupValue=="LWPOLYLINE") {
            currentObjectType = DL_ENTITY_LWPOLYLINE;
        } else if (groupValue=="VERTEX") {
            currentObjectType = DL_ENTITY_VERTEX;
        } else if (groupValue=="SPLINE") {
            currentObjectType = DL_ENTITY_SPLINE;
        } else if (groupValue=="ARC") {
            currentObjectType = DL_ENTITY_ARC;
        } else if (groupValue=="ELLIPSE") {
            currentObjectType = DL_ENTITY_ELLIPSE;
        } else if (groupValue=="CIRCLE") {
            currentObjectType = DL_ENTITY_CIRCLE;
        } else if (groupValue=="INSERT") {
            currentObjectType = DL_ENTITY_INSERT;
        } else if (groupValue=="TEXT") {
            currentObjectType = DL_ENTITY_TEXT;
        } else if (groupValue=="MTEXT") {
            currentObjectType = DL_ENTITY_MTEXT;
        } else if (groupValue=="ATTRIB") {
            currentObjectType = DL_ENTITY_ATTRIB;
        } else if (groupValue=="DIMENSION") {
            currentObjectType = DL_ENTITY_DIMENSION;
        } else if (groupValue=="LEADER") {
            currentObjectType = DL_ENTITY_LEADER;
        } else if (groupValue=="HATCH") {
            currentObjectType = DL_ENTITY_HATCH;
        } else if (groupValue=="IMAGE") {
            currentObjectType = DL_ENTITY_IMAGE;
        } else if (groupValue=="IMAGEDEF") {
            currentObjectType = DL_ENTITY_IMAGEDEF;
        } else if (groupValue=="TRACE") {
           currentObjectType = DL_ENTITY_TRACE;
        } else if (groupValue=="SOLID") {
           currentObjectType = DL_ENTITY_SOLID;
        } else if (groupValue=="3DFACE") {
           currentObjectType = DL_ENTITY_3DFACE;
        } else if (groupValue=="SEQEND") {
            currentObjectType = DL_ENTITY_SEQEND;
        } else if (groupValue=="XRECORD") {
            currentObjectType = DL_XRECORD;
        } else if (groupValue=="DICTIONARY") {
            currentObjectType = DL_DICTIONARY;
        } else {
            currentObjectType = DL_UNKNOWN;
        }

        // end of old style POLYLINE entity
        if (prevEntity==DL_ENTITY_VERTEX && currentObjectType!=DL_ENTITY_VERTEX) {
            endEntity(creationInterface);
        }

        // TODO: end of SPLINE entity
        //if (prevEntity==DL_ENTITY_CONTROLPOINT && currentEntity!=DL_ENTITY_CONTROLPOINT) {
        //    endEntity(creationInterface);
        //}

        return true;

    } else {
        // Group code does not indicate start of new entity or setting,
        // so this group must be continuation of data for the current
        // one.
        if (groupCode<DL_DXF_MAXGROUPCODE) {

            bool handled = false;

            switch (currentObjectType) {
            case DL_ENTITY_MTEXT:
                handled = handleMTextData(creationInterface);
                break;

            case DL_ENTITY_LWPOLYLINE:
                handled = handleLWPolylineData(creationInterface);
                break;

            case DL_ENTITY_SPLINE:
                handled = handleSplineData(creationInterface);
                break;

            case DL_ENTITY_LEADER:
                handled = handleLeaderData(creationInterface);
                break;

            case DL_ENTITY_HATCH:
                handled = handleHatchData(creationInterface);
                break;

            case DL_XRECORD:
                handled = handleXRecordData(creationInterface);
                break;

            case DL_DICTIONARY:
                handled = handleDictionaryData(creationInterface);
                break;

            case DL_LINETYPE:
                handled = handleLinetypeData(creationInterface);
                break;

            default:
                break;
            }

            // Always try to handle XData, unless we're in an XData record:
            if (currentObjectType!=DL_XRECORD) {
                handled = handleXData(creationInterface);
            }

            if (!handled) {
                // Normal group / value pair:
                values[groupCode] = groupValue;
            }
        }

        return false;
    }
    return false;
}



/**
 * Adds a comment from the DXF file.
 */
void DL_Dxf::addComment(DL_CreationInterface* creationInterface, const std::string& comment) {
    creationInterface->addComment(comment);
}

void DL_Dxf::addDictionary(DL_CreationInterface* creationInterface) {
    creationInterface->addDictionary(DL_DictionaryData(getStringValue(5, "")));
}

void DL_Dxf::addDictionaryEntry(DL_CreationInterface* creationInterface) {
    creationInterface->addDictionaryEntry(DL_DictionaryEntryData(getStringValue(3, ""), getStringValue(350, "")));
}



/**
 * Adds a variable from the DXF file.
 */
void DL_Dxf::addSetting(DL_CreationInterface* creationInterface) {
    int c = -1;
    std::map<int,std::string>::iterator it = values.begin();
    if (it!=values.end()) {
        c = it->first;
    }
//    for (int i=0; i<=380; ++i) {
//        if (values[i][0]!='\0') {
//            c = i;
//            break;
//        }
//    }

    // string
    if (c>=0 && c<=9) {
        creationInterface->setVariableString(settingKey, values[c], c);
 #ifdef DL_COMPAT
        // backwards compatibility:
        creationInterface->setVariableString(settingKey.c_str(), values[c].c_str(), c);
 #endif
    }
    // vector
    else if (c>=10 && c<=39) {
        if (c==10) {
            creationInterface->setVariableVector(
                settingKey,
                getRealValue(c, 0.0),
                getRealValue(c+10, 0.0),
                getRealValue(c+20, 0.0),
                c);
        }
    }
    // double
    else if (c>=40 && c<=59) {
        creationInterface->setVariableDouble(settingKey, getRealValue(c, 0.0), c);
    }
    // int
    else if (c>=60 && c<=99) {
        creationInterface->setVariableInt(settingKey, getIntValue(c, 0), c);
    }
    // misc
    else if (c>=0) {
        creationInterface->setVariableString(settingKey, getStringValue(c, ""), c);
    }
}



/**
 * Adds a layer that was read from the file via the creation interface.
 */
void DL_Dxf::addLayer(DL_CreationInterface* creationInterface) {
    // correct some invalid attributes for layers:
    attrib = creationInterface->getAttributes();
    if (attrib.getColor()==256 || attrib.getColor()==0) {
        attrib.setColor(7);
    }
    if (attrib.getWidth()<0) {
        attrib.setWidth(1);
    }

    std::string linetype = attrib.getLinetype();
    std::transform(linetype.begin(), linetype.end(), linetype.begin(), ::toupper);
    if (linetype=="BYLAYER" || linetype=="BYBLOCK") {
        attrib.setLinetype("CONTINUOUS");
    }

    // add layer
    std::string name = getStringValue(2, "");
    if (name.length()==0) {
        return;
    }

    creationInterface->addLayer(DL_LayerData(name, getIntValue(70, 0)));
}

/**
 * Adds a linetype that was read from the file via the creation interface.
 */
void DL_Dxf::addLinetype(DL_CreationInterface* creationInterface) {
    std::string name = getStringValue(2, "");
    if (name.length()==0) {
        return;
    }
    int numDashes = getIntValue(73, 0);
    //double dashes[numDashes];

    DL_LinetypeData d(
        // name:
        name,
        // description:
        getStringValue(3, ""),
        // flags
        getIntValue(70, 0),
        // number of dashes:
        numDashes,
        // pattern length:
        getRealValue(40, 0.0)
        // pattern:
        //dashes
    );

    if (name != "By Layer" && name != "By Block" && name != "BYLAYER" && name != "BYBLOCK") {
        creationInterface->addLinetype(d);
    }
}

/**
 * Handles all dashes in linetype pattern.
 */
bool DL_Dxf::handleLinetypeData(DL_CreationInterface* creationInterface) {
    if (groupCode == 49) {
        creationInterface->addLinetypeDash(toReal(groupValue));
        return true;
    }

    return false;
}


/**
 * Adds a block that was read from the file via the creation interface.
 */
void DL_Dxf::addBlock(DL_CreationInterface* creationInterface) {
    std::string name = getStringValue(2, "");
    if (name.length()==0) {
        return;
    }

    DL_BlockData d(
        // Name:
        name,
        // flags:
        getIntValue(70, 0),
        // base point:
        getRealValue(10, 0.0),
        getRealValue(20, 0.0),
        getRealValue(30, 0.0));

    creationInterface->addBlock(d);
}



/**
 * Ends a block that was read from the file via the creation interface.
 */
void DL_Dxf::endBlock(DL_CreationInterface* creationInterface) {
    creationInterface->endBlock();
}



void DL_Dxf::addTextStyle(DL_CreationInterface* creationInterface) {
    std::string name = getStringValue(2, "");
    if (name.length()==0) {
        return;
    }

    DL_StyleData d(
        // name:
        name,
        // flags
        getIntValue(70, 0),
        // fixed text height:
        getRealValue(40, 0.0),
        // width factor:
        getRealValue(41, 0.0),
        // oblique angle:
        getRealValue(50, 0.0),
        // text generation flags:
        getIntValue(71, 0),
        // last height used:
        getRealValue(42, 0.0),
        // primart font file:
        getStringValue(3, ""),
        // big font file:
        getStringValue(4, "")
        );
    creationInterface->addTextStyle(d);
}


/**
 * Adds a point entity that was read from the file via the creation interface.
 */
void DL_Dxf::addPoint(DL_CreationInterface* creationInterface) {
    DL_PointData d(getRealValue(10, 0.0),
                   getRealValue(20, 0.0),
                   getRealValue(30, 0.0));
    creationInterface->addPoint(d);
}



/**
 * Adds a line entity that was read from the file via the creation interface.
 */
void DL_Dxf::addLine(DL_CreationInterface* creationInterface) {
    DL_LineData d(getRealValue(10, 0.0),
                  getRealValue(20, 0.0),
                  getRealValue(30, 0.0),
                  getRealValue(11, 0.0),
                  getRealValue(21, 0.0),
                  getRealValue(31, 0.0));

    creationInterface->addLine(d);
}

/**
 * Adds an xline entity that was read from the file via the creation interface.
 */
void DL_Dxf::addXLine(DL_CreationInterface* creationInterface) {
    DL_XLineData d(getRealValue(10, 0.0),
                   getRealValue(20, 0.0),
                   getRealValue(30, 0.0),
                   getRealValue(11, 0.0),
                   getRealValue(21, 0.0),
                   getRealValue(31, 0.0));

    creationInterface->addXLine(d);
}

/**
 * Adds a ray entity that was read from the file via the creation interface.
 */
void DL_Dxf::addRay(DL_CreationInterface* creationInterface) {
    DL_RayData d(getRealValue(10, 0.0),
                 getRealValue(20, 0.0),
                 getRealValue(30, 0.0),
                 getRealValue(11, 0.0),
                 getRealValue(21, 0.0),
                 getRealValue(31, 0.0));

    creationInterface->addRay(d);
}



/**
 * Adds a polyline entity that was read from the file via the creation interface.
 */
void DL_Dxf::addPolyline(DL_CreationInterface* creationInterface) {
    DL_PolylineData pd(maxVertices, getIntValue(71, 0), getIntValue(72, 0), getIntValue(70, 0));
    creationInterface->addPolyline(pd);

    maxVertices = std::min(maxVertices, vertexIndex+1);

    if (currentObjectType==DL_ENTITY_LWPOLYLINE) {
        for (int i=0; i<maxVertices; i++) {
            DL_VertexData d(vertices[i*4],
                            vertices[i*4+1],
                            vertices[i*4+2],
                            vertices[i*4+3]);

            creationInterface->addVertex(d);
        }
        creationInterface->endEntity();
    }
}



/**
 * Adds a polyline vertex entity that was read from the file
 * via the creation interface.
 */
void DL_Dxf::addVertex(DL_CreationInterface* creationInterface) {

    // vertex defines a face of the mesh if its vertex flags group has the
    // 128 bit set but not the 64 bit. 10, 20, 30 are irrelevant and set to
    // 0 in this case
    if ((getIntValue(70, 0)&128) && !(getIntValue(70, 0)&64)) {
        return;
    }

    DL_VertexData d(getRealValue(10, 0.0),
                    getRealValue(20, 0.0),
                    getRealValue(30, 0.0),
                    getRealValue(42, 0.0));

    creationInterface->addVertex(d);
}


/**
 * Adds a spline entity that was read from the file via the creation interface.
 */
void DL_Dxf::addSpline(DL_CreationInterface* creationInterface) {
    DL_SplineData sd(getIntValue(71, 3), 
                     maxKnots, 
                     maxControlPoints, 
                     maxFitPoints,
                     getIntValue(70, 4));

    sd.tangentStartX = getRealValue(12, 0.0);
    sd.tangentStartY = getRealValue(22, 0.0);
    sd.tangentStartZ = getRealValue(32, 0.0);
    sd.tangentEndX = getRealValue(13, 0.0);
    sd.tangentEndY = getRealValue(23, 0.0);
    sd.tangentEndZ = getRealValue(33, 0.0);

    creationInterface->addSpline(sd);

    int i;
    for (i=0; i<maxControlPoints; i++) {
        DL_ControlPointData d(controlPoints[i*3],
                              controlPoints[i*3+1],
                              controlPoints[i*3+2], 
                              weights[i]);

        creationInterface->addControlPoint(d);
    }
    for (i=0; i<maxFitPoints; i++) {
        DL_FitPointData d(fitPoints[i*3],
                              fitPoints[i*3+1],
                              fitPoints[i*3+2]);

        creationInterface->addFitPoint(d);
    }
    for (i=0; i<maxKnots; i++) {
        DL_KnotData k(knots[i]);

        creationInterface->addKnot(k);
    }
    creationInterface->endEntity();
}



/**
 * Adds an arc entity that was read from the file via the creation interface.
 */
void DL_Dxf::addArc(DL_CreationInterface* creationInterface) {
    DL_ArcData d(getRealValue(10, 0.0),
                 getRealValue(20, 0.0),
                 getRealValue(30, 0.0),
                 getRealValue(40, 0.0),
                 getRealValue(50, 0.0),
                 getRealValue(51, 0.0));

    creationInterface->addArc(d);
}



/**
 * Adds a circle entity that was read from the file via the creation interface.
 */
void DL_Dxf::addCircle(DL_CreationInterface* creationInterface) {
    DL_CircleData d(getRealValue(10, 0.0),
                    getRealValue(20, 0.0),
                    getRealValue(30, 0.0),
                    getRealValue(40, 0.0));

    creationInterface->addCircle(d);
}



/**
 * Adds an ellipse entity that was read from the file via the creation interface.
 */
void DL_Dxf::addEllipse(DL_CreationInterface* creationInterface) {
    DL_EllipseData d(getRealValue(10, 0.0),
                     getRealValue(20, 0.0),
                     getRealValue(30, 0.0),
                     getRealValue(11, 0.0),
                     getRealValue(21, 0.0),
                     getRealValue(31, 0.0),
                     getRealValue(40, 1.0),
                     getRealValue(41, 0.0),
                     getRealValue(42, 2*M_PI));

    creationInterface->addEllipse(d);
}



/**
 * Adds an insert entity that was read from the file via the creation interface.
 */
void DL_Dxf::addInsert(DL_CreationInterface* creationInterface) {
    //printf("addInsert\n");
    //printf("code 50: %s\n", values[50]);
    //printf("code 50 length: %d\n", strlen(values[50]));
    //printf("code 50:\n");
    //getRealValue(50, 0.0);

    std::string name = getStringValue(2, "");
    if (name.length()==0) {
        return;
    }

    DL_InsertData d(name,
                    // insertion point
                    getRealValue(10, 0.0),
                    getRealValue(20, 0.0),
                    getRealValue(30, 0.0),
                    // scale:
                    getRealValue(41, 1.0),
                    getRealValue(42, 1.0),
                    getRealValue(43, 1.0),
                    // angle (deg):
                    getRealValue(50, 0.0),
                    // cols / rows:
                    getIntValue(70, 1),
                    getIntValue(71, 1),
                    // spacing:
                    getRealValue(44, 0.0),
                    getRealValue(45, 0.0));

    creationInterface->addInsert(d);
}



/**
 * Adds a trace entity (4 edge closed polyline) that was read from the file via the creation interface.
 *
 * @author AHM
 */
void DL_Dxf::addTrace(DL_CreationInterface* creationInterface) {
    DL_TraceData td;
    
    for (int k = 0; k < 4; k++) {
       td.x[k] = getRealValue(10 + k, 0.0);
       td.y[k] = getRealValue(20 + k, 0.0);
       td.z[k] = getRealValue(30 + k, 0.0);
    }
    creationInterface->addTrace(td);
}



/**
 * Adds a 3dface entity that was read from the file via the creation interface.
 */
void DL_Dxf::add3dFace(DL_CreationInterface* creationInterface) {
    DL_3dFaceData td;
    
    for (int k = 0; k < 4; k++) {
       td.x[k] = getRealValue(10 + k, 0.0);
       td.y[k] = getRealValue(20 + k, 0.0);
       td.z[k] = getRealValue(30 + k, 0.0);
    }
    creationInterface->add3dFace(td);
}



/**
 * Adds a solid entity (filled trace) that was read from the file via the creation interface.
 * 
 * @author AHM
 */
void DL_Dxf::addSolid(DL_CreationInterface* creationInterface) {
    DL_SolidData sd;
    
    for (int k = 0; k < 4; k++) {
       sd.x[k] = getRealValue(10 + k, 0.0);
       sd.y[k] = getRealValue(20 + k, 0.0);
       sd.z[k] = getRealValue(30 + k, 0.0);
    }
    creationInterface->addSolid(sd);
}


/**
 * Adds an MText entity that was read from the file via the creation interface.
 */
void DL_Dxf::addMText(DL_CreationInterface* creationInterface) {
    double angle = 0.0;

    if (hasValue(50)) {
        if (libVersion<=0x02000200) {
            // wrong but compatible with dxflib <=2.0.2.0:
            angle = getRealValue(50, 0.0);
        } else {
            angle = (getRealValue(50, 0.0)*2*M_PI)/360.0;
        }
    } else if (hasValue(11) && hasValue(21)) {
        double x = getRealValue(11, 0.0);
        double y = getRealValue(21, 0.0);

        if (fabs(x)<1.0e-6) {
            if (y>0.0) {
                angle = M_PI/2.0;
            } else {
                angle = M_PI/2.0*3.0;
            }
        } else {
            angle = atan(y/x);
        }
    }

    DL_MTextData d(
        // insertion point
        getRealValue(10, 0.0),
        getRealValue(20, 0.0),
        getRealValue(30, 0.0),
        // X direction vector
        getRealValue(11, 0.0),
        getRealValue(21, 0.0),
        getRealValue(31, 0.0),
        // height
        getRealValue(40, 2.5),
        // width
        getRealValue(41, 0.0),
        // attachment point
        getIntValue(71, 1),
        // drawing direction
        getIntValue(72, 1),
        // line spacing style
        getIntValue(73, 1),
        // line spacing factor
        getRealValue(44, 1.0),
        // text
        getStringValue(1, ""),
        // style
        getStringValue(7, ""),
        // angle
        angle);
    creationInterface->addMText(d);
}

/**
 * Handles all XRecord data.
 */
bool DL_Dxf::handleXRecordData(DL_CreationInterface* creationInterface) {
    if (groupCode==105) {
        return false;
    }

    if (groupCode==5) {
        creationInterface->addXRecord(groupValue);
        return true;
    }

    if (groupCode==280) {
        xRecordValues = true;
        return true;
    }

    if (!xRecordValues) {
        return false;
    }

    // string:
    if (groupCode<=9 ||
        groupCode==100 || groupCode==102 || groupCode==105 ||
        (groupCode>=300 && groupCode<=369) ||
        (groupCode>=1000 && groupCode<=1009)) {

        creationInterface->addXRecordString(groupCode, groupValue);
        return true;
    }

    // int:
    else if ((groupCode>=60 && groupCode<=99) || (groupCode>=160 && groupCode<=179) || (groupCode>=270 && groupCode<=289)) {
        creationInterface->addXRecordInt(groupCode, toInt(groupValue));
        return true;
    }

    // bool:
    else if (groupCode>=290 && groupCode<=299) {
        creationInterface->addXRecordBool(groupCode, toBool(groupValue));
        return true;
    }

    // double:
    else if ((groupCode>=10 && groupCode<=59) || (groupCode>=110 && groupCode<=149) || (groupCode>=210 && groupCode<=239)) {
        creationInterface->addXRecordReal(groupCode, toReal(groupValue));
        return true;
    }

    return false;
}

/**
 * Handles all dictionary data.
 */
bool DL_Dxf::handleDictionaryData(DL_CreationInterface* creationInterface) {
    if (groupCode==3) {
        return true;
    }

    if (groupCode==5) {
        creationInterface->addDictionary(DL_DictionaryData(groupValue));
        return true;
    }

    if (groupCode==350) {
        creationInterface->addDictionaryEntry(DL_DictionaryEntryData(getStringValue(3, ""), groupValue));
        return true;
    }
    return false;
}



/**
 * Handles XData for all object types.
 */
bool DL_Dxf::handleXData(DL_CreationInterface* creationInterface) {
    if (groupCode==1001) {
        creationInterface->addXDataApp(groupValue);
        return true;
    }
    else if (groupCode>=1000 && groupCode<=1009) {
        creationInterface->addXDataString(groupCode, groupValue);
        return true;
    }
    else if (groupCode>=1010 && groupCode<=1059) {
        creationInterface->addXDataReal(groupCode, toReal(groupValue));
        return true;
    }
    else if (groupCode>=1060 && groupCode<=1070) {
        creationInterface->addXDataInt(groupCode, toInt(groupValue));
        return true;
    }
    else if (groupCode==1071) {
        creationInterface->addXDataInt(groupCode, toInt(groupValue));
        return true;
    }

    return false;
}

/**
 * Handles additional MText data.
 */
bool DL_Dxf::handleMTextData(DL_CreationInterface* creationInterface) {
    // Special handling of text chunks for MTEXT entities:
    if (groupCode==3) {
        creationInterface->addMTextChunk(groupValue);
        return true;
    }

    return false;
}



/**
 * Handles additional polyline data.
 */
bool DL_Dxf::handleLWPolylineData(DL_CreationInterface* /*creationInterface*/) {
    // Allocate LWPolyline vertices (group code 90):
    if (groupCode==90) {
        maxVertices = toInt(groupValue);
        if (maxVertices>0) {
            if (vertices!=nullptr) {
                delete[] vertices;
            }
            vertices = new double[4*maxVertices];
            for (int i=0; i<maxVertices; ++i) {
                vertices[i*4] = 0.0;
                vertices[i*4+1] = 0.0;
                vertices[i*4+2] = 0.0;
                vertices[i*4+3] = 0.0;
            }
        }
        vertexIndex=-1;
        return true;
    }

    // Process LWPolylines vertices (group codes 10/20/30/42):
    else if (groupCode==10 || groupCode==20 ||
             groupCode==30 || groupCode==42) {

        if (vertexIndex<maxVertices-1 && groupCode==10) {
            vertexIndex++;
        }

        if (groupCode<=30) {
            if (vertexIndex>=0 && vertexIndex<maxVertices) {
                vertices[4*vertexIndex + (groupCode/10-1)] = toReal(groupValue);
            }
        } else if (groupCode==42 && vertexIndex<maxVertices) {
            vertices[4*vertexIndex + 3] = toReal(groupValue);
        }
        return true;
    }
    return false;
}



/**
 * Handles additional spline data.
 */
bool DL_Dxf::handleSplineData(DL_CreationInterface* /*creationInterface*/) {
    // Allocate Spline knots (group code 72):
    if (groupCode==72) {
        maxKnots = toInt(groupValue);
        if (maxKnots>0) {
            if (knots!=nullptr) {
                delete[] knots;
            }
            knots = new double[maxKnots];
            for (int i=0; i<maxKnots; ++i) {
                knots[i] = 0.0;
            }
        }
        knotIndex=-1;
        return true;
    }

    // Allocate Spline control points / weights (group code 73):
    else if (groupCode==73) {
        maxControlPoints = toInt(groupValue);
        if (maxControlPoints>0) {
            if (controlPoints!=nullptr) {
                delete[] controlPoints;
            }
            if (weights!=nullptr) {
                delete[] weights;
            }
            controlPoints = new double[3*maxControlPoints];
            weights = new double[maxControlPoints];
            for (int i=0; i<maxControlPoints; ++i) {
                controlPoints[i*3] = 0.0;
                controlPoints[i*3+1] = 0.0;
                controlPoints[i*3+2] = 0.0;
                weights[i] = 1.0;
            }
        }
        controlPointIndex=-1;
        weightIndex=-1;
        return true;
    }

    // Allocate Spline fit points (group code 74):
    else if (groupCode==74) {
        maxFitPoints = toInt(groupValue);
        if (maxFitPoints>0) {
            if (fitPoints!=nullptr) {
                delete[] fitPoints;
            }
            fitPoints = new double[3*maxFitPoints];
            for (int i=0; i<maxFitPoints; ++i) {
                fitPoints[i*3] = 0.0;
                fitPoints[i*3+1] = 0.0;
                fitPoints[i*3+2] = 0.0;
            }
        }
        fitPointIndex=-1;
        return true;
    }

    // Process spline knot vertices (group code 40):
    else if (groupCode==40) {
        if (knotIndex<maxKnots-1) {
            knotIndex++;
            knots[knotIndex] = toReal(groupValue);
        }
        return true;
    }

    // Process spline control points (group codes 10/20/30):
    else if (groupCode==10 || groupCode==20 ||
             groupCode==30) {

        if (controlPointIndex<maxControlPoints-1 && groupCode==10) {
            controlPointIndex++;
        }

        if (controlPointIndex>=0 && controlPointIndex<maxControlPoints) {
            controlPoints[3*controlPointIndex + (groupCode/10-1)] = toReal(groupValue);
        }
        return true;
    }

    // Process spline fit points (group codes 11/21/31):
    else if (groupCode==11 || groupCode==21 || groupCode==31) {
        if (fitPointIndex<maxFitPoints-1 && groupCode==11) {
            fitPointIndex++;
        }

        if (fitPointIndex>=0 && fitPointIndex<maxFitPoints) {
            fitPoints[3*fitPointIndex + ((groupCode-1)/10-1)] = toReal(groupValue);
        }
        return true;
    }

    // Process spline weights (group code 41)
    else if (groupCode==41) {

        if (weightIndex<maxControlPoints-1) {
            weightIndex++;
        }

        if (weightIndex>=0 && weightIndex<maxControlPoints) {
            weights[weightIndex] = toReal(groupValue);
        }
        return true;
    }
    return false;
}



/**
 * Handles additional leader data.
 */
bool DL_Dxf::handleLeaderData(DL_CreationInterface* /*creationInterface*/) {
    // Allocate Leader vertices (group code 76):
    if (groupCode==76) {
        maxLeaderVertices = toInt(groupValue);
        if (maxLeaderVertices>0) {
            if (leaderVertices!=nullptr) {
                delete[] leaderVertices;
            }
            leaderVertices = new double[3*maxLeaderVertices];
            for (int i=0; i<maxLeaderVertices; ++i) {
                leaderVertices[i*3] = 0.0;
                leaderVertices[i*3+1] = 0.0;
                leaderVertices[i*3+2] = 0.0;
            }
        }
        leaderVertexIndex=-1;
        return true;
    }

    // Process Leader vertices (group codes 10/20/30):
    else if (groupCode==10 || groupCode==20 || groupCode==30) {

        if (leaderVertexIndex<maxLeaderVertices-1 && groupCode==10) {
            leaderVertexIndex++;
        }

        if (groupCode<=30) {
            if (leaderVertexIndex>=0 &&
                    leaderVertexIndex<maxLeaderVertices) {
                leaderVertices[3*leaderVertexIndex + (groupCode/10-1)]
                = toReal(groupValue);
            }
        }
        return true;
    }

    return false;
}




/**
 * Adds an text entity that was read from the file via the creation interface.
 */
void DL_Dxf::addText(DL_CreationInterface* creationInterface) {
    DL_TextData d(
        // insertion point
        getRealValue(10, 0.0),
        getRealValue(20, 0.0),
        getRealValue(30, 0.0),
        // alignment point
        getRealValue(11, 0.0),
        getRealValue(21, 0.0),
        getRealValue(31, 0.0),
        // height
        getRealValue(40, 2.5),
        // x scale
        getRealValue(41, 1.0),
        // generation flags
        getIntValue(71, 0),
        // h just
        getIntValue(72, 0),
        // v just
        getIntValue(73, 0),
        // text
        getStringValue(1, ""),
        // style
        getStringValue(7, ""),
        // angle
        (getRealValue(50, 0.0)*2*M_PI)/360.0);

    creationInterface->addText(d);
}



/**
 * Adds an attrib entity that was read from the file via the creation interface.
 * @todo add attrib instead of normal text
 */
void DL_Dxf::addAttribute(DL_CreationInterface* creationInterface) {
    DL_AttributeData d(
        // insertion point
        getRealValue(10, 0.0),
        getRealValue(20, 0.0),
        getRealValue(30, 0.0),
        // alignment point
        getRealValue(11, 0.0),
        getRealValue(21, 0.0),
        getRealValue(31, 0.0),
        // height
        getRealValue(40, 2.5),
        // x scale
        getRealValue(41, 1.0),
        // generation flags
        getIntValue(71, 0),
        // h just
        getIntValue(72, 0),
        // v just
        getIntValue(74, 0),
        // tag
        getStringValue(2, ""),
        // text
        getStringValue(1, ""),
        // style
        getStringValue(7, ""),
        // angle
        (getRealValue(50, 0.0)*2*M_PI)/360.0);

    creationInterface->addAttribute(d);
}



/**
 * @return dimension data from current values.
 */
DL_DimensionData DL_Dxf::getDimData() {
    // generic dimension data:
    return DL_DimensionData(
               // def point
               getRealValue(10, 0.0),
               getRealValue(20, 0.0),
               getRealValue(30, 0.0),
               // text middle point
               getRealValue(11, 0.0),
               getRealValue(21, 0.0),
               getRealValue(31, 0.0),
               // type
               getIntValue(70, 0),
               // attachment point
               getIntValue(71, 5),
               // line sp. style
               getIntValue(72, 1),
               // line sp. factor
               getRealValue(41, 1.0),
               // text
               getStringValue(1, ""),
               // style
               getStringValue(3, ""),
               // angle
               getRealValue(53, 0.0));
}



/**
 * Adds a linear dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimLinear(DL_CreationInterface* creationInterface) {
    DL_DimensionData d = getDimData();

    // horizontal / vertical / rotated dimension:
    DL_DimLinearData dl(
        // definition point 1
        getRealValue(13, 0.0),
        getRealValue(23, 0.0),
        getRealValue(33, 0.0),
        // definition point 2
        getRealValue(14, 0.0),
        getRealValue(24, 0.0),
        getRealValue(34, 0.0),
        // angle
        getRealValue(50, 0.0),
        // oblique
        getRealValue(52, 0.0));
    creationInterface->addDimLinear(d, dl);
}



/**
 * Adds an aligned dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimAligned(DL_CreationInterface* creationInterface) {
    DL_DimensionData d = getDimData();

    // aligned dimension:
    DL_DimAlignedData da(
        // extension point 1
        getRealValue(13, 0.0),
        getRealValue(23, 0.0),
        getRealValue(33, 0.0),
        // extension point 2
        getRealValue(14, 0.0),
        getRealValue(24, 0.0),
        getRealValue(34, 0.0));
    creationInterface->addDimAlign(d, da);
}



/**
 * Adds a radial dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimRadial(DL_CreationInterface* creationInterface) {
    DL_DimensionData d = getDimData();

    DL_DimRadialData dr(
        // definition point
        getRealValue(15, 0.0),
        getRealValue(25, 0.0),
        getRealValue(35, 0.0),
        // leader length:
        getRealValue(40, 0.0));
    creationInterface->addDimRadial(d, dr);
}



/**
 * Adds a diametric dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimDiametric(DL_CreationInterface* creationInterface) {
    DL_DimensionData d = getDimData();

    // diametric dimension:
    DL_DimDiametricData dr(
        // definition point
        getRealValue(15, 0.0),
        getRealValue(25, 0.0),
        getRealValue(35, 0.0),
        // leader length:
        getRealValue(40, 0.0));
    creationInterface->addDimDiametric(d, dr);
}



/**
 * Adds an angular dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimAngular(DL_CreationInterface* creationInterface) {
    DL_DimensionData d = getDimData();

    // angular dimension:
    DL_DimAngularData da(
        // definition point 1
        getRealValue(13, 0.0),
        getRealValue(23, 0.0),
        getRealValue(33, 0.0),
        // definition point 2
        getRealValue(14, 0.0),
        getRealValue(24, 0.0),
        getRealValue(34, 0.0),
        // definition point 3
        getRealValue(15, 0.0),
        getRealValue(25, 0.0),
        getRealValue(35, 0.0),
        // definition point 4
        getRealValue(16, 0.0),
        getRealValue(26, 0.0),
        getRealValue(36, 0.0));
    creationInterface->addDimAngular(d, da);
}


/**
 * Adds an angular dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimAngular3P(DL_CreationInterface* creationInterface) {
    DL_DimensionData d = getDimData();

    // angular dimension (3P):
    DL_DimAngular3PData da(
        // definition point 1
        getRealValue(13, 0.0),
        getRealValue(23, 0.0),
        getRealValue(33, 0.0),
        // definition point 2
        getRealValue(14, 0.0),
        getRealValue(24, 0.0),
        getRealValue(34, 0.0),
        // definition point 3
        getRealValue(15, 0.0),
        getRealValue(25, 0.0),
        getRealValue(35, 0.0));
    creationInterface->addDimAngular3P(d, da);
}



/**
 * Adds an ordinate dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimOrdinate(DL_CreationInterface* creationInterface) {
    DL_DimensionData d = getDimData();

    // ordinate dimension:
    DL_DimOrdinateData dl(
        // definition point 1
        getRealValue(13, 0.0),
        getRealValue(23, 0.0),
        getRealValue(33, 0.0),
        // definition point 2
        getRealValue(14, 0.0),
        getRealValue(24, 0.0),
        getRealValue(34, 0.0),
        (getIntValue(70, 0)&64)==64         // true: X-type, false: Y-type
    );
    creationInterface->addDimOrdinate(d, dl);
}



/**
 * Adds a leader entity that was read from the file via the creation interface.
 */
void DL_Dxf::addLeader(DL_CreationInterface* creationInterface) {
    // leader (arrow)
    DL_LeaderData le(
        // arrow head flag
        getIntValue(71, 1),
        // leader path type
        getIntValue(72, 0),
        // Leader creation flag
        getIntValue(73, 3),
        // Hookline direction flag
        getIntValue(74, 1),
        // Hookline flag
        getIntValue(75, 0),
        // Text annotation height
        getRealValue(40, 1.0),
        // Text annotation width
        getRealValue(41, 1.0),
        // Number of vertices in leader
        getIntValue(76, 0)
    );
    creationInterface->addLeader(le);

    for (int i=0; i<maxLeaderVertices; i++) {
        DL_LeaderVertexData d(leaderVertices[i*3],
                              leaderVertices[i*3+1],
                              leaderVertices[i*3+2]);

        creationInterface->addLeaderVertex(d);
    }
    creationInterface->endEntity();
}

/**
 * Adds a hatch entity that was read from the file via the creation interface.
 */
void DL_Dxf::addHatch(DL_CreationInterface* creationInterface) {
    DL_HatchData hd(getIntValue(91, 1),
                    getIntValue(70, 0),
                    getRealValue(41, 1.0),
                    getRealValue(52, 0.0),
                    getStringValue(2, ""));

    creationInterface->addHatch(hd);

    for (size_t i=0; i<hatchEdges.size(); i++) {
        creationInterface->addHatchLoop(DL_HatchLoopData(hatchEdges[i].size()));
        for (unsigned int k=0; k<hatchEdges[i].size(); k++) {
            creationInterface->addHatchEdge(DL_HatchEdgeData(hatchEdges[i][k]));
        }
    }

    creationInterface->endEntity();
}

void DL_Dxf::addHatchLoop() {
    addHatchEdge();
    hatchEdges.push_back(std::vector<DL_HatchEdgeData>());
}

void DL_Dxf::addHatchEdge() {
    if (hatchEdge.defined) {
        if (hatchEdges.size()>0) {
            hatchEdges.back().push_back(hatchEdge);
        }
        hatchEdge = DL_HatchEdgeData();
    }
}

/**
 * Handles all hatch data.
 */
bool DL_Dxf::handleHatchData(DL_CreationInterface* creationInterface) {
    // New polyline loop, group code 92
    // or new loop with individual edges, group code 93
    if (groupCode==92 || groupCode==93) {
        if (firstHatchLoop) {
            hatchEdges.clear();
            firstHatchLoop = false;
        }
        if (groupCode==92 && (toInt(groupValue)&2)==2) {
            addHatchLoop();
        }
        if (groupCode==93) {
            addHatchLoop();
        }
        return true;
    }

    // New hatch edge or new section / entity: add last hatch edge:
    if (groupCode==72 || groupCode==0 || groupCode==78 || groupCode==98) {
        // polyline boundaries use code 72 for bulge flag:
        if (groupCode!=72 || (getIntValue(92, 0)&2)==0) {
            addHatchEdge();
        }

        if (groupCode==0 /*|| groupCode==78*/) {
            addHatch(creationInterface);
        }
        else {
            hatchEdge.type = toInt(groupValue);
        }
        return true;
    }

    // polyline boundary:
    if ((getIntValue(92, 0)&2)==2) {
        switch (groupCode) {
        case 10:
            hatchEdge.type = 0;
            hatchEdge.vertices.push_back(std::vector<double>());
            hatchEdge.vertices.back().push_back(toReal(groupValue));
            return true;
        case 20:
            if (!hatchEdge.vertices.empty()) {
                hatchEdge.vertices.back().push_back(toReal(groupValue));
                hatchEdge.defined = true;
            }
            return true;
        case 42:
            if (!hatchEdge.vertices.empty()) {
                hatchEdge.vertices.back().push_back(toReal(groupValue));
                hatchEdge.defined = true;
            }
            return true;
        }
    }
    else {
        // Line edge:
        if (hatchEdge.type==1) {
            switch (groupCode) {
            case 10:
                hatchEdge.x1 = toReal(groupValue);
                return true;
            case 20:
                hatchEdge.y1 = toReal(groupValue);
                return true;
            case 11:
                hatchEdge.x2 = toReal(groupValue);
                return true;
            case 21:
                hatchEdge.y2 = toReal(groupValue);
                hatchEdge.defined = true;
                return true;
            }
        }

        // Arc edge:
        if (hatchEdge.type==2) {
            switch(groupCode) {
            case 10:
                hatchEdge.cx = toReal(groupValue);
                return true;
            case 20:
                hatchEdge.cy = toReal(groupValue);
                return true;
            case 40:
                hatchEdge.radius = toReal(groupValue);
                return true;
            case 50:
                hatchEdge.angle1 = toReal(groupValue)/360.0*2*M_PI;
                return true;
            case 51:
                hatchEdge.angle2 = toReal(groupValue)/360.0*2*M_PI;
                return true;
            case 73:
                hatchEdge.ccw = static_cast<bool>(toInt(groupValue));
                hatchEdge.defined = true;
                return true;
            }
        }

        // Ellipse arc edge:
        if (hatchEdge.type==3) {
            switch (groupCode) {
            case 10:
                hatchEdge.cx = toReal(groupValue);
                return true;
            case 20:
                hatchEdge.cy = toReal(groupValue);
                return true;
            case 11:
                hatchEdge.mx = toReal(groupValue);
                return true;
            case 21:
                hatchEdge.my = toReal(groupValue);
                return true;
            case 40:
                hatchEdge.ratio = toReal(groupValue);
                return true;
            case 50:
                hatchEdge.angle1 = toReal(groupValue)/360.0*2*M_PI;
                return true;
            case 51:
                hatchEdge.angle2 = toReal(groupValue)/360.0*2*M_PI;
                return true;
            case 73:
                hatchEdge.ccw = static_cast<bool>(toInt(groupValue));
                hatchEdge.defined = true;
                return true;
            }
        }

        // Spline edge:
        if (hatchEdge.type==4) {
            switch (groupCode) {
            case 94:
                hatchEdge.degree = toInt(groupValue);
                return true;
            case 73:
                hatchEdge.rational = toBool(groupValue);
                return true;
            case 74:
                hatchEdge.periodic = toBool(groupValue);
                return true;
            case 95:
                hatchEdge.nKnots = toInt(groupValue);
                return true;
            case 96:
                hatchEdge.nControl = toInt(groupValue);
                return true;
            case 97:
                hatchEdge.nFit = toInt(groupValue);
                return true;
            case 40:
                if (hatchEdge.knots.size() < hatchEdge.nKnots) {
                    hatchEdge.knots.push_back(toReal(groupValue));
                }
                return true;
            case 10:
                if (hatchEdge.controlPoints.size() < hatchEdge.nControl) {
                    std::vector<double> v;
                    v.push_back(toReal(groupValue));
                    hatchEdge.controlPoints.push_back(v);
                }
                return true;
            case 20:
                if (!hatchEdge.controlPoints.empty() && hatchEdge.controlPoints.back().size()==1) {
                    hatchEdge.controlPoints.back().push_back(toReal(groupValue));
                }
                hatchEdge.defined = true;
                return true;
            case 42:
                if (hatchEdge.weights.size() < hatchEdge.nControl) {
                    hatchEdge.weights.push_back(toReal(groupValue));
                }
                return true;
            case 11:
                if (hatchEdge.fitPoints.size() < hatchEdge.nFit) {
                    std::vector<double> v;
                    v.push_back(toReal(groupValue));
                    hatchEdge.fitPoints.push_back(v);
                }
                return true;
            case 21:
                if (!hatchEdge.fitPoints.empty() && hatchEdge.fitPoints.back().size()==1) {
                    hatchEdge.fitPoints.back().push_back(toReal(groupValue));
                }
                hatchEdge.defined = true;
                return true;
            case 12:
                hatchEdge.startTangentX = toReal(groupValue);
                return true;
            case 22:
                hatchEdge.startTangentY = toReal(groupValue);
                return true;
            case 13:
                hatchEdge.endTangentX = toReal(groupValue);
                return true;
            case 23:
                hatchEdge.endTangentY = toReal(groupValue);
                return true;
            }
        }
    }

    return false;
}


/**
 * Adds an image entity that was read from the file via the creation interface.
 */
void DL_Dxf::addImage(DL_CreationInterface* creationInterface) {
    DL_ImageData id(// pass ref insead of name we don't have yet
        getStringValue(340, ""),
        // ins point:
        getRealValue(10, 0.0),
        getRealValue(20, 0.0),
        getRealValue(30, 0.0),
        // u vector:
        getRealValue(11, 1.0),
        getRealValue(21, 0.0),
        getRealValue(31, 0.0),
        // v vector:
        getRealValue(12, 0.0),
        getRealValue(22, 1.0),
        getRealValue(32, 0.0),
        // image size (pixel):
        getIntValue(13, 1),
        getIntValue(23, 1),
        // brightness, contrast, fade
        getIntValue(281, 50),
        getIntValue(282, 50),
        getIntValue(283, 0));

    creationInterface->addImage(id);
    creationInterface->endEntity();
    currentObjectType = DL_UNKNOWN;
}



/**
 * Adds an image definition that was read from the file via the creation interface.
 */
void DL_Dxf::addImageDef(DL_CreationInterface* creationInterface) {
    DL_ImageDefData id(// handle
        getStringValue(5, ""),
        getStringValue(1, ""));

    creationInterface->linkImage(id);
    creationInterface->endEntity();
    currentObjectType = DL_UNKNOWN;
}



/**
 * Ends some special entities like hatches or old style polylines.
 */
void DL_Dxf::endEntity(DL_CreationInterface* creationInterface) {
    creationInterface->endEntity();
}


/**
 * Ends a sequence and notifies the creation interface.
 */
void DL_Dxf::endSequence(DL_CreationInterface* creationInterface) {
    creationInterface->endSequence();
}


/**
 * Converts the given string into an int.
 * ok is set to false if there was an error.
 */
//int DL_Dxf::stringToInt(const char* s, bool* ok) {
//    if (ok!=NULL) {
//        // check string:
//        *ok = true;
//        int i=0;
//        bool dot = false;
//        do {
//            if (s[i]=='\0') {
//                break;
//            } else if (s[i]=='.') {
//                if (dot==true) {
//                    //std::cerr << "two dots\n";
//                    *ok = false;
//                } else {
//                    dot = true;
//                }
//            } else if (s[i]<'0' || s[i]>'9') {
//                //std::cerr << "NaN: '" << s[i] << "'\n";
//                *ok = false;
//            }
//            i++;
//        } while(s[i]!='\0' && *ok==true);
//    }

//    return atoi(s);
//}


/**
 * @brief Opens the given file for writing and returns a pointer
 * to the dxf writer. This pointer needs to be passed on to other
 * writing functions.
 *
 * @param file Full path of the file to open.
 *
 * @return Pointer to an ascii dxf writer object.
 */
DL_WriterA* DL_Dxf::out(const char* file, DL_Codes::version version) {
    char* f = new char[strlen(file)+1];
    strcpy(f, file);
    this->version = version;

    DL_WriterA* dw = new DL_WriterA(f, version);
    if (dw->openFailed()) {
        delete dw;
        delete[] f;
        return nullptr;
    } else {
        delete[] f;
        return dw;
    }
}



/**
 * @brief Writes a DXF header to the file currently opened 
 * by the given DXF writer object.
 */
void DL_Dxf::writeHeader(DL_WriterA& dw) {
    dw.comment("dxflib " DL_VERSION);
    dw.sectionHeader();

    dw.dxfString(9, "$ACADVER");
    switch (version) {
    case DL_Codes::AC1009:
        dw.dxfString(1, "AC1009");
        break;
    case DL_Codes::AC1012:
        dw.dxfString(1, "AC1012");
        break;
    case DL_Codes::AC1014:
        dw.dxfString(1, "AC1014");
        break;
    case DL_Codes::AC1015:
        dw.dxfString(1, "AC1015");
        break;
    }

    // Newer version require that (otherwise a*cad crashes..)
    if (version==DL_VERSION_2000) {
        dw.dxfString(9, "$HANDSEED");
        dw.dxfHex(5, 0xFFFF);
    }

    // commented out: more variables can be added after that by caller:
    //dw.sectionEnd();
}




/**
 * Writes a point entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writePoint(DL_WriterA& dw,
                        const DL_PointData& data,
                        const DL_Attributes& attrib) {
    dw.entity("POINT");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbPoint");
    }
    dw.entityAttributes(attrib);
    dw.coord(DL_POINT_COORD_CODE, data.x, data.y, data.z);
}



/**
 * Writes a line entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeLine(DL_WriterA& dw,
                       const DL_LineData& data,
                       const DL_Attributes& attrib) {
    dw.entity("LINE");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbLine");
    }
    dw.entityAttributes(attrib);
    dw.coord(DL_LINE_START_CODE, data.x1, data.y1, data.z1);
    dw.coord(DL_LINE_END_CODE, data.x2, data.y2, data.z2);
}



/**
 * Writes an x line entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeXLine(DL_WriterA& dw,
                       const DL_XLineData& data,
                       const DL_Attributes& attrib) {
    dw.entity("XLINE");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbLine");
    }
    dw.entityAttributes(attrib);
    dw.coord(DL_LINE_START_CODE, data.bx, data.by, data.bz);
    dw.coord(DL_LINE_END_CODE, data.dx, data.dy, data.dz);
}



/**
 * Writes a ray entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeRay(DL_WriterA& dw,
                        const DL_RayData& data,
                        const DL_Attributes& attrib) {
    dw.entity("RAY");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbLine");
    }
    dw.entityAttributes(attrib);
    dw.coord(DL_LINE_START_CODE, data.bx, data.by, data.bz);
    dw.coord(DL_LINE_END_CODE, data.dx, data.dy, data.dz);
}



/**
 * Writes a polyline entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 * @see writeVertex
 */
void DL_Dxf::writePolyline(DL_WriterA& dw,
                           const DL_PolylineData& data,
                           const DL_Attributes& attrib) {
    if (version==DL_VERSION_2000) {
        dw.entity("LWPOLYLINE");
        dw.entityAttributes(attrib);
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbPolyline");
        dw.dxfInt(90, static_cast<int>(data.number));
        dw.dxfInt(70, data.flags);
    } else {
        dw.entity("POLYLINE");
        dw.entityAttributes(attrib);
        polylineLayer = attrib.getLayer();
        dw.dxfInt(66, 1);
        dw.dxfInt(70, data.flags);
        dw.coord(DL_VERTEX_COORD_CODE, 0.0, 0.0, 0.0);
    }
}



/**
 * Writes a single vertex of a polyline to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeVertex(DL_WriterA& dw,
                         const DL_VertexData& data) {


    if (version==DL_VERSION_2000) {
        dw.dxfReal(10, data.x);
        dw.dxfReal(20, data.y);
        dw.dxfReal(30, data.z);
        if (fabs(data.bulge)>1.0e-10) {
            dw.dxfReal(42, data.bulge);
        }
    } else {
        dw.entity("VERTEX");
        //dw.entityAttributes(attrib);
        dw.dxfString(8, polylineLayer);
        dw.coord(DL_VERTEX_COORD_CODE, data.x, data.y, data.z);
        if (fabs(data.bulge)>1.0e-10) {
            dw.dxfReal(42, data.bulge);
        }
    }
}

    
    
/**
 * Writes the polyline end. Only needed for DXF R12.
 */
void DL_Dxf::writePolylineEnd(DL_WriterA& dw) {
    if (version==DL_VERSION_2000) {
    } else {
        dw.entity("SEQEND");
    }
}


/**
 * Writes a spline entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 * @see writeControlPoint
 */
void DL_Dxf::writeSpline(DL_WriterA& dw,
                         const DL_SplineData& data,
                         const DL_Attributes& attrib) {

    dw.entity("SPLINE");
    dw.entityAttributes(attrib);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbSpline");
    }
    dw.dxfInt(70, data.flags);
    dw.dxfInt(71, data.degree);
    dw.dxfInt(72, data.nKnots);            // number of knots
    dw.dxfInt(73, data.nControl);          // number of control points
    dw.dxfInt(74, data.nFit);              // number of fit points
}



/**
 * Writes a single control point of a spline to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeControlPoint(DL_WriterA& dw,
                               const DL_ControlPointData& data) {

    dw.dxfReal(10, data.x);
    dw.dxfReal(20, data.y);
    dw.dxfReal(30, data.z);
}



/**
 * Writes a single fit point of a spline to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeFitPoint(DL_WriterA& dw,
                           const DL_FitPointData& data) {

    dw.dxfReal(11, data.x);
    dw.dxfReal(21, data.y);
    dw.dxfReal(31, data.z);
}



/**
 * Writes a single knot of a spline to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeKnot(DL_WriterA& dw,
                       const DL_KnotData& data) {

    dw.dxfReal(40, data.k);
}



/**
 * Writes a circle entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeCircle(DL_WriterA& dw,
                         const DL_CircleData& data,
                         const DL_Attributes& attrib) {
    dw.entity("CIRCLE");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbCircle");
    }
    dw.entityAttributes(attrib);
    dw.coord(10, data.cx, data.cy, data.cz);
    dw.dxfReal(40, data.radius);
}



/**
 * Writes an arc entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeArc(DL_WriterA& dw,
                      const DL_ArcData& data,
                      const DL_Attributes& attrib) {
    dw.entity("ARC");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
    }
    dw.entityAttributes(attrib);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbCircle");
    }
    dw.coord(10, data.cx, data.cy, data.cz);
    dw.dxfReal(40, data.radius);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbArc");
    }
    dw.dxfReal(50, data.angle1);
    dw.dxfReal(51, data.angle2);
}



/**
 * Writes an ellipse entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeEllipse(DL_WriterA& dw,
                          const DL_EllipseData& data,
                          const DL_Attributes& attrib) {

    if (version>DL_VERSION_R12) {
        dw.entity("ELLIPSE");
        if (version==DL_VERSION_2000) {
            dw.dxfString(100, "AcDbEntity");
            dw.dxfString(100, "AcDbEllipse");
        }
        dw.entityAttributes(attrib);
        dw.coord(10, data.cx, data.cy, data.cz);
        dw.coord(11, data.mx, data.my, data.mz);
        dw.dxfReal(40, data.ratio);
        dw.dxfReal(41, data.angle1);
        dw.dxfReal(42, data.angle2);
    }
}
    
    

/**
 * Writes a solid entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeSolid(DL_WriterA& dw,
                   const DL_SolidData& data,
                   const DL_Attributes& attrib) {
    dw.entity("SOLID");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbTrace");
    }
    dw.entityAttributes(attrib);
    dw.coord(10, data.x[0], data.y[0], data.z[0]);
    dw.coord(11, data.x[1], data.y[1], data.z[1]);
    dw.coord(12, data.x[2], data.y[2], data.z[2]);
    dw.coord(13, data.x[3], data.y[3], data.z[3]);
    dw.dxfReal(39, data.thickness);
}

/**
 * Writes a trace entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeTrace(DL_WriterA& dw,
                        const DL_TraceData& data,
                        const DL_Attributes& attrib) {
    dw.entity("TRACE");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbTrace");
    }
    dw.entityAttributes(attrib);
    dw.coord(10, data.x[0], data.y[0], data.z[0]);
    dw.coord(11, data.x[1], data.y[1], data.z[1]);
    dw.coord(12, data.x[2], data.y[2], data.z[2]);
    dw.coord(13, data.x[3], data.y[3], data.z[3]);
    dw.dxfReal(39, data.thickness);
}



/**
 * Writes a 3d face entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::write3dFace(DL_WriterA& dw,
                   const DL_3dFaceData& data,
                   const DL_Attributes& attrib) {
    dw.entity("3DFACE");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbFace");
    }
    dw.entityAttributes(attrib);
    dw.coord(10, data.x[0], data.y[0], data.z[0]);
    dw.coord(11, data.x[1], data.y[1], data.z[1]);
    dw.coord(12, data.x[2], data.y[2], data.z[2]);
    dw.coord(13, data.x[3], data.y[3], data.z[3]);
}



/**
 * Writes an insert to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeInsert(DL_WriterA& dw,
                         const DL_InsertData& data,
                         const DL_Attributes& attrib) {

    if (data.name.empty()) {
        std::cerr << "DL_Dxf::writeInsert: "
        << "Block name must not be empty\n";
        return;
    }

    dw.entity("INSERT");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbBlockReference");
    }
    dw.entityAttributes(attrib);
    dw.dxfString(2, data.name);
    dw.dxfReal(10, data.ipx);
    dw.dxfReal(20, data.ipy);
    dw.dxfReal(30, data.ipz);
    if (data.sx!=1.0 || data.sy!=1.0) {
        dw.dxfReal(41, data.sx);
        dw.dxfReal(42, data.sy);
        dw.dxfReal(43, 1.0);
    }
    if (data.angle!=0.0) {
        dw.dxfReal(50, data.angle);
    }
    if (data.cols!=1 || data.rows!=1) {
        dw.dxfInt(70, data.cols);
        dw.dxfInt(71, data.rows);
    }
    if (data.colSp!=0.0 || data.rowSp!=0.0) {
        dw.dxfReal(44, data.colSp);
        dw.dxfReal(45, data.rowSp);
    }

}



/**
 * Writes a multi text entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeMText(DL_WriterA& dw,
                        const DL_MTextData& data,
                        const DL_Attributes& attrib) {

    dw.entity("MTEXT");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbMText");
    }
    dw.entityAttributes(attrib);
    dw.dxfReal(10, data.ipx);
    dw.dxfReal(20, data.ipy);
    dw.dxfReal(30, data.ipz);
    dw.dxfReal(40, data.height);
    dw.dxfReal(41, data.width);

    dw.dxfInt(71, data.attachmentPoint);
    dw.dxfInt(72, data.drawingDirection);

    // Creare text chunks of 250 characters each:
    int length = static_cast<int>( data.text.length() );
    char chunk[251];
    int i;
    for (i=250; i<length; i+=250) {
        strncpy(chunk, &data.text.c_str()[i-250], 250);
        chunk[250]='\0';
        dw.dxfString(3, chunk);
    }
    strncpy(chunk, &data.text.c_str()[i-250], 250);
    chunk[250]='\0';
    dw.dxfString(1, chunk);

    dw.dxfString(7, data.style);

    // since dxflib 2.0.2.1: degrees not rad (error in autodesk dxf doc)
    dw.dxfReal(50, data.angle/(2.0*M_PI)*360.0);

    dw.dxfInt(73, data.lineSpacingStyle);
    dw.dxfReal(44, data.lineSpacingFactor);
}



/**
 * Writes a text entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeText(DL_WriterA& dw,
                       const DL_TextData& data,
                       const DL_Attributes& attrib) {

    dw.entity("TEXT");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbText");
    }
    dw.entityAttributes(attrib);
    dw.dxfReal(10, data.ipx);
    dw.dxfReal(20, data.ipy);
    dw.dxfReal(30, data.ipz);
    dw.dxfReal(40, data.height);
    dw.dxfString(1, data.text);
    dw.dxfReal(50, data.angle/(2*M_PI)*360.0);
    dw.dxfReal(41, data.xScaleFactor);
    dw.dxfString(7, data.style);

    dw.dxfInt(71, data.textGenerationFlags);
    dw.dxfInt(72, data.hJustification);

    dw.dxfReal(11, data.apx);
    dw.dxfReal(21, data.apy);
    dw.dxfReal(31, data.apz);

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbText");
    }

    dw.dxfInt(73, data.vJustification);
}

void DL_Dxf::writeAttribute(DL_WriterA& dw,
                   const DL_AttributeData& data,
                   const DL_Attributes& attrib) {

    dw.entity("ATTRIB");
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbText");
    }
    dw.entityAttributes(attrib);
    dw.dxfReal(10, data.ipx);
    dw.dxfReal(20, data.ipy);
    dw.dxfReal(30, data.ipz);
    dw.dxfReal(40, data.height);
    dw.dxfString(1, data.text);
    dw.dxfReal(50, data.angle/(2*M_PI)*360.0);
    dw.dxfReal(41, data.xScaleFactor);
    dw.dxfString(7, data.style);

    dw.dxfInt(71, data.textGenerationFlags);
    dw.dxfInt(72, data.hJustification);

    dw.dxfReal(11, data.apx);
    dw.dxfReal(21, data.apy);
    dw.dxfReal(31, data.apz);

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbAttribute");
    }

    dw.dxfString(2, data.tag);
    dw.dxfInt(74, data.vJustification);
}

void DL_Dxf::writeDimStyleOverrides(DL_WriterA& dw,
                             const DL_DimensionData& data) {

    if (version==DL_VERSION_2000) {
        dw.dxfString(1001, "ACAD");
        dw.dxfString(1000, "DSTYLE");
        dw.dxfString(1002, "{");
        dw.dxfInt(1070, 144);
        dw.dxfInt(1040, data.linearFactor);
        dw.dxfString(1002, "}");
    }
}


/**
 * Writes an aligned dimension entity to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific aligned dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimAligned(DL_WriterA& dw,
                             const DL_DimensionData& data,
                             const DL_DimAlignedData& edata,
                             const DL_Attributes& attrib) {

    dw.entity("DIMENSION");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
    }
    dw.entityAttributes(attrib);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbDimension");
    }

    dw.dxfReal(10, data.dpx);
    dw.dxfReal(20, data.dpy);
    dw.dxfReal(30, data.dpz);

    dw.dxfReal(11, data.mpx);
    dw.dxfReal(21, data.mpy);
    dw.dxfReal(31, 0.0);

    dw.dxfInt(70, data.type);
    if (version>DL_VERSION_R12) {
        dw.dxfInt(71, data.attachmentPoint);
        dw.dxfInt(72, data.lineSpacingStyle); // opt
        dw.dxfReal(41, data.lineSpacingFactor); // opt
    }

    dw.dxfReal(42, data.angle);

    dw.dxfString(1, data.text);   // opt
    //dw.dxfString(3, data.style);
    dw.dxfString(3, "Standard");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbAlignedDimension");
    }

    dw.dxfReal(13, edata.epx1);
    dw.dxfReal(23, edata.epy1);
    dw.dxfReal(33, 0.0);

    dw.dxfReal(14, edata.epx2);
    dw.dxfReal(24, edata.epy2);
    dw.dxfReal(34, 0.0);

    writeDimStyleOverrides(dw, data);
}



/**
 * Writes a linear dimension entity to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific linear dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimLinear(DL_WriterA& dw,
                            const DL_DimensionData& data,
                            const DL_DimLinearData& edata,
                            const DL_Attributes& attrib) {

    dw.entity("DIMENSION");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
    }
    dw.entityAttributes(attrib);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbDimension");
    }

    dw.dxfReal(10, data.dpx);
    dw.dxfReal(20, data.dpy);
    dw.dxfReal(30, data.dpz);

    dw.dxfReal(11, data.mpx);
    dw.dxfReal(21, data.mpy);
    dw.dxfReal(31, 0.0);

    dw.dxfInt(70, data.type);
    if (version>DL_VERSION_R12) {
        dw.dxfInt(71, data.attachmentPoint);
        dw.dxfInt(72, data.lineSpacingStyle); // opt
        dw.dxfReal(41, data.lineSpacingFactor); // opt
    }

    dw.dxfReal(42, data.angle);

    dw.dxfString(1, data.text);   // opt
    //dw.dxfString(3, data.style);
    dw.dxfString(3, "Standard");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbAlignedDimension");
    }

    dw.dxfReal(13, edata.dpx1);
    dw.dxfReal(23, edata.dpy1);
    dw.dxfReal(33, 0.0);

    dw.dxfReal(14, edata.dpx2);
    dw.dxfReal(24, edata.dpy2);
    dw.dxfReal(34, 0.0);

    dw.dxfReal(50, edata.angle/(2.0*M_PI)*360.0);

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbRotatedDimension");
    }

    writeDimStyleOverrides(dw, data);
}



/**
 * Writes a radial dimension entity to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific radial dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimRadial(DL_WriterA& dw,
                            const DL_DimensionData& data,
                            const DL_DimRadialData& edata,
                            const DL_Attributes& attrib) {

    dw.entity("DIMENSION");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
    }
    dw.entityAttributes(attrib);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbDimension");
    }

    dw.dxfReal(10, data.dpx);
    dw.dxfReal(20, data.dpy);
    dw.dxfReal(30, data.dpz);

    dw.dxfReal(11, data.mpx);
    dw.dxfReal(21, data.mpy);
    dw.dxfReal(31, 0.0);

    dw.dxfInt(70, data.type);
    if (version>DL_VERSION_R12) {
        dw.dxfInt(71, data.attachmentPoint);
        dw.dxfInt(72, data.lineSpacingStyle); // opt
        dw.dxfReal(41, data.lineSpacingFactor); // opt
    }

    dw.dxfReal(42, data.angle);

    dw.dxfString(1, data.text);   // opt
    //dw.dxfString(3, data.style);
    dw.dxfString(3, "Standard");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbRadialDimension");
    }

    dw.dxfReal(15, edata.dpx);
    dw.dxfReal(25, edata.dpy);
    dw.dxfReal(35, 0.0);

    dw.dxfReal(40, edata.leader);

    writeDimStyleOverrides(dw, data);
}



/**
 * Writes a diametric dimension entity to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific diametric dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimDiametric(DL_WriterA& dw,
                               const DL_DimensionData& data,
                               const DL_DimDiametricData& edata,
                               const DL_Attributes& attrib) {

    dw.entity("DIMENSION");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
    }
    dw.entityAttributes(attrib);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbDimension");
    }

    dw.dxfReal(10, data.dpx);
    dw.dxfReal(20, data.dpy);
    dw.dxfReal(30, data.dpz);

    dw.dxfReal(11, data.mpx);
    dw.dxfReal(21, data.mpy);
    dw.dxfReal(31, 0.0);

    dw.dxfInt(70, data.type);
    if (version>DL_VERSION_R12) {
        dw.dxfInt(71, data.attachmentPoint);
        dw.dxfInt(72, data.lineSpacingStyle); // opt
        dw.dxfReal(41, data.lineSpacingFactor); // opt
    }

    dw.dxfReal(42, data.angle);

    dw.dxfString(1, data.text);   // opt
    //dw.dxfString(3, data.style);
    dw.dxfString(3, "Standard");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbDiametricDimension");
    }

    dw.dxfReal(15, edata.dpx);
    dw.dxfReal(25, edata.dpy);
    dw.dxfReal(35, 0.0);

    dw.dxfReal(40, edata.leader);

    writeDimStyleOverrides(dw, data);
}



/**
 * Writes an angular dimension entity to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific angular dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimAngular(DL_WriterA& dw,
                             const DL_DimensionData& data,
                             const DL_DimAngularData& edata,
                             const DL_Attributes& attrib) {

    dw.entity("DIMENSION");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
    }
    dw.entityAttributes(attrib);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbDimension");
    }

    dw.dxfReal(10, data.dpx);
    dw.dxfReal(20, data.dpy);
    dw.dxfReal(30, data.dpz);

    dw.dxfReal(11, data.mpx);
    dw.dxfReal(21, data.mpy);
    dw.dxfReal(31, 0.0);

    dw.dxfInt(70, data.type);
    if (version>DL_VERSION_R12) {
        dw.dxfInt(71, data.attachmentPoint);
        dw.dxfInt(72, data.lineSpacingStyle); // opt
        dw.dxfReal(41, data.lineSpacingFactor); // opt
    }

    dw.dxfReal(42, data.angle);

    dw.dxfString(1, data.text);   // opt
    //dw.dxfString(3, data.style);
    dw.dxfString(3, "Standard");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDb2LineAngularDimension");
    }

    dw.dxfReal(13, edata.dpx1);
    dw.dxfReal(23, edata.dpy1);
    dw.dxfReal(33, 0.0);

    dw.dxfReal(14, edata.dpx2);
    dw.dxfReal(24, edata.dpy2);
    dw.dxfReal(34, 0.0);

    dw.dxfReal(15, edata.dpx3);
    dw.dxfReal(25, edata.dpy3);
    dw.dxfReal(35, 0.0);

    dw.dxfReal(16, edata.dpx4);
    dw.dxfReal(26, edata.dpy4);
    dw.dxfReal(36, 0.0);
}



/**
 * Writes an angular dimension entity (3 points version) to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific angular dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimAngular3P(DL_WriterA& dw,
                               const DL_DimensionData& data,
                               const DL_DimAngular3PData& edata,
                               const DL_Attributes& attrib) {

    dw.entity("DIMENSION");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
    }
    dw.entityAttributes(attrib);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbDimension");
    }

    dw.dxfReal(10, data.dpx);
    dw.dxfReal(20, data.dpy);
    dw.dxfReal(30, data.dpz);

    dw.dxfReal(11, data.mpx);
    dw.dxfReal(21, data.mpy);
    dw.dxfReal(31, 0.0);

    dw.dxfInt(70, data.type);
    if (version>DL_VERSION_R12) {
        dw.dxfInt(71, data.attachmentPoint);
        dw.dxfInt(72, data.lineSpacingStyle); // opt
        dw.dxfReal(41, data.lineSpacingFactor); // opt
    }

    dw.dxfReal(42, data.angle);

    dw.dxfString(1, data.text);   // opt
    //dw.dxfString(3, data.style);
    dw.dxfString(3, "Standard");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDb3PointAngularDimension");
    }

    dw.dxfReal(13, edata.dpx1);
    dw.dxfReal(23, edata.dpy1);
    dw.dxfReal(33, 0.0);

    dw.dxfReal(14, edata.dpx2);
    dw.dxfReal(24, edata.dpy2);
    dw.dxfReal(34, 0.0);

    dw.dxfReal(15, edata.dpx3);
    dw.dxfReal(25, edata.dpy3);
    dw.dxfReal(35, 0.0);
}




/**
 * Writes an ordinate dimension entity to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific ordinate dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimOrdinate(DL_WriterA& dw,
                             const DL_DimensionData& data,
                             const DL_DimOrdinateData& edata,
                             const DL_Attributes& attrib) {

    dw.entity("DIMENSION");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
    }
    dw.entityAttributes(attrib);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbDimension");
    }

    dw.dxfReal(10, data.dpx);
    dw.dxfReal(20, data.dpy);
    dw.dxfReal(30, data.dpz);

    dw.dxfReal(11, data.mpx);
    dw.dxfReal(21, data.mpy);
    dw.dxfReal(31, 0.0);

    int type = data.type;
    if (edata.xtype) {
        type|=0x40;
    }

    dw.dxfInt(70, type);
    if (version>DL_VERSION_R12) {
        dw.dxfInt(71, data.attachmentPoint);
        dw.dxfInt(72, data.lineSpacingStyle); // opt
        dw.dxfReal(41, data.lineSpacingFactor); // opt
    }

    dw.dxfString(1, data.text);   // opt
    //dw.dxfString(3, data.style);
    dw.dxfString(3, "Standard");

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbOrdinateDimension");
    }

    dw.dxfReal(13, edata.dpx1);
    dw.dxfReal(23, edata.dpy1);
    dw.dxfReal(33, 0.0);

    dw.dxfReal(14, edata.dpx2);
    dw.dxfReal(24, edata.dpy2);
    dw.dxfReal(34, 0.0);
}



/**
 * Writes a leader entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 * @see writeVertex
 */
void DL_Dxf::writeLeader(DL_WriterA& dw,
                         const DL_LeaderData& data,
                         const DL_Attributes& attrib) {
    if (version>DL_VERSION_R12) {
        dw.entity("LEADER");
        dw.entityAttributes(attrib);
        if (version==DL_VERSION_2000) {
            dw.dxfString(100, "AcDbEntity");
            dw.dxfString(100, "AcDbLeader");
        }
        dw.dxfString(3, "Standard");
        dw.dxfInt(71, data.arrowHeadFlag);
        dw.dxfInt(72, data.leaderPathType);
        dw.dxfInt(73, data.leaderCreationFlag);
        dw.dxfInt(74, data.hooklineDirectionFlag);
        dw.dxfInt(75, data.hooklineFlag);
        dw.dxfReal(40, data.textAnnotationHeight);
        dw.dxfReal(41, data.textAnnotationWidth);
        dw.dxfInt(76, data.number);
    }
}



/**
 * Writes a single vertex of a leader to the file.
 *
 * @param dw DXF writer
 * @param data Entity data
 */
void DL_Dxf::writeLeaderVertex(DL_WriterA& dw,
                               const DL_LeaderVertexData& data) {
    if (version>DL_VERSION_R12) {
        dw.dxfReal(10, data.x);
        dw.dxfReal(20, data.y);
    }
}



/**
 * Writes the beginning of a hatch entity to the file.
 * This must be followed by one or more writeHatchLoop()
 * calls and a writeHatch2() call.
 *
 * @param dw DXF writer
 * @param data Entity data.
 * @param attrib Attributes
 */
void DL_Dxf::writeHatch1(DL_WriterA& dw,
                         const DL_HatchData& data,
                         const DL_Attributes& attrib) {

    dw.entity("HATCH");
    dw.entityAttributes(attrib);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbHatch");
    }
    dw.dxfReal(10, 0.0);             // elevation
    dw.dxfReal(20, 0.0);
    dw.dxfReal(30, 0.0);
    dw.dxfReal(210, 0.0);             // extrusion dir.
    dw.dxfReal(220, 0.0);
    dw.dxfReal(230, 1.0);
    if (data.solid==false) {
        dw.dxfString(2, data.pattern);
    } else {
        dw.dxfString(2, "SOLID");
    }
    dw.dxfInt(70, static_cast<int>(data.solid));
    dw.dxfInt(71, 0);                // non-associative
    dw.dxfInt(91, data.numLoops);
}



/**
 * Writes the end of a hatch entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data.
 * @param attrib Attributes
 */
void DL_Dxf::writeHatch2(DL_WriterA& dw,
                         const DL_HatchData& data,
                         const DL_Attributes& /*attrib*/) {

    dw.dxfInt(75, 0);                // odd parity
    dw.dxfInt(76, 1);                // pattern type
    if (data.solid==false) {
        dw.dxfReal(52, data.angle);
        dw.dxfReal(41, data.scale);
        dw.dxfInt(77, 0);            // not double
        //dw.dxfInt(78, 0);
        dw.dxfInt(78, 1);
        dw.dxfReal(53, 45.0);
        dw.dxfReal(43, 0.0);
        dw.dxfReal(44, 0.0);
        dw.dxfReal(45, -0.0883883476483184);
        dw.dxfReal(46, 0.0883883476483185);
        dw.dxfInt(79, 0);
    }
    dw.dxfInt(98, 0);

    if (version==DL_VERSION_2000) {
        dw.dxfString(1001, "ACAD");
        dw.dxfReal(1010, data.originX);
        dw.dxfReal(1020, data.originY);
        dw.dxfInt(1030, 0.0);
    }
}



/**
 * Writes the beginning of a hatch loop to the file. This
 * must happen after writing the beginning of a hatch entity.
 *
 * @param dw DXF writer
 * @param data Entity data.
 * @param attrib Attributes
 */
void DL_Dxf::writeHatchLoop1(DL_WriterA& dw,
                             const DL_HatchLoopData& data) {

    dw.dxfInt(92, 1);
    dw.dxfInt(93, data.numEdges);
    //dw.dxfInt(97, 0);
}



/**
 * Writes the end of a hatch loop to the file.
 *
 * @param dw DXF writer
 * @param data Entity data.
 * @param attrib Attributes
 */
void DL_Dxf::writeHatchLoop2(DL_WriterA& dw,
                             const DL_HatchLoopData& /*data*/) {

    dw.dxfInt(97, 0);
}


/**
 * Writes the beginning of a hatch entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data.
 * @param attrib Attributes
 */
void DL_Dxf::writeHatchEdge(DL_WriterA& dw,
                            const DL_HatchEdgeData& data) {

    if (data.type<1 || data.type>4) {
        printf("WARNING: unsupported hatch edge type: %d", data.type);
    }

    dw.dxfInt(72, data.type);

    switch (data.type) {
    // line:
    case 1:
        dw.dxfReal(10, data.x1);
        dw.dxfReal(20, data.y1);
        dw.dxfReal(11, data.x2);
        dw.dxfReal(21, data.y2);
        break;

    // arc:
    case 2:
        dw.dxfReal(10, data.cx);
        dw.dxfReal(20, data.cy);
        dw.dxfReal(40, data.radius);
        dw.dxfReal(50, data.angle1/(2*M_PI)*360.0);
        dw.dxfReal(51, data.angle2/(2*M_PI)*360.0);
        dw.dxfInt(73, static_cast<int>(data.ccw));
        break;

    // ellipse arc:
    case 3:
        dw.dxfReal(10, data.cx);
        dw.dxfReal(20, data.cy);
        dw.dxfReal(11, data.mx);
        dw.dxfReal(21, data.my);
        dw.dxfReal(40, data.ratio);
        dw.dxfReal(50, data.angle1/(2*M_PI)*360.0);
        dw.dxfReal(51, data.angle2/(2*M_PI)*360.0);
        dw.dxfInt(73, static_cast<int>(data.ccw));
        break;

    // spline:
    case 4:
        dw.dxfInt(94, data.degree);
        dw.dxfBool(73, data.rational);
        dw.dxfBool(74, data.periodic);
        dw.dxfInt(95, data.nKnots);
        dw.dxfInt(96, data.nControl);
        for (unsigned int i=0; i<data.knots.size(); i++) {
            dw.dxfReal(40, data.knots[i]);
        }
        for (unsigned int i=0; i<data.controlPoints.size(); i++) {
            dw.dxfReal(10, data.controlPoints[i][0]);
            dw.dxfReal(20, data.controlPoints[i][1]);
        }
        for (unsigned int i=0; i<data.weights.size(); i++) {
            dw.dxfReal(42, data.weights[i]);
        }
        if (data.nFit>0) {
            dw.dxfInt(97, data.nFit);
            for (unsigned int i=0; i<data.fitPoints.size(); i++) {
                dw.dxfReal(11, data.fitPoints[i][0]);
                dw.dxfReal(21, data.fitPoints[i][1]);
            }
        }
        if (fabs(data.startTangentX)>1.0e-4 || fabs(data.startTangentY)>1.0e-4) {
            dw.dxfReal(12, data.startTangentX);
            dw.dxfReal(22, data.startTangentY);
        }
        if (fabs(data.endTangentX)>1.0e-4 || fabs(data.endTangentY)>1.0e-4) {
            dw.dxfReal(13, data.endTangentX);
            dw.dxfReal(23, data.endTangentY);
        }
        break;

    default:
        break;
    }
}



/**
 * Writes an image entity.
 *
 * @return IMAGEDEF handle. Needed for the IMAGEDEF counterpart.
 */
int DL_Dxf::writeImage(DL_WriterA& dw,
                       const DL_ImageData& data,
                       const DL_Attributes& attrib) {

    /*if (data.file.empty()) {
        std::cerr << "DL_Dxf::writeImage: "
        << "Image file must not be empty\n";
        return;
}*/

    dw.entity("IMAGE");

    dw.entityAttributes(attrib);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbEntity");
        dw.dxfString(100, "AcDbRasterImage");
        dw.dxfInt(90, 0);
    }
    // insertion point
    dw.dxfReal(10, data.ipx);
    dw.dxfReal(20, data.ipy);
    dw.dxfReal(30, data.ipz);

    // vector along bottom side (1 pixel long)
    dw.dxfReal(11, data.ux);
    dw.dxfReal(21, data.uy);
    dw.dxfReal(31, data.uz);

    // vector along left side (1 pixel long)
    dw.dxfReal(12, data.vx);
    dw.dxfReal(22, data.vy);
    dw.dxfReal(32, data.vz);

    // image size in pixel
    dw.dxfReal(13, data.width);
    dw.dxfReal(23, data.height);

    // handle of IMAGEDEF object
    int handle = dw.incHandle();
    dw.dxfHex(340, handle);

    // flags
    dw.dxfInt(70, 15);

    // clipping:
    dw.dxfInt(280, 0);

    // brightness, contrast, fade
    dw.dxfInt(281, data.brightness);
    dw.dxfInt(282, data.contrast);
    dw.dxfInt(283, data.fade);

    return handle;
}



/**
 * Writes an image definiition entity.
 */
void DL_Dxf::writeImageDef(DL_WriterA& dw,
                           int handle,
                           const DL_ImageData& data) {

    /*if (data.file.empty()) {
        std::cerr << "DL_Dxf::writeImage: "
        << "Image file must not be empty\n";
        return;
}*/

    dw.dxfString(0, "IMAGEDEF");
    if (version==DL_VERSION_2000) {
        dw.dxfHex(5, handle);
    }

    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbRasterImageDef");
        dw.dxfInt(90, 0);
    }
    // file name:
    dw.dxfString(1, data.ref);

    // image size in pixel
    dw.dxfReal(10, data.width);
    dw.dxfReal(20, data.height);

    dw.dxfReal(11, 1.0);
    dw.dxfReal(21, 1.0);

    // loaded:
    dw.dxfInt(280, 1);
    // units:
    dw.dxfInt(281, 0);
}


/**
 * Writes a layer to the file. Layers are stored in the 
 * tables section of a DXF file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeLayer(DL_WriterA& dw,
                        const DL_LayerData& data,
                        const DL_Attributes& attrib) {

    if (data.name.empty()) {
        std::cerr << "DL_Dxf::writeLayer: "
        << "Layer name must not be empty\n";
        return;
    }

    int color = attrib.getColor();
    if (color>=256) {
        std::cerr << "Layer color cannot be " << color << ". Changed to 7.\n";
        color = 7;
    }

    if (data.name == "0") {
        dw.tableLayerEntry(0x10);
    } else {
        dw.tableLayerEntry();
    }

    dw.dxfString(2, data.name);
    dw.dxfInt(70, data.flags);
    dw.dxfInt(62, color);
    if (version>=DL_VERSION_2000 && attrib.getColor24()!=-1) {
        dw.dxfInt(420, attrib.getColor24());
    }

    dw.dxfString(6, (attrib.getLinetype().length()==0 ?
                     std::string("CONTINUOUS") : attrib.getLinetype()));

    if (version>=DL_VERSION_2000) {
        // layer defpoints cannot be plotted
        std::string lstr = data.name;
        std::transform(lstr.begin(), lstr.end(), lstr.begin(), tolower);
        if (lstr=="defpoints") {
            dw.dxfInt(290, 0);
        }
    }
    if (version>=DL_VERSION_2000 && attrib.getWidth()!=-1) {
        dw.dxfInt(370, attrib.getWidth());
    }
    if (version>=DL_VERSION_2000) {
        dw.dxfHex(390, 0xF);
    }
}



/**
 * Writes a line type to the file. Line types are stored in the 
 * tables section of a DXF file.
 */
void DL_Dxf::writeLinetype(DL_WriterA& dw,
                           const DL_LinetypeData& data) {

    std::string nameUpper = data.name;
    std::transform(nameUpper.begin(), nameUpper.end(), nameUpper.begin(), ::toupper);

    if (data.name.empty()) {
        std::cerr << "DL_Dxf::writeLinetype: "
        << "Line type name must not be empty\n";
        return;
    }

    // ignore BYLAYER, BYBLOCK for R12
    if (version<DL_VERSION_2000) {
        if (nameUpper=="BYBLOCK" || nameUpper=="BYLAYER") {
            return;
        }
    }

    // write id (not for R12)
    if (nameUpper=="BYBLOCK") {
        dw.tableLinetypeEntry(0x14);
    } else if (nameUpper=="BYLAYER") {
        dw.tableLinetypeEntry(0x15);
    } else if (nameUpper=="CONTINUOUS") {
        dw.tableLinetypeEntry(0x16);
    } else {
        dw.tableLinetypeEntry();
    }

    dw.dxfString(2, data.name);
    dw.dxfInt(70, data.flags);

    if (nameUpper=="BYBLOCK") {
        dw.dxfString(3, "");
        dw.dxfInt(72, 65);
        dw.dxfInt(73, 0);
        dw.dxfReal(40, 0.0);
    } else if (nameUpper=="BYLAYER") {
        dw.dxfString(3, "");
        dw.dxfInt(72, 65);
        dw.dxfInt(73, 0);
        dw.dxfReal(40, 0.0);
    } else if (nameUpper=="CONTINUOUS") {
        dw.dxfString(3, "Solid line");
        dw.dxfInt(72, 65);
        dw.dxfInt(73, 0);
        dw.dxfReal(40, 0.0);
    } else {
        dw.dxfString(3, data.description);
        dw.dxfInt(72, 65);
        dw.dxfInt(73, data.numberOfDashes);
        dw.dxfReal(40, data.patternLength);
        for (int i = 0; i < data.numberOfDashes; i++) {
            dw.dxfReal(49, data.pattern[i]);
            if (version>=DL_VERSION_R13) {
                dw.dxfInt(74, 0);
            }
        }
    }
}



/**
 * Writes the APPID section to the DXF file.
 *
 * @param name Application name
 */
void DL_Dxf::writeAppid(DL_WriterA& dw, const std::string& name) {
    if (name.empty()) {
        std::cerr << "DL_Dxf::writeAppid: "
        << "Application  name must not be empty\n";
        return;
    }

    std::string n = name;
    std::transform(n.begin(), n.end(), n.begin(), ::toupper);

    if (n=="ACAD") {
        dw.tableAppidEntry(0x12);
    } else {
        dw.tableAppidEntry();
    }
    dw.dxfString(2, name);
    dw.dxfInt(70, 0);
}



/**
 * Writes a block's definition (no entities) to the DXF file.
 */
void DL_Dxf::writeBlock(DL_WriterA& dw, const DL_BlockData& data) {
    if (data.name.empty()) {
        std::cerr << "DL_Dxf::writeBlock: "
        << "Block name must not be empty\n";
        return;
    }

    std::string n = data.name;
    std::transform(n.begin(), n.end(), n.begin(), ::toupper);

    if (n=="*PAPER_SPACE") {
        dw.sectionBlockEntry(0x1C);
    } else if (n=="*MODEL_SPACE") {
        dw.sectionBlockEntry(0x20);
    } else if (n=="*PAPER_SPACE0") {
        dw.sectionBlockEntry(0x24);
    } else {
        dw.sectionBlockEntry();
    }
    dw.dxfString(2, data.name);
    dw.dxfInt(70, 0);
    dw.coord(10, data.bpx, data.bpy, data.bpz);
    dw.dxfString(3, data.name);
    dw.dxfString(1, "");
}



/**
 * Writes a block end.
 *
 * @param name Block name
 */
void DL_Dxf::writeEndBlock(DL_WriterA& dw, const std::string& name) {
    std::string n = name;
    std::transform(n.begin(), n.end(), n.begin(), ::toupper);

    if (n=="*PAPER_SPACE") {
        dw.sectionBlockEntryEnd(0x1D);
    } else if (n=="*MODEL_SPACE") {
        dw.sectionBlockEntryEnd(0x21);
    } else if (n=="*PAPER_SPACE0") {
        dw.sectionBlockEntryEnd(0x25);
    } else {
        dw.sectionBlockEntryEnd();
    }
}



/**
 * Writes a viewport section. This section is needed in DL_VERSION_R13.
 * Note that this method currently only writes a faked VPORT section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeVPort(DL_WriterA& dw) {
    dw.dxfString(0, "TABLE");
    dw.dxfString(2, "VPORT");
    if (version==DL_VERSION_2000) {
        dw.dxfHex(5, 0x8);
    }
    //dw.dxfHex(330, 0);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbSymbolTable");
    }
    dw.dxfInt(70, 1);
    dw.dxfString(0, "VPORT");
    //dw.dxfHex(5, 0x2F);
    if (version==DL_VERSION_2000) {
        dw.handle();
    }
    //dw.dxfHex(330, 8);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbSymbolTableRecord");
        dw.dxfString(100, "AcDbViewportTableRecord");
    }
    dw.dxfString(  2, "*Active");
    dw.dxfInt( 70, 0);
    dw.dxfReal( 10, 0.0);
    dw.dxfReal( 20, 0.0);
    dw.dxfReal( 11, 1.0);
    dw.dxfReal( 21, 1.0);
    dw.dxfReal( 12, 286.3055555555555);
    dw.dxfReal( 22, 148.5);
    dw.dxfReal( 13, 0.0);
    dw.dxfReal( 23, 0.0);
    dw.dxfReal( 14, 10.0);
    dw.dxfReal( 24, 10.0);
    dw.dxfReal( 15, 10.0);
    dw.dxfReal( 25, 10.0);
    dw.dxfReal( 16, 0.0);
    dw.dxfReal( 26, 0.0);
    dw.dxfReal( 36, 1.0);
    dw.dxfReal( 17, 0.0);
    dw.dxfReal( 27, 0.0);
    dw.dxfReal( 37, 0.0);
    dw.dxfReal( 40, 297.0);
    dw.dxfReal( 41, 1.92798353909465);
    dw.dxfReal( 42, 50.0);
    dw.dxfReal( 43, 0.0);
    dw.dxfReal( 44, 0.0);
    dw.dxfReal( 50, 0.0);
    dw.dxfReal( 51, 0.0);
    dw.dxfInt( 71, 0);
    dw.dxfInt( 72, 100);
    dw.dxfInt( 73, 1);
    dw.dxfInt( 74, 3);
    dw.dxfInt( 75, 1);
    dw.dxfInt( 76, 1);
    dw.dxfInt( 77, 0);
    dw.dxfInt( 78, 0);

    if (version==DL_VERSION_2000) {
        dw.dxfInt(281, 0);
        dw.dxfInt( 65, 1);
        dw.dxfReal(110, 0.0);
        dw.dxfReal(120, 0.0);
        dw.dxfReal(130, 0.0);
        dw.dxfReal(111, 1.0);
        dw.dxfReal(121, 0.0);
        dw.dxfReal(131, 0.0);
        dw.dxfReal(112, 0.0);
        dw.dxfReal(122, 1.0);
        dw.dxfReal(132, 0.0);
        dw.dxfInt( 79, 0);
        dw.dxfReal(146, 0.0);
    }
    dw.dxfString(  0, "ENDTAB");
}



/**
 * Writes a style section. This section is needed in DL_VERSION_R13.
 */
void DL_Dxf::writeStyle(DL_WriterA& dw, const DL_StyleData& style) {
//    dw.dxfString(  0, "TABLE");
//    dw.dxfString(  2, "STYLE");
//    if (version==DL_VERSION_2000) {
//        dw.dxfHex(5, 3);
//    }
    //dw.dxfHex(330, 0);
//    if (version==DL_VERSION_2000) {
//        dw.dxfString(100, "AcDbSymbolTable");
//    }
//    dw.dxfInt( 70, 1);
    dw.dxfString(  0, "STYLE");
    if (version==DL_VERSION_2000) {
        if (style.name=="Standard") {
            //dw.dxfHex(5, 0x11);
            styleHandleStd = dw.handle();
        }
        else {
            dw.handle();
        }
    }
    //dw.dxfHex(330, 3);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbSymbolTableRecord");
        dw.dxfString(100, "AcDbTextStyleTableRecord");
    }
    dw.dxfString(  2, style.name);
    dw.dxfInt( 70, style.flags);
    dw.dxfReal( 40, style.fixedTextHeight);
    dw.dxfReal( 41, style.widthFactor);
    dw.dxfReal( 50, style.obliqueAngle);
    dw.dxfInt( 71, style.textGenerationFlags);
    dw.dxfReal( 42, style.lastHeightUsed);
    if (version==DL_VERSION_2000) {
        dw.dxfString(  3, "");
        dw.dxfString(  4, "");
        dw.dxfString(1001, "ACAD");
        //dw.dxfString(1000, style.name);
        dw.dxfString(1000, style.primaryFontFile);
        int xFlags = 0;
        if (style.bold) {
            xFlags = xFlags|0x2000000;
        }
        if (style.italic) {
            xFlags = xFlags|0x1000000;
        }
        dw.dxfInt(1071, xFlags);
    }
    else {
        dw.dxfString(  3, style.primaryFontFile);
        dw.dxfString(  4, style.bigFontFile);
    }
    //dw.dxfString(  0, "ENDTAB");
}



/**
 * Writes a view section. This section is needed in DL_VERSION_R13.
 * Note that this method currently only writes a faked VIEW section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeView(DL_WriterA& dw) {
    dw.dxfString(  0, "TABLE");
    dw.dxfString(  2, "VIEW");
    if (version==DL_VERSION_2000) {
        dw.dxfHex(5, 6);
    }
    //dw.dxfHex(330, 0);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbSymbolTable");
    }
    dw.dxfInt( 70, 0);
    dw.dxfString(  0, "ENDTAB");
}



/**
 * Writes a ucs section. This section is needed in DL_VERSION_R13.
 * Note that this method currently only writes a faked UCS section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeUcs(DL_WriterA& dw) {
    dw.dxfString(  0, "TABLE");
    dw.dxfString(  2, "UCS");
    if (version==DL_VERSION_2000) {
        dw.dxfHex(5, 7);
    }
    //dw.dxfHex(330, 0);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbSymbolTable");
    }
    dw.dxfInt( 70, 0);
    dw.dxfString(  0, "ENDTAB");
}



/**
 * Writes a dimstyle section. This section is needed in DL_VERSION_R13.
 * Note that this method currently only writes a faked DIMSTYLE section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeDimStyle(DL_WriterA& dw,
                    double dimasz, double dimexe, double dimexo,
                    double dimgap, double dimtxt) {

    dw.dxfString(  0, "TABLE");
    dw.dxfString(  2, "DIMSTYLE");
    if (version==DL_VERSION_2000) {
        dw.dxfHex(5, 0xA);
        dw.dxfString(100, "AcDbSymbolTable");
    }
    dw.dxfInt( 70, 1);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbDimStyleTable");
        dw.dxfInt( 71, 0);
    }


    dw.dxfString(  0, "DIMSTYLE");
    if (version==DL_VERSION_2000) {
        dw.dxfHex(105, 0x27);
    }
    //dw.handle(105);
    //dw.dxfHex(330, 0xA);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbSymbolTableRecord");
        dw.dxfString(100, "AcDbDimStyleTableRecord");
    }
    dw.dxfString(  2, "Standard");
    if (version==DL_VERSION_R12) {
        dw.dxfString(  3, "");
        dw.dxfString(  4, "");
        dw.dxfString(  5, "");
        dw.dxfString(  6, "");
        dw.dxfString(  7, "");
        dw.dxfReal( 40, 1.0);
    }

    dw.dxfReal( 41, dimasz);
    dw.dxfReal( 42, dimexo);
    dw.dxfReal( 43, 3.75);
    dw.dxfReal( 44, dimexe);
    if (version==DL_VERSION_R12) {
        dw.dxfReal( 45, 0.0);
        dw.dxfReal( 46, 0.0);
        dw.dxfReal( 47, 0.0);
        dw.dxfReal( 48, 0.0);
    }
    dw.dxfInt( 70, 0);
    if (version==DL_VERSION_R12) {
        dw.dxfInt( 71, 0);
        dw.dxfInt( 72, 0);
    }
    dw.dxfInt( 73, 0);
    dw.dxfInt( 74, 0);
    if (version==DL_VERSION_R12) {
        dw.dxfInt( 75, 0);
        dw.dxfInt( 76, 0);
    }
    dw.dxfInt( 77, 1);
    dw.dxfInt( 78, 8);
    dw.dxfReal(140, dimtxt);
    dw.dxfReal(141, 2.5);
    if (version==DL_VERSION_R12) {
        dw.dxfReal(142, 0.0);
    }
    dw.dxfReal(143, 0.03937007874016);
    if (version==DL_VERSION_R12) {
        dw.dxfReal(144, 1.0);
        dw.dxfReal(145, 0.0);
        dw.dxfReal(146, 1.0);
    }
    dw.dxfReal(147, dimgap);
    if (version==DL_VERSION_R12) {
        dw.dxfInt(170, 0);
    }
    dw.dxfInt(171, 3);
    dw.dxfInt(172, 1);
    if (version==DL_VERSION_R12) {
        dw.dxfInt(173, 0);
        dw.dxfInt(174, 0);
        dw.dxfInt(175, 0);
        dw.dxfInt(176, 0);
        dw.dxfInt(177, 0);
        dw.dxfInt(178, 0);
    }
    if (version==DL_VERSION_2000) {
        dw.dxfInt(271, 2);
        dw.dxfInt(272, 2);
        dw.dxfInt(274, 3);
        dw.dxfInt(278, 44);
        dw.dxfInt(283, 0);
        dw.dxfInt(284, 8);
        dw.dxfHex(340, styleHandleStd);
        //dw.dxfHex(340, 0x11);
    }
    // * /
    dw.dxfString(  0, "ENDTAB");
}



/**
 * Writes a blockrecord section. This section is needed in DL_VERSION_R13.
 * Note that this method currently only writes a faked BLOCKRECORD section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeBlockRecord(DL_WriterA& dw) {
    dw.dxfString(  0, "TABLE");
    dw.dxfString(  2, "BLOCK_RECORD");
    if (version==DL_VERSION_2000) {
        dw.dxfHex(5, 1);
    }
    //dw.dxfHex(330, 0);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbSymbolTable");
    }
    dw.dxfInt( 70, 1);

    dw.dxfString(  0, "BLOCK_RECORD");
    if (version==DL_VERSION_2000) {
        dw.dxfHex(5, 0x1F);
    }
    //int msh = dw.handle();
    //dw.setModelSpaceHandle(msh);
    //dw.dxfHex(330, 1);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbSymbolTableRecord");
        dw.dxfString(100, "AcDbBlockTableRecord");
    }
    dw.dxfString(  2, "*Model_Space");
    dw.dxfHex(340, 0x22);

    dw.dxfString(  0, "BLOCK_RECORD");
    if (version==DL_VERSION_2000) {
        dw.dxfHex(5, 0x1B);
    }
    //int psh = dw.handle();
    //dw.setPaperSpaceHandle(psh);
    //dw.dxfHex(330, 1);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbSymbolTableRecord");
        dw.dxfString(100, "AcDbBlockTableRecord");
    }
    dw.dxfString(  2, "*Paper_Space");
    dw.dxfHex(340, 0x1E);

    dw.dxfString(  0, "BLOCK_RECORD");
    if (version==DL_VERSION_2000) {
        dw.dxfHex(5, 0x23);
    }
    //int ps0h = dw.handle();
    //dw.setPaperSpace0Handle(ps0h);
    //dw.dxfHex(330, 1);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbSymbolTableRecord");
        dw.dxfString(100, "AcDbBlockTableRecord");
    }
    dw.dxfString(  2, "*Paper_Space0");
    dw.dxfHex(340, 0x26);

    //dw.dxfString(  0, "ENDTAB");
}



/**
 * Writes a single block record with the given name.
 */
void DL_Dxf::writeBlockRecord(DL_WriterA& dw, const std::string& name) {
    dw.dxfString(  0, "BLOCK_RECORD");
    if (version==DL_VERSION_2000) {
        dw.handle();
    }
    //dw->dxfHex(330, 1);
    if (version==DL_VERSION_2000) {
        dw.dxfString(100, "AcDbSymbolTableRecord");
        dw.dxfString(100, "AcDbBlockTableRecord");
    }
    dw.dxfString(  2, name);
    dw.dxfHex(340, 0);
}



/**
 * Writes a objects section. This section is needed in DL_VERSION_R13.
 * Note that this method currently only writes a faked OBJECTS section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeObjects(DL_WriterA& dw, const std::string& appDictionaryName) {
    dw.dxfString(  0, "SECTION");
    dw.dxfString(  2, "OBJECTS");


    dw.dxfString(  0, "DICTIONARY");
    dw.dxfHex(5, 0xC);
    dw.dxfString(100, "AcDbDictionary");
    dw.dxfInt(280, 0);
    dw.dxfInt(281, 1);
    dw.dxfString(  3, "ACAD_GROUP");
    dw.dxfHex(350, 0xD);
    dw.dxfString(  3, "ACAD_LAYOUT");
    dw.dxfHex(350, 0x1A);
    dw.dxfString(  3, "ACAD_MLINESTYLE");
    dw.dxfHex(350, 0x17);
    dw.dxfString(  3, "ACAD_PLOTSETTINGS");
    dw.dxfHex(350, 0x19);
    dw.dxfString(  3, "ACAD_PLOTSTYLENAME");
    dw.dxfHex(350, 0xE);
    dw.dxfString(  3, "AcDbVariableDictionary");
    int acDbVariableDictionaryHandle = dw.handle(350);
    //int acDbVariableDictionaryHandle = dw.getNextHandle();
    //dw.dxfHex(350, acDbVariableDictionaryHandle);
    //dw.incHandle();

    if (appDictionaryName.length()!=0) {
        dw.dxfString(  3, appDictionaryName);
        appDictionaryHandle = dw.handle(350);
        //appDictionaryHandle = dw.getNextHandle();
        //dw.dxfHex(350, appDictionaryHandle);
        //dw.incHandle();
    }

    dw.dxfString(  0, "DICTIONARY");
    dw.dxfHex(5, 0xD);
    //dw.handle();                                    // D
    //dw.dxfHex(330, 0xC);
    dw.dxfString(100, "AcDbDictionary");
    dw.dxfInt(280, 0);
    dw.dxfInt(281, 1);


    dw.dxfString(  0, "ACDBDICTIONARYWDFLT");
    dw.dxfHex(5, 0xE);
    //dicId4 = dw.handle();                           // E
    //dw.dxfHex(330, 0xC);                       // C
    dw.dxfString(100, "AcDbDictionary");
    dw.dxfInt(281, 1);
    dw.dxfString(  3, "Normal");
    dw.dxfHex(350, 0xF);
    //dw.dxfHex(350, dw.getNextHandle()+5);        // F
    dw.dxfString(100, "AcDbDictionaryWithDefault");
    dw.dxfHex(340, 0xF);
    //dw.dxfHex(340, dw.getNextHandle()+5);        // F


    dw.dxfString(  0, "ACDBPLACEHOLDER");
    dw.dxfHex(5, 0xF);
    //dw.handle();                                    // F
    //dw.dxfHex(330, dicId4);                      // E


    dw.dxfString(  0, "DICTIONARY");
    //dicId3 = dw.handle();                           // 17
    dw.dxfHex(5, 0x17);
    //dw.dxfHex(330, 0xC);                       // C
    dw.dxfString(100, "AcDbDictionary");
    dw.dxfInt(280, 0);
    dw.dxfInt(281, 1);
    dw.dxfString(  3, "Standard");
    dw.dxfHex(350, 0x18);
    //dw.dxfHex(350, dw.getNextHandle()+5);        // 18


    dw.dxfString(  0, "MLINESTYLE");
    dw.dxfHex(5, 0x18);
    //dw.handle();                                    // 18
    //dw.dxfHex(330, dicId3);                      // 17
    dw.dxfString(100, "AcDbMlineStyle");
    dw.dxfString(  2, "STANDARD");
    dw.dxfInt( 70, 0);
    dw.dxfString(  3, "");
    dw.dxfInt( 62, 256);
    dw.dxfReal( 51, 90.0);
    dw.dxfReal( 52, 90.0);
    dw.dxfInt( 71, 2);
    dw.dxfReal( 49, 0.5);
    dw.dxfInt( 62, 256);
    dw.dxfString(  6, "BYLAYER");
    dw.dxfReal( 49, -0.5);
    dw.dxfInt( 62, 256);
    dw.dxfString(  6, "BYLAYER");


    dw.dxfString(  0, "DICTIONARY");
    dw.dxfHex(5, 0x19);
    //dw.handle();                           // 17
    //dw.dxfHex(330, 0xC);                       // C
    dw.dxfString(100, "AcDbDictionary");
    dw.dxfInt(280, 0);
    dw.dxfInt(281, 1);


    dw.dxfString(  0, "DICTIONARY");
    //dicId2 = dw.handle();                           // 1A
    dw.dxfHex(5, 0x1A);
    //dw.dxfHex(330, 0xC);
    dw.dxfString(100, "AcDbDictionary");
    dw.dxfInt(281, 1);
    dw.dxfString(  3, "Layout1");
    dw.dxfHex(350, 0x1E);
    //dw.dxfHex(350, dw.getNextHandle()+2);        // 1E
    dw.dxfString(  3, "Layout2");
    dw.dxfHex(350, 0x26);
    //dw.dxfHex(350, dw.getNextHandle()+4);        // 26
    dw.dxfString(  3, "Model");
    dw.dxfHex(350, 0x22);
    //dw.dxfHex(350, dw.getNextHandle()+5);        // 22


    dw.dxfString(  0, "LAYOUT");
    dw.dxfHex(5, 0x1E);
    //dw.handle();                                    // 1E
    //dw.dxfHex(330, dicId2);                      // 1A
    dw.dxfString(100, "AcDbPlotSettings");
    dw.dxfString(  1, "");
    dw.dxfString(  2, "none_device");
    dw.dxfString(  4, "");
    dw.dxfString(  6, "");
    dw.dxfReal( 40, 0.0);
    dw.dxfReal( 41, 0.0);
    dw.dxfReal( 42, 0.0);
    dw.dxfReal( 43, 0.0);
    dw.dxfReal( 44, 0.0);
    dw.dxfReal( 45, 0.0);
    dw.dxfReal( 46, 0.0);
    dw.dxfReal( 47, 0.0);
    dw.dxfReal( 48, 0.0);
    dw.dxfReal( 49, 0.0);
    dw.dxfReal(140, 0.0);
    dw.dxfReal(141, 0.0);
    dw.dxfReal(142, 1.0);
    dw.dxfReal(143, 1.0);
    dw.dxfInt( 70, 688);
    dw.dxfInt( 72, 0);
    dw.dxfInt( 73, 0);
    dw.dxfInt( 74, 5);
    dw.dxfString(  7, "");
    dw.dxfInt( 75, 16);
    dw.dxfReal(147, 1.0);
    dw.dxfReal(148, 0.0);
    dw.dxfReal(149, 0.0);
    dw.dxfString(100, "AcDbLayout");
    dw.dxfString(  1, "Layout1");
    dw.dxfInt( 70, 1);
    dw.dxfInt( 71, 1);
    dw.dxfReal( 10, 0.0);
    dw.dxfReal( 20, 0.0);
    dw.dxfReal( 11, 420.0);
    dw.dxfReal( 21, 297.0);
    dw.dxfReal( 12, 0.0);
    dw.dxfReal( 22, 0.0);
    dw.dxfReal( 32, 0.0);
    dw.dxfReal( 14, 1.000000000000000E+20);
    dw.dxfReal( 24, 1.000000000000000E+20);
    dw.dxfReal( 34, 1.000000000000000E+20);
    dw.dxfReal( 15, -1.000000000000000E+20);
    dw.dxfReal( 25, -1.000000000000000E+20);
    dw.dxfReal( 35, -1.000000000000000E+20);
    dw.dxfReal(146, 0.0);
    dw.dxfReal( 13, 0.0);
    dw.dxfReal( 23, 0.0);
    dw.dxfReal( 33, 0.0);
    dw.dxfReal( 16, 1.0);
    dw.dxfReal( 26, 0.0);
    dw.dxfReal( 36, 0.0);
    dw.dxfReal( 17, 0.0);
    dw.dxfReal( 27, 1.0);
    dw.dxfReal( 37, 0.0);
    dw.dxfInt( 76, 0);
    //dw.dxfHex(330, dw.getPaperSpaceHandle());    // 1B
    dw.dxfHex(330, 0x1B);


    dw.dxfString(  0, "LAYOUT");
    dw.dxfHex(5, 0x22);
    //dw.handle();                                    // 22
    //dw.dxfHex(330, dicId2);                      // 1A
    dw.dxfString(100, "AcDbPlotSettings");
    dw.dxfString(  1, "");
    dw.dxfString(  2, "none_device");
    dw.dxfString(  4, "");
    dw.dxfString(  6, "");
    dw.dxfReal( 40, 0.0);
    dw.dxfReal( 41, 0.0);
    dw.dxfReal( 42, 0.0);
    dw.dxfReal( 43, 0.0);
    dw.dxfReal( 44, 0.0);
    dw.dxfReal( 45, 0.0);
    dw.dxfReal( 46, 0.0);
    dw.dxfReal( 47, 0.0);
    dw.dxfReal( 48, 0.0);
    dw.dxfReal( 49, 0.0);
    dw.dxfReal(140, 0.0);
    dw.dxfReal(141, 0.0);
    dw.dxfReal(142, 1.0);
    dw.dxfReal(143, 1.0);
    dw.dxfInt( 70, 1712);
    dw.dxfInt( 72, 0);
    dw.dxfInt( 73, 0);
    dw.dxfInt( 74, 0);
    dw.dxfString(  7, "");
    dw.dxfInt( 75, 0);
    dw.dxfReal(147, 1.0);
    dw.dxfReal(148, 0.0);
    dw.dxfReal(149, 0.0);
    dw.dxfString(100, "AcDbLayout");
    dw.dxfString(  1, "Model");
    dw.dxfInt( 70, 1);
    dw.dxfInt( 71, 0);
    dw.dxfReal( 10, 0.0);
    dw.dxfReal( 20, 0.0);
    dw.dxfReal( 11, 12.0);
    dw.dxfReal( 21, 9.0);
    dw.dxfReal( 12, 0.0);
    dw.dxfReal( 22, 0.0);
    dw.dxfReal( 32, 0.0);
    dw.dxfReal( 14, 0.0);
    dw.dxfReal( 24, 0.0);
    dw.dxfReal( 34, 0.0);
    dw.dxfReal( 15, 0.0);
    dw.dxfReal( 25, 0.0);
    dw.dxfReal( 35, 0.0);
    dw.dxfReal(146, 0.0);
    dw.dxfReal( 13, 0.0);
    dw.dxfReal( 23, 0.0);
    dw.dxfReal( 33, 0.0);
    dw.dxfReal( 16, 1.0);
    dw.dxfReal( 26, 0.0);
    dw.dxfReal( 36, 0.0);
    dw.dxfReal( 17, 0.0);
    dw.dxfReal( 27, 1.0);
    dw.dxfReal( 37, 0.0);
    dw.dxfInt( 76, 0);
    //dw.dxfHex(330, dw.getModelSpaceHandle());    // 1F
    dw.dxfHex(330, 0x1F);


    dw.dxfString(  0, "LAYOUT");
    //dw.handle();                                    // 26
    dw.dxfHex(5, 0x26);
    //dw.dxfHex(330, dicId2);                      // 1A
    dw.dxfString(100, "AcDbPlotSettings");
    dw.dxfString(  1, "");
    dw.dxfString(  2, "none_device");
    dw.dxfString(  4, "");
    dw.dxfString(  6, "");
    dw.dxfReal( 40, 0.0);
    dw.dxfReal( 41, 0.0);
    dw.dxfReal( 42, 0.0);
    dw.dxfReal( 43, 0.0);
    dw.dxfReal( 44, 0.0);
    dw.dxfReal( 45, 0.0);
    dw.dxfReal( 46, 0.0);
    dw.dxfReal( 47, 0.0);
    dw.dxfReal( 48, 0.0);
    dw.dxfReal( 49, 0.0);
    dw.dxfReal(140, 0.0);
    dw.dxfReal(141, 0.0);
    dw.dxfReal(142, 1.0);
    dw.dxfReal(143, 1.0);
    dw.dxfInt( 70, 688);
    dw.dxfInt( 72, 0);
    dw.dxfInt( 73, 0);
    dw.dxfInt( 74, 5);
    dw.dxfString(  7, "");
    dw.dxfInt( 75, 16);
    dw.dxfReal(147, 1.0);
    dw.dxfReal(148, 0.0);
    dw.dxfReal(149, 0.0);
    dw.dxfString(100, "AcDbLayout");
    dw.dxfString(  1, "Layout2");
    dw.dxfInt( 70, 1);
    dw.dxfInt( 71, 2);
    dw.dxfReal( 10, 0.0);
    dw.dxfReal( 20, 0.0);
    dw.dxfReal( 11, 12.0);
    dw.dxfReal( 21, 9.0);
    dw.dxfReal( 12, 0.0);
    dw.dxfReal( 22, 0.0);
    dw.dxfReal( 32, 0.0);
    dw.dxfReal( 14, 0.0);
    dw.dxfReal( 24, 0.0);
    dw.dxfReal( 34, 0.0);
    dw.dxfReal( 15, 0.0);
    dw.dxfReal( 25, 0.0);
    dw.dxfReal( 35, 0.0);
    dw.dxfReal(146, 0.0);
    dw.dxfReal( 13, 0.0);
    dw.dxfReal( 23, 0.0);
    dw.dxfReal( 33, 0.0);
    dw.dxfReal( 16, 1.0);
    dw.dxfReal( 26, 0.0);
    dw.dxfReal( 36, 0.0);
    dw.dxfReal( 17, 0.0);
    dw.dxfReal( 27, 1.0);
    dw.dxfReal( 37, 0.0);
    dw.dxfInt( 76, 0);
    //dw.dxfHex(330, dw.getPaperSpace0Handle());   // 23
    dw.dxfHex(330, 0x23);

    dw.dxfString(  0, "DICTIONARY");
    //dw.dxfHex(5, 0x2C);
    //dicId5 =
    dw.dxfHex(5, acDbVariableDictionaryHandle);
    //dw.handle();                           // 2C
    //dw.dxfHex(330, 0xC);                       // C
    dw.dxfString(100, "AcDbDictionary");
    dw.dxfInt(281, 1);
    dw.dxfString(  3, "DIMASSOC");
    //dw.dxfHex(350, 0x2F);
    dw.dxfHex(350, dw.getNextHandle()+1);        // 2E
    dw.dxfString(  3, "HIDETEXT");
    //dw.dxfHex(350, 0x2E);
    dw.dxfHex(350, dw.getNextHandle());        // 2D


    dw.dxfString(  0, "DICTIONARYVAR");
    //dw.dxfHex(5, 0x2E);
    dw.handle();                                    // 2E
    //dw.dxfHex(330, dicId5);                      // 2C
    dw.dxfString(100, "DictionaryVariables");
    dw.dxfInt(280, 0);
    dw.dxfInt(  1, 2);


    dw.dxfString(  0, "DICTIONARYVAR");
    //dw.dxfHex(5, 0x2D);
    dw.handle();                                    // 2D
    //dw.dxfHex(330, dicId5);                      // 2C
    dw.dxfString(100, "DictionaryVariables");
    dw.dxfInt(280, 0);
    dw.dxfInt(  1, 1);
}

void DL_Dxf::writeAppDictionary(DL_WriterA& dw) {
    dw.dxfString(  0, "DICTIONARY");
    //dw.handle();
    dw.dxfHex(5, appDictionaryHandle);
    dw.dxfString(100, "AcDbDictionary");
    dw.dxfInt(281, 1);
}

int DL_Dxf::writeDictionaryEntry(DL_WriterA& dw, const std::string& name) {
    dw.dxfString(  3, name);
    int handle = dw.getNextHandle();
    dw.dxfHex(350, handle);
    dw.incHandle();
    return handle;
}

void DL_Dxf::writeXRecord(DL_WriterA& dw, int handle, int value) {
    dw.dxfString(  0, "XRECORD");
    dw.dxfHex(5, handle);
    dw.dxfHex(330, appDictionaryHandle);
    dw.dxfString(100, "AcDbXrecord");
    dw.dxfInt(280, 1);
    dw.dxfInt(90, value);
}

void DL_Dxf::writeXRecord(DL_WriterA& dw, int handle, double value) {
    dw.dxfString(  0, "XRECORD");
    dw.dxfHex(5, handle);
    dw.dxfHex(330, appDictionaryHandle);
    dw.dxfString(100, "AcDbXrecord");
    dw.dxfInt(280, 1);
    dw.dxfReal(40, value);
}

void DL_Dxf::writeXRecord(DL_WriterA& dw, int handle, bool value) {
    dw.dxfString(  0, "XRECORD");
    dw.dxfHex(5, handle);
    dw.dxfHex(330, appDictionaryHandle);
    dw.dxfString(100, "AcDbXrecord");
    dw.dxfInt(280, 1);
    dw.dxfBool(290, value);
}

void DL_Dxf::writeXRecord(DL_WriterA& dw, int handle, const std::string& value) {
    dw.dxfString(  0, "XRECORD");
    dw.dxfHex(5, handle);
    dw.dxfHex(330, appDictionaryHandle);
    dw.dxfString(100, "AcDbXrecord");
    dw.dxfInt(280, 1);
    dw.dxfString(1000, value);
}

/**
 * Writes the end of the objects section. This section is needed in DL_VERSION_R13.
 * Note that this method currently only writes a faked OBJECTS section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeObjectsEnd(DL_WriterA& dw) {
    dw.dxfString(  0, "ENDSEC");
}

    

/**
 * Writes a comment to the DXF file.
 */
void DL_Dxf::writeComment(DL_WriterA& dw, const std::string& comment) {
    dw.dxfString(999, comment);
}


/**
 * Checks if the given variable is known by the given DXF version.
 */
bool DL_Dxf::checkVariable(const char* var, DL_Codes::version version) {
    if (version>=DL_VERSION_2000) {
        return true;
    } else if (version==DL_VERSION_R12) {
        // these are all the variables recognized by dxf r12:
        if (!strcmp(var, "$ACADVER")) {
            return true;
        }
        if (!strcmp(var, "$ACADVER")) {
            return true;
        }
        if (!strcmp(var, "$ANGBASE")) {
            return true;
        }
        if (!strcmp(var, "$ANGDIR")) {
            return true;
        }
        if (!strcmp(var, "$ATTDIA")) {
            return true;
        }
        if (!strcmp(var, "$ATTMODE")) {
            return true;
        }
        if (!strcmp(var, "$ATTREQ")) {
            return true;
        }
        if (!strcmp(var, "$AUNITS")) {
            return true;
        }
        if (!strcmp(var, "$AUPREC")) {
            return true;
        }
        if (!strcmp(var, "$AXISMODE")) {
            return true;
        }
        if (!strcmp(var, "$AXISUNIT")) {
            return true;
        }
        if (!strcmp(var, "$BLIPMODE")) {
            return true;
        }
        if (!strcmp(var, "$CECOLOR")) {
            return true;
        }
        if (!strcmp(var, "$CELTYPE")) {
            return true;
        }
        if (!strcmp(var, "$CHAMFERA")) {
            return true;
        }
        if (!strcmp(var, "$CHAMFERB")) {
            return true;
        }
        if (!strcmp(var, "$CLAYER")) {
            return true;
        }
        if (!strcmp(var, "$COORDS")) {
            return true;
        }
        if (!strcmp(var, "$DIMALT")) {
            return true;
        }
        if (!strcmp(var, "$DIMALTD")) {
            return true;
        }
        if (!strcmp(var, "$DIMALTF")) {
            return true;
        }
        if (!strcmp(var, "$DIMAPOST")) {
            return true;
        }
        if (!strcmp(var, "$DIMASO")) {
            return true;
        }
        if (!strcmp(var, "$DIMASZ")) {
            return true;
        }
        if (!strcmp(var, "$DIMBLK")) {
            return true;
        }
        if (!strcmp(var, "$DIMBLK1")) {
            return true;
        }
        if (!strcmp(var, "$DIMBLK2")) {
            return true;
        }
        if (!strcmp(var, "$DIMCEN")) {
            return true;
        }
        if (!strcmp(var, "$DIMCLRD")) {
            return true;
        }
        if (!strcmp(var, "$DIMCLRE")) {
            return true;
        }
        if (!strcmp(var, "$DIMCLRT")) {
            return true;
        }
        if (!strcmp(var, "$DIMDLE")) {
            return true;
        }
        if (!strcmp(var, "$DIMDLI")) {
            return true;
        }
        if (!strcmp(var, "$DIMEXE")) {
            return true;
        }
        if (!strcmp(var, "$DIMEXO")) {
            return true;
        }
        if (!strcmp(var, "$DIMGAP")) {
            return true;
        }
        if (!strcmp(var, "$DIMLFAC")) {
            return true;
        }
        if (!strcmp(var, "$DIMLIM")) {
            return true;
        }
        if (!strcmp(var, "$DIMPOST")) {
            return true;
        }
        if (!strcmp(var, "$DIMRND")) {
            return true;
        }
        if (!strcmp(var, "$DIMSAH")) {
            return true;
        }
        if (!strcmp(var, "$DIMSCALE")) {
            return true;
        }
        if (!strcmp(var, "$DIMSE1")) {
            return true;
        }
        if (!strcmp(var, "$DIMSE2")) {
            return true;
        }
        if (!strcmp(var, "$DIMSHO")) {
            return true;
        }
        if (!strcmp(var, "$DIMSOXD")) {
            return true;
        }
        if (!strcmp(var, "$DIMSTYLE")) {
            return true;
        }
        if (!strcmp(var, "$DIMTAD")) {
            return true;
        }
        if (!strcmp(var, "$DIMTFAC")) {
            return true;
        }
        if (!strcmp(var, "$DIMTIH")) {
            return true;
        }
        if (!strcmp(var, "$DIMTIX")) {
            return true;
        }
        if (!strcmp(var, "$DIMTM")) {
            return true;
        }
        if (!strcmp(var, "$DIMTOFL")) {
            return true;
        }
        if (!strcmp(var, "$DIMTOH")) {
            return true;
        }
        if (!strcmp(var, "$DIMTOL")) {
            return true;
        }
        if (!strcmp(var, "$DIMTP")) {
            return true;
        }
        if (!strcmp(var, "$DIMTSZ")) {
            return true;
        }
        if (!strcmp(var, "$DIMTVP")) {
            return true;
        }
        if (!strcmp(var, "$DIMTXT")) {
            return true;
        }
        if (!strcmp(var, "$DIMZIN")) {
            return true;
        }
        if (!strcmp(var, "$DWGCODEPAGE")) {
            return true;
        }
        if (!strcmp(var, "$DRAGMODE")) {
            return true;
        }
        if (!strcmp(var, "$ELEVATION")) {
            return true;
        }
        if (!strcmp(var, "$EXTMAX")) {
            return true;
        }
        if (!strcmp(var, "$EXTMIN")) {
            return true;
        }
        if (!strcmp(var, "$FILLETRAD")) {
            return true;
        }
        if (!strcmp(var, "$FILLMODE")) {
            return true;
        }
        if (!strcmp(var, "$HANDLING")) {
            return true;
        }
        if (!strcmp(var, "$HANDSEED")) {
            return true;
        }
        if (!strcmp(var, "$INSBASE")) {
            return true;
        }
        if (!strcmp(var, "$LIMCHECK")) {
            return true;
        }
        if (!strcmp(var, "$LIMMAX")) {
            return true;
        }
        if (!strcmp(var, "$LIMMIN")) {
            return true;
        }
        if (!strcmp(var, "$LTSCALE")) {
            return true;
        }
        if (!strcmp(var, "$LUNITS")) {
            return true;
        }
        if (!strcmp(var, "$LUPREC")) {
            return true;
        }
        if (!strcmp(var, "$MAXACTVP")) {
            return true;
        }
        if (!strcmp(var, "$MENU")) {
            return true;
        }
        if (!strcmp(var, "$MIRRTEXT")) {
            return true;
        }
        if (!strcmp(var, "$ORTHOMODE")) {
            return true;
        }
        if (!strcmp(var, "$OSMODE")) {
            return true;
        }
        if (!strcmp(var, "$PDMODE")) {
            return true;
        }
        if (!strcmp(var, "$PDSIZE")) {
            return true;
        }
        if (!strcmp(var, "$PELEVATION")) {
            return true;
        }
        if (!strcmp(var, "$PEXTMAX")) {
            return true;
        }
        if (!strcmp(var, "$PEXTMIN")) {
            return true;
        }
        if (!strcmp(var, "$PLIMCHECK")) {
            return true;
        }
        if (!strcmp(var, "$PLIMMAX")) {
            return true;
        }
        if (!strcmp(var, "$PLIMMIN")) {
            return true;
        }
        if (!strcmp(var, "$PLINEGEN")) {
            return true;
        }
        if (!strcmp(var, "$PLINEWID")) {
            return true;
        }
        if (!strcmp(var, "$PSLTSCALE")) {
            return true;
        }
        if (!strcmp(var, "$PUCSNAME")) {
            return true;
        }
        if (!strcmp(var, "$PUCSORG")) {
            return true;
        }
        if (!strcmp(var, "$PUCSXDIR")) {
            return true;
        }
        if (!strcmp(var, "$PUCSYDIR")) {
            return true;
        }
        if (!strcmp(var, "$QTEXTMODE")) {
            return true;
        }
        if (!strcmp(var, "$REGENMODE")) {
            return true;
        }
        if (!strcmp(var, "$SHADEDGE")) {
            return true;
        }
        if (!strcmp(var, "$SHADEDIF")) {
            return true;
        }
        if (!strcmp(var, "$SKETCHINC")) {
            return true;
        }
        if (!strcmp(var, "$SKPOLY")) {
            return true;
        }
        if (!strcmp(var, "$SPLFRAME")) {
            return true;
        }
        if (!strcmp(var, "$SPLINESEGS")) {
            return true;
        }
        if (!strcmp(var, "$SPLINETYPE")) {
            return true;
        }
        if (!strcmp(var, "$SURFTAB1")) {
            return true;
        }
        if (!strcmp(var, "$SURFTAB2")) {
            return true;
        }
        if (!strcmp(var, "$SURFTYPE")) {
            return true;
        }
        if (!strcmp(var, "$SURFU")) {
            return true;
        }
        if (!strcmp(var, "$SURFV")) {
            return true;
        }
        if (!strcmp(var, "$TDCREATE")) {
            return true;
        }
        if (!strcmp(var, "$TDINDWG")) {
            return true;
        }
        if (!strcmp(var, "$TDUPDATE")) {
            return true;
        }
        if (!strcmp(var, "$TDUSRTIMER")) {
            return true;
        }
        if (!strcmp(var, "$TEXTSIZE")) {
            return true;
        }
        if (!strcmp(var, "$TEXTSTYLE")) {
            return true;
        }
        if (!strcmp(var, "$THICKNESS")) {
            return true;
        }
        if (!strcmp(var, "$TILEMODE")) {
            return true;
        }
        if (!strcmp(var, "$TRACEWID")) {
            return true;
        }
        if (!strcmp(var, "$UCSNAME")) {
            return true;
        }
        if (!strcmp(var, "$UCSORG")) {
            return true;
        }
        if (!strcmp(var, "$UCSXDIR")) {
            return true;
        }
        if (!strcmp(var, "$UCSYDIR")) {
            return true;
        }
        if (!strcmp(var, "$UNITMODE")) {
            return true;
        }
        if (!strcmp(var, "$USERI1")) {
            return true;
        }
        if (!strcmp(var, "$USERR1")) {
            return true;
        }
        if (!strcmp(var, "$USRTIMER")) {
            return true;
        }
        if (!strcmp(var, "$VISRETAIN")) {
            return true;
        }
        if (!strcmp(var, "$WORLDVIEW")) {
            return true;
        }
        if (!strcmp(var, "$FASTZOOM")) {
            return true;
        }
        if (!strcmp(var, "$GRIDMODE")) {
            return true;
        }
        if (!strcmp(var, "$GRIDUNIT")) {
            return true;
        }
        if (!strcmp(var, "$SNAPANG")) {
            return true;
        }
        if (!strcmp(var, "$SNAPBASE")) {
            return true;
        }
        if (!strcmp(var, "$SNAPISOPAIR")) {
            return true;
        }
        if (!strcmp(var, "$SNAPMODE")) {
            return true;
        }
        if (!strcmp(var, "$SNAPSTYLE")) {
            return true;
        }
        if (!strcmp(var, "$SNAPUNIT")) {
            return true;
        }
        if (!strcmp(var, "$VIEWCTR")) {
            return true;
        }
        if (!strcmp(var, "$VIEWDIR")) {
            return true;
        }
        if (!strcmp(var, "$VIEWSIZE")) {
            return true;
        }
        return false;
    }

    return false;
}



/**
 * @returns the library version as int (4 bytes, each byte one version number).
 * e.g. if str = "2.0.2.0" getLibVersion returns 0x02000200
 */
int DL_Dxf::getLibVersion(const std::string& str) {
    size_t d[4];
    int idx = 0;
    //char v[4][5];
    std::string v[4];
    int ret = 0;

    for (size_t i=0; i<str.length() && idx<3; ++i) {
        if (str[i]=='.') {
            d[idx] = i;
            idx++;
        }
    }

    if (idx>=2) {
        d[3] = str.length();

        v[0] = str.substr(0, d[0]);
        v[1] = str.substr(d[0]+1, d[1]-d[0]-1);
        v[2] = str.substr(d[1]+1, d[2]-d[1]-1);
        if (idx>=3) {
            v[3] = str.substr(d[2]+1, d[3]-d[2]-1);
        }
        else {
            v[3] = "0";
        }

        ret = (atoi(v[0].c_str())<<(3*8)) +
              (atoi(v[1].c_str())<<(2*8)) +
              (atoi(v[2].c_str())<<(1*8)) +
              (atoi(v[3].c_str())<<(0*8));

        return ret;
    } else {
        std::cerr << "DL_Dxf::getLibVersion: invalid version number: " << str << "\n";
        return 0;
    }
}

/**
 * Converts the given string into a double or returns the given
 * default valud (def) if value is NULL or empty.
 */
//double DL_Dxf::toReal(const char* value, double def) {
//    if (value!=NULL && value[0] != '\0') {
//        printf("toReal: not empty: %s\n", value);
//        printf("toReal: val: %f\n", atof(value));
//        printf("toReal: 0: %d\n", value[0]);
//        printf("toReal: 1: %d\n", value[1]);
//        printf("toReal: 2: %d\n", value[2]);
//        double ret;
//        if (strchr(value, ',') != NULL) {
//            char* tmp = new char[strlen(value)+1];
//            strcpy(tmp, value);
//            DL_WriterA::strReplace(tmp, ',', '.');
//            ret = atof(tmp);
//            delete[] tmp;
//        }
//        else {
//            ret = atof(value);
//        }
//        return ret;
//    } else {
//        return def;
//    }
//}


/**
 * Some test routines.
 */
void DL_Dxf::test() {
    char* buf1;
    char* buf2;
    char* buf3;
    char* buf4;
    char* buf5;
    char* buf6;

    buf1 = new char[10];
    buf2 = new char[10];
    buf3 = new char[10];
    buf4 = new char[10];
    buf5 = new char[10];
    buf6 = new char[10];

    strcpy(buf1, "  10\n");
    strcpy(buf2, "10");
    strcpy(buf3, "10\n");
    strcpy(buf4, "  10 \n");
    strcpy(buf5, "  10 \r");
    strcpy(buf6, "\t10 \n");

    std::cout << "1 buf1: '" << buf1 << "'\n";
    stripWhiteSpace(&buf1);
    std::cout << "2 buf1: '" << buf1 << "'\n";
    //assert(!strcmp(buf1, "10"));

    std::cout << "1 buf2: '" << buf2 << "'\n";
    stripWhiteSpace(&buf2);
    std::cout << "2 buf2: '" << buf2 << "'\n";

    std::cout << "1 buf3: '" << buf3 << "'\n";
    stripWhiteSpace(&buf3);
    std::cout << "2 buf3: '" << buf3 << "'\n";

    std::cout << "1 buf4: '" << buf4 << "'\n";
    stripWhiteSpace(&buf4);
    std::cout << "2 buf4: '" << buf4 << "'\n";

    std::cout << "1 buf5: '" << buf5 << "'\n";
    stripWhiteSpace(&buf5);
    std::cout << "2 buf5: '" << buf5 << "'\n";

    std::cout << "1 buf6: '" << buf6 << "'\n";
    stripWhiteSpace(&buf6);
    std::cout << "2 buf6: '" << buf6 << "'\n";

}


