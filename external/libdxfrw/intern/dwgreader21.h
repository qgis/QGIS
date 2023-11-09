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

#ifndef DWGREADER21_H
#define DWGREADER21_H

#include <map>
#include <list>
#include "drw_textcodec.h"
#include "dwgbuffer.h"
#include "dwgreader.h"

//reader for AC1021 aka v2007, chapter 5
class dwgReader21 : public dwgReader {
public:
    dwgReader21(std::ifstream *stream, dwgR *p):dwgReader(stream, p){
    }
    bool readMetaData() override;
    bool readFileHeader() override;
    bool readDwgHeader(DRW_Header& hdr) override;
    bool readDwgClasses() override;
    bool readDwgHandles() override;
    bool readDwgTables(DRW_Header& hdr) override;
    bool readDwgBlocks(DRW_Interface& intfa) override;
    bool readDwgEntities(DRW_Interface& intfa) override {
        bool ret = true;
        dwgBuffer dataBuf(objData.get(), dataSize, &decoder);
        ret = dwgReader::readDwgEntities(intfa, &dataBuf);
        return ret;
    }
    bool readDwgObjects(DRW_Interface& intfa) override {
        bool ret = true;
        dwgBuffer dataBuf(objData.get(), dataSize, &decoder);
        ret = dwgReader::readDwgObjects(intfa, &dataBuf);
        return ret;
    }
//bool readDwgEntity(objHandle& obj, DRW_Interface& intfa){
//    return false;
//}

private:
    bool parseSysPage(duint64 sizeCompressed, duint64 sizeUncompressed, duint64 correctionFactor, duint64 offset, duint8 *decompData);
    bool parseDataPage(const dwgSectionInfo &si, duint8 *dData);

    std::unique_ptr<duint8 []> objData;
    duint64 dataSize{0};

};

#endif // DWGREADER21_H
