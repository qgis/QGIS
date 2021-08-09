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

#ifndef DWGREADER18_H
#define DWGREADER18_H

#include <map>
#include <list>
#include "dwgreader.h"
//#include "../drw_textcodec.h"
#include "dwgbuffer.h"

static const int DRW_magicNum18[] = {
    0x29, 0x23, 0xbe, 0x84, 0xe1, 0x6c, 0xd6, 0xae,
    0x52, 0x90, 0x49, 0xf1, 0xf1, 0xbb, 0xe9, 0xeb,
    0xb3, 0xa6, 0xdb, 0x3c, 0x87, 0x0c, 0x3e, 0x99,
    0x24, 0x5e, 0x0d, 0x1c, 0x06, 0xb7, 0x47, 0xde,
    0xb3, 0x12, 0x4d, 0xc8, 0x43, 0xbb, 0x8b, 0xa6,
    0x1f, 0x03, 0x5a, 0x7d, 0x09, 0x38, 0x25, 0x1f,
    0x5d, 0xd4, 0xcb, 0xfc, 0x96, 0xf5, 0x45, 0x3b,
    0x13, 0x0d, 0x89, 0x0a, 0x1c, 0xdb, 0xae, 0x32,
    0x20, 0x9a, 0x50, 0xee, 0x40, 0x78, 0x36, 0xfd,
    0x12, 0x49, 0x32, 0xf6, 0x9e, 0x7d, 0x49, 0xdc,
    0xad, 0x4f, 0x14, 0xf2, 0x44, 0x40, 0x66, 0xd0,
    0x6b, 0xc4, 0x30, 0xb7, 0x32, 0x3b, 0xa1, 0x22,
    0xf6, 0x22, 0x91, 0x9d, 0xe1, 0x8b, 0x1f, 0xda,
    0xb0, 0xca, 0x99, 0x02
};

static const int DRW_magicNumEnd18[] = {
    0xf8, 0x46, 0x6a, 0x04, 0x96, 0x73, 0x0e, 0xd9,
    0x16, 0x2f, 0x67, 0x68, 0xd4, 0xf7, 0x4a, 0x4a,
    0xd0, 0x57, 0x68, 0x76};

class dwgReader18 : public dwgReader {
public:
    dwgReader18(std::ifstream *stream, dwgR *p):dwgReader(stream, p){
        objData = NULL;
    }
    virtual ~dwgReader18(){
        if (objData != NULL)
            delete[] objData;
    }
    bool readMetaData();
    bool readFileHeader();
    bool readDwgHeader(DRW_Header& hdr);
    bool readDwgClasses();
    bool readDwgHandles();
    bool readDwgTables(DRW_Header& hdr);
    bool readDwgBlocks(DRW_Interface& intfa){
        bool ret = true;
        dwgBuffer dataBuf(objData, uncompSize, &decoder);
        ret = dwgReader::readDwgBlocks(intfa, &dataBuf);
        return ret;
    }

    virtual bool readDwgEntities(DRW_Interface& intfa){
        bool ret = true;
        dwgBuffer dataBuf(objData, uncompSize, &decoder);
        ret = dwgReader::readDwgEntities(intfa, &dataBuf);
        return ret;
    }
    virtual bool readDwgObjects(DRW_Interface& intfa){
        bool ret = true;
        dwgBuffer dataBuf(objData, uncompSize, &decoder);
        ret = dwgReader::readDwgObjects(intfa, &dataBuf);
        return ret;
    }

//    bool readDwgEntity(objHandle& obj, DRW_Interface& intfa){
//        bool ret = true;
//        return ret;
//    }

protected:
    duint8 *objData;
    duint64 uncompSize;

private:
    void genMagicNumber();
//    dwgBuffer* bufObj;
    void parseSysPage(duint8 *decompSec, duint32 decompSize); //called: Section page map: 0x41630e3b
    bool parseDataPage(dwgSectionInfo si/*, duint8 *dData*/); //called ???: Section map: 0x4163003b
    duint32 checksum(duint32 seed, duint8* data, duint32 sz);

private:
duint32 securityFlags;
};

#endif // DWGREADER18_H
