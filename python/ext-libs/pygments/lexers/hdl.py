# -*- coding: utf-8 -*-
"""
    pygments.lexers.hdl
    ~~~~~~~~~~~~~~~~~~~

    Lexers for hardware descriptor languages.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import re
from pygments.lexer import RegexLexer, bygroups, include, using, this
from pygments.token import \
     Text, Comment, Operator, Keyword, Name, String, Number, Punctuation, \
     Error

__all__ = ['VerilogLexer', 'SystemVerilogLexer', 'VhdlLexer']


class VerilogLexer(RegexLexer):
    """
    For verilog source code with preprocessor directives.

    *New in Pygments 1.4.*
    """
    name = 'verilog'
    aliases = ['verilog', 'v']
    filenames = ['*.v']
    mimetypes = ['text/x-verilog']

    #: optional Comment or Whitespace
    _ws = r'(?:\s|//.*?\n|/[*].*?[*]/)+'

    tokens = {
        'root': [
            (r'^\s*`define', Comment.Preproc, 'macro'),
            (r'\n', Text),
            (r'\s+', Text),
            (r'\\\n', Text), # line continuation
            (r'/(\\\n)?/(\n|(.|\n)*?[^\\]\n)', Comment.Single),
            (r'/(\\\n)?[*](.|\n)*?[*](\\\n)?/', Comment.Multiline),
            (r'[{}#@]', Punctuation),
            (r'L?"', String, 'string'),
            (r"L?'(\\.|\\[0-7]{1,3}|\\x[a-fA-F0-9]{1,2}|[^\\\'\n])'", String.Char),
            (r'(\d+\.\d*|\.\d+|\d+)[eE][+-]?\d+[lL]?', Number.Float),
            (r'(\d+\.\d*|\.\d+|\d+[fF])[fF]?', Number.Float),
            (r'([0-9]+)|(\'h)[0-9a-fA-F]+', Number.Hex),
            (r'([0-9]+)|(\'b)[0-1]+', Number.Hex),   # should be binary
            (r'([0-9]+)|(\'d)[0-9]+', Number.Integer),
            (r'([0-9]+)|(\'o)[0-7]+', Number.Oct),
            (r'\'[01xz]', Number),
            (r'\d+[Ll]?', Number.Integer),
            (r'\*/', Error),
            (r'[~!%^&*+=|?:<>/-]', Operator),
            (r'[()\[\],.;\']', Punctuation),
            (r'`[a-zA-Z_][a-zA-Z0-9_]*', Name.Constant),

            (r'^(\s*)(package)(\s+)', bygroups(Text, Keyword.Namespace, Text)),
            (r'^(\s*)(import)(\s+)', bygroups(Text, Keyword.Namespace, Text),
             'import'),

            (r'(always|always_comb|always_ff|always_latch|and|assign|automatic|'
             r'begin|break|buf|bufif0|bufif1|case|casex|casez|cmos|const|'
             r'continue|deassign|default|defparam|disable|do|edge|else|end|endcase|'
             r'endfunction|endgenerate|endmodule|endpackage|endprimitive|endspecify|'
             r'endtable|endtask|enum|event|final|for|force|forever|fork|function|'
             r'generate|genvar|highz0|highz1|if|initial|inout|input|'
             r'integer|join|large|localparam|macromodule|medium|module|'
             r'nand|negedge|nmos|nor|not|notif0|notif1|or|output|packed|'
             r'parameter|pmos|posedge|primitive|pull0|pull1|pulldown|pullup|rcmos|'
             r'ref|release|repeat|return|rnmos|rpmos|rtran|rtranif0|'
             r'rtranif1|scalared|signed|small|specify|specparam|strength|'
             r'string|strong0|strong1|struct|table|task|'
             r'tran|tranif0|tranif1|type|typedef|'
             r'unsigned|var|vectored|void|wait|weak0|weak1|while|'
             r'xnor|xor)\b', Keyword),

            (r'`(accelerate|autoexpand_vectornets|celldefine|default_nettype|'
             r'else|elsif|endcelldefine|endif|endprotect|endprotected|'
             r'expand_vectornets|ifdef|ifndef|include|noaccelerate|noexpand_vectornets|'
             r'noremove_gatenames|noremove_netnames|nounconnected_drive|'
             r'protect|protected|remove_gatenames|remove_netnames|resetall|'
             r'timescale|unconnected_drive|undef)\b', Comment.Preproc),

            (r'\$(bits|bitstoreal|bitstoshortreal|countdrivers|display|fclose|'
             r'fdisplay|finish|floor|fmonitor|fopen|fstrobe|fwrite|'
             r'getpattern|history|incsave|input|itor|key|list|log|'
             r'monitor|monitoroff|monitoron|nokey|nolog|printtimescale|'
             r'random|readmemb|readmemh|realtime|realtobits|reset|reset_count|'
             r'reset_value|restart|rtoi|save|scale|scope|shortrealtobits|'
             r'showscopes|showvariables|showvars|sreadmemb|sreadmemh|'
             r'stime|stop|strobe|time|timeformat|write)\b', Name.Builtin),

            (r'(byte|shortint|int|longint|integer|time|'
             r'bit|logic|reg|'
             r'supply0|supply1|tri|triand|trior|tri0|tri1|trireg|uwire|wire|wand|wor'
             r'shortreal|real|realtime)\b', Keyword.Type),
            ('[a-zA-Z_][a-zA-Z0-9_]*:(?!:)', Name.Label),
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name),
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
        'import': [
            (r'[a-zA-Z0-9_:]+\*?', Name.Namespace, '#pop')
        ]
    }

    def get_tokens_unprocessed(self, text):
        for index, token, value in \
            RegexLexer.get_tokens_unprocessed(self, text):
            # Convention: mark all upper case names as constants
            if token is Name:
                if value.isupper():
                    token = Name.Constant
            yield index, token, value


class SystemVerilogLexer(RegexLexer):
    """
    Extends verilog lexer to recognise all SystemVerilog keywords from IEEE
    1800-2009 standard.

    *New in Pygments 1.5.*
    """
    name = 'systemverilog'
    aliases = ['systemverilog', 'sv']
    filenames = ['*.sv', '*.svh']
    mimetypes = ['text/x-systemverilog']

    #: optional Comment or Whitespace
    _ws = r'(?:\s|//.*?\n|/[*].*?[*]/)+'

    tokens = {
        'root': [
            (r'^\s*`define', Comment.Preproc, 'macro'),
            (r'^(\s*)(package)(\s+)', bygroups(Text, Keyword.Namespace, Text)),
            (r'^(\s*)(import)(\s+)', bygroups(Text, Keyword.Namespace, Text), 'import'),

            (r'\n', Text),
            (r'\s+', Text),
            (r'\\\n', Text), # line continuation
            (r'/(\\\n)?/(\n|(.|\n)*?[^\\]\n)', Comment.Single),
            (r'/(\\\n)?[*](.|\n)*?[*](\\\n)?/', Comment.Multiline),
            (r'[{}#@]', Punctuation),
            (r'L?"', String, 'string'),
            (r"L?'(\\.|\\[0-7]{1,3}|\\x[a-fA-F0-9]{1,2}|[^\\\'\n])'", String.Char),
            (r'(\d+\.\d*|\.\d+|\d+)[eE][+-]?\d+[lL]?', Number.Float),
            (r'(\d+\.\d*|\.\d+|\d+[fF])[fF]?', Number.Float),
            (r'([0-9]+)|(\'h)[0-9a-fA-F]+', Number.Hex),
            (r'([0-9]+)|(\'b)[0-1]+', Number.Hex),   # should be binary
            (r'([0-9]+)|(\'d)[0-9]+', Number.Integer),
            (r'([0-9]+)|(\'o)[0-7]+', Number.Oct),
            (r'\'[01xz]', Number),
            (r'\d+[Ll]?', Number.Integer),
            (r'\*/', Error),
            (r'[~!%^&*+=|?:<>/-]', Operator),
            (r'[()\[\],.;\']', Punctuation),
            (r'`[a-zA-Z_][a-zA-Z0-9_]*', Name.Constant),

            (r'(accept_on|alias|always|always_comb|always_ff|always_latch|'
             r'and|assert|assign|assume|automatic|before|begin|bind|bins|'
             r'binsof|bit|break|buf|bufif0|bufif1|byte|case|casex|casez|'
             r'cell|chandle|checker|class|clocking|cmos|config|const|constraint|'
             r'context|continue|cover|covergroup|coverpoint|cross|deassign|'
             r'default|defparam|design|disable|dist|do|edge|else|end|endcase|'
             r'endchecker|endclass|endclocking|endconfig|endfunction|endgenerate|'
             r'endgroup|endinterface|endmodule|endpackage|endprimitive|'
             r'endprogram|endproperty|endsequence|endspecify|endtable|'
             r'endtask|enum|event|eventually|expect|export|extends|extern|'
             r'final|first_match|for|force|foreach|forever|fork|forkjoin|'
             r'function|generate|genvar|global|highz0|highz1|if|iff|ifnone|'
             r'ignore_bins|illegal_bins|implies|import|incdir|include|'
             r'initial|inout|input|inside|instance|int|integer|interface|'
             r'intersect|join|join_any|join_none|large|let|liblist|library|'
             r'local|localparam|logic|longint|macromodule|matches|medium|'
             r'modport|module|nand|negedge|new|nexttime|nmos|nor|noshowcancelled|'
             r'not|notif0|notif1|null|or|output|package|packed|parameter|'
             r'pmos|posedge|primitive|priority|program|property|protected|'
             r'pull0|pull1|pulldown|pullup|pulsestyle_ondetect|pulsestyle_onevent|'
             r'pure|rand|randc|randcase|randsequence|rcmos|real|realtime|'
             r'ref|reg|reject_on|release|repeat|restrict|return|rnmos|'
             r'rpmos|rtran|rtranif0|rtranif1|s_always|s_eventually|s_nexttime|'
             r's_until|s_until_with|scalared|sequence|shortint|shortreal|'
             r'showcancelled|signed|small|solve|specify|specparam|static|'
             r'string|strong|strong0|strong1|struct|super|supply0|supply1|'
             r'sync_accept_on|sync_reject_on|table|tagged|task|this|throughout|'
             r'time|timeprecision|timeunit|tran|tranif0|tranif1|tri|tri0|'
             r'tri1|triand|trior|trireg|type|typedef|union|unique|unique0|'
             r'unsigned|until|until_with|untyped|use|uwire|var|vectored|'
             r'virtual|void|wait|wait_order|wand|weak|weak0|weak1|while|'
             r'wildcard|wire|with|within|wor|xnor|xor)\b', Keyword ),

            (r'(`__FILE__|`__LINE__|`begin_keywords|`celldefine|`default_nettype|'
             r'`define|`else|`elsif|`end_keywords|`endcelldefine|`endif|'
             r'`ifdef|`ifndef|`include|`line|`nounconnected_drive|`pragma|'
             r'`resetall|`timescale|`unconnected_drive|`undef|`undefineall)\b',
             Comment.Preproc ),

            (r'(\$display|\$displayb|\$displayh|\$displayo|\$dumpall|\$dumpfile|'
             r'\$dumpflush|\$dumplimit|\$dumpoff|\$dumpon|\$dumpports|'
             r'\$dumpportsall|\$dumpportsflush|\$dumpportslimit|\$dumpportsoff|'
             r'\$dumpportson|\$dumpvars|\$fclose|\$fdisplay|\$fdisplayb|'
             r'\$fdisplayh|\$fdisplayo|\$feof|\$ferror|\$fflush|\$fgetc|'
             r'\$fgets|\$fmonitor|\$fmonitorb|\$fmonitorh|\$fmonitoro|'
             r'\$fopen|\$fread|\$fscanf|\$fseek|\$fstrobe|\$fstrobeb|\$fstrobeh|'
             r'\$fstrobeo|\$ftell|\$fwrite|\$fwriteb|\$fwriteh|\$fwriteo|'
             r'\$monitor|\$monitorb|\$monitorh|\$monitoro|\$monitoroff|'
             r'\$monitoron|\$plusargs|\$readmemb|\$readmemh|\$rewind|\$sformat|'
             r'\$sformatf|\$sscanf|\$strobe|\$strobeb|\$strobeh|\$strobeo|'
             r'\$swrite|\$swriteb|\$swriteh|\$swriteo|\$test|\$ungetc|'
             r'\$value\$plusargs|\$write|\$writeb|\$writeh|\$writememb|'
             r'\$writememh|\$writeo)\b' , Name.Builtin ),

            (r'(class)(\s+)', bygroups(Keyword, Text), 'classname'),
            (r'(byte|shortint|int|longint|integer|time|'
             r'bit|logic|reg|'
             r'supply0|supply1|tri|triand|trior|tri0|tri1|trireg|uwire|wire|wand|wor'
             r'shortreal|real|realtime)\b', Keyword.Type),
            ('[a-zA-Z_][a-zA-Z0-9_]*:(?!:)', Name.Label),
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name),
        ],
        'classname': [
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name.Class, '#pop'),
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
        'import': [
            (r'[a-zA-Z0-9_:]+\*?', Name.Namespace, '#pop')
        ]
    }

    def get_tokens_unprocessed(self, text):
        for index, token, value in \
            RegexLexer.get_tokens_unprocessed(self, text):
            # Convention: mark all upper case names as constants
            if token is Name:
                if value.isupper():
                    token = Name.Constant
            yield index, token, value

    def analyse_text(text):
        if text.startswith('//') or text.startswith('/*'):
            return 0.5


class VhdlLexer(RegexLexer):
    """
    For VHDL source code.

    *New in Pygments 1.5.*
    """
    name = 'vhdl'
    aliases = ['vhdl']
    filenames = ['*.vhdl', '*.vhd']
    mimetypes = ['text/x-vhdl']
    flags = re.MULTILINE | re.IGNORECASE

    tokens = {
        'root': [
            (r'\n', Text),
            (r'\s+', Text),
            (r'\\\n', Text), # line continuation
            (r'--(?![!#$%&*+./<=>?@\^|_~]).*?$', Comment.Single),
            (r"'(U|X|0|1|Z|W|L|H|-)'", String.Char),
            (r'[~!%^&*+=|?:<>/-]', Operator),
            (r"'[a-zA-Z_][a-zA-Z0-9_]*", Name.Attribute),
            (r'[()\[\],.;\']', Punctuation),
            (r'"[^\n\\]*"', String),

            (r'(library)(\s+)([a-zA-Z_][a-zA-Z0-9_]*)',
             bygroups(Keyword, Text, Name.Namespace)),
            (r'(use)(\s+)(entity)', bygroups(Keyword, Text, Keyword)),
            (r'(use)(\s+)([a-zA-Z_][\.a-zA-Z0-9_]*)',
             bygroups(Keyword, Text, Name.Namespace)),
            (r'(entity|component)(\s+)([a-zA-Z_][a-zA-Z0-9_]*)',
             bygroups(Keyword, Text, Name.Class)),
            (r'(architecture|configuration)(\s+)([a-zA-Z_][a-zA-Z0-9_]*)(\s+)'
             r'(of)(\s+)([a-zA-Z_][a-zA-Z0-9_]*)(\s+)(is)',
             bygroups(Keyword, Text, Name.Class, Text, Keyword, Text,
                      Name.Class, Text, Keyword)),

            (r'(end)(\s+)', bygroups(using(this), Text), 'endblock'),

            include('types'),
            include('keywords'),
            include('numbers'),

            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name),
        ],
        'endblock': [
            include('keywords'),
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name.Class),
            (r'(\s+)', Text),
            (r';', Punctuation, '#pop'),
        ],
        'types': [
            (r'(boolean|bit|character|severity_level|integer|time|delay_length|'
             r'natural|positive|string|bit_vector|file_open_kind|'
             r'file_open_status|std_ulogic|std_ulogic_vector|std_logic|'
             r'std_logic_vector)\b', Keyword.Type),
        ],
        'keywords': [
            (r'(abs|access|after|alias|all|and|'
             r'architecture|array|assert|attribute|begin|block|'
             r'body|buffer|bus|case|component|configuration|'
             r'constant|disconnect|downto|else|elsif|end|'
             r'entity|exit|file|for|function|generate|'
             r'generic|group|guarded|if|impure|in|'
             r'inertial|inout|is|label|library|linkage|'
             r'literal|loop|map|mod|nand|new|'
             r'next|nor|not|null|of|on|'
             r'open|or|others|out|package|port|'
             r'postponed|procedure|process|pure|range|record|'
             r'register|reject|return|rol|ror|select|'
             r'severity|signal|shared|sla|sli|sra|'
             r'srl|subtype|then|to|transport|type|'
             r'units|until|use|variable|wait|when|'
             r'while|with|xnor|xor)\b', Keyword),
        ],
        'numbers': [
            (r'\d{1,2}#[0-9a-fA-F_]+#?', Number.Integer),
            (r'[0-1_]+(\.[0-1_])', Number.Integer),
            (r'\d+', Number.Integer),
            (r'(\d+\.\d*|\.\d+|\d+)[eE][+-]?\d+', Number.Float),
            (r'H"[0-9a-fA-F_]+"', Number.Oct),
            (r'O"[0-7_]+"', Number.Oct),
            (r'B"[0-1_]+"', Number.Oct),
        ],
    }
