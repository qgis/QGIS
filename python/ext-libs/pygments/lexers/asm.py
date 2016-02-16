# -*- coding: utf-8 -*-
"""
    pygments.lexers.asm
    ~~~~~~~~~~~~~~~~~~~

    Lexers for assembly languages.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import re

from pygments.lexer import RegexLexer, include, bygroups, using, DelegatingLexer
from pygments.lexers.compiled import DLexer, CppLexer, CLexer
from pygments.token import Text, Name, Number, String, Comment, Punctuation, \
     Other, Keyword, Operator

__all__ = ['GasLexer', 'ObjdumpLexer','DObjdumpLexer', 'CppObjdumpLexer',
           'CObjdumpLexer', 'LlvmLexer', 'NasmLexer', 'Ca65Lexer']


class GasLexer(RegexLexer):
    """
    For Gas (AT&T) assembly code.
    """
    name = 'GAS'
    aliases = ['gas']
    filenames = ['*.s', '*.S']
    mimetypes = ['text/x-gas']

    #: optional Comment or Whitespace
    string = r'"(\\"|[^"])*"'
    char = r'[a-zA-Z$._0-9@-]'
    identifier = r'(?:[a-zA-Z$_]' + char + '*|\.' + char + '+)'
    number = r'(?:0[xX][a-zA-Z0-9]+|\d+)'

    tokens = {
        'root': [
            include('whitespace'),
            (identifier + ':', Name.Label),
            (r'\.' + identifier, Name.Attribute, 'directive-args'),
            (r'lock|rep(n?z)?|data\d+', Name.Attribute),
            (identifier, Name.Function, 'instruction-args'),
            (r'[\r\n]+', Text)
        ],
        'directive-args': [
            (identifier, Name.Constant),
            (string, String),
            ('@' + identifier, Name.Attribute),
            (number, Number.Integer),
            (r'[\r\n]+', Text, '#pop'),

            (r'#.*?$', Comment, '#pop'),

            include('punctuation'),
            include('whitespace')
        ],
        'instruction-args': [
            # For objdump-disassembled code, shouldn't occur in
            # actual assembler input
            ('([a-z0-9]+)( )(<)('+identifier+')(>)',
                bygroups(Number.Hex, Text, Punctuation, Name.Constant,
                         Punctuation)),
            ('([a-z0-9]+)( )(<)('+identifier+')([-+])('+number+')(>)',
                bygroups(Number.Hex, Text, Punctuation, Name.Constant,
                         Punctuation, Number.Integer, Punctuation)),

            # Address constants
            (identifier, Name.Constant),
            (number, Number.Integer),
            # Registers
            ('%' + identifier, Name.Variable),
            # Numeric constants
            ('$'+number, Number.Integer),
            (r"$'(.|\\')'", String.Char),
            (r'[\r\n]+', Text, '#pop'),
            (r'#.*?$', Comment, '#pop'),
            include('punctuation'),
            include('whitespace')
        ],
        'whitespace': [
            (r'\n', Text),
            (r'\s+', Text),
            (r'#.*?\n', Comment)
        ],
        'punctuation': [
            (r'[-*,.():]+', Punctuation)
        ]
    }

    def analyse_text(text):
        if re.match(r'^\.(text|data|section)', text, re.M):
            return True
        elif re.match(r'^\.\w+', text, re.M):
            return 0.1


class ObjdumpLexer(RegexLexer):
    """
    For the output of 'objdump -dr'
    """
    name = 'objdump'
    aliases = ['objdump']
    filenames = ['*.objdump']
    mimetypes = ['text/x-objdump']

    hex = r'[0-9A-Za-z]'

    tokens = {
        'root': [
            # File name & format:
            ('(.*?)(:)( +file format )(.*?)$',
                bygroups(Name.Label, Punctuation, Text, String)),
            # Section header
            ('(Disassembly of section )(.*?)(:)$',
                bygroups(Text, Name.Label, Punctuation)),
            # Function labels
            # (With offset)
            ('('+hex+'+)( )(<)(.*?)([-+])(0[xX][A-Za-z0-9]+)(>:)$',
                bygroups(Number.Hex, Text, Punctuation, Name.Function,
                         Punctuation, Number.Hex, Punctuation)),
            # (Without offset)
            ('('+hex+'+)( )(<)(.*?)(>:)$',
                bygroups(Number.Hex, Text, Punctuation, Name.Function,
                         Punctuation)),
            # Code line with disassembled instructions
            ('( *)('+hex+r'+:)(\t)((?:'+hex+hex+' )+)( *\t)([a-zA-Z].*?)$',
                bygroups(Text, Name.Label, Text, Number.Hex, Text,
                         using(GasLexer))),
            # Code line with ascii
            ('( *)('+hex+r'+:)(\t)((?:'+hex+hex+' )+)( *)(.*?)$',
                bygroups(Text, Name.Label, Text, Number.Hex, Text, String)),
            # Continued code line, only raw opcodes without disassembled
            # instruction
            ('( *)('+hex+r'+:)(\t)((?:'+hex+hex+' )+)$',
                bygroups(Text, Name.Label, Text, Number.Hex)),
            # Skipped a few bytes
            (r'\t\.\.\.$', Text),
            # Relocation line
            # (With offset)
            (r'(\t\t\t)('+hex+r'+:)( )([^\t]+)(\t)(.*?)([-+])(0x' + hex + '+)$',
                bygroups(Text, Name.Label, Text, Name.Property, Text,
                         Name.Constant, Punctuation, Number.Hex)),
            # (Without offset)
            (r'(\t\t\t)('+hex+r'+:)( )([^\t]+)(\t)(.*?)$',
                bygroups(Text, Name.Label, Text, Name.Property, Text,
                         Name.Constant)),
            (r'[^\n]+\n', Other)
        ]
    }


class DObjdumpLexer(DelegatingLexer):
    """
    For the output of 'objdump -Sr on compiled D files'
    """
    name = 'd-objdump'
    aliases = ['d-objdump']
    filenames = ['*.d-objdump']
    mimetypes = ['text/x-d-objdump']

    def __init__(self, **options):
        super(DObjdumpLexer, self).__init__(DLexer, ObjdumpLexer, **options)


class CppObjdumpLexer(DelegatingLexer):
    """
    For the output of 'objdump -Sr on compiled C++ files'
    """
    name = 'cpp-objdump'
    aliases = ['cpp-objdump', 'c++-objdumb', 'cxx-objdump']
    filenames = ['*.cpp-objdump', '*.c++-objdump', '*.cxx-objdump']
    mimetypes = ['text/x-cpp-objdump']

    def __init__(self, **options):
        super(CppObjdumpLexer, self).__init__(CppLexer, ObjdumpLexer, **options)


class CObjdumpLexer(DelegatingLexer):
    """
    For the output of 'objdump -Sr on compiled C files'
    """
    name = 'c-objdump'
    aliases = ['c-objdump']
    filenames = ['*.c-objdump']
    mimetypes = ['text/x-c-objdump']

    def __init__(self, **options):
        super(CObjdumpLexer, self).__init__(CLexer, ObjdumpLexer, **options)


class LlvmLexer(RegexLexer):
    """
    For LLVM assembly code.
    """
    name = 'LLVM'
    aliases = ['llvm']
    filenames = ['*.ll']
    mimetypes = ['text/x-llvm']

    #: optional Comment or Whitespace
    string = r'"[^"]*?"'
    identifier = r'([-a-zA-Z$._][-a-zA-Z$._0-9]*|' + string + ')'

    tokens = {
        'root': [
            include('whitespace'),

            # Before keywords, because keywords are valid label names :(...
            (identifier + '\s*:', Name.Label),

            include('keyword'),

            (r'%' + identifier, Name.Variable),#Name.Identifier.Local),
            (r'@' + identifier, Name.Variable.Global),#Name.Identifier.Global),
            (r'%\d+', Name.Variable.Anonymous),#Name.Identifier.Anonymous),
            (r'@\d+', Name.Variable.Global),#Name.Identifier.Anonymous),
            (r'!' + identifier, Name.Variable),
            (r'!\d+', Name.Variable.Anonymous),
            (r'c?' + string, String),

            (r'0[xX][a-fA-F0-9]+', Number),
            (r'-?\d+(?:[.]\d+)?(?:[eE][-+]?\d+(?:[.]\d+)?)?', Number),

            (r'[=<>{}\[\]()*.,!]|x\b', Punctuation)
        ],
        'whitespace': [
            (r'(\n|\s)+', Text),
            (r';.*?\n', Comment)
        ],
        'keyword': [
            # Regular keywords
            (r'(begin|end'
             r'|true|false'
             r'|declare|define'
             r'|global|constant'

             r'|private|linker_private|internal|available_externally|linkonce'
             r'|linkonce_odr|weak|weak_odr|appending|dllimport|dllexport'
             r'|common|default|hidden|protected|extern_weak|external'
             r'|thread_local|zeroinitializer|undef|null|to|tail|target|triple'
             r'|datalayout|volatile|nuw|nsw|nnan|ninf|nsz|arcp|fast|exact|inbounds'
             r'|align|addrspace|section|alias|module|asm|sideeffect|gc|dbg'

             r'|ccc|fastcc|coldcc|x86_stdcallcc|x86_fastcallcc|arm_apcscc'
             r'|arm_aapcscc|arm_aapcs_vfpcc'

             r'|cc|c'

             r'|signext|zeroext|inreg|sret|nounwind|noreturn|noalias|nocapture'
             r'|byval|nest|readnone|readonly'

             r'|inlinehint|noinline|alwaysinline|optsize|ssp|sspreq|noredzone'
             r'|noimplicitfloat|naked'

             r'|type|opaque'

             r'|eq|ne|slt|sgt|sle'
             r'|sge|ult|ugt|ule|uge'
             r'|oeq|one|olt|ogt|ole'
             r'|oge|ord|uno|ueq|une'
             r'|x'

             # instructions
             r'|add|fadd|sub|fsub|mul|fmul|udiv|sdiv|fdiv|urem|srem|frem|shl'
             r'|lshr|ashr|and|or|xor|icmp|fcmp'

             r'|phi|call|trunc|zext|sext|fptrunc|fpext|uitofp|sitofp|fptoui'
             r'fptosi|inttoptr|ptrtoint|bitcast|select|va_arg|ret|br|switch'
             r'|invoke|unwind|unreachable'

             r'|malloc|alloca|free|load|store|getelementptr'

             r'|extractelement|insertelement|shufflevector|getresult'
             r'|extractvalue|insertvalue'

             r')\b', Keyword),

            # Types
            (r'void|float|double|x86_fp80|fp128|ppc_fp128|label|metadata',
             Keyword.Type),

            # Integer types
            (r'i[1-9]\d*', Keyword)
        ]
    }


class NasmLexer(RegexLexer):
    """
    For Nasm (Intel) assembly code.
    """
    name = 'NASM'
    aliases = ['nasm']
    filenames = ['*.asm', '*.ASM']
    mimetypes = ['text/x-nasm']

    identifier = r'[a-zA-Z$._?][a-zA-Z0-9$._?#@~]*'
    hexn = r'(?:0[xX][0-9a-fA-F]+|$0[0-9a-fA-F]*|[0-9]+[0-9a-fA-F]*h)'
    octn = r'[0-7]+q'
    binn = r'[01]+b'
    decn = r'[0-9]+'
    floatn = decn + r'\.e?' + decn
    string = r'"(\\"|[^"\n])*"|' + r"'(\\'|[^'\n])*'|" + r"`(\\`|[^`\n])*`"
    declkw = r'(?:res|d)[bwdqt]|times'
    register = (r'r[0-9][0-5]?[bwd]|'
                r'[a-d][lh]|[er]?[a-d]x|[er]?[sb]p|[er]?[sd]i|[c-gs]s|st[0-7]|'
                r'mm[0-7]|cr[0-4]|dr[0-367]|tr[3-7]')
    wordop = r'seg|wrt|strict'
    type = r'byte|[dq]?word'
    directives = (r'BITS|USE16|USE32|SECTION|SEGMENT|ABSOLUTE|EXTERN|GLOBAL|'
                  r'ORG|ALIGN|STRUC|ENDSTRUC|COMMON|CPU|GROUP|UPPERCASE|IMPORT|'
                  r'EXPORT|LIBRARY|MODULE')

    flags = re.IGNORECASE | re.MULTILINE
    tokens = {
        'root': [
            include('whitespace'),
            (r'^\s*%', Comment.Preproc, 'preproc'),
            (identifier + ':', Name.Label),
            (r'(%s)(\s+)(equ)' % identifier,
                bygroups(Name.Constant, Keyword.Declaration, Keyword.Declaration),
                'instruction-args'),
            (directives, Keyword, 'instruction-args'),
            (declkw, Keyword.Declaration, 'instruction-args'),
            (identifier, Name.Function, 'instruction-args'),
            (r'[\r\n]+', Text)
        ],
        'instruction-args': [
            (string, String),
            (hexn, Number.Hex),
            (octn, Number.Oct),
            (binn, Number),
            (floatn, Number.Float),
            (decn, Number.Integer),
            include('punctuation'),
            (register, Name.Builtin),
            (identifier, Name.Variable),
            (r'[\r\n]+', Text, '#pop'),
            include('whitespace')
        ],
        'preproc': [
            (r'[^;\n]+', Comment.Preproc),
            (r';.*?\n', Comment.Single, '#pop'),
            (r'\n', Comment.Preproc, '#pop'),
        ],
        'whitespace': [
            (r'\n', Text),
            (r'[ \t]+', Text),
            (r';.*', Comment.Single)
        ],
        'punctuation': [
            (r'[,():\[\]]+', Punctuation),
            (r'[&|^<>+*/%~-]+', Operator),
            (r'[$]+', Keyword.Constant),
            (wordop, Operator.Word),
            (type, Keyword.Type)
        ],
    }


class Ca65Lexer(RegexLexer):
    """
    For ca65 assembler sources.

    *New in Pygments 1.6.*
    """
    name = 'ca65'
    aliases = ['ca65']
    filenames = ['*.s']

    flags = re.IGNORECASE

    tokens = {
        'root': [
            (r';.*', Comment.Single),
            (r'\s+', Text),
            (r'[a-z_.@$][\w.@$]*:', Name.Label),
            (r'((ld|st)[axy]|(in|de)[cxy]|asl|lsr|ro[lr]|adc|sbc|cmp|cp[xy]'
             r'|cl[cvdi]|se[cdi]|jmp|jsr|bne|beq|bpl|bmi|bvc|bvs|bcc|bcs'
             r'|p[lh][ap]|rt[is]|brk|nop|ta[xy]|t[xy]a|txs|tsx|and|ora|eor'
             r'|bit)\b', Keyword),
            (r'\.[a-z0-9_]+', Keyword.Pseudo),
            (r'[-+~*/^&|!<>=]', Operator),
            (r'"[^"\n]*.', String),
            (r"'[^'\n]*.", String.Char),
            (r'\$[0-9a-f]+|[0-9a-f]+h\b', Number.Hex),
            (r'\d+|%[01]+', Number.Integer),
            (r'[#,.:()=]', Punctuation),
            (r'[a-z_.@$][\w.@$]*', Name),
        ]
    }

    def analyse_text(self, text):
        # comments in GAS start with "#"
        if re.match(r'^\s*;', text, re.MULTILINE):
            return 0.9
