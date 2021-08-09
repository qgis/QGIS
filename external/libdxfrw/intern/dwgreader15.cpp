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
#include "dwgreader15.h"
#include "drw_textcodec.h"
#include "../libdwgr.h"

bool dwgReader15::readMetaData() {
    version = parent->getVersion();
    decoder.setVersion(version, false);
    DRW_DBG("dwgReader15::readMetaData\n");
    if (! fileBuf->setPosition(13))
        return false;
    previewImagePos = fileBuf->getRawLong32();
    DRW_DBG("previewImagePos (seekerImageData) = "); DRW_DBG(previewImagePos);
    /* MEASUREMENT system variable 2 bytes*/
    duint16 meas = fileBuf->getRawShort16();
    DRW_DBG("\nMEASUREMENT (0 = English, 1 = Metric)= "); DRW_DBG(meas);
    duint16 cp = fileBuf->getRawShort16();
    DRW_DBG("\ncodepage= "); DRW_DBG(cp); DRW_DBG("\n");
    if (cp == 29) //TODO RLZ: locate wath code page and correct this
        decoder.setCodePage("ANSI_1252", false);
    if (cp == 30)
        decoder.setCodePage("ANSI_1252", false);
    return true;
}

bool dwgReader15::readFileHeader() {
    bool ret = true;
    DRW_DBG("dwgReader15::readFileHeader\n");
    if (! fileBuf->setPosition(21))
        return false;
    duint32 count = fileBuf->getRawLong32();
    DRW_DBG("count records= "); DRW_DBG(count); DRW_DBG("\n");

    for (unsigned int i = 0; i < count; i++) {
        duint8 rec = fileBuf->getRawChar8();
        duint32 address = fileBuf->getRawLong32();
        duint32 size = fileBuf->getRawLong32();
        dwgSectionInfo si;
        si.Id = rec;
        si.size = size;
        si.address = address;
        if (rec == 0) {
            DRW_DBG("\nSection HEADERS address= ");
            DRW_DBG(address); DRW_DBG(" size= "); DRW_DBG(size);
            sections[secEnum::HEADER] = si;
        } else if (rec == 1) {
            DRW_DBG("\nSection CLASSES address= ");
            DRW_DBG(address); DRW_DBG(" size= "); DRW_DBG(size);
            sections[secEnum::CLASSES] = si;
        } else if (rec == 2) {
            DRW_DBG("\nSection OBJECTS (handles) address= ");
            DRW_DBG(address); DRW_DBG(" size= "); DRW_DBG(size);
            sections[secEnum::HANDLES] = si;
        } else if (rec == 3) {
            DRW_DBG("\nSection UNKNOWN address= ");
            DRW_DBG(address); DRW_DBG(" size= "); DRW_DBG(size);
            sections[secEnum::UNKNOWNS] = si;
        } else if (rec == 4) {
            DRW_DBG("\nSection R14DATA (AcDb:Template) address= ");
            DRW_DBG(address); DRW_DBG(" size= "); DRW_DBG(size);
            sections[secEnum::TEMPLATE] = si;
        } else if (rec == 5) {
            DRW_DBG("\nSection R14REC5 (AcDb:AuxHeader) address= ");
            DRW_DBG(address); DRW_DBG(" size= "); DRW_DBG(size);
            sections[secEnum::AUXHEADER] = si;
        } else {
            std::cerr << "\nUnsupported section number\n";
        }
    }
    if (! fileBuf->isGood())
        return false;
    DRW_DBG("\nposition after read section locator records= "); DRW_DBG(fileBuf->getPosition());
    DRW_DBG(", bit are= "); DRW_DBG(fileBuf->getBitPos());
    duint32 ckcrc = fileBuf->crc8(0,0,fileBuf->getPosition());
    DRW_DBG("\nfile header crc8 0 result= "); DRW_DBG(ckcrc);
    switch (count){
    case 3:
        ckcrc = ckcrc ^ 0xA598;
        break;
    case 4:
        ckcrc = ckcrc ^ 0x8101;
        break;
    case 5:
        ckcrc = ckcrc ^ 0x3CC4;
        break;
    case 6:
        ckcrc = ckcrc ^ 0x8461;
    }
    DRW_DBG("\nfile header crc8 xor result= "); DRW_DBG(ckcrc);
    DRW_DBG("\nfile header CRC= "); DRW_DBG(fileBuf->getRawShort16());
    DRW_DBG("\nfile header sentinel= ");
    checkSentinel(fileBuf, secEnum::FILEHEADER, false);

    DRW_DBG("\nposition after read file header sentinel= "); DRW_DBG(fileBuf->getPosition());
    DRW_DBG(", bit are= "); DRW_DBG(fileBuf->getBitPos());

    DRW_DBG("\ndwgReader15::readFileHeader END\n");
    return ret;
}

bool dwgReader15::readDwgHeader(DRW_Header& hdr){
    DRW_DBG("dwgReader15::readDwgHeader\n");
    dwgSectionInfo si = sections[secEnum::HEADER];
    if (si.Id<0)//not found, ends
        return false;
    if (!fileBuf->setPosition(si.address))
        return false;
    duint8 *tmpByteStr = new duint8[si.size];
    fileBuf->getBytes(tmpByteStr, si.size);
    dwgBuffer buff(tmpByteStr, si.size, &decoder);
    DRW_DBG("Header section sentinel= ");
    checkSentinel(&buff, secEnum::HEADER, true);
    bool ret = dwgReader::readDwgHeader(hdr, &buff, &buff);
    delete[]tmpByteStr;
    return ret;
}


bool dwgReader15::readDwgClasses(){
    DRW_DBG("\ndwgReader15::readDwgClasses\n");
    dwgSectionInfo si = sections[secEnum::CLASSES];
    if (si.Id<0)//not found, ends
        return false;
    if (!fileBuf->setPosition(si.address))
        return false;

    DRW_DBG("classes section sentinel= ");
    checkSentinel(fileBuf, secEnum::CLASSES, true);

    duint32 size = fileBuf->getRawLong32();
    if (size != (si.size - 38)) {
        DRW_DBG("\nWARNING dwgReader15::readDwgClasses size are "); DRW_DBG(size);
        DRW_DBG(" and secSize - 38 are "); DRW_DBG(si.size - 38); DRW_DBG("\n");
    }
    duint8 *tmpByteStr = new duint8[size];
    fileBuf->getBytes(tmpByteStr, size);
    dwgBuffer buff(tmpByteStr, size, &decoder);
    size--; //reduce 1 byte instead of check pos + bitPos
    while (size > buff.getPosition()) {
        DRW_Class *cl = new DRW_Class();
        cl->parseDwg(version, &buff, &buff);
        classesmap[cl->classNum] = cl;
    }
     DRW_DBG("\nCRC: "); DRW_DBGH(fileBuf->getRawShort16());
     DRW_DBG("\nclasses section end sentinel= ");
     checkSentinel(fileBuf, secEnum::CLASSES, false);
     bool ret = buff.isGood();
     delete[]tmpByteStr;
     return ret;
}

bool dwgReader15::readDwgHandles() {
    DRW_DBG("\ndwgReader15::readDwgHandles\n");
    dwgSectionInfo si = sections[secEnum::HANDLES];
    if (si.Id<0)//not found, ends
        return false;

    bool ret = dwgReader::readDwgHandles(fileBuf, si.address, si.size);
    return ret;
}

/*********** objects ************************/
/**
 * Reads all the object referenced in the object map section of the DWG file
 * (using their object file offsets)
 */
bool dwgReader15::readDwgTables(DRW_Header& hdr) {
    bool ret = dwgReader::readDwgTables(hdr, fileBuf);

    return ret;
}

/**
 * Reads all the object referenced in the object map section of the DWG file
 * (using their object file offsets)
 */
bool dwgReader15::readDwgBlocks(DRW_Interface& intfa) {
    bool ret = true;
    ret = dwgReader::readDwgBlocks(intfa, fileBuf);
    return ret;
}

