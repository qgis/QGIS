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

#include <sstream>
#include "drw_dbg.h"
#include "dwgutil.h"
#include "rscodec.h"
#include "../libdwgr.h"

/** utility function
 * convert a int to string in hex
 **/
namespace DRW {
std::string toHexStr(int n){
#if defined(__APPLE__)
    char buffer[9]= {'\0'};
    snprintf(buffer,9, "%X", n);
    return std::string(buffer);
#else
    std::ostringstream Convert;
    Convert << std::uppercase << std::hex << n;
    return Convert.str();
#endif
}
}

/**
 * @brief dwgRSCodec::decode239I
 * @param in : input data (at least 255*blk bytes)
 * @param out : output data (at least 239*blk bytes)
 * @param blk number of codewords ( 1 cw == 255 bytes)
 */
void dwgRSCodec::decode239I(unsigned char *in, unsigned char *out, duint32 blk){
    int k=0;
    unsigned char data[255];
    RScodec rsc(0x96, 8, 8); //(255, 239)
    for (duint32 i=0; i<blk; i++){
        k = i;
        for (int j=0; j<255; j++) {
            data[j] = in[k];
            k +=blk;
        }
        int r = rsc.decode(data);
        if (r<0)
            DRW_DBG("\nWARNING: dwgRSCodec::decode239I, can't correct all errors");
        k = i*239;
        for (int j=0; j<239; j++) {
            out[k++] = data[j];
        }
    }
}

/**
 * @brief dwgRSCodec::decode251I
 * @param in : input data (at least 255*blk bytes)
 * @param out : output data (at least 251*blk bytes)
 * @param blk number of codewords ( 1 cw == 255 bytes)
 */
void dwgRSCodec::decode251I(unsigned char *in, unsigned char *out, duint32 blk){
    int k=0;
    unsigned char data[255];
    RScodec rsc(0xB8, 8, 2); //(255, 251)
    for (duint32 i=0; i<blk; i++){
        k = i;
        for (int j=0; j<255; j++) {
            data[j] = in[k];
            k +=blk;
        }
        int r = rsc.decode(data);
        if (r<0)
            DRW_DBG("\nWARNING: dwgRSCodec::decode251I, can't correct all errors");
        k = i*251;
        for (int j=0; j<251; j++) {
            out[k++] = data[j];
        }
    }
}

duint32 dwgCompressor::twoByteOffset(duint32 *ll){
    duint32 cont = 0;
    duint8 fb = bufC[pos++];
    cont = (fb >> 2) | (bufC[pos++] << 6);
    *ll = (fb & 0x03);
    return cont;
}

duint32 dwgCompressor::longCompressionOffset(){
    duint32 cont = 0;
    duint8 ll = bufC[pos++];
    while (ll == 0x00){
        cont += 0xFF;
        ll = bufC[pos++];
    }
    cont += ll;
    return cont;
}

duint32 dwgCompressor::long20CompressionOffset(){
//    duint32 cont = 0;
    duint32 cont = 0x0F;
    duint8 ll = bufC[pos++];
    while (ll == 0x00){
//        cont += 0xFF;
        ll = bufC[pos++];
    }
    cont += ll;
    return cont;
}

duint32 dwgCompressor::litLength18(){
    duint32 cont=0;
    duint8 ll = bufC[pos++];
    //no literal length, this byte is next opCode
    if (ll > 0x0F) {
        pos--;
        return 0;
    }

    if (ll == 0x00) {
        cont = 0x0F;
        ll = bufC[pos++];
        while (ll == 0x00){//repeat until ll != 0x00
            cont +=0xFF;
            ll = bufC[pos++];
        }
    }
    cont +=ll;
    cont +=3; //already sum 3
    return cont;
}

void dwgCompressor::decompress18(duint8 *cbuf, duint8 *dbuf, duint32 csize, duint32 dsize){
    bufC = cbuf;
    sizeC = csize -2;
    DRW_DBG("dwgCompressor::decompress, last 2 bytes: ");
    DRW_DBGH(bufC[sizeC]);DRW_DBGH(bufC[sizeC+1]);DRW_DBG("\n");
    sizeC = csize;

    duint32 compBytes;
    duint32 compOffset;
    duint32 litCount;

    pos=0; //current position in compressed buffer
    duint32 rpos=0; //current position in resulting decompresed buffer
    litCount = litLength18();
    //copy first literal length
    for (duint32 i=0; i < litCount; ++i) {
        dbuf[rpos++] = bufC[pos++];
    }

    while (pos < csize && (rpos < dsize+1)){//rpos < dsize to prevent crash more robust are needed
        duint8 oc = bufC[pos++]; //next opcode
        if (oc == 0x10){
            compBytes = longCompressionOffset()+ 9;
            compOffset = twoByteOffset(&litCount) + 0x3FFF;
            if (litCount == 0)
                litCount= litLength18();
        } else if (oc > 0x11 && oc< 0x20){
            compBytes = (oc & 0x0F) + 2;
            compOffset = twoByteOffset(&litCount) + 0x3FFF;
            if (litCount == 0)
                litCount= litLength18();
        } else if (oc == 0x20){
            compBytes = longCompressionOffset() + 0x21;
            compOffset = twoByteOffset(&litCount);
            if (litCount == 0)
                litCount= litLength18();
            else
                oc = 0x00;
        } else if (oc > 0x20 && oc< 0x40){
            compBytes = oc - 0x1E;
            compOffset = twoByteOffset(&litCount);
            if (litCount == 0)
                litCount= litLength18();
        } else if ( oc > 0x3F){
            compBytes = ((oc & 0xF0) >> 4) - 1;
            duint8 ll2 = bufC[pos++];
            compOffset =  (ll2 << 2) | ((oc & 0x0C) >> 2);
            litCount = oc & 0x03;
            if (litCount < 1){
                litCount= litLength18();}
        } else if (oc == 0x11){
            DRW_DBG("dwgCompressor::decompress, end of input stream, Cpos: ");
            DRW_DBG(pos);DRW_DBG(", Dpos: ");DRW_DBG(rpos);DRW_DBG("\n");
            return; //end of input stream
        } else { //ll < 0x10
            DRW_DBG("WARNING dwgCompressor::decompress, failed, illegal char, Cpos: ");
            DRW_DBG(pos);DRW_DBG(", Dpos: ");DRW_DBG(rpos);DRW_DBG("\n");
            return; //fails, not valid
        }
        //copy "compressed data", TODO Needed verify out of bounds
        duint32 remaining = dsize - (litCount+rpos);
        if (remaining < compBytes){
            compBytes = remaining;
            DRW_DBG("WARNING dwgCompressor::decompress, bad compBytes size, Cpos: ");
            DRW_DBG(pos);DRW_DBG(", Dpos: ");DRW_DBG(rpos);DRW_DBG("\n");
        }
        for (duint32 i=0, j= rpos - compOffset -1; i < compBytes; i++) {
            dbuf[rpos++] = dbuf[j++];
        }
        //copy "uncompressed data", TODO Needed verify out of bounds
        for (duint32 i=0; i < litCount; i++) {
            dbuf[rpos++] = bufC[pos++];
        }
    }
    DRW_DBG("WARNING dwgCompressor::decompress, bad out, Cpos: ");DRW_DBG(pos);DRW_DBG(", Dpos: ");DRW_DBG(rpos);DRW_DBG("\n");
}


void dwgCompressor::decrypt18Hdr(duint8 *buf, duint32 size, duint32 offset){
    duint8 max = size / 4;
    duint32 secMask = 0x4164536b ^ offset;
    duint32* pHdr = reinterpret_cast<duint32*>(buf);
    for (duint8 j = 0; j < max; j++)
        *pHdr++ ^= secMask;
}

/*void dwgCompressor::decrypt18Data(duint8 *buf, duint32 size, duint32 offset){
    duint8 max = size / 4;
    duint32 secMask = 0x4164536b ^ offset;
    duint32* pHdr = (duint32*)buf;
    for (duint8 j = 0; j < max; j++)
        *pHdr++ ^= secMask;
}*/

duint32 dwgCompressor::litLength21(duint8 *cbuf, duint8 oc, duint32 *si){

    duint32 srcIndex=*si;

    duint32 length = oc + 8;
    if (length == 0x17) {
        duint32 n = cbuf[srcIndex++];
        length += n;
        if (n == 0xff) {
            do {
                n = cbuf[srcIndex++];
                n |= static_cast<duint32>(cbuf[srcIndex++] << 8);
                length += n;
            } while (n == 0xffff);
        }
    }

    *si = srcIndex;
    return length;
}

void dwgCompressor::decompress21(duint8 *cbuf, duint8 *dbuf, duint32 csize, duint32 dsize){
    duint32 srcIndex=0;
    duint32 dstIndex=0;
    duint32 length=0;
    duint32 sourceOffset;
    duint8 opCode;

    opCode = cbuf[srcIndex++];
    if ((opCode >> 4) == 2){
        srcIndex = srcIndex +2;
        length = cbuf[srcIndex++] & 0x07;
    }

    while (srcIndex < csize && (dstIndex < dsize+1)){//dstIndex < dsize to prevent crash more robust are needed
        if (length == 0)
            length = litLength21(cbuf, opCode, &srcIndex);
        copyCompBytes21(cbuf, dbuf, length, srcIndex, dstIndex);
        srcIndex += length;
        dstIndex += length;
        if (dstIndex >=dsize) break; //check if last chunk are compressed & terminate

        length = 0;
        opCode = cbuf[srcIndex++];
        readInstructions21(cbuf, &srcIndex, &opCode, &sourceOffset, &length);
        while (true) {
            //prevent crash with corrupted data
            if (sourceOffset > dstIndex){
                DRW_DBG("\nWARNING dwgCompressor::decompress21 => sourceOffset> dstIndex.\n");
                DRW_DBG("csize = "); DRW_DBG(csize); DRW_DBG("  srcIndex = "); DRW_DBG(srcIndex);
                DRW_DBG("\ndsize = "); DRW_DBG(dsize); DRW_DBG("  dstIndex = "); DRW_DBG(dstIndex);
                sourceOffset = dstIndex;
            }
            //prevent crash with corrupted data
            if (length > dsize - dstIndex){
                DRW_DBG("\nWARNING dwgCompressor::decompress21 => length > dsize - dstIndex.\n");
                DRW_DBG("csize = "); DRW_DBG(csize); DRW_DBG("  srcIndex = "); DRW_DBG(srcIndex);
                DRW_DBG("\ndsize = "); DRW_DBG(dsize); DRW_DBG("  dstIndex = "); DRW_DBG(dstIndex);
                length = dsize - dstIndex;
                srcIndex = csize;//force exit
            }
            sourceOffset = dstIndex-sourceOffset;
            for (duint32 i=0; i< length; i++)
                dbuf[dstIndex++] = dbuf[sourceOffset+i];

            length = opCode & 7;
            if ((length != 0) || (srcIndex >= csize)) {
                break;
            }
            opCode = cbuf[srcIndex++];
            if ((opCode >> 4) == 0) {
                break;
            }
            if ((opCode >> 4) == 15) {
                opCode &= 15;
            }
            readInstructions21(cbuf, &srcIndex, &opCode, &sourceOffset, &length);
        }
    }
    DRW_DBG("\ncsize = "); DRW_DBG(csize); DRW_DBG("  srcIndex = "); DRW_DBG(srcIndex);
    DRW_DBG("\ndsize = "); DRW_DBG(dsize); DRW_DBG("  dstIndex = "); DRW_DBG(dstIndex);DRW_DBG("\n");
}

void dwgCompressor::readInstructions21(duint8 *cbuf, duint32 *si, duint8 *oc, duint32 *so, duint32 *l){
    duint32 length;
    duint32 srcIndex = *si;
    duint32 sourceOffset;
    unsigned char opCode = *oc;
    switch ((opCode >> 4)) {
    case 0:
        length = (opCode & 0xf) + 0x13;
        sourceOffset = cbuf[srcIndex++];
        opCode = cbuf[srcIndex++];
        length = ((opCode >> 3) & 0x10) + length;
        sourceOffset = ((opCode & 0x78) << 5) + 1 + sourceOffset;
        break;
    case 1:
        length = (opCode & 0xf) + 3;
        sourceOffset = cbuf[srcIndex++];
        opCode = cbuf[srcIndex++];
        sourceOffset = ((opCode & 0xf8) << 5) + 1 + sourceOffset;
        break;
    case 2:
        sourceOffset = cbuf[srcIndex++];
        sourceOffset = ((cbuf[srcIndex++] << 8) & 0xff00) | sourceOffset;
        length = opCode & 7;
        if ((opCode & 8) == 0) {
            opCode = cbuf[srcIndex++];
            length = (opCode & 0xf8) + length;
        } else {
            sourceOffset++;
            length = (cbuf[srcIndex++] << 3) + length;
            opCode = cbuf[srcIndex++];
            length = (((opCode & 0xf8) << 8) + length) + 0x100;
        }
        break;
    default:
        length = opCode >> 4;
        sourceOffset = opCode & 15;
        opCode = cbuf[srcIndex++];
        sourceOffset = (((opCode & 0xf8) << 1) + sourceOffset) + 1;
        break;
    }
    *oc = opCode;
    *si = srcIndex;
    *so = sourceOffset;
    *l = length;
}


void dwgCompressor::copyCompBytes21(duint8 *cbuf, duint8 *dbuf, duint32 l, duint32 si, duint32 di){
    duint32 length =l;
    duint32 dix = di;
    duint32 six = si;

    while (length > 31){
        //in doc: 16-31, 0-15
        for (duint32 i = six+24; i<six+32; i++)
            dbuf[dix++] = cbuf[i];
        for (duint32 i = six+16; i<six+24; i++)
            dbuf[dix++] = cbuf[i];
        for (duint32 i = six+8; i<six+16; i++)
            dbuf[dix++] = cbuf[i];
        for (duint32 i = six; i<six+8; i++)
            dbuf[dix++] = cbuf[i];
        six = six + 32;
        length = length -32;
    }

    switch (length) {
    case 0:
        break;
    case 1: //Ok
        dbuf[dix] = cbuf[six];
        break;
    case 2: //Ok
        dbuf[dix++] = cbuf[six+1];
        dbuf[dix] = cbuf[six];
        break;
    case 3: //Ok
        dbuf[dix++] = cbuf[six+2];
        dbuf[dix++] = cbuf[six+1];
        dbuf[dix] = cbuf[six];
        break;
    case 4: //Ok
        for (int i = 0; i<4;i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 5: //Ok
        dbuf[dix++] = cbuf[six+4];
        for (int i = 0; i<4;i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 6: //Ok
        dbuf[dix++] = cbuf[six+5];
        for (int i = 1; i<5;i++)
            dbuf[dix++] = cbuf[six+i];
        dbuf[dix] = cbuf[six];
        break;
    case 7:
        //in doc: six+5, six+6, 1-5, six+0
        dbuf[dix++] = cbuf[six+6];
        dbuf[dix++] = cbuf[six+5];
        for (int i = 1; i<5;i++)
            dbuf[dix++] = cbuf[six+i];
        dbuf[dix] = cbuf[six];
        break;
    case 8: //Ok
        for (int i = 0; i<8;i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 9: //Ok
        dbuf[dix++] = cbuf[six+8];
        for (int i = 0; i<8;i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 10: //Ok
        dbuf[dix++] = cbuf[six+9];
        for (int i = 1; i<9;i++)
            dbuf[dix++] = cbuf[six+i];
        dbuf[dix] = cbuf[six];
        break;
    case 11:
        //in doc: six+9, six+10, 1-9, six+0
        dbuf[dix++] = cbuf[six+10];
        dbuf[dix++] = cbuf[six+9];
        for (int i = 1; i<9;i++)
            dbuf[dix++] = cbuf[six+i];
        dbuf[dix] = cbuf[six];
        break;
    case 12: //Ok
        for (int i = 8; i<12;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8;i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 13: //Ok
        dbuf[dix++] = cbuf[six+12];
        for (int i = 8; i<12;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8;i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 14: //Ok
        dbuf[dix++] = cbuf[six+13];
        for (int i = 9; i<13; i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 1; i<9; i++)
            dbuf[dix++] = cbuf[six+i];
        dbuf[dix] = cbuf[six];
        break;
    case 15:
        //in doc: six+13, six+14, 9-12, 1-8, six+0
        dbuf[dix++] = cbuf[six+14];
        dbuf[dix++] = cbuf[six+13];
        for (int i = 9; i<13; i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 1; i<9; i++)
            dbuf[dix++] = cbuf[six+i];
        dbuf[dix] = cbuf[six];
        break;
    case 16: //Ok
        for (int i = 8; i<16;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8;i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 17: //Ok
        for (int i = 9; i<17;i++)
            dbuf[dix++] = cbuf[six+i];
        dbuf[dix++] = cbuf[six+8];
        for (int i = 0; i<8;i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 18:
        //in doc: six+17, 1-16, six+0
        dbuf[dix++] = cbuf[six+17];
        for (int i = 9; i<17;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 1; i<9;i++)
            dbuf[dix++] = cbuf[six+i];
        dbuf[dix] = cbuf[six];
        break;
    case 19:
        //in doc: 16-18, 0-15
        dbuf[dix++] = cbuf[six+18];
        dbuf[dix++] = cbuf[six+17];
        dbuf[dix++] = cbuf[six+16];
        for (int i = 8; i<16;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8;i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 20:
        //in doc: 16-19, 0-15
        for (int i = 16; i<20;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 8; i<16;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8;i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 21:
        //in doc: six+20, 16-19, 0-15
        dbuf[dix++] = cbuf[six+20];
        for (int i = 16; i<20;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 8; i<16;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8;i++)
            dbuf[dix++] = cbuf[six+i];
        break;
    case 22:
        //in doc: six+20, six+21, 16-19, 0-15
        dbuf[dix++] = cbuf[six+21];
        dbuf[dix++] = cbuf[six+20];
        for (int i = 16; i<20;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 8; i<16;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8;i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 23:
        //in doc: six+20, six+21, six+22, 16-19, 0-15
        dbuf[dix++] = cbuf[six+22];
        dbuf[dix++] = cbuf[six+21];
        dbuf[dix++] = cbuf[six+20];
        for (int i = 16; i<20;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 8; i<16;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8;i++)
            dbuf[dix++] = cbuf[six+i];
        break;
    case 24:
        //in doc: 16-23, 0-15
        for (int i = 16; i<24;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 8; i<16;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8; i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 25:
        //in doc: 17-24, six+16, 0-15
        for (int i = 17; i<25;i++)
            dbuf[dix++] = cbuf[six+i];
        dbuf[dix++] = cbuf[six+16];
        for (int i = 8; i<16; i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8; i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 26:
        //in doc: six+25, 17-24, six+16, 0-15
        dbuf[dix++] = cbuf[six+25];
        for (int i = 17; i<25;i++)
            dbuf[dix++] = cbuf[six+i];
        dbuf[dix++] = cbuf[six+16];
        for (int i = 8; i<16;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8; i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 27:
        //in doc: six+25, six+26, 17-24, six+16, 0-15
        dbuf[dix++] = cbuf[six+26];
        dbuf[dix++] = cbuf[six+25];
        for (int i = 17; i<25;i++)
            dbuf[dix++] = cbuf[six+i];
        dbuf[dix++] = cbuf[six+16];
        for (int i = 8; i<16;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8; i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 28:
        //in doc: 24-27, 16-23, 0-15
        for (int i = 24; i<28; i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 16; i<24;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 8; i<16; i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8; i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 29:
        //in doc: six+28, 24-27, 16-23, 0-15
        dbuf[dix++] = cbuf[six+28];
        for (int i = 24; i<28; i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 16; i<24;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 8; i<16;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8; i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 30:
        //in doc: six+28, six+29, 24-27, 16-23, 0-15
        dbuf[dix++] = cbuf[six+29];
        dbuf[dix++] = cbuf[six+28];
        for (int i = 24; i<28; i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 16; i<24;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 8; i<16;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 0; i<8; i++)
            dbuf[dix++] = cbuf[six++];
        break;
    case 31:
        //in doc: six+30, 26-29, 18-25, 2-17, 0-1
        dbuf[dix++] = cbuf[six+30];
        for (int i = 26; i<30;i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 18; i<26;i++)
            dbuf[dix++] = cbuf[six+i];
/*        for (int i = 2; i<18; i++)
            dbuf[dix++] = cbuf[six+i];*/
        for (int i = 10; i<18; i++)
            dbuf[dix++] = cbuf[six+i];
        for (int i = 2; i<10; i++)
            dbuf[dix++] = cbuf[six+i];
        dbuf[dix++] = cbuf[six+1];
        dbuf[dix] = cbuf[six];
        break;
    default:
        DRW_DBG("WARNING dwgCompressor::copyCompBytes21, bad output.\n");
        break;
    }
}


secEnum::DWGSection secEnum::getEnum(const std::string &nameSec){
    //TODO: complete it
    if (nameSec=="AcDb:Header"){
        return HEADER;
    } else if (nameSec=="AcDb:Classes"){
        return CLASSES;
    } else if (nameSec=="AcDb:SummaryInfo"){
        return SUMMARYINFO;
    } else if (nameSec=="AcDb:Preview"){
        return PREVIEW;
    } else if (nameSec=="AcDb:VBAProject"){
        return VBAPROY;
    } else if (nameSec=="AcDb:AppInfo"){
        return APPINFO;
    } else if (nameSec=="AcDb:FileDepList"){
        return FILEDEP;
    } else if (nameSec=="AcDb:RevHistory"){
        return REVHISTORY;
    } else if (nameSec=="AcDb:Security"){
        return SECURITY;
    } else if (nameSec=="AcDb:AcDbObjects"){
        return OBJECTS;
    } else if (nameSec=="AcDb:ObjFreeSpace"){
        return OBJFREESPACE;
    } else if (nameSec=="AcDb:Template"){
        return TEMPLATE;
    } else if (nameSec=="AcDb:Handles"){
        return HANDLES;
    } else if (nameSec=="AcDb:AcDsPrototype_1b"){
        return PROTOTYPE;
    } else if (nameSec=="AcDb:AuxHeader"){
        return AUXHEADER;
    } else if (nameSec=="AcDb:Signature"){
        return SIGNATURE;
    } else if (nameSec=="AcDb:AppInfoHistory"){ //in ac1021
        return APPINFOHISTORY;
//    } else if (nameSec=="AcDb:Extended Entity Data"){
//        return EXTEDATA;
//    } else if (nameSec=="AcDb:PROXY ENTITY GRAPHICS"){
//        return PROXYGRAPHICS;
    }
    return UNKNOWNS;
}
