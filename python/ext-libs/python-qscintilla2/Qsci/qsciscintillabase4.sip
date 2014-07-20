// This is the SIP interface definition for QsciScintillaBase.
//
// Copyright (c) 2014 Riverbank Computing Limited <info@riverbankcomputing.com>
// 
// This file is part of QScintilla.
// 
// This file may be used under the terms of the GNU General Public
// License versions 2.0 or 3.0 as published by the Free Software
// Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
// included in the packaging of this file.  Alternatively you may (at
// your option) use any later version of the GNU General Public
// License if such license has been publicly approved by Riverbank
// Computing Limited (or its successors, if any) and the KDE Free Qt
// Foundation. In addition, as a special exception, Riverbank gives you
// certain additional rights. These rights are described in the Riverbank
// GPL Exception version 1.1, which can be found in the file
// GPL_EXCEPTION.txt in this package.
// 
// If you are unsure which license is appropriate for your use, please
// contact the sales department at sales@riverbankcomputing.com.
// 
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.


class QsciScintillaBase : QAbstractScrollArea
{
%TypeHeaderCode
#include <Qsci/qsciscintillabase.h>
%End

public:
    enum {
        SCI_START,
        SCI_OPTIONAL_START,
        SCI_LEXER_START,
        SCI_ADDTEXT,
        SCI_ADDSTYLEDTEXT,
        SCI_INSERTTEXT,
        SCI_CLEARALL,
        SCI_CLEARDOCUMENTSTYLE,
        SCI_GETLENGTH,
        SCI_GETCHARAT,
        SCI_GETCURRENTPOS,
        SCI_GETANCHOR,
        SCI_GETSTYLEAT,
        SCI_REDO,
        SCI_SETUNDOCOLLECTION,
        SCI_SELECTALL,
        SCI_SETSAVEPOINT,
        SCI_GETSTYLEDTEXT,
        SCI_CANREDO,
        SCI_MARKERLINEFROMHANDLE,
        SCI_MARKERDELETEHANDLE,
        SCI_GETUNDOCOLLECTION,
        SCI_GETVIEWWS,
        SCI_SETVIEWWS,
        SCI_POSITIONFROMPOINT,
        SCI_POSITIONFROMPOINTCLOSE,
        SCI_GOTOLINE,
        SCI_GOTOPOS,
        SCI_SETANCHOR,
        SCI_GETCURLINE,
        SCI_GETENDSTYLED,
        SCI_CONVERTEOLS,
        SCI_GETEOLMODE,
        SCI_SETEOLMODE,
        SCI_STARTSTYLING,
        SCI_SETSTYLING,
        SCI_GETBUFFEREDDRAW,
        SCI_SETBUFFEREDDRAW,
        SCI_SETTABWIDTH,
        SCI_GETTABWIDTH,
        SCI_SETCODEPAGE,
        SCI_MARKERDEFINE,
        SCI_MARKERSETFORE,
        SCI_MARKERSETBACK,
        SCI_MARKERADD,
        SCI_MARKERDELETE,
        SCI_MARKERDELETEALL,
        SCI_MARKERGET,
        SCI_MARKERNEXT,
        SCI_MARKERPREVIOUS,
        SCI_MARKERDEFINEPIXMAP,
        SCI_SETMARGINTYPEN,
        SCI_GETMARGINTYPEN,
        SCI_SETMARGINWIDTHN,
        SCI_GETMARGINWIDTHN,
        SCI_SETMARGINMASKN,
        SCI_GETMARGINMASKN,
        SCI_SETMARGINSENSITIVEN,
        SCI_GETMARGINSENSITIVEN,
        SCI_SETMARGINCURSORN,
        SCI_GETMARGINCURSORN,
        SCI_STYLECLEARALL,
        SCI_STYLESETFORE,
        SCI_STYLESETBACK,
        SCI_STYLESETBOLD,
        SCI_STYLESETITALIC,
        SCI_STYLESETSIZE,
        SCI_STYLESETFONT,
        SCI_STYLESETEOLFILLED,
        SCI_STYLERESETDEFAULT,
        SCI_STYLESETUNDERLINE,
        SCI_STYLESETCASE,
        SCI_STYLESETCHARACTERSET,
        SCI_SETSELFORE,
        SCI_SETSELBACK,
        SCI_SETCARETFORE,
        SCI_ASSIGNCMDKEY,
        SCI_CLEARCMDKEY,
        SCI_CLEARALLCMDKEYS,
        SCI_SETSTYLINGEX,
        SCI_STYLESETVISIBLE,
        SCI_GETCARETPERIOD,
        SCI_SETCARETPERIOD,
        SCI_SETWORDCHARS,
        SCI_BEGINUNDOACTION,
        SCI_ENDUNDOACTION,
        SCI_INDICSETSTYLE,
        SCI_INDICGETSTYLE,
        SCI_INDICSETFORE,
        SCI_INDICGETFORE,
        SCI_SETWHITESPACEFORE,
        SCI_SETWHITESPACEBACK,
        SCI_SETWHITESPACESIZE,
        SCI_GETWHITESPACESIZE,
        SCI_SETSTYLEBITS,
        SCI_GETSTYLEBITS,
        SCI_SETLINESTATE,
        SCI_GETLINESTATE,
        SCI_GETMAXLINESTATE,
        SCI_GETCARETLINEVISIBLE,
        SCI_SETCARETLINEVISIBLE,
        SCI_GETCARETLINEBACK,
        SCI_SETCARETLINEBACK,
        SCI_STYLESETCHANGEABLE,
        SCI_AUTOCSHOW,
        SCI_AUTOCCANCEL,
        SCI_AUTOCACTIVE,
        SCI_AUTOCPOSSTART,
        SCI_AUTOCCOMPLETE,
        SCI_AUTOCSTOPS,
        SCI_AUTOCSETSEPARATOR,
        SCI_AUTOCGETSEPARATOR,
        SCI_AUTOCSELECT,
        SCI_AUTOCSETCANCELATSTART,
        SCI_AUTOCGETCANCELATSTART,
        SCI_AUTOCSETFILLUPS,
        SCI_AUTOCSETCHOOSESINGLE,
        SCI_AUTOCGETCHOOSESINGLE,
        SCI_AUTOCSETIGNORECASE,
        SCI_AUTOCGETIGNORECASE,
        SCI_USERLISTSHOW,
        SCI_AUTOCSETAUTOHIDE,
        SCI_AUTOCGETAUTOHIDE,
        SCI_AUTOCSETDROPRESTOFWORD,
        SCI_AUTOCGETDROPRESTOFWORD,
        SCI_SETINDENT,
        SCI_GETINDENT,
        SCI_SETUSETABS,
        SCI_GETUSETABS,
        SCI_SETLINEINDENTATION,
        SCI_GETLINEINDENTATION,
        SCI_GETLINEINDENTPOSITION,
        SCI_GETCOLUMN,
        SCI_SETHSCROLLBAR,
        SCI_GETHSCROLLBAR,
        SCI_SETINDENTATIONGUIDES,
        SCI_GETINDENTATIONGUIDES,
        SCI_SETHIGHLIGHTGUIDE,
        SCI_GETHIGHLIGHTGUIDE,
        SCI_GETLINEENDPOSITION,
        SCI_GETCODEPAGE,
        SCI_GETCARETFORE,
        SCI_GETREADONLY,
        SCI_SETCURRENTPOS,
        SCI_SETSELECTIONSTART,
        SCI_GETSELECTIONSTART,
        SCI_SETSELECTIONEND,
        SCI_GETSELECTIONEND,
        SCI_SETPRINTMAGNIFICATION,
        SCI_GETPRINTMAGNIFICATION,
        SCI_SETPRINTCOLOURMODE,
        SCI_GETPRINTCOLOURMODE,
        SCI_FINDTEXT,
        SCI_FORMATRANGE,
        SCI_GETFIRSTVISIBLELINE,
        SCI_GETLINE,
        SCI_GETLINECOUNT,
        SCI_SETMARGINLEFT,
        SCI_GETMARGINLEFT,
        SCI_SETMARGINRIGHT,
        SCI_GETMARGINRIGHT,
        SCI_GETMODIFY,
        SCI_SETSEL,
        SCI_GETSELTEXT,
        SCI_GETTEXTRANGE,
        SCI_HIDESELECTION,
        SCI_POINTXFROMPOSITION,
        SCI_POINTYFROMPOSITION,
        SCI_LINEFROMPOSITION,
        SCI_POSITIONFROMLINE,
        SCI_LINESCROLL,
        SCI_SCROLLCARET,
        SCI_REPLACESEL,
        SCI_SETREADONLY,
        SCI_NULL,
        SCI_CANPASTE,
        SCI_CANUNDO,
        SCI_EMPTYUNDOBUFFER,
        SCI_UNDO,
        SCI_CUT,
        SCI_COPY,
        SCI_PASTE,
        SCI_CLEAR,
        SCI_SETTEXT,
        SCI_GETTEXT,
        SCI_GETTEXTLENGTH,
        SCI_GETDIRECTFUNCTION,
        SCI_GETDIRECTPOINTER,
        SCI_SETOVERTYPE,
        SCI_GETOVERTYPE,
        SCI_SETCARETWIDTH,
        SCI_GETCARETWIDTH,
        SCI_SETTARGETSTART,
        SCI_GETTARGETSTART,
        SCI_SETTARGETEND,
        SCI_GETTARGETEND,
        SCI_REPLACETARGET,
        SCI_REPLACETARGETRE,
        SCI_SEARCHINTARGET,
        SCI_SETSEARCHFLAGS,
        SCI_GETSEARCHFLAGS,
        SCI_CALLTIPSHOW,
        SCI_CALLTIPCANCEL,
        SCI_CALLTIPACTIVE,
        SCI_CALLTIPPOSSTART,
        SCI_CALLTIPSETHLT,
        SCI_CALLTIPSETBACK,
        SCI_CALLTIPSETFORE,
        SCI_CALLTIPSETFOREHLT,
        SCI_AUTOCSETMAXWIDTH,
        SCI_AUTOCGETMAXWIDTH,
        SCI_AUTOCSETMAXHEIGHT,
        SCI_AUTOCGETMAXHEIGHT,
        SCI_CALLTIPUSESTYLE,
        SCI_VISIBLEFROMDOCLINE,
        SCI_DOCLINEFROMVISIBLE,
        SCI_SETFOLDLEVEL,
        SCI_GETFOLDLEVEL,
        SCI_GETLASTCHILD,
        SCI_GETFOLDPARENT,
        SCI_SHOWLINES,
        SCI_HIDELINES,
        SCI_GETLINEVISIBLE,
        SCI_SETFOLDEXPANDED,
        SCI_GETFOLDEXPANDED,
        SCI_TOGGLEFOLD,
        SCI_ENSUREVISIBLE,
        SCI_SETFOLDFLAGS,
        SCI_ENSUREVISIBLEENFORCEPOLICY,
        SCI_WRAPCOUNT,
        SCI_SETTABINDENTS,
        SCI_GETTABINDENTS,
        SCI_SETBACKSPACEUNINDENTS,
        SCI_GETBACKSPACEUNINDENTS,
        SCI_SETMOUSEDWELLTIME,
        SCI_GETMOUSEDWELLTIME,
        SCI_WORDSTARTPOSITION,
        SCI_WORDENDPOSITION,
        SCI_SETWRAPMODE,
        SCI_GETWRAPMODE,
        SCI_SETLAYOUTCACHE,
        SCI_GETLAYOUTCACHE,
        SCI_SETSCROLLWIDTH,
        SCI_GETSCROLLWIDTH,
        SCI_TEXTWIDTH,
        SCI_SETENDATLASTLINE,
        SCI_GETENDATLASTLINE,
        SCI_TEXTHEIGHT,
        SCI_SETVSCROLLBAR,
        SCI_GETVSCROLLBAR,
        SCI_APPENDTEXT,
        SCI_GETTWOPHASEDRAW,
        SCI_SETTWOPHASEDRAW,
        SCI_AUTOCGETTYPESEPARATOR,
        SCI_AUTOCSETTYPESEPARATOR,
        SCI_TARGETFROMSELECTION,
        SCI_LINESJOIN,
        SCI_LINESSPLIT,
        SCI_SETFOLDMARGINCOLOUR,
        SCI_SETFOLDMARGINHICOLOUR,
        SCI_MARKERSETBACKSELECTED,
        SCI_MARKERENABLEHIGHLIGHT,
        SCI_LINEDOWN,
        SCI_LINEDOWNEXTEND,
        SCI_LINEUP,
        SCI_LINEUPEXTEND,
        SCI_CHARLEFT,
        SCI_CHARLEFTEXTEND,
        SCI_CHARRIGHT,
        SCI_CHARRIGHTEXTEND,
        SCI_WORDLEFT,
        SCI_WORDLEFTEXTEND,
        SCI_WORDRIGHT,
        SCI_WORDRIGHTEXTEND,
        SCI_HOME,
        SCI_HOMEEXTEND,
        SCI_LINEEND,
        SCI_LINEENDEXTEND,
        SCI_DOCUMENTSTART,
        SCI_DOCUMENTSTARTEXTEND,
        SCI_DOCUMENTEND,
        SCI_DOCUMENTENDEXTEND,
        SCI_PAGEUP,
        SCI_PAGEUPEXTEND,
        SCI_PAGEDOWN,
        SCI_PAGEDOWNEXTEND,
        SCI_EDITTOGGLEOVERTYPE,
        SCI_CANCEL,
        SCI_DELETEBACK,
        SCI_TAB,
        SCI_BACKTAB,
        SCI_NEWLINE,
        SCI_FORMFEED,
        SCI_VCHOME,
        SCI_VCHOMEEXTEND,
        SCI_ZOOMIN,
        SCI_ZOOMOUT,
        SCI_DELWORDLEFT,
        SCI_DELWORDRIGHT,
        SCI_LINECUT,
        SCI_LINEDELETE,
        SCI_LINETRANSPOSE,
        SCI_LOWERCASE,
        SCI_UPPERCASE,
        SCI_LINESCROLLDOWN,
        SCI_LINESCROLLUP,
        SCI_DELETEBACKNOTLINE,
        SCI_HOMEDISPLAY,
        SCI_HOMEDISPLAYEXTEND,
        SCI_LINEENDDISPLAY,
        SCI_LINEENDDISPLAYEXTEND,
        SCI_MOVECARETINSIDEVIEW,
        SCI_LINELENGTH,
        SCI_BRACEHIGHLIGHT,
        SCI_BRACEBADLIGHT,
        SCI_BRACEMATCH,
        SCI_GETVIEWEOL,
        SCI_SETVIEWEOL,
        SCI_GETDOCPOINTER,
        SCI_SETDOCPOINTER,
        SCI_SETMODEVENTMASK,
        SCI_GETEDGECOLUMN,
        SCI_SETEDGECOLUMN,
        SCI_GETEDGEMODE,
        SCI_SETEDGEMODE,
        SCI_GETEDGECOLOUR,
        SCI_SETEDGECOLOUR,
        SCI_SEARCHANCHOR,
        SCI_SEARCHNEXT,
        SCI_SEARCHPREV,
        SCI_LINESONSCREEN,
        SCI_USEPOPUP,
        SCI_SELECTIONISRECTANGLE,
        SCI_SETZOOM,
        SCI_GETZOOM,
        SCI_CREATEDOCUMENT,
        SCI_ADDREFDOCUMENT,
        SCI_RELEASEDOCUMENT,
        SCI_GETMODEVENTMASK,
        SCI_SETFOCUS,
        SCI_GETFOCUS,
        SCI_SETSTATUS,
        SCI_GETSTATUS,
        SCI_SETMOUSEDOWNCAPTURES,
        SCI_GETMOUSEDOWNCAPTURES,
        SCI_SETCURSOR,
        SCI_GETCURSOR,
        SCI_SETCONTROLCHARSYMBOL,
        SCI_GETCONTROLCHARSYMBOL,
        SCI_WORDPARTLEFT,
        SCI_WORDPARTLEFTEXTEND,
        SCI_WORDPARTRIGHT,
        SCI_WORDPARTRIGHTEXTEND,
        SCI_SETVISIBLEPOLICY,
        SCI_DELLINELEFT,
        SCI_DELLINERIGHT,
        SCI_SETXOFFSET,
        SCI_GETXOFFSET,
        SCI_CHOOSECARETX,
        SCI_GRABFOCUS,
        SCI_SETXCARETPOLICY,
        SCI_SETYCARETPOLICY,
        SCI_LINEDUPLICATE,
        SCI_REGISTERIMAGE,
        SCI_SETPRINTWRAPMODE,
        SCI_GETPRINTWRAPMODE,
        SCI_CLEARREGISTEREDIMAGES,
        SCI_STYLESETHOTSPOT,
        SCI_SETHOTSPOTACTIVEFORE,
        SCI_SETHOTSPOTACTIVEBACK,
        SCI_SETHOTSPOTACTIVEUNDERLINE,
        SCI_PARADOWN,
        SCI_PARADOWNEXTEND,
        SCI_PARAUP,
        SCI_PARAUPEXTEND,
        SCI_POSITIONBEFORE,
        SCI_POSITIONAFTER,
        SCI_COPYRANGE,
        SCI_COPYTEXT,
        SCI_SETSELECTIONMODE,
        SCI_GETSELECTIONMODE,
        SCI_GETLINESELSTARTPOSITION,
        SCI_GETLINESELENDPOSITION,
        SCI_LINEDOWNRECTEXTEND,
        SCI_LINEUPRECTEXTEND,
        SCI_CHARLEFTRECTEXTEND,
        SCI_CHARRIGHTRECTEXTEND,
        SCI_HOMERECTEXTEND,
        SCI_VCHOMERECTEXTEND,
        SCI_LINEENDRECTEXTEND,
        SCI_PAGEUPRECTEXTEND,
        SCI_PAGEDOWNRECTEXTEND,
        SCI_STUTTEREDPAGEUP,
        SCI_STUTTEREDPAGEUPEXTEND,
        SCI_STUTTEREDPAGEDOWN,
        SCI_STUTTEREDPAGEDOWNEXTEND,
        SCI_WORDLEFTEND,
        SCI_WORDLEFTENDEXTEND,
        SCI_WORDRIGHTEND,
        SCI_WORDRIGHTENDEXTEND,
        SCI_SETWHITESPACECHARS,
        SCI_SETCHARSDEFAULT,
        SCI_AUTOCGETCURRENT,
        SCI_ALLOCATE,
        SCI_HOMEWRAP,
        SCI_HOMEWRAPEXTEND,
        SCI_LINEENDWRAP,
        SCI_LINEENDWRAPEXTEND,
        SCI_VCHOMEWRAP,
        SCI_VCHOMEWRAPEXTEND,
        SCI_LINECOPY,
        SCI_FINDCOLUMN,
        SCI_GETCARETSTICKY,
        SCI_SETCARETSTICKY,
        SCI_TOGGLECARETSTICKY,
        SCI_SETWRAPVISUALFLAGS,
        SCI_GETWRAPVISUALFLAGS,
        SCI_SETWRAPVISUALFLAGSLOCATION,
        SCI_GETWRAPVISUALFLAGSLOCATION,
        SCI_SETWRAPSTARTINDENT,
        SCI_GETWRAPSTARTINDENT,
        SCI_MARKERADDSET,
        SCI_SETPASTECONVERTENDINGS,
        SCI_GETPASTECONVERTENDINGS,
        SCI_SELECTIONDUPLICATE,
        SCI_SETCARETLINEBACKALPHA,
        SCI_GETCARETLINEBACKALPHA,
        SCI_SETWRAPINDENTMODE,
        SCI_GETWRAPINDENTMODE,
        SCI_MARKERSETALPHA,
        SCI_GETSELALPHA,
        SCI_SETSELALPHA,
        SCI_GETSELEOLFILLED,
        SCI_SETSELEOLFILLED,
        SCI_STYLEGETFORE,
        SCI_STYLEGETBACK,
        SCI_STYLEGETBOLD,
        SCI_STYLEGETITALIC,
        SCI_STYLEGETSIZE,
        SCI_STYLEGETFONT,
        SCI_STYLEGETEOLFILLED,
        SCI_STYLEGETUNDERLINE,
        SCI_STYLEGETCASE,
        SCI_STYLEGETCHARACTERSET,
        SCI_STYLEGETVISIBLE,
        SCI_STYLEGETCHANGEABLE,
        SCI_STYLEGETHOTSPOT,
        SCI_GETHOTSPOTACTIVEFORE,
        SCI_GETHOTSPOTACTIVEBACK,
        SCI_GETHOTSPOTACTIVEUNDERLINE,
        SCI_GETHOTSPOTSINGLELINE,
        SCI_BRACEHIGHLIGHTINDICATOR,
        SCI_BRACEBADLIGHTINDICATOR,
        SCI_SETINDICATORCURRENT,
        SCI_GETINDICATORCURRENT,
        SCI_SETINDICATORVALUE,
        SCI_GETINDICATORVALUE,
        SCI_INDICATORFILLRANGE,
        SCI_INDICATORCLEARRANGE,
        SCI_INDICATORALLONFOR,
        SCI_INDICATORVALUEAT,
        SCI_INDICATORSTART,
        SCI_INDICATOREND,
        SCI_INDICSETUNDER,
        SCI_INDICGETUNDER,
        SCI_SETCARETSTYLE,
        SCI_GETCARETSTYLE,
        SCI_SETPOSITIONCACHE,
        SCI_GETPOSITIONCACHE,
        SCI_SETSCROLLWIDTHTRACKING,
        SCI_GETSCROLLWIDTHTRACKING,
        SCI_DELWORDRIGHTEND,
        SCI_COPYALLOWLINE,
        SCI_GETCHARACTERPOINTER,
        SCI_SETKEYSUNICODE,
        SCI_GETKEYSUNICODE,
        SCI_INDICSETALPHA,
        SCI_INDICGETALPHA,
        SCI_SETEXTRAASCENT,
        SCI_GETEXTRAASCENT,
        SCI_SETEXTRADESCENT,
        SCI_GETEXTRADESCENT,
        SCI_MARKERSYMBOLDEFINED,
        SCI_MARGINSETTEXT,
        SCI_MARGINGETTEXT,
        SCI_MARGINSETSTYLE,
        SCI_MARGINGETSTYLE,
        SCI_MARGINSETSTYLES,
        SCI_MARGINGETSTYLES,
        SCI_MARGINTEXTCLEARALL,
        SCI_MARGINSETSTYLEOFFSET,
        SCI_MARGINGETSTYLEOFFSET,
        SCI_SETMARGINOPTIONS,
        SCI_ANNOTATIONSETTEXT,
        SCI_ANNOTATIONGETTEXT,
        SCI_ANNOTATIONSETSTYLE,
        SCI_ANNOTATIONGETSTYLE,
        SCI_ANNOTATIONSETSTYLES,
        SCI_ANNOTATIONGETSTYLES,
        SCI_ANNOTATIONGETLINES,
        SCI_ANNOTATIONCLEARALL,
        SCI_ANNOTATIONSETVISIBLE,
        SCI_ANNOTATIONGETVISIBLE,
        SCI_ANNOTATIONSETSTYLEOFFSET,
        SCI_ANNOTATIONGETSTYLEOFFSET,
        SCI_SETEMPTYSELECTION,
        SCI_GETMARGINOPTIONS,
        SCI_INDICSETOUTLINEALPHA,
        SCI_INDICGETOUTLINEALPHA,
        SCI_ADDUNDOACTION,
        SCI_CHARPOSITIONFROMPOINT,
        SCI_CHARPOSITIONFROMPOINTCLOSE,
        SCI_SETMULTIPLESELECTION,
        SCI_GETMULTIPLESELECTION,
        SCI_SETADDITIONALSELECTIONTYPING,
        SCI_GETADDITIONALSELECTIONTYPING,
        SCI_SETADDITIONALCARETSBLINK,
        SCI_GETADDITIONALCARETSBLINK,
        SCI_SCROLLRANGE,
        SCI_SETADDITIONALCARETSVISIBLE,
        SCI_GETADDITIONALCARETSVISIBLE,
        SCI_AUTOCGETCURRENTTEXT,
        SCI_GETSELECTIONS,
        SCI_CLEARSELECTIONS,
        SCI_SETSELECTION,
        SCI_ADDSELECTION,
        SCI_SETMAINSELECTION,
        SCI_GETMAINSELECTION,
        SCI_SETSELECTIONNCARET,
        SCI_GETSELECTIONNCARET,
        SCI_SETSELECTIONNANCHOR,
        SCI_GETSELECTIONNANCHOR,
        SCI_SETSELECTIONNCARETVIRTUALSPACE,
        SCI_GETSELECTIONNCARETVIRTUALSPACE,
        SCI_SETSELECTIONNANCHORVIRTUALSPACE,
        SCI_GETSELECTIONNANCHORVIRTUALSPACE,
        SCI_SETSELECTIONNSTART,
        SCI_GETSELECTIONNSTART,
        SCI_SETSELECTIONNEND,
        SCI_GETSELECTIONNEND,
        SCI_SETRECTANGULARSELECTIONCARET,
        SCI_GETRECTANGULARSELECTIONCARET,
        SCI_SETRECTANGULARSELECTIONANCHOR,
        SCI_GETRECTANGULARSELECTIONANCHOR,
        SCI_SETRECTANGULARSELECTIONCARETVIRTUALSPACE,
        SCI_GETRECTANGULARSELECTIONCARETVIRTUALSPACE,
        SCI_SETRECTANGULARSELECTIONANCHORVIRTUALSPACE,
        SCI_GETRECTANGULARSELECTIONANCHORVIRTUALSPACE,
        SCI_SETVIRTUALSPACEOPTIONS,
        SCI_GETVIRTUALSPACEOPTIONS,
        SCI_SETRECTANGULARSELECTIONMODIFIER,
        SCI_GETRECTANGULARSELECTIONMODIFIER,
        SCI_SETADDITIONALSELFORE,
        SCI_SETADDITIONALSELBACK,
        SCI_SETADDITIONALSELALPHA,
        SCI_GETADDITIONALSELALPHA,
        SCI_SETADDITIONALCARETFORE,
        SCI_GETADDITIONALCARETFORE,
        SCI_ROTATESELECTION,
        SCI_SWAPMAINANCHORCARET,
        SCI_SETFONTQUALITY,
        SCI_GETFONTQUALITY,
        SCI_SETFIRSTVISIBLELINE,
        SCI_SETMULTIPASTE,
        SCI_GETMULTIPASTE,
        SCI_GETTAG,
        SCI_CHANGELEXERSTATE,
        SCI_CONTRACTEDFOLDNEXT,
        SCI_VERTICALCENTRECARET,
        SCI_MOVESELECTEDLINESUP,
        SCI_MOVESELECTEDLINESDOWN,
        SCI_SETIDENTIFIER,
        SCI_GETIDENTIFIER,
        SCI_RGBAIMAGESETWIDTH,
        SCI_RGBAIMAGESETHEIGHT,
        SCI_MARKERDEFINERGBAIMAGE,
        SCI_REGISTERRGBAIMAGE,
        SCI_SCROLLTOSTART,
        SCI_SCROLLTOEND,
        SCI_STARTRECORD,
        SCI_STOPRECORD,
        SCI_SETLEXER,
        SCI_GETLEXER,
        SCI_COLOURISE,
        SCI_SETPROPERTY,
        SCI_SETKEYWORDS,
        SCI_SETLEXERLANGUAGE,
        SCI_LOADLEXERLIBRARY,
        SCI_GETPROPERTY,
        SCI_GETPROPERTYEXPANDED,
        SCI_GETPROPERTYINT,
        SCI_GETSTYLEBITSNEEDED,
        SCI_GETLEXERLANGUAGE,
        SCI_PRIVATELEXERCALL,
        SCI_PROPERTYNAMES,
        SCI_PROPERTYTYPE,
        SCI_DESCRIBEPROPERTY,
        SCI_DESCRIBEKEYWORDSETS,

        SCI_AUTOCSETCASEINSENSITIVEBEHAVIOUR,
        SCI_AUTOCGETCASEINSENSITIVEBEHAVIOUR,
        SCI_CALLTIPSETPOSITION,
        SCI_COUNTCHARACTERS,
        SCI_CREATELOADER,
        SCI_DELETERANGE,
        SCI_FINDINDICATORFLASH,
        SCI_FINDINDICATORHIDE,
        SCI_FINDINDICATORSHOW,
        SCI_GETALLLINESVISIBLE,
        SCI_GETGAPPOSITION,
        SCI_GETPUNCTUATIONCHARS,
        SCI_GETRANGEPOINTER,
        SCI_GETSELECTIONEMPTY,
        SCI_GETTECHNOLOGY,
        SCI_GETWHITESPACECHARS,
        SCI_GETWORDCHARS,
        SCI_RGBAIMAGESETSCALE,
        SCI_SETPUNCTUATIONCHARS,
        SCI_SETTECHNOLOGY,
        SCI_STYLESETSIZEFRACTIONAL,
        SCI_STYLEGETSIZEFRACTIONAL,
        SCI_STYLESETWEIGHT,             
        SCI_STYLEGETWEIGHT,

        SCI_FOLDLINE,
        SCI_FOLDCHILDREN,
        SCI_EXPANDCHILDREN,
        SCI_FOLDALL,
        SCI_SETAUTOMATICFOLD,
        SCI_GETAUTOMATICFOLD,
        SCI_AUTOCSETORDER,
        SCI_AUTOCGETORDER,
        SCI_POSITIONRELATIVE,
        SCI_RELEASEALLEXTENDEDSTYLES,
        SCI_ALLOCATEEXTENDEDSTYLES,
        SCI_SETMOUSESELECTIONRECTANGULARSWITCH,
        SCI_GETMOUSESELECTIONRECTANGULARSWITCH,
        SCI_GETCARETLINEVISIBLEALWAYS,
        SCI_SETCARETLINEVISIBLEALWAYS,
        SCI_SETREPRESENTATION,
        SCI_GETREPRESENTATION,
        SCI_CLEARREPRESENTATION,
    };

    enum {
        SC_ALPHA_TRANSPARENT,
        SC_ALPHA_OPAQUE,
        SC_ALPHA_NOALPHA,
    };

    enum {
        SC_CARETSTICKY_OFF,
        SC_CARETSTICKY_ON,
        SC_CARETSTICKY_WHITESPACE,
    };

    enum {
        SC_EFF_QUALITY_MASK,
        SC_EFF_QUALITY_DEFAULT,
        SC_EFF_QUALITY_NON_ANTIALIASED,
        SC_EFF_QUALITY_ANTIALIASED,
        SC_EFF_QUALITY_LCD_OPTIMIZED,
    };

    enum {
        SC_MARGINOPTION_NONE,
        SC_MARGINOPTION_SUBLINESELECT,
    };

    enum {
        SC_MULTIPASTE_ONCE,
        SC_MULTIPASTE_EACH,
    };

    enum {
        SC_TYPE_BOOLEAN,
        SC_TYPE_INTEGER,
        SC_TYPE_STRING,
    };

    enum {
        SC_UPDATE_CONTENT,
        SC_UPDATE_SELECTION,
        SC_UPDATE_V_SCROLL,
        SC_UPDATE_H_SCROLL,
    };

    enum {
        SC_WRAPVISUALFLAG_NONE,
        SC_WRAPVISUALFLAG_END,
        SC_WRAPVISUALFLAG_START,
        SC_WRAPVISUALFLAG_MARGIN,
    };

    enum {
        SC_WRAPVISUALFLAGLOC_DEFAULT,
        SC_WRAPVISUALFLAGLOC_END_BY_TEXT,
        SC_WRAPVISUALFLAGLOC_START_BY_TEXT
    };

    enum {
        SC_SEL_STREAM,
        SC_SEL_RECTANGLE,
        SC_SEL_LINES,
        SC_SEL_THIN,
    };

    enum {
        SCVS_NONE,
        SCVS_RECTANGULARSELECTION,
        SCVS_USERACCESSIBLE,
    };

    enum {
        SCWS_INVISIBLE,
        SCWS_VISIBLEALWAYS,
        SCWS_VISIBLEAFTERINDENT
    };

    enum {
        SC_EOL_CRLF,
        SC_EOL_CR,
        SC_EOL_LF
    };

    enum {
        SC_CP_DBCS,
        SC_CP_UTF8
    };

    enum {
        SC_MARK_CIRCLE,
        SC_MARK_ROUNDRECT,
        SC_MARK_ARROW,
        SC_MARK_SMALLRECT,
        SC_MARK_SHORTARROW,
        SC_MARK_EMPTY,
        SC_MARK_ARROWDOWN,
        SC_MARK_MINUS,
        SC_MARK_PLUS,
        SC_MARK_VLINE,
        SC_MARK_LCORNER,
        SC_MARK_TCORNER,
        SC_MARK_BOXPLUS,
        SC_MARK_BOXPLUSCONNECTED,
        SC_MARK_BOXMINUS,
        SC_MARK_BOXMINUSCONNECTED,
        SC_MARK_LCORNERCURVE,
        SC_MARK_TCORNERCURVE,
        SC_MARK_CIRCLEPLUS,
        SC_MARK_CIRCLEPLUSCONNECTED,
        SC_MARK_CIRCLEMINUS,
        SC_MARK_CIRCLEMINUSCONNECTED,
        SC_MARK_BACKGROUND,
        SC_MARK_DOTDOTDOT,
        SC_MARK_ARROWS,
        SC_MARK_PIXMAP,
        SC_MARK_FULLRECT,
        SC_MARK_LEFTRECT,
        SC_MARK_AVAILABLE,
        SC_MARK_UNDERLINE,
        SC_MARK_RGBAIMAGE,
        SC_MARK_CHARACTER
    };

    enum {
        SC_MARKNUM_FOLDEREND,
        SC_MARKNUM_FOLDEROPENMID,
        SC_MARKNUM_FOLDERMIDTAIL,
        SC_MARKNUM_FOLDERTAIL,
        SC_MARKNUM_FOLDERSUB,
        SC_MARKNUM_FOLDER,
        SC_MARKNUM_FOLDEROPEN,
        SC_MASK_FOLDERS
    };

    enum {
        SC_MARGIN_SYMBOL,
        SC_MARGIN_NUMBER,
        SC_MARGIN_BACK,
        SC_MARGIN_FORE,
        SC_MARGIN_TEXT,
        SC_MARGIN_RTEXT,
    };

    enum {
        STYLE_DEFAULT,
        STYLE_LINENUMBER,
        STYLE_BRACELIGHT,
        STYLE_BRACEBAD,
        STYLE_CONTROLCHAR,
        STYLE_INDENTGUIDE,
        STYLE_CALLTIP,
        STYLE_LASTPREDEFINED,
        STYLE_MAX
    };

    enum {
        SC_CHARSET_ANSI,
        SC_CHARSET_DEFAULT,
        SC_CHARSET_BALTIC,
        SC_CHARSET_CHINESEBIG5,
        SC_CHARSET_EASTEUROPE,
        SC_CHARSET_GB2312,
        SC_CHARSET_GREEK,
        SC_CHARSET_HANGUL,
        SC_CHARSET_MAC,
        SC_CHARSET_OEM,
        SC_CHARSET_RUSSIAN,
        SC_CHARSET_SHIFTJIS,
        SC_CHARSET_SYMBOL,
        SC_CHARSET_TURKISH,
        SC_CHARSET_JOHAB,
        SC_CHARSET_HEBREW,
        SC_CHARSET_ARABIC,
        SC_CHARSET_VIETNAMESE,
        SC_CHARSET_THAI,
        SC_CHARSET_8859_15,
    };

    enum {
        SC_CASE_MIXED,
        SC_CASE_UPPER,
        SC_CASE_LOWER
    };

    enum
    {
        SC_IV_NONE,
        SC_IV_REAL,
        SC_IV_LOOKFORWARD,
        SC_IV_LOOKBOTH
    };

    enum {
        INDIC_PLAIN,
        INDIC_SQUIGGLE,
        INDIC_TT,
        INDIC_DIAGONAL,
        INDIC_STRIKE,
        INDIC_HIDDEN,
        INDIC_BOX,
        INDIC_ROUNDBOX,
        INDIC_STRAIGHTBOX,
        INDIC_DASH,
        INDIC_DOTS,
        INDIC_SQUIGGLELOW,
        INDIC_DOTBOX,
        INDIC_SQUIGGLEPIXMAP,
        INDIC_COMPOSITIONTHICK,

        INDIC_CONTAINER,
        INDIC_MAX,
        INDIC0_MASK,
        INDIC1_MASK,
        INDIC2_MASK,
        INDICS_MASK
    };

    enum {
        SC_PRINT_NORMAL,
        SC_PRINT_INVERTLIGHT,
        SC_PRINT_BLACKONWHITE,
        SC_PRINT_COLOURONWHITE,
        SC_PRINT_COLOURONWHITEDEFAULTBG
    };

    enum {
        SCFIND_WHOLEWORD,
        SCFIND_MATCHCASE,
        SCFIND_WORDSTART,
        SCFIND_REGEXP,
        SCFIND_POSIX
    };

    enum {
        SC_FOLDLEVELBASE,
        SC_FOLDLEVELWHITEFLAG,
        SC_FOLDLEVELHEADERFLAG,
        SC_FOLDLEVELNUMBERMASK
    };

    enum
    {
        SC_FOLDFLAG_LINEBEFORE_EXPANDED,
        SC_FOLDFLAG_LINEBEFORE_CONTRACTED,
        SC_FOLDFLAG_LINEAFTER_EXPANDED,
        SC_FOLDFLAG_LINEAFTER_CONTRACTED,
        SC_FOLDFLAG_LEVELNUMBERS
    };

    enum {
        SC_TIME_FOREVER
    };

    enum {
        SC_WRAP_NONE,
        SC_WRAP_WORD,
        SC_WRAP_CHAR,
    };

    enum {
        SC_WRAPINDENT_FIXED,
        SC_WRAPINDENT_SAME,
        SC_WRAPINDENT_INDENT,
    };

    enum {
        SC_CACHE_NONE,
        SC_CACHE_CARET,
        SC_CACHE_PAGE,
        SC_CACHE_DOCUMENT
    };

    enum {
        ANNOTATION_HIDDEN,
        ANNOTATION_STANDARD,
        ANNOTATION_BOXED,
    };

    enum {
        EDGE_NONE,
        EDGE_LINE,
        EDGE_BACKGROUND
    };

    enum {
        SC_CURSORNORMAL,
        SC_CURSORARROW,
        SC_CURSORWAIT,
        SC_CURSORREVERSEARROW
    };

    enum {
        UNDO_MAY_COALESCE,
    };

    enum {
        VISIBLE_SLOP,
        VISIBLE_STRICT
    };

    enum {
        CARET_SLOP,
        CARET_STRICT,
        CARET_JUMPS,
        CARET_EVEN
    };

    enum
    {
        CARETSTYLE_INVISIBLE,
        CARETSTYLE_LINE,
        CARETSTYLE_BLOCK
    };

    enum {
        SC_MOD_INSERTTEXT,
        SC_MOD_DELETETEXT,
        SC_MOD_CHANGESTYLE,
        SC_MOD_CHANGEFOLD,
        SC_PERFORMED_USER,
        SC_PERFORMED_UNDO,
        SC_PERFORMED_REDO,
        SC_MULTISTEPUNDOREDO,
        SC_LASTSTEPINUNDOREDO,
        SC_MOD_CHANGEMARKER,
        SC_MOD_BEFOREINSERT,
        SC_MOD_BEFOREDELETE,
        SC_MULTILINEUNDOREDO,
        SC_STARTACTION,
        SC_MOD_CHANGEINDICATOR,
        SC_MOD_CHANGELINESTATE,
        SC_MOD_CHANGEMARGIN,
        SC_MOD_CHANGEANNOTATION,
        SC_MOD_CONTAINER,
        SC_MOD_LEXERSTATE,
        SC_MODEVENTMASKALL
    };

    enum {
        SCK_DOWN,
        SCK_UP,
        SCK_LEFT,
        SCK_RIGHT,
        SCK_HOME,
        SCK_END,
        SCK_PRIOR,
        SCK_NEXT,
        SCK_DELETE,
        SCK_INSERT,
        SCK_ESCAPE,
        SCK_BACK,
        SCK_TAB,
        SCK_RETURN,
        SCK_ADD,
        SCK_SUBTRACT,
        SCK_DIVIDE,
        SCK_WIN,
        SCK_RWIN,
        SCK_MENU
    };

    enum {
        SCMOD_NORM,
        SCMOD_SHIFT,
        SCMOD_CTRL,
        SCMOD_ALT,
        SCMOD_SUPER,
        SCMOD_META,
    };

    enum {
        SCLEX_CONTAINER,
        SCLEX_NULL,
        SCLEX_PYTHON,
        SCLEX_CPP,
        SCLEX_HTML,
        SCLEX_XML,
        SCLEX_PERL,
        SCLEX_SQL,
        SCLEX_VB,
        SCLEX_PROPERTIES,
        SCLEX_ERRORLIST,
        SCLEX_MAKEFILE,
        SCLEX_BATCH,
        SCLEX_LATEX,
        SCLEX_LUA,
        SCLEX_DIFF,
        SCLEX_CONF,
        SCLEX_PASCAL,
        SCLEX_AVE,
        SCLEX_ADA,
        SCLEX_LISP,
        SCLEX_RUBY,
        SCLEX_EIFFEL,
        SCLEX_EIFFELKW,
        SCLEX_TCL,
        SCLEX_NNCRONTAB,
        SCLEX_BULLANT,
        SCLEX_VBSCRIPT,
        SCLEX_ASP,
        SCLEX_PHP,
        SCLEX_BAAN,
        SCLEX_MATLAB,
        SCLEX_SCRIPTOL,
        SCLEX_ASM,
        SCLEX_CPPNOCASE,
        SCLEX_FORTRAN,
        SCLEX_F77,
        SCLEX_CSS,
        SCLEX_POV,
        SCLEX_LOUT,
        SCLEX_ESCRIPT,
        SCLEX_PS,
        SCLEX_NSIS,
        SCLEX_MMIXAL,
        SCLEX_CLW,
        SCLEX_CLWNOCASE,
        SCLEX_LOT,
        SCLEX_YAML,
        SCLEX_TEX,
        SCLEX_METAPOST,
        SCLEX_POWERBASIC,
        SCLEX_FORTH,
        SCLEX_ERLANG,
        SCLEX_OCTAVE,
        SCLEX_MSSQL,
        SCLEX_VERILOG,
        SCLEX_KIX,
        SCLEX_GUI4CLI,
        SCLEX_SPECMAN,
        SCLEX_AU3,
        SCLEX_APDL,
        SCLEX_BASH,
        SCLEX_ASN1,
        SCLEX_VHDL,
        SCLEX_CAML,
        SCLEX_BLITZBASIC,
        SCLEX_PUREBASIC,
        SCLEX_HASKELL,
        SCLEX_PHPSCRIPT,
        SCLEX_TADS3,
        SCLEX_REBOL,
        SCLEX_SMALLTALK,
        SCLEX_FLAGSHIP,
        SCLEX_CSOUND,
        SCLEX_FREEBASIC,
        SCLEX_INNOSETUP,
        SCLEX_OPAL,
        SCLEX_SPICE,
        SCLEX_D,
        SCLEX_CMAKE,
        SCLEX_GAP,
        SCLEX_PLM,
        SCLEX_PROGRESS,
        SCLEX_ABAQUS,
        SCLEX_ASYMPTOTE,
        SCLEX_R,
        SCLEX_MAGIK,
        SCLEX_POWERSHELL,
        SCLEX_MYSQL,
        SCLEX_PO,
        SCLEX_TAL,
        SCLEX_COBOL,
        SCLEX_TACL,
        SCLEX_SORCUS,
        SCLEX_POWERPRO,
        SCLEX_NIMROD,
        SCLEX_SML,
        SCLEX_MARKDOWN,
        SCLEX_TXT2TAGS,
        SCLEX_A68K,
        SCLEX_MODULA,
        SCLEX_COFFEESCRIPT,
        SCLEX_TCMD,
        SCLEX_AVS,
        SCLEX_ECL,
        SCLEX_OSCRIPT,
        SCLEX_VISUALPROLOG,
        SCLEX_LITERATEHASKELL,
        SCLEX_STTXT,
        SCLEX_KVIRC,
        SCLEX_RUST,
    };

    enum {
        SC_WEIGHT_NORMAL,
        SC_WEIGHT_SEMIBOLD,
        SC_WEIGHT_BOLD,
    };

    enum {
        SC_TECHNOLOGY_DEFAULT,
        SC_TECHNOLOGY_DIRECTWRITE,
    };

    enum {
        SC_CASEINSENSITIVEBEHAVIOUR_RESPECTCASE,
        SC_CASEINSENSITIVEBEHAVIOUR_IGNORECASE,
    };

    enum {
        SC_FONT_SIZE_MULTIPLIER,
    };

    enum
    {
        SC_FOLDACTION_CONTRACT,
        SC_FOLDACTION_EXPAND,
        SC_FOLDACTION_TOGGLE,
    };

    enum
    {
        SC_AUTOMATICFOLD_SHOW,
        SC_AUTOMATICFOLD_CLICK,
        SC_AUTOMATICFOLD_CHANGE,
    };

    enum
    {
        SC_ORDER_PRESORTED,
        SC_ORDER_PERFORMSORT,
        SC_ORDER_CUSTOM,
    };
    
    explicit QsciScintillaBase(QWidget *parent /TransferThis/ = 0);
    virtual ~QsciScintillaBase();

%ConvertToSubClassCode
static struct class_graph {
    const char *name;
    sipTypeDef **type;
    int yes, no;
} graph[] = {
    {sipName_QsciScintillaBase, &sipType_QsciScintillaBase, 4, 1},
    {sipName_QsciMacro, &sipType_QsciMacro, -1, 2},
    {sipName_QsciLexer, &sipType_QsciLexer, 5, 3},
    {sipName_QsciAbstractAPIs, &sipType_QsciAbstractAPIs, 42, -1},
    {sipName_QsciScintilla, &sipType_QsciScintilla, -1, -1},
    {sipName_QsciLexerPO, &sipType_QsciLexerPO, -1, 6},
    {sipName_QsciLexerBash, &sipType_QsciLexerBash, -1, 7},
    {sipName_QsciLexerPython, &sipType_QsciLexerPython, -1, 8},
    {sipName_QsciLexerSQL, &sipType_QsciLexerSQL, -1, 9},
    {sipName_QsciLexerPascal, &sipType_QsciLexerPascal, -1, 10},
    {sipName_QsciLexerDiff, &sipType_QsciLexerDiff, -1, 11},
    {sipName_QsciLexerCustom, &sipType_QsciLexerCustom, -1, 12},
    {sipName_QsciLexerPOV, &sipType_QsciLexerPOV, -1, 13},
    {sipName_QsciLexerLua, &sipType_QsciLexerLua, -1, 14},
    {sipName_QsciLexerSpice, &sipType_QsciLexerSpice, -1, 15},
    {sipName_QsciLexerMatlab, &sipType_QsciLexerMatlab, 35, 16},
    {sipName_QsciLexerCSS, &sipType_QsciLexerCSS, -1, 17},
    {sipName_QsciLexerCMake, &sipType_QsciLexerCMake, -1, 18},
    {sipName_QsciLexerVHDL, &sipType_QsciLexerVHDL, -1, 19},
    {sipName_QsciLexerVerilog, &sipType_QsciLexerVerilog, -1, 20},
    {sipName_QsciLexerHTML, &sipType_QsciLexerHTML, 36, 21},
    {sipName_QsciLexerMakefile, &sipType_QsciLexerMakefile, -1, 22},
    {sipName_QsciLexerPostScript, &sipType_QsciLexerPostScript, -1, 23},
    {sipName_QsciLexerTCL, &sipType_QsciLexerTCL, -1, 24},
    {sipName_QsciLexerAVS, &sipType_QsciLexerAVS, -1, 25},
    {sipName_QsciLexerPerl, &sipType_QsciLexerPerl, -1, 26},
    {sipName_QsciLexerCoffeeScript, &sipType_QsciLexerCoffeeScript, -1, 27},
    {sipName_QsciLexerFortran77, &sipType_QsciLexerFortran77, 37, 28},
    {sipName_QsciLexerYAML, &sipType_QsciLexerYAML, -1, 29},
    {sipName_QsciLexerRuby, &sipType_QsciLexerRuby, -1, 30},
    {sipName_QsciLexerProperties, &sipType_QsciLexerProperties, -1, 31},
    {sipName_QsciLexerTeX, &sipType_QsciLexerTeX, -1, 32},
    {sipName_QsciLexerD, &sipType_QsciLexerD, -1, 33},
    {sipName_QsciLexerCPP, &sipType_QsciLexerCPP, 38, 34},
    {sipName_QsciLexerBatch, &sipType_QsciLexerBatch, -1, -1},
    {sipName_QsciLexerOctave, &sipType_QsciLexerOctave, -1, -1},
    {sipName_QsciLexerXML, &sipType_QsciLexerXML, -1, -1},
    {sipName_QsciLexerFortran, &sipType_QsciLexerFortran, -1, -1},
    {sipName_QsciLexerJava, &sipType_QsciLexerJava, -1, 39},
    {sipName_QsciLexerCSharp, &sipType_QsciLexerCSharp, -1, 40},
    {sipName_QsciLexerIDL, &sipType_QsciLexerIDL, -1, 41},
    {sipName_QsciLexerJavaScript, &sipType_QsciLexerJavaScript, -1, -1},
    {sipName_QsciAPIs, &sipType_QsciAPIs, -1, -1},
};

int i = 0;

sipType = NULL;

do
{
    struct class_graph *cg = &graph[i];

    if (cg->name != NULL && sipCpp->inherits(cg->name))
    {
        sipType = *cg->type;
        i = cg->yes;
    }
    else
        i = cg->no;
}
while (i >= 0);
%End

    static QsciScintillaBase *pool();

    long SendScintilla(unsigned int msg, unsigned long wParam = 0,
            long lParam = 0) const;
    long SendScintilla(unsigned int msg, unsigned long wParam,
            void *lParam) const;
    long SendScintilla(unsigned int msg, unsigned long wParam,
            const char *lParam /DocType="Py_v3:bytes;str", Encoding="None"/) const;
    long SendScintilla(unsigned int msg,
            const char *lParam /DocType="Py_v3:bytes;str", Encoding="None"/); const
    long SendScintilla(unsigned int msg,
            const char *wParam /DocType="Py_v3:bytes;str", Encoding="None"/,
            const char *lParam /DocType="Py_v3:bytes;str", Encoding="None"/) const;
    long SendScintilla(unsigned int msg, long wParam) const;
    //long SendScintilla(unsigned int msg, int wParam) const;
    long SendScintilla(unsigned int msg, long cpMin, long cpMax,
            char *lpstrText /DocType="Py_v3:bytes;str", Encoding="None"/) const;
    long SendScintilla(unsigned int msg, unsigned long wParam,
            const QColor &col) const;
    long SendScintilla(unsigned int msg, const QColor &col) const;
    long SendScintilla(unsigned int msg, unsigned long wParam, QPainter *hdc,
            const QRect &rc, long cpMin, long cpMax) const;
    long SendScintilla(unsigned int msg, unsigned long wParam,
            const QPixmap &lParam) const;
    long SendScintilla(unsigned int msg, unsigned long wParam,
            const QImage &lParam) const;

    void *SendScintillaPtrResult(unsigned int msg) const;


signals:
    void QSCN_SELCHANGED(bool yes);

    void SCEN_CHANGE();

    void SCN_AUTOCCANCELLED();
    void SCN_AUTOCCHARDELETED();
    void SCN_AUTOCSELECTION(const char *selection, int position);
    void SCN_CALLTIPCLICK(int direction);
    void SCN_CHARADDED(int charadded);
    void SCN_DOUBLECLICK(int position, int line, int modifiers);
    void SCN_DWELLEND(int, int, int);
    void SCN_DWELLSTART(int, int, int);
    void SCN_FOCUSIN();
    void SCN_FOCUSOUT();
    void SCN_HOTSPOTCLICK(int position, int modifiers);
    void SCN_HOTSPOTDOUBLECLICK(int position, int modifiers);
    void SCN_HOTSPOTRELEASECLICK(int position, int modifiers);
    void SCN_INDICATORCLICK(int position, int modifiers);
    void SCN_INDICATORRELEASE(int position, int modifiers);
    void SCN_MACRORECORD(unsigned int, unsigned long, void *);
    void SCN_MARGINCLICK(int position, int modifiers, int margin);
    void SCN_MODIFIED(int, int, const char *, int, int, int, int, int, int, int);
    void SCN_MODIFYATTEMPTRO();
    void SCN_NEEDSHOWN(int, int);
    void SCN_PAINTED();
    void SCN_SAVEPOINTLEFT();
    void SCN_SAVEPOINTREACHED();
    void SCN_STYLENEEDED(int position);
    void SCN_UPDATEUI(int updated);
    void SCN_USERLISTSELECTION(const char *, int);
    void SCN_ZOOM();

protected:
    virtual bool canInsertFromMimeData(const QMimeData *source) const;
    virtual QByteArray fromMimeData(const QMimeData *source, bool &rectangular) const;
    virtual QMimeData *toMimeData(const QByteArray &text, bool rectangular) const;

    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dragLeaveEvent(QDragLeaveEvent *e);
    virtual void dragMoveEvent(QDragMoveEvent *e);
    virtual void dropEvent(QDropEvent *e);
    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);
    virtual bool focusNextPrevChild(bool next);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void inputMethodEvent(QInputMethodEvent *e);
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void scrollContentsBy(int dx, int dy);

private:
    QsciScintillaBase(const QsciScintillaBase &);
};
