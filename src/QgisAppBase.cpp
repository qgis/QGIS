/****************************************************************************
** Form implementation generated from reading ui file 'QgisAppBase.ui'
**
** Created: Fri Jul 5 08:49:38 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "QgisAppBase.h"

#include <qvariant.h>
#include <qframe.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>

#include "QgisAppBase.ui.h"
static const char* const image0_data[] = { 
"16 13 5 1",
"# c #040404",
"c c #808304",
". c #bfc2bf",
"a c #f3f704",
"b c #f3f7f3",
".........###....",
"........#...#.#.",
".............##.",
".###........###.",
"#aba#######.....",
"#babababab#.....",
"#ababababa#.....",
"#baba###########",
"#aba#ccccccccc#.",
"#ba#ccccccccc#..",
"#a#ccccccccc#...",
"##ccccccccc#....",
"###########....."};

static const char* const image1_data[] = { 
"22 22 165 2",
"Qt c None",
".f c #000000",
"aq c #090909",
"aI c #0b0b0b",
"#l c #0c0c0c",
".o c #1d1d1d",
"#w c #232323",
".T c #29292a",
"aA c #313131",
"aE c #323233",
"aG c #343434",
"aH c #363636",
"ar c #363637",
"ap c #373738",
"au c #39393a",
"#C c #398491",
"aB c #3a3a3a",
"ax c #3a3f40",
".a c #3b3b3c",
"aw c #3c3c3c",
".c c #3c3c3d",
"aC c #3d3d3e",
"#t c #3d8996",
".b c #3e3e3f",
"a. c #404040",
".d c #404041",
"#E c #404647",
"#Y c #414648",
"ag c #414748",
"af c #424849",
"#B c #4298a7",
".A c #434343",
"#m c #434344",
"#j c #4399a8",
".K c #444444",
".q c #444445",
"#N c #4496a5",
"#s c #449aa9",
"#c c #454545",
".p c #474747",
"#G c #484848",
"#D c #489ead",
"#Q c #494949",
".h c #49494a",
"ak c #4a4a4a",
".B c #4c4c4d",
"#A c #4cb7cb",
"a# c #4d4d4e",
"#r c #4dbace",
"aj c #4f4f4f",
"#O c #50a3b5",
"## c #50a6b9",
"#M c #52a7b3",
"#k c #54acb9",
".e c #585859",
"aa c #595959",
".# c #59595a",
"#0 c #5b5b5b",
"#i c #5bb1c4",
".V c #5c5c5c",
"#T c #5c5d5c",
".g c #5d5d5e",
"#L c #60b3c5",
".5 c #626262",
"#9 c #635920",
"#u c #63b5c4",
"al c #686868",
"#z c #68bdcf",
"#a c #6ab5c3",
"av c #6b6b6b",
"#7 c #6b7274",
"#x c #6c6c6c",
"#1 c #6c6d6c",
".2 c #6cbacb",
"#. c #6cc3d9",
".U c #6d6d6d",
".z c #6f6f70",
"ae c #707071",
"ai c #717171",
"ad c #717172",
"ay c #727273",
"ab c #737374",
"an c #747475",
"#h c #74c8dd",
"ac c #757576",
"ao c #767677",
"as c #777778",
"#y c #78c3d3",
"#H c #797a79",
"#3 c #79989c",
"#K c #79bfcc",
".n c #7a7a7b",
".r c #7b7879",
"#2 c #7b7c7b",
".3 c #7bc4d4",
".C c #7d7e7d",
"aF c #7f7f80",
".Q c #81c9d6",
".i c #868787",
".9 c #86d1e1",
"#F c #878787",
"#W c #87cad6",
".1 c #87cddc",
"#6 c #88a8ad",
".P c #8acdd9",
"#P c #8acfe1",
"#S c #8e8e8e",
"am c #928237",
"#q c #96d5e5",
"#g c #96d7e7",
"#V c #99d4e0",
".0 c #99d8e6",
"#R c #9b9b9b",
".I c #9bd6e2",
".4 c #9da2a1",
".O c #9dd9e6",
".W c #a2a7a7",
"#J c #a4dbe9",
".H c #a5dcea",
"#n c #a6abaa",
"#f c #a8e1eb",
"#4 c #a9c7ce",
"aD c #aaaaaa",
"#v c #abdfea",
"#Z c #ac993f",
".Z c #ace0eb",
".j c #afb4b4",
".R c #afe1ec",
"#p c #b1dbe4",
".N c #b1e2ec",
".m c #b2b7b7",
"#b c #b2c7cb",
"#5 c #b3cad0",
".L c #bababa",
".6 c #bac0c1",
"#U c #bae4ef",
".k c #bbc4c6",
".l c #bbc5c6",
".J c #bbe7ee",
"ah c #bda44c",
"#X c #bde6f0",
".G c #bde7f0",
".Y c #c3e9f0",
".M c #c5cfd2",
"#e c #c5e7ec",
".F c #c6edf5",
".8 c #cbe9f2",
".x c #cceff7",
"az c #cecece",
".S c #ced9d9",
".s c #cfdada",
"#I c #d7f0f4",
".w c #d7f3f8",
"#o c #d8edf1",
".y c #dce7e9",
".u c #dcf2f5",
".v c #ddf4fa",
"#8 c #dece9d",
"at c #e1e1e1",
"#d c #e3f2f4",
".t c #e5f0f2",
".X c #e5f4f8",
".7 c #e7f3f6",
".D c #e9f8fa",
".E c #ffffff",
"QtQtQtQtQt.#.a.b.c.d.e.f.fQtQtQtQtQtQtQtQtQt",
"QtQtQt.g.h.i.j.k.l.m.n.o.p.f.fQtQtQtQtQtQtQt",
"QtQt.q.r.s.t.u.v.w.x.v.y.z.A.f.fQtQtQtQtQtQt",
"Qt.B.C.y.D.E.D.F.G.H.I.J.y.C.K.f.fQtQtQtQtQt",
".L.h.M.D.E.E.E.D.N.O.P.Q.R.S.T.U.f.fQtQtQtQt",
".V.W.y.u.X.E.D.Y.Z.0.1.2.3.R.4.5.f.fQtQtQtQt",
".h.6.7.8.8.D.Y.R.0.9#.###a.N#b#c.f.f.fQtQtQt",
".h.k#d#e.J.N#f#g.9#h#i#j#k#f#b#l.f.f.fQtQtQt",
"#m#n#o#p.I.0#q.9#h#r#s#t#u#v.W#w.f.f.fQtQtQt",
"#x.C.s.R.P.Q#y#z#A#B#C#D#q.s#E.p.f.f.fQtQtQt",
"#F#G#H#I#J#K#L#M#N#t#O#P#o.4.f#Q.f.f.fQtQtQt",
"#R#S#T.C#o#U#V#W#y.1#X#o.C#Y.f#Z.f.f.fQtQtQt",
"#G.f#0#1#2#3#4#e#e#5#6#7#Y.f.E#8#9.f.fQtQtQt",
".f.fa..Va#aaabacadaeafag.fah#8.Eab.b.f.fQtQt",
".f.f.f.f.faia#ajak#Qal#l#9amafan.Eaoap.f.fQt",
"Qt.f.f.f.f.f.f.f.f.faq.f.f#9arafasat.nau.f.f",
"QtQtavaw.f.f.f.f.f.f.f.f.f.fararaxayaz.zaA.f",
"QtQtavaB.f.f.f.f.f.f.f.f.f.f.f.farafaeazaeaC",
"avavaDaBavav.f.f.f.f.f.f.f.f.f.faEarafaeaF.d",
"aGaGaH.f.f.fQtQtQtQtQtQtQt.f.f.f.f.farafaI.d",
"QtQtal.fQtQtQtQtQtQtQtQtQtQt.f.f.f.f.f.f.d.f",
"QtQtav.fQtQtQtQtQtQtQtQtQtQtQt.f.f.f.f.f.f.f"};

static const char* const image2_data[] = { 
"22 22 83 2",
"Qt c None",
".b c #000000",
".E c #060504",
".L c #070605",
".D c #080605",
".h c #0a0807",
".a c #0f0d0a",
".i c #100d0b",
".o c #110e0b",
"#e c #120e0c",
"#l c #13100d",
".p c #14100d",
".# c #14110d",
".Q c #15110e",
"#n c #17120f",
".9 c #191511",
"#b c #1b1612",
"#j c #1c1713",
".T c #1f1a15",
".W c #201a15",
".V c #231c17",
".Z c #241d18",
".2 c #241e18",
".K c #251f19",
".c c #261f19",
"#q c #27201a",
".R c #28201a",
"#k c #28211b",
".P c #2b231c",
".v c #2b231d",
".U c #2d241e",
".8 c #2d251e",
"#o c #2e261f",
"#i c #302720",
".g c #322921",
".1 c #332a22",
"#p c #342b23",
".S c #352b24",
"#d c #372d25",
".d c #392f27",
"#h c #3a2f27",
"#f c #3b3027",
"#m c #3c3128",
".j c #3e3229",
"#. c #3e332a",
".5 c #40342a",
".7 c #40342b",
"#g c #41352b",
".q c #42362c",
"## c #44372d",
".C c #44382d",
"#a c #45392f",
".0 c #46392e",
".N c #46392f",
".X c #483b30",
".F c #493c31",
".O c #4a3c31",
".M c #4a3c32",
".k c #4b3d31",
".6 c #4b3d32",
".B c #4c3e32",
".G c #4c3e33",
".4 c #4c3f33",
".n c #4d3f33",
".Y c #4d3f34",
"#c c #4d4034",
".J c #4e4034",
".r c #4f4035",
".H c #514236",
".x c #524337",
".e c #544438",
".m c #554538",
".f c #554539",
".u c #564639",
".l c #57473a",
".3 c #57483a",
".y c #58483b",
".s c #5a4a3c",
".I c #5e4c3e",
".A c #5e4e3f",
".t c #615041",
".z c #645243",
".w c #ffffff",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQt.#.#.aQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQt.b.c.d.e.f.g.hQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQt.i.j.k.l.m.n.n.oQtQtQtQt",
"QtQtQtQtQtQtQtQtQt.p.q.r.s.t.m.u.v.wQtQtQtQt",
"QtQtQtQtQtQtQtQtQt.c.x.y.z.A.B.C.DQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.E.F.G.H.I.n.J.K.wQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.L.M.N.G.O.P.o.wQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.w.Q.R.S.T.w.wQtQtQtQtQtQtQt",
"QtQtQtQtQtQt.U.VQt.w.w.w.wQtQtQtQt.b.bQtQtQt",
"QtQtQtQtQt.W.X.Y.ZQtQtQtQtQtQt.b.b.0.1.bQtQt",
"QtQtQtQtQt.2.M.3.4.bQtQtQtQt.b.C.5.6.7.bQtQt",
"QtQtQtQtQt.w.8.b.9.wQtQtQt.b.6.F#..N##.bQtQt",
"QtQtQtQtQtQt.w.w.wQtQtQtQt.b#a.G.n.x#b.wQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQt.Z#c.N.q.G#d#eQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQt.b.6#f#g.N.v.bQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQt.b###h.7#i.b.wQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQt#j.7.7.b.b.wQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQt.hQt.w.b.b.w.wQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQt#k.O#lQt.w.wQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQt.b.H#m#nQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQt#o#p#q.iQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQt.w.b.b.wQtQtQtQtQtQtQtQtQt"};

static const char* const image3_data[] = { 
"22 22 163 2",
"Qt c None",
".f c #000000",
"av c #060606",
"aq c #090909",
"aG c #0b0b0b",
"#l c #0c0c0c",
".o c #1d1d1d",
"#w c #232323",
".T c #29292a",
"az c #313131",
"aC c #323233",
"aE c #343434",
"aF c #363636",
"ar c #363637",
"ap c #373738",
"au c #39393a",
"#C c #398491",
"aw c #3a3f40",
".a c #3b3b3c",
".c c #3c3c3d",
"aA c #3d3d3e",
"#t c #3d8996",
".b c #3e3e3f",
"a. c #404040",
".d c #404041",
"#E c #404647",
"#Y c #414648",
"ag c #414748",
"af c #424849",
"#B c #4298a7",
".A c #434343",
"#m c #434344",
"#j c #4399a8",
".K c #444444",
".q c #444445",
"#N c #4496a5",
"#s c #449aa9",
"#c c #454545",
".p c #474747",
"#G c #484848",
"#D c #489ead",
"#Q c #494949",
".h c #49494a",
"ak c #4a4a4a",
".B c #4c4c4d",
"#A c #4cb7cb",
"a# c #4d4d4e",
"#r c #4dbace",
"aj c #4f4f4f",
"#O c #50a3b5",
"## c #50a6b9",
"#M c #52a7b3",
"#k c #54acb9",
".e c #585859",
"aa c #595959",
".# c #59595a",
"#0 c #5b5b5b",
"#i c #5bb1c4",
".V c #5c5c5c",
"#T c #5c5d5c",
".g c #5d5d5e",
"#L c #60b3c5",
".5 c #626262",
"#9 c #635920",
"#u c #63b5c4",
"al c #686868",
"#z c #68bdcf",
"#a c #6ab5c3",
"aB c #6b6b6b",
"#7 c #6b7274",
"#x c #6c6c6c",
"#1 c #6c6d6c",
".2 c #6cbacb",
"#. c #6cc3d9",
".U c #6d6d6d",
".z c #6f6f70",
"ae c #707071",
"ai c #717171",
"ad c #717172",
"ax c #727273",
"ab c #737374",
"an c #747475",
"#h c #74c8dd",
"ac c #757576",
"ao c #767677",
"as c #777778",
"#y c #78c3d3",
"#H c #797a79",
"#3 c #79989c",
"#K c #79bfcc",
".n c #7a7a7b",
".r c #7b7879",
"#2 c #7b7c7b",
".3 c #7bc4d4",
".C c #7d7e7d",
"aD c #7f7f80",
".Q c #81c9d6",
".i c #868787",
".9 c #86d1e1",
"#F c #878787",
"#W c #87cad6",
".1 c #87cddc",
"#6 c #88a8ad",
".P c #8acdd9",
"#P c #8acfe1",
"#S c #8e8e8e",
"am c #928237",
"#q c #96d5e5",
"#g c #96d7e7",
"#V c #99d4e0",
".0 c #99d8e6",
"#R c #9b9b9b",
".I c #9bd6e2",
".4 c #9da2a1",
".O c #9dd9e6",
".W c #a2a7a7",
"#J c #a4dbe9",
".H c #a5dcea",
"#n c #a6abaa",
"#f c #a8e1eb",
"#4 c #a9c7ce",
"#v c #abdfea",
"#Z c #ac993f",
".Z c #ace0eb",
".j c #afb4b4",
".R c #afe1ec",
"#p c #b1dbe4",
".N c #b1e2ec",
".m c #b2b7b7",
"#b c #b2c7cb",
"#5 c #b3cad0",
".L c #bababa",
".6 c #bac0c1",
"#U c #bae4ef",
".k c #bbc4c6",
".l c #bbc5c6",
".J c #bbe7ee",
"ah c #bda44c",
"#X c #bde6f0",
".G c #bde7f0",
".Y c #c3e9f0",
".M c #c5cfd2",
"#e c #c5e7ec",
".F c #c6edf5",
".8 c #cbe9f2",
".x c #cceff7",
"ay c #cecece",
".S c #ced9d9",
".s c #cfdada",
"#I c #d7f0f4",
".w c #d7f3f8",
"#o c #d8edf1",
".y c #dce7e9",
".u c #dcf2f5",
".v c #ddf4fa",
"#8 c #dece9d",
"at c #e1e1e1",
"#d c #e3f2f4",
".t c #e5f0f2",
".X c #e5f4f8",
".7 c #e7f3f6",
".D c #e9f8fa",
".E c #ffffff",
"QtQtQtQtQt.#.a.b.c.d.e.f.fQtQtQtQtQtQtQtQtQt",
"QtQtQt.g.h.i.j.k.l.m.n.o.p.f.fQtQtQtQtQtQtQt",
"QtQt.q.r.s.t.u.v.w.x.v.y.z.A.f.fQtQtQtQtQtQt",
"Qt.B.C.y.D.E.D.F.G.H.I.J.y.C.K.f.fQtQtQtQtQt",
".L.h.M.D.E.E.E.D.N.O.P.Q.R.S.T.U.f.fQtQtQtQt",
".V.W.y.u.X.E.D.Y.Z.0.1.2.3.R.4.5.f.fQtQtQtQt",
".h.6.7.8.8.D.Y.R.0.9#.###a.N#b#c.f.f.fQtQtQt",
".h.k#d#e.J.N#f#g.9#h#i#j#k#f#b#l.f.f.fQtQtQt",
"#m#n#o#p.I.0#q.9#h#r#s#t#u#v.W#w.f.f.fQtQtQt",
"#x.C.s.R.P.Q#y#z#A#B#C#D#q.s#E.p.f.f.fQtQtQt",
"#F#G#H#I#J#K#L#M#N#t#O#P#o.4.f#Q.f.f.fQtQtQt",
"#R#S#T.C#o#U#V#W#y.1#X#o.C#Y.f#Z.f.f.fQtQtQt",
"#G.f#0#1#2#3#4#e#e#5#6#7#Y.f.E#8#9.f.fQtQtQt",
".f.fa..Va#aaabacadaeafag.fah#8.Eab.b.f.fQtQt",
".f.f.f.f.faia#ajak#Qal#l#9amafan.Eaoap.f.fQt",
"Qt.f.f.f.f.f.f.f.f.faq.f.f#9arafasat.nau.f.f",
"QtQt.fav.f.f.f.f.f.f.f.f.f.fararawaxay.zaz.f",
"QtQtQt.f.f.f.f.f.f.f.f.f.f.f.f.farafaeayaeaA",
"aBaBaBaBaBaB.f.f.f.f.f.f.f.f.f.faCarafaeaD.d",
"aEaEaF.f.f.fQtQtQtQtQtQtQt.f.f.f.f.farafaG.d",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.f.f.f.f.d.f",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.f.f.f.f.f.f.f"};

static const char* const image4_data[] = { 
"32 32 5 1",
". c None",
"a c #00c0c0",
"c c #808080",
"b c #c0ffff",
"# c #ffff00",
"....................#...........",
"................................",
"................................",
"............#.....#.............",
"......#.........................",
".................aaabc..........",
".................aaab#..........",
".................aabcc.....#...#",
".....#..#..#....a#a#c..###......",
"..............#.aabc##..........",
"...............aaabc#..#.......#",
".##......#.....aaabc#...........",
"...##.....#...##abcc.......#....",
"..............aaa#c...........#.",
"..........#...aaabc.#..#......#.",
"...aaaaaa#aaaaaaaaaaaaaaaaaaaa..",
"...aaaaaaaaaaaaaaaaaaaaaaaaaac..",
"...cbbb#bbbbbaabbbbbbbbbbbbbbc..",
"...cccccccccaabbccccccc#cccccc..",
"...#.......aaabcc..#............",
"...........aaab#...#............",
"..#..#.....aabcc...#............",
"...#......aaabc.................",
"..#.......aabbc.....#.#.........",
".......#.aa#bcc#................",
"....#....aabbc....#.#...........",
"..#.....aaabcc...#..#...#.......",
"..#.....aaabc...................",
".......bbbbbc...................",
".......cccccc...........#.......",
"..................#....#........",
"................................"};


/* 
 *  Constructs a QgisAppBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 */
QgisAppBase::QgisAppBase( QWidget* parent,  const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    (void)statusBar();
    QPixmap image0( ( const char** ) image0_data );
    QPixmap image1( ( const char** ) image1_data );
    QPixmap image2( ( const char** ) image2_data );
    QPixmap image3( ( const char** ) image3_data );
    QPixmap image4( ( const char** ) image4_data );
    if ( !name )
	setName( "QgisAppBase" );
    resize( 609, 451 ); 
    QFont f( font() );
    f.setFamily( "Helvetica [Urw]" );
    setFont( f ); 
    setCaption( trUtf8( "Quantum GIS" ) );
    setCentralWidget( new QWidget( this, "qt_central_widget" ) );
    QgisAppBaseLayout = new QGridLayout( centralWidget(), 1, 1, 3, 6, "QgisAppBaseLayout"); 

    frameMain = new QFrame( centralWidget(), "frameMain" );
    frameMain->setFrameShape( QFrame::StyledPanel );
    frameMain->setFrameShadow( QFrame::Raised );

    QgisAppBaseLayout->addWidget( frameMain, 0, 0 );

    // actions
    actionFileOpen = new QAction( this, "actionFileOpen" );
    actionFileOpen->setIconSet( QIconSet( image0 ) );
    actionFileOpen->setText( trUtf8( "Open Project" ) );
    actionFileExit = new QAction( this, "actionFileExit" );
    actionFileExit->setText( trUtf8( "Exit" ) );
    actionFileExit->setMenuText( trUtf8( "E&xit" ) );
    actionZoomIn = new QAction( this, "actionZoomIn" );
    actionZoomIn->setIconSet( QIconSet( image1 ) );
    actionZoomIn->setText( trUtf8( "Zoom In" ) );
    actionZoomIn->setMenuText( trUtf8( "Zoom &In" ) );
    actionPan = new QAction( this, "actionPan" );
    actionPan->setIconSet( QIconSet( image2 ) );
    actionPan->setText( trUtf8( "Pan" ) );
    actionPan->setMenuText( trUtf8( "&Pan" ) );
    actionZoomOut = new QAction( this, "actionZoomOut" );
    actionZoomOut->setIconSet( QIconSet( image3 ) );
    actionZoomOut->setText( trUtf8( "Zoom out" ) );
    actionZoomOut->setMenuText( trUtf8( "Zoom &out" ) );
    actionAddLayer = new QAction( this, "actionAddLayer" );
    actionAddLayer->setIconSet( QIconSet( image4 ) );
    actionAddLayer->setText( trUtf8( "Add Layer" ) );


    // toolbars
    Toolbar = new QToolBar( "", this, DockTop ); 

    Toolbar->setLabel( trUtf8( "Toolbar" ) );
    actionFileOpen->addTo( Toolbar );
    mapNavigationToolbar = new QToolBar( "", this, DockTop ); 

    mapNavigationToolbar->setLabel( trUtf8( "Toolbar_2" ) );
    actionZoomIn->addTo( mapNavigationToolbar );
    actionZoomOut->addTo( mapNavigationToolbar );
    actionPan->addTo( mapNavigationToolbar );
    Toolbar_2 = new QToolBar( "", this, DockTop ); 

    Toolbar_2->setLabel( trUtf8( "Toolbar_2" ) );
    actionAddLayer->addTo( Toolbar_2 );


    // menubar
    menubar = new QMenuBar( this, "menubar" );

    QFont menubar_font(  menubar->font() );
    menubar_font.setPointSize( 10 );
    menubar->setFont( menubar_font ); 
    menubar->setFrameShape( QMenuBar::MenuBarPanel );
    menubar->setFrameShadow( QMenuBar::Raised );
    PopupMenu = new QPopupMenu( this ); 
    actionFileOpen->addTo( PopupMenu );
    actionFileExit->addTo( PopupMenu );
    menubar->insertItem( trUtf8( "File" ), PopupMenu );

    PopupMenu_2 = new QPopupMenu( this ); 
    actionAddLayer->addTo( PopupMenu_2 );
    menubar->insertItem( trUtf8( "View" ), PopupMenu_2 );



    // signals and slots connections
    connect( actionFileExit, SIGNAL( activated() ), this, SLOT( fileExit() ) );
    connect( actionFileOpen, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
    connect( actionAddLayer, SIGNAL( activated() ), this, SLOT( addLayer() ) );
    connect( actionZoomOut, SIGNAL( activated() ), this, SLOT( zoomOut() ) );
    connect( actionZoomIn, SIGNAL( activated() ), this, SLOT( zoomIn() ) );
    init();
}

/*  
 *  Destroys the object and frees any allocated resources
 */
QgisAppBase::~QgisAppBase()
{
    // no need to delete child widgets, Qt does it all for us
}

