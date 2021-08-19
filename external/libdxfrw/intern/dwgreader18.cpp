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
#include "dwgreader18.h"
#include "dwgutil.h"
#include "drw_textcodec.h"
#include "../libdwgr.h"

void dwgReader18::genMagicNumber(){
    int size =0x114;
    duint8 *tmpMagicStr = new duint8[size];
    duint8 *p = tmpMagicStr;
    int rSeed =1;
    while (size--) {
        rSeed *= 0x343fd;
        rSeed += 0x269ec3;
        *p++ = static_cast<duint8>(rSeed >> 0x10);
    }
    int j = 0;
    size =0x114;
    for (int i=0; i< size;i++) {
        DRW_DBGH(tmpMagicStr[i]);
        if (j == 15) {
            DRW_DBG("\n");
            j = 0;
        } else {
            DRW_DBG(", ");
            j++;
        }
    }
    delete[]tmpMagicStr;
}

duint32 dwgReader18::checksum(duint32 seed, duint8* data, duint32 sz){
    duint32 size = sz;
    duint32 sum1 = seed & 0xffff;
    duint32 sum2 = seed >> 0x10;
    while (size != 0) {
//        duint32 chunkSize = min(0x15b0, size);
        duint32 chunkSize = 0x15b0 < size? 0x15b0:size;
        size -= chunkSize;
        for (duint32 i = 0; i < chunkSize; i++) {
            sum1 += *data++;
            sum2 += sum1;
        }
        sum1 %= 0xFFF1;
        sum2 %= 0xFFF1;
    }
    return (sum2 << 0x10) | (sum1 & 0xffff);
}

 //called: Section page map: 0x41630e3b
void dwgReader18::parseSysPage(duint8 *decompSec, duint32 decompSize){
    DRW_DBG("\nparseSysPage:\n ");
    duint32 compSize = fileBuf->getRawLong32();
    DRW_DBG("Compressed size= "); DRW_DBG(compSize); DRW_DBG(", "); DRW_DBGH(compSize);
    DRW_DBG("\nCompression type= "); DRW_DBGH(fileBuf->getRawLong32());
    DRW_DBG("\nSection page checksum= "); DRW_DBGH(fileBuf->getRawLong32()); DRW_DBG("\n");

    duint8 hdrData[20];
    fileBuf->moveBitPos(-160);
    fileBuf->getBytes(hdrData, 20);
    for (duint8 i= 16; i<20; ++i)
        hdrData[i]=0;
    duint32 calcsH = checksum(0, hdrData, 20);
    DRW_DBG("Calc hdr checksum= "); DRW_DBGH(calcsH);
    duint8 *tmpCompSec = new duint8[compSize];
    fileBuf->getBytes(tmpCompSec, compSize);
    duint32 calcsD = checksum(calcsH, tmpCompSec, compSize);
    DRW_DBG("\nCalc data checksum= "); DRW_DBGH(calcsD); DRW_DBG("\n");

#ifdef DRW_DBG_DUMP
    for (unsigned int i=0, j=0; i< compSize;i++) {
        DRW_DBGH( (unsigned char)compSec[i]);
        if (j == 7) { DRW_DBG("\n"); j = 0;
        } else { DRW_DBG(", "); j++; }
    } DRW_DBG("\n");
#endif
    DRW_DBG("decompressing "); DRW_DBG(compSize); DRW_DBG(" bytes in "); DRW_DBG(decompSize); DRW_DBG(" bytes\n");
    dwgCompressor comp;
    comp.decompress18(tmpCompSec, decompSec, compSize, decompSize);
#ifdef DRW_DBG_DUMP
    for (unsigned int i=0, j=0; i< decompSize;i++) {
        DRW_DBGH( decompSec[i]);
        if (j == 7) { DRW_DBG("\n"); j = 0;
        } else { DRW_DBG(", "); j++; }
    } DRW_DBG("\n");
#endif
    delete[]tmpCompSec;
}

 //called ???: Section map: 0x4163003b
bool dwgReader18::parseDataPage(const dwgSectionInfo &si/*, duint8 *dData*/){
    DRW_DBG("\nparseDataPage\n ");
    objData.reset( new duint8 [si.pageCount * si.maxSize] );

    for (auto it=si.pages.begin(); it!=si.pages.end(); ++it){
        dwgPageInfo pi = it->second;
        if (!fileBuf->setPosition(pi.address))
            return false;
        //decript section header
        duint8 hdrData[32];
        fileBuf->getBytes(hdrData, 32);
        dwgCompressor::decrypt18Hdr(hdrData, 32, pi.address);
        DRW_DBG("Section  "); DRW_DBG(si.name); DRW_DBG(" page header=\n");
        for (unsigned int i=0, j=0; i< 32;i++) {
            DRW_DBGH( static_cast<unsigned char>(hdrData[i]));
            if (j == 7) {
                DRW_DBG("\n");
                j = 0;
            } else {
                DRW_DBG(", ");
                j++;
            }
        } DRW_DBG("\n");

        DRW_DBG("\n    Page number= "); DRW_DBGH(pi.Id);
        DRW_DBG("\n    size in file= "); DRW_DBGH(pi.size);
        DRW_DBG("\n    address in file= "); DRW_DBGH(pi.address);
        DRW_DBG("\n    Data size= "); DRW_DBGH(pi.dataSize);
        DRW_DBG("\n    Start offset= "); DRW_DBGH(pi.startOffset); DRW_DBG("\n");
        dwgBuffer bufHdr(hdrData, 32, &decoder);
        DRW_DBG("      section page type= "); DRW_DBGH(bufHdr.getRawLong32());
        DRW_DBG("\n      section number= "); DRW_DBGH(bufHdr.getRawLong32());
        pi.cSize = bufHdr.getRawLong32();
        DRW_DBG("\n      data size (compressed)= "); DRW_DBGH(pi.cSize); DRW_DBG(" dec "); DRW_DBG(pi.cSize);
        pi.uSize = bufHdr.getRawLong32();
        DRW_DBG("\n      page size (decompressed)= "); DRW_DBGH(pi.uSize); DRW_DBG(" dec "); DRW_DBG(pi.uSize);
        DRW_DBG("\n      start offset (in decompressed buffer)= "); DRW_DBGH(bufHdr.getRawLong32());
        DRW_DBG("\n      unknown= "); DRW_DBGH(bufHdr.getRawLong32());
        DRW_DBG("\n      header checksum= "); DRW_DBGH(bufHdr.getRawLong32());
        DRW_DBG("\n      data checksum= "); DRW_DBGH(bufHdr.getRawLong32()); DRW_DBG("\n");

        //get compressed data
        duint8 *cData = new duint8[pi.cSize];
        if (!fileBuf->setPosition(pi.address+32))
        {
            delete [] cData;
            return false;
        }
        fileBuf->getBytes(cData, pi.cSize);

        //calculate checksum
        duint32 calcsD = checksum(0, cData, pi.cSize);
        for (duint8 i= 24; i<28; ++i)
            hdrData[i]=0;
        duint32 calcsH = checksum(calcsD, hdrData, 32);
        DRW_DBG("Calc header checksum= "); DRW_DBGH(calcsH);
        DRW_DBG("\nCalc data checksum= "); DRW_DBGH(calcsD); DRW_DBG("\n");

        duint8* oData = objData.get() + pi.startOffset;
        pi.uSize = si.maxSize;
        DRW_DBG("decompressing "); DRW_DBG(pi.cSize); DRW_DBG(" bytes in "); DRW_DBG(pi.uSize); DRW_DBG(" bytes\n");
        dwgCompressor comp;
        comp.decompress18(cData, oData, pi.cSize, pi.uSize);
        delete[]cData;
    }
    return true;
}

bool dwgReader18::readMetaData() {
    version = parent->getVersion();
    decoder.setVersion(version, false);
    DRW_DBG("dwgReader18::readMetaData\n");
    if (! fileBuf->setPosition(11))
        return false;
    maintenanceVersion = fileBuf->getRawChar8();
    DRW_DBG("maintenance version= "); DRW_DBGH(maintenanceVersion);
    DRW_DBG("\nbyte at 0x0C= "); DRW_DBGH(fileBuf->getRawChar8());
    previewImagePos = fileBuf->getRawLong32(); //+ page header size (0x20).
    DRW_DBG("\npreviewImagePos (seekerImageData) = "); DRW_DBG(previewImagePos);
    DRW_DBG("\napp Dwg version= "); DRW_DBGH(fileBuf->getRawChar8()); DRW_DBG(", ");
    DRW_DBG("\napp maintenance version= "); DRW_DBGH(fileBuf->getRawChar8());
    duint16 cp = fileBuf->getRawShort16();
    DRW_DBG("\ncodepage= "); DRW_DBG(cp);
    if (cp == 30)
        decoder.setCodePage("ANSI_1252", false);
    DRW_DBG("\n3 0x00 bytes(seems 0x00, appDwgV & appMaintV) = "); DRW_DBGH(fileBuf->getRawChar8()); DRW_DBG(", ");
    DRW_DBGH(fileBuf->getRawChar8()); DRW_DBG(", "); DRW_DBGH(fileBuf->getRawChar8());
    securityFlags = fileBuf->getRawLong32();
    DRW_DBG("\nsecurity flags= "); DRW_DBG(securityFlags);
    // UNKNOWN SECTION 4 bytes
    duint32 uk =    fileBuf->getRawLong32();
    DRW_DBG("\nUNKNOWN SECTION ( 4 bytes) = "); DRW_DBG(uk);
    duint32 sumInfoAddr =    fileBuf->getRawLong32();
    DRW_DBG("\nsummary Info Address= "); DRW_DBG(sumInfoAddr);
    duint32 vbaAdd =    fileBuf->getRawLong32();
    DRW_DBG("\nVBA address= "); DRW_DBGH(vbaAdd);
    DRW_DBG("\npos 0x28 are 0x00000080= "); DRW_DBGH(fileBuf->getRawLong32());
     DRW_DBG("\n");
    return true;
}

bool dwgReader18::readFileHeader() {

    if (! fileBuf->setPosition(0x80))
        return false;

//    genMagicNumber(); DBG("\n"); DBG("\n");
    DRW_DBG("Encrypted Header Data=\n");
    duint8 byteStr[0x6C];
    int size =0x6C;
    for (int i=0, j=0; i< 0x6C;i++) {
        duint8 ch = fileBuf->getRawChar8();
        DRW_DBGH(ch);
        if (j == 15) {
            DRW_DBG("\n");
            j = 0;
        } else {
            DRW_DBG(", ");
            j++;
        }
        byteStr[i] = DRW_magicNum18[i] ^ ch;
    }
    DRW_DBG("\n");

//    size =0x6C;
    DRW_DBG("Decrypted Header Data=\n");
    for (int i=0, j = 0; i< size;i++) {
        DRW_DBGH( static_cast<unsigned char>(byteStr[i]));
        if (j == 15) {
            DRW_DBG("\n");
            j = 0;
        } else {
            DRW_DBG(", ");
            j++;
        }
    }
    dwgBuffer buff(byteStr, 0x6C, &decoder);
    std::string name = reinterpret_cast<char*>(byteStr);
    DRW_DBG("\nFile ID string (AcFssFcAJMB)= "); DRW_DBG(name.c_str());
    //ID string + NULL = 12
    buff.setPosition(12);
    DRW_DBG("\n0x00 long= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\n0x6c long= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\n0x04 long= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\nRoot tree node gap= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\nLowermost left tree node gap= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\nLowermost right tree node gap= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\nUnknown long (1)= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\nLast section page Id= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\nLast section page end address 64b= "); DRW_DBGH(buff.getRawLong64());
    DRW_DBG("\nStart of second header data address 64b= "); DRW_DBGH(buff.getRawLong64());
    DRW_DBG("\nGap amount= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\nSection page amount= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\n0x20 long= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\n0x80 long= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\n0x40 long= "); DRW_DBGH(buff.getRawLong32());
    dint32 secPageMapId = buff.getRawLong32();
    DRW_DBG("\nSection Page Map Id= "); DRW_DBGH(secPageMapId);
    duint64 secPageMapAddr = buff.getRawLong64()+0x100;
    DRW_DBG("\nSection Page Map address 64b= "); DRW_DBGH(secPageMapAddr);
    DRW_DBG("\nSection Page Map address 64b dec= "); DRW_DBG(secPageMapAddr);
    duint32 secMapId = buff.getRawLong32();
    DRW_DBG("\nSection Map Id= "); DRW_DBGH(secMapId);
    DRW_DBG("\nSection page array size= "); DRW_DBGH(buff.getRawLong32());
    DRW_DBG("\nGap array size= "); DRW_DBGH(buff.getRawLong32());
    //TODO: verify CRC
    DRW_DBG("\nCRC32= "); DRW_DBGH(buff.getRawLong32());
    for (duint8 i = 0x68; i < 0x6c; ++i)
        byteStr[i] = '\0';
//    byteStr[i] = '\0';
    duint32 crcCalc = buff.crc32(0x00,0,0x6C);
    DRW_DBG("\nCRC32 calculated= "); DRW_DBGH(crcCalc);

    DRW_DBG("\nEnd Encrypted Data. Reads 0x14 bytes, equal to magic number:\n");
    for (int i=0, j=0; i< 0x14;i++) {
        DRW_DBG("magic num: "); DRW_DBGH( static_cast<unsigned char>(DRW_magicNumEnd18[i]));
        DRW_DBG(",read "); DRW_DBGH( static_cast<unsigned char>(fileBuf->getRawChar8()));
        if (j == 3) {
            DRW_DBG("\n");
            j = 0;
        } else {
            DRW_DBG(", ");
            j++;
        }
    }
// At this point are parsed the first 256 bytes
    DRW_DBG("\nJump to Section Page Map address: "); DRW_DBGH(secPageMapAddr);

    if (! fileBuf->setPosition(secPageMapAddr))
        return false;
    duint32 pageType = fileBuf->getRawLong32();
    DRW_DBG("\nSection page type= "); DRW_DBGH(pageType);
    duint32 decompSize = fileBuf->getRawLong32();
    DRW_DBG("\nDecompressed size= "); DRW_DBG(decompSize); DRW_DBG(", "); DRW_DBGH(decompSize);
    if (pageType != 0x41630e3b){
        //bad page type, ends
        DRW_DBG("Warning, bad page type, was expected 0x41630e3b instead of");  DRW_DBGH(pageType); DRW_DBG("\n");
        return false;
    }
    duint8 *tmpDecompSec = new duint8[decompSize];
    parseSysPage(tmpDecompSec, decompSize);

//parses "Section page map" decompressed data
    dwgBuffer buff2(tmpDecompSec, decompSize, &decoder);
    duint32 address = 0x100;
    //stores temporaly info of all pages:
    std::map<duint32, dwgPageInfo >sectionPageMapTmp;

    for (unsigned int i = 0; i < decompSize;) {
        dint32 id = buff2.getRawLong32();//RLZ bad can be +/-
        duint32 size = buff2.getRawLong32();
        i += 8;
        DRW_DBG("Page num= "); DRW_DBG(id); DRW_DBG(" size= "); DRW_DBGH(size);
        DRW_DBG(" address= "); DRW_DBGH(address);  DRW_DBG("\n");
        //TODO num can be negative indicating gap
//        duint64 ind = id > 0 ? id : -id;
        if (id < 0){
            DRW_DBG("Parent= "); DRW_DBG(buff2.getRawLong32());
            DRW_DBG("\nLeft= "); DRW_DBG(buff2.getRawLong32());
            DRW_DBG(", Right= "); DRW_DBG(buff2.getRawLong32());
            DRW_DBG(", 0x00= ");DRW_DBGH(buff2.getRawLong32()); DRW_DBG("\n");
            i += 16;
        }

        sectionPageMapTmp[id] = dwgPageInfo(id, address, size);
        address += size;
    }
    delete[]tmpDecompSec;

    DRW_DBG("\n*** dwgReader18: Processing Data Section Map ***\n");
    dwgPageInfo sectionMap = sectionPageMapTmp[secMapId];
    if (!fileBuf->setPosition(sectionMap.address))
        return false;
    pageType = fileBuf->getRawLong32();
    DRW_DBG("\nSection page type= "); DRW_DBGH(pageType);
    decompSize = fileBuf->getRawLong32();
    DRW_DBG("\nDecompressed size= "); DRW_DBG(decompSize); DRW_DBG(", "); DRW_DBGH(decompSize);
    if (pageType != 0x4163003b){
        //bad page type, ends
        DRW_DBG("Warning, bad page type, was expected 0x4163003b instead of");  DRW_DBGH(pageType); DRW_DBG("\n");
        return false;
    }
    tmpDecompSec = new duint8[decompSize];
    parseSysPage(tmpDecompSec, decompSize);

//reads sections:
    DRW_DBG("\n*** dwgReader18: reads sections:");
    dwgBuffer buff3(tmpDecompSec, decompSize, &decoder);
    duint32 numDescriptions = buff3.getRawLong32();
    DRW_DBG("\nnumDescriptions (sections)= "); DRW_DBG(numDescriptions);
    DRW_DBG("\n0x02 long= "); DRW_DBGH(buff3.getRawLong32());
    DRW_DBG("\n0x00007400 long= "); DRW_DBGH(buff3.getRawLong32());
    DRW_DBG("\n0x00 long= "); DRW_DBGH(buff3.getRawLong32());
    DRW_DBG("\nunknown long (numDescriptions?)= "); DRW_DBG(buff3.getRawLong32()); DRW_DBG("\n");

    for (unsigned int i = 0; i < numDescriptions; i++) {
        dwgSectionInfo secInfo;
        secInfo.size = buff3.getRawLong64();
        DRW_DBG("\nSize of section= "); DRW_DBGH(secInfo.size);
        secInfo.pageCount = buff3.getRawLong32();
        DRW_DBG("\nPage count= "); DRW_DBGH(secInfo.pageCount);
        secInfo.maxSize = buff3.getRawLong32();
        DRW_DBG("\nMax Decompressed Size= "); DRW_DBGH(secInfo.maxSize);
        DRW_DBG("\nunknown long= "); DRW_DBGH(buff3.getRawLong32());
        secInfo.compressed = buff3.getRawLong32();
        DRW_DBG("\nis Compressed? 1:no, 2:yes= "); DRW_DBGH(secInfo.compressed);
        secInfo.Id = buff3.getRawLong32();
        DRW_DBG("\nSection Id= "); DRW_DBGH(secInfo.Id);
        secInfo.encrypted = buff3.getRawLong32();
        //encrypted (doc: 0 no, 1 yes, 2 unkn) on read: objects 0 and encrypted yes
        DRW_DBG("\nEncrypted= "); DRW_DBGH(secInfo.encrypted);
        duint8 nameCStr[64];
        buff3.getBytes(nameCStr, 64);
        secInfo.name = reinterpret_cast<char*>(nameCStr);
        DRW_DBG("\nSection std::Name= "); DRW_DBG( secInfo.name.c_str() ); DRW_DBG("\n");
        for (unsigned int i = 0; i < secInfo.pageCount; i++){
            duint32 pn = buff3.getRawLong32();
            dwgPageInfo pi = sectionPageMapTmp[pn]; //get a copy
            DRW_DBG(" reading pag num = "); DRW_DBGH(pn);
            pi.dataSize = buff3.getRawLong32();
            pi.startOffset = buff3.getRawLong64();
            secInfo.pages[pn]= pi;//complete copy in secInfo
            DRW_DBG("\n    Page number= "); DRW_DBGH(secInfo.pages[pn].Id);
            DRW_DBG("\n    size in file= "); DRW_DBGH(secInfo.pages[pn].size);
            DRW_DBG("\n    address in file= "); DRW_DBGH(secInfo.pages[pn].address);
            DRW_DBG("\n    Data size= "); DRW_DBGH(secInfo.pages[pn].dataSize);
            DRW_DBG("\n    Start offset= "); DRW_DBGH(secInfo.pages[pn].startOffset); DRW_DBG("\n");
        }
        //do not save empty section
        if (!secInfo.name.empty()) {
            DRW_DBG("Saved section Name= "); DRW_DBG( secInfo.name.c_str() ); DRW_DBG("\n");
            sections[secEnum::getEnum(secInfo.name)] = secInfo;
        }
    }
    delete[]tmpDecompSec;

    if (! fileBuf->isGood())
        return false;
    DRW_DBG("\ndwgReader18::readFileHeader END\n\n");
    return true;
}

bool dwgReader18::readDwgHeader(DRW_Header& hdr){
    DRW_DBG("dwgReader18::readDwgHeader\n");
    dwgSectionInfo si = sections[secEnum::HEADER];
    if (si.Id<0)//not found, ends
        return false;
    bool ret = parseDataPage(si/*, objData*/);
    //global store for uncompressed data of all pages
    uncompSize=si.size;
    if (ret) {
        dwgBuffer dataBuf(objData.get(), si.size, &decoder);
        DRW_DBG("Header section sentinel= ");
        checkSentinel(&dataBuf, secEnum::HEADER, true);
        if (version == DRW::AC1018){
            ret = dwgReader::readDwgHeader(hdr, &dataBuf, &dataBuf);
        } else {
            dwgBuffer handleBuf(objData.get(), si.size, &decoder);
            ret = dwgReader::readDwgHeader(hdr, &dataBuf, &handleBuf);
        }
    }
    //Cleanup: global store for uncompressed data of all pages
    objData.reset();
    return ret;
}


bool dwgReader18::readDwgClasses(){
    DRW_DBG("\ndwgReader18::readDwgClasses\n");
    dwgSectionInfo si = sections[secEnum::CLASSES];
    if (si.Id<0)//not found, ends
        return false;
    bool ret = parseDataPage(si/*, objData*/);
    //global store for uncompressed data of all pages
    uncompSize=si.size;
    if (ret) {

    dwgBuffer dataBuf(objData.get(), uncompSize, &decoder);

    DRW_DBG("classes section sentinel= ");
    checkSentinel(&dataBuf, secEnum::CLASSES, true);

    duint32 size = dataBuf.getRawLong32();
    DRW_DBG("\ndata size in bytes "); DRW_DBG(size);
    if (version > DRW::AC1021 && maintenanceVersion > 3) { //2010+
        duint32 hSize = dataBuf.getRawLong32();
        DRW_DBG("\n2010+ & MV> 3, height 32b: "); DRW_DBG(hSize);
    }
    duint32 bitSize = 0;
    if (version > DRW::AC1021) {//2007+
    bitSize = dataBuf.getRawLong32();
    DRW_DBG("\ntotal size in bits "); DRW_DBG(bitSize);
}
    duint32 maxClassNum = dataBuf.getBitShort();
    DRW_DBG("\nMaximum class number "); DRW_DBG(maxClassNum);
    DRW_DBG("\nRc 1 "); DRW_DBG(dataBuf.getRawChar8());
    DRW_DBG("\nRc 2 "); DRW_DBG(dataBuf.getRawChar8());
    DRW_DBG("\nBit "); DRW_DBG(dataBuf.getBit());

    /*******************************/
    dwgBuffer *strBuf = &dataBuf;
    dwgBuffer strBuff(objData.get(), uncompSize, &decoder);
    //prepare string stream for 2007+
    if (version > DRW::AC1021) {//2007+
        strBuf = &strBuff;
        duint32 strStartPos = bitSize+191;//size in bits + 24 bytes (sn+size+hSize) - 1 bit (endbit)
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
    }

    /*******************************/

    duint32 endDataPos = maxClassNum-499;
    DRW_DBG("\nbuff.getPosition: "); DRW_DBG(dataBuf.getPosition());
    for (duint32 i= 0; i<endDataPos;i++) {
        DRW_Class *cl = new DRW_Class();
        cl->parseDwg(version, &dataBuf, strBuf);
        classesmap[cl->classNum] = cl;
        DRW_DBG("\nbuff.getPosition: "); DRW_DBG(dataBuf.getPosition());
    }
    DRW_DBG("\nend classes data buff.getPosition: "); DRW_DBG(dataBuf.getPosition());
    DRW_DBG("\nend classes data buff.getBitPos: "); DRW_DBG(dataBuf.getBitPos());
    DRW_DBG("\nend classes strings buff.getPosition: "); DRW_DBG(strBuf->getPosition());
    DRW_DBG("\nend classes strings buff.getBitPos: "); DRW_DBG(strBuf->getBitPos());

/***************/

    strBuf->setPosition(strBuf->getPosition()+1);//skip remaining bits
    DRW_DBG("\nCRC: "); DRW_DBGH(strBuf->getRawShort16());
    if (version > DRW::AC1018){
        DRW_DBG("\nunknown CRC: "); DRW_DBGH(strBuf->getRawShort16());
    }
    DRW_DBG("\nclasses section end sentinel= ");
    checkSentinel(strBuf, secEnum::CLASSES, false);

    ret = strBuf->isGood();
    }
    //Cleanup: global store for uncompressed data of all pages
    objData.reset();
    return ret;
}


/*********** objects map ************************/
/** Note: object map are split in sections with max size 2035?
 *  heach section are 2 bytes size + data bytes + 2 bytes crc
 *  size value are data bytes + 2 and to calculate crc are used
 *  2 bytes size + data bytes
 *  last section are 2 bytes size + 2 bytes crc (size value always 2)
**/
bool dwgReader18::readDwgHandles() {
    DRW_DBG("\ndwgReader18::readDwgHandles\n");
    dwgSectionInfo si = sections[secEnum::HANDLES];
    if (si.Id<0)//not found, ends
        return false;
    bool ret = parseDataPage(si);
    //global store for uncompressed data of all pages
    uncompSize=si.size;
    if (ret) {

        dwgBuffer dataBuf(objData.get(), uncompSize, &decoder);

        ret = dwgReader::readDwgHandles(&dataBuf, 0, si.size);
    }
    //Cleanup: global store for uncompressed data of all pages
    if (objData){
        objData.reset();
        uncompSize = 0;
    }
    return ret;
}


/*********** objects ************************/
/**
 * Reads all the object referenced in the object map section of the DWG file
 * (using their object file offsets)
 */
bool dwgReader18::readDwgTables(DRW_Header& hdr) {
    DRW_DBG("\ndwgReader18::readDwgTables\n");
    dwgSectionInfo si = sections[secEnum::OBJECTS];

    if (si.Id<0)//not found, ends
        return false;
    bool ret = parseDataPage(si/*, objData*/);
    //global store for uncompressed data of all pages
    uncompSize=si.size;
    if (ret) {

        dwgBuffer dataBuf(objData.get(), uncompSize, &decoder);

        ret = dwgReader::readDwgTables(hdr, &dataBuf);

    }
    //Do not delete objData in this point, needed in the remaining code
    return ret;
}


