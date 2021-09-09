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

#include <iostream>
#include <cmath>

#include "drw_objects.h"
#include "intern/dxfreader.h"
#include "intern/dxfwriter.h"
#include "intern/dwgbuffer.h"
#include "intern/drw_dbg.h"
#include "intern/dwgutil.h"


#define RESERVE( vector, size ) try { \
    vector.reserve(size); \
  } catch(const std::exception &e) { \
   /* QgsDebugMsg( QString( "allocation exception (size=%1; error=%2)" ).arg( size ).arg( e.what() ) );*/ \
    throw e; \
  }

/*!
 *  Base class for tables entries
 *  @author Rallaz
 */
void DRW_TableEntry::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 5:
        handle = reader->getHandleString();
        break;
    case 330:
        parentHandle = reader->getHandleString();
        break;
    case 2:
        name = reader->getUtf8String();
        break;
    case 70:
        flags = reader->getInt32();
        break;
    case 1000:
    case 1001:
    case 1002:
    case 1003:
    case 1004:
    case 1005:
        extData.push_back(new DRW_Variant(code, reader->getString()));
        break;
    case 1010:
    case 1011:
    case 1012:
    case 1013:
        curr = new DRW_Variant(code, DRW_Coord(reader->getDouble(), 0.0, 0.0));
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
        curr=NULL;
        break;
    case 1040:
    case 1041:
    case 1042:
        extData.push_back(new DRW_Variant(code, reader->getDouble()));
        break;
    case 1070:
    case 1071:
        extData.push_back(new DRW_Variant(code, reader->getInt32() ));
        break;
    default:
        break;
    }
}

bool DRW_TableEntry::parseDwg(DRW::Version version, dwgBuffer *buf, dwgBuffer *strBuf, duint32 bs){
DRW_DBG("\n***************************** parsing table entry *********************************************\n");
    objSize=0;
    oType = buf->getObjType(version);
    DRW_DBG("Object type: "); DRW_DBG(oType); DRW_DBG(", "); DRW_DBGH(oType);
    if (version > DRW::AC1014 && version < DRW::AC1024) {//2000 to 2007
        objSize = buf->getRawLong32();  //RL 32bits object size in bits
        DRW_DBG(" Object size: "); DRW_DBG(objSize); DRW_DBG("\n");
    }
    if (version > DRW::AC1021) {//2010+
        duint32 ms = buf->size();
        objSize = ms*8 - bs;
        DRW_DBG(" Object size: "); DRW_DBG(objSize); DRW_DBG("\n");
    }
    if (strBuf != NULL && version > DRW::AC1018) {//2007+
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
    DRW_DBG("TableEntry Handle: "); DRW_DBGHL(ho.code, ho.size, ho.ref);
    dint16 extDataSize = buf->getBitShort(); //BS
    DRW_DBG(" ext data size: "); DRW_DBG(extDataSize);
    while (extDataSize>0 && buf->isGood()) {
        /* RLZ: TODO */
        dwgHandle ah = buf->getHandle();
        DRW_DBG("App Handle: "); DRW_DBGHL(ah.code, ah.size, ah.ref);
        duint8 *tmpExtData = new duint8[extDataSize];
        buf->getBytes(tmpExtData, extDataSize);
        dwgBuffer tmpExtDataBuf(tmpExtData, extDataSize, buf->decoder);
        int pos = tmpExtDataBuf.getPosition();
        int bpos = tmpExtDataBuf.getBitPos();
        DRW_DBG("ext data pos: "); DRW_DBG(pos); DRW_DBG("."); DRW_DBG(bpos); DRW_DBG("\n");
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
        DRW_DBG("ext data pos: "); DRW_DBG(tmpExtDataBuf.getPosition()); DRW_DBG("."); DRW_DBG(tmpExtDataBuf.getBitPos()); DRW_DBG("\n");
        delete[]tmpExtData;
        extDataSize = buf->getBitShort(); //BS
        DRW_DBG(" ext data size: "); DRW_DBG(extDataSize);
    } //end parsing extData (EED)
    if (version < DRW::AC1015) {//14-
        objSize = buf->getRawLong32();  //RL 32bits size in bits
    }
    DRW_DBG(" objSize in bits: "); DRW_DBG(objSize);

    numReactors = buf->getBitLong(); //BL
    DRW_DBG(", numReactors: "); DRW_DBG(numReactors); DRW_DBG("\n");
    if (version > DRW::AC1015) {//2004+
        xDictFlag = buf->getBit();
        DRW_DBG("xDictFlag: "); DRW_DBG(xDictFlag);
    }
    if (version > DRW::AC1024) {//2013+
        duint8 bd = buf->getBit();
        DRW_DBG(" Have binary data: "); DRW_DBG(bd); DRW_DBG("\n");
    }
    return buf->isGood();
}

//! Class to handle dimstyle entries
/*!
*  Class to handle ldim style symbol table entries
*  @author Rallaz
*/
void DRW_Dimstyle::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 105:
        handle = reader->getHandleString();
        break;
    case 3:
        dimpost = reader->getUtf8String();
        break;
    case 4:
        dimapost = reader->getUtf8String();
        break;
    case 5:
        dimblk = reader->getUtf8String();
        break;
    case 6:
        dimblk1 = reader->getUtf8String();
        break;
    case 7:
        dimblk2 = reader->getUtf8String();
        break;
    case 40:
        dimscale = reader->getDouble();
        break;
    case 41:
        dimasz = reader->getDouble();
        break;
    case 42:
        dimexo = reader->getDouble();
        break;
    case 43:
        dimdli = reader->getDouble();
        break;
    case 44:
        dimexe = reader->getDouble();
        break;
    case 45:
        dimrnd = reader->getDouble();
        break;
    case 46:
        dimdle = reader->getDouble();
        break;
    case 47:
        dimtp = reader->getDouble();
        break;
    case 48:
        dimtm = reader->getDouble();
        break;
    case 49:
        dimfxl = reader->getDouble();
        break;
    case 140:
        dimtxt = reader->getDouble();
        break;
    case 141:
        dimcen = reader->getDouble();
        break;
    case 142:
        dimtsz = reader->getDouble();
        break;
    case 143:
        dimaltf = reader->getDouble();
        break;
    case 144:
        dimlfac = reader->getDouble();
        break;
    case 145:
        dimtvp = reader->getDouble();
        break;
    case 146:
        dimtfac = reader->getDouble();
        break;
    case 147:
        dimgap = reader->getDouble();
        break;
    case 148:
        dimaltrnd = reader->getDouble();
        break;
    case 71:
        dimtol = reader->getInt32();
        break;
    case 72:
        dimlim = reader->getInt32();
        break;
    case 73:
        dimtih = reader->getInt32();
        break;
    case 74:
        dimtoh = reader->getInt32();
        break;
    case 75:
        dimse1 = reader->getInt32();
        break;
    case 76:
        dimse2 = reader->getInt32();
        break;
    case 77:
        dimtad = reader->getInt32();
        break;
    case 78:
        dimzin = reader->getInt32();
        break;
    case 79:
        dimazin = reader->getInt32();
        break;
    case 170:
        dimalt = reader->getInt32();
        break;
    case 171:
        dimaltd = reader->getInt32();
        break;
    case 172:
        dimtofl = reader->getInt32();
        break;
    case 173:
        dimsah = reader->getInt32();
        break;
    case 174:
        dimtix = reader->getInt32();
        break;
    case 175:
        dimsoxd = reader->getInt32();
        break;
    case 176:
        dimclrd = reader->getInt32();
        break;
    case 177:
        dimclre = reader->getInt32();
        break;
    case 178:
        dimclrt = reader->getInt32();
        break;
    case 179:
        dimadec = reader->getInt32();
        break;
    case 270:
        dimunit = reader->getInt32();
        break;
    case 271:
        dimdec = reader->getInt32();
        break;
    case 272:
        dimtdec = reader->getInt32();
        break;
    case 273:
        dimaltu = reader->getInt32();
        break;
    case 274:
        dimalttd = reader->getInt32();
        break;
    case 275:
        dimaunit = reader->getInt32();
        break;
    case 276:
        dimfrac = reader->getInt32();
        break;
    case 277:
        dimlunit = reader->getInt32();
        break;
    case 278:
        dimdsep = reader->getInt32();
        break;
    case 279:
        dimtmove = reader->getInt32();
        break;
    case 280:
        dimjust = reader->getInt32();
        break;
    case 281:
        dimsd1 = reader->getInt32();
        break;
    case 282:
        dimsd2 = reader->getInt32();
        break;
    case 283:
        dimtolj = reader->getInt32();
        break;
    case 284:
        dimtzin = reader->getInt32();
        break;
    case 285:
        dimaltz = reader->getInt32();
        break;
    case 286:
        dimaltttz = reader->getInt32();
        break;
    case 287:
        dimfit = reader->getInt32();
        break;
    case 288:
        dimupt = reader->getInt32();
        break;
    case 289:
        dimatfit = reader->getInt32();
        break;
    case 290:
        dimfxlon = reader->getInt32();
        break;
    case 340:
        dimtxsty = reader->getUtf8String();
        break;
    case 341:
        dimldrblk = reader->getUtf8String();
        break;
    case 342:
        dimblk = reader->getUtf8String();
        break;
    case 343:
        dimblk1 = reader->getUtf8String();
        break;
    case 344:
        dimblk2 = reader->getUtf8String();
        break;
    default:
        DRW_TableEntry::parseCode(code, reader);
        break;
    }
}

bool DRW_Dimstyle::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
    DRW_DBG("\n***************************** parsing dimension style **************************************\n");
    if (!ret)
        return ret;
    name = sBuf->getVariableText(version, false);
    DRW_DBG("dimension style name: "); DRW_DBG(name.c_str()); DRW_DBG("\n");

//    handleObj = shpControlH.ref;
    DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    //    RS crc;   //RS */
    return buf->isGood();
}


//! Class to handle line type entries
/*!
*  Class to handle line type symbol table entries
*  @author Rallaz
*/
void DRW_LType::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 3:
        desc = reader->getUtf8String();
        break;
    case 73:
      size = reader->getInt32();
      RESERVE( path, size );
      break;
    case 40:
        length = reader->getDouble();
        break;
    case 49:
        path.push_back(reader->getDouble());
        pathIdx++;
        break;
/*    case 74:
        haveShape = reader->getInt32();
        break;*/
    default:
        DRW_TableEntry::parseCode(code, reader);
        break;
    }
}

//! Update line type
/*!
*  Update the size and length of line type according to the path
*  @author Rallaz
*/
/*TODO: control max length permitted */
void DRW_LType::update(){
  double d = 0;
  size = path.size();
  for ( std::vector<double>::size_type i = 0;  i < size; i++ )
  {
    d += std::fabs( path.at( i ) );
  }
  length = d;
}

bool DRW_LType::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
    DRW_DBG("\n***************************** parsing line type *********************************************\n");
    if (!ret)
        return ret;
    name = sBuf->getVariableText(version, false);
    DRW_DBG("linetype name: "); DRW_DBG(name.c_str()); DRW_DBG("\n");
    flags = buf->getBit()<< 6;
    DRW_DBG("flags: "); DRW_DBG(flags);
    if (version > DRW::AC1018) {//2007+
    } else {//2004- //RLZ: verify in 2004, 2010 &2013
        dint16 xrefindex = buf->getBitShort();
        DRW_DBG(" xrefindex: "); DRW_DBG(xrefindex);
    }
    duint8 xdep = buf->getBit();
    DRW_DBG(" xdep: "); DRW_DBG(xdep);
    flags |= xdep<< 4;
    DRW_DBG(" flags: "); DRW_DBG(flags);
    desc = sBuf->getVariableText(version, false);
    DRW_DBG(" desc: "); DRW_DBG(desc.c_str());
    length = buf->getBitDouble();
    DRW_DBG(" pattern length: "); DRW_DBG(length);
    char align = buf->getRawChar8();
    DRW_DBG(" align: "); DRW_DBG(std::string(&align, 1));
    size = buf->getRawChar8();
    DRW_DBG(" num dashes, size: "); DRW_DBG(size);
    DRW_DBG("\n    dashes:\n");
    bool haveStrArea = false;
  for ( std::vector<double>::size_type i = 0; i < size; i++ )
  {
      path.push_back(buf->getBitDouble());
      /*int bs1 =*/ buf->getBitShort();
      /*double d1= */buf->getRawDouble();
      /*double d2=*/ buf->getRawDouble();
      /*double d3= */buf->getBitDouble();
      /*double d4= */buf->getBitDouble();
      int bs2 = buf->getBitShort();
      if((bs2 & 2) !=0) haveStrArea = true;
  }
  for (unsigned i=0; i<path.size() ; i++){
      DRW_DBG(", "); DRW_DBG(path[i]); DRW_DBG("\n");
  }
  DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
  if (version < DRW::AC1021) { //2004-
      duint8 strarea[256];
      buf->getBytes(strarea, 256);
      DRW_DBG("string area 256 bytes:\n"); DRW_DBG(reinterpret_cast<char*>(strarea)); DRW_DBG("\n");
  } else { //2007+
      //first verify flag
      if (haveStrArea) {
          duint8 strarea[512];
          buf->getBytes(strarea, 512);
          DRW_DBG("string area 512 bytes:\n"); DRW_DBG(reinterpret_cast<char*>(strarea)); DRW_DBG("\n");
      } else
          DRW_DBG("string area 512 bytes not present\n");
  }

  if (version > DRW::AC1021) {//2007+ skip string area
      DRW_DBG(" ltype end of object data pos 2010: "); DRW_DBG(buf->getPosition()); DRW_DBG(" strBuf bpos 2007: "); DRW_DBG(buf->getBitPos()); DRW_DBG("\n");
  }
  if (version > DRW::AC1018) {//2007+ skip string area
      buf->setPosition(objSize >> 3);
      buf->setBitPos(objSize & 7);
  }

  if (version > DRW::AC1021) {//2007+ skip string area
      DRW_DBG(" ltype start of handles data pos 2010: "); DRW_DBG(buf->getPosition()); DRW_DBG(" strBuf bpos 2007: "); DRW_DBG(buf->getBitPos()); DRW_DBG("\n");
  }

  DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
  dwgHandle ltControlH = buf->getHandle();
  DRW_DBG("linetype control Handle: "); DRW_DBGHL(ltControlH.code, ltControlH.size, ltControlH.ref);
  parentHandle = ltControlH.ref;
  DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
  for (int i=0; i< numReactors;++i) {
      dwgHandle reactorsH = buf->getHandle();
      DRW_DBG(" reactorsH control Handle: "); DRW_DBGHL(reactorsH.code, reactorsH.size, reactorsH.ref); DRW_DBG("\n");
  }
  if (xDictFlag !=1){//linetype in 2004 seems not have XDicObjH or NULL handle
      dwgHandle XDicObjH = buf->getHandle();
      DRW_DBG(" XDicObj control Handle: "); DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref); DRW_DBG("\n");
      DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
  }
  if(size>0){
      dwgHandle XRefH = buf->getHandle();
      DRW_DBG(" XRefH control Handle: "); DRW_DBGHL(XRefH.code, XRefH.size, XRefH.ref); DRW_DBG("\n");
      dwgHandle shpHandle = buf->getHandle();
      DRW_DBG(" shapeFile Handle: "); DRW_DBGHL(shpHandle.code, shpHandle.size, shpHandle.ref);
      DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
  }
  dwgHandle shpHandle = buf->getHandle();
  DRW_DBG(" shapeFile +1 Handle ??: "); DRW_DBGHL(shpHandle.code, shpHandle.size, shpHandle.ref); DRW_DBG("\n");

  DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
//    RS crc;   //RS */
  return buf->isGood();
}

//! Class to handle layer entries
/*!
*  Class to handle layer symbol table entries
*  @author Rallaz
*/
void DRW_Layer::parseCode(int code, dxfReader *reader){
  switch (code) {
  case 6:
      lineType = reader->getUtf8String();
      break;
  case 62:
      color = reader->getInt32();
      break;
  case 290:
      plotF = reader->getBool();
      break;
  case 370:
      lWeight = DRW_LW_Conv::dxfInt2lineWidth(reader->getInt32());
      break;
  case 390:
      handlePlotS = reader->getString();
      break;
  case 347:
      handleMaterialS = reader->getString();
      break;
  case 420:
      color24 = reader->getInt32();
      break;
    case 440:
      transparency = reader->getInt32();
      break;
    default:
      DRW_TableEntry::parseCode( code, reader );
      break;
  }
}

bool DRW_Layer::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
    DRW_DBG("\n***************************** parsing layer *********************************************\n");
    if (!ret)
        return ret;
    name = sBuf->getVariableText(version, false);
    DRW_DBG("layer name: "); DRW_DBG(name.c_str());

    flags |= buf->getBit()<< 6;//layer have entity
    if (version < DRW::AC1021) {//2004-
        DRW_DBG(", xrefindex = "); DRW_DBG(buf->getBitShort()); DRW_DBG("\n");
        //dint16 xrefindex = buf->getBitShort();
    }
    flags |= buf->getBit() << 4;//is refx dependent
    if (version < DRW::AC1015) {//14-
        flags |= buf->getBit(); //layer frozen
        /*flags |=*/ buf->getBit(); //unused, negate the color
        flags |= buf->getBit() << 1;//frozen in new
        flags |= buf->getBit()<< 3;//locked
    }
    if (version > DRW::AC1014) {//2000+
        dint16 f = buf->getSBitShort();//bit2 are layer on
        DRW_DBG(", flags 2000+: "); DRW_DBG(f); DRW_DBG("\n");
        flags |= f & 0x0001; //layer frozen
        flags |= ( f>> 1) & 0x0002;//frozen in new
        flags |= ( f>> 1) & 0x0004;//locked
        plotF = ( f>> 4) & 0x0001;
        lWeight = DRW_LW_Conv::dwgInt2lineWidth( (f & 0x03E0) >> 5 );
    }
    color = buf->getCmColor(version); //BS or CMC //ok for R14 or negate
    DRW_DBG(", entity color: "); DRW_DBG(color); DRW_DBG("\n");

    if (version > DRW::AC1018) {//2007+ skip string area
        buf->setPosition(objSize >> 3);
        buf->setBitPos(objSize & 7);
    }
    dwgHandle layerControlH = buf->getHandle();
    DRW_DBG("layer control Handle: "); DRW_DBGHL(layerControlH.code, layerControlH.size, layerControlH.ref);
    parentHandle = layerControlH.ref;

    if (xDictFlag !=1){//linetype in 2004 seems not have XDicObjH or NULL handle
        dwgHandle XDicObjH = buf->getHandle();
        DRW_DBG(" XDicObj control Handle: "); DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref); DRW_DBG("\n");
    }
    dwgHandle XRefH = buf->getHandle();
    DRW_DBG(" XRefH control Handle: "); DRW_DBGHL(XRefH.code, XRefH.size, XRefH.ref); DRW_DBG("\n");
    if (version > DRW::AC1014) {//2000+
        dwgHandle plotStyH = buf->getHandle();
        DRW_DBG(" PLot style control Handle: "); DRW_DBGHL(plotStyH.code, plotStyH.size, plotStyH.ref); DRW_DBG("\n");
        handlePlotS = DRW::toHexStr(plotStyH.ref);// std::string(plotStyH.ref);//RLZ: verify conversion
    }
    if (version > DRW::AC1018) {//2007+
        dwgHandle materialH = buf->getHandle();
        DRW_DBG(" Material control Handle: "); DRW_DBGHL(materialH.code, materialH.size, materialH.ref); DRW_DBG("\n");
        handleMaterialS = DRW::toHexStr(materialH.ref);//RLZ: verify conversion
    }
    //lineType handle
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    lTypeH = buf->getHandle();
    DRW_DBG("line type Handle: "); DRW_DBGHL(lTypeH.code, lTypeH.size, lTypeH.ref);
    DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
//    RS crc;   //RS */
    return buf->isGood();
}

bool DRW_Block_Record::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
    DRW_DBG("\n***************************** parsing block record ******************************************\n");
    if (!ret)
        return ret;
    duint32 insertCount = 0;//only 2000+
    duint32 objectCount = 0; //only 2004+

    name = sBuf->getVariableText(version, false);
    DRW_DBG("block record name: "); DRW_DBG(name.c_str()); DRW_DBG("\n");

    flags |= buf->getBit()<< 6;//referenced external reference, block code 70, bit 7 (64)
    if (version > DRW::AC1018) {//2007+
    } else {//2004- //RLZ: verify in 2004, 2010 &2013
        dint16 xrefindex = buf->getBitShort();
        DRW_DBG(" xrefindex: "); DRW_DBG(xrefindex); DRW_DBG("\n");
    }
    flags |= buf->getBit() << 4;//is refx dependent, block code 70, bit 5 (16)
    flags |= buf->getBit(); //if is anonimous block (*U) block code 70, bit 1 (1)
    flags |= buf->getBit() << 1; //if block contains attdefs, block code 70, bit 2 (2)
    bool blockIsXref = buf->getBit(); //if is a Xref, block code 70, bit 3 (4)
    bool xrefOverlaid = buf->getBit(); //if is a overlaid Xref, block code 70, bit 4 (8)
    flags |= blockIsXref << 2; //if is a Xref, block code 70, bit 3 (4)
    flags |= xrefOverlaid << 3; //if is a overlaid Xref, block code 70, bit 4 (8)
    if (version > DRW::AC1014) {//2000+
        flags |= buf->getBit() << 5; //if is a loaded Xref, block code 70, bit 6 (32)
    }
    DRW_DBG("flags: "); DRW_DBG(flags); DRW_DBG(", ");
    if (version > DRW::AC1015) {//2004+ fails in 2007
        objectCount = buf->getBitLong(); //Number of objects owned by this block
    RESERVE( entMap, objectCount );
  }
    basePoint.x = buf->getBitDouble();
    basePoint.y = buf->getBitDouble();
    basePoint.z = buf->getBitDouble();
    DRW_DBG("insertion point: "); DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z); DRW_DBG("\n");
    UTF8STRING path = sBuf->getVariableText(version, false);
    DRW_DBG("XRef path name: "); DRW_DBG(path.c_str()); DRW_DBG("\n");

    if (version > DRW::AC1014) {//2000+
        insertCount = 0;
        while (duint8 i = buf->getRawChar8() != 0)
            insertCount +=i;
        UTF8STRING bkdesc = sBuf->getVariableText(version, false);
        DRW_DBG("Block description: "); DRW_DBG(bkdesc.c_str()); DRW_DBG("\n");

        duint32 prevData = buf->getBitLong();
        for (unsigned int j= 0; j < prevData; ++j)
            buf->getRawChar8();
    }
    if (version > DRW::AC1018) {//2007+
        duint16 insUnits = buf->getBitShort();
        bool canExplode = buf->getBit(); //if block can be exploded
        duint8 bkScaling = buf->getRawChar8();

        DRW_UNUSED(insUnits);
        DRW_UNUSED(canExplode);
        DRW_UNUSED(bkScaling);
    }

    if (version > DRW::AC1018) {//2007+ skip string area
        buf->setPosition(objSize >> 3);
        buf->setBitPos(objSize & 7);
    }

    dwgHandle blockControlH = buf->getHandle();
    DRW_DBG("block control Handle: "); DRW_DBGHL(blockControlH.code, blockControlH.size, blockControlH.ref); DRW_DBG("\n");
    parentHandle = blockControlH.ref;

    for (int i=0; i<numReactors; i++){
        dwgHandle reactorH = buf->getHandle();
        DRW_DBG(" reactor Handle #"); DRW_DBG(i); DRW_DBG(": "); DRW_DBGHL(reactorH.code, reactorH.size, reactorH.ref); DRW_DBG("\n");
    }
    if (xDictFlag !=1) {//R14+ //seems present in 2000
        dwgHandle XDicObjH = buf->getHandle();
        DRW_DBG(" XDicObj control Handle: "); DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref); DRW_DBG("\n");
    }
    if (version != DRW::AC1021) {//2007+ XDicObjH or NullH not present
    }
    dwgHandle NullH = buf->getHandle();
    DRW_DBG(" NullH control Handle: "); DRW_DBGHL(NullH.code, NullH.size, NullH.ref); DRW_DBG("\n");
    dwgHandle blockH = buf->getOffsetHandle(handle);
    DRW_DBG(" blockH Handle: "); DRW_DBGHL(blockH.code, blockH.size, blockH.ref); DRW_DBG("\n");
    block = blockH.ref;

    if (version > DRW::AC1015) {//2004+
        for (unsigned int i=0; i< objectCount; i++){
            dwgHandle entityH = buf->getHandle();
            DRW_DBG(" entityH Handle #"); DRW_DBG(i); DRW_DBG(": "); DRW_DBGHL(entityH.code, entityH.size, entityH.ref); DRW_DBG("\n");
            entMap.push_back(entityH.ref);
        }
    } else {//2000-
        if(!blockIsXref && !xrefOverlaid){
            dwgHandle firstH = buf->getHandle();
            DRW_DBG(" firstH entity Handle: "); DRW_DBGHL(firstH.code, firstH.size, firstH.ref); DRW_DBG("\n");
            firstEH = firstH.ref;
            dwgHandle lastH = buf->getHandle();
            DRW_DBG(" lastH entity Handle: "); DRW_DBGHL(lastH.code, lastH.size, lastH.ref); DRW_DBG("\n");
            lastEH = lastH.ref;
        }
    }
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    dwgHandle endBlockH = buf->getOffsetHandle(handle);
    DRW_DBG(" endBlockH Handle: "); DRW_DBGHL(endBlockH.code, endBlockH.size, endBlockH.ref); DRW_DBG("\n");
    endBlock = endBlockH.ref;

    if (version > DRW::AC1014) {//2000+
        for (unsigned int i=0; i< insertCount; i++){
            dwgHandle insertsH = buf->getHandle();
            DRW_DBG(" insertsH Handle #"); DRW_DBG(i); DRW_DBG(": "); DRW_DBGHL(insertsH.code, insertsH.size, insertsH.ref); DRW_DBG("\n");
        }
        DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
        dwgHandle layoutH = buf->getHandle();
        DRW_DBG(" layoutH Handle: "); DRW_DBGHL(layoutH.code, layoutH.size, layoutH.ref); DRW_DBG("\n");
    }
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n\n");
//    RS crc;   //RS */
    return buf->isGood();
}

//! Class to handle text style entries
/*!
*  Class to handle text style symbol table entries
*  @author Rallaz
*/
void DRW_Textstyle::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 3:
        font = reader->getUtf8String();
        break;
    case 4:
        bigFont = reader->getUtf8String();
        break;
    case 40:
        height = reader->getDouble();
        break;
    case 41:
        width = reader->getDouble();
        break;
    case 50:
        oblique = reader->getDouble();
        break;
    case 42:
        lastHeight = reader->getDouble();
        break;
    case 71:
        genFlag = reader->getInt32();
        break;
    case 1071:
        fontFamily = reader->getInt32();
        break;
    default:
        DRW_TableEntry::parseCode(code, reader);
        break;
    }
}

bool DRW_Textstyle::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
    DRW_DBG("\n***************************** parsing text style *********************************************\n");
    if (!ret)
        return ret;
    name = sBuf->getVariableText(version, false);
    DRW_DBG("text style name: "); DRW_DBG(name.c_str()); DRW_DBG("\n");
    flags |= buf->getBit()<< 6;//style are referenced for a entity, style code 70, bit 7 (64)
    /*dint16 xrefindex =*/ buf->getBitShort();
    flags |= buf->getBit() << 4; //is refx dependent, style code 70, bit 5 (16)
    flags |= buf->getBit() << 2; //vertical text, stile code 70, bit 3 (4)
    flags |= buf->getBit(); //if is a shape file instead of text, style code 70, bit 1 (1)
    height = buf->getBitDouble();
    width = buf->getBitDouble();
    oblique = buf->getBitDouble();
    genFlag = buf->getRawChar8();
    lastHeight = buf->getBitDouble();
    font = sBuf->getVariableText(version, false);
    bigFont = sBuf->getVariableText(version, false);
    if (version > DRW::AC1018) {//2007+ skip string area
        buf->setPosition(objSize >> 3);
        buf->setBitPos(objSize & 7);
    }
    dwgHandle shpControlH = buf->getHandle();
    DRW_DBG(" parentControlH Handle: "); DRW_DBGHL(shpControlH.code, shpControlH.size, shpControlH.ref); DRW_DBG("\n");
    parentHandle = shpControlH.ref;
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    if (xDictFlag !=1){//linetype in 2004 seems not have XDicObjH or NULL handle
        dwgHandle XDicObjH = buf->getHandle();
        DRW_DBG(" XDicObj control Handle: "); DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref); DRW_DBG("\n");
        DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    }
/*RLZ: fails verify this part*/    dwgHandle XRefH = buf->getHandle();
    DRW_DBG(" XRefH control Handle: "); DRW_DBGHL(XRefH.code, XRefH.size, XRefH.ref); DRW_DBG("\n");

    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n\n");
    //    RS crc;   //RS */
    return buf->isGood();
}

//! Class to handle vport entries
/*!
*  Class to handle vport symbol table entries
*  @author Rallaz
*/
void DRW_Vport::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 10:
        lowerLeft.x = reader->getDouble();
        break;
    case 20:
        lowerLeft.y = reader->getDouble();
        break;
    case 11:
        UpperRight.x = reader->getDouble();
        break;
    case 21:
        UpperRight.y = reader->getDouble();
        break;
    case 12:
        center.x = reader->getDouble();
        break;
    case 22:
        center.y = reader->getDouble();
        break;
    case 13:
        snapBase.x = reader->getDouble();
        break;
    case 23:
        snapBase.y = reader->getDouble();
        break;
    case 14:
        snapSpacing.x = reader->getDouble();
        break;
    case 24:
        snapSpacing.y = reader->getDouble();
        break;
    case 15:
        gridSpacing.x = reader->getDouble();
        break;
    case 25:
        gridSpacing.y = reader->getDouble();
        break;
    case 16:
        viewDir.x = reader->getDouble();
        break;
    case 26:
        viewDir.y = reader->getDouble();
        break;
    case 36:
        viewDir.z = reader->getDouble();
        break;
    case 17:
        viewTarget.x = reader->getDouble();
        break;
    case 27:
        viewTarget.y = reader->getDouble();
        break;
    case 37:
        viewTarget.z = reader->getDouble();
        break;
    case 40:
        height = reader->getDouble();
        break;
    case 41:
        ratio = reader->getDouble();
        break;
    case 42:
        lensHeight = reader->getDouble();
        break;
    case 43:
        frontClip = reader->getDouble();
        break;
    case 44:
        backClip = reader->getDouble();
        break;
    case 50:
        snapAngle = reader->getDouble();
        break;
    case 51:
        twistAngle = reader->getDouble();
        break;
    case 71:
        viewMode = reader->getInt32();
        break;
    case 72:
        circleZoom = reader->getInt32();
        break;
    case 73:
        fastZoom = reader->getInt32();
        break;
    case 74:
        ucsIcon = reader->getInt32();
        break;
    case 75:
        snap = reader->getInt32();
        break;
    case 76:
        grid = reader->getInt32();
        break;
    case 77:
        snapStyle = reader->getInt32();
        break;
    case 78:
        snapIsopair = reader->getInt32();
        break;
    default:
        DRW_TableEntry::parseCode(code, reader);
        break;
    }
}

bool DRW_Vport::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
    DRW_DBG("\n***************************** parsing VPort ************************************************\n");
    if (!ret)
        return ret;
    name = sBuf->getVariableText(version, false);
    DRW_DBG("vport name: "); DRW_DBG(name.c_str()); DRW_DBG("\n");
    flags |= buf->getBit()<< 6;// code 70, bit 7 (64)
    if (version < DRW::AC1021) { //2004-
        /*dint16 xrefindex =*/ buf->getBitShort();
    }
    flags |= buf->getBit() << 4; //is refx dependent, style code 70, bit 5 (16)
    height = buf->getBitDouble();
    ratio = buf->getBitDouble();
    DRW_DBG("flags: "); DRW_DBG(flags); DRW_DBG(" height: "); DRW_DBG(height);
    DRW_DBG(" ratio: "); DRW_DBG(ratio);
    center = buf->get2RawDouble();
    DRW_DBG("\nview center: "); DRW_DBGPT(center.x, center.y, center.z);
    viewTarget.x = buf->getBitDouble();
    viewTarget.y = buf->getBitDouble();
    viewTarget.z = buf->getBitDouble();
    DRW_DBG("\nview target: "); DRW_DBGPT(viewTarget.x, viewTarget.y, viewTarget.z);
    viewDir.x = buf->getBitDouble();
    viewDir.y = buf->getBitDouble();
    viewDir.z = buf->getBitDouble();
    DRW_DBG("\nview dir: "); DRW_DBGPT(viewDir.x, viewDir.y, viewDir.z);
    twistAngle = buf->getBitDouble();
    lensHeight = buf->getBitDouble();
    frontClip = buf->getBitDouble();
    backClip = buf->getBitDouble();
    DRW_DBG("\ntwistAngle: "); DRW_DBG(twistAngle); DRW_DBG(" lensHeight: "); DRW_DBG(lensHeight);
    DRW_DBG(" frontClip: "); DRW_DBG(frontClip); DRW_DBG(" backClip: "); DRW_DBG(backClip);
    viewMode = buf->getBit(); //view mode, code 71, bit 0 (1)
    viewMode |= buf->getBit() << 1; //view mode, code 71, bit 1 (2)
    viewMode |= buf->getBit() << 2; //view mode, code 71, bit 2 (4)
    viewMode |= buf->getBit() << 4; //view mode, code 71, bit 4 (16)
    if (version > DRW::AC1014) { //2000+
        //duint8 renderMode = buf->getRawChar8();
        DRW_DBG("\n renderMode: "); DRW_DBG(buf->getRawChar8());
        if (version > DRW::AC1018) { //2007+
            DRW_DBG("\n use default lights: "); DRW_DBG(buf->getBit());
            DRW_DBG(" default lighting type: "); DRW_DBG(buf->getRawChar8());
            DRW_DBG(" brightness: "); DRW_DBG(buf->getBitDouble());
            DRW_DBG("\n contrast: "); DRW_DBG(buf->getBitDouble()); DRW_DBG("\n");
            DRW_DBG(" ambient color CMC: "); DRW_DBG(buf->getCmColor(version));
        }
    }
    lowerLeft = buf->get2RawDouble();
    DRW_DBG("\nlowerLeft: "); DRW_DBGPT(lowerLeft.x, lowerLeft.y, lowerLeft.z);
    UpperRight = buf->get2RawDouble();
    DRW_DBG("\nUpperRight: "); DRW_DBGPT(UpperRight.x, UpperRight.y, UpperRight.z);
    viewMode |= buf->getBit() << 3; //UCSFOLLOW, view mode, code 71, bit 3 (8)
    circleZoom = buf->getBitShort();
    fastZoom = buf->getBit();
    DRW_DBG("\nviewMode: "); DRW_DBG(viewMode); DRW_DBG(" circleZoom: ");
    DRW_DBG(circleZoom); DRW_DBG(" fastZoom: "); DRW_DBG(fastZoom);
    ucsIcon = buf->getBit(); //ucs Icon, code 74, bit 0 (1)
    ucsIcon |= buf->getBit() << 1; //ucs Icon, code 74, bit 1 (2)
    grid = buf->getBit();
    DRW_DBG("\nucsIcon: "); DRW_DBG(ucsIcon); DRW_DBG(" grid: "); DRW_DBG(grid);
    gridSpacing = buf->get2RawDouble();
    DRW_DBG("\ngrid Spacing: "); DRW_DBGPT(gridSpacing.x, gridSpacing.y, gridSpacing.z);
    snap = buf->getBit();
    snapStyle = buf->getBit();
    DRW_DBG("\nsnap on/off: "); DRW_DBG(snap); DRW_DBG(" snap Style: "); DRW_DBG(snapStyle);
    snapIsopair = buf->getBitShort();
    snapAngle = buf->getBitDouble();
    DRW_DBG("\nsnap Isopair: "); DRW_DBG(snapIsopair); DRW_DBG(" snap Angle: "); DRW_DBG(snapAngle);
    snapBase = buf->get2RawDouble();
    DRW_DBG("\nsnap Base: "); DRW_DBGPT(snapBase.x, snapBase.y, snapBase.z);
    snapSpacing = buf->get2RawDouble();
    DRW_DBG("\nsnap Spacing: "); DRW_DBGPT(snapSpacing.x, snapSpacing.y, snapSpacing.z);
    if (version > DRW::AC1014) { //2000+
        DRW_DBG("\n Unknown: "); DRW_DBG(buf->getBit());
        DRW_DBG(" UCS per Viewport: "); DRW_DBG(buf->getBit());
        DRW_DBG("\nUCS origin: "); DRW_DBGPT(buf->getBitDouble(), buf->getBitDouble(), buf->getBitDouble());
        DRW_DBG("\nUCS X Axis: "); DRW_DBGPT(buf->getBitDouble(), buf->getBitDouble(), buf->getBitDouble());
        DRW_DBG("\nUCS Y Axis: "); DRW_DBGPT(buf->getBitDouble(), buf->getBitDouble(), buf->getBitDouble());
        DRW_DBG("\nUCS elevation: "); DRW_DBG(buf->getBitDouble());
        DRW_DBG(" UCS Orthographic type: "); DRW_DBG(buf->getBitShort());
        if (version > DRW::AC1018) { //2007+
            gridBehavior = buf->getBitShort();
            DRW_DBG(" gridBehavior (flags): "); DRW_DBG(gridBehavior);
            DRW_DBG(" Grid major: "); DRW_DBG(buf->getBitShort());
        }
    }

    //common handles
    if (version > DRW::AC1018) {//2007+ skip string area
        buf->setPosition(objSize >> 3);
        buf->setBitPos(objSize & 7);
    }
    dwgHandle vpControlH = buf->getHandle();
    DRW_DBG("\n parentControlH Handle: "); DRW_DBGHL(vpControlH.code, vpControlH.size, vpControlH.ref); DRW_DBG("\n");
    parentHandle = vpControlH.ref;
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    if (xDictFlag !=1){
        dwgHandle XDicObjH = buf->getHandle();
        DRW_DBG(" XDicObj control Handle: "); DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref); DRW_DBG("\n");
        DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    }
/*RLZ: fails verify this part*/    dwgHandle XRefH = buf->getHandle();
    DRW_DBG(" XRefH control Handle: "); DRW_DBGHL(XRefH.code, XRefH.size, XRefH.ref);

    if (version > DRW::AC1014) { //2000+
        DRW_DBG("\nRemaining bytes: "); DRW_DBG(buf->numRemainingBytes());
        if (version > DRW::AC1018) { //2007+
            dwgHandle bkgrdH = buf->getHandle();
            DRW_DBG(" background Handle: "); DRW_DBGHL(bkgrdH.code, bkgrdH.size, bkgrdH.ref);
            DRW_DBG("\nRemaining bytes: "); DRW_DBG(buf->numRemainingBytes());
            dwgHandle visualStH = buf->getHandle();
            DRW_DBG(" visual style Handle: "); DRW_DBGHL(visualStH.code, visualStH.size, visualStH.ref);
            DRW_DBG("\nRemaining bytes: "); DRW_DBG(buf->numRemainingBytes());
            dwgHandle sunH = buf->getHandle();
            DRW_DBG(" sun Handle: "); DRW_DBGHL(sunH.code, sunH.size, sunH.ref);
            DRW_DBG("\nRemaining bytes: "); DRW_DBG(buf->numRemainingBytes());
        }
        dwgHandle namedUCSH = buf->getHandle();
        DRW_DBG(" named UCS Handle: "); DRW_DBGHL(namedUCSH.code, namedUCSH.size, namedUCSH.ref);
        DRW_DBG("\nRemaining bytes: "); DRW_DBG(buf->numRemainingBytes());
        dwgHandle baseUCSH = buf->getHandle();
        DRW_DBG(" base UCS Handle: "); DRW_DBGHL(baseUCSH.code, baseUCSH.size, baseUCSH.ref);
    }

    DRW_DBG("\n Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    //    RS crc;   //RS */
    return buf->isGood();
}

void DRW_ImageDef::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 1:
        name = reader->getUtf8String();
        break;
    case 5:
        handle = reader->getHandleString();
        break;
    case 10:
        u = reader->getDouble();
        break;
    case 20:
        v = reader->getDouble();
        break;
    case 11:
        up = reader->getDouble();
        break;
    case 12:
        vp = reader->getDouble();
        break;
    case 21:
        vp = reader->getDouble();
        break;
    case 280:
        loaded = reader->getInt32();
        break;
    case 281:
        resolution = reader->getInt32();
        break;
    default:
        break;
    }
}

bool DRW_ImageDef::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
    DRW_DBG("\n***************************** parsing Image Def *********************************************\n");
    if (!ret)
        return ret;
    dint32 imgVersion = buf->getBitLong();
    DRW_DBG("class Version: "); DRW_DBG(imgVersion);
    DRW_Coord size = buf->get2RawDouble();
    DRW_UNUSED(size);//RLZ: temporary, complete API
    name = sBuf->getVariableText(version, false);
    DRW_DBG("appId name: "); DRW_DBG(name.c_str()); DRW_DBG("\n");
    loaded = buf->getBit();
    resolution = buf->getRawChar8();
    up = buf->getRawDouble();
    vp = buf->getRawDouble();

    dwgHandle parentH = buf->getHandle();
    DRW_DBG(" parentH Handle: "); DRW_DBGHL(parentH.code, parentH.size, parentH.ref); DRW_DBG("\n");
    parentHandle = parentH.ref;
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    //RLZ: Reactors handles
    if (xDictFlag !=1){
        dwgHandle XDicObjH = buf->getHandle();
        DRW_DBG(" XDicObj control Handle: "); DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref); DRW_DBG("\n");
        DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    }
/*RLZ: fails verify this part*/    dwgHandle XRefH = buf->getHandle();
    DRW_DBG(" XRefH control Handle: "); DRW_DBGHL(XRefH.code, XRefH.size, XRefH.ref); DRW_DBG("\n");

    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n\n");
    //    RS crc;   //RS */
    return buf->isGood();
}

bool DRW_AppId::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
    dwgBuffer sBuff = *buf;
    dwgBuffer *sBuf = buf;
    if (version > DRW::AC1018) {//2007+
        sBuf = &sBuff; //separate buffer for strings
    }
    bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
    DRW_DBG("\n***************************** parsing app Id *********************************************\n");
    if (!ret)
        return ret;
    name = sBuf->getVariableText(version, false);
    DRW_DBG("appId name: "); DRW_DBG(name.c_str()); DRW_DBG("\n");
    flags |= buf->getBit()<< 6;// code 70, bit 7 (64)
    /*dint16 xrefindex =*/ buf->getBitShort();
    flags |= buf->getBit() << 4; //is refx dependent, style code 70, bit 5 (16)
    duint8 unknown = buf->getRawChar8(); // unknown code 71
    DRW_DBG("unknown code 71: "); DRW_DBG(unknown); DRW_DBG("\n");
    if (version > DRW::AC1018) {//2007+ skip string area
        buf->setPosition(objSize >> 3);
        buf->setBitPos(objSize & 7);
    }
    dwgHandle appIdControlH = buf->getHandle();
    DRW_DBG(" parentControlH Handle: "); DRW_DBGHL(appIdControlH.code, appIdControlH.size, appIdControlH.ref); DRW_DBG("\n");
    parentHandle = appIdControlH.ref;
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    if (xDictFlag !=1){//linetype in 2004 seems not have XDicObjH or NULL handle
        dwgHandle XDicObjH = buf->getHandle();
        DRW_DBG(" XDicObj control Handle: "); DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref); DRW_DBG("\n");
        DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    }
/*RLZ: fails verify this part*/    dwgHandle XRefH = buf->getHandle();
    DRW_DBG(" XRefH control Handle: "); DRW_DBGHL(XRefH.code, XRefH.size, XRefH.ref); DRW_DBG("\n");

    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n\n");
    //    RS crc;   //RS */
    return buf->isGood();
}
