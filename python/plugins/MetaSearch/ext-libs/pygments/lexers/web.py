# -*- coding: utf-8 -*-
"""
    pygments.lexers.web
    ~~~~~~~~~~~~~~~~~~~

    Lexers for web-related languages and markup.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import re
import copy

from pygments.lexer import RegexLexer, ExtendedRegexLexer, bygroups, using, \
     include, this
from pygments.token import Text, Comment, Operator, Keyword, Name, String, \
     Number, Other, Punctuation, Literal
from pygments.util import get_bool_opt, get_list_opt, looks_like_xml, \
                          html_doctype_matches, unirange
from pygments.lexers.agile import RubyLexer
from pygments.lexers.compiled import ScalaLexer


__all__ = ['HtmlLexer', 'XmlLexer', 'JavascriptLexer', 'JsonLexer', 'CssLexer',
           'PhpLexer', 'ActionScriptLexer', 'XsltLexer', 'ActionScript3Lexer',
           'MxmlLexer', 'HaxeLexer', 'HamlLexer', 'SassLexer', 'ScssLexer',
           'ObjectiveJLexer', 'CoffeeScriptLexer', 'LiveScriptLexer',
           'DuelLexer', 'ScamlLexer', 'JadeLexer', 'XQueryLexer',
           'DtdLexer', 'DartLexer', 'LassoLexer', 'QmlLexer', 'TypeScriptLexer']


class JavascriptLexer(RegexLexer):
    """
    For JavaScript source code.
    """

    name = 'JavaScript'
    aliases = ['js', 'javascript']
    filenames = ['*.js', ]
    mimetypes = ['application/javascript', 'application/x-javascript',
                 'text/x-javascript', 'text/javascript', ]

    flags = re.DOTALL
    tokens = {
        'commentsandwhitespace': [
            (r'\s+', Text),
            (r'<!--', Comment),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline)
        ],
        'slashstartsregex': [
            include('commentsandwhitespace'),
            (r'/(\\.|[^[/\\\n]|\[(\\.|[^\]\\\n])*])+/'
             r'([gim]+\b|\B)', String.Regex, '#pop'),
            (r'(?=/)', Text, ('#pop', 'badregex')),
            (r'', Text, '#pop')
        ],
        'badregex': [
            (r'\n', Text, '#pop')
        ],
        'root': [
            (r'^(?=\s|/|<!--)', Text, 'slashstartsregex'),
            include('commentsandwhitespace'),
            (r'\+\+|--|~|&&|\?|:|\|\||\\(?=\n)|'
             r'(<<|>>>?|==?|!=?|[-<>+*%&\|\^/])=?', Operator, 'slashstartsregex'),
            (r'[{(\[;,]', Punctuation, 'slashstartsregex'),
            (r'[})\].]', Punctuation),
            (r'(for|in|while|do|break|return|continue|switch|case|default|if|else|'
             r'throw|try|catch|finally|new|delete|typeof|instanceof|void|'
             r'this)\b', Keyword, 'slashstartsregex'),
            (r'(var|let|with|function)\b', Keyword.Declaration, 'slashstartsregex'),
            (r'(abstract|boolean|byte|char|class|const|debugger|double|enum|export|'
             r'extends|final|float|goto|implements|import|int|interface|long|native|'
             r'package|private|protected|public|short|static|super|synchronized|throws|'
             r'transient|volatile)\b', Keyword.Reserved),
            (r'(true|false|null|NaN|Infinity|undefined)\b', Keyword.Constant),
            (r'(Array|Boolean|Date|Error|Function|Math|netscape|'
             r'Number|Object|Packages|RegExp|String|sun|decodeURI|'
             r'decodeURIComponent|encodeURI|encodeURIComponent|'
             r'Error|eval|isFinite|isNaN|parseFloat|parseInt|document|this|'
             r'window)\b', Name.Builtin),
            (r'[$a-zA-Z_][a-zA-Z0-9_]*', Name.Other),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+', Number.Integer),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
        ]
    }


class JsonLexer(RegexLexer):
    """
    For JSON data structures.

    *New in Pygments 1.5.*
    """

    name = 'JSON'
    aliases = ['json']
    filenames = ['*.json']
    mimetypes = [ 'application/json', ]

    # integer part of a number
    int_part = r'-?(0|[1-9]\d*)'

    # fractional part of a number
    frac_part = r'\.\d+'

    # exponential part of a number
    exp_part = r'[eE](\+|-)?\d+'


    flags = re.DOTALL
    tokens = {
        'whitespace': [
            (r'\s+', Text),
        ],

        # represents a simple terminal value
        'simplevalue': [
            (r'(true|false|null)\b', Keyword.Constant),
            (('%(int_part)s(%(frac_part)s%(exp_part)s|'
              '%(exp_part)s|%(frac_part)s)') % vars(),
             Number.Float),
            (int_part, Number.Integer),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
        ],


        # the right hand side of an object, after the attribute name
        'objectattribute': [
            include('value'),
            (r':', Punctuation),
            # comma terminates the attribute but expects more
            (r',', Punctuation, '#pop'),
            # a closing bracket terminates the entire object, so pop twice
            (r'}', Punctuation, ('#pop', '#pop')),
        ],

        # a json object - { attr, attr, ... }
        'objectvalue': [
            include('whitespace'),
            (r'"(\\\\|\\"|[^"])*"', Name.Tag, 'objectattribute'),
            (r'}', Punctuation, '#pop'),
        ],

        # json array - [ value, value, ... }
        'arrayvalue': [
            include('whitespace'),
            include('value'),
            (r',', Punctuation),
            (r']', Punctuation, '#pop'),
        ],

        # a json value - either a simple value or a complex value (object or array)
        'value': [
            include('whitespace'),
            include('simplevalue'),
            (r'{', Punctuation, 'objectvalue'),
            (r'\[', Punctuation, 'arrayvalue'),
        ],


        # the root of a json document whould be a value
        'root': [
            include('value'),
        ],

    }

JSONLexer = JsonLexer  # for backwards compatibility with Pygments 1.5


class ActionScriptLexer(RegexLexer):
    """
    For ActionScript source code.

    *New in Pygments 0.9.*
    """

    name = 'ActionScript'
    aliases = ['as', 'actionscript']
    filenames = ['*.as']
    mimetypes = ['application/x-actionscript3', 'text/x-actionscript3',
                 'text/actionscript3']

    flags = re.DOTALL
    tokens = {
        'root': [
            (r'\s+', Text),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'/(\\\\|\\/|[^/\n])*/[gim]*', String.Regex),
            (r'[~\^\*!%&<>\|+=:;,/?\\-]+', Operator),
            (r'[{}\[\]();.]+', Punctuation),
            (r'(case|default|for|each|in|while|do|break|return|continue|if|else|'
             r'throw|try|catch|var|with|new|typeof|arguments|instanceof|this|'
             r'switch)\b', Keyword),
            (r'(class|public|final|internal|native|override|private|protected|'
             r'static|import|extends|implements|interface|intrinsic|return|super|'
             r'dynamic|function|const|get|namespace|package|set)\b',
             Keyword.Declaration),
            (r'(true|false|null|NaN|Infinity|-Infinity|undefined|Void)\b',
             Keyword.Constant),
            (r'(Accessibility|AccessibilityProperties|ActionScriptVersion|'
             r'ActivityEvent|AntiAliasType|ApplicationDomain|AsBroadcaster|Array|'
             r'AsyncErrorEvent|AVM1Movie|BevelFilter|Bitmap|BitmapData|'
             r'BitmapDataChannel|BitmapFilter|BitmapFilterQuality|BitmapFilterType|'
             r'BlendMode|BlurFilter|Boolean|ByteArray|Camera|Capabilities|CapsStyle|'
             r'Class|Color|ColorMatrixFilter|ColorTransform|ContextMenu|'
             r'ContextMenuBuiltInItems|ContextMenuEvent|ContextMenuItem|'
             r'ConvultionFilter|CSMSettings|DataEvent|Date|DefinitionError|'
             r'DeleteObjectSample|Dictionary|DisplacmentMapFilter|DisplayObject|'
             r'DisplacmentMapFilterMode|DisplayObjectContainer|DropShadowFilter|'
             r'Endian|EOFError|Error|ErrorEvent|EvalError|Event|EventDispatcher|'
             r'EventPhase|ExternalInterface|FileFilter|FileReference|'
             r'FileReferenceList|FocusDirection|FocusEvent|Font|FontStyle|FontType|'
             r'FrameLabel|FullScreenEvent|Function|GlowFilter|GradientBevelFilter|'
             r'GradientGlowFilter|GradientType|Graphics|GridFitType|HTTPStatusEvent|'
             r'IBitmapDrawable|ID3Info|IDataInput|IDataOutput|IDynamicPropertyOutput'
             r'IDynamicPropertyWriter|IEventDispatcher|IExternalizable|'
             r'IllegalOperationError|IME|IMEConversionMode|IMEEvent|int|'
             r'InteractiveObject|InterpolationMethod|InvalidSWFError|InvokeEvent|'
             r'IOError|IOErrorEvent|JointStyle|Key|Keyboard|KeyboardEvent|KeyLocation|'
             r'LineScaleMode|Loader|LoaderContext|LoaderInfo|LoadVars|LocalConnection|'
             r'Locale|Math|Matrix|MemoryError|Microphone|MorphShape|Mouse|MouseEvent|'
             r'MovieClip|MovieClipLoader|Namespace|NetConnection|NetStatusEvent|'
             r'NetStream|NewObjectSample|Number|Object|ObjectEncoding|PixelSnapping|'
             r'Point|PrintJob|PrintJobOptions|PrintJobOrientation|ProgressEvent|Proxy|'
             r'QName|RangeError|Rectangle|ReferenceError|RegExp|Responder|Sample|Scene|'
             r'ScriptTimeoutError|Security|SecurityDomain|SecurityError|'
             r'SecurityErrorEvent|SecurityPanel|Selection|Shape|SharedObject|'
             r'SharedObjectFlushStatus|SimpleButton|Socket|Sound|SoundChannel|'
             r'SoundLoaderContext|SoundMixer|SoundTransform|SpreadMethod|Sprite|'
             r'StackFrame|StackOverflowError|Stage|StageAlign|StageDisplayState|'
             r'StageQuality|StageScaleMode|StaticText|StatusEvent|String|StyleSheet|'
             r'SWFVersion|SyncEvent|SyntaxError|System|TextColorType|TextField|'
             r'TextFieldAutoSize|TextFieldType|TextFormat|TextFormatAlign|'
             r'TextLineMetrics|TextRenderer|TextSnapshot|Timer|TimerEvent|Transform|'
             r'TypeError|uint|URIError|URLLoader|URLLoaderDataFormat|URLRequest|'
             r'URLRequestHeader|URLRequestMethod|URLStream|URLVariabeles|VerifyError|'
             r'Video|XML|XMLDocument|XMLList|XMLNode|XMLNodeType|XMLSocket|XMLUI)\b',
             Name.Builtin),
            (r'(decodeURI|decodeURIComponent|encodeURI|escape|eval|isFinite|isNaN|'
             r'isXMLName|clearInterval|fscommand|getTimer|getURL|getVersion|'
             r'isFinite|parseFloat|parseInt|setInterval|trace|updateAfterEvent|'
             r'unescape)\b',Name.Function),
            (r'[$a-zA-Z_][a-zA-Z0-9_]*', Name.Other),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-f]+', Number.Hex),
            (r'[0-9]+', Number.Integer),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
        ]
    }


class ActionScript3Lexer(RegexLexer):
    """
    For ActionScript 3 source code.

    *New in Pygments 0.11.*
    """

    name = 'ActionScript 3'
    aliases = ['as3', 'actionscript3']
    filenames = ['*.as']
    mimetypes = ['application/x-actionscript', 'text/x-actionscript',
                 'text/actionscript']

    identifier = r'[$a-zA-Z_][a-zA-Z0-9_]*'
    typeidentifier = identifier + '(?:\.<\w+>)?'

    flags = re.DOTALL | re.MULTILINE
    tokens = {
        'root': [
            (r'\s+', Text),
            (r'(function\s+)(' + identifier + r')(\s*)(\()',
             bygroups(Keyword.Declaration, Name.Function, Text, Operator),
             'funcparams'),
            (r'(var|const)(\s+)(' + identifier + r')(\s*)(:)(\s*)(' +
             typeidentifier + r')',
             bygroups(Keyword.Declaration, Text, Name, Text, Punctuation, Text,
                      Keyword.Type)),
            (r'(import|package)(\s+)((?:' + identifier + r'|\.)+)(\s*)',
             bygroups(Keyword, Text, Name.Namespace, Text)),
            (r'(new)(\s+)(' + typeidentifier + r')(\s*)(\()',
             bygroups(Keyword, Text, Keyword.Type, Text, Operator)),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'/(\\\\|\\/|[^\n])*/[gisx]*', String.Regex),
            (r'(\.)(' + identifier + r')', bygroups(Operator, Name.Attribute)),
            (r'(case|default|for|each|in|while|do|break|return|continue|if|else|'
             r'throw|try|catch|with|new|typeof|arguments|instanceof|this|'
             r'switch|import|include|as|is)\b',
             Keyword),
            (r'(class|public|final|internal|native|override|private|protected|'
             r'static|import|extends|implements|interface|intrinsic|return|super|'
             r'dynamic|function|const|get|namespace|package|set)\b',
             Keyword.Declaration),
            (r'(true|false|null|NaN|Infinity|-Infinity|undefined|void)\b',
             Keyword.Constant),
            (r'(decodeURI|decodeURIComponent|encodeURI|escape|eval|isFinite|isNaN|'
             r'isXMLName|clearInterval|fscommand|getTimer|getURL|getVersion|'
             r'isFinite|parseFloat|parseInt|setInterval|trace|updateAfterEvent|'
             r'unescape)\b', Name.Function),
            (identifier, Name),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-f]+', Number.Hex),
            (r'[0-9]+', Number.Integer),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
            (r'[~\^\*!%&<>\|+=:;,/?\\{}\[\]().-]+', Operator),
        ],
        'funcparams': [
            (r'\s+', Text),
            (r'(\s*)(\.\.\.)?(' + identifier + r')(\s*)(:)(\s*)(' +
             typeidentifier + r'|\*)(\s*)',
             bygroups(Text, Punctuation, Name, Text, Operator, Text,
                      Keyword.Type, Text), 'defval'),
            (r'\)', Operator, 'type')
        ],
        'type': [
            (r'(\s*)(:)(\s*)(' + typeidentifier + r'|\*)',
             bygroups(Text, Operator, Text, Keyword.Type), '#pop:2'),
            (r'\s*', Text, '#pop:2')
        ],
        'defval': [
            (r'(=)(\s*)([^(),]+)(\s*)(,?)',
             bygroups(Operator, Text, using(this), Text, Operator), '#pop'),
            (r',?', Operator, '#pop')
        ]
    }

    def analyse_text(text):
        if re.match(r'\w+\s*:\s*\w', text):
            return 0.3
        return 0


class CssLexer(RegexLexer):
    """
    For CSS (Cascading Style Sheets).
    """

    name = 'CSS'
    aliases = ['css']
    filenames = ['*.css']
    mimetypes = ['text/css']

    tokens = {
        'root': [
            include('basics'),
        ],
        'basics': [
            (r'\s+', Text),
            (r'/\*(?:.|\n)*?\*/', Comment),
            (r'{', Punctuation, 'content'),
            (r'\:[a-zA-Z0-9_-]+', Name.Decorator),
            (r'\.[a-zA-Z0-9_-]+', Name.Class),
            (r'\#[a-zA-Z0-9_-]+', Name.Function),
            (r'@[a-zA-Z0-9_-]+', Keyword, 'atrule'),
            (r'[a-zA-Z0-9_-]+', Name.Tag),
            (r'[~\^\*!%&\[\]\(\)<>\|+=@:;,./?-]', Operator),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single)
        ],
        'atrule': [
            (r'{', Punctuation, 'atcontent'),
            (r';', Punctuation, '#pop'),
            include('basics'),
        ],
        'atcontent': [
            include('basics'),
            (r'}', Punctuation, '#pop:2'),
        ],
        'content': [
            (r'\s+', Text),
            (r'}', Punctuation, '#pop'),
            (r'url\(.*?\)', String.Other),
            (r'^@.*?$', Comment.Preproc),
            (r'(azimuth|background-attachment|background-color|'
             r'background-image|background-position|background-repeat|'
             r'background|border-bottom-color|border-bottom-style|'
             r'border-bottom-width|border-left-color|border-left-style|'
             r'border-left-width|border-right|border-right-color|'
             r'border-right-style|border-right-width|border-top-color|'
             r'border-top-style|border-top-width|border-bottom|'
             r'border-collapse|border-left|border-width|border-color|'
             r'border-spacing|border-style|border-top|border|caption-side|'
             r'clear|clip|color|content|counter-increment|counter-reset|'
             r'cue-after|cue-before|cue|cursor|direction|display|'
             r'elevation|empty-cells|float|font-family|font-size|'
             r'font-size-adjust|font-stretch|font-style|font-variant|'
             r'font-weight|font|height|letter-spacing|line-height|'
             r'list-style-type|list-style-image|list-style-position|'
             r'list-style|margin-bottom|margin-left|margin-right|'
             r'margin-top|margin|marker-offset|marks|max-height|max-width|'
             r'min-height|min-width|opacity|orphans|outline|outline-color|'
             r'outline-style|outline-width|overflow(?:-x|-y)?|padding-bottom|'
             r'padding-left|padding-right|padding-top|padding|page|'
             r'page-break-after|page-break-before|page-break-inside|'
             r'pause-after|pause-before|pause|pitch|pitch-range|'
             r'play-during|position|quotes|richness|right|size|'
             r'speak-header|speak-numeral|speak-punctuation|speak|'
             r'speech-rate|stress|table-layout|text-align|text-decoration|'
             r'text-indent|text-shadow|text-transform|top|unicode-bidi|'
             r'vertical-align|visibility|voice-family|volume|white-space|'
             r'widows|width|word-spacing|z-index|bottom|left|'
             r'above|absolute|always|armenian|aural|auto|avoid|baseline|'
             r'behind|below|bidi-override|blink|block|bold|bolder|both|'
             r'capitalize|center-left|center-right|center|circle|'
             r'cjk-ideographic|close-quote|collapse|condensed|continuous|'
             r'crop|crosshair|cross|cursive|dashed|decimal-leading-zero|'
             r'decimal|default|digits|disc|dotted|double|e-resize|embed|'
             r'extra-condensed|extra-expanded|expanded|fantasy|far-left|'
             r'far-right|faster|fast|fixed|georgian|groove|hebrew|help|'
             r'hidden|hide|higher|high|hiragana-iroha|hiragana|icon|'
             r'inherit|inline-table|inline|inset|inside|invert|italic|'
             r'justify|katakana-iroha|katakana|landscape|larger|large|'
             r'left-side|leftwards|level|lighter|line-through|list-item|'
             r'loud|lower-alpha|lower-greek|lower-roman|lowercase|ltr|'
             r'lower|low|medium|message-box|middle|mix|monospace|'
             r'n-resize|narrower|ne-resize|no-close-quote|no-open-quote|'
             r'no-repeat|none|normal|nowrap|nw-resize|oblique|once|'
             r'open-quote|outset|outside|overline|pointer|portrait|px|'
             r'relative|repeat-x|repeat-y|repeat|rgb|ridge|right-side|'
             r'rightwards|s-resize|sans-serif|scroll|se-resize|'
             r'semi-condensed|semi-expanded|separate|serif|show|silent|'
             r'slow|slower|small-caps|small-caption|smaller|soft|solid|'
             r'spell-out|square|static|status-bar|super|sw-resize|'
             r'table-caption|table-cell|table-column|table-column-group|'
             r'table-footer-group|table-header-group|table-row|'
             r'table-row-group|text|text-bottom|text-top|thick|thin|'
             r'transparent|ultra-condensed|ultra-expanded|underline|'
             r'upper-alpha|upper-latin|upper-roman|uppercase|url|'
             r'visible|w-resize|wait|wider|x-fast|x-high|x-large|x-loud|'
             r'x-low|x-small|x-soft|xx-large|xx-small|yes)\b', Keyword),
            (r'(indigo|gold|firebrick|indianred|yellow|darkolivegreen|'
             r'darkseagreen|mediumvioletred|mediumorchid|chartreuse|'
             r'mediumslateblue|black|springgreen|crimson|lightsalmon|brown|'
             r'turquoise|olivedrab|cyan|silver|skyblue|gray|darkturquoise|'
             r'goldenrod|darkgreen|darkviolet|darkgray|lightpink|teal|'
             r'darkmagenta|lightgoldenrodyellow|lavender|yellowgreen|thistle|'
             r'violet|navy|orchid|blue|ghostwhite|honeydew|cornflowerblue|'
             r'darkblue|darkkhaki|mediumpurple|cornsilk|red|bisque|slategray|'
             r'darkcyan|khaki|wheat|deepskyblue|darkred|steelblue|aliceblue|'
             r'gainsboro|mediumturquoise|floralwhite|coral|purple|lightgrey|'
             r'lightcyan|darksalmon|beige|azure|lightsteelblue|oldlace|'
             r'greenyellow|royalblue|lightseagreen|mistyrose|sienna|'
             r'lightcoral|orangered|navajowhite|lime|palegreen|burlywood|'
             r'seashell|mediumspringgreen|fuchsia|papayawhip|blanchedalmond|'
             r'peru|aquamarine|white|darkslategray|ivory|dodgerblue|'
             r'lemonchiffon|chocolate|orange|forestgreen|slateblue|olive|'
             r'mintcream|antiquewhite|darkorange|cadetblue|moccasin|'
             r'limegreen|saddlebrown|darkslateblue|lightskyblue|deeppink|'
             r'plum|aqua|darkgoldenrod|maroon|sandybrown|magenta|tan|'
             r'rosybrown|pink|lightblue|palevioletred|mediumseagreen|'
             r'dimgray|powderblue|seagreen|snow|mediumblue|midnightblue|'
             r'paleturquoise|palegoldenrod|whitesmoke|darkorchid|salmon|'
             r'lightslategray|lawngreen|lightgreen|tomato|hotpink|'
             r'lightyellow|lavenderblush|linen|mediumaquamarine|green|'
             r'blueviolet|peachpuff)\b', Name.Builtin),
            (r'\!important', Comment.Preproc),
            (r'/\*(?:.|\n)*?\*/', Comment),
            (r'\#[a-zA-Z0-9]{1,6}', Number),
            (r'[\.-]?[0-9]*[\.]?[0-9]+(em|px|\%|pt|pc|in|mm|cm|ex|s)\b', Number),
            (r'-?[0-9]+', Number),
            (r'[~\^\*!%&<>\|+=@:,./?-]+', Operator),
            (r'[\[\]();]+', Punctuation),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name)
        ]
    }


class ObjectiveJLexer(RegexLexer):
    """
    For Objective-J source code with preprocessor directives.

    *New in Pygments 1.3.*
    """

    name = 'Objective-J'
    aliases = ['objective-j', 'objectivej', 'obj-j', 'objj']
    filenames = ['*.j']
    mimetypes = ['text/x-objective-j']

    #: optional Comment or Whitespace
    _ws = r'(?:\s|//.*?\n|/[*].*?[*]/)*'

    flags = re.DOTALL | re.MULTILINE

    tokens = {
        'root': [
            include('whitespace'),

            # function definition
            (r'^(' + _ws + r'[\+-]' + _ws + r')([\(a-zA-Z_].*?[^\(])(' + _ws + '{)',
             bygroups(using(this), using(this, state='function_signature'),
                      using(this))),

            # class definition
            (r'(@interface|@implementation)(\s+)', bygroups(Keyword, Text),
             'classname'),
            (r'(@class|@protocol)(\s*)', bygroups(Keyword, Text),
             'forward_classname'),
            (r'(\s*)(@end)(\s*)', bygroups(Text, Keyword, Text)),

            include('statements'),
            ('[{\(\)}]', Punctuation),
            (';', Punctuation),
        ],
        'whitespace': [
            (r'(@import)(\s+)("(?:\\\\|\\"|[^"])*")',
             bygroups(Comment.Preproc, Text, String.Double)),
            (r'(@import)(\s+)(<(?:\\\\|\\>|[^>])*>)',
             bygroups(Comment.Preproc, Text, String.Double)),
            (r'(#(?:include|import))(\s+)("(?:\\\\|\\"|[^"])*")',
             bygroups(Comment.Preproc, Text, String.Double)),
            (r'(#(?:include|import))(\s+)(<(?:\\\\|\\>|[^>])*>)',
             bygroups(Comment.Preproc, Text, String.Double)),

            (r'#if\s+0', Comment.Preproc, 'if0'),
            (r'#', Comment.Preproc, 'macro'),

            (r'\n', Text),
            (r'\s+', Text),
            (r'\\\n', Text), # line continuation
            (r'//(\n|(.|\n)*?[^\\]\n)', Comment.Single),
            (r'/(\\\n)?[*](.|\n)*?[*](\\\n)?/', Comment.Multiline),
            (r'<!--', Comment),
        ],
        'slashstartsregex': [
            include('whitespace'),
            (r'/(\\.|[^[/\\\n]|\[(\\.|[^\]\\\n])*])+/'
             r'([gim]+\b|\B)', String.Regex, '#pop'),
            (r'(?=/)', Text, ('#pop', 'badregex')),
            (r'', Text, '#pop'),
        ],
        'badregex': [
            (r'\n', Text, '#pop'),
        ],
        'statements': [
            (r'(L|@)?"', String, 'string'),
            (r"(L|@)?'(\\.|\\[0-7]{1,3}|\\x[a-fA-F0-9]{1,2}|[^\\\'\n])'",
             String.Char),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
            (r'(\d+\.\d*|\.\d+|\d+)[eE][+-]?\d+[lL]?', Number.Float),
            (r'(\d+\.\d*|\.\d+|\d+[fF])[fF]?', Number.Float),
            (r'0x[0-9a-fA-F]+[Ll]?', Number.Hex),
            (r'0[0-7]+[Ll]?', Number.Oct),
            (r'\d+[Ll]?', Number.Integer),

            (r'^(?=\s|/|<!--)', Text, 'slashstartsregex'),

            (r'\+\+|--|~|&&|\?|:|\|\||\\(?=\n)|'
             r'(<<|>>>?|==?|!=?|[-<>+*%&\|\^/])=?',
             Operator, 'slashstartsregex'),
            (r'[{(\[;,]', Punctuation, 'slashstartsregex'),
            (r'[})\].]', Punctuation),

            (r'(for|in|while|do|break|return|continue|switch|case|default|if|'
             r'else|throw|try|catch|finally|new|delete|typeof|instanceof|void|'
             r'prototype|__proto__)\b', Keyword, 'slashstartsregex'),

            (r'(var|with|function)\b', Keyword.Declaration, 'slashstartsregex'),

            (r'(@selector|@private|@protected|@public|@encode|'
             r'@synchronized|@try|@throw|@catch|@finally|@end|@property|'
             r'@synthesize|@dynamic|@for|@accessors|new)\b', Keyword),

            (r'(int|long|float|short|double|char|unsigned|signed|void|'
             r'id|BOOL|bool|boolean|IBOutlet|IBAction|SEL|@outlet|@action)\b',
             Keyword.Type),

            (r'(self|super)\b', Name.Builtin),

            (r'(TRUE|YES|FALSE|NO|Nil|nil|NULL)\b', Keyword.Constant),
            (r'(true|false|null|NaN|Infinity|undefined)\b', Keyword.Constant),
            (r'(ABS|ASIN|ACOS|ATAN|ATAN2|SIN|COS|TAN|EXP|POW|CEIL|FLOOR|ROUND|'
             r'MIN|MAX|RAND|SQRT|E|LN2|LN10|LOG2E|LOG10E|PI|PI2|PI_2|SQRT1_2|'
             r'SQRT2)\b', Keyword.Constant),

            (r'(Array|Boolean|Date|Error|Function|Math|netscape|'
             r'Number|Object|Packages|RegExp|String|sun|decodeURI|'
             r'decodeURIComponent|encodeURI|encodeURIComponent|'
             r'Error|eval|isFinite|isNaN|parseFloat|parseInt|document|this|'
             r'window)\b', Name.Builtin),

            (r'([$a-zA-Z_][a-zA-Z0-9_]*)(' + _ws + r')(?=\()',
             bygroups(Name.Function, using(this))),

            (r'[$a-zA-Z_][a-zA-Z0-9_]*', Name),
        ],
        'classname' : [
            # interface definition that inherits
            (r'([a-zA-Z_][a-zA-Z0-9_]*)(' + _ws + r':' + _ws +
             r')([a-zA-Z_][a-zA-Z0-9_]*)?',
             bygroups(Name.Class, using(this), Name.Class), '#pop'),
            # interface definition for a category
            (r'([a-zA-Z_][a-zA-Z0-9_]*)(' + _ws + r'\()([a-zA-Z_][a-zA-Z0-9_]*)(\))',
             bygroups(Name.Class, using(this), Name.Label, Text), '#pop'),
            # simple interface / implementation
            (r'([a-zA-Z_][a-zA-Z0-9_]*)', Name.Class, '#pop'),
        ],
        'forward_classname' : [
            (r'([a-zA-Z_][a-zA-Z0-9_]*)(\s*,\s*)',
             bygroups(Name.Class, Text), '#push'),
            (r'([a-zA-Z_][a-zA-Z0-9_]*)(\s*;?)',
             bygroups(Name.Class, Text), '#pop'),
        ],
        'function_signature': [
            include('whitespace'),

            # start of a selector w/ parameters
            (r'(\(' + _ws + r')'                # open paren
             r'([a-zA-Z_][a-zA-Z0-9_]+)'        # return type
             r'(' + _ws + r'\)' + _ws + r')'    # close paren
             r'([$a-zA-Z_][a-zA-Z0-9_]+' + _ws + r':)', # function name
             bygroups(using(this), Keyword.Type, using(this),
                 Name.Function), 'function_parameters'),

            # no-param function
            (r'(\(' + _ws + r')'                # open paren
             r'([a-zA-Z_][a-zA-Z0-9_]+)'        # return type
             r'(' + _ws + r'\)' + _ws + r')'    # close paren
             r'([$a-zA-Z_][a-zA-Z0-9_]+)',      # function name
             bygroups(using(this), Keyword.Type, using(this),
                 Name.Function), "#pop"),

            # no return type given, start of a selector w/ parameters
            (r'([$a-zA-Z_][a-zA-Z0-9_]+' + _ws + r':)', # function name
             bygroups (Name.Function), 'function_parameters'),

            # no return type given, no-param function
            (r'([$a-zA-Z_][a-zA-Z0-9_]+)',      # function name
             bygroups(Name.Function), "#pop"),

            ('', Text, '#pop'),
        ],
        'function_parameters': [
            include('whitespace'),

            # parameters
            (r'(\(' + _ws + ')'                 # open paren
             r'([^\)]+)'                        # type
             r'(' + _ws + r'\)' + _ws + r')'    # close paren
             r'([$a-zA-Z_][a-zA-Z0-9_]+)',      # param name
             bygroups(using(this), Keyword.Type, using(this), Text)),

            # one piece of a selector name
            (r'([$a-zA-Z_][a-zA-Z0-9_]+' + _ws + r':)',     # function name
             Name.Function),

            # smallest possible selector piece
            (r'(:)', Name.Function),

            # var args
            (r'(,' + _ws + r'\.\.\.)', using(this)),

            # param name
            (r'([$a-zA-Z_][a-zA-Z0-9_]+)', Text),
        ],
        'expression' : [
            (r'([$a-zA-Z_][a-zA-Z0-9_]*)(\()', bygroups(Name.Function,
                                                        Punctuation)),
            (r'(\))', Punctuation, "#pop"),
        ],
        'string': [
            (r'"', String, '#pop'),
            (r'\\([\\abfnrtv"\']|x[a-fA-F0-9]{2,4}|[0-7]{1,3})', String.Escape),
            (r'[^\\"\n]+', String), # all other characters
            (r'\\\n', String), # line continuation
            (r'\\', String), # stray backslash
        ],
        'macro': [
            (r'[^/\n]+', Comment.Preproc),
            (r'/[*](.|\n)*?[*]/', Comment.Multiline),
            (r'//.*?\n', Comment.Single, '#pop'),
            (r'/', Comment.Preproc),
            (r'(?<=\\)\n', Comment.Preproc),
            (r'\n', Comment.Preproc, '#pop'),
        ],
        'if0': [
            (r'^\s*#if.*?(?<!\\)\n', Comment.Preproc, '#push'),
            (r'^\s*#endif.*?(?<!\\)\n', Comment.Preproc, '#pop'),
            (r'.*?\n', Comment),
        ]
    }

    def analyse_text(text):
        if re.search('^\s*@import\s+[<"]', text, re.MULTILINE):
            # special directive found in most Objective-J files
            return True
        return False


class HtmlLexer(RegexLexer):
    """
    For HTML 4 and XHTML 1 markup. Nested JavaScript and CSS is highlighted
    by the appropriate lexer.
    """

    name = 'HTML'
    aliases = ['html']
    filenames = ['*.html', '*.htm', '*.xhtml', '*.xslt']
    mimetypes = ['text/html', 'application/xhtml+xml']

    flags = re.IGNORECASE | re.DOTALL
    tokens = {
        'root': [
            ('[^<&]+', Text),
            (r'&\S*?;', Name.Entity),
            (r'\<\!\[CDATA\[.*?\]\]\>', Comment.Preproc),
            ('<!--', Comment, 'comment'),
            (r'<\?.*?\?>', Comment.Preproc),
            ('<![^>]*>', Comment.Preproc),
            (r'<\s*script\s*', Name.Tag, ('script-content', 'tag')),
            (r'<\s*style\s*', Name.Tag, ('style-content', 'tag')),
            (r'<\s*[a-zA-Z0-9:]+', Name.Tag, 'tag'),
            (r'<\s*/\s*[a-zA-Z0-9:]+\s*>', Name.Tag),
        ],
        'comment': [
            ('[^-]+', Comment),
            ('-->', Comment, '#pop'),
            ('-', Comment),
        ],
        'tag': [
            (r'\s+', Text),
            (r'[a-zA-Z0-9_:-]+\s*=', Name.Attribute, 'attr'),
            (r'[a-zA-Z0-9_:-]+', Name.Attribute),
            (r'/?\s*>', Name.Tag, '#pop'),
        ],
        'script-content': [
            (r'<\s*/\s*script\s*>', Name.Tag, '#pop'),
            (r'.+?(?=<\s*/\s*script\s*>)', using(JavascriptLexer)),
        ],
        'style-content': [
            (r'<\s*/\s*style\s*>', Name.Tag, '#pop'),
            (r'.+?(?=<\s*/\s*style\s*>)', using(CssLexer)),
        ],
        'attr': [
            ('".*?"', String, '#pop'),
            ("'.*?'", String, '#pop'),
            (r'[^\s>]+', String, '#pop'),
        ],
    }

    def analyse_text(text):
        if html_doctype_matches(text):
            return 0.5


class PhpLexer(RegexLexer):
    """
    For `PHP <http://www.php.net/>`_ source code.
    For PHP embedded in HTML, use the `HtmlPhpLexer`.

    Additional options accepted:

    `startinline`
        If given and ``True`` the lexer starts highlighting with
        php code (i.e.: no starting ``<?php`` required).  The default
        is ``False``.
    `funcnamehighlighting`
        If given and ``True``, highlight builtin function names
        (default: ``True``).
    `disabledmodules`
        If given, must be a list of module names whose function names
        should not be highlighted. By default all modules are highlighted
        except the special ``'unknown'`` module that includes functions
        that are known to php but are undocumented.

        To get a list of allowed modules have a look into the
        `_phpbuiltins` module:

        .. sourcecode:: pycon

            >>> from pygments.lexers._phpbuiltins import MODULES
            >>> MODULES.keys()
            ['PHP Options/Info', 'Zip', 'dba', ...]

        In fact the names of those modules match the module names from
        the php documentation.
    """

    name = 'PHP'
    aliases = ['php', 'php3', 'php4', 'php5']
    filenames = ['*.php', '*.php[345]', '*.inc']
    mimetypes = ['text/x-php']

    flags = re.IGNORECASE | re.DOTALL | re.MULTILINE
    tokens = {
        'root': [
            (r'<\?(php)?', Comment.Preproc, 'php'),
            (r'[^<]+', Other),
            (r'<', Other)
        ],
        'php': [
            (r'\?>', Comment.Preproc, '#pop'),
            (r'<<<(\'?)([a-zA-Z_][a-zA-Z0-9_]*)\1\n.*?\n\2\;?\n', String),
            (r'\s+', Text),
            (r'#.*?\n', Comment.Single),
            (r'//.*?\n', Comment.Single),
            # put the empty comment here, it is otherwise seen as
            # the start of a docstring
            (r'/\*\*/', Comment.Multiline),
            (r'/\*\*.*?\*/', String.Doc),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'(->|::)(\s*)([a-zA-Z_][a-zA-Z0-9_]*)',
             bygroups(Operator, Text, Name.Attribute)),
            (r'[~!%^&*+=|:.<>/?@-]+', Operator),
            (r'[\[\]{}();,]+', Punctuation),
            (r'(class)(\s+)', bygroups(Keyword, Text), 'classname'),
            (r'(function)(\s*)(?=\()', bygroups(Keyword, Text)),
            (r'(function)(\s+)(&?)(\s*)',
              bygroups(Keyword, Text, Operator, Text), 'functionname'),
            (r'(const)(\s+)([a-zA-Z_][a-zA-Z0-9_]*)',
              bygroups(Keyword, Text, Name.Constant)),
            (r'(and|E_PARSE|old_function|E_ERROR|or|as|E_WARNING|parent|'
             r'eval|PHP_OS|break|exit|case|extends|PHP_VERSION|cfunction|'
             r'FALSE|print|for|require|continue|foreach|require_once|'
             r'declare|return|default|static|do|switch|die|stdClass|'
             r'echo|else|TRUE|elseif|var|empty|if|xor|enddeclare|include|'
             r'virtual|endfor|include_once|while|endforeach|global|__FILE__|'
             r'endif|list|__LINE__|endswitch|new|__sleep|endwhile|not|'
             r'array|__wakeup|E_ALL|NULL|final|php_user_filter|interface|'
             r'implements|public|private|protected|abstract|clone|try|'
             r'catch|throw|this|use|namespace|trait)\b', Keyword),
            (r'(true|false|null)\b', Keyword.Constant),
            (r'\$\{\$+[a-zA-Z_][a-zA-Z0-9_]*\}', Name.Variable),
            (r'\$+[a-zA-Z_][a-zA-Z0-9_]*', Name.Variable),
            (r'[\\a-zA-Z_][\\a-zA-Z0-9_]*', Name.Other),
            (r'(\d+\.\d*|\d*\.\d+)([eE][+-]?[0-9]+)?', Number.Float),
            (r'\d+[eE][+-]?[0-9]+', Number.Float),
            (r'0[0-7]+', Number.Oct),
            (r'0[xX][a-fA-F0-9]+', Number.Hex),
            (r'\d+', Number.Integer),
            (r"'([^'\\]*(?:\\.[^'\\]*)*)'", String.Single),
            (r'`([^`\\]*(?:\\.[^`\\]*)*)`', String.Backtick),
            (r'"', String.Double, 'string'),
        ],
        'classname': [
            (r'[a-zA-Z_][\\a-zA-Z0-9_]*', Name.Class, '#pop')
        ],
        'functionname': [
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name.Function, '#pop')
        ],
        'string': [
            (r'"', String.Double, '#pop'),
            (r'[^{$"\\]+', String.Double),
            (r'\\([nrt\"$\\]|[0-7]{1,3}|x[0-9A-Fa-f]{1,2})', String.Escape),
            (r'\$[a-zA-Z_][a-zA-Z0-9_]*(\[\S+\]|->[a-zA-Z_][a-zA-Z0-9_]*)?',
             String.Interpol),
            (r'(\{\$\{)(.*?)(\}\})',
             bygroups(String.Interpol, using(this, _startinline=True),
                      String.Interpol)),
            (r'(\{)(\$.*?)(\})',
             bygroups(String.Interpol, using(this, _startinline=True),
                      String.Interpol)),
            (r'(\$\{)(\S+)(\})',
             bygroups(String.Interpol, Name.Variable, String.Interpol)),
            (r'[${\\]+', String.Double)
        ],
    }

    def __init__(self, **options):
        self.funcnamehighlighting = get_bool_opt(
            options, 'funcnamehighlighting', True)
        self.disabledmodules = get_list_opt(
            options, 'disabledmodules', ['unknown'])
        self.startinline = get_bool_opt(options, 'startinline', False)

        # private option argument for the lexer itself
        if '_startinline' in options:
            self.startinline = options.pop('_startinline')

        # collect activated functions in a set
        self._functions = set()
        if self.funcnamehighlighting:
            from pygments.lexers._phpbuiltins import MODULES
            for key, value in MODULES.iteritems():
                if key not in self.disabledmodules:
                    self._functions.update(value)
        RegexLexer.__init__(self, **options)

    def get_tokens_unprocessed(self, text):
        stack = ['root']
        if self.startinline:
            stack.append('php')
        for index, token, value in \
            RegexLexer.get_tokens_unprocessed(self, text, stack):
            if token is Name.Other:
                if value in self._functions:
                    yield index, Name.Builtin, value
                    continue
            yield index, token, value

    def analyse_text(text):
        rv = 0.0
        if re.search(r'<\?(?!xml)', text):
            rv += 0.3
        if '?>' in text:
            rv += 0.1
        return rv


class DtdLexer(RegexLexer):
    """
    A lexer for DTDs (Document Type Definitions).

    *New in Pygments 1.5.*
    """

    flags = re.MULTILINE | re.DOTALL

    name = 'DTD'
    aliases = ['dtd']
    filenames = ['*.dtd']
    mimetypes = ['application/xml-dtd']

    tokens = {
        'root': [
            include('common'),

            (r'(<!ELEMENT)(\s+)(\S+)',
                bygroups(Keyword, Text, Name.Tag), 'element'),
            (r'(<!ATTLIST)(\s+)(\S+)',
                bygroups(Keyword, Text, Name.Tag), 'attlist'),
            (r'(<!ENTITY)(\s+)(\S+)',
                bygroups(Keyword, Text, Name.Entity), 'entity'),
            (r'(<!NOTATION)(\s+)(\S+)',
                bygroups(Keyword, Text, Name.Tag), 'notation'),
            (r'(<!\[)([^\[\s]+)(\s*)(\[)', # conditional sections
                bygroups(Keyword, Name.Entity, Text, Keyword)),

            (r'(<!DOCTYPE)(\s+)([^>\s]+)',
                bygroups(Keyword, Text, Name.Tag)),
            (r'PUBLIC|SYSTEM', Keyword.Constant),
            (r'[\[\]>]', Keyword),
        ],

        'common': [
            (r'\s+', Text),
            (r'(%|&)[^;]*;', Name.Entity),
            ('<!--', Comment, 'comment'),
            (r'[(|)*,?+]', Operator),
            (r'"[^"]*"', String.Double),
            (r'\'[^\']*\'', String.Single),
        ],

        'comment': [
            ('[^-]+', Comment),
            ('-->', Comment, '#pop'),
            ('-', Comment),
        ],

        'element': [
            include('common'),
            (r'EMPTY|ANY|#PCDATA', Keyword.Constant),
            (r'[^>\s\|()?+*,]+', Name.Tag),
            (r'>', Keyword, '#pop'),
        ],

        'attlist': [
            include('common'),
            (r'CDATA|IDREFS|IDREF|ID|NMTOKENS|NMTOKEN|ENTITIES|ENTITY|NOTATION',
             Keyword.Constant),
            (r'#REQUIRED|#IMPLIED|#FIXED', Keyword.Constant),
            (r'xml:space|xml:lang', Keyword.Reserved),
            (r'[^>\s\|()?+*,]+', Name.Attribute),
            (r'>', Keyword, '#pop'),
        ],

        'entity': [
            include('common'),
            (r'SYSTEM|PUBLIC|NDATA', Keyword.Constant),
            (r'[^>\s\|()?+*,]+', Name.Entity),
            (r'>', Keyword, '#pop'),
        ],

        'notation': [
            include('common'),
            (r'SYSTEM|PUBLIC', Keyword.Constant),
            (r'[^>\s\|()?+*,]+', Name.Attribute),
            (r'>', Keyword, '#pop'),
        ],
    }

    def analyse_text(text):
        if not looks_like_xml(text) and \
            ('<!ELEMENT' in text or '<!ATTLIST' in text or '<!ENTITY' in text):
            return 0.8

class XmlLexer(RegexLexer):
    """
    Generic lexer for XML (eXtensible Markup Language).
    """

    flags = re.MULTILINE | re.DOTALL | re.UNICODE

    name = 'XML'
    aliases = ['xml']
    filenames = ['*.xml', '*.xsl', '*.rss', '*.xslt', '*.xsd', '*.wsdl']
    mimetypes = ['text/xml', 'application/xml', 'image/svg+xml',
                 'application/rss+xml', 'application/atom+xml']

    tokens = {
        'root': [
            ('[^<&]+', Text),
            (r'&\S*?;', Name.Entity),
            (r'\<\!\[CDATA\[.*?\]\]\>', Comment.Preproc),
            ('<!--', Comment, 'comment'),
            (r'<\?.*?\?>', Comment.Preproc),
            ('<![^>]*>', Comment.Preproc),
            (r'<\s*[\w:.-]+', Name.Tag, 'tag'),
            (r'<\s*/\s*[\w:.-]+\s*>', Name.Tag),
        ],
        'comment': [
            ('[^-]+', Comment),
            ('-->', Comment, '#pop'),
            ('-', Comment),
        ],
        'tag': [
            (r'\s+', Text),
            (r'[\w.:-]+\s*=', Name.Attribute, 'attr'),
            (r'/?\s*>', Name.Tag, '#pop'),
        ],
        'attr': [
            ('\s+', Text),
            ('".*?"', String, '#pop'),
            ("'.*?'", String, '#pop'),
            (r'[^\s>]+', String, '#pop'),
        ],
    }

    def analyse_text(text):
        if looks_like_xml(text):
            return 0.5


class XsltLexer(XmlLexer):
    '''
    A lexer for XSLT.

    *New in Pygments 0.10.*
    '''

    name = 'XSLT'
    aliases = ['xslt']
    filenames = ['*.xsl', '*.xslt', '*.xpl']  # xpl is XProc
    mimetypes = ['application/xsl+xml', 'application/xslt+xml']

    EXTRA_KEYWORDS = set([
        'apply-imports', 'apply-templates', 'attribute',
        'attribute-set', 'call-template', 'choose', 'comment',
        'copy', 'copy-of', 'decimal-format', 'element', 'fallback',
        'for-each', 'if', 'import', 'include', 'key', 'message',
        'namespace-alias', 'number', 'otherwise', 'output', 'param',
        'preserve-space', 'processing-instruction', 'sort',
        'strip-space', 'stylesheet', 'template', 'text', 'transform',
        'value-of', 'variable', 'when', 'with-param'
    ])

    def get_tokens_unprocessed(self, text):
        for index, token, value in XmlLexer.get_tokens_unprocessed(self, text):
            m = re.match('</?xsl:([^>]*)/?>?', value)

            if token is Name.Tag and m and m.group(1) in self.EXTRA_KEYWORDS:
                yield index, Keyword, value
            else:
                yield index, token, value

    def analyse_text(text):
        if looks_like_xml(text) and '<xsl' in text:
            return 0.8


class MxmlLexer(RegexLexer):
    """
    For MXML markup.
    Nested AS3 in <script> tags is highlighted by the appropriate lexer.

    *New in Pygments 1.1.*
    """
    flags = re.MULTILINE | re.DOTALL
    name = 'MXML'
    aliases = ['mxml']
    filenames = ['*.mxml']
    mimetimes = ['text/xml', 'application/xml']

    tokens = {
            'root': [
                ('[^<&]+', Text),
                (r'&\S*?;', Name.Entity),
                (r'(\<\!\[CDATA\[)(.*?)(\]\]\>)',
                 bygroups(String, using(ActionScript3Lexer), String)),
                ('<!--', Comment, 'comment'),
                (r'<\?.*?\?>', Comment.Preproc),
                ('<![^>]*>', Comment.Preproc),
                (r'<\s*[a-zA-Z0-9:._-]+', Name.Tag, 'tag'),
                (r'<\s*/\s*[a-zA-Z0-9:._-]+\s*>', Name.Tag),
            ],
            'comment': [
                ('[^-]+', Comment),
                ('-->', Comment, '#pop'),
                ('-', Comment),
            ],
            'tag': [
                (r'\s+', Text),
                (r'[a-zA-Z0-9_.:-]+\s*=', Name.Attribute, 'attr'),
                (r'/?\s*>', Name.Tag, '#pop'),
            ],
            'attr': [
                ('\s+', Text),
                ('".*?"', String, '#pop'),
                ("'.*?'", String, '#pop'),
                (r'[^\s>]+', String, '#pop'),
            ],
        }


class HaxeLexer(RegexLexer):
    """
    For haXe source code (http://haxe.org/).

    *New in Pygments 1.3.*
    """

    name = 'haXe'
    aliases = ['hx', 'haXe']
    filenames = ['*.hx']
    mimetypes = ['text/haxe']

    ident = r'(?:[a-zA-Z_][a-zA-Z0-9_]*)'
    typeid = r'(?:(?:[a-z0-9_\.])*[A-Z_][A-Za-z0-9_]*)'
    key_prop = r'(?:default|null|never)'
    key_decl_mod = r'(?:public|private|override|static|inline|extern|dynamic)'

    flags = re.DOTALL | re.MULTILINE

    tokens = {
        'root': [
            include('whitespace'),
            include('comments'),
            (key_decl_mod, Keyword.Declaration),
            include('enumdef'),
            include('typedef'),
            include('classdef'),
            include('imports'),
        ],

        # General constructs
        'comments': [
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'#[^\n]*', Comment.Preproc),
        ],
        'whitespace': [
            include('comments'),
            (r'\s+', Text),
        ],
        'codekeywords': [
            (r'\b(if|else|while|do|for|in|break|continue|'
             r'return|switch|case|try|catch|throw|null|trace|'
             r'new|this|super|untyped|cast|callback|here)\b',
             Keyword.Reserved),
        ],
        'literals': [
            (r'0[xX][0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+', Number.Integer),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r'~/([^\n])*?/[gisx]*', String.Regex),
            (r'\b(true|false|null)\b', Keyword.Constant),
        ],
        'codeblock': [
          include('whitespace'),
          include('new'),
          include('case'),
          include('anonfundef'),
          include('literals'),
          include('vardef'),
          include('codekeywords'),
          (r'[();,\[\]]', Punctuation),
          (r'(?:=|\+=|-=|\*=|/=|%=|&=|\|=|\^=|<<=|>>=|>>>=|\|\||&&|'
           r'\.\.\.|==|!=|>|<|>=|<=|\||&|\^|<<|>>>|>>|\+|\-|\*|/|%|'
           r'!|\+\+|\-\-|~|\.|\?|\:)',
           Operator),
          (ident, Name),

          (r'}', Punctuation,'#pop'),
          (r'{', Punctuation,'#push'),
        ],

        # Instance/Block level constructs
        'propertydef': [
            (r'(\()(' + key_prop + ')(,)(' + key_prop + ')(\))',
             bygroups(Punctuation, Keyword.Reserved, Punctuation,
                      Keyword.Reserved, Punctuation)),
        ],
        'new': [
            (r'\bnew\b', Keyword, 'typedecl'),
        ],
        'case': [
            (r'\b(case)(\s+)(' + ident + ')(\s*)(\()',
             bygroups(Keyword.Reserved, Text, Name, Text, Punctuation),
             'funargdecl'),
        ],
        'vardef': [
            (r'\b(var)(\s+)(' + ident + ')',
             bygroups(Keyword.Declaration, Text, Name.Variable), 'vardecl'),
        ],
        'vardecl': [
            include('whitespace'),
            include('typelabel'),
            (r'=', Operator,'#pop'),
            (r';', Punctuation,'#pop'),
        ],
        'instancevardef': [
            (key_decl_mod,Keyword.Declaration),
            (r'\b(var)(\s+)(' + ident + ')',
             bygroups(Keyword.Declaration, Text, Name.Variable.Instance),
             'instancevardecl'),
        ],
        'instancevardecl': [
            include('vardecl'),
            include('propertydef'),
        ],

        'anonfundef': [
            (r'\bfunction\b', Keyword.Declaration, 'fundecl'),
        ],
        'instancefundef': [
            (key_decl_mod, Keyword.Declaration),
            (r'\b(function)(\s+)(' + ident + ')',
             bygroups(Keyword.Declaration, Text, Name.Function), 'fundecl'),
        ],
        'fundecl': [
            include('whitespace'),
            include('typelabel'),
            include('generictypedecl'),
            (r'\(',Punctuation,'funargdecl'),
            (r'(?=[a-zA-Z0-9_])',Text,'#pop'),
            (r'{',Punctuation,('#pop','codeblock')),
            (r';',Punctuation,'#pop'),
        ],
        'funargdecl': [
            include('whitespace'),
            (ident, Name.Variable),
            include('typelabel'),
            include('literals'),
            (r'=', Operator),
            (r',', Punctuation),
            (r'\?', Punctuation),
            (r'\)', Punctuation, '#pop'),
        ],

        'typelabel': [
            (r':', Punctuation, 'type'),
        ],
        'typedecl': [
            include('whitespace'),
            (typeid, Name.Class),
            (r'<', Punctuation, 'generictypedecl'),
            (r'(?=[{}()=,a-z])', Text,'#pop'),
        ],
        'type': [
            include('whitespace'),
            (typeid, Name.Class),
            (r'<', Punctuation, 'generictypedecl'),
            (r'->', Keyword.Type),
            (r'(?=[{}(),;=])', Text, '#pop'),
        ],
        'generictypedecl': [
            include('whitespace'),
            (typeid, Name.Class),
            (r'<', Punctuation, '#push'),
            (r'>', Punctuation, '#pop'),
            (r',', Punctuation),
        ],

        # Top level constructs
        'imports': [
            (r'(package|import|using)(\s+)([^;]+)(;)',
             bygroups(Keyword.Namespace, Text, Name.Namespace,Punctuation)),
        ],
        'typedef': [
            (r'typedef', Keyword.Declaration, ('typedefprebody', 'typedecl')),
        ],
        'typedefprebody': [
            include('whitespace'),
            (r'(=)(\s*)({)', bygroups(Punctuation, Text, Punctuation),
             ('#pop', 'typedefbody')),
        ],
        'enumdef': [
            (r'enum', Keyword.Declaration, ('enumdefprebody', 'typedecl')),
        ],
        'enumdefprebody': [
            include('whitespace'),
            (r'{', Punctuation, ('#pop','enumdefbody')),
        ],
        'classdef': [
            (r'class', Keyword.Declaration, ('classdefprebody', 'typedecl')),
        ],
        'classdefprebody': [
            include('whitespace'),
            (r'(extends|implements)', Keyword.Declaration,'typedecl'),
            (r'{', Punctuation, ('#pop', 'classdefbody')),
        ],
        'interfacedef': [
            (r'interface', Keyword.Declaration,
             ('interfacedefprebody', 'typedecl')),
        ],
        'interfacedefprebody': [
            include('whitespace'),
            (r'(extends)', Keyword.Declaration, 'typedecl'),
            (r'{', Punctuation, ('#pop', 'classdefbody')),
        ],

        'typedefbody': [
          include('whitespace'),
          include('instancevardef'),
          include('instancefundef'),
          (r'>', Punctuation, 'typedecl'),
          (r',', Punctuation),
          (r'}', Punctuation, '#pop'),
        ],
        'enumdefbody': [
          include('whitespace'),
          (ident, Name.Variable.Instance),
          (r'\(', Punctuation, 'funargdecl'),
          (r';', Punctuation),
          (r'}', Punctuation, '#pop'),
        ],
        'classdefbody': [
          include('whitespace'),
          include('instancevardef'),
          include('instancefundef'),
          (r'}', Punctuation, '#pop'),
          include('codeblock'),
        ],
    }

    def analyse_text(text):
        if re.match(r'\w+\s*:\s*\w', text): return 0.3


def _indentation(lexer, match, ctx):
    indentation = match.group(0)
    yield match.start(), Text, indentation
    ctx.last_indentation = indentation
    ctx.pos = match.end()

    if hasattr(ctx, 'block_state') and ctx.block_state and \
            indentation.startswith(ctx.block_indentation) and \
            indentation != ctx.block_indentation:
        ctx.stack.append(ctx.block_state)
    else:
        ctx.block_state = None
        ctx.block_indentation = None
        ctx.stack.append('content')

def _starts_block(token, state):
    def callback(lexer, match, ctx):
        yield match.start(), token, match.group(0)

        if hasattr(ctx, 'last_indentation'):
            ctx.block_indentation = ctx.last_indentation
        else:
            ctx.block_indentation = ''

        ctx.block_state = state
        ctx.pos = match.end()

    return callback


class HamlLexer(ExtendedRegexLexer):
    """
    For Haml markup.

    *New in Pygments 1.3.*
    """

    name = 'Haml'
    aliases = ['haml', 'HAML']
    filenames = ['*.haml']
    mimetypes = ['text/x-haml']

    flags = re.IGNORECASE
    # Haml can include " |\n" anywhere,
    # which is ignored and used to wrap long lines.
    # To accomodate this, use this custom faux dot instead.
    _dot = r'(?: \|\n(?=.* \|)|.)'

    # In certain places, a comma at the end of the line
    # allows line wrapping as well.
    _comma_dot = r'(?:,\s*\n|' + _dot + ')'
    tokens = {
        'root': [
            (r'[ \t]*\n', Text),
            (r'[ \t]*', _indentation),
        ],

        'css': [
            (r'\.[a-z0-9_:-]+', Name.Class, 'tag'),
            (r'\#[a-z0-9_:-]+', Name.Function, 'tag'),
        ],

        'eval-or-plain': [
            (r'[&!]?==', Punctuation, 'plain'),
            (r'([&!]?[=~])(' + _comma_dot + r'*\n)',
             bygroups(Punctuation, using(RubyLexer)),
             'root'),
            (r'', Text, 'plain'),
        ],

        'content': [
            include('css'),
            (r'%[a-z0-9_:-]+', Name.Tag, 'tag'),
            (r'!!!' + _dot + r'*\n', Name.Namespace, '#pop'),
            (r'(/)(\[' + _dot + '*?\])(' + _dot + r'*\n)',
             bygroups(Comment, Comment.Special, Comment),
             '#pop'),
            (r'/' + _dot + r'*\n', _starts_block(Comment, 'html-comment-block'),
             '#pop'),
            (r'-#' + _dot + r'*\n', _starts_block(Comment.Preproc,
                                                 'haml-comment-block'), '#pop'),
            (r'(-)(' + _comma_dot + r'*\n)',
             bygroups(Punctuation, using(RubyLexer)),
             '#pop'),
            (r':' + _dot + r'*\n', _starts_block(Name.Decorator, 'filter-block'),
             '#pop'),
            include('eval-or-plain'),
        ],

        'tag': [
            include('css'),
            (r'\{(,\n|' + _dot + ')*?\}', using(RubyLexer)),
            (r'\[' + _dot + '*?\]', using(RubyLexer)),
            (r'\(', Text, 'html-attributes'),
            (r'/[ \t]*\n', Punctuation, '#pop:2'),
            (r'[<>]{1,2}(?=[ \t=])', Punctuation),
            include('eval-or-plain'),
        ],

        'plain': [
            (r'([^#\n]|#[^{\n]|(\\\\)*\\#\{)+', Text),
            (r'(#\{)(' + _dot + '*?)(\})',
             bygroups(String.Interpol, using(RubyLexer), String.Interpol)),
            (r'\n', Text, 'root'),
        ],

        'html-attributes': [
            (r'\s+', Text),
            (r'[a-z0-9_:-]+[ \t]*=', Name.Attribute, 'html-attribute-value'),
            (r'[a-z0-9_:-]+', Name.Attribute),
            (r'\)', Text, '#pop'),
        ],

        'html-attribute-value': [
            (r'[ \t]+', Text),
            (r'[a-z0-9_]+', Name.Variable, '#pop'),
            (r'@[a-z0-9_]+', Name.Variable.Instance, '#pop'),
            (r'\$[a-z0-9_]+', Name.Variable.Global, '#pop'),
            (r"'(\\\\|\\'|[^'\n])*'", String, '#pop'),
            (r'"(\\\\|\\"|[^"\n])*"', String, '#pop'),
        ],

        'html-comment-block': [
            (_dot + '+', Comment),
            (r'\n', Text, 'root'),
        ],

        'haml-comment-block': [
            (_dot + '+', Comment.Preproc),
            (r'\n', Text, 'root'),
        ],

        'filter-block': [
            (r'([^#\n]|#[^{\n]|(\\\\)*\\#\{)+', Name.Decorator),
            (r'(#\{)(' + _dot + '*?)(\})',
             bygroups(String.Interpol, using(RubyLexer), String.Interpol)),
            (r'\n', Text, 'root'),
        ],
    }


common_sass_tokens = {
    'value': [
        (r'[ \t]+', Text),
        (r'[!$][\w-]+', Name.Variable),
        (r'url\(', String.Other, 'string-url'),
        (r'[a-z_-][\w-]*(?=\()', Name.Function),
        (r'(azimuth|background-attachment|background-color|'
         r'background-image|background-position|background-repeat|'
         r'background|border-bottom-color|border-bottom-style|'
         r'border-bottom-width|border-left-color|border-left-style|'
         r'border-left-width|border-right|border-right-color|'
         r'border-right-style|border-right-width|border-top-color|'
         r'border-top-style|border-top-width|border-bottom|'
         r'border-collapse|border-left|border-width|border-color|'
         r'border-spacing|border-style|border-top|border|caption-side|'
         r'clear|clip|color|content|counter-increment|counter-reset|'
         r'cue-after|cue-before|cue|cursor|direction|display|'
         r'elevation|empty-cells|float|font-family|font-size|'
         r'font-size-adjust|font-stretch|font-style|font-variant|'
         r'font-weight|font|height|letter-spacing|line-height|'
         r'list-style-type|list-style-image|list-style-position|'
         r'list-style|margin-bottom|margin-left|margin-right|'
         r'margin-top|margin|marker-offset|marks|max-height|max-width|'
         r'min-height|min-width|opacity|orphans|outline|outline-color|'
         r'outline-style|outline-width|overflow|padding-bottom|'
         r'padding-left|padding-right|padding-top|padding|page|'
         r'page-break-after|page-break-before|page-break-inside|'
         r'pause-after|pause-before|pause|pitch|pitch-range|'
         r'play-during|position|quotes|richness|right|size|'
         r'speak-header|speak-numeral|speak-punctuation|speak|'
         r'speech-rate|stress|table-layout|text-align|text-decoration|'
         r'text-indent|text-shadow|text-transform|top|unicode-bidi|'
         r'vertical-align|visibility|voice-family|volume|white-space|'
         r'widows|width|word-spacing|z-index|bottom|left|'
         r'above|absolute|always|armenian|aural|auto|avoid|baseline|'
         r'behind|below|bidi-override|blink|block|bold|bolder|both|'
         r'capitalize|center-left|center-right|center|circle|'
         r'cjk-ideographic|close-quote|collapse|condensed|continuous|'
         r'crop|crosshair|cross|cursive|dashed|decimal-leading-zero|'
         r'decimal|default|digits|disc|dotted|double|e-resize|embed|'
         r'extra-condensed|extra-expanded|expanded|fantasy|far-left|'
         r'far-right|faster|fast|fixed|georgian|groove|hebrew|help|'
         r'hidden|hide|higher|high|hiragana-iroha|hiragana|icon|'
         r'inherit|inline-table|inline|inset|inside|invert|italic|'
         r'justify|katakana-iroha|katakana|landscape|larger|large|'
         r'left-side|leftwards|level|lighter|line-through|list-item|'
         r'loud|lower-alpha|lower-greek|lower-roman|lowercase|ltr|'
         r'lower|low|medium|message-box|middle|mix|monospace|'
         r'n-resize|narrower|ne-resize|no-close-quote|no-open-quote|'
         r'no-repeat|none|normal|nowrap|nw-resize|oblique|once|'
         r'open-quote|outset|outside|overline|pointer|portrait|px|'
         r'relative|repeat-x|repeat-y|repeat|rgb|ridge|right-side|'
         r'rightwards|s-resize|sans-serif|scroll|se-resize|'
         r'semi-condensed|semi-expanded|separate|serif|show|silent|'
         r'slow|slower|small-caps|small-caption|smaller|soft|solid|'
         r'spell-out|square|static|status-bar|super|sw-resize|'
         r'table-caption|table-cell|table-column|table-column-group|'
         r'table-footer-group|table-header-group|table-row|'
         r'table-row-group|text|text-bottom|text-top|thick|thin|'
         r'transparent|ultra-condensed|ultra-expanded|underline|'
         r'upper-alpha|upper-latin|upper-roman|uppercase|url|'
         r'visible|w-resize|wait|wider|x-fast|x-high|x-large|x-loud|'
         r'x-low|x-small|x-soft|xx-large|xx-small|yes)\b', Name.Constant),
        (r'(indigo|gold|firebrick|indianred|darkolivegreen|'
         r'darkseagreen|mediumvioletred|mediumorchid|chartreuse|'
         r'mediumslateblue|springgreen|crimson|lightsalmon|brown|'
         r'turquoise|olivedrab|cyan|skyblue|darkturquoise|'
         r'goldenrod|darkgreen|darkviolet|darkgray|lightpink|'
         r'darkmagenta|lightgoldenrodyellow|lavender|yellowgreen|thistle|'
         r'violet|orchid|ghostwhite|honeydew|cornflowerblue|'
         r'darkblue|darkkhaki|mediumpurple|cornsilk|bisque|slategray|'
         r'darkcyan|khaki|wheat|deepskyblue|darkred|steelblue|aliceblue|'
         r'gainsboro|mediumturquoise|floralwhite|coral|lightgrey|'
         r'lightcyan|darksalmon|beige|azure|lightsteelblue|oldlace|'
         r'greenyellow|royalblue|lightseagreen|mistyrose|sienna|'
         r'lightcoral|orangered|navajowhite|palegreen|burlywood|'
         r'seashell|mediumspringgreen|papayawhip|blanchedalmond|'
         r'peru|aquamarine|darkslategray|ivory|dodgerblue|'
         r'lemonchiffon|chocolate|orange|forestgreen|slateblue|'
         r'mintcream|antiquewhite|darkorange|cadetblue|moccasin|'
         r'limegreen|saddlebrown|darkslateblue|lightskyblue|deeppink|'
         r'plum|darkgoldenrod|sandybrown|magenta|tan|'
         r'rosybrown|pink|lightblue|palevioletred|mediumseagreen|'
         r'dimgray|powderblue|seagreen|snow|mediumblue|midnightblue|'
         r'paleturquoise|palegoldenrod|whitesmoke|darkorchid|salmon|'
         r'lightslategray|lawngreen|lightgreen|tomato|hotpink|'
         r'lightyellow|lavenderblush|linen|mediumaquamarine|'
         r'blueviolet|peachpuff)\b', Name.Entity),
        (r'(black|silver|gray|white|maroon|red|purple|fuchsia|green|'
         r'lime|olive|yellow|navy|blue|teal|aqua)\b', Name.Builtin),
        (r'\!(important|default)', Name.Exception),
        (r'(true|false)', Name.Pseudo),
        (r'(and|or|not)', Operator.Word),
        (r'/\*', Comment.Multiline, 'inline-comment'),
        (r'//[^\n]*', Comment.Single),
        (r'\#[a-z0-9]{1,6}', Number.Hex),
        (r'(-?\d+)(\%|[a-z]+)?', bygroups(Number.Integer, Keyword.Type)),
        (r'(-?\d*\.\d+)(\%|[a-z]+)?', bygroups(Number.Float, Keyword.Type)),
        (r'#{', String.Interpol, 'interpolation'),
        (r'[~\^\*!&%<>\|+=@:,./?-]+', Operator),
        (r'[\[\]()]+', Punctuation),
        (r'"', String.Double, 'string-double'),
        (r"'", String.Single, 'string-single'),
        (r'[a-z_-][\w-]*', Name),
    ],

    'interpolation': [
        (r'\}', String.Interpol, '#pop'),
        include('value'),
    ],

    'selector': [
        (r'[ \t]+', Text),
        (r'\:', Name.Decorator, 'pseudo-class'),
        (r'\.', Name.Class, 'class'),
        (r'\#', Name.Namespace, 'id'),
        (r'[a-zA-Z0-9_-]+', Name.Tag),
        (r'#\{', String.Interpol, 'interpolation'),
        (r'&', Keyword),
        (r'[~\^\*!&\[\]\(\)<>\|+=@:;,./?-]', Operator),
        (r'"', String.Double, 'string-double'),
        (r"'", String.Single, 'string-single'),
    ],

    'string-double': [
        (r'(\\.|#(?=[^\n{])|[^\n"#])+', String.Double),
        (r'#\{', String.Interpol, 'interpolation'),
        (r'"', String.Double, '#pop'),
    ],

    'string-single': [
        (r"(\\.|#(?=[^\n{])|[^\n'#])+", String.Double),
        (r'#\{', String.Interpol, 'interpolation'),
        (r"'", String.Double, '#pop'),
    ],

    'string-url': [
        (r'(\\#|#(?=[^\n{])|[^\n#)])+', String.Other),
        (r'#\{', String.Interpol, 'interpolation'),
        (r'\)', String.Other, '#pop'),
    ],

    'pseudo-class': [
        (r'[\w-]+', Name.Decorator),
        (r'#\{', String.Interpol, 'interpolation'),
        (r'', Text, '#pop'),
    ],

    'class': [
        (r'[\w-]+', Name.Class),
        (r'#\{', String.Interpol, 'interpolation'),
        (r'', Text, '#pop'),
    ],

    'id': [
        (r'[\w-]+', Name.Namespace),
        (r'#\{', String.Interpol, 'interpolation'),
        (r'', Text, '#pop'),
    ],

    'for': [
        (r'(from|to|through)', Operator.Word),
        include('value'),
    ],
}

class SassLexer(ExtendedRegexLexer):
    """
    For Sass stylesheets.

    *New in Pygments 1.3.*
    """

    name = 'Sass'
    aliases = ['sass', 'SASS']
    filenames = ['*.sass']
    mimetypes = ['text/x-sass']

    flags = re.IGNORECASE
    tokens = {
        'root': [
            (r'[ \t]*\n', Text),
            (r'[ \t]*', _indentation),
        ],

        'content': [
            (r'//[^\n]*', _starts_block(Comment.Single, 'single-comment'),
             'root'),
            (r'/\*[^\n]*', _starts_block(Comment.Multiline, 'multi-comment'),
             'root'),
            (r'@import', Keyword, 'import'),
            (r'@for', Keyword, 'for'),
            (r'@(debug|warn|if|while)', Keyword, 'value'),
            (r'(@mixin)( [\w-]+)', bygroups(Keyword, Name.Function), 'value'),
            (r'(@include)( [\w-]+)', bygroups(Keyword, Name.Decorator), 'value'),
            (r'@extend', Keyword, 'selector'),
            (r'@[a-z0-9_-]+', Keyword, 'selector'),
            (r'=[\w-]+', Name.Function, 'value'),
            (r'\+[\w-]+', Name.Decorator, 'value'),
            (r'([!$][\w-]\w*)([ \t]*(?:(?:\|\|)?=|:))',
             bygroups(Name.Variable, Operator), 'value'),
            (r':', Name.Attribute, 'old-style-attr'),
            (r'(?=.+?[=:]([^a-z]|$))', Name.Attribute, 'new-style-attr'),
            (r'', Text, 'selector'),
        ],

        'single-comment': [
            (r'.+', Comment.Single),
            (r'\n', Text, 'root'),
        ],

        'multi-comment': [
            (r'.+', Comment.Multiline),
            (r'\n', Text, 'root'),
        ],

        'import': [
            (r'[ \t]+', Text),
            (r'\S+', String),
            (r'\n', Text, 'root'),
        ],

        'old-style-attr': [
            (r'[^\s:="\[]+', Name.Attribute),
            (r'#{', String.Interpol, 'interpolation'),
            (r'[ \t]*=', Operator, 'value'),
            (r'', Text, 'value'),
        ],

        'new-style-attr': [
            (r'[^\s:="\[]+', Name.Attribute),
            (r'#{', String.Interpol, 'interpolation'),
            (r'[ \t]*[=:]', Operator, 'value'),
        ],

        'inline-comment': [
            (r"(\\#|#(?=[^\n{])|\*(?=[^\n/])|[^\n#*])+", Comment.Multiline),
            (r'#\{', String.Interpol, 'interpolation'),
            (r"\*/", Comment, '#pop'),
        ],
    }
    for group, common in common_sass_tokens.iteritems():
        tokens[group] = copy.copy(common)
    tokens['value'].append((r'\n', Text, 'root'))
    tokens['selector'].append((r'\n', Text, 'root'))


class ScssLexer(RegexLexer):
    """
    For SCSS stylesheets.
    """

    name = 'SCSS'
    aliases = ['scss']
    filenames = ['*.scss']
    mimetypes = ['text/x-scss']

    flags = re.IGNORECASE | re.DOTALL
    tokens = {
        'root': [
            (r'\s+', Text),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'@import', Keyword, 'value'),
            (r'@for', Keyword, 'for'),
            (r'@(debug|warn|if|while)', Keyword, 'value'),
            (r'(@mixin)( [\w-]+)', bygroups(Keyword, Name.Function), 'value'),
            (r'(@include)( [\w-]+)', bygroups(Keyword, Name.Decorator), 'value'),
            (r'@extend', Keyword, 'selector'),
            (r'@[a-z0-9_-]+', Keyword, 'selector'),
            (r'(\$[\w-]*\w)([ \t]*:)', bygroups(Name.Variable, Operator), 'value'),
            (r'(?=[^;{}][;}])', Name.Attribute, 'attr'),
            (r'(?=[^;{}:]+:[^a-z])', Name.Attribute, 'attr'),
            (r'', Text, 'selector'),
        ],

        'attr': [
            (r'[^\s:="\[]+', Name.Attribute),
            (r'#{', String.Interpol, 'interpolation'),
            (r'[ \t]*:', Operator, 'value'),
        ],

        'inline-comment': [
            (r"(\\#|#(?=[^{])|\*(?=[^/])|[^#*])+", Comment.Multiline),
            (r'#\{', String.Interpol, 'interpolation'),
            (r"\*/", Comment, '#pop'),
        ],
    }
    for group, common in common_sass_tokens.iteritems():
        tokens[group] = copy.copy(common)
    tokens['value'].extend([(r'\n', Text), (r'[;{}]', Punctuation, 'root')])
    tokens['selector'].extend([(r'\n', Text), (r'[;{}]', Punctuation, 'root')])


class CoffeeScriptLexer(RegexLexer):
    """
    For `CoffeeScript`_ source code.

    .. _CoffeeScript: http://coffeescript.org

    *New in Pygments 1.3.*
    """

    name = 'CoffeeScript'
    aliases = ['coffee-script', 'coffeescript']
    filenames = ['*.coffee']
    mimetypes = ['text/coffeescript']

    flags = re.DOTALL
    tokens = {
        'commentsandwhitespace': [
            (r'\s+', Text),
            (r'###[^#].*?###', Comment.Multiline),
            (r'#(?!##[^#]).*?\n', Comment.Single),
        ],
        'multilineregex': [
            (r'[^/#]+', String.Regex),
            (r'///([gim]+\b|\B)', String.Regex, '#pop'),
            (r'#{', String.Interpol, 'interpoling_string'),
            (r'[/#]', String.Regex),
        ],
        'slashstartsregex': [
            include('commentsandwhitespace'),
            (r'///', String.Regex, ('#pop', 'multilineregex')),
            (r'/(?! )(\\.|[^[/\\\n]|\[(\\.|[^\]\\\n])*])+/'
             r'([gim]+\b|\B)', String.Regex, '#pop'),
            (r'', Text, '#pop'),
        ],
        'root': [
            # this next expr leads to infinite loops root -> slashstartsregex
            #(r'^(?=\s|/|<!--)', Text, 'slashstartsregex'),
            include('commentsandwhitespace'),
            (r'\+\+|~|&&|\band\b|\bor\b|\bis\b|\bisnt\b|\bnot\b|\?|:|'
             r'\|\||\\(?=\n)|(<<|>>>?|==?|!=?|'
             r'=(?!>)|-(?!>)|[<>+*`%&\|\^/])=?',
             Operator, 'slashstartsregex'),
            (r'(?:\([^()]+\))?\s*[=-]>', Name.Function),
            (r'[{(\[;,]', Punctuation, 'slashstartsregex'),
            (r'[})\].]', Punctuation),
            (r'(?<![\.\$])(for|own|in|of|while|until|'
             r'loop|break|return|continue|'
             r'switch|when|then|if|unless|else|'
             r'throw|try|catch|finally|new|delete|typeof|instanceof|super|'
             r'extends|this|class|by)\b', Keyword, 'slashstartsregex'),
            (r'(?<![\.\$])(true|false|yes|no|on|off|null|'
             r'NaN|Infinity|undefined)\b',
             Keyword.Constant),
            (r'(Array|Boolean|Date|Error|Function|Math|netscape|'
             r'Number|Object|Packages|RegExp|String|sun|decodeURI|'
             r'decodeURIComponent|encodeURI|encodeURIComponent|'
             r'eval|isFinite|isNaN|parseFloat|parseInt|document|window)\b',
             Name.Builtin),
            (r'[$a-zA-Z_][a-zA-Z0-9_\.:\$]*\s*[:=]\s', Name.Variable,
              'slashstartsregex'),
            (r'@[$a-zA-Z_][a-zA-Z0-9_\.:\$]*\s*[:=]\s', Name.Variable.Instance,
              'slashstartsregex'),
            (r'@', Name.Other, 'slashstartsregex'),
            (r'@?[$a-zA-Z_][a-zA-Z0-9_\$]*', Name.Other, 'slashstartsregex'),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+', Number.Integer),
            ('"""', String, 'tdqs'),
            ("'''", String, 'tsqs'),
            ('"', String, 'dqs'),
            ("'", String, 'sqs'),
        ],
        'strings': [
            (r'[^#\\\'"]+', String),
            # note that all coffee script strings are multi-line.
            # hashmarks, quotes and backslashes must be parsed one at a time
        ],
        'interpoling_string' : [
            (r'}', String.Interpol, "#pop"),
            include('root')
        ],
        'dqs': [
            (r'"', String, '#pop'),
            (r'\\.|\'', String), # double-quoted string don't need ' escapes
            (r'#{', String.Interpol, "interpoling_string"),
            include('strings')
        ],
        'sqs': [
            (r"'", String, '#pop'),
            (r'#|\\.|"', String), # single quoted strings don't need " escapses
            include('strings')
        ],
        'tdqs': [
            (r'"""', String, '#pop'),
            (r'\\.|\'|"', String), # no need to escape quotes in triple-string
            (r'#{', String.Interpol, "interpoling_string"),
            include('strings'),
        ],
        'tsqs': [
            (r"'''", String, '#pop'),
            (r'#|\\.|\'|"', String), # no need to escape quotes in triple-strings
            include('strings')
        ],
    }


class LiveScriptLexer(RegexLexer):
    """
    For `LiveScript`_ source code.

    .. _LiveScript: http://gkz.github.com/LiveScript/

    New in Pygments 1.6.
    """

    name = 'LiveScript'
    aliases = ['live-script', 'livescript']
    filenames = ['*.ls']
    mimetypes = ['text/livescript']

    flags = re.DOTALL
    tokens = {
        'commentsandwhitespace': [
            (r'\s+', Text),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'#.*?\n', Comment.Single),
        ],
        'multilineregex': [
            include('commentsandwhitespace'),
            (r'//([gim]+\b|\B)', String.Regex, '#pop'),
            (r'/', String.Regex),
            (r'[^/#]+', String.Regex)
        ],
        'slashstartsregex': [
            include('commentsandwhitespace'),
            (r'//', String.Regex, ('#pop', 'multilineregex')),
            (r'/(?! )(\\.|[^[/\\\n]|\[(\\.|[^\]\\\n])*])+/'
             r'([gim]+\b|\B)', String.Regex, '#pop'),
            (r'', Text, '#pop'),
        ],
        'root': [
            # this next expr leads to infinite loops root -> slashstartsregex
            #(r'^(?=\s|/|<!--)', Text, 'slashstartsregex'),
            include('commentsandwhitespace'),
            (r'(?:\([^()]+\))?[ ]*[~-]{1,2}>|'
             r'(?:\(?[^()\n]+\)?)?[ ]*<[~-]{1,2}', Name.Function),
            (r'\+\+|&&|(?<![\.\$])\b(?:and|x?or|is|isnt|not)\b|\?|:|=|'
             r'\|\||\\(?=\n)|(<<|>>>?|==?|!=?|'
             r'~(?!\~?>)|-(?!\-?>)|<(?!\[)|(?<!\])>|'
             r'[+*`%&\|\^/])=?',
             Operator, 'slashstartsregex'),
            (r'[{(\[;,]', Punctuation, 'slashstartsregex'),
            (r'[})\].]', Punctuation),
            (r'(?<![\.\$])(for|own|in|of|while|until|loop|break|'
             r'return|continue|switch|when|then|if|unless|else|'
             r'throw|try|catch|finally|new|delete|typeof|instanceof|super|'
             r'extends|this|class|by|const|var|to|til)\b', Keyword,
              'slashstartsregex'),
            (r'(?<![\.\$])(true|false|yes|no|on|off|'
             r'null|NaN|Infinity|undefined|void)\b',
              Keyword.Constant),
            (r'(Array|Boolean|Date|Error|Function|Math|netscape|'
             r'Number|Object|Packages|RegExp|String|sun|decodeURI|'
             r'decodeURIComponent|encodeURI|encodeURIComponent|'
             r'eval|isFinite|isNaN|parseFloat|parseInt|document|window)\b',
              Name.Builtin),
            (r'[$a-zA-Z_][a-zA-Z0-9_\.\-:\$]*\s*[:=]\s', Name.Variable,
              'slashstartsregex'),
            (r'@[$a-zA-Z_][a-zA-Z0-9_\.\-:\$]*\s*[:=]\s', Name.Variable.Instance,
              'slashstartsregex'),
            (r'@', Name.Other, 'slashstartsregex'),
            (r'@?[$a-zA-Z_][a-zA-Z0-9_\-]*', Name.Other, 'slashstartsregex'),
            (r'[0-9]+\.[0-9]+([eE][0-9]+)?[fd]?(?:[a-zA-Z_]+)?', Number.Float),
            (r'[0-9]+(~[0-9a-z]+)?(?:[a-zA-Z_]+)?', Number.Integer),
            ('"""', String, 'tdqs'),
            ("'''", String, 'tsqs'),
            ('"', String, 'dqs'),
            ("'", String, 'sqs'),
            (r'\\[\w$-]+', String),
            (r'<\[.*\]>', String),
        ],
        'strings': [
            (r'[^#\\\'"]+', String),
            # note that all coffee script strings are multi-line.
            # hashmarks, quotes and backslashes must be parsed one at a time
        ],
        'interpoling_string' : [
            (r'}', String.Interpol, "#pop"),
            include('root')
        ],
        'dqs': [
            (r'"', String, '#pop'),
            (r'\\.|\'', String), # double-quoted string don't need ' escapes
            (r'#{', String.Interpol, "interpoling_string"),
            (r'#', String),
            include('strings')
        ],
        'sqs': [
            (r"'", String, '#pop'),
            (r'#|\\.|"', String), # single quoted strings don't need " escapses
            include('strings')
        ],
        'tdqs': [
            (r'"""', String, '#pop'),
            (r'\\.|\'|"', String), # no need to escape quotes in triple-string
            (r'#{', String.Interpol, "interpoling_string"),
            (r'#', String),
            include('strings'),
        ],
        'tsqs': [
            (r"'''", String, '#pop'),
            (r'#|\\.|\'|"', String), # no need to escape quotes in triple-strings
            include('strings')
        ],
    }


class DuelLexer(RegexLexer):
    """
    Lexer for Duel Views Engine (formerly JBST) markup with JavaScript code blocks.
    See http://duelengine.org/.
    See http://jsonml.org/jbst/.

    *New in Pygments 1.4.*
    """

    name = 'Duel'
    aliases = ['duel', 'Duel Engine', 'Duel View', 'JBST', 'jbst', 'JsonML+BST']
    filenames = ['*.duel','*.jbst']
    mimetypes = ['text/x-duel','text/x-jbst']

    flags = re.DOTALL

    tokens = {
        'root': [
            (r'(<%[@=#!:]?)(.*?)(%>)',
             bygroups(Name.Tag, using(JavascriptLexer), Name.Tag)),
            (r'(<%\$)(.*?)(:)(.*?)(%>)',
             bygroups(Name.Tag, Name.Function, Punctuation, String, Name.Tag)),
            (r'(<%--)(.*?)(--%>)',
             bygroups(Name.Tag, Comment.Multiline, Name.Tag)),
            (r'(<script.*?>)(.*?)(</script>)',
             bygroups(using(HtmlLexer),
                      using(JavascriptLexer), using(HtmlLexer))),
            (r'(.+?)(?=<)', using(HtmlLexer)),
            (r'.+', using(HtmlLexer)),
        ],
    }


class ScamlLexer(ExtendedRegexLexer):
    """
    For `Scaml markup <http://scalate.fusesource.org/>`_.  Scaml is Haml for Scala.

    *New in Pygments 1.4.*
    """

    name = 'Scaml'
    aliases = ['scaml', 'SCAML']
    filenames = ['*.scaml']
    mimetypes = ['text/x-scaml']

    flags = re.IGNORECASE
    # Scaml does not yet support the " |\n" notation to
    # wrap long lines.  Once it does, use the custom faux
    # dot instead.
    # _dot = r'(?: \|\n(?=.* \|)|.)'
    _dot = r'.'

    tokens = {
        'root': [
            (r'[ \t]*\n', Text),
            (r'[ \t]*', _indentation),
        ],

        'css': [
            (r'\.[a-z0-9_:-]+', Name.Class, 'tag'),
            (r'\#[a-z0-9_:-]+', Name.Function, 'tag'),
        ],

        'eval-or-plain': [
            (r'[&!]?==', Punctuation, 'plain'),
            (r'([&!]?[=~])(' + _dot + r'*\n)',
             bygroups(Punctuation, using(ScalaLexer)),
             'root'),
            (r'', Text, 'plain'),
        ],

        'content': [
            include('css'),
            (r'%[a-z0-9_:-]+', Name.Tag, 'tag'),
            (r'!!!' + _dot + r'*\n', Name.Namespace, '#pop'),
            (r'(/)(\[' + _dot + '*?\])(' + _dot + r'*\n)',
             bygroups(Comment, Comment.Special, Comment),
             '#pop'),
            (r'/' + _dot + r'*\n', _starts_block(Comment, 'html-comment-block'),
             '#pop'),
            (r'-#' + _dot + r'*\n', _starts_block(Comment.Preproc,
                                                 'scaml-comment-block'), '#pop'),
            (r'(-@\s*)(import)?(' + _dot + r'*\n)',
             bygroups(Punctuation, Keyword, using(ScalaLexer)),
             '#pop'),
            (r'(-)(' + _dot + r'*\n)',
             bygroups(Punctuation, using(ScalaLexer)),
             '#pop'),
            (r':' + _dot + r'*\n', _starts_block(Name.Decorator, 'filter-block'),
             '#pop'),
            include('eval-or-plain'),
        ],

        'tag': [
            include('css'),
            (r'\{(,\n|' + _dot + ')*?\}', using(ScalaLexer)),
            (r'\[' + _dot + '*?\]', using(ScalaLexer)),
            (r'\(', Text, 'html-attributes'),
            (r'/[ \t]*\n', Punctuation, '#pop:2'),
            (r'[<>]{1,2}(?=[ \t=])', Punctuation),
            include('eval-or-plain'),
        ],

        'plain': [
            (r'([^#\n]|#[^{\n]|(\\\\)*\\#\{)+', Text),
            (r'(#\{)(' + _dot + '*?)(\})',
             bygroups(String.Interpol, using(ScalaLexer), String.Interpol)),
            (r'\n', Text, 'root'),
        ],

        'html-attributes': [
            (r'\s+', Text),
            (r'[a-z0-9_:-]+[ \t]*=', Name.Attribute, 'html-attribute-value'),
            (r'[a-z0-9_:-]+', Name.Attribute),
            (r'\)', Text, '#pop'),
        ],

        'html-attribute-value': [
            (r'[ \t]+', Text),
            (r'[a-z0-9_]+', Name.Variable, '#pop'),
            (r'@[a-z0-9_]+', Name.Variable.Instance, '#pop'),
            (r'\$[a-z0-9_]+', Name.Variable.Global, '#pop'),
            (r"'(\\\\|\\'|[^'\n])*'", String, '#pop'),
            (r'"(\\\\|\\"|[^"\n])*"', String, '#pop'),
        ],

        'html-comment-block': [
            (_dot + '+', Comment),
            (r'\n', Text, 'root'),
        ],

        'scaml-comment-block': [
            (_dot + '+', Comment.Preproc),
            (r'\n', Text, 'root'),
        ],

        'filter-block': [
            (r'([^#\n]|#[^{\n]|(\\\\)*\\#\{)+', Name.Decorator),
            (r'(#\{)(' + _dot + '*?)(\})',
             bygroups(String.Interpol, using(ScalaLexer), String.Interpol)),
            (r'\n', Text, 'root'),
        ],
    }


class JadeLexer(ExtendedRegexLexer):
    """
    For Jade markup.
    Jade is a variant of Scaml, see:
    http://scalate.fusesource.org/documentation/scaml-reference.html

    *New in Pygments 1.4.*
    """

    name = 'Jade'
    aliases = ['jade', 'JADE']
    filenames = ['*.jade']
    mimetypes = ['text/x-jade']

    flags = re.IGNORECASE
    _dot = r'.'

    tokens = {
        'root': [
            (r'[ \t]*\n', Text),
            (r'[ \t]*', _indentation),
        ],

        'css': [
            (r'\.[a-z0-9_:-]+', Name.Class, 'tag'),
            (r'\#[a-z0-9_:-]+', Name.Function, 'tag'),
        ],

        'eval-or-plain': [
            (r'[&!]?==', Punctuation, 'plain'),
            (r'([&!]?[=~])(' + _dot + r'*\n)',
             bygroups(Punctuation, using(ScalaLexer)),  'root'),
            (r'', Text, 'plain'),
        ],

        'content': [
            include('css'),
            (r'!!!' + _dot + r'*\n', Name.Namespace, '#pop'),
            (r'(/)(\[' + _dot + '*?\])(' + _dot + r'*\n)',
             bygroups(Comment, Comment.Special, Comment),
             '#pop'),
            (r'/' + _dot + r'*\n', _starts_block(Comment, 'html-comment-block'),
             '#pop'),
            (r'-#' + _dot + r'*\n', _starts_block(Comment.Preproc,
                                                 'scaml-comment-block'), '#pop'),
            (r'(-@\s*)(import)?(' + _dot + r'*\n)',
             bygroups(Punctuation, Keyword, using(ScalaLexer)),
             '#pop'),
            (r'(-)(' + _dot + r'*\n)',
             bygroups(Punctuation, using(ScalaLexer)),
             '#pop'),
            (r':' + _dot + r'*\n', _starts_block(Name.Decorator, 'filter-block'),
             '#pop'),
            (r'[a-z0-9_:-]+', Name.Tag, 'tag'),
            (r'\|', Text, 'eval-or-plain'),
        ],

        'tag': [
            include('css'),
            (r'\{(,\n|' + _dot + ')*?\}', using(ScalaLexer)),
            (r'\[' + _dot + '*?\]', using(ScalaLexer)),
            (r'\(', Text, 'html-attributes'),
            (r'/[ \t]*\n', Punctuation, '#pop:2'),
            (r'[<>]{1,2}(?=[ \t=])', Punctuation),
            include('eval-or-plain'),
        ],

        'plain': [
            (r'([^#\n]|#[^{\n]|(\\\\)*\\#\{)+', Text),
            (r'(#\{)(' + _dot + '*?)(\})',
             bygroups(String.Interpol, using(ScalaLexer), String.Interpol)),
            (r'\n', Text, 'root'),
        ],

        'html-attributes': [
            (r'\s+', Text),
            (r'[a-z0-9_:-]+[ \t]*=', Name.Attribute, 'html-attribute-value'),
            (r'[a-z0-9_:-]+', Name.Attribute),
            (r'\)', Text, '#pop'),
        ],

        'html-attribute-value': [
            (r'[ \t]+', Text),
            (r'[a-z0-9_]+', Name.Variable, '#pop'),
            (r'@[a-z0-9_]+', Name.Variable.Instance, '#pop'),
            (r'\$[a-z0-9_]+', Name.Variable.Global, '#pop'),
            (r"'(\\\\|\\'|[^'\n])*'", String, '#pop'),
            (r'"(\\\\|\\"|[^"\n])*"', String, '#pop'),
        ],

        'html-comment-block': [
            (_dot + '+', Comment),
            (r'\n', Text, 'root'),
        ],

        'scaml-comment-block': [
            (_dot + '+', Comment.Preproc),
            (r'\n', Text, 'root'),
        ],

        'filter-block': [
            (r'([^#\n]|#[^{\n]|(\\\\)*\\#\{)+', Name.Decorator),
            (r'(#\{)(' + _dot + '*?)(\})',
             bygroups(String.Interpol, using(ScalaLexer), String.Interpol)),
            (r'\n', Text, 'root'),
        ],
    }


class XQueryLexer(ExtendedRegexLexer):
    """
    An XQuery lexer, parsing a stream and outputting the tokens needed to
    highlight xquery code.

    *New in Pygments 1.4.*
    """
    name = 'XQuery'
    aliases = ['xquery', 'xqy', 'xq', 'xql', 'xqm']
    filenames = ['*.xqy', '*.xquery', '*.xq', '*.xql', '*.xqm']
    mimetypes = ['text/xquery', 'application/xquery']

    xquery_parse_state = []

    # FIX UNICODE LATER
    #ncnamestartchar = (
    #    ur"[A-Z]|_|[a-z]|[\u00C0-\u00D6]|[\u00D8-\u00F6]|[\u00F8-\u02FF]|"
    #    ur"[\u0370-\u037D]|[\u037F-\u1FFF]|[\u200C-\u200D]|[\u2070-\u218F]|"
    #    ur"[\u2C00-\u2FEF]|[\u3001-\uD7FF]|[\uF900-\uFDCF]|[\uFDF0-\uFFFD]|"
    #    ur"[\u10000-\uEFFFF]"
    #)
    ncnamestartchar = r"(?:[A-Z]|_|[a-z])"
    # FIX UNICODE LATER
    #ncnamechar = ncnamestartchar + (ur"|-|\.|[0-9]|\u00B7|[\u0300-\u036F]|"
    #                                ur"[\u203F-\u2040]")
    ncnamechar = r"(?:" + ncnamestartchar + r"|-|\.|[0-9])"
    ncname = "(?:%s+%s*)" % (ncnamestartchar, ncnamechar)
    pitarget_namestartchar = r"(?:[A-KN-WY-Z]|_|:|[a-kn-wy-z])"
    pitarget_namechar = r"(?:" + pitarget_namestartchar + r"|-|\.|[0-9])"
    pitarget = "%s+%s*" % (pitarget_namestartchar, pitarget_namechar)
    prefixedname = "%s:%s" % (ncname, ncname)
    unprefixedname = ncname
    qname = "(?:%s|%s)" % (prefixedname, unprefixedname)

    entityref = r'(?:&(?:lt|gt|amp|quot|apos|nbsp);)'
    charref = r'(?:&#[0-9]+;|&#x[0-9a-fA-F]+;)'

    stringdouble = r'(?:"(?:' + entityref + r'|' + charref + r'|""|[^&"])*")'
    stringsingle = r"(?:'(?:" + entityref + r"|" + charref + r"|''|[^&'])*')"

    # FIX UNICODE LATER
    #elementcontentchar = (ur'\t|\r|\n|[\u0020-\u0025]|[\u0028-\u003b]|'
    #                      ur'[\u003d-\u007a]|\u007c|[\u007e-\u007F]')
    elementcontentchar = r'[A-Za-z]|\s|\d|[!"#$%\(\)\*\+,\-\./\:;=\?\@\[\\\]^_\'`\|~]'
    #quotattrcontentchar = (ur'\t|\r|\n|[\u0020-\u0021]|[\u0023-\u0025]|'
    #                       ur'[\u0027-\u003b]|[\u003d-\u007a]|\u007c|[\u007e-\u007F]')
    quotattrcontentchar = r'[A-Za-z]|\s|\d|[!#$%\(\)\*\+,\-\./\:;=\?\@\[\\\]^_\'`\|~]'
    #aposattrcontentchar = (ur'\t|\r|\n|[\u0020-\u0025]|[\u0028-\u003b]|'
    #                       ur'[\u003d-\u007a]|\u007c|[\u007e-\u007F]')
    aposattrcontentchar = r'[A-Za-z]|\s|\d|[!"#$%\(\)\*\+,\-\./\:;=\?\@\[\\\]^_`\|~]'


    # CHAR elements - fix the above elementcontentchar, quotattrcontentchar,
    #                 aposattrcontentchar
    #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]

    flags = re.DOTALL | re.MULTILINE | re.UNICODE

    def punctuation_root_callback(lexer, match, ctx):
        yield match.start(), Punctuation, match.group(1)
        # transition to root always - don't pop off stack
        ctx.stack = ['root']
        ctx.pos = match.end()

    def operator_root_callback(lexer, match, ctx):
        yield match.start(), Operator, match.group(1)
        # transition to root always - don't pop off stack
        ctx.stack = ['root']
        ctx.pos = match.end()

    def popstate_tag_callback(lexer, match, ctx):
        yield match.start(), Name.Tag, match.group(1)
        ctx.stack.append(lexer.xquery_parse_state.pop())
        ctx.pos = match.end()

    def popstate_xmlcomment_callback(lexer, match, ctx):
        yield match.start(), String.Doc, match.group(1)
        ctx.stack.append(lexer.xquery_parse_state.pop())
        ctx.pos = match.end()

    def popstate_kindtest_callback(lexer, match, ctx):
        yield match.start(), Punctuation, match.group(1)
        next_state = lexer.xquery_parse_state.pop()
        if next_state == 'occurrenceindicator':
            if re.match("[?*+]+", match.group(2)):
                yield match.start(), Punctuation, match.group(2)
                ctx.stack.append('operator')
                ctx.pos = match.end()
            else:
                ctx.stack.append('operator')
                ctx.pos = match.end(1)
        else:
            ctx.stack.append(next_state)
            ctx.pos = match.end(1)

    def popstate_callback(lexer, match, ctx):
        yield match.start(), Punctuation, match.group(1)
        # if we have run out of our state stack, pop whatever is on the pygments
        # state stack
        if len(lexer.xquery_parse_state) == 0:
            ctx.stack.pop()
        elif len(ctx.stack) > 1:
            ctx.stack.append(lexer.xquery_parse_state.pop())
        else:
            # i don't know if i'll need this, but in case, default back to root
            ctx.stack = ['root']
        ctx.pos = match.end()

    def pushstate_element_content_starttag_callback(lexer, match, ctx):
        yield match.start(), Name.Tag, match.group(1)
        lexer.xquery_parse_state.append('element_content')
        ctx.stack.append('start_tag')
        ctx.pos = match.end()

    def pushstate_cdata_section_callback(lexer, match, ctx):
        yield match.start(), String.Doc, match.group(1)
        ctx.stack.append('cdata_section')
        lexer.xquery_parse_state.append(ctx.state.pop)
        ctx.pos = match.end()

    def pushstate_starttag_callback(lexer, match, ctx):
        yield match.start(), Name.Tag, match.group(1)
        lexer.xquery_parse_state.append(ctx.state.pop)
        ctx.stack.append('start_tag')
        ctx.pos = match.end()

    def pushstate_operator_order_callback(lexer, match, ctx):
        yield match.start(), Keyword, match.group(1)
        yield match.start(), Text, match.group(2)
        yield match.start(), Punctuation, match.group(3)
        ctx.stack = ['root']
        lexer.xquery_parse_state.append('operator')
        ctx.pos = match.end()

    def pushstate_operator_root_validate(lexer, match, ctx):
        yield match.start(), Keyword, match.group(1)
        yield match.start(), Text, match.group(2)
        yield match.start(), Punctuation, match.group(3)
        ctx.stack = ['root']
        lexer.xquery_parse_state.append('operator')
        ctx.pos = match.end()

    def pushstate_operator_root_validate_withmode(lexer, match, ctx):
        yield match.start(), Keyword, match.group(1)
        yield match.start(), Text, match.group(2)
        yield match.start(), Keyword, match.group(3)
        ctx.stack = ['root']
        lexer.xquery_parse_state.append('operator')
        ctx.pos = match.end()

    def pushstate_operator_processing_instruction_callback(lexer, match, ctx):
        yield match.start(), String.Doc, match.group(1)
        ctx.stack.append('processing_instruction')
        lexer.xquery_parse_state.append('operator')
        ctx.pos = match.end()

    def pushstate_element_content_processing_instruction_callback(lexer, match, ctx):
        yield match.start(), String.Doc, match.group(1)
        ctx.stack.append('processing_instruction')
        lexer.xquery_parse_state.append('element_content')
        ctx.pos = match.end()

    def pushstate_element_content_cdata_section_callback(lexer, match, ctx):
        yield match.start(), String.Doc, match.group(1)
        ctx.stack.append('cdata_section')
        lexer.xquery_parse_state.append('element_content')
        ctx.pos = match.end()

    def pushstate_operator_cdata_section_callback(lexer, match, ctx):
        yield match.start(), String.Doc, match.group(1)
        ctx.stack.append('cdata_section')
        lexer.xquery_parse_state.append('operator')
        ctx.pos = match.end()

    def pushstate_element_content_xmlcomment_callback(lexer, match, ctx):
        yield match.start(), String.Doc, match.group(1)
        ctx.stack.append('xml_comment')
        lexer.xquery_parse_state.append('element_content')
        ctx.pos = match.end()

    def pushstate_operator_xmlcomment_callback(lexer, match, ctx):
        yield match.start(), String.Doc, match.group(1)
        ctx.stack.append('xml_comment')
        lexer.xquery_parse_state.append('operator')
        ctx.pos = match.end()

    def pushstate_kindtest_callback(lexer, match, ctx):
        yield match.start(), Keyword, match.group(1)
        yield match.start(), Text, match.group(2)
        yield match.start(), Punctuation, match.group(3)
        lexer.xquery_parse_state.append('kindtest')
        ctx.stack.append('kindtest')
        ctx.pos = match.end()

    def pushstate_operator_kindtestforpi_callback(lexer, match, ctx):
        yield match.start(), Keyword, match.group(1)
        yield match.start(), Text, match.group(2)
        yield match.start(), Punctuation, match.group(3)
        lexer.xquery_parse_state.append('operator')
        ctx.stack.append('kindtestforpi')
        ctx.pos = match.end()

    def pushstate_operator_kindtest_callback(lexer, match, ctx):
        yield match.start(), Keyword, match.group(1)
        yield match.start(), Text, match.group(2)
        yield match.start(), Punctuation, match.group(3)
        lexer.xquery_parse_state.append('operator')
        ctx.stack.append('kindtest')
        ctx.pos = match.end()

    def pushstate_occurrenceindicator_kindtest_callback(lexer, match, ctx):
        yield match.start(), Name.Tag, match.group(1)
        yield match.start(), Text, match.group(2)
        yield match.start(), Punctuation, match.group(3)
        lexer.xquery_parse_state.append('occurrenceindicator')
        ctx.stack.append('kindtest')
        ctx.pos = match.end()

    def pushstate_operator_starttag_callback(lexer, match, ctx):
        yield match.start(), Name.Tag, match.group(1)
        lexer.xquery_parse_state.append('operator')
        ctx.stack.append('start_tag')
        ctx.pos = match.end()

    def pushstate_operator_root_callback(lexer, match, ctx):
        yield match.start(), Punctuation, match.group(1)
        lexer.xquery_parse_state.append('operator')
        ctx.stack = ['root']#.append('root')
        ctx.pos = match.end()

    def pushstate_operator_root_construct_callback(lexer, match, ctx):
        yield match.start(), Keyword, match.group(1)
        yield match.start(), Text, match.group(2)
        yield match.start(), Punctuation, match.group(3)
        lexer.xquery_parse_state.append('operator')
        ctx.stack = ['root']
        ctx.pos = match.end()

    def pushstate_root_callback(lexer, match, ctx):
        yield match.start(), Punctuation, match.group(1)
        cur_state = ctx.stack.pop()
        lexer.xquery_parse_state.append(cur_state)
        ctx.stack = ['root']#.append('root')
        ctx.pos = match.end()

    def pushstate_operator_attribute_callback(lexer, match, ctx):
        yield match.start(), Name.Attribute, match.group(1)
        ctx.stack.append('operator')
        ctx.pos = match.end()

    def pushstate_operator_callback(lexer, match, ctx):
        yield match.start(), Keyword, match.group(1)
        yield match.start(), Text, match.group(2)
        yield match.start(), Punctuation, match.group(3)
        lexer.xquery_parse_state.append('operator')
        ctx.pos = match.end()

    tokens = {
        'comment': [
            # xquery comments
            (r'(:\))', Comment, '#pop'),
            (r'(\(:)', Comment, '#push'),
            (r'[^:)]', Comment),
            (r'([^:)]|:|\))', Comment),
        ],
        'whitespace': [
            (r'\s+', Text),
        ],
        'operator': [
            include('whitespace'),
            (r'(\})', popstate_callback),
            (r'\(:', Comment, 'comment'),

            (r'(\{)', pushstate_root_callback),
            (r'then|else|external|at|div|except', Keyword, 'root'),
            (r'order by', Keyword, 'root'),
            (r'is|mod|order\s+by|stable\s+order\s+by', Keyword, 'root'),
            (r'and|or', Operator.Word, 'root'),
            (r'(eq|ge|gt|le|lt|ne|idiv|intersect|in)(?=\b)',
             Operator.Word, 'root'),
            (r'return|satisfies|to|union|where|preserve\s+strip',
             Keyword, 'root'),
            (r'(>=|>>|>|<=|<<|<|-|\*|!=|\+|\||:=|=)',
             operator_root_callback),
            (r'(::|;|\[|//|/|,)',
             punctuation_root_callback),
            (r'(castable|cast)(\s+)(as)\b',
             bygroups(Keyword, Text, Keyword), 'singletype'),
            (r'(instance)(\s+)(of)\b',
             bygroups(Keyword, Text, Keyword), 'itemtype'),
            (r'(treat)(\s+)(as)\b',
             bygroups(Keyword, Text, Keyword), 'itemtype'),
            (r'(case|as)\b', Keyword, 'itemtype'),
            (r'(\))(\s*)(as)',
             bygroups(Punctuation, Text, Keyword), 'itemtype'),
            (r'\$', Name.Variable, 'varname'),
            (r'(for|let)(\s+)(\$)',
             bygroups(Keyword, Text, Name.Variable), 'varname'),
            #(r'\)|\?|\]', Punctuation, '#push'),
            (r'\)|\?|\]', Punctuation),
            (r'(empty)(\s+)(greatest|least)', bygroups(Keyword, Text, Keyword)),
            (r'ascending|descending|default', Keyword, '#push'),
            (r'external', Keyword),
            (r'collation', Keyword, 'uritooperator'),
            # finally catch all string literals and stay in operator state
            (stringdouble, String.Double),
            (stringsingle, String.Single),

            (r'(catch)(\s*)', bygroups(Keyword, Text), 'root'),
        ],
        'uritooperator': [
            (stringdouble, String.Double, '#pop'),
            (stringsingle, String.Single, '#pop'),
        ],
        'namespacedecl': [
            include('whitespace'),
            (r'\(:', Comment, 'comment'),
            (r'(at)(\s+)('+stringdouble+')', bygroups(Keyword, Text, String.Double)),
            (r"(at)(\s+)("+stringsingle+')', bygroups(Keyword, Text, String.Single)),
            (stringdouble, String.Double),
            (stringsingle, String.Single),
            (r',', Punctuation),
            (r'=', Operator),
            (r';', Punctuation, 'root'),
            (ncname, Name.Namespace),
        ],
        'namespacekeyword': [
            include('whitespace'),
            (r'\(:', Comment, 'comment'),
            (stringdouble, String.Double, 'namespacedecl'),
            (stringsingle, String.Single, 'namespacedecl'),
            (r'inherit|no-inherit', Keyword, 'root'),
            (r'namespace', Keyword, 'namespacedecl'),
            (r'(default)(\s+)(element)', bygroups(Keyword, Text, Keyword)),
            (r'preserve|no-preserve', Keyword),
            (r',', Punctuation),
        ],
        'varname': [
            (r'\(:', Comment, 'comment'),
            (qname, Name.Variable, 'operator'),
        ],
        'singletype': [
            (r'\(:', Comment, 'comment'),
            (ncname + r'(:\*)', Name.Variable, 'operator'),
            (qname, Name.Variable, 'operator'),
        ],
        'itemtype': [
            include('whitespace'),
            (r'\(:', Comment, 'comment'),
            (r'\$', Punctuation, 'varname'),
            (r'(void)(\s*)(\()(\s*)(\))',
             bygroups(Keyword, Text, Punctuation, Text, Punctuation), 'operator'),
            (r'(element|attribute|schema-element|schema-attribute|comment|text|'
             r'node|binary|document-node|empty-sequence)(\s*)(\()',
             pushstate_occurrenceindicator_kindtest_callback),
            # Marklogic specific type?
            (r'(processing-instruction)(\s*)(\()',
             bygroups(Keyword, Text, Punctuation),
             ('occurrenceindicator', 'kindtestforpi')),
            (r'(item)(\s*)(\()(\s*)(\))(?=[*+?])',
             bygroups(Keyword, Text, Punctuation, Text, Punctuation),
             'occurrenceindicator'),
            (r'\(\#', Punctuation, 'pragma'),
            (r';', Punctuation, '#pop'),
            (r'then|else', Keyword, '#pop'),
            (r'(at)(\s+)(' + stringdouble + ')',
             bygroups(Keyword, Text, String.Double), 'namespacedecl'),
            (r'(at)(\s+)(' + stringsingle + ')',
             bygroups(Keyword, Text, String.Single), 'namespacedecl'),
            (r'except|intersect|in|is|return|satisfies|to|union|where',
             Keyword, 'root'),
            (r'and|div|eq|ge|gt|le|lt|ne|idiv|mod|or', Operator.Word, 'root'),
            (r':=|=|,|>=|>>|>|\[|\(|<=|<<|<|-|!=|\|', Operator, 'root'),
            (r'external|at', Keyword, 'root'),
            (r'(stable)(\s+)(order)(\s+)(by)',
             bygroups(Keyword, Text, Keyword, Text, Keyword), 'root'),
            (r'(castable|cast)(\s+)(as)',
             bygroups(Keyword, Text, Keyword), 'singletype'),
            (r'(treat)(\s+)(as)', bygroups(Keyword, Text, Keyword)),
            (r'(instance)(\s+)(of)', bygroups(Keyword, Text, Keyword)),
            (r'case|as', Keyword, 'itemtype'),
            (r'(\))(\s*)(as)', bygroups(Operator, Text, Keyword), 'itemtype'),
            (ncname + r':\*', Keyword.Type, 'operator'),
            (qname, Keyword.Type, 'occurrenceindicator'),
        ],
        'kindtest': [
            (r'\(:', Comment, 'comment'),
            (r'{', Punctuation, 'root'),
            (r'(\))([*+?]?)', popstate_kindtest_callback),
            (r'\*', Name, 'closekindtest'),
            (qname, Name, 'closekindtest'),
            (r'(element|schema-element)(\s*)(\()', pushstate_kindtest_callback),
        ],
        'kindtestforpi': [
            (r'\(:', Comment, 'comment'),
            (r'\)', Punctuation, '#pop'),
            (ncname, Name.Variable),
            (stringdouble, String.Double),
            (stringsingle, String.Single),
        ],
        'closekindtest': [
            (r'\(:', Comment, 'comment'),
            (r'(\))', popstate_callback),
            (r',', Punctuation),
            (r'(\{)', pushstate_operator_root_callback),
            (r'\?', Punctuation),
        ],
        'xml_comment': [
            (r'(-->)', popstate_xmlcomment_callback),
            (r'[^-]{1,2}', Literal),
            (ur'\t|\r|\n|[\u0020-\uD7FF]|[\uE000-\uFFFD]|' +
             unirange(0x10000, 0x10ffff), Literal),
        ],
        'processing_instruction': [
            (r'\s+', Text, 'processing_instruction_content'),
            (r'\?>', String.Doc, '#pop'),
            (pitarget, Name),
        ],
        'processing_instruction_content': [
            (r'\?>', String.Doc, '#pop'),
            (ur'\t|\r|\n|[\u0020-\uD7FF]|[\uE000-\uFFFD]|' +
             unirange(0x10000, 0x10ffff), Literal),
        ],
        'cdata_section': [
            (r']]>', String.Doc, '#pop'),
            (ur'\t|\r|\n|[\u0020-\uD7FF]|[\uE000-\uFFFD]|' +
             unirange(0x10000, 0x10ffff), Literal),
        ],
        'start_tag': [
            include('whitespace'),
            (r'(/>)', popstate_tag_callback),
            (r'>', Name.Tag, 'element_content'),
            (r'"', Punctuation, 'quot_attribute_content'),
            (r"'", Punctuation, 'apos_attribute_content'),
            (r'=', Operator),
            (qname, Name.Tag),
        ],
        'quot_attribute_content': [
            (r'"', Punctuation, 'start_tag'),
            (r'(\{)', pushstate_root_callback),
            (r'""', Name.Attribute),
            (quotattrcontentchar, Name.Attribute),
            (entityref, Name.Attribute),
            (charref, Name.Attribute),
            (r'\{\{|\}\}', Name.Attribute),
        ],
        'apos_attribute_content': [
            (r"'", Punctuation, 'start_tag'),
            (r'\{', Punctuation, 'root'),
            (r"''", Name.Attribute),
            (aposattrcontentchar, Name.Attribute),
            (entityref, Name.Attribute),
            (charref, Name.Attribute),
            (r'\{\{|\}\}', Name.Attribute),
        ],
        'element_content': [
            (r'</', Name.Tag, 'end_tag'),
            (r'(\{)', pushstate_root_callback),
            (r'(<!--)', pushstate_element_content_xmlcomment_callback),
            (r'(<\?)', pushstate_element_content_processing_instruction_callback),
            (r'(<!\[CDATA\[)', pushstate_element_content_cdata_section_callback),
            (r'(<)', pushstate_element_content_starttag_callback),
            (elementcontentchar, Literal),
            (entityref, Literal),
            (charref, Literal),
            (r'\{\{|\}\}', Literal),
        ],
        'end_tag': [
            include('whitespace'),
            (r'(>)', popstate_tag_callback),
            (qname, Name.Tag),
        ],
        'xmlspace_decl': [
            (r'\(:', Comment, 'comment'),
            (r'preserve|strip', Keyword, '#pop'),
        ],
        'declareordering': [
            (r'\(:', Comment, 'comment'),
            include('whitespace'),
            (r'ordered|unordered', Keyword, '#pop'),
        ],
        'xqueryversion': [
            include('whitespace'),
            (r'\(:', Comment, 'comment'),
            (stringdouble, String.Double),
            (stringsingle, String.Single),
            (r'encoding', Keyword),
            (r';', Punctuation, '#pop'),
        ],
        'pragma': [
            (qname, Name.Variable, 'pragmacontents'),
        ],
        'pragmacontents': [
            (r'#\)', Punctuation, 'operator'),
            (ur'\t|\r|\n|[\u0020-\uD7FF]|[\uE000-\uFFFD]|' +
             unirange(0x10000, 0x10ffff), Literal),
            (r'(\s+)', Text),
        ],
        'occurrenceindicator': [
            include('whitespace'),
            (r'\(:', Comment, 'comment'),
            (r'\*|\?|\+', Operator, 'operator'),
            (r':=', Operator, 'root'),
            (r'', Text, 'operator'),
        ],
        'option': [
            include('whitespace'),
            (qname, Name.Variable, '#pop'),
        ],
        'qname_braren': [
            include('whitespace'),
            (r'(\{)', pushstate_operator_root_callback),
            (r'(\()', Punctuation, 'root'),
        ],
        'element_qname': [
            (qname, Name.Variable, 'root'),
        ],
        'attribute_qname': [
            (qname, Name.Variable, 'root'),
        ],
        'root': [
            include('whitespace'),
            (r'\(:', Comment, 'comment'),

            # handle operator state
            # order on numbers matters - handle most complex first
            (r'\d+(\.\d*)?[eE][\+\-]?\d+', Number.Double, 'operator'),
            (r'(\.\d+)[eE][\+\-]?\d+', Number.Double, 'operator'),
            (r'(\.\d+|\d+\.\d*)', Number, 'operator'),
            (r'(\d+)', Number.Integer, 'operator'),
            (r'(\.\.|\.|\))', Punctuation, 'operator'),
            (r'(declare)(\s+)(construction)',
             bygroups(Keyword, Text, Keyword), 'operator'),
            (r'(declare)(\s+)(default)(\s+)(order)',
             bygroups(Keyword, Text, Keyword, Text, Keyword), 'operator'),
            (ncname + ':\*', Name, 'operator'),
            ('\*:'+ncname, Name.Tag, 'operator'),
            ('\*', Name.Tag, 'operator'),
            (stringdouble, String.Double, 'operator'),
            (stringsingle, String.Single, 'operator'),

            (r'(\})', popstate_callback),

            #NAMESPACE DECL
            (r'(declare)(\s+)(default)(\s+)(collation)',
             bygroups(Keyword, Text, Keyword, Text, Keyword)),
            (r'(module|declare)(\s+)(namespace)',
             bygroups(Keyword, Text, Keyword), 'namespacedecl'),
            (r'(declare)(\s+)(base-uri)',
             bygroups(Keyword, Text, Keyword), 'namespacedecl'),

            #NAMESPACE KEYWORD
            (r'(declare)(\s+)(default)(\s+)(element|function)',
             bygroups(Keyword, Text, Keyword, Text, Keyword), 'namespacekeyword'),
            (r'(import)(\s+)(schema|module)',
             bygroups(Keyword.Pseudo, Text, Keyword.Pseudo), 'namespacekeyword'),
            (r'(declare)(\s+)(copy-namespaces)',
             bygroups(Keyword, Text, Keyword), 'namespacekeyword'),

            #VARNAMEs
            (r'(for|let|some|every)(\s+)(\$)',
             bygroups(Keyword, Text, Name.Variable), 'varname'),
            (r'\$', Name.Variable, 'varname'),
            (r'(declare)(\s+)(variable)(\s+)(\$)',
             bygroups(Keyword, Text, Keyword, Text, Name.Variable), 'varname'),

            #ITEMTYPE
            (r'(\))(\s+)(as)', bygroups(Operator, Text, Keyword), 'itemtype'),

            (r'(element|attribute|schema-element|schema-attribute|comment|'
             r'text|node|document-node|empty-sequence)(\s+)(\()',
             pushstate_operator_kindtest_callback),

            (r'(processing-instruction)(\s+)(\()',
             pushstate_operator_kindtestforpi_callback),

            (r'(<!--)', pushstate_operator_xmlcomment_callback),

            (r'(<\?)', pushstate_operator_processing_instruction_callback),

            (r'(<!\[CDATA\[)', pushstate_operator_cdata_section_callback),

            # (r'</', Name.Tag, 'end_tag'),
            (r'(<)', pushstate_operator_starttag_callback),

            (r'(declare)(\s+)(boundary-space)',
             bygroups(Keyword, Text, Keyword), 'xmlspace_decl'),

            (r'(validate)(\s+)(lax|strict)',
             pushstate_operator_root_validate_withmode),
            (r'(validate)(\s*)(\{)', pushstate_operator_root_validate),
            (r'(typeswitch)(\s*)(\()', bygroups(Keyword, Text, Punctuation)),
            (r'(element|attribute)(\s*)(\{)',
             pushstate_operator_root_construct_callback),

            (r'(document|text|processing-instruction|comment)(\s*)(\{)',
             pushstate_operator_root_construct_callback),
            #ATTRIBUTE
            (r'(attribute)(\s+)(?=' + qname + r')',
             bygroups(Keyword, Text), 'attribute_qname'),
            #ELEMENT
            (r'(element)(\s+)(?=' +qname+ r')',
             bygroups(Keyword, Text), 'element_qname'),
            #PROCESSING_INSTRUCTION
            (r'(processing-instruction)(\s+)(' + ncname + r')(\s*)(\{)',
             bygroups(Keyword, Text, Name.Variable, Text, Punctuation),
             'operator'),

            (r'(declare|define)(\s+)(function)',
             bygroups(Keyword, Text, Keyword)),

            (r'(\{)', pushstate_operator_root_callback),

            (r'(unordered|ordered)(\s*)(\{)',
             pushstate_operator_order_callback),

            (r'(declare)(\s+)(ordering)',
             bygroups(Keyword, Text, Keyword), 'declareordering'),

            (r'(xquery)(\s+)(version)',
             bygroups(Keyword.Pseudo, Text, Keyword.Pseudo), 'xqueryversion'),

            (r'(\(#)', Punctuation, 'pragma'),

            # sometimes return can occur in root state
            (r'return', Keyword),

            (r'(declare)(\s+)(option)', bygroups(Keyword, Text, Keyword),
             'option'),

            #URI LITERALS - single and double quoted
            (r'(at)(\s+)('+stringdouble+')', String.Double, 'namespacedecl'),
            (r'(at)(\s+)('+stringsingle+')', String.Single, 'namespacedecl'),

            (r'(ancestor-or-self|ancestor|attribute|child|descendant-or-self)(::)',
             bygroups(Keyword, Punctuation)),
            (r'(descendant|following-sibling|following|parent|preceding-sibling'
             r'|preceding|self)(::)', bygroups(Keyword, Punctuation)),

            (r'(if)(\s*)(\()', bygroups(Keyword, Text, Punctuation)),

            (r'then|else', Keyword),

            # ML specific
            (r'(try)(\s*)', bygroups(Keyword, Text), 'root'),
            (r'(catch)(\s*)(\()(\$)',
             bygroups(Keyword, Text, Punctuation, Name.Variable), 'varname'),

            (r'(@'+qname+')', Name.Attribute),
            (r'(@'+ncname+')', Name.Attribute),
            (r'@\*:'+ncname, Name.Attribute),
            (r'(@)', Name.Attribute),

            (r'//|/|\+|-|;|,|\(|\)', Punctuation),

            # STANDALONE QNAMES
            (qname + r'(?=\s*{)', Name.Tag, 'qname_braren'),
            (qname + r'(?=\s*\([^:])', Name.Function, 'qname_braren'),
            (qname, Name.Tag, 'operator'),
        ]
    }


class DartLexer(RegexLexer):
    """
    For `Dart <http://dartlang.org/>`_ source code.

    *New in Pygments 1.5.*
    """

    name = 'Dart'
    aliases = ['dart']
    filenames = ['*.dart']
    mimetypes = ['text/x-dart']

    flags = re.MULTILINE | re.DOTALL

    tokens = {
        'root': [
            include('string_literal'),
            (r'#!(.*?)$', Comment.Preproc),
            (r'\b(import|export)\b', Keyword, 'import_decl'),
            (r'\b(library|source|part of|part)\b', Keyword),
            (r'[^\S\n]+', Text),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'\b(class)\b(\s+)',
             bygroups(Keyword.Declaration, Text), 'class'),
            (r'\b(assert|break|case|catch|continue|default|do|else|finally|for|'
             r'if|in|is|new|return|super|switch|this|throw|try|while)\b',
             Keyword),
            (r'\b(abstract|const|extends|factory|final|get|implements|'
             r'native|operator|set|static|typedef|var)\b', Keyword.Declaration),
            (r'\b(bool|double|Dynamic|int|num|Object|String|void)\b', Keyword.Type),
            (r'\b(false|null|true)\b', Keyword.Constant),
            (r'[~!%^&*+=|?:<>/-]|as', Operator),
            (r'[a-zA-Z_$][a-zA-Z0-9_]*:', Name.Label),
            (r'[a-zA-Z_$][a-zA-Z0-9_]*', Name),
            (r'[(){}\[\],.;]', Punctuation),
            (r'0[xX][0-9a-fA-F]+', Number.Hex),
            # DIGIT+ (‘.’ DIGIT*)? EXPONENT?
            (r'\d+(\.\d*)?([eE][+-]?\d+)?', Number),
            (r'\.\d+([eE][+-]?\d+)?', Number), # ‘.’ DIGIT+ EXPONENT?
            (r'\n', Text)
            # pseudo-keyword negate intentionally left out
        ],
        'class': [
            (r'[a-zA-Z_$][a-zA-Z0-9_]*', Name.Class, '#pop')
        ],
        'import_decl': [
            include('string_literal'),
            (r'\s+', Text),
            (r'\b(as|show|hide)\b', Keyword),
            (r'[a-zA-Z_$][a-zA-Z0-9_]*', Name),
            (r'\,', Punctuation),
            (r'\;', Punctuation, '#pop')
        ],
        'string_literal': [
            # Raw strings.
            (r'r"""([\s|\S]*?)"""', String.Double),
            (r"r'''([\s|\S]*?)'''", String.Single),
            (r'r"(.*?)"', String.Double),
            (r"r'(.*?)'", String.Single),
            # Normal Strings.
            (r'"""', String.Double, 'string_double_multiline'),
            (r"'''", String.Single, 'string_single_multiline'),
            (r'"', String.Double, 'string_double'),
            (r"'", String.Single, 'string_single')
        ],
        'string_common': [
            (r"\\(x[0-9A-Fa-f]{2}|u[0-9A-Fa-f]{4}|u\{[0-9A-Fa-f]*\}|[a-z\'\"$\\])",
             String.Escape),
            (r'(\$)([a-zA-Z_][a-zA-Z0-9_]*)', bygroups(String.Interpol, Name)),
            (r'(\$\{)(.*?)(\})',
             bygroups(String.Interpol, using(this), String.Interpol))
        ],
        'string_double': [
            (r'"', String.Double, '#pop'),
            (r'[^\"$\\\n]+', String.Double),
            include('string_common'),
            (r'\$+', String.Double)
        ],
        'string_double_multiline': [
            (r'"""', String.Double, '#pop'),
            (r'[^\"$\\]+', String.Double),
            include('string_common'),
            (r'(\$|\")+', String.Double)
        ],
        'string_single': [
            (r"'", String.Single, '#pop'),
            (r"[^\'$\\\n]+", String.Single),
            include('string_common'),
            (r'\$+', String.Single)
        ],
        'string_single_multiline': [
            (r"'''", String.Single, '#pop'),
            (r'[^\'$\\]+', String.Single),
            include('string_common'),
            (r'(\$|\')+', String.Single)
        ]
    }


class TypeScriptLexer(RegexLexer):
    """
    For `TypeScript <http://www.python.org>`_ source code.

    *New in Pygments 1.6.*
    """

    name = 'TypeScript'
    aliases = ['ts']
    filenames = ['*.ts']
    mimetypes = ['text/x-typescript']

    flags = re.DOTALL
    tokens = {
        'commentsandwhitespace': [
            (r'\s+', Text),
            (r'<!--', Comment),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline)
        ],
        'slashstartsregex': [
            include('commentsandwhitespace'),
            (r'/(\\.|[^[/\\\n]|\[(\\.|[^\]\\\n])*])+/'
             r'([gim]+\b|\B)', String.Regex, '#pop'),
            (r'(?=/)', Text, ('#pop', 'badregex')),
            (r'', Text, '#pop')
        ],
        'badregex': [
            (r'\n', Text, '#pop')
        ],
        'root': [
            (r'^(?=\s|/|<!--)', Text, 'slashstartsregex'),
            include('commentsandwhitespace'),
            (r'\+\+|--|~|&&|\?|:|\|\||\\(?=\n)|'
             r'(<<|>>>?|==?|!=?|[-<>+*%&\|\^/])=?', Operator, 'slashstartsregex'),
            (r'[{(\[;,]', Punctuation, 'slashstartsregex'),
            (r'[})\].]', Punctuation),
            (r'(for|in|while|do|break|return|continue|switch|case|default|if|else|'
             r'throw|try|catch|finally|new|delete|typeof|instanceof|void|'
             r'this)\b', Keyword, 'slashstartsregex'),
            (r'(var|let|with|function)\b', Keyword.Declaration, 'slashstartsregex'),
            (r'(abstract|boolean|byte|char|class|const|debugger|double|enum|export|'
             r'extends|final|float|goto|implements|import|int|interface|long|native|'
             r'package|private|protected|public|short|static|super|synchronized|throws|'
             r'transient|volatile)\b', Keyword.Reserved),
            (r'(true|false|null|NaN|Infinity|undefined)\b', Keyword.Constant),
            (r'(Array|Boolean|Date|Error|Function|Math|netscape|'
             r'Number|Object|Packages|RegExp|String|sun|decodeURI|'
             r'decodeURIComponent|encodeURI|encodeURIComponent|'
             r'Error|eval|isFinite|isNaN|parseFloat|parseInt|document|this|'
             r'window)\b', Name.Builtin),
            # Match stuff like: module name {...}
            (r'\b(module)(\s*)(\s*[a-zA-Z0-9_?.$][\w?.$]*)(\s*)',
             bygroups(Keyword.Reserved, Text, Name.Other, Text), 'slashstartsregex'),
            # Match variable type keywords
            (r'\b(string|bool|number)\b', Keyword.Type),
            # Match stuff like: constructor
            (r'\b(constructor|declare|interface|as|AS)\b', Keyword.Reserved),
            # Match stuff like: super(argument, list)
            (r'(super)(\s*)(\([a-zA-Z0-9,_?.$\s]+\s*\))',
             bygroups(Keyword.Reserved, Text), 'slashstartsregex'),
            # Match stuff like: function() {...}
            (r'([a-zA-Z_?.$][\w?.$]*)\(\) \{', Name.Other, 'slashstartsregex'),
            # Match stuff like: (function: return type)
            (r'([a-zA-Z0-9_?.$][\w?.$]*)(\s*:\s*)([a-zA-Z0-9_?.$][\w?.$]*)',
             bygroups(Name.Other, Text, Keyword.Type)),
            (r'[$a-zA-Z_][a-zA-Z0-9_]*', Name.Other),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+', Number.Integer),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
        ]
    }


class LassoLexer(RegexLexer):
    """
    For `Lasso <http://www.lassosoft.com/>`_ source code, covering both Lasso 9
    syntax and LassoScript for Lasso 8.6 and earlier. For Lasso embedded in
    HTML, use the `LassoHtmlLexer`.

    Additional options accepted:

    `builtinshighlighting`
        If given and ``True``, highlight builtin tags, types, traits, and
        methods (default: ``True``).
    `requiredelimiters`
        If given and ``True``, only highlight code between delimiters as Lasso
        (default: ``False``).

    *New in Pygments 1.6.*
    """

    name = 'Lasso'
    aliases = ['lasso', 'lassoscript']
    filenames = ['*.lasso', '*.lasso[89]']
    alias_filenames = ['*.incl', '*.inc', '*.las']
    mimetypes = ['text/x-lasso']
    flags = re.IGNORECASE | re.DOTALL | re.MULTILINE

    tokens = {
        'root': [
            (r'^#!.+lasso9\b', Comment.Preproc, 'lasso'),
            (r'\[no_square_brackets\]', Comment.Preproc, 'nosquarebrackets'),
            (r'\[noprocess\]', Comment.Preproc, ('delimiters', 'noprocess')),
            (r'\[', Comment.Preproc, ('delimiters', 'squarebrackets')),
            (r'<\?(LassoScript|lasso|=)', Comment.Preproc,
                ('delimiters', 'anglebrackets')),
            (r'<', Other, 'delimiters'),
            (r'\s+', Other),
            (r'', Other, ('delimiters', 'lassofile')),
        ],
        'delimiters': [
            (r'\[no_square_brackets\]', Comment.Preproc, 'nosquarebrackets'),
            (r'\[noprocess\]', Comment.Preproc, 'noprocess'),
            (r'\[', Comment.Preproc, 'squarebrackets'),
            (r'<\?(LassoScript|lasso|=)', Comment.Preproc, 'anglebrackets'),
            (r'<', Other),
            (r'[^[<]+', Other),
        ],
        'nosquarebrackets': [
            (r'<\?(LassoScript|lasso|=)', Comment.Preproc, 'anglebrackets'),
            (r'<', Other),
            (r'[^<]+', Other),
        ],
        'noprocess': [
            (r'\[/noprocess\]', Comment.Preproc, '#pop'),
            (r'\[', Other),
            (r'[^[]', Other),
        ],
        'squarebrackets': [
            (r'\]', Comment.Preproc, '#pop'),
            include('lasso'),
        ],
        'anglebrackets': [
            (r'\?>', Comment.Preproc, '#pop'),
            include('lasso'),
        ],
        'lassofile': [
            (r'\]', Comment.Preproc, '#pop'),
            (r'\?>', Comment.Preproc, '#pop'),
            include('lasso'),
        ],
        'whitespacecomments': [
            (r'\s+', Text),
            (r'//.*?\n', Comment.Single),
            (r'/\*\*!.*?\*/', String.Doc),
            (r'/\*.*?\*/', Comment.Multiline),
        ],
        'lasso': [
            # whitespace/comments
            include('whitespacecomments'),

            # literals
            (r'\d*\.\d+(e[+-]?\d+)?', Number.Float),
            (r'0x[\da-f]+', Number.Hex),
            (r'\d+', Number.Integer),
            (r'([+-]?)(infinity|NaN)\b', bygroups(Operator, Number)),
            (r"'", String.Single, 'singlestring'),
            (r'"', String.Double, 'doublestring'),
            (r'`[^`]*`', String.Backtick),

            # names
            (r'\$[a-z_][\w.]*', Name.Variable),
            (r'#[a-z_][\w.]*|#\d+', Name.Variable.Instance),
            (r"(\.)('[a-z_][\w.]*')",
                bygroups(Name.Builtin.Pseudo, Name.Variable.Class)),
            (r"(self)(->)('[a-z_][\w.]*')",
                bygroups(Name.Builtin.Pseudo, Operator, Name.Variable.Class)),
            (r'(\.\.?)([a-z_][\w.]*)',
                bygroups(Name.Builtin.Pseudo, Name.Other)),
            (r'(self|inherited|global|void)\b', Name.Builtin.Pseudo),
            (r'-[a-z_][\w.]*', Name.Attribute),
            (r'(::\s*)([a-z_][\w.]*)', bygroups(Punctuation, Name.Label)),
            (r'(error_(code|msg)_\w+|Error_AddError|Error_ColumnRestriction|'
             r'Error_DatabaseConnectionUnavailable|Error_DatabaseTimeout|'
             r'Error_DeleteError|Error_FieldRestriction|Error_FileNotFound|'
             r'Error_InvalidDatabase|Error_InvalidPassword|'
             r'Error_InvalidUsername|Error_ModuleNotFound|'
             r'Error_NoError|Error_NoPermission|Error_OutOfMemory|'
             r'Error_ReqColumnMissing|Error_ReqFieldMissing|'
             r'Error_RequiredColumnMissing|Error_RequiredFieldMissing|'
             r'Error_UpdateError)\b', Name.Exception),

            # definitions
            (r'(define)(\s+)([a-z_][\w.]*)(\s*)(=>)(\s*)(type|trait|thread)\b',
                bygroups(Keyword.Declaration, Text, Name.Class, Text, Operator,
                         Text, Keyword)),
            (r'(define)(\s+)([a-z_][\w.]*)(->)([a-z_][\w.]*=?|[-+*/%<>]|==)',
                bygroups(Keyword.Declaration, Text, Name.Class, Operator,
                         Name.Function), 'signature'),
            (r'(define)(\s+)([a-z_][\w.]*)',
                bygroups(Keyword.Declaration, Text, Name.Function),
                'signature'),
            (r'(public|protected|private|provide)(\s+)(([a-z_][\w.]*=?|'
             r'[-+*/%<>]|==)(?=\s*\())', bygroups(Keyword, Text, Name.Function),
                'signature'),
            (r'(public|protected|private)(\s+)([a-z_][\w.]*)',
                bygroups(Keyword, Text, Name.Function)),

            # keywords
            (r'(true|false|none|minimal|full|all)\b', Keyword.Constant),
            (r'(local|var|variable|data)\b', Keyword.Declaration),
            (r'(array|date|decimal|duration|integer|map|pair|string|tag|xml|'
             r'null)\b', Keyword.Type),
            (r'([a-z_][\w.]*)(\s+)(in)\b', bygroups(Name, Text, Keyword)),
            (r'(let|into)(\s+)([a-z_][\w.]*)', bygroups(Keyword, Text, Name)),
            (r'require\b', Keyword, 'requiresection'),
            (r'(/?)(Namespace_Using)\b',
                bygroups(Punctuation, Keyword.Namespace)),
            (r'(/?)(Cache|Database_Names|Database_SchemaNames|'
             r'Database_TableNames|Define_Tag|Define_Type|Email_Batch|'
             r'Encode_Set|HTML_Comment|Handle|Handle_Error|Header|If|Inline|'
             r'Iterate|LJAX_Target|Link|Link_CurrentAction|Link_CurrentGroup|'
             r'Link_CurrentRecord|Link_Detail|Link_FirstGroup|'
             r'Link_FirstRecord|Link_LastGroup|Link_LastRecord|Link_NextGroup|'
             r'Link_NextRecord|Link_PrevGroup|Link_PrevRecord|Log|Loop|'
             r'NoProcess|Output_None|Portal|Private|Protect|Records|Referer|'
             r'Referrer|Repeating|ResultSet|Rows|Search_Args|Search_Arguments|'
             r'Select|Sort_Args|Sort_Arguments|Thread_Atomic|Value_List|While|'
             r'Abort|Case|Else|If_Empty|If_False|If_Null|If_True|Loop_Abort|'
             r'Loop_Continue|Loop_Count|Params|Params_Up|Return|Return_Value|'
             r'Run_Children|SOAP_DefineTag|SOAP_LastRequest|SOAP_LastResponse|'
             r'Tag_Name|ascending|average|by|define|descending|do|equals|'
             r'frozen|group|handle_failure|import|in|into|join|let|match|max|'
             r'min|on|order|parent|protected|provide|public|require|skip|'
             r'split_thread|sum|take|thread|to|trait|type|where|with|yield)\b',
                 bygroups(Punctuation, Keyword)),

            # other
            (r'(([a-z_][\w.]*=?|[-+*/%<>]|==)(?=\s*\([^)]*\)\s*=>))',
                Name.Function, 'signature'),
            (r'(and|or|not)\b', Operator.Word),
            (r'([a-z_][\w.]*)(\s*)(::\s*)([a-z_][\w.]*)(\s*)(=)',
                bygroups(Name, Text, Punctuation, Name.Label, Text, Operator)),
            (r'((?<!->)[a-z_][\w.]*)(\s*)(=(?!=))',
                bygroups(Name, Text, Operator)),
            (r'(/?)([\w.]+)', bygroups(Punctuation, Name.Other)),
            (r'(=)(bw|ew|cn|lte?|gte?|n?eq|ft|n?rx)\b',
                bygroups(Operator, Operator.Word)),
            (r':=|[-+*/%=<>&|!?\\]+', Operator),
            (r'[{}():;,@^]', Punctuation),
        ],
        'singlestring': [
            (r"'", String.Single, '#pop'),
            (r"[^'\\]+", String.Single),
            include('escape'),
            (r"\\+", String.Single),
        ],
        'doublestring': [
            (r'"', String.Double, '#pop'),
            (r'[^"\\]+', String.Double),
            include('escape'),
            (r'\\+', String.Double),
        ],
        'escape': [
            (r'\\(U[\da-f]{8}|u[\da-f]{4}|x[\da-f]{1,2}|[0-7]{1,3}|:[^:]+:|'
             r'[abefnrtv?\"\'\\]|$)', String.Escape),
        ],
        'signature': [
            (r'=>', Operator, '#pop'),
            (r'\)', Punctuation, '#pop'),
            (r'[(,]', Punctuation, 'parameter'),
            include('lasso'),
        ],
        'parameter': [
            (r'\)', Punctuation, '#pop'),
            (r'-?[a-z_][\w.]*', Name.Attribute, '#pop'),
            (r'\.\.\.', Name.Builtin.Pseudo),
            include('lasso'),
        ],
        'requiresection': [
            (r'(([a-z_][\w.]*=?|[-+*/%<>]|==)(?=\s*\())', Name, 'requiresignature'),
            (r'(([a-z_][\w.]*=?|[-+*/%<>]|==)(?=(\s*::\s*[\w.]+)?\s*,))', Name),
            (r'[a-z_][\w.]*=?|[-+*/%<>]|==', Name, '#pop'),
            (r'(::\s*)([a-z_][\w.]*)', bygroups(Punctuation, Name.Label)),
            (r',', Punctuation),
            include('whitespacecomments'),
        ],
        'requiresignature': [
            (r'(\)(?=(\s*::\s*[\w.]+)?\s*,))', Punctuation, '#pop'),
            (r'\)', Punctuation, '#pop:2'),
            (r'-?[a-z_][\w.]*', Name.Attribute),
            (r'(::\s*)([a-z_][\w.]*)', bygroups(Punctuation, Name.Label)),
            (r'\.\.\.', Name.Builtin.Pseudo),
            (r'[(,]', Punctuation),
            include('whitespacecomments'),
        ],
    }

    def __init__(self, **options):
        self.builtinshighlighting = get_bool_opt(
            options, 'builtinshighlighting', True)
        self.requiredelimiters = get_bool_opt(
            options, 'requiredelimiters', False)

        self._builtins = set()
        if self.builtinshighlighting:
            from pygments.lexers._lassobuiltins import BUILTINS
            for key, value in BUILTINS.iteritems():
                self._builtins.update(value)
        RegexLexer.__init__(self, **options)

    def get_tokens_unprocessed(self, text):
        stack = ['root']
        if self.requiredelimiters:
            stack.append('delimiters')
        for index, token, value in \
            RegexLexer.get_tokens_unprocessed(self, text, stack):
            if token is Name.Other:
                if value.lower() in self._builtins:
                    yield index, Name.Builtin, value
                    continue
            yield index, token, value

    def analyse_text(text):
        rv = 0.0
        if 'bin/lasso9' in text:
            rv += 0.8
        if re.search(r'<\?(=|lasso)', text, re.I):
            rv += 0.4
        if re.search(r'local\(', text, re.I):
            rv += 0.4
        if re.search(r'\[\n|\?>', text):
            rv += 0.4
        return rv


class QmlLexer(RegexLexer):
    """
    For QML files. See http://doc.qt.digia.com/4.7/qdeclarativeintroduction.html.

    *New in Pygments 1.6.*
    """

    # QML is based on javascript, so much of this is taken from the
    # JavascriptLexer above.

    name = 'QML'
    aliases = ['qml', 'Qt Meta Language', 'Qt modeling Language']
    filenames = ['*.qml',]
    mimetypes = [ 'application/x-qml',]


    # pasted from JavascriptLexer, with some additions
    flags = re.DOTALL
    tokens = {
        'commentsandwhitespace': [
            (r'\s+', Text),
            (r'<!--', Comment),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline)
        ],
        'slashstartsregex': [
            include('commentsandwhitespace'),
            (r'/(\\.|[^[/\\\n]|\[(\\.|[^\]\\\n])*])+/'
             r'([gim]+\b|\B)', String.Regex, '#pop'),
            (r'(?=/)', Text, ('#pop', 'badregex')),
            (r'', Text, '#pop')
        ],
        'badregex': [
            (r'\n', Text, '#pop')
        ],
        'root' : [
            (r'^(?=\s|/|<!--)', Text, 'slashstartsregex'),
            include('commentsandwhitespace'),
            (r'\+\+|--|~|&&|\?|:|\|\||\\(?=\n)|'
             r'(<<|>>>?|==?|!=?|[-<>+*%&\|\^/])=?', Operator, 'slashstartsregex'),
            (r'[{(\[;,]', Punctuation, 'slashstartsregex'),
            (r'[})\].]', Punctuation),

            # QML insertions
            (r'\bid\s*:\s*[A-Za-z][_A-Za-z.0-9]*',Keyword.Declaration,
             'slashstartsregex'),
            (r'\b[A-Za-z][_A-Za-z.0-9]*\s*:',Keyword, 'slashstartsregex'),

            # the rest from JavascriptLexer
            (r'(for|in|while|do|break|return|continue|switch|case|default|if|else|'
             r'throw|try|catch|finally|new|delete|typeof|instanceof|void|'
             r'this)\b', Keyword, 'slashstartsregex'),
            (r'(var|let|with|function)\b', Keyword.Declaration, 'slashstartsregex'),
            (r'(abstract|boolean|byte|char|class|const|debugger|double|enum|export|'
             r'extends|final|float|goto|implements|import|int|interface|long|native|'
             r'package|private|protected|public|short|static|super|synchronized|throws|'
             r'transient|volatile)\b', Keyword.Reserved),
            (r'(true|false|null|NaN|Infinity|undefined)\b', Keyword.Constant),
            (r'(Array|Boolean|Date|Error|Function|Math|netscape|'
             r'Number|Object|Packages|RegExp|String|sun|decodeURI|'
             r'decodeURIComponent|encodeURI|encodeURIComponent|'
             r'Error|eval|isFinite|isNaN|parseFloat|parseInt|document|this|'
             r'window)\b', Name.Builtin),
            (r'[$a-zA-Z_][a-zA-Z0-9_]*', Name.Other),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+', Number.Integer),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
        ]
    }
