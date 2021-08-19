/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2015 José F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#include <cstdlib>
#include "drw_entities.h"
#include "intern/dxfreader.h"
#include "intern/dwgbuffer.h"
#include "intern/drw_dbg.h"

#define RESERVE( vector, size ) try { \
    vector.reserve(size); \
  } catch(const std::exception &e) { \
   /* QgsDebugMsg( QString( "allocation exception (size=%1; error=%2)" ).arg( size ).arg( e.what() ) );*/ \
    throw e; \
  }

//! Calculate arbitrary axis
/*!
*   Calculate arbitrary axis for apply extrusions
*  @author Rallaz
*/
void DRW_Entity::calculateAxis(const DRW_Coord &extPoint){
    //Follow the arbitrary DXF definitions for extrusion axes.
    if ( std::fabs( extPoint.x ) < 0.015625 && std::fabs( extPoint.y ) < 0.015625 ) {
        //If we get here, implement Ax = Wy x N where Wy is [0,1,0] per the DXF spec.
        //The cross product works out to Wy.y*N.z-Wy.z*N.y, Wy.z*N.x-Wy.x*N.z, Wy.x*N.y-Wy.y*N.x
        //Factoring in the fixed values for Wy gives N.z,0,-N.x
        extAxisX.x = extPoint.z;
        extAxisX.y = 0;
        extAxisX.z = -extPoint.x;
    } else {
        //Otherwise, implement Ax = Wz x N where Wz is [0,0,1] per the DXF spec.
        //The cross product works out to Wz.y*N.z-Wz.z*N.y, Wz.z*N.x-Wz.x*N.z, Wz.x*N.y-Wz.y*N.x
        //Factoring in the fixed values for Wz gives -N.y,N.x,0.
        extAxisX.x = -extPoint.y;
        extAxisX.y = extPoint.x;
        extAxisX.z = 0;
    }

    extAxisX.unitize();

    //Ay = N x Ax
    extAxisY.x = (extPoint.y * extAxisX.z) - (extAxisX.y * extPoint.z);
    extAxisY.y = (extPoint.z * extAxisX.x) - (extAxisX.z * extPoint.x);
    extAxisY.z = (extPoint.x * extAxisX.y) - (extAxisX.x * extPoint.y);

    extAxisY.unitize();
}

//! Extrude a point using arbitrary axis
/*!
*   apply extrusion in a point using arbitrary axis (previously calculated)
*  @author Rallaz
*/
void DRW_Entity::extrudePoint(const DRW_Coord &extPoint, DRW_Coord *point){
    double px, py, pz;
    px = (extAxisX.x*point->x)+(extAxisY.x*point->y)+(extPoint.x*point->z);
    py = (extAxisX.y*point->x)+(extAxisY.y*point->y)+(extPoint.y*point->z);
    pz = (extAxisX.z*point->x)+(extAxisY.z*point->y)+(extPoint.z*point->z);

    point->x = px;
    point->y = py;
    point->z = pz;
}

bool DRW_Entity::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 5:
        handle = reader->getHandleString();
        break;
    case 330:
        parentHandle = reader->getHandleString();
        break;
    case 8:
        layer = reader->getUtf8String();
        break;
    case 6:
        lineType = reader->getUtf8String();
        break;
    case 62:
        color = reader->getInt32();
        break;
    case 370:
        lWeight = DRW_LW_Conv::dxfInt2lineWidth(reader->getInt32());
        break;
    case 48:
        ltypeScale = reader->getDouble();
        break;
    case 60:
        visible = reader->getBool();
        break;
    case 420:
        color24 = reader->getInt32();
        break;
    case 430:
        colorName = reader->getString();
        break;
    case 440:
      transparency = reader->getInt32();
      break;
    case 67:
        space = static_cast<DRW::Space>(reader->getInt32());
        break;
    case 102:
        parseDxfGroups(code, reader);
        break;
    case 1000:
    case 1001:
    case 1002:
    case 1003:
    case 1004:
    case 1005:
		extData.push_back(std::make_shared<DRW_Variant>(code, reader->getString()));
        break;
    case 1010:
    case 1011:
    case 1012:
    case 1013:
		curr =std::make_shared<DRW_Variant>(code, DRW_Coord(reader->getDouble(), 0.0, 0.0));
        extData.push_back(curr);
        break;
    case 1020:
    case 1021:
    case 1022:
    case 1023:
        if (curr)
            curr->setCoordY(reader->getDouble());
        break;
    case 1030:
    case 1031:
    case 1032:
    case 1033:
        if (curr)
            curr->setCoordZ(reader->getDouble());
		//FIXME, why do we discard curr right after setting the its Z
//        curr=NULL;
        break;
    case 1040:
    case 1041:
    case 1042:
		extData.push_back(std::make_shared<DRW_Variant>(code, reader->getDouble() ));
        break;
    case 1070:
    case 1071:
		extData.push_back(std::make_shared<DRW_Variant>(code, reader->getInt32() ));
        break;
    default:
        break;
    }
    return true;
}

//parses dxf 102 groups to read entity
bool DRW_Entity::parseDxfGroups(int code, dxfReader *reader){
    std::list<DRW_Variant> ls;
    DRW_Variant curr;
    int nc;
    std::string appName= reader->getString();
    if (!appName.empty() && appName.at(0)== '{'){
        curr.addString( code, appName.substr( 1, static_cast< int >( appName.size() ) - 1 ) );
        ls.push_back(curr);
        while (code !=102 && appName.at(0)== '}'){
            reader->readRec(&nc);//RLZ curr.code = code or nc?
//            curr.code = code;
            //RLZ code == 330 || code == 360 OR nc == 330 || nc == 360 ?
            if (code == 330 || code == 360)
                curr.addInt(code, reader->getHandleString());//RLZ code or nc
            else {
                switch (reader->type) {
                case dxfReader::STRING:
                    curr.addString(code, reader->getString());//RLZ code or nc
                    break;
                case dxfReader::INT32:
                case dxfReader::INT64:
                    curr.addInt(code, reader->getInt32());//RLZ code or nc
                    break;
                case dxfReader::DOUBLE:
                    curr.addDouble(code, reader->getDouble());//RLZ code or nc
                    break;
                case dxfReader::BOOL:
                    curr.addInt(code, reader->getInt32());//RLZ code or nc
                    break;
                default:
                    break;
                }
            }
            ls.push_back(curr);
        }
    }

    appData.push_back(ls);
    return true;
}

bool DRW_Entity::parseDwg(DRW::Version version, dwgBuffer *buf, dwgBuffer* strBuf, duint32 bs){
    objSize=0;
    DRW_DBG("\n***************************** parsing entity *********************************************\n");
    oType = buf->getObjType(version);
    DRW_DBG("Object type: "); DRW_DBG(oType); DRW_DBG(", "); DRW_DBGH(oType);

    if (version > DRW::AC1014 && version < DRW::AC1024) {//2000 & 2004
        objSize = buf->getRawLong32();  //RL 32bits object size in bits
        DRW_DBG(" Object size: "); DRW_DBG(objSize); DRW_DBG("\n");
    }
    if (version > DRW::AC1021) {//2010+
        duint32 ms = buf->size();
        objSize = ms*8 - bs;
        DRW_DBG(" Object size: "); DRW_DBG(objSize); DRW_DBG("\n");
    }

    if (strBuf && version > DRW::AC1018) {//2007+
        strBuf->moveBitPos(objSize-1);
        DRW_DBG(" strBuf strbit pos 2007: "); DRW_DBG(strBuf->getPosition()); DRW_DBG(" strBuf bpos 2007: "); DRW_DBG(strBuf->getBitPos()); DRW_DBG("\n");
        if (strBuf->getBit() == 1){
            DRW_DBG("DRW_TableEntry::parseDwg string bit is 1\n");
            strBuf->moveBitPos(-17);
            duint16 strDataSize = strBuf->getRawShort16();
            DRW_DBG("\nDRW_TableEntry::parseDwg string strDataSize: "); DRW_DBGH(strDataSize); DRW_DBG("\n");
            if ( (strDataSize& 0x8000) == 0x8000){
                DRW_DBG("\nDRW_TableEntry::parseDwg string 0x8000 bit is set");
                strBuf->moveBitPos(-33);//RLZ pending to verify
                duint16 hiSize = strBuf->getRawShort16();
                strDataSize = ((strDataSize&0x7fff) | (hiSize<<15));
            }
            strBuf->moveBitPos( -strDataSize -16); //-14
            DRW_DBG("strBuf start strDataSize pos 2007: "); DRW_DBG(strBuf->getPosition()); DRW_DBG(" strBuf bpos 2007: "); DRW_DBG(strBuf->getBitPos()); DRW_DBG("\n");
        } else
            DRW_DBG("\nDRW_TableEntry::parseDwg string bit is 0");
        DRW_DBG("strBuf start pos 2007: "); DRW_DBG(strBuf->getPosition()); DRW_DBG(" strBuf bpos 2007: "); DRW_DBG(strBuf->getBitPos()); DRW_DBG("\n");
    }

    dwgHandle ho = buf->getHandle();
    handle = ho.ref;
    DRW_DBG("Entity Handle: "); DRW_DBGHL(ho.code, ho.size, ho.ref);
    dint16 extDataSize = buf->getBitShort(); //BS
    DRW_DBG(" ext data size: "); DRW_DBG(extDataSize);
    while (extDataSize>0 && buf->isGood()) {
        /* RLZ: TODO */
        dwgHandle ah = buf->getHandle();
        DRW_DBG("App Handle: "); DRW_DBGHL(ah.code, ah.size, ah.ref);
        duint8 *tmpExtData = new duint8[extDataSize];
        buf->getBytes(tmpExtData, extDataSize);
        dwgBuffer tmpExtDataBuf(tmpExtData, extDataSize, buf->decoder);

        duint8 dxfCode = tmpExtDataBuf.getRawChar8();
        DRW_DBG(" dxfCode: "); DRW_DBG(dxfCode);
        switch (dxfCode){
        case 0:{
            duint8 strLength = tmpExtDataBuf.getRawChar8();
            DRW_DBG(" strLength: "); DRW_DBG(strLength);
            duint16 cp = tmpExtDataBuf.getBERawShort16();
            DRW_DBG(" str codepage: "); DRW_DBG(cp);
            for (int i=0;i< strLength+1;i++) {//string length + null terminating char
                duint8 dxfChar = tmpExtDataBuf.getRawChar8();
                DRW_DBG(" dxfChar: "); DRW_DBG(dxfChar);
            }
            break;
        }
        default:
            /* RLZ: TODO */
            break;
        }
        delete[]tmpExtData;
        extDataSize = buf->getBitShort(); //BS
        DRW_DBG(" ext data size: "); DRW_DBG(extDataSize);
    } //end parsing extData (EED)
    duint8 graphFlag = buf->getBit(); //B
    DRW_DBG(" graphFlag: "); DRW_DBG(graphFlag); DRW_DBG("\n");
    if (graphFlag) {
        duint32 graphDataSize = buf->getRawLong32();  //RL 32bits
        DRW_DBG("graphData in bytes: "); DRW_DBG(graphDataSize); DRW_DBG("\n");
// RLZ: TODO
        //skip graphData bytes
        duint8 *tmpGraphData = new duint8[graphDataSize];
        buf->getBytes(tmpGraphData, graphDataSize);
        dwgBuffer tmpGraphDataBuf(tmpGraphData, graphDataSize, buf->decoder);
        DRW_DBG("graph data remaining bytes: "); DRW_DBG(tmpGraphDataBuf.numRemainingBytes()); DRW_DBG("\n");
        delete[]tmpGraphData;
    }
    if (version < DRW::AC1015) {//14-
        objSize = buf->getRawLong32();  //RL 32bits object size in bits
        DRW_DBG(" Object size in bits: "); DRW_DBG(objSize); DRW_DBG("\n");
    }

    duint8 entmode = buf->get2Bits(); //BB
    if (entmode == 0)
        ownerHandle= true;
//        entmode = 2;
    else if(entmode ==2)
        entmode = 0;
    space = static_cast< DRW::Space >( entmode ); //RLZ verify cast values
    DRW_DBG("entmode: "); DRW_DBG(entmode);
    numReactors = buf->getBitShort(); //BS
    DRW_DBG(", numReactors: "); DRW_DBG(numReactors);

    if (version < DRW::AC1015) {//14-
        if(buf->getBit()) {//is bylayer line type
            lineType = "BYLAYER";
            ltFlags = 0;
        } else {
            lineType = "";
            ltFlags = 3;
        }
        DRW_DBG(" lineType: "); DRW_DBG(lineType.c_str());
        DRW_DBG(" ltFlags: "); DRW_DBG(ltFlags);
    }
    if (version > DRW::AC1015) {//2004+
        xDictFlag = buf->getBit();
        DRW_DBG(" xDictFlag: "); DRW_DBG(xDictFlag); DRW_DBG("\n");
    }

    if (version > DRW::AC1024 || version < DRW::AC1018) {
        haveNextLinks = buf->getBit(); //aka nolinks //B
        DRW_DBG(", haveNextLinks (0 yes, 1 prev next): "); DRW_DBG(haveNextLinks); DRW_DBG("\n");
    } else {
        haveNextLinks = 1; //aka nolinks //B
        DRW_DBG(", haveNextLinks (forced): "); DRW_DBG(haveNextLinks); DRW_DBG("\n");
    }
//ENC color
    color = buf->getEnColor( version, color24, transparency ); //BS or CMC //OK for R14 or negate
    ltypeScale = buf->getBitDouble(); //BD
    DRW_DBG(" entity color: "); DRW_DBG(color);
    DRW_DBG(" ltScale: "); DRW_DBG(ltypeScale); DRW_DBG("\n");
    if (version > DRW::AC1014) {//2000+
        UTF8STRING plotStyleName;
        for (duint8 i = 0; i<2;++i) { //two flags in one
            plotFlags = buf->get2Bits(); //BB
            if (plotFlags == 1)
                plotStyleName = "byblock";
            else if (plotFlags == 2)
                plotStyleName = "continuous";
            else if (plotFlags == 0)
                plotStyleName = "bylayer";
            else //handle at end
                plotStyleName = "";
            if (i == 0) {
                ltFlags = plotFlags;
                lineType = plotStyleName; //RLZ: howto solve? if needed plotStyleName;
                DRW_DBG("ltFlags: "); DRW_DBG(ltFlags);
                DRW_DBG(" lineType: "); DRW_DBG(lineType.c_str());
            } else {
                DRW_DBG(", plotFlags: "); DRW_DBG(plotFlags);
            }
        }
    }
    if (version > DRW::AC1018) {//2007+
        materialFlag = buf->get2Bits(); //BB
        DRW_DBG("materialFlag: "); DRW_DBG(materialFlag);
        shadowFlag = buf->getRawChar8(); //RC
        DRW_DBG("shadowFlag: "); DRW_DBG(shadowFlag); DRW_DBG("\n");
    }
    if (version > DRW::AC1021) {//2010+
        duint8 visualFlags = buf->get2Bits(); //full & face visual style
        DRW_DBG("shadowFlag 2: "); DRW_DBG(visualFlags); DRW_DBG("\n");
        duint8 unk = buf->getBit(); //edge visual style
        DRW_DBG("unknown bit: "); DRW_DBG(unk); DRW_DBG("\n");
    }
    dint16 invisibleFlag = buf->getBitShort(); //BS
    DRW_DBG(" invisibleFlag: "); DRW_DBG(invisibleFlag);
    if (version > DRW::AC1014) {//2000+
        lWeight = DRW_LW_Conv::dwgInt2lineWidth( buf->getRawChar8() ); //RC
        DRW_DBG(" lwFlag (lWeight): "); DRW_DBG(lWeight); DRW_DBG("\n");
    }
    //Only in blocks ????????
//    if (version > DRW::AC1018) {//2007+
//        duint8 unk = buf->getBit();
//        DRW_DBG("unknown bit: "); DRW_DBG(unk); DRW_DBG("\n");
//    }
    return buf->isGood();
}

bool DRW_Entity::parseDwgEntHandle(DRW::Version version, dwgBuffer *buf){
    if (version > DRW::AC1018) {//2007+ skip string area
        buf->setPosition(objSize >> 3);
        buf->setBitPos(objSize & 7);
    }

    if(ownerHandle){//entity are in block or in a polyline
        dwgHandle ownerH = buf->getOffsetHandle(handle);
        DRW_DBG("owner (parent) Handle: "); DRW_DBGHL(ownerH.code, ownerH.size, ownerH.ref); DRW_DBG("\n");
        DRW_DBG("   Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
        parentHandle = ownerH.ref;
        DRW_DBG("Block (parent) Handle: "); DRW_DBGHL(ownerH.code, ownerH.size, parentHandle); DRW_DBG("\n");
    } else
        DRW_DBG("NO Block (parent) Handle\n");

    DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    for (int i=0; i< numReactors;++i) {
        dwgHandle reactorsH = buf->getHandle();
        DRW_DBG(" reactorsH control Handle: "); DRW_DBGHL(reactorsH.code, reactorsH.size, reactorsH.ref); DRW_DBG("\n");
    }
    if (xDictFlag !=1){//linetype in 2004 seems not have XDicObjH or NULL handle
        dwgHandle XDicObjH = buf->getHandle();
        DRW_DBG(" XDicObj control Handle: "); DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref); DRW_DBG("\n");
    }
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

    if (version < DRW::AC1015) {//R14-
        //layer handle
        layerH = buf->getOffsetHandle(handle);
        DRW_DBG(" layer Handle: "); DRW_DBGHL(layerH.code, layerH.size, layerH.ref); DRW_DBG("\n");
        DRW_DBG("   Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
        //lineType handle
        if(ltFlags == 3){
            lTypeH = buf->getOffsetHandle(handle);
            DRW_DBG("linetype Handle: "); DRW_DBGHL(lTypeH.code, lTypeH.size, lTypeH.ref); DRW_DBG("\n");
            DRW_DBG("   Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
        }
    }
    if (version < DRW::AC1018) {//2000+
        if (haveNextLinks == 0) {
            dwgHandle nextLinkH = buf->getOffsetHandle(handle);
            DRW_DBG(" prev nextLinkers Handle: "); DRW_DBGHL(nextLinkH.code, nextLinkH.size, nextLinkH.ref); DRW_DBG("\n");
            DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
            prevEntLink = nextLinkH.ref;
            nextLinkH = buf->getOffsetHandle(handle);
            DRW_DBG(" next nextLinkers Handle: "); DRW_DBGHL(nextLinkH.code, nextLinkH.size, nextLinkH.ref); DRW_DBG("\n");
            DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
            nextEntLink = nextLinkH.ref;
        } else {
            nextEntLink = handle+1;
            prevEntLink = handle-1;
        }
    }
    if (version > DRW::AC1015) {//2004+
        //Parses Bookcolor handle
    }
    if (version > DRW::AC1014) {//2000+
        //layer handle
        layerH = buf->getOffsetHandle(handle);
        DRW_DBG(" layer Handle: "); DRW_DBGHL(layerH.code, layerH.size, layerH.ref); DRW_DBG("\n");
        DRW_DBG("   Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
        //lineType handle
        if(ltFlags == 3){
            lTypeH = buf->getOffsetHandle(handle);
            DRW_DBG("linetype Handle: "); DRW_DBGHL(lTypeH.code, lTypeH.size, lTypeH.ref); DRW_DBG("\n");
            DRW_DBG("   Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
        }
    }
    if (version > DRW::AC1014) {//2000+
        if (version > DRW::AC1018) {//2007+
            if (materialFlag == 3) {
                dwgHandle materialH = buf->getOffsetHandle(handle);
                DRW_DBG(" material Handle: "); DRW_DBGHL(materialH.code, materialH.size, materialH.ref); DRW_DBG("\n");
                DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
            }
            if (shadowFlag == 3) {
                dwgHandle shadowH = buf->getOffsetHandle(handle);
                DRW_DBG(" shadow Handle: "); DRW_DBGHL(shadowH.code, shadowH.size, shadowH.ref); DRW_DBG("\n");
                DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
            }
        }
        if (plotFlags == 3) {
            dwgHandle plotStyleH = buf->getOffsetHandle(handle);
            DRW_DBG(" plot style Handle: "); DRW_DBGHL(plotStyleH.code, plotStyleH.size, plotStyleH.ref); DRW_DBG("\n");
            DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
        }
    }
    DRW_DBG("\n DRW_Entity::parseDwgEntHandle Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    return buf->isGood();
}

void DRW_Point::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 10:
        basePoint.x = reader->getDouble();
        break;
    case 20:
        basePoint.y = reader->getDouble();
        break;
    case 30:
        basePoint.z = reader->getDouble();
        break;
    case 39:
        thickness = reader->getDouble();
        break;
    case 210:
        haveExtrusion = true;
        extPoint.x = reader->getDouble();
        break;
    case 220:
        extPoint.y = reader->getDouble();
        break;
    case 230:
        extPoint.z = reader->getDouble();
        break;
    default:
        DRW_Entity::parseCode(code, reader);
        break;
    }
}

bool DRW_Point::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing point *********************************************\n");

    basePoint.x = buf->getBitDouble();
    basePoint.y = buf->getBitDouble();
    basePoint.z = buf->getBitDouble();
    DRW_DBG("point: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
    thickness = buf->getThickness(version > DRW::AC1014);//BD
    DRW_DBG("\nthickness: "); DRW_DBG(thickness);
    extPoint = buf->getExtrusion( version > DRW::AC1014, haveExtrusion );
    DRW_DBG(", Extrusion: "); DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);

    double x_axis = buf->getBitDouble();//BD
    DRW_DBG("\n  x_axis: ");DRW_DBG(x_axis);DRW_DBG("\n");
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
    //    RS crc;   //RS */

    return buf->isGood();
}

void DRW_Line::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 11:
        secPoint.x = reader->getDouble();
        break;
    case 21:
        secPoint.y = reader->getDouble();
        break;
    case 31:
        secPoint.z = reader->getDouble();
        break;
    default:
        DRW_Point::parseCode(code, reader);
        break;
    }
}

bool DRW_Line::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing line *********************************************\n");

    if (version < DRW::AC1015) {//14-
        basePoint.x = buf->getBitDouble();
        basePoint.y = buf->getBitDouble();
        basePoint.z = buf->getBitDouble();
        secPoint.x = buf->getBitDouble();
        secPoint.y = buf->getBitDouble();
        secPoint.z = buf->getBitDouble();
    }
    if (version > DRW::AC1014) {//2000+
        bool zIsZero = buf->getBit(); //B
        basePoint.x = buf->getRawDouble();//RD
        secPoint.x = buf->getDefaultDouble(basePoint.x);//DD
        basePoint.y = buf->getRawDouble();//RD
        secPoint.y = buf->getDefaultDouble(basePoint.y);//DD
        if (!zIsZero) {
            basePoint.z = buf->getRawDouble();//RD
            secPoint.z = buf->getDefaultDouble(basePoint.z);//DD
        }
    }
    DRW_DBG("start point: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
    DRW_DBG("\nend point: "); DRW_DBGPT(secPoint.x, secPoint.y, secPoint.z);
    thickness = buf->getThickness(version > DRW::AC1014);//BD
    DRW_DBG("\nthickness: "); DRW_DBG(thickness);
    extPoint = buf->getExtrusion( version > DRW::AC1014, haveExtrusion );
    DRW_DBG(", Extrusion: "); DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);DRW_DBG("\n");
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
//    RS crc;   //RS */
    return buf->isGood();
}

bool DRW_Ray::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing ray/xline *********************************************\n");
    basePoint.x = buf->getBitDouble();
    basePoint.y = buf->getBitDouble();
    basePoint.z = buf->getBitDouble();
    secPoint.x = buf->getBitDouble();
    secPoint.y = buf->getBitDouble();
    secPoint.z = buf->getBitDouble();
    DRW_DBG("start point: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
    DRW_DBG("\nvector: "); DRW_DBGPT(secPoint.x, secPoint.y, secPoint.z);
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
//    RS crc;   //RS */
    return buf->isGood();
}

void DRW_Circle::applyExtrusion(){
    if (haveExtrusion) {
        //NOTE: Commenting these out causes the the arcs being tested to be located
        //on the other side of the y axis (all x dimensions are negated).
        calculateAxis(extPoint);
        extrudePoint(extPoint, &basePoint);
    }
}

void DRW_Circle::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 40:
        radius = reader->getDouble();
        break;
    default:
        DRW_Point::parseCode(code, reader);
        break;
    }
}

bool DRW_Circle::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing circle *********************************************\n");

    basePoint.x = buf->getBitDouble();
    basePoint.y = buf->getBitDouble();
    basePoint.z = buf->getBitDouble();
    DRW_DBG("center: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
    radius = buf->getBitDouble();
    DRW_DBG("\nradius: "); DRW_DBG(radius);

    thickness = buf->getThickness(version > DRW::AC1014);
    DRW_DBG(" thickness: "); DRW_DBG(thickness);
    extPoint = buf->getExtrusion( version > DRW::AC1014, haveExtrusion );
    DRW_DBG("\nextrusion: "); DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z); DRW_DBG("\n");

    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
//    RS crc;   //RS */
    return buf->isGood();
}

void DRW_Arc::applyExtrusion(){
    DRW_Circle::applyExtrusion();

    if(haveExtrusion){
        // If the extrusion vector has a z value less than 0, the angles for the arc
        // have to be mirrored since DXF files use the right hand rule.
        // Note that the following code only handles the special case where there is a 2D
        // drawing with the z axis heading into the paper (or rather screen). An arbitrary
        // extrusion axis (with x and y values greater than 1/64) may still have issues.
        if ( std::fabs( extPoint.x ) < 0.015625 && std::fabs( extPoint.y ) < 0.015625 && extPoint.z < 0.0 ) {
            staangle=M_PI-staangle;
            endangle=M_PI-endangle;

            double temp = staangle;
            staangle=endangle;
            endangle=temp;
        }
    }
}

void DRW_Arc::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 50:
        staangle = reader->getDouble()/ ARAD;
        break;
    case 51:
        endangle = reader->getDouble()/ ARAD;
        break;
    default:
        DRW_Circle::parseCode(code, reader);
        break;
    }
}

bool DRW_Arc::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing circle arc *********************************************\n");

    basePoint.x = buf->getBitDouble();
    basePoint.y = buf->getBitDouble();
    basePoint.z = buf->getBitDouble();
    DRW_DBG("center point: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);

    radius = buf->getBitDouble();
    DRW_DBG("\nradius: "); DRW_DBG(radius);
    thickness = buf->getThickness(version > DRW::AC1014);
    DRW_DBG(" thickness: "); DRW_DBG(thickness);
    extPoint = buf->getExtrusion( version > DRW::AC1014, haveExtrusion );
    DRW_DBG("\nextrusion: "); DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
    staangle = buf->getBitDouble();
    DRW_DBG("\nstart angle: "); DRW_DBG(staangle);
    endangle = buf->getBitDouble();
    DRW_DBG(" end angle: "); DRW_DBG(endangle); DRW_DBG("\n");
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
    return buf->isGood();
}

void DRW_Ellipse::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 40:
        ratio = reader->getDouble();
        break;
    case 41:
        staparam = reader->getDouble();
        break;
    case 42:
        endparam = reader->getDouble();
        break;
    default:
        DRW_Line::parseCode(code, reader);
        break;
    }
}

void DRW_Ellipse::applyExtrusion(){
    if (haveExtrusion) {
        calculateAxis(extPoint);
        extrudePoint(extPoint, &secPoint);
        double initialparam = staparam;
        if (extPoint.z < 0.){
            staparam = M_PIx2 - endparam;
            endparam = M_PIx2 - initialparam;
        }
    }
}

//if ratio > 1 minor axis are greather than major axis, correct it
void DRW_Ellipse::correctAxis(){
    bool complete = false;
    if (staparam == endparam) {
        staparam = 0.0;
        endparam = M_PIx2; //2*M_PI;
        complete = true;
    }
    if (ratio > 1){
        if ( std::fabs(endparam - staparam - M_PIx2) < 1.0e-10)
            complete = true;
        double incX = secPoint.x;
        secPoint.x = -(secPoint.y * ratio);
        secPoint.y = incX*ratio;
        ratio = 1/ratio;
        if (!complete){
            if (staparam < M_PI_2)
                staparam += M_PI *2;
            if (endparam < M_PI_2)
                endparam += M_PI *2;
            endparam -= M_PI_2;
            staparam -= M_PI_2;
        }
    }
}

bool DRW_Ellipse::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing ellipse *********************************************\n");

    basePoint =buf->get3BitDouble();
    DRW_DBG("center: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
    secPoint =buf->get3BitDouble();
    DRW_DBG(", axis: "); DRW_DBGPT(secPoint.x, secPoint.y, secPoint.z); DRW_DBG("\n");
    extPoint =buf->get3BitDouble();
    DRW_DBG("Extrusion: "); DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
    ratio = buf->getBitDouble();//BD
    DRW_DBG("\nratio: "); DRW_DBG(ratio);
    staparam = buf->getBitDouble();//BD
    DRW_DBG(" start param: "); DRW_DBG(staparam);
    endparam = buf->getBitDouble();//BD
    DRW_DBG(" end param: "); DRW_DBG(endparam); DRW_DBG("\n");

    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
//    RS crc;   //RS */
    return buf->isGood();
}


//parts are the number of vertices to split the polyline, default 128
void DRW_Ellipse::toPolyline(DRW_Polyline *pol, int parts) const {
  double radMajor = std::hypot(secPoint.x, secPoint.y);
  double radMinor = radMajor * ratio;
  //calculate sin & cos of included angle
  double incAngle = std::atan2( secPoint.y, secPoint.x );
  double cosRot = std::cos( incAngle );
  double sinRot = std::sin( incAngle );

  incAngle = M_PIx2 / parts;
  double curAngle = staparam;
  double endAngle = endparam;
  if ( endAngle <= curAngle )
    endAngle += M_PIx2;

  while ( curAngle < endAngle )
  {
    double cosCurr = std::cos( curAngle );
    double sinCurr = std::sin( curAngle );
    double x = basePoint.x + cosCurr * cosRot * radMajor - sinCurr * sinRot * radMinor;
    double y = basePoint.y + cosCurr * sinRot * radMajor + sinCurr * cosRot * radMinor;
    pol->addVertex( DRW_Vertex( x, y, 0.0, 0.0 ) );
    curAngle += incAngle;
  }

  if ( std::fabs( endAngle - staparam - M_PIx2 ) < 1.0e-10 )
  {
    pol->flags = 1;
  }

  pol->layer = layer;
  pol->lineType = lineType;
  pol->color = color;
  pol->lWeight = lWeight;
  pol->extPoint = extPoint;
}

void DRW_Trace::applyExtrusion(){
    if (haveExtrusion) {
        calculateAxis(extPoint);
        extrudePoint(extPoint, &basePoint);
        extrudePoint(extPoint, &secPoint);
        extrudePoint(extPoint, &thirdPoint);
        extrudePoint(extPoint, &forthPoint);
    }
}

void DRW_Trace::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 12:
        thirdPoint.x = reader->getDouble();
        break;
    case 22:
        thirdPoint.y = reader->getDouble();
        break;
    case 32:
        thirdPoint.z = reader->getDouble();
        break;
    case 13:
        forthPoint.x = reader->getDouble();
        break;
    case 23:
        forthPoint.y = reader->getDouble();
        break;
    case 33:
        forthPoint.z = reader->getDouble();
        break;
    default:
        DRW_Line::parseCode(code, reader);
        break;
    }
}

bool DRW_Trace::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing Trace *********************************************\n");

    thickness = buf->getThickness(version>DRW::AC1014);
    basePoint.z = buf->getBitDouble();
    basePoint.x = buf->getRawDouble();
    basePoint.y = buf->getRawDouble();
    secPoint.x = buf->getRawDouble();
    secPoint.y = buf->getRawDouble();
    secPoint.z = basePoint.z;
    thirdPoint.x = buf->getRawDouble();
    thirdPoint.y = buf->getRawDouble();
    thirdPoint.z = basePoint.z;
    forthPoint.x = buf->getRawDouble();
    forthPoint.y = buf->getRawDouble();
    forthPoint.z = basePoint.z;
    extPoint = buf->getExtrusion(version>DRW::AC1014, haveExtrusion);

    DRW_DBG(" - base "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
    DRW_DBG("\n - sec "); DRW_DBGPT(secPoint.x, secPoint.y, secPoint.z);
    DRW_DBG("\n - third "); DRW_DBGPT(thirdPoint.x, thirdPoint.y, thirdPoint.z);
    DRW_DBG("\n - fourth "); DRW_DBGPT(forthPoint.x, forthPoint.y, forthPoint.z);
    DRW_DBG("\n - extrusion: "); DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
    DRW_DBG("\n - thickness: "); DRW_DBG(thickness); DRW_DBG("\n");

    /* Common Entity Handle Data */
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;

    /* CRC X --- */
    return buf->isGood();
}


void DRW_Solid::parseCode(int code, dxfReader *reader){
    DRW_Trace::parseCode(code, reader);
}

bool DRW_Solid::parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs){
    DRW_DBG("\n***************************** parsing Solid *********************************************\n");
    return DRW_Trace::parseDwg(v, buf, bs);
}

void DRW_3Dface::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 70:
        invisibleflag = reader->getInt32();
        break;
    default:
        DRW_Trace::parseCode(code, reader);
        break;
    }
}

bool DRW_3Dface::parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs){
    bool ret = DRW_Entity::parseDwg(v, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing 3Dface *********************************************\n");

    if ( v < DRW::AC1015 ) {// R13 & R14
        basePoint.x = buf->getBitDouble();
        basePoint.y = buf->getBitDouble();
        basePoint.z = buf->getBitDouble();
        secPoint.x = buf->getBitDouble();
        secPoint.y = buf->getBitDouble();
        secPoint.z = buf->getBitDouble();
        thirdPoint.x = buf->getBitDouble();
        thirdPoint.y = buf->getBitDouble();
        thirdPoint.z = buf->getBitDouble();
        forthPoint.x = buf->getBitDouble();
        forthPoint.y = buf->getBitDouble();
        forthPoint.z = buf->getBitDouble();
        invisibleflag = buf->getBitShort();
    } else { // 2000+
        bool has_no_flag = buf->getBit();
        bool z_is_zero = buf->getBit();
        basePoint.x = buf->getRawDouble();
        basePoint.y = buf->getRawDouble();
        basePoint.z = z_is_zero ? 0.0 : buf->getRawDouble();
        secPoint.x = buf->getDefaultDouble(basePoint.x);
        secPoint.y = buf->getDefaultDouble(basePoint.y);
        secPoint.z = buf->getDefaultDouble(basePoint.z);
        thirdPoint.x = buf->getDefaultDouble(secPoint.x);
        thirdPoint.y = buf->getDefaultDouble(secPoint.y);
        thirdPoint.z = buf->getDefaultDouble(secPoint.z);
        forthPoint.x = buf->getDefaultDouble(thirdPoint.x);
        forthPoint.y = buf->getDefaultDouble(thirdPoint.y);
        forthPoint.z = buf->getDefaultDouble(thirdPoint.z);
        invisibleflag = has_no_flag ? static_cast< int >( NoEdge ) : buf->getBitShort();
    }
    drw_assert(invisibleflag>=NoEdge);
    drw_assert(invisibleflag<=AllEdges);

    DRW_DBG(" - base "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z); DRW_DBG("\n");
    DRW_DBG(" - sec "); DRW_DBGPT(secPoint.x, secPoint.y, secPoint.z); DRW_DBG("\n");
    DRW_DBG(" - third "); DRW_DBGPT(thirdPoint.x, thirdPoint.y, thirdPoint.z); DRW_DBG("\n");
    DRW_DBG(" - fourth "); DRW_DBGPT(forthPoint.x, forthPoint.y, forthPoint.z); DRW_DBG("\n");
    DRW_DBG(" - Invisibility mask: "); DRW_DBG(invisibleflag); DRW_DBG("\n");

    /* Common Entity Handle Data */
    ret = DRW_Entity::parseDwgEntHandle(v, buf);
    if (!ret)
        return ret;
    return buf->isGood();
}

void DRW_Block::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 2:
        name = reader->getUtf8String();
        break;
    case 70:
        flags = reader->getInt32();
        break;
    default:
        DRW_Point::parseCode(code, reader);
        break;
    }
}

bool DRW_Block::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
    if (!ret)
        return ret;
    if (!isEnd){
        DRW_DBG("\n***************************** parsing block *********************************************\n");
        name = sBuf->getVariableText(version, false);
        DRW_DBG("Block name: "); DRW_DBG(name.c_str()); DRW_DBG("\n");
    } else {
        DRW_DBG("\n***************************** parsing end block *********************************************\n");
    }
    if (version > DRW::AC1018) {//2007+
        duint8 unk = buf->getBit();
        DRW_DBG("unknown bit: "); DRW_DBG(unk); DRW_DBG("\n");
    }
//    X handleAssoc;   //X
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
//    RS crc;   //RS */
    return buf->isGood();
}

void DRW_Insert::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 2:
        name = reader->getUtf8String();
        break;
    case 41:
        xscale = reader->getDouble();
        break;
    case 42:
        yscale = reader->getDouble();
        break;
    case 43:
        zscale = reader->getDouble();
        break;
    case 50:
        angle = reader->getDouble();
        angle = angle/ARAD; //convert to radian
        break;
    case 70:
        colcount = reader->getInt32();
        break;
    case 71:
        rowcount = reader->getInt32();
        break;
    case 44:
        colspace = reader->getDouble();
        break;
    case 45:
        rowspace = reader->getDouble();
        break;
    default:
        DRW_Point::parseCode(code, reader);
        break;
    }
}

bool DRW_Insert::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dint32 objCount = 0;
    bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n************************** parsing insert/minsert *****************************************\n");
    basePoint.x = buf->getBitDouble();
    basePoint.y = buf->getBitDouble();
    basePoint.z = buf->getBitDouble();
    DRW_DBG("insertion point: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z); DRW_DBG("\n");
    if (version < DRW::AC1015) {//14-
        xscale = buf->getBitDouble();
        yscale = buf->getBitDouble();
        zscale = buf->getBitDouble();
    } else {
        duint8 dataFlags = buf->get2Bits();
        if (dataFlags == 3){
            //none default value 1,1,1
        } else if (dataFlags == 1){ //x default value 1, y & z can be x value
            yscale = buf->getDefaultDouble(xscale);
            zscale = buf->getDefaultDouble(xscale);
        } else if (dataFlags == 2){
            xscale = buf->getRawDouble();
            yscale = zscale = xscale;
        } else { //dataFlags == 0
            xscale = buf->getRawDouble();
            yscale = buf->getDefaultDouble(xscale);
            zscale = buf->getDefaultDouble(xscale);
        }
    }
    angle = buf->getBitDouble();
    DRW_DBG("scale : "); DRW_DBGPT(xscale, yscale, zscale); DRW_DBG(", angle: "); DRW_DBG(angle);
    extPoint = buf->getExtrusion(false, haveExtrusion); //3BD R14 style
    DRW_DBG("\nextrusion: "); DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);

    bool hasAttrib = buf->getBit();
    DRW_DBG("   has Attrib: "); DRW_DBG(hasAttrib);

    if (hasAttrib && version > DRW::AC1015) {//2004+
        objCount = buf->getBitLong();
        DRW_UNUSED(objCount);
        DRW_DBG("   objCount: "); DRW_DBG(objCount); DRW_DBG("\n");
    }
    if (oType == 8) {//entity are minsert
        colcount = buf->getBitShort();
        rowcount = buf->getBitShort();
        colspace = buf->getBitDouble();
        rowspace = buf->getBitDouble();
    }
    DRW_DBG("   Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    blockRecH = buf->getHandle(); /* H 2 BLOCK HEADER (hard pointer) */
    DRW_DBG("BLOCK HEADER Handle: "); DRW_DBGHL(blockRecH.code, blockRecH.size, blockRecH.ref); DRW_DBG("\n");
    DRW_DBG("   Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

    /*attribs follows*/
    if (hasAttrib) {
        if (version < DRW::AC1018) {//2000-
            dwgHandle attH = buf->getHandle(); /* H 2 BLOCK HEADER (hard pointer) */
            DRW_DBG("first attrib Handle: "); DRW_DBGHL(attH.code, attH.size, attH.ref); DRW_DBG("\n");
            attH = buf->getHandle(); /* H 2 BLOCK HEADER (hard pointer) */
            DRW_DBG("second attrib Handle: "); DRW_DBGHL(attH.code, attH.size, attH.ref); DRW_DBG("\n");
        } else {
            for (duint8 i=0; i< objCount; ++i){
                dwgHandle attH = buf->getHandle(); /* H 2 BLOCK HEADER (hard pointer) */
                DRW_DBG("attrib Handle #"); DRW_DBG(i); DRW_DBG(": "); DRW_DBGHL(attH.code, attH.size, attH.ref); DRW_DBG("\n");
            }
        }
        seqendH = buf->getHandle(); /* H 2 BLOCK HEADER (hard pointer) */
        DRW_DBG("seqendH Handle: "); DRW_DBGHL(seqendH.code, seqendH.size, seqendH.ref); DRW_DBG("\n");
    }
    DRW_DBG("   Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

    if (!ret)
        return ret;
//    RS crc;   //RS */
    return buf->isGood();
}

void DRW_LWPolyline::applyExtrusion(){
    if (haveExtrusion) {
        calculateAxis(extPoint);
        for (unsigned int i=0; i<vertlist.size(); i++) {
			auto& vert = vertlist.at(i);
            DRW_Coord v(vert->x, vert->y, elevation);
            extrudePoint(extPoint, &v);
            vert->x = v.x;
            vert->y = v.y;
        }
    }
}

void DRW_LWPolyline::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 10: {
		vertex = std::make_shared<DRW_Vertex2D>();
        vertlist.push_back(vertex);
        vertex->x = reader->getDouble();
        break; }
    case 20:
		if(vertex)
            vertex->y = reader->getDouble();
        break;
    case 40:
		if(vertex)
            vertex->stawidth = reader->getDouble();
        break;
    case 41:
		if(vertex)
            vertex->endwidth = reader->getDouble();
        break;
    case 42:
		if(vertex)
            vertex->bulge = reader->getDouble();
        break;
    case 38:
        elevation = reader->getDouble();
        break;
    case 39:
        thickness = reader->getDouble();
        break;
    case 43:
        width = reader->getDouble();
        break;
    case 70:
        flags = reader->getInt32();
        break;
    case 90:
        vertexnum = reader->getInt32();
        RESERVE( vertlist, vertexnum );
        break;
    case 210:
        haveExtrusion = true;
        extPoint.x = reader->getDouble();
        break;
    case 220:
        extPoint.y = reader->getDouble();
        break;
    case 230:
        extPoint.z = reader->getDouble();
        break;
    default:
        DRW_Entity::parseCode(code, reader);
        break;
    }
}

bool DRW_LWPolyline::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing LWPolyline *******************************************\n");

    flags = buf->getBitShort();
    DRW_DBG("flags value: "); DRW_DBG(flags);
    if (flags & 4)
        width = buf->getBitDouble();
    if (flags & 8)
        elevation = buf->getBitDouble();
    if (flags & 2)
        thickness = buf->getBitDouble();
    if (flags & 1)
        extPoint = buf->getExtrusion(false, haveExtrusion);
    vertexnum = buf->getBitLong();
    RESERVE( vertlist, vertexnum );
    unsigned int bulgesnum = 0;
    if (flags & 16)
        bulgesnum = buf->getBitLong();
    int vertexIdCount = 0;
    if (version > DRW::AC1021) {//2010+
        if (flags & 1024)
            vertexIdCount = buf->getBitLong();
    }

    unsigned int widthsnum = 0;
    if (flags & 32)
        widthsnum = buf->getBitLong();
    DRW_DBG("\nvertex num: "); DRW_DBG(vertexnum); DRW_DBG(" bulges num: "); DRW_DBG(bulgesnum);
    DRW_DBG(" vertexIdCount: "); DRW_DBG(vertexIdCount); DRW_DBG(" widths num: "); DRW_DBG(widthsnum);
    //clear all bit except 128 = plinegen and set 1 to open/close //RLZ:verify plinegen & open
    //dxf: plinegen 128 & open 1
    flags = (flags & 512)? (flags | 1):(flags | 0);
    flags &= 129;
    DRW_DBG("end flags value: "); DRW_DBG(flags);

    if (vertexnum > 0) { //verify if is lwpol without vertex (empty)
        // add vertexs
		vertex = std::make_shared<DRW_Vertex2D>();
        vertex->x = buf->getRawDouble();
        vertex->y = buf->getRawDouble();
        vertlist.push_back(vertex);
		auto pv = vertex;
        for (int i = 1; i< vertexnum; i++){
			vertex = std::make_shared<DRW_Vertex2D>();
			if (version < DRW::AC1015) {//14-
                vertex->x = buf->getRawDouble();
                vertex->y = buf->getRawDouble();
            } else {
//                DRW_Vertex2D *pv = vertlist.back();
                vertex->x = buf->getDefaultDouble(pv->x);
                vertex->y = buf->getDefaultDouble(pv->y);
            }
            pv = vertex;
            vertlist.push_back(vertex);
        }
        //add bulges
        for (unsigned int i = 0; i < bulgesnum; i++){
            double bulge = buf->getBitDouble();
            if (vertlist.size()> i)
                vertlist.at(i)->bulge = bulge;
        }
        //add vertexId
        if (version > DRW::AC1021) {//2010+
            for (int i = 0; i < vertexIdCount; i++){
                dint32 vertexId = buf->getBitLong();
                //TODO implement vertexId, do not exist in dxf
                DRW_UNUSED(vertexId);
//                if (vertlist.size()< i)
//                    vertlist.at(i)->vertexId = vertexId;
            }
        }
        //add widths
        for (unsigned int i = 0; i < widthsnum; i++){
            double staW = buf->getBitDouble();
            double endW = buf->getBitDouble();
            if (vertlist.size()< i) {
                vertlist.at(i)->stawidth = staW;
                vertlist.at(i)->endwidth = endW;
            }
        }
    }
    if (DRW_DBGGL == DRW_dbg::Level::Debug){
        DRW_DBG("\nVertex list: ");
		for (auto& pv: vertlist) {
            DRW_DBG("\n   x: "); DRW_DBG(pv->x); DRW_DBG(" y: "); DRW_DBG(pv->y); DRW_DBG(" bulge: "); DRW_DBG(pv->bulge);
            DRW_DBG(" stawidth: "); DRW_DBG(pv->stawidth); DRW_DBG(" endwidth: "); DRW_DBG(pv->endwidth);
        }
    }

    DRW_DBG("\n");
    /* Common Entity Handle Data */
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
    /* CRC X --- */
    return buf->isGood();
}


void DRW_Text::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 40:
        height = reader->getDouble();
        break;
    case 41:
        widthscale = reader->getDouble();
        break;
    case 50:
        angle = reader->getDouble() / ARAD;
        break;
    case 51:
        oblique = reader->getDouble() / ARAD;
        break;
    case 71:
        textgen = reader->getInt32();
        break;
    case 72:
        alignH = static_cast< HAlign >( reader->getInt32() );
        break;
    case 73:
        alignV = static_cast< VAlign >( reader->getInt32() );
        break;
    case 1:
        text = reader->getUtf8String();
        break;
    case 7:
        style = reader->getUtf8String();
        break;
    default:
        DRW_Line::parseCode(code, reader);
        break;
    }
}

bool DRW_Text::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing text *********************************************\n");

 // DataFlags RC Used to determine presence of subsequent data, set to 0xFF for R14-
    duint8 data_flags = 0x00;
    if (version > DRW::AC1014) {//2000+
        data_flags = buf->getRawChar8(); /* DataFlags RC Used to determine presence of subsequent data */
        DRW_DBG("data_flags: "); DRW_DBG(data_flags); DRW_DBG("\n");
        if ( !(data_flags & 0x01) ) { /* Elevation RD --- present if !(DataFlags & 0x01) */
            basePoint.z = buf->getRawDouble();
        }
    } else {//14-
        basePoint.z = buf->getBitDouble(); /* Elevation BD --- */
    }
    basePoint.x = buf->getRawDouble(); /* Insertion pt 2RD 10 */
    basePoint.y = buf->getRawDouble();
    DRW_DBG("Insert point: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z); DRW_DBG("\n");
    if (version > DRW::AC1014) {//2000+
        if ( !(data_flags & 0x02) ) { /* Alignment pt 2DD 11 present if !(DataFlags & 0x02), use 10 & 20 values for 2 default values.*/
            secPoint.x = buf->getDefaultDouble(basePoint.x);
            secPoint.y = buf->getDefaultDouble(basePoint.y);
        } else {
            secPoint = basePoint;
        }
    } else {//14-
        secPoint.x = buf->getRawDouble();  /* Alignment pt 2RD 11 */
        secPoint.y = buf->getRawDouble();
    }
    secPoint.z = basePoint.z;
    DRW_DBG("Alignment: "); DRW_DBGPT(secPoint.x, secPoint.y, basePoint.z); DRW_DBG("\n");
    extPoint = buf->getExtrusion(version > DRW::AC1014, haveExtrusion);
    DRW_DBG("Extrusion: "); DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z); DRW_DBG("\n");
    thickness = buf->getThickness(version > DRW::AC1014); /* Thickness BD 39 */

    if (version > DRW::AC1014) {//2000+
        if ( !(data_flags & 0x04) ) { /* Oblique ang RD 51 present if !(DataFlags & 0x04) */
            oblique = buf->getRawDouble();
        }
        if ( !(data_flags & 0x08) ) { /* Rotation ang RD 50 present if !(DataFlags & 0x08) */
            angle = buf->getRawDouble();
        }
        height = buf->getRawDouble(); /* Height RD 40 */
        if ( !(data_flags & 0x10) ) { /* Width factor RD 41 present if !(DataFlags & 0x10) */
            widthscale = buf->getRawDouble();
        }
    } else {//14-
        oblique = buf->getBitDouble(); /* Oblique ang BD 51 */
        angle = buf->getBitDouble(); /* Rotation ang BD 50 */
        height = buf->getBitDouble(); /* Height BD 40 */
        widthscale = buf->getBitDouble(); /* Width factor BD 41 */
    }
    DRW_DBG("thickness: "); DRW_DBG(thickness); DRW_DBG(", Oblique ang: "); DRW_DBG(oblique); DRW_DBG(", Width: ");
    DRW_DBG(widthscale); DRW_DBG(", Rotation: "); DRW_DBG(angle); DRW_DBG(", height: "); DRW_DBG(height); DRW_DBG("\n");
    text = sBuf->getVariableText(version, false); /* Text value TV 1 */
    DRW_DBG("text string: "); DRW_DBG(text.c_str());DRW_DBG("\n");
    //textgen, alignH, alignV always present in R14-, data_flags set in initialisation
    if ( !(data_flags & 0x20) ) { /* Generation BS 71 present if !(DataFlags & 0x20) */
        textgen = buf->getBitShort();
        DRW_DBG("textgen: "); DRW_DBG(textgen);
    }
    if ( !(data_flags & 0x40) ) { /* Horiz align. BS 72 present if !(DataFlags & 0x40) */
        alignH = static_cast< HAlign >( buf->getBitShort() );
        DRW_DBG(", alignH: "); DRW_DBG(alignH);
    }
    if ( !(data_flags & 0x80) ) { /* Vert align. BS 73 present if !(DataFlags & 0x80) */
        alignV = static_cast< VAlign >( buf->getBitShort() );
        DRW_DBG(", alignV: "); DRW_DBG(alignV);
    }
    DRW_DBG("\n");

    /* Common Entity Handle Data */
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;

    styleH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
    DRW_DBG("text style Handle: "); DRW_DBGHL(styleH.code, styleH.size, styleH.ref); DRW_DBG("\n");

    /* CRC X --- */
    return buf->isGood();
}

void DRW_MText::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 1:
        text += reader->getString();
        text = reader->toUtf8String(text);
        break;
    case 11:
        hasXAxisVec = true;
        DRW_Text::parseCode(code, reader);
        break;
    case 3:
        text += reader->getString();
        break;
    case 44:
        interlin = reader->getDouble();
        break;
    default:
        DRW_Text::parseCode(code, reader);
        break;
    }
}

bool DRW_MText::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing mtext *********************************************\n");

    basePoint = buf->get3BitDouble(); /* Insertion pt 3BD 10 - First picked point. */
    DRW_DBG("Insertion: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z); DRW_DBG("\n");
    extPoint = buf->get3BitDouble(); /* Extrusion 3BD 210 Undocumented; */
    secPoint = buf->get3BitDouble(); /* X-axis dir 3BD 11 */
    hasXAxisVec = true;
    updateAngle();
    widthscale = buf->getBitDouble(); /* Rect width BD 41 */
    if (version > DRW::AC1018) {//2007+
        /* Rect height BD 46 Reference rectangle height. */
        /** @todo */buf->getBitDouble();
    }
    height = buf->getBitDouble();/* Text height BD 40 Undocumented */
    textgen = buf->getBitShort(); /* Attachment BS 71 Similar to justification; */
    /* Drawing dir BS 72 Left to right, etc.; see DXF doc */
    dint16 draw_dir = buf->getBitShort();
    DRW_UNUSED(draw_dir);
    /* Extents ht BD Undocumented and not present in DXF or entget */
    double ext_ht = buf->getBitDouble();
    DRW_UNUSED(ext_ht);
    /* Extents wid BD Undocumented and not present in DXF or entget The extents
    rectangle, when rotated the same as the text, fits the actual text image on
    the screen (although we've seen it include an extra row of text in height). */
    double ext_wid = buf->getBitDouble();
    DRW_UNUSED(ext_wid);
    /* Text TV 1 All text in one long string (without '\n's 3 for line wrapping).
    ACAD seems to add braces ({ }) and backslash-P's to indicate paragraphs
    based on the "\r\n"'s found in the imported file. But, all the text is in
    this one long string -- not broken into 1- and 3-groups as in DXF and
    entget. ACAD's entget breaks this string into 250-char pieces (not 255 as
    doc'd) – even if it's mid-word. The 1-group always gets the tag end;
    therefore, the 3's are always 250 chars long. */
    text = sBuf->getVariableText(version, false); /* Text value TV 1 */
    if (version > DRW::AC1014) {//2000+
        buf->getBitShort();/* Linespacing Style BS 73 */
        interlin = buf->getBitDouble();/* Linespacing Factor BD 44 */
        buf->getBit();/* Unknown bit B */
    }
    if (version > DRW::AC1015) {//2004+
        /* Background flags BL 0 = no background, 1 = background fill, 2 =background
        fill with drawing fill color. */
        dint32 bk_flags = buf->getBitLong(); /** @todo add to DRW_MText */
        if ( bk_flags == 1 ) {
            /* Background scale factor BL Present if background flags = 1, default = 1.5*/
            buf->getBitLong();
            /* Background color CMC Present if background flags = 1 */
            buf->getCmColor(version); //RLZ: warning CMC or ENC
            /** @todo buf->getCMC */
            /* Background transparency BL Present if background flags = 1 */
            buf->getBitLong();
        }
    }

    /* Common Entity Handle Data */
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;

    styleH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
    DRW_DBG("text style Handle: "); DRW_DBG(styleH.code); DRW_DBG(".");
    DRW_DBG(styleH.size); DRW_DBG("."); DRW_DBG(styleH.ref); DRW_DBG("\n");

    /* CRC X --- */
    return buf->isGood();
}

void DRW_MText::updateAngle() {
    if (hasXAxisVec) {
       angle = std::atan2( secPoint.y, secPoint.x ) * ARAD;
    }
}

void DRW_Polyline::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 70:
        flags = reader->getInt32();
        break;
    case 40:
        defstawidth = reader->getDouble();
        break;
    case 41:
        defendwidth = reader->getDouble();
        break;
    case 71:
        vertexcount = reader->getInt32();
        break;
    case 72:
        facecount = reader->getInt32();
        break;
    case 73:
        smoothM = reader->getInt32();
        break;
    case 74:
        smoothN = reader->getInt32();
        break;
    case 75:
        curvetype = reader->getInt32();
        break;
    default:
        DRW_Point::parseCode(code, reader);
        break;
    }
}

//0x0F polyline 2D bit 4(8) & 5(16) NOT set
//0x10 polyline 3D bit 4(8) set
//0x1D PFACE bit 5(16) set
bool DRW_Polyline::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing polyline *********************************************\n");

    dint32 ooCount = 0;
    if (oType == 0x0F) { //pline 2D
        flags = buf->getBitShort();
        DRW_DBG("flags value: "); DRW_DBG(flags);
        curvetype = buf->getBitShort();
        defstawidth = buf->getBitDouble();
        defendwidth = buf->getBitDouble();
        thickness = buf->getThickness(version > DRW::AC1014);
        basePoint = DRW_Coord(0,0,buf->getBitDouble());
        extPoint = buf->getExtrusion( version > DRW::AC1014, haveExtrusion );
    } else if (oType == 0x10) { //pline 3D
        duint8 tmpFlag = buf->getRawChar8();
        DRW_DBG("flags 1 value: "); DRW_DBG(tmpFlag);
        if (tmpFlag & 1)
            curvetype = 5;
        else if (tmpFlag & 2)
            curvetype = 6;
        if (tmpFlag & 3) {
            curvetype = 8;
            flags |= 4;
        }
        tmpFlag = buf->getRawChar8();
        if (tmpFlag & 1)
            flags |= 1;
        flags |= 8; //indicate 3DPOL
        DRW_DBG("flags 2 value: "); DRW_DBG(tmpFlag);
    } else if (oType == 0x1D) { //PFACE
        flags = 64;
        vertexcount = buf->getBitShort();
        DRW_DBG("vertex count: "); DRW_DBG(vertexcount);
        facecount = buf->getBitShort();
        DRW_DBG("face count: "); DRW_DBG(facecount);
        DRW_DBG("flags value: "); DRW_DBG(flags);
    }
    if (version > DRW::AC1015){ //2004+
        ooCount = buf->getBitLong();
    }

    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;

    if (version < DRW::AC1018){ //2000-
        dwgHandle objectH = buf->getOffsetHandle(handle);
        firstEH = objectH.ref;
        DRW_DBG(" first Vertex Handle: "); DRW_DBGHL(objectH.code, objectH.size, objectH.ref); DRW_DBG("\n");
        objectH = buf->getOffsetHandle(handle);
        lastEH = objectH.ref;
        DRW_DBG(" last Vertex Handle: "); DRW_DBGHL(objectH.code, objectH.size, objectH.ref); DRW_DBG("\n");
        DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    } else {
        for (dint32 i = 0; i < ooCount; ++i){
                dwgHandle objectH = buf->getOffsetHandle(handle);
                handlesList.push_back (objectH.ref);
                DRW_DBG(" Vertex Handle: "); DRW_DBGHL(objectH.code, objectH.size, objectH.ref); DRW_DBG("\n");
                DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
        }
    }
    seqEndH = buf->getOffsetHandle(handle);
    DRW_DBG(" SEQEND Handle: "); DRW_DBGHL(seqEndH.code, seqEndH.size, seqEndH.ref); DRW_DBG("\n");
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

//    RS crc;   //RS */
    return buf->isGood();
}

void DRW_Vertex::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 70:
        flags = reader->getInt32();
        break;
    case 40:
        stawidth = reader->getDouble();
        break;
    case 41:
        endwidth = reader->getDouble();
        break;
    case 42:
        bulge = reader->getDouble();
        break;
    case 50:
        tgdir = reader->getDouble();
        break;
    case 71:
        vindex1 = reader->getInt32();
        break;
    case 72:
        vindex2 = reader->getInt32();
        break;
    case 73:
        vindex3 = reader->getInt32();
        break;
    case 74:
        vindex4 = reader->getInt32();
        break;
    case 91:
        identifier = reader->getInt32();
        break;
    default:
        DRW_Point::parseCode(code, reader);
        break;
    }
}

//0x0A vertex 2D
//0x0B vertex 3D
//0x0C MESH
//0x0D PFACE
//0x0E PFACE FACE
bool DRW_Vertex::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs, double el){
    bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing pline Vertex *********************************************\n");

    if (oType == 0x0A) { //pline 2D, needed example
        flags = buf->getRawChar8(); //RLZ: EC  unknown type
        DRW_DBG("flags value: "); DRW_DBG(flags);
        basePoint = buf->get3BitDouble();
        basePoint.z = el;
        DRW_DBG("basePoint: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
        stawidth = buf->getBitDouble();
        if (stawidth < 0)
            endwidth = stawidth = std::fabs(stawidth);
        else
            endwidth = buf->getBitDouble();
        bulge = buf->getBitDouble();
        if (version > DRW::AC1021) { //2010+
            DRW_DBG("Vertex ID: "); DRW_DBG(buf->getBitLong());
        }
        tgdir = buf->getBitDouble();
    } else if (oType == 0x0B || oType == 0x0C || oType == 0x0D) { //PFACE
        flags = buf->getRawChar8(); //RLZ: EC  unknown type
        DRW_DBG("flags value: "); DRW_DBG(flags);
        basePoint = buf->get3BitDouble();
        DRW_DBG("basePoint: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
    } else if (oType == 0x0E) { //PFACE FACE
        vindex1 = buf->getBitShort();
        vindex2 = buf->getBitShort();
        vindex3 = buf->getBitShort();
        vindex4 = buf->getBitShort();
    }

    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
//    RS crc;   //RS */
    return buf->isGood();
}

void DRW_Hatch::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 2:
        name = reader->getUtf8String();
        break;
    case 70:
        solid = reader->getInt32();
        break;
    case 71:
        associative = reader->getInt32();
        break;
    case 72:        /*edge type*/
        if (ispol){ //if is polyline is a as_bulge flag
            break;
        } else if (reader->getInt32() == 1){ //line
            addLine();
        } else if (reader->getInt32() == 2){ //arc
            addArc();
        } else if (reader->getInt32() == 3){ //elliptic arc
            addEllipse();
        } else if (reader->getInt32() == 4){ //spline
            addSpline();
        }
        break;
    case 10:
        if (pt) pt->basePoint.x = reader->getDouble();
        else if (pline) {
            plvert = pline->addVertex();
            plvert->x = reader->getDouble();
        }
        break;
    case 20:
        if (pt) pt->basePoint.y = reader->getDouble();
        else if (plvert) plvert ->y = reader->getDouble();
        break;
    case 11:
        if (line) line->secPoint.x = reader->getDouble();
        else if (ellipse) ellipse->secPoint.x = reader->getDouble();
        break;
    case 21:
        if (line) line->secPoint.y = reader->getDouble();
        else if (ellipse) ellipse->secPoint.y = reader->getDouble();
        break;
    case 40:
        if (arc) arc->radius = reader->getDouble();
        else if (ellipse) ellipse->ratio = reader->getDouble();
        break;
    case 41:
        scale = reader->getDouble();
        break;
    case 42:
        if (plvert) plvert ->bulge = reader->getDouble();
        break;
    case 50:
        if (arc) arc->staangle = reader->getDouble()/ARAD;
        else if (ellipse) ellipse->staparam = reader->getDouble()/ARAD;
        break;
    case 51:
        if (arc) arc->endangle = reader->getDouble()/ARAD;
        else if (ellipse) ellipse->endparam = reader->getDouble()/ARAD;
        break;
    case 52:
        angle = reader->getDouble() / ARAD;
        break;
    case 73:
        if (arc) arc->isccw = reader->getInt32();
        else if (pline) pline->flags = reader->getInt32();
        break;
    case 75:
        hstyle = reader->getInt32();
        break;
    case 76:
        hpattern = reader->getInt32();
        break;
    case 77:
        doubleflag = reader->getInt32();
        break;
    case 78:
        deflines = reader->getInt32();
        break;
    case 91:
        loopsnum = reader->getInt32();
        RESERVE( looplist, loopsnum );
        break;
    case 92:
	loop = std::make_shared<DRW_HatchLoop>(reader->getInt32());
        looplist.push_back(loop);
        if (reader->getInt32() & 2) {
            ispol = true;
            clearEntities();
            pline = std::make_shared<DRW_LWPolyline>();
            loop->objlist.push_back(pline);
        } else ispol = false;
        break;
    case 93:
        if (pline) pline->vertexnum = reader->getInt32();
        else loop->numedges = reader->getInt32();//aqui reserve
        break;
    case 98: //seed points ??
        clearEntities();
        break;
    default:
        DRW_Point::parseCode(code, reader);
        break;
    }
}

bool DRW_Hatch::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    duint32 totalBoundItems = 0;
    bool hasPixelSize = false;

    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing hatch *********************************************\n");

    //Gradient data, RLZ: is ok or if grad > 0 continue read ?
    if (version > DRW::AC1015) { //2004+
        dint32 isGradient = buf->getBitLong();
        DRW_DBG("is Gradient: "); DRW_DBG(isGradient);
        dint32 res = buf->getBitLong();
        DRW_DBG(" reserved: "); DRW_DBG(res);
        double gradAngle = buf->getBitDouble();
        DRW_DBG(" Gradient angle: "); DRW_DBG(gradAngle);
        double gradShift = buf->getBitDouble();
        DRW_DBG(" Gradient shift: "); DRW_DBG(gradShift);
        dint32 singleCol = buf->getBitLong();
        DRW_DBG("\nsingle color Grad: "); DRW_DBG(singleCol);
        double gradTint = buf->getBitDouble();
        DRW_DBG(" Gradient tint: "); DRW_DBG(gradTint);
        dint32 numCol = buf->getBitLong();
        DRW_DBG(" num colors: "); DRW_DBG(numCol);
        for (dint32 i = 0 ; i < numCol; ++i){
            double unkDouble = buf->getBitDouble();
            DRW_DBG("\nunkDouble: "); DRW_DBG(unkDouble);
            duint16 unkShort = buf->getBitShort();
            DRW_DBG(" unkShort: "); DRW_DBG(unkShort);
            dint32 rgbCol = buf->getBitLong();
            DRW_DBG(" rgb color: "); DRW_DBG(rgbCol);
            duint8 ignCol = buf->getRawChar8();
            DRW_DBG(" ignored color: "); DRW_DBG(ignCol);
        }
        UTF8STRING gradName = sBuf->getVariableText(version, false);
        DRW_DBG("\ngradient name: "); DRW_DBG(gradName.c_str()); DRW_DBG("\n");
    }
    basePoint.z = buf->getBitDouble();
    extPoint = buf->get3BitDouble();
    DRW_DBG("base point: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
    DRW_DBG("\nextrusion: "); DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
    name = sBuf->getVariableText(version, false);
    DRW_DBG("\nhatch pattern name: "); DRW_DBG(name.c_str()); DRW_DBG("\n");
    solid = buf->getBit();
    associative = buf->getBit();
    loopsnum = buf->getBitLong();

    //read loops
    for (dint32 i = 0 ; i < loopsnum; ++i){
        loop = std::make_shared<DRW_HatchLoop>(buf->getBitLong());
        hasPixelSize |= loop->type & 4;
        if (!(loop->type & 2)){ //Not polyline
            dint32 numPathSeg = buf->getBitLong();
            for (dint32 j = 0; j<numPathSeg && buf->isGood();++j){
                duint8 typePath = buf->getRawChar8();
                if (typePath == 1){ //line
                    addLine();
                    line->basePoint = buf->get2RawDouble();
                    line->secPoint = buf->get2RawDouble();
                } else if (typePath == 2){ //circle arc
                    addArc();
                    arc->basePoint = buf->get2RawDouble();
                    arc->radius = buf->getBitDouble();
                    arc->staangle = buf->getBitDouble();
                    arc->endangle = buf->getBitDouble();
                    arc->isccw = buf->getBit();
                } else if (typePath == 3){ //ellipse arc
                    addEllipse();
                    ellipse->basePoint = buf->get2RawDouble();
                    ellipse->secPoint = buf->get2RawDouble();
                    ellipse->ratio = buf->getBitDouble();
                    ellipse->staparam = buf->getBitDouble();
                    ellipse->endparam = buf->getBitDouble();
                    ellipse->isccw = buf->getBit();
                } else if (typePath == 4){ //spline
                    addSpline();
                    spline->degree = buf->getBitLong();
                    bool isRational = buf->getBit();
                    spline->flags |= (isRational << 2); //rational
                    spline->flags |= (buf->getBit() << 1); //periodic
                    spline->nknots = buf->getBitLong();
                    spline->ncontrol = buf->getBitLong();
                    RESERVE( spline->knotslist, spline->nknots );
                    for (dint32 j = 0; j < spline->nknots && buf->isGood();++j){
                        spline->knotslist.push_back( buf->getBitDouble() );
                    }

                    if ( !buf->isGood() )
                    {
                      DRW_DBG( "NOT GOOD at ");DRW_DBG(j);DRW_DBG( "!  degree:");DRW_DBG(spline->degree);
                      DRW_DBG(" flags:");DRW_DBGH(spline->flags);DRW_DBG(" nknots:");DRW_DBG(spline->nknots);
                      DRW_DBG(" ncontrol:");DRW_DBG( spline->ncontrol ); DRW_DBG("\n" );
                    }

                    RESERVE( spline->controllist, spline->ncontrol );
                    for (dint32 j = 0; j < spline->ncontrol && buf->isGood();++j){
                        std::shared_ptr<DRW_Coord> crd = std::make_shared<DRW_Coord>(buf->get2RawDouble());
                        if (isRational)
                            crd->z = buf->getBitDouble(); //RLZ: investigate how store weight
                        spline->controllist.push_back(crd);
                    }
                    if (version > DRW::AC1021) { //2010+
                        spline->nfit = buf->getBitLong();
                        DRW_DBG("   nfit:");DRW_DBG(spline->nfit);DRW_DBG("\n");
                        RESERVE( spline->fitlist, spline->nfit );
                        for (dint32 j = 0; j < spline->nfit && buf->isGood();++j){
                            std::shared_ptr<DRW_Coord> crd = std::make_shared<DRW_Coord>(buf->get2RawDouble());
                            spline->fitlist.push_back(crd);
                        }
                        spline->tgStart = buf->get2RawDouble();
                        spline->tgEnd = buf->get2RawDouble();
                    }
                }
            }
        } else { //end not pline, start polyline
			pline = std::make_shared<DRW_LWPolyline>();
            bool hasBulges = buf->getBit();
            pline->flags = buf->getBit();//closed bit
            dint32 numVert = buf->getBitLong();
            for (dint32 j = 0; j<numVert && buf->isGood();++j){
                DRW_Vertex2D v;
                v.x = buf->getRawDouble();
                v.y = buf->getRawDouble();
                if (hasBulges)
                    v.bulge = buf->getBitDouble();
                pline->addVertex(v);
            }
            loop->objlist.push_back(pline);
        }//end polyline
        loop->update();
        looplist.push_back(loop);
        totalBoundItems += buf->getBitLong();
        DRW_DBG(" totalBoundItems: "); DRW_DBG(totalBoundItems);
    } //end read loops

    hstyle = buf->getBitShort();
    hpattern = buf->getBitShort();
    DRW_DBG("\nhatch style: "); DRW_DBG(hstyle); DRW_DBG(" pattern type"); DRW_DBG(hpattern);
    if (!solid){
        angle = buf->getBitDouble();
        scale = buf->getBitDouble();
        doubleflag = buf->getBit();
        deflines = buf->getBitShort();
        for (dint32 i = 0 ; i < deflines && buf->isGood(); ++i){
            DRW_Coord ptL, offL;
            double angleL = buf->getBitDouble();
            ptL.x = buf->getBitDouble();
            ptL.y = buf->getBitDouble();
            offL.x = buf->getBitDouble();
            offL.y = buf->getBitDouble();
            duint16 numDashL = buf->getBitShort();
            DRW_DBG("\ndef line: "); DRW_DBG(angleL); DRW_DBG(","); DRW_DBG(ptL.x); DRW_DBG(","); DRW_DBG(ptL.y);
            DRW_DBG(","); DRW_DBG(offL.x); DRW_DBG(","); DRW_DBG(offL.y); DRW_DBG(","); DRW_DBG(angleL);
            for (duint16 i = 0 ; i < numDashL; ++i){
                double lengthL = buf->getBitDouble();
                DRW_DBG(","); DRW_DBG(lengthL);
            }
        }//end deflines
    } //end not solid

    if (hasPixelSize){
        ddouble64 pixsize = buf->getBitDouble();
        DRW_DBG("\npixel size: "); DRW_DBG(pixsize);
    }
    dint32 numSeedPoints = buf->getBitLong();
    DRW_DBG("\nnum Seed Points  "); DRW_DBG(numSeedPoints);
    //read Seed Points
    DRW_Coord seedPt;
    for (dint32 i = 0 ; i < numSeedPoints && buf->isGood(); ++i){
        seedPt.x = buf->getRawDouble();
        seedPt.y = buf->getRawDouble();
        DRW_DBG("\n  "); DRW_DBG(seedPt.x); DRW_DBG(","); DRW_DBG(seedPt.y);
    }

    DRW_DBG("\n");
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

    for (duint32 i = 0 ; i < totalBoundItems && buf->isGood(); ++i){
        dwgHandle biH = buf->getHandle();
        DRW_DBG("Boundary Items Handle: "); DRW_DBGHL(biH.code, biH.size, biH.ref);
    }
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
//    RS crc;   //RS */
    return buf->isGood();
}

void DRW_Spline::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 210:
        normalVec.x = reader->getDouble();
        break;
    case 220:
        normalVec.y = reader->getDouble();
        break;
    case 230:
        normalVec.z = reader->getDouble();
        break;
    case 12:
        tgStart.x = reader->getDouble();
        break;
    case 22:
        tgStart.y = reader->getDouble();
        break;
    case 32:
        tgStart.z = reader->getDouble();
        break;
    case 13:
        tgEnd.x = reader->getDouble();
        break;
    case 23:
        tgEnd.y = reader->getDouble();
        break;
    case 33:
        tgEnd.z = reader->getDouble();
        break;
    case 70:
        flags = reader->getInt32();
        break;
    case 71:
        degree = reader->getInt32();
        break;
    case 72:
        nknots = reader->getInt32();
        break;
    case 73:
        ncontrol = reader->getInt32();
        break;
    case 74:
        nfit = reader->getInt32();
        break;
    case 42:
        tolknot = reader->getDouble();
        break;
    case 43:
        tolcontrol = reader->getDouble();
        break;
    case 44:
        tolfit = reader->getDouble();
        break;
    case 10: {
        controlpoint = std::make_shared<DRW_Coord>();
        controllist.push_back(controlpoint);
        controlpoint->x = reader->getDouble();
        break; }
    case 20:
        if(controlpoint)
            controlpoint->y = reader->getDouble();
        break;
    case 30:
        if(controlpoint)
            controlpoint->z = reader->getDouble();
        break;
    case 11: {
        fitpoint = std::make_shared<DRW_Coord>();
        fitlist.push_back(fitpoint);
        fitpoint->x = reader->getDouble();
        break; }
    case 21:
        if(fitpoint)
            fitpoint->y = reader->getDouble();
        break;
    case 31:
        if(fitpoint)
            fitpoint->z = reader->getDouble();
        break;
    case 40:
        knotslist.push_back(reader->getDouble());
        break;
    case 41:
        weightlist.push_back(reader->getDouble());
        break;
    default:
        DRW_Entity::parseCode(code, reader);
        break;
    }
}

bool DRW_Spline::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing spline *********************************************\n");
    duint8 weight = 0; // RLZ ??? flags, weight, code 70, bit 4 (16)

    dint32 scenario = buf->getBitLong();
    DRW_DBG("scenario: "); DRW_DBG(scenario);
    if (version > DRW::AC1024) {
        dint32 splFlag1 = buf->getBitLong();
        if (splFlag1 & 1)
            scenario = 2;
        dint32 knotParam = buf->getBitLong();
        DRW_DBG("2013 splFlag1: "); DRW_DBG(splFlag1); DRW_DBG(" 2013 knotParam: ");
        DRW_DBG(knotParam);
//        DRW_DBG("unk bit: "); DRW_DBG(buf->getBit());
    }
    degree = buf->getBitLong(); //RLZ: code 71, verify with dxf
    DRW_DBG(" degree: "); DRW_DBG(degree); DRW_DBG("\n");
    if (scenario == 2) {
        flags = 8;//scenario 2 = not rational & planar
        tolfit = buf->getBitDouble();//BD
        DRW_DBG("flags: "); DRW_DBG(flags); DRW_DBG(" tolfit: "); DRW_DBG(tolfit);
        tgStart =buf->get3BitDouble();
        DRW_DBG(" Start Tangent: "); DRW_DBGPT(tgStart.x, tgStart.y, tgStart.z);
        tgEnd =buf->get3BitDouble();
        DRW_DBG("\nEnd Tangent: "); DRW_DBGPT(tgEnd.x, tgEnd.y, tgEnd.z);
        nfit = buf->getBitLong();
        DRW_DBG("\nnumber of fit points: "); DRW_DBG(nfit);
    } else if (scenario == 1) {
        flags = 8;//scenario 1 = rational & planar
        flags |= buf->getBit() << 2; //flags, rational, code 70, bit 2 (4)
        flags |= buf->getBit(); //flags, closed, code 70, bit 0 (1)
        flags |= buf->getBit() << 1; //flags, periodic, code 70, bit 1 (2)
        tolknot = buf->getBitDouble();
        tolcontrol = buf->getBitDouble();
        DRW_DBG("flags: "); DRW_DBG(flags); DRW_DBG(" knot tolerance: "); DRW_DBG(tolknot);
        DRW_DBG(" control point tolerance: "); DRW_DBG(tolcontrol);
        nknots = buf->getBitLong();
        ncontrol = buf->getBitLong();
        weight = buf->getBit(); // RLZ ??? flags, weight, code 70, bit 4 (16)
        DRW_DBG("\nnum of knots: "); DRW_DBG(nknots); DRW_DBG(" num of control pt: ");
        DRW_DBG(ncontrol); DRW_DBG(" weight bit: "); DRW_DBG(weight);
    } else {
        DRW_DBG("\ndwg Ellipse, unknouwn scenario\n");
        return false; //RLZ: from doc only 1 or 2 are ok ?
    }

    RESERVE( knotslist, nknots );
    for (dint32 i= 0; i<nknots; ++i){
        knotslist.push_back (buf->getBitDouble());
    }
    RESERVE( controllist, ncontrol );
    for (dint32 i= 0; i<ncontrol; ++i){
        controllist.push_back(std::make_shared<DRW_Coord>(buf->get3BitDouble()));
        if (weight) {
            DRW_DBG("\n w: "); DRW_DBG(buf->getBitDouble()); //RLZ Warning: D (BD or RD)
        }
    }
    RESERVE( fitlist, nfit );
  for (dint32 i= 0; i<nfit; ++i)
      fitlist.push_back(std::make_shared<DRW_Coord>(buf->get3BitDouble()));

    if (DRW_DBGGL == DRW_dbg::Level::Debug){
          DRW_DBG("\nknots list: ");
          for (auto const& v: knotslist) {
              DRW_DBG("\n"); DRW_DBG(v);
          }
          DRW_DBG("\ncontrol point list: ");
          for (auto const& v: controllist) {
              DRW_DBG("\n"); DRW_DBGPT(v->x, v->y, v->z);
          }
          DRW_DBG("\nfit point list: ");
          for (auto const& v: fitlist) {
              DRW_DBG("\n"); DRW_DBGPT(v->x, v->y, v->z);
          }
    }

    /* Common Entity Handle Data */
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
//    RS crc;   //RS */
    return buf->isGood();
}

void DRW_Image::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 12:
        vVector.x = reader->getDouble();
        break;
    case 22:
        vVector.y = reader->getDouble();
        break;
    case 32:
        vVector.z = reader->getDouble();
        break;
    case 13:
        sizeu = reader->getDouble();
        break;
    case 23:
        sizev = reader->getDouble();
        break;
    case 340:
        ref = reader->getHandleString();
        break;
    case 280:
        clip = reader->getInt32();
        break;
    case 281:
        brightness = reader->getInt32();
        break;
    case 282:
        contrast = reader->getInt32();
        break;
    case 283:
        fade = reader->getInt32();
        break;
    default:
        DRW_Line::parseCode(code, reader);
        break;
    }
}

bool DRW_Image::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing image *********************************************\n");

    dint32 classVersion = buf->getBitLong();
    DRW_DBG("class Version: "); DRW_DBG(classVersion);
    basePoint = buf->get3BitDouble();
    DRW_DBG("\nbase point: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
    secPoint = buf->get3BitDouble();
    DRW_DBG("\nU vector: "); DRW_DBGPT(secPoint.x, secPoint.y, secPoint.z);
    vVector = buf->get3BitDouble();
    DRW_DBG("\nV vector: "); DRW_DBGPT(vVector.x, vVector.y, vVector.z);
    sizeu = buf->getRawDouble();
    sizev = buf->getRawDouble();
    DRW_DBG("\nsize U: "); DRW_DBG(sizeu); DRW_DBG("\nsize V: "); DRW_DBG(sizev);
    duint16 displayProps = buf->getBitShort();
    DRW_UNUSED(displayProps);//RLZ: temporary, complete API
    clip = buf->getBit();
    brightness = buf->getRawChar8();
    contrast = buf->getRawChar8();
    fade = buf->getRawChar8();
    if (version > DRW::AC1021){ //2010+
        bool clipMode = buf->getBit();
        DRW_UNUSED(clipMode);//RLZ: temporary, complete API
    }
    duint16 clipType = buf->getBitShort();
    if (clipType == 1){
        buf->get2RawDouble();
        buf->get2RawDouble();
    } else { //clipType == 2
        dint32 numVerts = buf->getBitLong();
        for (int i= 0; i< numVerts;++i)
            buf->get2RawDouble();
    }

    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

    dwgHandle biH = buf->getHandle();
    DRW_DBG("ImageDef Handle: "); DRW_DBGHL(biH.code, biH.size, biH.ref);
    ref = biH.ref;
    biH = buf->getHandle();
    DRW_DBG("ImageDefReactor Handle: "); DRW_DBGHL(biH.code, biH.size, biH.ref);
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
//    RS crc;   //RS */
    return buf->isGood();
}

void DRW_Dimension::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 1:
        text = reader->getUtf8String();
        break;
    case 2:
        name = reader->getString();
        break;
    case 3:
        style = reader->getUtf8String();
        break;
    case 70:
        type = reader->getInt32();
        break;
    case 71:
        align = reader->getInt32();
        break;
    case 72:
        linesty = reader->getInt32();
        break;
    case 10:
        defPoint.x = reader->getDouble();
        break;
    case 20:
        defPoint.y = reader->getDouble();
        break;
    case 30:
        defPoint.z = reader->getDouble();
        break;
    case 11:
        textPoint.x = reader->getDouble();
        break;
    case 21:
        textPoint.y = reader->getDouble();
        break;
    case 31:
        textPoint.z = reader->getDouble();
        break;
    case 12:
        clonePoint.x = reader->getDouble();
        break;
    case 22:
        clonePoint.y = reader->getDouble();
        break;
    case 32:
        clonePoint.z = reader->getDouble();
        break;
    case 13:
        def1.x = reader->getDouble();
        break;
    case 23:
        def1.y = reader->getDouble();
        break;
    case 33:
        def1.z = reader->getDouble();
        break;
    case 14:
        def2.x = reader->getDouble();
        break;
    case 24:
        def2.y = reader->getDouble();
        break;
    case 34:
        def2.z = reader->getDouble();
        break;
    case 15:
        circlePoint.x = reader->getDouble();
        break;
    case 25:
        circlePoint.y = reader->getDouble();
        break;
    case 35:
        circlePoint.z = reader->getDouble();
        break;
    case 16:
        arcPoint.x = reader->getDouble();
        break;
    case 26:
        arcPoint.y = reader->getDouble();
        break;
    case 36:
        arcPoint.z = reader->getDouble();
        break;
    case 41:
        linefactor = reader->getDouble();
        break;
    case 53:
        rot = reader->getDouble();
        break;
    case 50:
        angle = reader->getDouble() / ARAD;
        break;
    case 52:
        oblique = reader->getDouble() / ARAD;
        break;
    case 40:
        length = reader->getDouble();
        break;
    case 51:
        hdir = reader->getDouble();
        break;
    default:
        DRW_Entity::parseCode(code, reader);
        break;
    }
}

bool DRW_Dimension::parseDwg(DRW::Version version, dwgBuffer *buf, dwgBuffer *sBuf){
    DRW_DBG("\n***************************** parsing dimension *********************************************");
    if (version > DRW::AC1021) { //2010+
        duint8 dimVersion = buf->getRawChar8();
        DRW_DBG("\ndimVersion: "); DRW_DBG(dimVersion);
    }
    extPoint = buf->getExtrusion(version > DRW::AC1014, haveExtrusion);
    DRW_DBG("\nextPoint: "); DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
    if (version > DRW::AC1014) { //2000+
        DRW_DBG("\nFive unknown bits: "); DRW_DBG(buf->getBit()); DRW_DBG(buf->getBit());
        DRW_DBG(buf->getBit()); DRW_DBG(buf->getBit()); DRW_DBG(buf->getBit());
    }
    textPoint.x = buf->getRawDouble();
    textPoint.y = buf->getRawDouble();
    textPoint.z = buf->getBitDouble();
    DRW_DBG("\ntextPoint: "); DRW_DBGPT(textPoint.x, textPoint.y, textPoint.z);
    type = buf->getRawChar8();
    DRW_DBG("\ntype (70) read: "); DRW_DBG(type);
    type =  (type & 1) ? type & 0x7F : type | 0x80; //set bit 7
    type =  (type & 2) ? type | 0x20 : type & 0xDF; //set bit 5
    DRW_DBG(" type (70) set: "); DRW_DBG(type);
    //clear last 3 bits to set integer dim type
    type &= 0xF8;
    text = sBuf->getVariableText(version, false);
    DRW_DBG("\nforced dim text: "); DRW_DBG(text.c_str());
    rot = buf->getBitDouble();
    hdir = buf->getBitDouble();
    DRW_Coord inspoint = buf->get3BitDouble();
    DRW_DBG("\ninspoint: "); DRW_DBGPT(inspoint.x, inspoint.y, inspoint.z);
    double insRot_code54 = buf->getBitDouble(); //RLZ: unknown, investigate
    DRW_DBG(" insRot_code54: "); DRW_DBG(insRot_code54);
    if (version > DRW::AC1014) { //2000+
        align = buf->getBitShort();
        linesty = buf->getBitShort();
        linefactor = buf->getBitDouble();
        double actMeas = buf->getBitDouble();
        DRW_DBG("\n  actMeas_code42: "); DRW_DBG(actMeas);
        if (version > DRW::AC1018) { //2007+
            bool unk = buf->getBit();
            bool flip1 = buf->getBit();
            bool flip2 = buf->getBit();
            DRW_DBG("\n2007, unk, flip1, flip2: "); DRW_DBG(unk); DRW_DBG(flip1); DRW_DBG(flip2);
        }
    }
    clonePoint.x = buf->getRawDouble();
    clonePoint.y = buf->getRawDouble();
    DRW_DBG("\nclonePoint: "); DRW_DBGPT(clonePoint.x, clonePoint.y, clonePoint.z);

    return buf->isGood();
}

bool DRW_DimAligned::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
    if (!ret)
        return ret;
    ret = DRW_Dimension::parseDwg(version, buf, sBuf);
    if (!ret)
        return ret;
    if (oType == 0x15)
        DRW_DBG("\n***************************** parsing dim linear *********************************************\n");
    else
        DRW_DBG("\n***************************** parsing dim aligned *********************************************\n");
    DRW_Coord pt = buf->get3BitDouble();
    setPt3(pt); //def1
    DRW_DBG("def1: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    pt = buf->get3BitDouble();
    setPt4(pt);
    DRW_DBG("\ndef2: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    pt = buf->get3BitDouble();
    setDefPoint(pt);
    DRW_DBG("\ndefPoint: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    setOb52(buf->getBitDouble());
    if (oType == 0x15)
        setAn50(buf->getBitDouble() * ARAD);
    else
        type |= 1;
    DRW_DBG("\n  type (70) final: "); DRW_DBG(type); DRW_DBG("\n");

    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    if (!ret)
        return ret;
    dimStyleH = buf->getHandle();
    DRW_DBG("dim style Handle: "); DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref); DRW_DBG("\n");
    blockH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
    DRW_DBG("anon block Handle: "); DRW_DBGHL(blockH.code, blockH.size, blockH.ref); DRW_DBG("\n");
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

    //    RS crc;   //RS */
    return buf->isGood();
 }

 bool DRW_DimRadial::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
     dwgBuffer sBuff = *buf;
     dwgBuffer *sBuf = buf;
     if (version > DRW::AC1018) {//2007+
         sBuf = &sBuff; //separate buffer for strings
     }
     bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
     if (!ret)
         return ret;
     ret = DRW_Dimension::parseDwg(version, buf, sBuf);
     if (!ret)
         return ret;
     DRW_DBG("\n***************************** parsing dim radial *********************************************\n");
     DRW_Coord pt = buf->get3BitDouble();
     setDefPoint(pt); //code 10
     DRW_DBG("defPoint: "); DRW_DBGPT(pt.x, pt.y, pt.z);
     pt = buf->get3BitDouble();
     setPt5(pt); //center pt  code 15
     DRW_DBG("\ncenter point: "); DRW_DBGPT(pt.x, pt.y, pt.z);
     setRa40(buf->getBitDouble()); //leader length code 40
     DRW_DBG("\nleader length: "); DRW_DBG(getRa40());
     type |= 4;
     DRW_DBG("\n  type (70) final: "); DRW_DBG(type); DRW_DBG("\n");

     ret = DRW_Entity::parseDwgEntHandle(version, buf);
     DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
     if (!ret)
         return ret;
     dimStyleH = buf->getHandle();
     DRW_DBG("dim style Handle: "); DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref); DRW_DBG("\n");
     blockH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
     DRW_DBG("anon block Handle: "); DRW_DBGHL(blockH.code, blockH.size, blockH.ref); DRW_DBG("\n");
     DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

     //    RS crc;   //RS */
     return buf->isGood();
 }

 bool DRW_DimDiametric::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
     dwgBuffer sBuff = *buf;
     dwgBuffer *sBuf = buf;
     if (version > DRW::AC1018) {//2007+
         sBuf = &sBuff; //separate buffer for strings
     }
     bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
     if (!ret)
         return ret;
     ret = DRW_Dimension::parseDwg(version, buf, sBuf);
     if (!ret)
         return ret;
     DRW_DBG("\n***************************** parsing dim diametric *********************************************\n");
     DRW_Coord pt = buf->get3BitDouble();
     setPt5(pt); //center pt  code 15
     DRW_DBG("center point: "); DRW_DBGPT(pt.x, pt.y, pt.z);
     pt = buf->get3BitDouble();
     setDefPoint(pt); //code 10
     DRW_DBG("\ndefPoint: "); DRW_DBGPT(pt.x, pt.y, pt.z);
     setRa40(buf->getBitDouble()); //leader length code 40
     DRW_DBG("\nleader length: "); DRW_DBG(getRa40());
     type |= 3;
     DRW_DBG("\n  type (70) final: "); DRW_DBG(type); DRW_DBG("\n");

     ret = DRW_Entity::parseDwgEntHandle(version, buf);
     DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
     if (!ret)
         return ret;
     dimStyleH = buf->getHandle();
     DRW_DBG("dim style Handle: "); DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref); DRW_DBG("\n");
     blockH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
     DRW_DBG("anon block Handle: "); DRW_DBGHL(blockH.code, blockH.size, blockH.ref); DRW_DBG("\n");
     DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

     //    RS crc;   //RS */
     return buf->isGood();
 }

bool DRW_DimAngular::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
    if (!ret)
        return ret;
    ret = DRW_Dimension::parseDwg(version, buf, sBuf);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing dim angular *********************************************\n");
    DRW_Coord pt;
    pt.x = buf->getRawDouble();
    pt.y = buf->getRawDouble();
    setPt6(pt); //code 16
    DRW_DBG("arc Point: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    pt = buf->get3BitDouble();
    setPt3(pt); //def1  code 13
    DRW_DBG("\ndef1: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    pt = buf->get3BitDouble();
    setPt4(pt); //def2  code 14
    DRW_DBG("\ndef2: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    pt = buf->get3BitDouble();
    setPt5(pt); //center pt  code 15
    DRW_DBG("\ncenter point: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    pt = buf->get3BitDouble();
    setDefPoint(pt); //code 10
    DRW_DBG("\ndefPoint: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    type |= 0x02;
    DRW_DBG("\n  type (70) final: "); DRW_DBG(type); DRW_DBG("\n");

    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    if (!ret)
        return ret;
    dimStyleH = buf->getHandle();
    DRW_DBG("dim style Handle: "); DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref); DRW_DBG("\n");
    blockH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
    DRW_DBG("anon block Handle: "); DRW_DBGHL(blockH.code, blockH.size, blockH.ref); DRW_DBG("\n");
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

    //    RS crc;   //RS */
    return buf->isGood();
}

bool DRW_DimAngular3p::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
    if (!ret)
        return ret;
    ret = DRW_Dimension::parseDwg(version, buf, sBuf);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing dim angular3p *********************************************\n");
    DRW_Coord pt = buf->get3BitDouble();
    setDefPoint(pt); //code 10
    DRW_DBG("defPoint: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    pt = buf->get3BitDouble();
    setPt3(pt); //def1  code 13
    DRW_DBG("\ndef1: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    pt = buf->get3BitDouble();
    setPt4(pt); //def2  code 14
    DRW_DBG("\ndef2: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    pt = buf->get3BitDouble();
    setPt5(pt); //center pt  code 15
    DRW_DBG("\ncenter point: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    type |= 0x05;
    DRW_DBG("\n  type (70) final: "); DRW_DBG(type); DRW_DBG("\n");

    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    if (!ret)
        return ret;
    dimStyleH = buf->getHandle();
    DRW_DBG("dim style Handle: "); DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref); DRW_DBG("\n");
    blockH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
    DRW_DBG("anon block Handle: "); DRW_DBGHL(blockH.code, blockH.size, blockH.ref); DRW_DBG("\n");
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

    //    RS crc;   //RS */
    return buf->isGood();
}

bool DRW_DimOrdinate::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
    if (!ret)
        return ret;
    ret = DRW_Dimension::parseDwg(version, buf, sBuf);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing dim ordinate *********************************************\n");
    DRW_Coord pt = buf->get3BitDouble();
    setDefPoint(pt);
    DRW_DBG("defPoint: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    pt = buf->get3BitDouble();
    setPt3(pt); //def1
    DRW_DBG("\ndef1: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    pt = buf->get3BitDouble();
    setPt4(pt);
    DRW_DBG("\ndef2: "); DRW_DBGPT(pt.x, pt.y, pt.z);
    duint8 type2 = buf->getRawChar8();//RLZ: correct this
    DRW_DBG("type2 (70) read: "); DRW_DBG(type2);
    type =  (type2 & 1) ? type | 0x80 : type & 0xBF; //set bit 6
    DRW_DBG(" type (70) set: "); DRW_DBG(type);
    type |= 6;
    DRW_DBG("\n  type (70) final: "); DRW_DBG(type);

    ret = DRW_Entity::parseDwgEntHandle(version, buf); DRW_DBG("\n");
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    if (!ret)
        return ret;
    dimStyleH = buf->getHandle();
    DRW_DBG("dim style Handle: "); DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref); DRW_DBG("\n");
    blockH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
    DRW_DBG("anon block Handle: "); DRW_DBGHL(blockH.code, blockH.size, blockH.ref); DRW_DBG("\n");
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

    //    RS crc;   //RS */
    return buf->isGood();
}

void DRW_Leader::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 3:
        style = reader->getUtf8String();
        break;
    case 71:
        arrow = reader->getInt32();
        break;
    case 72:
        leadertype = reader->getInt32();
        break;
    case 73:
        flag = reader->getInt32();
        break;
    case 74:
        hookline = reader->getInt32();
        break;
    case 75:
        hookflag = reader->getInt32();
        break;
    case 76:
        vertnum = reader->getInt32();
        break;
    case 77:
        coloruse = reader->getInt32();
        break;
    case 40:
        textheight = reader->getDouble();
        break;
    case 41:
        textwidth = reader->getDouble();
        break;
    case 10:
        vertexpoint= std::make_shared<DRW_Coord>();
        vertexlist.push_back(vertexpoint);
        vertexpoint->x = reader->getDouble();
        break;
    case 20:
        if(vertexpoint)
            vertexpoint->y = reader->getDouble();
        break;
    case 30:
        if(vertexpoint)
            vertexpoint->z = reader->getDouble();
        break;
    case 340:
        annotHandle = reader->getHandleString();
        break;
    case 210:
        extrusionPoint.x = reader->getDouble();
        break;
    case 220:
        extrusionPoint.y = reader->getDouble();
        break;
    case 230:
        extrusionPoint.z = reader->getDouble();
        break;
    case 211:
        horizdir.x = reader->getDouble();
        break;
    case 221:
        horizdir.y = reader->getDouble();
        break;
    case 231:
        horizdir.z = reader->getDouble();
        break;
    case 212:
        offsetblock.x = reader->getDouble();
        break;
    case 222:
        offsetblock.y = reader->getDouble();
        break;
    case 232:
        offsetblock.z = reader->getDouble();
        break;
    case 213:
        offsettext.x = reader->getDouble();
        break;
    case 223:
        offsettext.y = reader->getDouble();
        break;
    case 233:
        offsettext.z = reader->getDouble();
        break;
    default:
        DRW_Entity::parseCode(code, reader);
        break;
    }
}

bool DRW_Leader::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing leader *********************************************\n");
    DRW_DBG("unknown bit "); DRW_DBG(buf->getBit());
    DRW_DBG(" annot type "); DRW_DBG(buf->getBitShort());
    DRW_DBG(" Path type "); DRW_DBG(buf->getBitShort());
    dint32 nPt = buf->getBitLong();
    DRW_DBG(" Num pts "); DRW_DBG(nPt);

    // add vertices
    for (int i = 0; i< nPt; i++){
		DRW_Coord vertex = buf->get3BitDouble();
		vertexlist.push_back(std::make_shared<DRW_Coord>(vertex));
		DRW_DBG("\nvertex "); DRW_DBGPT(vertex.x, vertex.y, vertex.z);
    }
    DRW_Coord Endptproj = buf->get3BitDouble();
    DRW_DBG("\nEndptproj "); DRW_DBGPT(Endptproj.x, Endptproj.y, Endptproj.z);
    extrusionPoint = buf->getExtrusion(version > DRW::AC1014, haveExtrusion);
    DRW_DBG("\nextrusionPoint "); DRW_DBGPT(extrusionPoint.x, extrusionPoint.y, extrusionPoint.z);
    if (version > DRW::AC1014) { //2000+
        DRW_DBG("\nFive unknown bits: "); DRW_DBG(buf->getBit()); DRW_DBG(buf->getBit());
        DRW_DBG(buf->getBit()); DRW_DBG(buf->getBit()); DRW_DBG(buf->getBit());
    }
    horizdir = buf->get3BitDouble();
    DRW_DBG("\nhorizdir "); DRW_DBGPT(horizdir.x, horizdir.y, horizdir.z);
    offsetblock = buf->get3BitDouble();
    DRW_DBG("\noffsetblock "); DRW_DBGPT(offsetblock.x, offsetblock.y, offsetblock.z);
    if (version > DRW::AC1012) { //R14+
        DRW_Coord unk = buf->get3BitDouble();
        DRW_DBG("\nunknown "); DRW_DBGPT(unk.x, unk.y, unk.z);
    }
    if (version < DRW::AC1015) { //R14 -
        DRW_DBG("\ndimgap "); DRW_DBG(buf->getBitDouble());
    }
    if (version < DRW::AC1024) { //2010-
        textheight = buf->getBitDouble();
        textwidth = buf->getBitDouble();
        DRW_DBG("\ntextheight "); DRW_DBG(textheight); DRW_DBG(" textwidth "); DRW_DBG(textwidth);
    }
    hookline = buf->getBit();
    arrow = buf->getBit();
    DRW_DBG(" hookline "); DRW_DBG(hookline); DRW_DBG(" arrow flag "); DRW_DBG(arrow);

    if (version < DRW::AC1015) { //R14 -
        DRW_DBG("\nArrow head type "); DRW_DBG(buf->getBitShort());
        DRW_DBG("dimasz "); DRW_DBG(buf->getBitDouble());
        DRW_DBG("\nunk bit "); DRW_DBG(buf->getBit());
        DRW_DBG(" unk bit "); DRW_DBG(buf->getBit());
        DRW_DBG(" unk short "); DRW_DBG(buf->getBitShort());
        DRW_DBG(" byBlock color "); DRW_DBG(buf->getBitShort());
        DRW_DBG(" unk bit "); DRW_DBG(buf->getBit());
        DRW_DBG(" unk bit "); DRW_DBG(buf->getBit());
    } else { //R2000+
        DRW_DBG("\nunk short "); DRW_DBG(buf->getBitShort());
        DRW_DBG(" unk bit "); DRW_DBG(buf->getBit());
        DRW_DBG(" unk bit "); DRW_DBG(buf->getBit());
    }
    DRW_DBG("\n");
    ret = DRW_Entity::parseDwgEntHandle(version, buf);
    if (!ret)
        return ret;
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    AnnotH = buf->getHandle();
    annotHandle = AnnotH.ref;
    DRW_DBG("annot block Handle: "); DRW_DBGHL(AnnotH.code, AnnotH.size, dimStyleH.ref); DRW_DBG("\n");
    dimStyleH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
    DRW_DBG("dim style Handle: "); DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref); DRW_DBG("\n");
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
//    RS crc;   //RS */
    return buf->isGood();
}

void DRW_Viewport::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 40:
        pswidth = reader->getDouble();
        break;
    case 41:
        psheight = reader->getDouble();
        break;
    case 68:
        vpstatus = reader->getInt32();
        break;
    case 69:
        vpID = reader->getInt32();
        break;
    case 12: {
        centerPX = reader->getDouble();
        break; }
    case 22:
        centerPY = reader->getDouble();
        break;
    default:
        DRW_Point::parseCode(code, reader);
        break;
    }
}
//ex 22 dec 34
bool DRW_Viewport::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
    if (!ret)
        return ret;
    DRW_DBG("\n***************************** parsing viewport *****************************************\n");
    basePoint.x = buf->getBitDouble();
    basePoint.y = buf->getBitDouble();
    basePoint.z = buf->getBitDouble();
    DRW_DBG("center "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
    pswidth = buf->getBitDouble();
    psheight = buf->getBitDouble();
    DRW_DBG("\nWidth: "); DRW_DBG(pswidth); DRW_DBG(", Height: "); DRW_DBG(psheight); DRW_DBG("\n");
    //RLZ TODO: complete in dxf
    if (version > DRW::AC1014) {//2000+
        viewTarget.x = buf->getBitDouble();
        viewTarget.y = buf->getBitDouble();
        viewTarget.z = buf->getBitDouble();
        DRW_DBG("view Target "); DRW_DBGPT(viewTarget.x, viewTarget.y, viewTarget.z);
        viewDir.x = buf->getBitDouble();
        viewDir.y = buf->getBitDouble();
        viewDir.z = buf->getBitDouble();
        DRW_DBG("\nview direction "); DRW_DBGPT(viewDir.x, viewDir.y, viewDir.z);
        twistAngle = buf->getBitDouble();
        DRW_DBG("\nView twist Angle: "); DRW_DBG(twistAngle);
        viewHeight = buf->getBitDouble();
        DRW_DBG("\nview Height: "); DRW_DBG(viewHeight);
        viewLength = buf->getBitDouble();
        DRW_DBG(" Lens Length: "); DRW_DBG(viewLength);
        frontClip = buf->getBitDouble();
        DRW_DBG("\nfront Clip Z: "); DRW_DBG(frontClip);
        backClip = buf->getBitDouble();
        DRW_DBG(" back Clip Z: "); DRW_DBG(backClip);
        snapAngle = buf->getBitDouble();
        DRW_DBG("\n snap Angle: "); DRW_DBG(snapAngle);
        centerPX = buf->getRawDouble();
        centerPY = buf->getRawDouble();
        DRW_DBG("\nview center X: "); DRW_DBG(centerPX); DRW_DBG(", Y: "); DRW_DBG(centerPX);
        snapPX = buf->getRawDouble();
        snapPY = buf->getRawDouble();
        DRW_DBG("\nSnap base point X: "); DRW_DBG(snapPX); DRW_DBG(", Y: "); DRW_DBG(snapPY);
        snapSpPX = buf->getRawDouble();
        snapSpPY = buf->getRawDouble();
        DRW_DBG("\nSnap spacing X: "); DRW_DBG(snapSpPX); DRW_DBG(", Y: "); DRW_DBG(snapSpPY);
        //RLZ: need to complete
        DRW_DBG("\nGrid spacing X: "); DRW_DBG(buf->getRawDouble()); DRW_DBG(", Y: "); DRW_DBG(buf->getRawDouble());DRW_DBG("\n");
        DRW_DBG("Circle zoom?: "); DRW_DBG(buf->getBitShort()); DRW_DBG("\n");
    }
    if (version > DRW::AC1018) {//2007+
        DRW_DBG("Grid major?: "); DRW_DBG(buf->getBitShort()); DRW_DBG("\n");
    }
    if (version > DRW::AC1014) {//2000+
        frozenLyCount = buf->getBitLong();
        DRW_DBG("Frozen Layer count?: "); DRW_DBG(frozenLyCount); DRW_DBG("\n");
        DRW_DBG("Status Flags?: "); DRW_DBG(buf->getBitLong()); DRW_DBG("\n");
        //RLZ: Warning needed separate string buffer
        DRW_DBG("Style sheet?: "); DRW_DBG(sBuf->getVariableText(version, false)); DRW_DBG("\n");
        DRW_DBG("Render mode?: "); DRW_DBG(buf->getRawChar8()); DRW_DBG("\n");
        DRW_DBG("UCS OMore...: "); DRW_DBG(buf->getBit()); DRW_DBG("\n");
        DRW_DBG("UCS VMore...: "); DRW_DBG(buf->getBit()); DRW_DBG("\n");
        DRW_DBG("UCS OMore...: "); DRW_DBGPT(buf->getBitDouble(), buf->getBitDouble(), buf->getBitDouble()); DRW_DBG("\n");
        DRW_DBG("ucs XAMore...: "); DRW_DBGPT(buf->getBitDouble(), buf->getBitDouble(), buf->getBitDouble()); DRW_DBG("\n");
        DRW_DBG("UCS YMore....: "); DRW_DBGPT(buf->getBitDouble(), buf->getBitDouble(), buf->getBitDouble()); DRW_DBG("\n");
        DRW_DBG("UCS EMore...: "); DRW_DBG(buf->getBitDouble()); DRW_DBG("\n");
        DRW_DBG("UCS OVMore...: "); DRW_DBG(buf->getBitShort()); DRW_DBG("\n");
    }
    if (version > DRW::AC1015) {//2004+
        DRW_DBG("ShadePlot Mode...: "); DRW_DBG(buf->getBitShort()); DRW_DBG("\n");
    }
    if (version > DRW::AC1018) {//2007+
        DRW_DBG("Use def Light...: "); DRW_DBG(buf->getBit()); DRW_DBG("\n");
        DRW_DBG("Def light type?: "); DRW_DBG(buf->getRawChar8()); DRW_DBG("\n");
        DRW_DBG("Brightness: "); DRW_DBG(buf->getBitDouble()); DRW_DBG("\n");
        DRW_DBG("Contrast: "); DRW_DBG(buf->getBitDouble()); DRW_DBG("\n");
//        DRW_DBG("Ambient Cmc or Enc: "); DRW_DBG(buf->getCmColor(version)); DRW_DBG("\n");
        DRW_DBG("Ambient (Cmc or Enc?), Enc: "); DRW_DBG(buf->getEnColor(version, color24, transparency)); DRW_DBG("\n");
    }
    ret = DRW_Entity::parseDwgEntHandle(version, buf);

    dwgHandle someHdl;
    if (version < DRW::AC1015) {//R13 & R14 only
        DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
        someHdl = buf->getHandle();
        DRW_DBG("ViewPort ent header: "); DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref); DRW_DBG("\n");
    }
    if (version > DRW::AC1014) {//2000+
        for (duint8 i=0; i < frozenLyCount; ++i){
            someHdl = buf->getHandle();
            DRW_DBG("Frozen layer handle "); DRW_DBG(i); DRW_DBG(": "); DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref); DRW_DBG("\n");
        }
        someHdl = buf->getHandle();
        DRW_DBG("Clip bpundary handle: "); DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref); DRW_DBG("\n");
        if (version == DRW::AC1015) {//2000 only
            someHdl = buf->getHandle();
            DRW_DBG("ViewPort ent header: "); DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref); DRW_DBG("\n");
        }
        someHdl = buf->getHandle();
        DRW_DBG("Named ucs handle: "); DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref); DRW_DBG("\n");
        DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
        someHdl = buf->getHandle();
        DRW_DBG("base ucs handle: "); DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref); DRW_DBG("\n");
    }
    if (version > DRW::AC1018) {//2007+
        someHdl = buf->getHandle();
        DRW_DBG("background handle: "); DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref); DRW_DBG("\n");
        someHdl = buf->getHandle();
        DRW_DBG("visual style handle: "); DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref); DRW_DBG("\n");
        someHdl = buf->getHandle();
        DRW_DBG("shadeplot ID handle: "); DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref); DRW_DBG("\n");
        DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
        someHdl = buf->getHandle();
        DRW_DBG("SUN handle: "); DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref); DRW_DBG("\n");
    }
    DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

    if (!ret)
        return ret;
    return buf->isGood();
}
