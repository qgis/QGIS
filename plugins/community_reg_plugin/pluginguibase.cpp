/****************************************************************************
** Form implementation generated from reading ui file 'pluginguibase.ui'
**
** Created: Wed Mar 16 23:39:35 2005
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "pluginguibase.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

#include "pluginguibase.ui.h"
static const unsigned char image0_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x02,
    0x2c, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0x95, 0x93, 0x4b, 0x48, 0x54,
    0x51, 0x1c, 0xc6, 0x7f, 0xf7, 0x11, 0x96, 0xd3, 0x9d, 0x5b, 0xa6, 0x46,
    0x93, 0x4c, 0x36, 0x65, 0x64, 0x2d, 0x2c, 0x7a, 0x91, 0x99, 0x0d, 0x95,
    0x34, 0x09, 0x25, 0xf4, 0x58, 0x15, 0x08, 0xe2, 0xa3, 0x07, 0x46, 0x14,
    0x08, 0xb6, 0x08, 0x93, 0x20, 0x2a, 0xaa, 0x45, 0x6e, 0x2c, 0x84, 0x5a,
    0x58, 0xcb, 0x68, 0x11, 0x25, 0xb9, 0x08, 0x42, 0x57, 0xb9, 0x10, 0xac,
    0x2c, 0xb0, 0x2c, 0x1f, 0xf8, 0x9a, 0x69, 0x1c, 0xef, 0x3c, 0x9c, 0x3b,
    0x77, 0x4e, 0x0b, 0x71, 0xe8, 0x56, 0x13, 0xf6, 0xdf, 0x9d, 0xc3, 0xf7,
    0xfd, 0x38, 0xdf, 0xf7, 0xe7, 0x48, 0xdd, 0xdd, 0xdd, 0xfc, 0xcf, 0x34,
    0x3c, 0x38, 0x2a, 0xd6, 0xc9, 0x5e, 0xea, 0xaa, 0x2f, 0x49, 0x00, 0xea,
    0x42, 0x8d, 0x27, 0x5b, 0x4a, 0x45, 0xc0, 0x99, 0x40, 0x09, 0x66, 0x70,
    0xab, 0x61, 0xce, 0x0c, 0x20, 0x2f, 0xc4, 0xdc, 0x74, 0xef, 0x98, 0x08,
    0x21, 0x11, 0x8d, 0x81, 0x62, 0xd9, 0x2d, 0x69, 0x01, 0x9d, 0x1d, 0xe7,
    0xc4, 0xa7, 0x81, 0x6a, 0x31, 0x30, 0x71, 0x46, 0x94, 0x95, 0x3b, 0x89,
    0x68, 0x26, 0xca, 0x84, 0x0c, 0x91, 0x24, 0x37, 0x6e, 0x5f, 0x11, 0xf3,
    0xba, 0xbf, 0x46, 0x48, 0xca, 0x37, 0xc5, 0x96, 0x12, 0x93, 0xae, 0x4e,
    0x0f, 0x46, 0xcc, 0xe2, 0xab, 0xeb, 0x1d, 0xd6, 0x0c, 0x78, 0xa6, 0xf3,
    0x70, 0x67, 0x3a, 0x10, 0xa6, 0x49, 0x5a, 0x40, 0x6f, 0xdf, 0x35, 0x71,
    0xb0, 0x3c, 0xc0, 0xb3, 0xfb, 0x1e, 0x8e, 0x57, 0x34, 0x4a, 0x87, 0x5b,
    0xf7, 0x09, 0x23, 0x62, 0xa2, 0x4d, 0x42, 0x7d, 0x95, 0xc6, 0xae, 0xc2,
    0x87, 0xd2, 0xaf, 0xfa, 0x3f, 0x22, 0x64, 0xe7, 0xbf, 0x67, 0x70, 0x68,
    0x0a, 0xff, 0xf8, 0x52, 0x00, 0x66, 0xd5, 0x04, 0x96, 0x1f, 0x4e, 0x14,
    0xb9, 0x29, 0xdb, 0xef, 0xe7, 0xe5, 0xeb, 0xbb, 0x22, 0x2d, 0xa0, 0xb5,
    0xad, 0x51, 0x78, 0x36, 0x24, 0x19, 0x1e, 0x71, 0x52, 0x5b, 0x5b, 0x2f,
    0xf9, 0x9e, 0xee, 0x15, 0x31, 0x59, 0x90, 0x35, 0xa6, 0xb3, 0xc9, 0xbd,
    0x1e, 0x87, 0x66, 0xb2, 0xd8, 0x11, 0x4a, 0x5f, 0xe2, 0x22, 0x55, 0x41,
    0x89, 0x27, 0x49, 0xc4, 0xe6, 0x92, 0x45, 0x13, 0x16, 0xf2, 0xa8, 0x44,
    0x9e, 0xea, 0x42, 0x51, 0x0c, 0x84, 0x25, 0x88, 0x47, 0x67, 0x6d, 0x00,
    0x5b, 0x07, 0x55, 0x95, 0xd7, 0xa5, 0xd1, 0xc9, 0x53, 0x42, 0xd3, 0xc3,
    0xb4, 0xbf, 0xba, 0x20, 0xce, 0x97, 0x2c, 0x67, 0xb5, 0xac, 0x11, 0x1e,
    0xc8, 0x45, 0x5f, 0x36, 0x44, 0xd8, 0x90, 0x31, 0x66, 0x56, 0xa4, 0x7f,
    0x01, 0x40, 0xc4, 0x9f, 0x8d, 0xcb, 0x15, 0xa0, 0x78, 0x67, 0x90, 0xf6,
    0x96, 0x28, 0xfd, 0x6f, 0xdc, 0x94, 0xf8, 0x86, 0x59, 0x53, 0xa8, 0xd2,
    0xd3, 0xeb, 0x26, 0x30, 0xe9, 0xf8, 0x37, 0xe0, 0xc9, 0x23, 0x95, 0x17,
    0x5d, 0x32, 0x6b, 0xdd, 0x3f, 0xf0, 0x6d, 0x2b, 0xc0, 0x9a, 0x19, 0xc3,
    0x9c, 0x8e, 0x10, 0x8b, 0xce, 0xf2, 0xb6, 0x23, 0x9f, 0x9a, 0x9a, 0xb3,
    0xb6, 0x2d, 0x48, 0xbf, 0xff, 0x05, 0x5f, 0xdb, 0x1e, 0x11, 0x42, 0xb0,
    0x35, 0x91, 0xc3, 0xe9, 0xe2, 0x22, 0x12, 0x46, 0x9c, 0xc2, 0x95, 0x5f,
    0x98, 0x1a, 0x91, 0xf9, 0x18, 0x5f, 0x85, 0x8a, 0xce, 0x91, 0x03, 0x4d,
    0x29, 0x88, 0xad, 0x83, 0xca, 0x3b, 0x15, 0x42, 0x9e, 0x76, 0xa0, 0xe8,
    0x61, 0xfc, 0x01, 0x85, 0xdd, 0x75, 0xcd, 0x12, 0xc0, 0x44, 0xf4, 0xa2,
    0xf0, 0x7a, 0x87, 0xc9, 0xed, 0x31, 0xe8, 0x1f, 0xb4, 0x77, 0x60, 0x03,
    0x3c, 0xbe, 0xfc, 0x5c, 0x02, 0x38, 0xd4, 0x5c, 0x2a, 0x32, 0x1d, 0x19,
    0xa9, 0xfb, 0xef, 0xdf, 0xb2, 0x90, 0x93, 0x99, 0x8c, 0x07, 0x33, 0xf8,
    0xfc, 0x41, 0xa7, 0x78, 0x47, 0x1a, 0xc0, 0xfc, 0xe4, 0x8c, 0x2f, 0xc1,
    0xe9, 0x54, 0x52, 0xe7, 0xed, 0x1b, 0xaf, 0x4a, 0x00, 0xd9, 0x05, 0xb0,
    0xb9, 0xc0, 0xae, 0xfd, 0x09, 0xd8, 0xd9, 0xcc, 0x2e, 0xed, 0x29, 0x08,
    0x32, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60,
    0x82
};

static const char* const image1_data[] = { 
"162 350 195 2",
".d c #000000",
".j c #010101",
"#f c #020202",
".i c #030303",
"#b c #040404",
".n c #060606",
".X c #070707",
".K c #080808",
".9 c #090909",
".I c #0a0a0a",
".N c #0c0c0c",
".O c #0d0d0d",
"#c c #101010",
".8 c #131313",
".o c #141414",
".5 c #161616",
"#G c #171717",
"a. c #181818",
".u c #191919",
"aj c #1a1a1a",
"#v c #1b1b1b",
"#a c #202020",
"#o c #222222",
".F c #242424",
".W c #262626",
".6 c #272727",
"#Y c #2a2a2a",
".m c #2b2b2b",
"#r c #2c2c2c",
".2 c #2d2d2d",
".T c #303030",
"#0 c #333333",
"#l c #353535",
".1 c #373737",
".S c #383838",
"#A c #393939",
"#Q c #3a3a3a",
"#X c #3b3b3b",
".A c #3d3d3d",
"#9 c #3e3e3e",
"#g c #404040",
"#T c #414141",
"#O c #424242",
"#k c #434343",
".E c #454545",
".Y c #484848",
"#. c #494949",
"#y c #4b4b4b",
"#Z c #4c4c4c",
"#h c #4d4d4d",
"#x c #4f4f4f",
".a c #505050",
"ag c #525252",
".M c #545454",
".v c #555555",
".P c #565656",
"#d c #585858",
"#H c #595959",
".B c #5a5a5a",
".7 c #5e5e5e",
"ak c #5f5f5f",
".w c #646464",
".p c #666666",
"ae c #676767",
"ai c #6a6a6a",
"#s c #777777",
".l c #7a7a7a",
"#w c #7c7c7c",
"#2 c #7d7d7d",
"#N c #7f7f7f",
"ao c #808080",
".J c #818181",
"a3 c #828282",
"aJ c #838383",
".e c #83868c",
"aH c #848484",
"aD c #858585",
"aU c #868686",
"#W c #878787",
"a2 c #888888",
"aZ c #898989",
"aF c #8a8a8a",
"an c #8b8b8b",
"a6 c #8c8c8c",
"ay c #8d8d8d",
"at c #8e8e8e",
"am c #8f8f8f",
"az c #909090",
".h c #919191",
"aA c #929292",
"aw c #939393",
"au c #949494",
"ah c #959595",
"## c #969696",
".R c #979797",
".0 c #989898",
".4 c #999999",
"aR c #9a9a9a",
"as c #9b9b9b",
"#S c #9c9c9c",
"a9 c #9d9d9d",
"al c #9e9e9e",
"ar c #9f9f9f",
"aS c #a1a1a1",
"aV c #a2a2a2",
"a8 c #a3a3a3",
"#1 c #a4a4a4",
"ap c #a5a5a5",
"ba c #a6a6a6",
"a7 c #a7a7a7",
"aT c #a8a8a8",
"aa c #a9a9a9",
"aN c #acacac",
".V c #adadad",
"#7 c #afafaf",
"aO c #b0b0b0",
"a0 c #b1b1b1",
"ab c #b2b2b2",
"ad c #b3b3b3",
"a4 c #b4b4b4",
"aq c #b5b5b5",
"av c #b6b6b6",
".H c #b7b7b7",
"#C c #b8b8b8",
"aK c #b9b9b9",
".# c #b9d4e7",
"#8 c #bababa",
"#M c #bbbbbb",
"aX c #bcbcbc",
"#e c #bdbdbd",
"#P c #bebebe",
"aP c #bfbfbf",
"#E c #c0c0c0",
".C c #c3c3c3",
".k c #c4c4c4",
".x c #c6c6c6",
"#U c #c7c7c7",
"#V c #c8c8c8",
"a5 c #c9c9c9",
"ax c #cacaca",
"aQ c #cbcbcb",
"b# c #cccccc",
"aL c #cdcdcd",
"aC c #cecece",
".L c #cfcfcf",
".3 c #d0d0d0",
"#3 c #d2d2d2",
"aY c #d3d3d3",
".U c #d4d4d4",
"aW c #d5d5d5",
"aB c #d6d6d6",
"a1 c #d7d7d7",
"af c #d8d8d8",
"#K c #d9d9d9",
"#5 c #dadada",
"#J c #dbdbdb",
"#4 c #dcdcdc",
"#m c #dddddd",
"#t c #dedede",
".r c #dfdfdf",
"#I c #e0e0e0",
"#L c #e1e1e1",
"#6 c #e2e2e2",
"#B c #e3e3e3",
".z c #e4e4e4",
".b c #e5e5e5",
"#q c #e6e6e6",
"#F c #e7e7e7",
"#z c #e8e8e8",
".t c #e9e9e9",
"aE c #eaeaea",
".Q c #ebebeb",
"#j c #ececec",
"#i c #ededed",
".g c #edf3fe",
"#p c #eeeeee",
".Z c #efefef",
"aM c #f0f0f0",
".G c #f1f1f1",
"ac c #f2f2f2",
".q c #f3f3f3",
"#u c #f4f4f4",
"#n c #f5f5f5",
"b. c #f6f6f6",
"aI c #f7f7f7",
".s c #f8f8f8",
"#R c #f9f9f9",
".f c #f9fbfe",
"aG c #fafafa",
".y c #fbfbfb",
"#D c #fcfcfc",
".D c #fdfdfd",
"a# c #fefefe",
".c c #ff8000",
"Qt c #ffffff",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".a.a.a.a.a.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".a.a.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".a.a.a.a.a.a.a.a.a.a.a.a.a.#.a.a.#.a.#.#.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".a.b.b.b.b.b.a.a.a.b.a.a.a.a.a.a.a.a.a.a.a.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.#.a.a.a.a.a.a.#.a.a.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".b.b.b.b.b.b.b.b.a.a.a.a.a.b.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.#.a.a.a.a.#.#.#.a.#.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".b.b.b.b.b.b.b.b.a.a.b.b.b.b.a.#.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.#.a.a.a.a.a.a.#.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.a.a.b.a.a.b.b.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.b.b.b.b.b.a.a.a.a.a.a.a.#.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.b.a.a.a.#.#.a.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.b.a.a.#.#.#.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.#.#.#.#",
".b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.#.#.a.a.a.a.a.a.b.b.a.#.#.#.#",
".b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#",
".b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.b.b.b.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#",
".b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".b.b.b.b.b.b.a.b.b.b.b.b.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".b.b.b.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a",
".b.b.b.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a",
".b.a.a.b.b.b.b.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b",
".b.a.a.a.a.a.a.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b",
".b.b.a.a.a.a.a.a.a.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b",
".b.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.#.a.a.a.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b",
".b.a.a.a.b.b.b.b.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.b.a.b.b.b.b.a.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.#.#.a.a.a.b.b.b.b.b",
".b.b.b.b.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.a.b.a.a.b.a.b.b.b.b.a.a.a.b.b.b.b.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.a.#.#.#.#.a.a.a.#.#.#.#.#.a.a.a.#.a.b.b.b.b.b.b.a.a.a.a.a.a.a.a.b.b.b",
".a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.#.#.#.a.a.a.a.#.#.#.a.a.#.a.a.a.a.a.b.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.a.b.a.a.#.#.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.a.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.a.a.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.#.a.a.a.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.a.a.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.a.a.b.b.b.a.a.a.a.a.a.a.b.b.b.b.b.a.a.a.a.a.a.a.a.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a",
".#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.a.b.b.b.b.b.a.a.a.#.a.a.a.a.a.a.a.a.a.a.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.a.a.b.b.b.b.b.b.a.b.b.b.b.b.b.b.a.a.a.a.a.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.a.a.a.b.b.b.b.b.b.a.b.b.b.b.b.b.b.a.a.a.a.b.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.b.b.b.b.b.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.a.a.a.a.b.b.b.b.b.b.b.b.b.a.a.a.#.a.a.a.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.a.a.a.a.#.a.a.a.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.b.b.b.b.a.b.b.b.b.b.b.b.b.b.a.a.a.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.a.a.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.a.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.a.a.a.a.a.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.a.c.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.c.c.c.c.c.a.#.a.a.a.a.b.b.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b.b.a.a.b.a.a.b.b.b.b.b.a.c.c.c.c.c.a.#.#.#.a.a.a.a.a.a.a.a.#.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.c.c.c.c.c.c.c.#.#.#.a.a.a.a.a.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.a.a.a.#.#.a.a.b.b.b.b.b.b.b.c.c.c.c.c.#.#.#.a.a.a.a.a.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.#.#.a.a.a.a.b.b.b.b.b.b.b.c.c.c.c.c.#.#.#.a.a.#.#.a.a.a.a.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.a.a.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.b.b.b.b.b.a.a.#.c.a.#.#.#.a.a.a.a.a.a.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.#.a.b.b.b.b.b.a.a.#.#.#.#.#.#.a.a.a.a.a.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.a.a.a.a.a.a.b.a.a.a.a.a.#.#.#.#.#.#.a.a.a.b.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.#.a.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b.b.b.a.a.b.b.b.a.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.#.a.#.a.a.a.a.a.#.a.a.a.a.a.b.b.b.b.a.a.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.b.b.b.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.#.a.a.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.a.a.a.a.c.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.a.#.#.a.a.a.b.b.b.c.a.#.#.#.#.#.#.#.a.c.a.a.a.a.a.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.c.c.c.c.c.b.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.a.#.#.a.a.b.b.c.c.c.c.c.a.a.#.#.#.c.c.c.c.c.a.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.c.c.c.c.c.b.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.a.a.a.#.a.a.b.b.b.c.c.c.c.c.b.a.#.#.#.c.c.c.c.c.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.c.c.c.c.c.c.c.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.#.#.#.a.a.a.a.c.c.c.c.c.c.c.#.#.a.c.c.c.c.c.c.c.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.c.c.c.c.c.b.a.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.c.c.c.c.c.a.a.a.a.a.c.c.c.c.c.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.c.c.c.c.c.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.a.a.a.b.a.a.a.a.a.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.c.c.c.c.c.a.a.a.a.a.c.c.c.c.c.b.b.b.b.b.b.b.b.a.a.a.b.b.a.a.a.a.b.b.b.b.b.c.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.a.a.c.a.a.a.b.b.b.b.a.a.c.a.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.b.a.a.a.a.b.b.b.b.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.a.a.a.b.a.a.b.b.b.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.a.b.b.a.a.a.a.a.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.a.a.b.b.b.b.a.a.b.b.b.b.a.a.a.a.a.a.a.a.b.b.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.a.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.a.a.b.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.a.a.b.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.b.b.b.b.b.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.b.b.b.b.b.a.b.b.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.a.a.b.a.a.#.a.a.a.a.a.a.a.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.b.a.a.a.b.a.a.b.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.#.#.#.a.a.a.a.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.a.#.a.a.a.a.a.b.a.a.a.a.b.b.b.a.a.a.b.b.b.b.b.b.b.b.a.a.a.#.#.#.#.#.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.a.a.a.a.b.b.b.b.b.b.a.a.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.b.b.b.a.a.#.a.a.a.a.a.b.b.b.a.b.b.b.a.a.a.b.b.b.b.a.a.a.a.a.#.#.#.#.#.#.#.a.a.#.#.#.#.#.a.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.a.a.a.b.b.b.b.b.b.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.a.a.a.a.a.b.a.b.a.#.#.a.a.b.b.b.a.a.#.a.a.a.a.a.b.a.a.b.b.b.b.a.a.a.a.a.a.a.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.b.b.b.b.b.b.b.b.a.#.#.#.#.#.a.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.b.b.a.#.#.#.a.a.a.#.#.#.a.#.a.a.b.b.b.a.a.#.#.a.a.a.a.a.a.a.b.b.b.a.a.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.b.b.a.a.#.#.#.#.a.a.a.a.b.b.b.b.a.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.#.#.#.#.a.a.#.#.a.a.b.b.b.a.a.#.#.#.#.a.a.a.a.a.a.a.a.b.b.b.b.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.a.a.b.a.b.b.b.a.a.#.#.#.#.#.a.a.a.a.a.b.a.b.a.a.a.a.a.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.a.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.a.a.#.#.#.a.a.b.b.b.a.a.a.#.#.#.#.a.b.a.b.b.b.a.a.a.a.b.a.a.a.a.a.#.#.#.#.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b.a.a.a.a.a.a.#.#.#.#.a.a.#.a.a.a.a.b.a.b.b.b.a.b.b.b.b.b.b.b.b.b.a.a.b.b.b.a.a.a.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.a.a.b.b.b.b.b.b.b.b.b.b.a.a.a.#.#.#.#.#.#.#.#.#.a.a.a.a.#.#.#.#.a.a.a.b.b.a.a.a.#.#.a.b.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.a.a.a.a.a.a.a.a.a.b.b.a.a.a.a.a.a.a.a.a.b.a.#.#.#.#.#.#.#.a.b.a.a.a.a.b.b.b.a.a.a.a.a.b.b.b.b.b.b.a.a.a.a.a.a.a.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.a.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.#.#.#.#.#.#.a.a.b.b.a.a.a.a.a.b.a.a.b.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.a.a.b.b.b.b.b.a.#.#.#.#.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.a.a.a.b.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.a.a.b.b.b.b.b.b.b.b.b.b.a.#.#.#.a.a.#.a.#.#.#.#.#.#.a.a.a.#.#.#.#.#.#.#.a.a.b.a.#.a.a.a.a.a.b.b.a.a.a.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.b.a.b.a.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.a.a.a.a.a.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.a.a.b.b.b.b.b.b.b.b.b.a.a.#.a.#.a.a.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.a.a.a.#.#.#.a.a.a.b.b.a.a.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.b.a.#.#.#.#.#.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.a.a.a.a.a.a.a",
".#.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.a.a.b.b.b.b.b.b.b.b.b.a.a.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.a.a.a.a.a.a.a.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.a.a.b.a.a.a.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.a.a.b.b.a.a",
".#.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.a.b.b.b.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.#.#.#.#.#.#.a.a.a.a.a.a.a.b.#.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.a.#.#.#.#.#.#.a.b.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.a.a.b.b.a.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.b.b.b.b.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.a.#.#.#.a.a.a.#.a.#.a.a.a.a.a.a.#.#.#.#.#.#.#.a.a.a.b.a.a.a.a.a.a.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.b.b.b.b.b.b.a.a.a.#.#.#.#.a.a.a.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.a.a.b",
".#.#.#.#.#.#.#.b.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.a.a.a.a.a.a.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.b.a.a.a.#.#.#.#.a.a.a.#.#.#.#.#.#.#.#.#.a.a.a.#.b.a.a.a.a.a.a.a.a.b.a.a.a.a.b.b.a.a.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.a.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.a.b.b.b.b.a.a.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.#.#.#.#.#.#.a.a.a.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.a.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.b.#.#.#.#.#.#.#.#.#.a.a.a.#.#.a.a.a.a.#.#.a.a.b.b.b.b.b.b.a.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.b.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.#.#.a.#.#.#.#.#.#.#.a.a.a.#.#.#.a.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.#.#.#.#.#.#.#.#.#.a.a.a.#.#.#.a.a.b.b.b.b.b.b.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.a.a.a.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.a.a.a.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.a.a.b.b.b.b.a.a.a.#.#.#.#.#.#.a.a.a.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.a.#.#.#.#.#.#.a.b.b.b.b.b.a.a.a.a.a.a.#.#.#.#.#.a.a.#.#.#.#.a.a.a.b.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.a.a.a.a.a.#.#.a.b.b.b.b.b.b.b.a.b.b.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.b.b.a.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.a.a.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.a.b.b.a.a.b.b.b.b.b.b.b.b.b.a.b.b.b.b.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.#.a.a.#.#.a.a.#.#.#.a.a.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.#.a.a.a.a.a.a.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.#.#.#.a.a.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.a.a.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.#.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.a.a.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.a.a.a.a.a.a.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.#.#.a.a.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.a.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.a.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.a.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.#.#.#.#.a.a.a.a.#.#.#.#.#.#.a.a.a.#.#.#.#.#.#.a.a.b.b.b.b.b.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.a.b.b.b.b.b.b.b.b.a.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.a.a.a.a.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.a.a.b.b.b.b.b.b.b.b.a.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.a.b.b.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.a.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.a.a.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.a.a.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.a.a.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.a.a.a.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.a.b.b.b.b.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.a.b.b.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.a.a.a.a.a.b.b.b.b.a.b.b.b.b.b.b.b.a.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.a.a.a.#.b.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.a.#.#.#.b.a.a.b.b.b.a.a.b.a.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.a.a.a.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.a.a.#.#.#.#.a.b.b.b.b.a.a.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.a.b.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.a.a.#.#.#.b.a.b.b.b.b.a.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.b.a.a.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.a.b.b.a.a.a.#.#.a.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.a.#.a.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.a.a.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.a.a.a.a.b.b.b.b.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.b.b.a.a.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.a.b.b.b.b.a.b.b.b.a.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.b.#.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.b.b.b.b.b.a.b.a.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.b.b.b.b.b.b.b.a.a.b.b.b.a.a.b.b.a.a.a.a.a.a.a.a.a.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.a.a.a.a.a.a.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.b.b.a.b.b.b.b.a.a.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.a.a.b.b.b.b.b.b.b.b.b.a.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.a.a.a.b.b.b.b.a.a.a.b.b.a.a.a.a.a.b.b.b.a.a.a.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.a.a.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.a.a.b.b.b.a.b.b.b.b.b.b.b.a.a.b.b.b.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.a.a.a.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.a.a.a.b.a.a.b.b.b.b.b.b.b.a.b.b.b.b.a.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.a.a.b.b.b.b.b.a.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.a.b.a.b.a.b.b.b.b.b.b.b.b.a.b.b.b.b.a.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.a.a.a.a.a.a.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.a.a.a.b.b.b.b.b.b.b.a.b.b.b.b.b.a.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.a.a.b.b.b.b.b.b.b.a.b.b.b.b.b.a.b.a.a.a.a.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.a.a.a.b.b.b.b.b.b.a.b.b.b.b.b.a.a.a.a.a.a.a.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.a.b.b.b.b.a.a.a.a.b.b.a.a.a.a.#.#.#.#.#.a.a.b.b.b.b.a.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.a.a.a.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.a.b.a.b.a.a.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.a.a.a.a.b.b.b.b.a.a.a.a.a.a.a.a.b.a.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.a.b.b.b.b.b.a.b.b.a.a.a.a.a.a.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.a.a.a.a.a.a.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b.b.b.b.a.a.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.b.b.b.b.b.a.a.a.a.a.a.a.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.a.a.a.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.a.a.b.b.b.b.b.b.a.a.b.a.b.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.a.a.a.a.b.b.b.b.a.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.#.#.a.a.a.a.b.b.a.a.a.a.a.a.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.b.a.a.a.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.a.b.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.a.b.b.b.b.b.b.a.a.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.a.b.b.b.b.b.b.a.a.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.a.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.a.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.a.a.b.a.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.a.a.a.a.a.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.a.a.a.b.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.a.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.a.b.b.b.b.b.b.b.b.b.b.a.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.a.b.b.b.b.b.b.b.b.b.b.a.a.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.a.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.a.a.b.b.a.b.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.a.a.a.a.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.a.b.b.a.a.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.a.b.b.b.b.b.b.a.b.b.b.b.b.b.a.a.b.a.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.b.b.b.b.a.b.b.b.b.b.b.a.a.b.a.a.a.b.b.a.a.a.a.a.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.a.a.a.a.b.a.b.b.b.b.b.b.a.b.a.a.a.a.a.a.a.b.b.b.a.#.#.#.a.a.#.#.#.#.#.b.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.a.a.a.a.a.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.a.#.#.#.#.b.a.a.a.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.b.b.b.b.b.b.b.a.a.a.b.b.b.b.a.b.a.a.b.b.b.b.b.b.b.b.a.#.#.#.#.#.#.a.a.#.#.#.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.a.a.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.a.a.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.b.a.a.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.a.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.a.a.a.a.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.a.a.a.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.a.a.b.a.b.b.b.b.b.b.a.#.#.#.#.#.#.#.a.a.a.a.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.b.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.a.a.b.a.a.a.a.b.b.a.a.a.a.b.b.b.b.b.a.#.#.#.#.#.#.a.a.a.a.a.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.a.a.a.b.b.b.b.b.a.a.b.b.a.a.b.b.b.b.a.a.a.#.#.#.#.b.#.a.b.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.a.a.a.a.b.b.b.a.b.b.b.b.b.b.b.b.a.b.b.a.a.b.b.a.a.a.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.a.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.a.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.b.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.a.a.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.a.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.a.a.a.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.a.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.a.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.a.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.a.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.a.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.a.a.b.b.b.b.b.b.b.b.a.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.a.a.b.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.a.a.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.b.b.b.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.a.a.a.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.b.b.a.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.a.a.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.b.b.b.b.b.b.b.b.b.b.b.b.b.b.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.b.b.b.b.b.b.b.b.b.b.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.b.a.a.a.a.a.a.a.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.a.a.a.a.#",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
".d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
".d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.d.d.dQtQtQtQt.d.d.dQtQtQt.dQtQtQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.d.d.d.d.dQtQt.d.d.dQtQtQtQt.dQtQtQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQtQtQt.dQtQtQtQt.d.d.dQtQtQtQt.dQtQtQtQtQt.dQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQt.dQtQt.dQtQtQtQtQt.dQt.dQtQtQt.dQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQt.dQtQtQt.dQtQt.dQt.dQtQtQt.dQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.d.dQtQtQtQt.dQt.dQtQt.dQtQtQt.dQtQt.dQt.dQtQtQt.dQt.dQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQt.dQtQtQtQtQt.dQtQtQt.dQt.dQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQtQtQtQt.dQt.dQtQtQt.dQt.dQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQt.dQtQtQt.dQtQtQt.dQt.dQtQtQt.dQt.dQtQtQt.dQt.dQtQtQt.dQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.d.dQtQt.d.d.d.dQtQt.dQtQtQt.dQt.dQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQtQtQt.dQtQt.dQtQtQt.dQt.dQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQt.dQtQtQt.dQtQt.d.d.dQtQt.dQtQtQt.dQt.dQtQtQt.dQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQt.dQtQtQt.dQt.dQtQtQt.dQt.dQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQtQtQt.dQtQtQt.dQtQtQt.dQt.dQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQt.dQtQtQt.dQt.dQtQtQt.dQt.dQtQtQt.dQt.dQtQtQt.dQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQt.dQtQtQt.dQt.dQtQtQt.dQt.dQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQtQt.dQtQtQtQt.dQtQtQt.dQt.dQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQt.dQtQtQt.dQt.dQtQtQt.dQt.dQtQtQt.dQt.dQtQtQt.dQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQt.dQt.dQtQtQt.dQtQt.dQt.dQtQtQt.dQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQtQt.dQtQtQtQtQtQt.dQt.dQtQtQt.dQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQtQt.dQt.dQtQt.dQtQtQt.dQtQt.dQt.dQtQtQt.dQt.dQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.d.d.dQtQtQt.d.d.dQtQtQtQt.dQtQtQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.dQtQtQtQt.d.d.d.d.dQtQtQt.dQtQtQtQtQt.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.d.d.d.d.dQtQtQt.dQtQtQtQt.d.d.dQtQtQtQt.dQtQtQtQtQt.dQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.eQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.g.g.g.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.g.g.g.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQt.h.i.i.i.i.i.j.dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.i.kQtQtQtQtQtQtQtQtQtQtQtQtQtQt.l.m.n.o.p.qQtQtQtQtQtQt.r.sQtQtQt.i.kQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.g.g.g.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQtQtQt.t.u.vQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.w.x.y.z.A.BQtQtQtQtQtQt.d.CQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.g.g.g.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQtQt.D.E.F.GQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.H.IQtQtQtQtQtQt.d.CQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.g.g.g.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQtQt.J.K.LQtQtQt.t.M.N.O.P.QQtQt.t.M.N.O.P.QQtQt.d.R.S.K.T.U.V.W.X.Y.ZQtQtQtQtQtQt.d.CQtQt.d.0.1.X.2.3QtQtQtQtQtQtQtQtQtQt.4.5QtQtQtQtQt.6.d.i.i.7Qt.d.CQtQt.d.R.S.K.T.U.V.W.X.Y.ZQtQt.q.p.8.9#..zQtQt###a#b#c#dQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.d.d.d.d.d.d.d.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQt#e#f.0QtQtQtQt#g#h#i#j.Y#kQtQt#g#h#i#j.Y#kQtQt.d#l#m#n.B#o.7#p#q#r#sQtQtQtQtQtQt.d.CQtQt.d.1#t#u#d.1QtQtQtQtQtQtQtQtQt.Q#v#wQtQtQtQtQtQt.d.CQtQtQt.d.CQtQt.d#l#m#n.B#o.7#p#q#r#sQtQt#x#y#z#n.w#AQt#B.j#C#D.G#EQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.d.d.d.d.d.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQt#F#G#HQtQtQtQt#I.d#JQtQt#K.d#L#I.d#JQtQt#K.d#LQt.d.VQtQt#M.d.tQtQt#N#OQtQtQtQtQtQt.d.CQtQt.d.VQtQt#P#bQtQtQtQtQtQtQtQt#n#Q#A#RQtQtQtQtQtQt.d.CQtQtQt.d.CQtQt.d.VQtQt#M.d.tQtQt#N#OQt.b.d#tQtQt#j.d#t.Q.X#S.GQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.d.d.d.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQt.D#T.6.qQtQtQtQt#U.d#DQtQt.y.d#V#U.d#DQtQt.y.d#VQt.d.CQtQt.C.dQtQtQt#W#XQtQtQtQtQtQt.d.CQtQt.d.CQtQt#U.d.yQtQtQtQtQtQt.s.E#Y#jQtQtQtQtQtQtQt.d.CQtQtQt.d.CQtQt.d.CQtQt.C.dQtQtQt#W#XQt#V.d.i.i.i.i.i.xQt#M#Z.8#0#1QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.g.d.g.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQt#2.9#3QtQtQtQtQt#I.d#4QtQt#5.d#L#I.d#4QtQt#5.d#LQt.d.CQtQt.C.dQtQtQt#W#XQtQtQtQtQtQt.d.CQtQt.d.CQtQt#U.d.yQtQtQtQtQt.y#h#a.bQtQtQtQtQtQtQtQt.d.CQtQtQt.d.CQtQt.d.CQtQt.C.dQtQtQt#W#XQt#6.d.QQtQtQtQtQtQtQtQt.D#7.d#4QtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.g.g.g.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQt#8#f#SQtQtQtQtQtQtQt#9.a#p#i#y#TQtQt#9.a#p#i#y#TQtQt.d.CQtQt.C.dQtQtQt#W#XQtQtQtQtQtQt.d.CQtQt.d.CQtQt#U.d.yQtQtQtQtQt.Pa.#4QtQtQtQtQtQtQtQtQt.8.4a#QtQt.d.CQtQt.d.CQtQt.C.dQtQtQt#W#XQtQt#h.a#LQt#zaaQt.rabac.Dad.j#mQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.g.g.g.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtae.d.i.i.i.i.i.iafQt#zag.N.N.M.tQtQt#zag.N.N.M.tQtQt.d.CQtQt.C.dQtQtQt#W#XQtQtQtQtQtQt.d.CQtQt.d.CQtQt#U.d.yQtQtQtQt.y.d.j.i.i.i.iQtQtQtQtQtQtah.5.i.7Qt.d.CQtQt.d.CQtQt.C.dQtQtQt#W#XQtQtacai#G#bajakQt#4#Q.N#b.WalQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.g.g.g.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.g.g.g.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.g.g.g.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.g.g.g.g.g.g.g.g.g.g.g.g.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.e.e.e.e.e.e.e.e.e.e.e.e.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.eQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.e.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.e.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.fQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt##amamamanaoQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.Dao.y.Dao.yQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQta#apaqQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.Dao.y.Dao.yQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt#ear.DQtQt#iasatas#pQtQt#iasatas#pQt.Dao.Hauan#Uavawan.UQtQtQtQtQtQtaxayazaA.LQt.Dao.y.Dao.yQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtaBam#uQtQtQtaraCQtaCarQtQtaraCQtaCarQt.DaoafQt.bao#JQt#LaAQtQtQtQtQtQt#u.DQt.ZaDa#.Dao.y.Dao.yQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtaEaF#qQtQtQt.D.JaGQtaG.J.D.D.JaGQtaG.J.D.Dao.yQt#Raoa#Qt#naHQtQtQtQtQtQt.taa.hataoaG.Dao.y.Dao.yQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtaIaA#3QtQtQtQt.D.JaGQtaG.J.D.D.JaGQtaG.J.D.Dao.yQt#Rao.DQt#naJQtQtQtQtQtQtay.L#DQtao#R.Dao.y.Dao.yQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQta##1aKQtQtQtQtQtQtaraCQtaLarQtQtaraCQtaLarQt.Dao.yQt#Rao.DQt#naJQtQtQtQtQt.D.JaMQt#Lao#R.Dao.y.Dao.yQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.Zaoatamamamam.GQt#jasatas#iQtQt#jasatas#iQt.Dao.yQt#Rao.DQt#naJQtQtQtQtQtQtaKaHan.Vao#R.Dao.y.Dao.yQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.G#uQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQt#paoaNQtQtQtQtQtQtQtQtao#RQtQtQtQtQtao#RQtQtQtQt.HadQtQtQtaOaPQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.Dao.yQtaQaRaS#zQtQtQtamamamaoamamaTQtQtQtQtQtQtQtQtQtQtamamamaoamamaT.Dao.yQtQtQtQta#am#DQtQtQtQtQtQtQtQtQtQtQtQtaoaaQtQtQtaNao#RQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.Gao#RQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQt#E#7aU#RQtQtQtQtQtQtQtao#RQtQtQtQtQtao#RQtQtQtQt.say.zQt#Iau#DQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.Dao.yQtaD.GQtQtQtQtQtQtQt#RaoQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt#RaoQtQtQt.Dao.yQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtao.0#jQt.Z.4ao#RQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt#uao#DQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtaA#FaV.LQtQtQtacarat#Sao#RQtacarat#Sao#RQtQtQtQtQt.U.4.yau.rQt#iasatas#pQt#RaoQtQt#RaoQt.Dao.HaraOQtaWahamataXQtQt#uapamamaYQt.Dao.yabaoatarQtQtQtQtQtQt#RaoQtQtQt#iasatas#pQtQtQtQtQtQtQt#RaoQtQtQt.Dao#CauanaCQtQtaZQtQtaWahamataXQtQtQtQtQtQtao#3a0Qtav#3ao#RQtaxayazaA.LQt.Daoa0aAaz#6QtQt.saoQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt#BawQt.LaSQtQtQtap.CQtaGao#RQtap.CQtaGao#RQtQtQtQtQtQtaVaR#7QtQtaraCQtaCarQt#RaoQtQt#RaoQt.Dao#IQtQtQtaD.ZQt#R#tQtQtapa1Qt#BaHaG.Dao.yQtao#nQtQtQtQtQtQtQt#RaoQtQtQtaraCQtaCarQtQtQtQtQtQtQt#RaoQtQtQt.DaoafQt#qa2QtQtaZQtQtaD.ZQt#R#tQtQtQtQtQtQtao#R.R#zah.Dao#RQt#u.DQt.ZaDa#.Dao.rQt.U.0QtQt.ya3QtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQta4aPQt.saH.GQta#a3aIQtQtao#Ra#a3aIQtQtao#RQtQtQtQtQtQt.Qao#nQt.D.JaGQtaG.J.D#RaoQtQt#RaoQt.Dao.yQtQtQt#1apa5#Ba#Qta#a3amamamao.Q.Dao.yQtao#nQtQtQtQtQtQtQt#RaoQtQt.D.JaGQtaG.J.DQtQtQtQtQtQt#RaoQtQtQt.Dao.yQt#Dao#DQtaZQtQt#1apa5#Ba#QtQtQtQtQtQtao#RaCa6a5Qtao#RQt.taa.hataoaG.Dao.yQt.yao.DQtQtaUQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQt#DaZamamam#W.kQt.DaoaGQtQtao#R.DaoaGQtQtao#RQtQtQtQtQtQt#RaoQtQt.D.JaGQtaG.J.DaGaoQtQt#RaoQt.Dao.yQtQtQtQt.zaQa7#1Qt.D.J.sQtQtQtQt.Dao.yQtao#nQtQtQtQtQtQtQt#RaoQtQt.D.JaGQtaG.J.DQtQtQtQtQtQt#RaoQtQtQt.Dao.yQt.Dao.yQtaZQtQtQt.zaQa7#1QtQtQtQtQtQtao#R#D#1aGQtao#RQtay.L#DQtao#R.Dao.yQt#Ra3a#QtQta8QtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQta1a9QtQtQtafahQtQt.0#3Qt.bao#RQt.0#3Qt.bao#RQtQtQtQtQtQt#RaoQtQtQtaraCQtaLarQta#aU#qQta1aoQt.Dao.yQtQt#D#3.yQt#za2QtQtaVaKa#Qt#J#D.Dao.yQtao#nQtQtQtQtQtQtQt#RaoQtQtQtaraCQtaLarQtQtQtQtQtQtQt#RaoQtQtQt.Dao.yQt.Dao.yQtaZQt#D#3.yQt#za2QtQtQtQtQtQtao#RQtQtQtQtao#R.D.JaMQt#Lao#R.Daob.Qt#VapQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtaab#QtQtQt.DaF#FQt.z.h.h#7ao#RQt.z.h.h#7ao#RQtQtQtQtQtQt#RaoQtQtQt#jasatas#iQtQtaQanawavaoQt.Dao.yQtQt#Dapanam###KQtQtacbaayatap#D.Dao.yQtao#nQtQtQtQtQtQtQt#RaoQtQtQt#jasatas#iQtQtQtQtQtQtQt#RaoQtQtQt.Dao.yQt.Dao.yQtaZQt#Dapanam###KQtQtQtQtQtQtao#RQtQtQtQtao#RQtaKaHan.Vao#R.Dao#Satal.GQtQtaIama#QtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.G#uQtQtQt.Dao.yQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.Dao.yQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt"};


/*
 *  Constructs a PluginGuiBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PluginGuiBase::PluginGuiBase( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ),
      image1( (const char **) image1_data )
{
    QImage img;
    img.loadFromData( image0_data, sizeof( image0_data ), "PNG" );
    image0 = img;
    if ( !name )
	setName( "PluginGuiBase" );
    setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    setIcon( image0 );
    PluginGuiBaseLayout = new QGridLayout( this, 1, 1, 11, 6, "PluginGuiBaseLayout"); 

    line1 = new QFrame( this, "line1" );
    line1->setMaximumSize( QSize( 2, 32767 ) );
    line1->setFrameShape( QFrame::VLine );
    line1->setFrameShadow( QFrame::Sunken );
    line1->setFrameShape( QFrame::VLine );

    PluginGuiBaseLayout->addMultiCellWidget( line1, 0, 3, 1, 1 );

    layout73 = new QHBoxLayout( 0, 0, 6, "layout73"); 
    spacer2 = new QSpacerItem( 320, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout73->addItem( spacer2 );

    pbnOK = new QPushButton( this, "pbnOK" );
    pbnOK->setMaximumSize( QSize( 32767, 32767 ) );
    layout73->addWidget( pbnOK );

    pbnCancel = new QPushButton( this, "pbnCancel" );
    layout73->addWidget( pbnCancel );

    PluginGuiBaseLayout->addMultiCellLayout( layout73, 4, 4, 0, 2 );

    txtHeading = new QLabel( this, "txtHeading" );
    txtHeading->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, 0, 0, txtHeading->sizePolicy().hasHeightForWidth() ) );
    QFont txtHeading_font(  txtHeading->font() );
    txtHeading_font.setPointSize( 24 );
    txtHeading_font.setBold( TRUE );
    txtHeading->setFont( txtHeading_font ); 
    txtHeading->setAlignment( int( QLabel::AlignCenter ) );

    PluginGuiBaseLayout->addWidget( txtHeading, 0, 2 );

    teInstructions_2 = new QTextEdit( this, "teInstructions_2" );
    teInstructions_2->setWordWrap( QTextEdit::WidgetWidth );
    teInstructions_2->setReadOnly( TRUE );

    PluginGuiBaseLayout->addWidget( teInstructions_2, 1, 2 );

    layout3 = new QGridLayout( 0, 1, 1, 0, 6, "layout3"); 

    textLabel3 = new QLabel( this, "textLabel3" );

    layout3->addWidget( textLabel3, 3, 0 );

    textLabel1 = new QLabel( this, "textLabel1" );

    layout3->addWidget( textLabel1, 1, 0 );

    textLabel2_2 = new QLabel( this, "textLabel2_2" );

    layout3->addWidget( textLabel2_2, 5, 0 );

    leLongitude = new QLineEdit( this, "leLongitude" );

    layout3->addWidget( leLongitude, 7, 1 );

    textLabel1_4 = new QLabel( this, "textLabel1_4" );

    layout3->addWidget( textLabel1_4, 0, 0 );

    leEmail = new QLineEdit( this, "leEmail" );

    layout3->addWidget( leEmail, 1, 1 );

    textLabel1_3 = new QLabel( this, "textLabel1_3" );

    layout3->addWidget( textLabel1_3, 7, 0 );

    leName = new QLineEdit( this, "leName" );

    layout3->addWidget( leName, 0, 1 );

    leCountry = new QLineEdit( this, "leCountry" );

    layout3->addWidget( leCountry, 4, 1 );

    textLabel1_2 = new QLabel( this, "textLabel1_2" );

    layout3->addWidget( textLabel1_2, 4, 0 );

    leImageUrl = new QLineEdit( this, "leImageUrl" );

    layout3->addWidget( leImageUrl, 2, 1 );

    leHomeUrl = new QLineEdit( this, "leHomeUrl" );

    layout3->addWidget( leHomeUrl, 3, 1 );

    textLabel2 = new QLabel( this, "textLabel2" );

    layout3->addWidget( textLabel2, 2, 0 );

    textLabel3_2 = new QLabel( this, "textLabel3_2" );

    layout3->addWidget( textLabel3_2, 6, 0 );

    leLatitude = new QLineEdit( this, "leLatitude" );

    layout3->addWidget( leLatitude, 6, 1 );

    lePlaceDescription = new QLineEdit( this, "lePlaceDescription" );

    layout3->addWidget( lePlaceDescription, 5, 1 );

    PluginGuiBaseLayout->addMultiCellLayout( layout3, 2, 3, 2, 2 );

    pixmapLabel2 = new QLabel( this, "pixmapLabel2" );
    pixmapLabel2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, pixmapLabel2->sizePolicy().hasHeightForWidth() ) );
    pixmapLabel2->setMaximumSize( QSize( 150, 400 ) );
    pixmapLabel2->setPixmap( image1 );
    pixmapLabel2->setScaledContents( TRUE );

    PluginGuiBaseLayout->addMultiCellWidget( pixmapLabel2, 0, 2, 0, 0 );

    pbnGetCoords = new QPushButton( this, "pbnGetCoords" );

    PluginGuiBaseLayout->addWidget( pbnGetCoords, 3, 0 );
    languageChange();
    resize( QSize(702, 508).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( pbnOK, SIGNAL( clicked() ), this, SLOT( pbnOK_clicked() ) );
    connect( pbnCancel, SIGNAL( clicked() ), this, SLOT( pbnCancel_clicked() ) );
    connect( pbnGetCoords, SIGNAL( clicked() ), this, SLOT( pbnGetCoords_clicked() ) );

    // tab order
    setTabOrder( leName, leEmail );
    setTabOrder( leEmail, leImageUrl );
    setTabOrder( leImageUrl, leHomeUrl );
    setTabOrder( leHomeUrl, leCountry );
    setTabOrder( leCountry, lePlaceDescription );
    setTabOrder( lePlaceDescription, leLatitude );
    setTabOrder( leLatitude, leLongitude );
    setTabOrder( leLongitude, pbnGetCoords );
    setTabOrder( pbnGetCoords, pbnOK );
    setTabOrder( pbnOK, pbnCancel );
    setTabOrder( pbnCancel, teInstructions_2 );
}

/*
 *  Destroys the object and frees any allocated resources
 */
PluginGuiBase::~PluginGuiBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void PluginGuiBase::languageChange()
{
    setCaption( tr( "QGIS Plugin QGIS Community Registration Plugin" ) );
    pbnOK->setText( tr( "&OK" ) );
    pbnOK->setAccel( QKeySequence( tr( "Alt+O" ) ) );
    pbnCancel->setText( tr( "&Cancel" ) );
    pbnCancel->setAccel( QKeySequence( tr( "Alt+C" ) ) );
    txtHeading->setText( tr( "qgis.community.org" ) );
    teInstructions_2->setText( tr( "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\"font-size:12pt;font-family:Arial\">\n"
"<p style=\"margin-top:16px\"><span style=\"font-size:17pt;font-weight:600\">Description</span></p>\n"
"<p>This plugin will register you on the community.qgis.org users map.</p>\n"
"</body></html>\n"
"" ) );
    textLabel3->setText( tr( "Home URL:" ) );
    textLabel1->setText( tr( "Email:" ) );
    textLabel2_2->setText( tr( "Place Description:" ) );
    textLabel1_4->setText( tr( "Name:" ) );
    textLabel1_3->setText( tr( "Longitude (dec. degrees):" ) );
    textLabel1_2->setText( tr( "Country:" ) );
    textLabel2->setText( tr( "Image URL (50x50):" ) );
    textLabel3_2->setText( tr( "Latitude (dec. degrees):" ) );
    pbnGetCoords->setText( tr( "Get From Map" ) );
}

