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
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "drw_dbg.h"
#include "dwgreader21.h"
#include "drw_textcodec.h"
#include "../libdwgr.h"

bool dwgReader21::readMetaData() {
    version = parent->getVersion();
    decoder.setVersion(version, false);
    DRW_DBG("dwgReader21::readFileHeader()\n");
    DRW_DBG("dwgReader21::parsing metadata\n");
    if (! fileBuf->setPosition(11))
        return false;
    maintenanceVersion = fileBuf->getRawChar8();
    DRW_DBG("maintenance version= "); DRW_DBGH(maintenanceVersion);
    DRW_DBG("\nbyte at 0x0C= "); DRW_DBG(fileBuf->getRawChar8());
    previewImagePos = fileBuf->getRawLong32();
    DRW_DBG("previewImagePos (seekerImageData) = "); DRW_DBG(previewImagePos);
    DRW_DBG("\n\napp writer version= "); DRW_DBGH(fileBuf->getRawChar8());
    DRW_DBG("\napp writer maintenance version= "); DRW_DBGH(fileBuf->getRawChar8());
    duint16 cp = fileBuf->getRawShort16();
    DRW_DBG("\ncodepage= "); DRW_DBG(cp);
    if (cp == 30)
        decoder.setCodePage("ANSI_1252", false);
    /* UNKNOUWN SECTION 2 bytes*/
    DRW_DBG("\nUNKNOWN SECTION= "); DRW_DBG(fileBuf->getRawShort16());
    DRW_DBG("\nUNKNOUWN SECTION 3b= "); DRW_DBG(fileBuf->getRawChar8());
    duint32 secType = fileBuf->getRawLong32();
    DRW_DBG("\nsecurity type flag= "); DRW_DBGH(secType);
    /* UNKNOWN2 SECTION 4 bytes*/
    DRW_DBG("\nUNKNOWN SECTION 4bytes= "); DRW_DBG(fileBuf->getRawLong32());

    DRW_DBG("\nSummary info address= "); DRW_DBGH(fileBuf->getRawLong32());
    DRW_DBG("\nVBA project address= "); DRW_DBGH(fileBuf->getRawLong32());
    DRW_DBG("\n0x00000080 32b= "); DRW_DBGH(fileBuf->getRawLong32());
    DRW_DBG("\nApp info address= "); DRW_DBGH(fileBuf->getRawLong32());
    //current position are 0x30 from here to 0x80 are undocumented
    DRW_DBG("\nAnother address? = "); DRW_DBGH(fileBuf->getRawLong32());
    return true;
}

bool dwgReader21::parseSysPage(duint64 sizeCompressed, duint64 sizeUncompressed, duint64 correctionFactor, duint64 offset, duint8 *decompData){
    //round to 8
    duint64 alsize = (sizeCompressed + 7) &(-8);
    //minimum RS chunk:
    duint32 chunks = (((alsize * correctionFactor)+238)/239);
    duint64 fpsize = chunks * 255;

    if (! fileBuf->setPosition(offset))
        return false;
    duint8 *tmpDataRaw = new duint8[fpsize];
    fileBuf->getBytes(tmpDataRaw, fpsize);
    duint8 *tmpDataRS = new duint8[fpsize];
    dwgRSCodec::decode239I(tmpDataRaw, tmpDataRS, fpsize/255);
    dwgCompressor::decompress21(tmpDataRS, decompData, sizeCompressed, sizeUncompressed);
    delete[]tmpDataRaw;
    delete[]tmpDataRS;
    return true;
}

bool dwgReader21::parseDataPage(const dwgSectionInfo &si, duint8 *dData){
    DRW_DBG("parseDataPage, section size: "); DRW_DBG(si.size);
    for (auto it=si.pages.begin(); it!=si.pages.end(); ++it){
        dwgPageInfo pi = it->second;
        if (!fileBuf->setPosition(pi.address))
            return false;

        duint8 *tmpPageRaw = new duint8[pi.size];
        fileBuf->getBytes(tmpPageRaw, pi.size);
    #ifdef DRW_DBG_DUMP
        DRW_DBG("\nSection OBJECTS raw data=\n");
        for (unsigned int i=0, j=0; i< pi.size;i++) {
            DRW_DBGH( (unsigned char)tmpPageRaw[i]);
            if (j == 7) { DRW_DBG("\n"); j = 0;
            } else { DRW_DBG(", "); j++; }
        } DRW_DBG("\n");
    #endif

        duint8 *tmpPageRS = new duint8[pi.size];
        duint8 chunks =pi.size / 255;
        dwgRSCodec::decode251I(tmpPageRaw, tmpPageRS, chunks);
    #ifdef DRW_DBG_DUMP
        DRW_DBG("\nSection OBJECTS RS data=\n");
        for (unsigned int i=0, j=0; i< pi.size;i++) {
            DRW_DBGH( (unsigned char)tmpPageRS[i]);
            if (j == 7) { DRW_DBG("\n"); j = 0;
            } else { DRW_DBG(", "); j++; }
        } DRW_DBG("\n");
    #endif

        DRW_DBG("\npage uncomp size: "); DRW_DBG(pi.uSize); DRW_DBG(" comp size: "); DRW_DBG(pi.cSize);
        DRW_DBG("\noffset: "); DRW_DBG(pi.startOffset);
        duint8 *pageData = dData + pi.startOffset;
        dwgCompressor::decompress21(tmpPageRS, pageData, pi.cSize, pi.uSize);

    #ifdef DRW_DBG_DUMP
        DRW_DBG("\n\nSection OBJECTS decompressed data=\n");
        for (unsigned int i=0, j=0; i< pi.uSize;i++) {
            DRW_DBGH( (unsigned char)pageData[i]);
            if (j == 7) { DRW_DBG("\n"); j = 0;
            } else { DRW_DBG(", "); j++; }
        } DRW_DBG("\n");
    #endif

        delete[]tmpPageRaw;
        delete[]tmpPageRS;
    }
    DRW_DBG("\n");
    return true;
}

bool dwgReader21::readFileHeader() {

    DRW_DBG("\n\ndwgReader21::parsing file header\n");
    if (! fileBuf->setPosition(0x80))
        return false;
    duint8 fileHdrRaw[0x2FD];//0x3D8
    fileBuf->getBytes(fileHdrRaw, 0x2FD);
    duint8 fileHdrdRS[0x2CD];
    dwgRSCodec::decode239I(fileHdrRaw, fileHdrdRS, 3);

#ifdef DRW_DBG_DUMP
    DRW_DBG("\ndwgReader21::parsed Reed Solomon decode:\n");
    int j = 0;
    for (int i=0, j=0; i<0x2CD; i++){
        DRW_DBGH( (unsigned char)fileHdrdRS[i]);
        if (j== 15){ j=0; DRW_DBG("\n");
        } else{ j++; DRW_DBG(", "); }
    } DRW_DBG("\n");
#endif

    dwgBuffer fileHdrBuf(fileHdrdRS, 0x2CD, &decoder);
    DRW_DBG("\nCRC 64b= "); DRW_DBGH(fileHdrBuf.getRawLong64());
    DRW_DBG("\nunknown key 64b= "); DRW_DBGH(fileHdrBuf.getRawLong64());
    DRW_DBG("\ncomp data CRC 64b= "); DRW_DBGH(fileHdrBuf.getRawLong64());
    dint32 fileHdrCompLength = fileHdrBuf.getRawLong32();
    DRW_DBG("\ncompr len 4bytes= "); DRW_DBG(fileHdrCompLength);
    dint32 fileHdrCompLength2 = fileHdrBuf.getRawLong32();
    DRW_DBG("\nlength2 4bytes= "); DRW_DBG(fileHdrCompLength2);

    int fileHdrDataLength = 0x110;
    duint8 *fileHdrData;
    if (fileHdrCompLength < 0) {
        fileHdrDataLength = fileHdrCompLength * -1;
        fileHdrData = new duint8[fileHdrDataLength];
        fileHdrBuf.getBytes(fileHdrData, fileHdrDataLength);
    }else {
        DRW_DBG("\ndwgReader21:: file header are compressed:\n");
        duint8 *compByteStr = new duint8[fileHdrCompLength];
        fileHdrBuf.getBytes(compByteStr, fileHdrCompLength);
        fileHdrData = new duint8[fileHdrDataLength];
        dwgCompressor::decompress21(compByteStr, fileHdrData, fileHdrCompLength, fileHdrDataLength);
        delete[] compByteStr;
    }

#ifdef DRW_DBG_DUMP
    DRW_DBG("\ndwgReader21::parsed file header:\n");
    for (int i=0, j=0; i<fileHdrDataLength; i++){
        DRW_DBGH( (unsigned char)fileHdrData[i]);
        if (j== 15){ j=0; DRW_DBG("\n");
        } else{ j++; DRW_DBG(", "); }
    } DRW_DBG("\n");
#endif

    dwgBuffer fileHdrDataBuf(fileHdrData, fileHdrDataLength, &decoder);
    DRW_DBG("\nHeader size = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nFile size = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nPagesMapCrcCompressed = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    duint64 PagesMapCorrectionFactor = fileHdrDataBuf.getRawLong64();
    DRW_DBG("\nPagesMapCorrectionFactor = "); DRW_DBG(PagesMapCorrectionFactor);
    DRW_DBG("\nPagesMapCrcSeed = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nPages map2offset = "); DRW_DBGH(fileHdrDataBuf.getRawLong64()); //relative to data page map 1, add 0x480 to get stream position
    DRW_DBG("\nPages map2Id = "); DRW_DBG(fileHdrDataBuf.getRawLong64());
    duint64 PagesMapOffset = fileHdrDataBuf.getRawLong64();
    DRW_DBG("\nPagesMapOffset = "); DRW_DBGH(PagesMapOffset); //relative to data page map 1, add 0x480 to get stream position
    DRW_DBG("\nPagesMapId = "); DRW_DBG(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nHeader2offset = "); DRW_DBGH(fileHdrDataBuf.getRawLong64()); //relative to data page map 1, add 0x480 to get stream position
    duint64 PagesMapSizeCompressed = fileHdrDataBuf.getRawLong64();
    DRW_DBG("\nPagesMapSizeCompressed = "); DRW_DBG(PagesMapSizeCompressed);
    duint64 PagesMapSizeUncompressed = fileHdrDataBuf.getRawLong64();
    DRW_DBG("\nPagesMapSizeUncompressed = "); DRW_DBG(PagesMapSizeUncompressed);
    DRW_DBG("\nPagesAmount = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    duint64 PagesMaxId = fileHdrDataBuf.getRawLong64();
    DRW_DBG("\nPagesMaxId = "); DRW_DBG(PagesMaxId);
    DRW_DBG("\nUnknown (normally 0x20) = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nUnknown (normally 0x40) = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nPagesMapCrcUncompressed = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nUnknown (normally 0xf800) = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nUnknown (normally 4) = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nUnknown (normally 1) = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nSectionsAmount (number of sections + 1) = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nSectionsMapCrcUncompressed = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    duint64 SectionsMapSizeCompressed = fileHdrDataBuf.getRawLong64();
    DRW_DBG("\nSectionsMapSizeCompressed = "); DRW_DBGH(SectionsMapSizeCompressed);
    DRW_DBG("\nSectionsMap2Id = "); DRW_DBG(fileHdrDataBuf.getRawLong64());
    duint64 SectionsMapId = fileHdrDataBuf.getRawLong64();
    DRW_DBG("\nSectionsMapId = "); DRW_DBG(SectionsMapId);
    duint64 SectionsMapSizeUncompressed = fileHdrDataBuf.getRawLong64();
    DRW_DBG("\nSectionsMapSizeUncompressed = "); DRW_DBGH(SectionsMapSizeUncompressed);
    DRW_DBG("\nSectionsMapCrcCompressed = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    duint64 SectionsMapCorrectionFactor = fileHdrDataBuf.getRawLong64();
    DRW_DBG("\nSectionsMapCorrectionFactor = "); DRW_DBG(SectionsMapCorrectionFactor);
    DRW_DBG("\nSectionsMapCrcSeed = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nStreamVersion (normally 0x60100) = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nCrcSeed = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nCrcSeedEncoded = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nRandomSeed = "); DRW_DBGH(fileHdrDataBuf.getRawLong64());
    DRW_DBG("\nHeader CRC64 = "); DRW_DBGH(fileHdrDataBuf.getRawLong64()); DRW_DBG("\n");

    delete[] fileHdrData;

    DRW_DBG("\ndwgReader21::parse page map:\n");
    duint8 *PagesMapData = new duint8[PagesMapSizeUncompressed];

    bool ret = parseSysPage(PagesMapSizeCompressed, PagesMapSizeUncompressed, PagesMapCorrectionFactor, 0x480+PagesMapOffset, PagesMapData);
    if (!ret) {
        delete[]PagesMapData;
        return false;
    }

    duint64 address = 0x480;
    duint64 i = 0;
    dwgBuffer PagesMapBuf(PagesMapData, PagesMapSizeUncompressed, &decoder);
    //stores temporaly info of all pages:
    std::map<duint32, dwgPageInfo >sectionPageMapTmp;

//    dwgPageInfo *m_pages= new dwgPageInfo[PagesMaxId+1];
    while (PagesMapSizeUncompressed > i ) {
        duint64 size = PagesMapBuf.getRawLong64();
        dint64 id = PagesMapBuf.getRawLong64();
        duint64 ind = id > 0 ? id : -id;
        i += 16;

        DRW_DBG("Page gap= "); DRW_DBG(id); DRW_DBG(" Page num= "); DRW_DBG(ind); DRW_DBG(" size= "); DRW_DBGH(size);
        DRW_DBG(" address= "); DRW_DBGH(address);  DRW_DBG("\n");
        sectionPageMapTmp[ind] = dwgPageInfo(ind, address,size);
        address += size;
        //TODO num can be negative indicating gap
//        seek += offset;
    }
    delete[]PagesMapData;

    DRW_DBG("\n*** dwgReader21: Processing Section Map ***\n");
    duint8 *SectionsMapData = new duint8[SectionsMapSizeUncompressed];
    dwgPageInfo sectionMap = sectionPageMapTmp[SectionsMapId];
    ret = parseSysPage(SectionsMapSizeCompressed, SectionsMapSizeUncompressed, SectionsMapCorrectionFactor, sectionMap.address, SectionsMapData);
    if (!ret)
        return false;

//reads sections:
    //Note: compressed value are not stored in file then, commpresed field are use to store
    // encoding value
    dwgBuffer SectionsMapBuf(SectionsMapData, SectionsMapSizeUncompressed, &decoder);
    duint8 nextId =1;
    while(SectionsMapBuf.getPosition() < SectionsMapBuf.size()){
        dwgSectionInfo secInfo;
        secInfo.size = SectionsMapBuf.getRawLong64();
        DRW_DBG("\nSize of section (data size)= "); DRW_DBGH(secInfo.size);
        secInfo.maxSize = SectionsMapBuf.getRawLong64();
        DRW_DBG("\nMax Decompressed Size= "); DRW_DBGH(secInfo.maxSize);
        secInfo.encrypted = SectionsMapBuf.getRawLong64();
        //encrypted (doc: 0 no, 1 yes, 2 unkn) on read: objects 0 and encrypted yes
        DRW_DBG("\nencription= "); DRW_DBGH(secInfo.encrypted);
        DRW_DBG("\nHashCode = "); DRW_DBGH(SectionsMapBuf.getRawLong64());
        duint64 SectionNameLength = SectionsMapBuf.getRawLong64();
        DRW_DBG("\nSectionNameLength = "); DRW_DBG(SectionNameLength);
        DRW_DBG("\nUnknown = "); DRW_DBGH(SectionsMapBuf.getRawLong64());
        secInfo.compressed = SectionsMapBuf.getRawLong64();
        DRW_DBG("\nEncoding (compressed) = "); DRW_DBGH(secInfo.compressed);
        secInfo.pageCount = SectionsMapBuf.getRawLong64();
        DRW_DBG("\nPage count= "); DRW_DBGH(secInfo.pageCount);
        secInfo.name = SectionsMapBuf.getUCSStr(SectionNameLength);
        DRW_DBG("\nSection name = "); DRW_DBG(secInfo.name); DRW_DBG("\n");

        for (unsigned int i=0; i< secInfo.pageCount; i++){
            duint64 po = SectionsMapBuf.getRawLong64();
            duint32 ds = SectionsMapBuf.getRawLong64();
            duint32 pn = SectionsMapBuf.getRawLong64();
            DRW_DBG("  pag Id = "); DRW_DBGH(pn); DRW_DBG(" data size = "); DRW_DBGH(ds);
            dwgPageInfo pi = sectionPageMapTmp[pn]; //get a copy
            pi.dataSize = ds;
            pi.startOffset = po;
            pi.uSize = SectionsMapBuf.getRawLong64();
            pi.cSize = SectionsMapBuf.getRawLong64();
            secInfo.pages[pn]= pi;//complete copy in secInfo
            DRW_DBG("\n    Page number= "); DRW_DBGH(secInfo.pages[pn].Id);
            DRW_DBG("\n    address in file= "); DRW_DBGH(secInfo.pages[pn].address);
            DRW_DBG("\n    size in file= "); DRW_DBGH(secInfo.pages[pn].size);
            DRW_DBG("\n    Data size= "); DRW_DBGH(secInfo.pages[pn].dataSize);
            DRW_DBG("\n    Start offset= "); DRW_DBGH(secInfo.pages[pn].startOffset);
            DRW_DBG("\n    Page uncompressed size = "); DRW_DBGH(secInfo.pages[pn].uSize);
            DRW_DBG("\n    Page compressed size = "); DRW_DBGH(secInfo.pages[pn].cSize);

            DRW_DBG("\n    Page checksum = "); DRW_DBGH(SectionsMapBuf.getRawLong64());
            DRW_DBG("\n    Page CRC = "); DRW_DBGH(SectionsMapBuf.getRawLong64()); DRW_DBG("\n");
        }

        if (!secInfo.name.empty()) {
            secInfo.Id = nextId++;
            DRW_DBG("Saved section Name= "); DRW_DBG( secInfo.name.c_str() ); DRW_DBG("\n");
            sections[secEnum::getEnum(secInfo.name)] = secInfo;
        }
    }
    delete[]SectionsMapData;

    if (! fileBuf->isGood())
        return false;

    DRW_DBG("\ndwgReader21::readFileHeader END\n");
    return true;
}

bool dwgReader21::readDwgHeader(DRW_Header& hdr){
    DRW_DBG("\ndwgReader21::readDwgHeader\n");
    dwgSectionInfo si = sections[secEnum::HEADER];
    if (si.Id<0)//not found, ends
        return false;
    DRW_DBG("\nprepare section of size "); DRW_DBG(si.size);DRW_DBG("\n");
    duint8 *tmpHeaderData = new duint8[si.size];
    bool ret = dwgReader21::parseDataPage(si, tmpHeaderData);
    if (!ret) {
        delete[]tmpHeaderData;
        return ret;
    }

    dwgBuffer dataBuf(tmpHeaderData, si.size, &decoder);
    dwgBuffer handleBuf(tmpHeaderData, si.size, &decoder);
    DRW_DBG("Header section sentinel= ");
    checkSentinel(&dataBuf, secEnum::HEADER, true);
    ret = dwgReader::readDwgHeader(hdr, &dataBuf, &handleBuf);
    delete[]tmpHeaderData;
    return ret;
}

bool dwgReader21::readDwgClasses(){
    DRW_DBG("\ndwgReader21::readDwgClasses");
    dwgSectionInfo si = sections[secEnum::CLASSES];
    if (si.Id<0)//not found, ends
        return false;

    DRW_DBG("\nprepare section of size "); DRW_DBG(si.size);DRW_DBG("\n");
    duint8 *tmpClassesData = new duint8[si.size];
    bool ret = dwgReader21::parseDataPage(si, tmpClassesData);
    if (!ret)
        return ret;

    dwgBuffer buff(tmpClassesData, si.size, &decoder);
    DRW_DBG("classes section sentinel= ");
    checkSentinel(&buff, secEnum::CLASSES, true);

    duint32 size = buff.getRawLong32();
    DRW_DBG("\ndata size in bytes "); DRW_DBG(size);

    duint32 bitSize = buff.getRawLong32();
    DRW_DBG("\ntotal size in bits "); DRW_DBG(bitSize);

    duint32 maxClassNum = buff.getBitShort();
    DRW_DBG("\nMaximum class number "); DRW_DBG(maxClassNum);
    DRW_DBG("\nRc 1 "); DRW_DBG(buff.getRawChar8());
    DRW_DBG("\nRc 2 "); DRW_DBG(buff.getRawChar8());
    DRW_DBG("\nBit "); DRW_DBG(buff.getBit());

    /*******************************/
    //prepare string stream
    dwgBuffer strBuff(tmpClassesData, si.size, &decoder);
    duint32 strStartPos = bitSize + 159;//size in bits + 20 bytes (sn+size) - 1 bit (endbit)
    DRW_DBG("\nstrStartPos: "); DRW_DBG(strStartPos);
    strBuff.setPosition(strStartPos >> 3);
    strBuff.setBitPos(strStartPos & 7);
    DRW_DBG("\nclasses strings buff.getPosition: "); DRW_DBG(strBuff.getPosition());
    DRW_DBG("\nclasses strings buff.getBitPos: "); DRW_DBG(strBuff.getBitPos());
    DRW_DBG("\nendBit "); DRW_DBG(strBuff.getBit());
    strStartPos -= 16;//decrement 16 bits
    DRW_DBG("\nstrStartPos: "); DRW_DBG(strStartPos);
    strBuff.setPosition(strStartPos >> 3);
    strBuff.setBitPos(strStartPos & 7);
    DRW_DBG("\nclasses strings buff.getPosition: "); DRW_DBG(strBuff.getPosition());
    DRW_DBG("\nclasses strings buff.getBitPos: "); DRW_DBG(strBuff.getBitPos());
    duint32 strDataSize = strBuff.getRawShort16();
    DRW_DBG("\nstrDataSize: "); DRW_DBG(strDataSize);
    if (strDataSize & 0x8000) {
        strStartPos -= 16;//decrement 16 bits
        strDataSize &= 0x7FFF; //strip 0x8000;
        strBuff.setPosition(strStartPos >> 3);
        strBuff.setBitPos(strStartPos & 7);
        duint32 hiSize = strBuff.getRawShort16();
        strDataSize |= (hiSize << 15);
    }
    strStartPos -= strDataSize;
    DRW_DBG("\nstrStartPos: "); DRW_DBG(strStartPos);
    strBuff.setPosition(strStartPos >> 3);
    strBuff.setBitPos(strStartPos & 7);
    DRW_DBG("\nclasses strings buff.getPosition: "); DRW_DBG(strBuff.getPosition());
    DRW_DBG("\nclasses strings buff.getBitPos: "); DRW_DBG(strBuff.getBitPos());


    /*******************************/

    duint32 endDataPos = maxClassNum-499;
    DRW_DBG("\nbuff.getPosition: "); DRW_DBG(buff.getPosition());
    for (duint32 i= 0; i<endDataPos;i++) {
        DRW_Class *cl = new DRW_Class();
        cl->parseDwg(version, &buff, &strBuff);
        classesmap[cl->classNum] = cl;
        DRW_DBG("\nbuff.getPosition: "); DRW_DBG(buff.getPosition());
    }
    DRW_DBG("\nend classes data buff.getPosition: "); DRW_DBG(buff.getPosition());
    DRW_DBG("\nend classes data buff.getBitPos: "); DRW_DBG(buff.getBitPos());

    buff.setPosition(size+20);//sizeVal+sn+32bSize
    DRW_DBG("\nCRC: "); DRW_DBGH(buff.getRawShort16());
    DRW_DBG("\nclasses section end sentinel= ");
    checkSentinel(&buff, secEnum::CLASSES, true);
    delete[]tmpClassesData;
    return buff.isGood();
}


bool dwgReader21::readDwgHandles(){
    DRW_DBG("\ndwgReader21::readDwgHandles");
    dwgSectionInfo si = sections[secEnum::HANDLES];
    if (si.Id<0)//not found, ends
        return false;

    DRW_DBG("\nprepare section of size "); DRW_DBG(si.size);DRW_DBG("\n");
    duint8 *tmpHandlesData = new duint8[si.size];
    bool ret = dwgReader21::parseDataPage(si, tmpHandlesData);
    if (!ret)
        return ret;

    dwgBuffer dataBuf(tmpHandlesData, si.size, &decoder);

    ret = dwgReader::readDwgHandles(&dataBuf, 0, si.size);
    delete[]tmpHandlesData;
    return ret;
}

/*********** objects ************************/
/**
 * Reads all the object referenced in the object map section of the DWG file
 * (using their object file offsets)
 */
bool dwgReader21::readDwgTables(DRW_Header& hdr) {
    DRW_DBG("\ndwgReader21::readDwgTables\n");
    dwgSectionInfo si = sections[secEnum::OBJECTS];
    if (si.Id<0)//not found, ends
        return false;

    DRW_DBG("\nprepare section of size "); DRW_DBG(si.size);DRW_DBG("\n");
    dataSize = si.size;
    objData.reset( new duint8 [dataSize] );
    bool ret = dwgReader21::parseDataPage(si, objData.get());
    if (!ret)
        return ret;

    DRW_DBG("readDwgTables total data size= "); DRW_DBG(dataSize); DRW_DBG("\n");
    dwgBuffer dataBuf(objData.get(), dataSize, &decoder);
    ret = dwgReader::readDwgTables(hdr, &dataBuf);

    return ret;
}


bool dwgReader21::readDwgBlocks(DRW_Interface& intfa){
    bool ret = true;
    dwgBuffer dataBuf(objData.get(), dataSize, &decoder);
    ret = dwgReader::readDwgBlocks(intfa, &dataBuf);
    return ret;
}


