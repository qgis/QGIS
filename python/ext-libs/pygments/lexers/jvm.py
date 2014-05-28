# -*- coding: utf-8 -*-
"""
    pygments.lexers.jvm
    ~~~~~~~~~~~~~~~~~~~

    Pygments lexers for JVM languages.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import re

from pygments.lexer import Lexer, RegexLexer, include, bygroups, using, \
     this
from pygments.token import Text, Comment, Operator, Keyword, Name, String, \
     Number, Punctuation
from pygments.util import get_choice_opt
from pygments import unistring as uni


__all__ = ['JavaLexer', 'ScalaLexer', 'GosuLexer', 'GosuTemplateLexer',
           'GroovyLexer', 'IokeLexer', 'ClojureLexer', 'KotlinLexer',
           'XtendLexer', 'AspectJLexer', 'CeylonLexer']


class JavaLexer(RegexLexer):
    """
    For `Java <http://www.sun.com/java/>`_ source code.
    """

    name = 'Java'
    aliases = ['java']
    filenames = ['*.java']
    mimetypes = ['text/x-java']

    flags = re.MULTILINE | re.DOTALL

    tokens = {
        'root': [
            # method names
            (r'^(\s*(?:[a-zA-Z_][a-zA-Z0-9_\.\[\]<>]*\s+)+?)' # return arguments
             r'([a-zA-Z_][a-zA-Z0-9_]*)'                      # method name
             r'(\s*)(\()',                                    # signature start
             bygroups(using(this), Name.Function, Text, Operator)),
            (r'[^\S\n]+', Text),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'@[a-zA-Z_][a-zA-Z0-9_\.]*', Name.Decorator),
            (r'(assert|break|case|catch|continue|default|do|else|finally|for|'
             r'if|goto|instanceof|new|return|switch|this|throw|try|while)\b',
             Keyword),
            (r'(abstract|const|enum|extends|final|implements|native|private|'
             r'protected|public|static|strictfp|super|synchronized|throws|'
             r'transient|volatile)\b', Keyword.Declaration),
            (r'(boolean|byte|char|double|float|int|long|short|void)\b',
             Keyword.Type),
            (r'(package)(\s+)', bygroups(Keyword.Namespace, Text)),
            (r'(true|false|null)\b', Keyword.Constant),
            (r'(class|interface)(\s+)', bygroups(Keyword.Declaration, Text), 'class'),
            (r'(import)(\s+)', bygroups(Keyword.Namespace, Text), 'import'),
            (r'"(\\\\|\\"|[^"])*"', String),
            (r"'\\.'|'[^\\]'|'\\u[0-9a-fA-F]{4}'", String.Char),
            (r'(\.)([a-zA-Z_][a-zA-Z0-9_]*)', bygroups(Operator, Name.Attribute)),
            (r'[a-zA-Z_][a-zA-Z0-9_]*:', Name.Label),
            (r'[a-zA-Z_\$][a-zA-Z0-9_]*', Name),
            (r'[~\^\*!%&\[\]\(\)\{\}<>\|+=:;,./?-]', Operator),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+L?', Number.Integer),
            (r'\n', Text)
        ],
        'class': [
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name.Class, '#pop')
        ],
        'import': [
            (r'[a-zA-Z0-9_.]+\*?', Name.Namespace, '#pop')
        ],
    }


class AspectJLexer(JavaLexer):
    """
    For `AspectJ <http://www.eclipse.org/aspectj/>`_ source code.

    *New in Pygments 1.6.*
    """

    name = 'AspectJ'
    aliases = ['aspectj']
    filenames = ['*.aj']
    mimetypes = ['text/x-aspectj']

    aj_keywords = [
        'aspect', 'pointcut', 'privileged', 'call', 'execution',
        'initialization', 'preinitialization', 'handler', 'get', 'set',
        'staticinitialization', 'target', 'args', 'within', 'withincode',
        'cflow', 'cflowbelow', 'annotation', 'before', 'after', 'around',
        'proceed', 'throwing', 'returning', 'adviceexecution', 'declare',
        'parents', 'warning', 'error', 'soft', 'precedence', 'thisJoinPoint',
        'thisJoinPointStaticPart', 'thisEnclosingJoinPointStaticPart',
        'issingleton', 'perthis', 'pertarget', 'percflow', 'percflowbelow',
        'pertypewithin', 'lock', 'unlock', 'thisAspectInstance'
    ]
    aj_inter_type = ['parents:', 'warning:', 'error:', 'soft:', 'precedence:']
    aj_inter_type_annotation = ['@type', '@method', '@constructor', '@field']

    def get_tokens_unprocessed(self, text):
        for index, token, value in JavaLexer.get_tokens_unprocessed(self, text):
            if token is Name and value in self.aj_keywords:
                yield index, Keyword, value
            elif token is Name.Label and value in self.aj_inter_type:
                yield index, Keyword, value[:-1]
                yield index, Operator, value[-1]
            elif token is Name.Decorator and value in self.aj_inter_type_annotation:
                yield index, Keyword, value
            else:
                yield index, token, value


class ScalaLexer(RegexLexer):
    """
    For `Scala <http://www.scala-lang.org>`_ source code.
    """

    name = 'Scala'
    aliases = ['scala']
    filenames = ['*.scala']
    mimetypes = ['text/x-scala']

    flags = re.MULTILINE | re.DOTALL

    # don't use raw unicode strings!
    op = (u'[-~\\^\\*!%&\\\\<>\\|+=:/?@\u00a6-\u00a7\u00a9\u00ac\u00ae\u00b0-\u00b1'
          u'\u00b6\u00d7\u00f7\u03f6\u0482\u0606-\u0608\u060e-\u060f\u06e9'
          u'\u06fd-\u06fe\u07f6\u09fa\u0b70\u0bf3-\u0bf8\u0bfa\u0c7f\u0cf1-\u0cf2'
          u'\u0d79\u0f01-\u0f03\u0f13-\u0f17\u0f1a-\u0f1f\u0f34\u0f36\u0f38'
          u'\u0fbe-\u0fc5\u0fc7-\u0fcf\u109e-\u109f\u1360\u1390-\u1399\u1940'
          u'\u19e0-\u19ff\u1b61-\u1b6a\u1b74-\u1b7c\u2044\u2052\u207a-\u207c'
          u'\u208a-\u208c\u2100-\u2101\u2103-\u2106\u2108-\u2109\u2114\u2116-\u2118'
          u'\u211e-\u2123\u2125\u2127\u2129\u212e\u213a-\u213b\u2140-\u2144'
          u'\u214a-\u214d\u214f\u2190-\u2328\u232b-\u244a\u249c-\u24e9\u2500-\u2767'
          u'\u2794-\u27c4\u27c7-\u27e5\u27f0-\u2982\u2999-\u29d7\u29dc-\u29fb'
          u'\u29fe-\u2b54\u2ce5-\u2cea\u2e80-\u2ffb\u3004\u3012-\u3013\u3020'
          u'\u3036-\u3037\u303e-\u303f\u3190-\u3191\u3196-\u319f\u31c0-\u31e3'
          u'\u3200-\u321e\u322a-\u3250\u3260-\u327f\u328a-\u32b0\u32c0-\u33ff'
          u'\u4dc0-\u4dff\ua490-\ua4c6\ua828-\ua82b\ufb29\ufdfd\ufe62\ufe64-\ufe66'
          u'\uff0b\uff1c-\uff1e\uff5c\uff5e\uffe2\uffe4\uffe8-\uffee\ufffc-\ufffd]+')

    letter = (u'[a-zA-Z\\$_\u00aa\u00b5\u00ba\u00c0-\u00d6\u00d8-\u00f6'
              u'\u00f8-\u02af\u0370-\u0373\u0376-\u0377\u037b-\u037d\u0386'
              u'\u0388-\u03f5\u03f7-\u0481\u048a-\u0556\u0561-\u0587\u05d0-\u05f2'
              u'\u0621-\u063f\u0641-\u064a\u066e-\u066f\u0671-\u06d3\u06d5'
              u'\u06ee-\u06ef\u06fa-\u06fc\u06ff\u0710\u0712-\u072f\u074d-\u07a5'
              u'\u07b1\u07ca-\u07ea\u0904-\u0939\u093d\u0950\u0958-\u0961'
              u'\u0972-\u097f\u0985-\u09b9\u09bd\u09ce\u09dc-\u09e1\u09f0-\u09f1'
              u'\u0a05-\u0a39\u0a59-\u0a5e\u0a72-\u0a74\u0a85-\u0ab9\u0abd'
              u'\u0ad0-\u0ae1\u0b05-\u0b39\u0b3d\u0b5c-\u0b61\u0b71\u0b83-\u0bb9'
              u'\u0bd0\u0c05-\u0c3d\u0c58-\u0c61\u0c85-\u0cb9\u0cbd\u0cde-\u0ce1'
              u'\u0d05-\u0d3d\u0d60-\u0d61\u0d7a-\u0d7f\u0d85-\u0dc6\u0e01-\u0e30'
              u'\u0e32-\u0e33\u0e40-\u0e45\u0e81-\u0eb0\u0eb2-\u0eb3\u0ebd-\u0ec4'
              u'\u0edc-\u0f00\u0f40-\u0f6c\u0f88-\u0f8b\u1000-\u102a\u103f'
              u'\u1050-\u1055\u105a-\u105d\u1061\u1065-\u1066\u106e-\u1070'
              u'\u1075-\u1081\u108e\u10a0-\u10fa\u1100-\u135a\u1380-\u138f'
              u'\u13a0-\u166c\u166f-\u1676\u1681-\u169a\u16a0-\u16ea\u16ee-\u1711'
              u'\u1720-\u1731\u1740-\u1751\u1760-\u1770\u1780-\u17b3\u17dc'
              u'\u1820-\u1842\u1844-\u18a8\u18aa-\u191c\u1950-\u19a9\u19c1-\u19c7'
              u'\u1a00-\u1a16\u1b05-\u1b33\u1b45-\u1b4b\u1b83-\u1ba0\u1bae-\u1baf'
              u'\u1c00-\u1c23\u1c4d-\u1c4f\u1c5a-\u1c77\u1d00-\u1d2b\u1d62-\u1d77'
              u'\u1d79-\u1d9a\u1e00-\u1fbc\u1fbe\u1fc2-\u1fcc\u1fd0-\u1fdb'
              u'\u1fe0-\u1fec\u1ff2-\u1ffc\u2071\u207f\u2102\u2107\u210a-\u2113'
              u'\u2115\u2119-\u211d\u2124\u2126\u2128\u212a-\u212d\u212f-\u2139'
              u'\u213c-\u213f\u2145-\u2149\u214e\u2160-\u2188\u2c00-\u2c7c'
              u'\u2c80-\u2ce4\u2d00-\u2d65\u2d80-\u2dde\u3006-\u3007\u3021-\u3029'
              u'\u3038-\u303a\u303c\u3041-\u3096\u309f\u30a1-\u30fa\u30ff-\u318e'
              u'\u31a0-\u31b7\u31f0-\u31ff\u3400-\u4db5\u4e00-\ua014\ua016-\ua48c'
              u'\ua500-\ua60b\ua610-\ua61f\ua62a-\ua66e\ua680-\ua697\ua722-\ua76f'
              u'\ua771-\ua787\ua78b-\ua801\ua803-\ua805\ua807-\ua80a\ua80c-\ua822'
              u'\ua840-\ua873\ua882-\ua8b3\ua90a-\ua925\ua930-\ua946\uaa00-\uaa28'
              u'\uaa40-\uaa42\uaa44-\uaa4b\uac00-\ud7a3\uf900-\ufb1d\ufb1f-\ufb28'
              u'\ufb2a-\ufd3d\ufd50-\ufdfb\ufe70-\ufefc\uff21-\uff3a\uff41-\uff5a'
              u'\uff66-\uff6f\uff71-\uff9d\uffa0-\uffdc]')

    upper = (u'[A-Z\\$_\u00c0-\u00d6\u00d8-\u00de\u0100\u0102\u0104\u0106\u0108'
             u'\u010a\u010c\u010e\u0110\u0112\u0114\u0116\u0118\u011a\u011c'
             u'\u011e\u0120\u0122\u0124\u0126\u0128\u012a\u012c\u012e\u0130'
             u'\u0132\u0134\u0136\u0139\u013b\u013d\u013f\u0141\u0143\u0145'
             u'\u0147\u014a\u014c\u014e\u0150\u0152\u0154\u0156\u0158\u015a'
             u'\u015c\u015e\u0160\u0162\u0164\u0166\u0168\u016a\u016c\u016e'
             u'\u0170\u0172\u0174\u0176\u0178-\u0179\u017b\u017d\u0181-\u0182'
             u'\u0184\u0186-\u0187\u0189-\u018b\u018e-\u0191\u0193-\u0194'
             u'\u0196-\u0198\u019c-\u019d\u019f-\u01a0\u01a2\u01a4\u01a6-\u01a7'
             u'\u01a9\u01ac\u01ae-\u01af\u01b1-\u01b3\u01b5\u01b7-\u01b8\u01bc'
             u'\u01c4\u01c7\u01ca\u01cd\u01cf\u01d1\u01d3\u01d5\u01d7\u01d9'
             u'\u01db\u01de\u01e0\u01e2\u01e4\u01e6\u01e8\u01ea\u01ec\u01ee'
             u'\u01f1\u01f4\u01f6-\u01f8\u01fa\u01fc\u01fe\u0200\u0202\u0204'
             u'\u0206\u0208\u020a\u020c\u020e\u0210\u0212\u0214\u0216\u0218'
             u'\u021a\u021c\u021e\u0220\u0222\u0224\u0226\u0228\u022a\u022c'
             u'\u022e\u0230\u0232\u023a-\u023b\u023d-\u023e\u0241\u0243-\u0246'
             u'\u0248\u024a\u024c\u024e\u0370\u0372\u0376\u0386\u0388-\u038f'
             u'\u0391-\u03ab\u03cf\u03d2-\u03d4\u03d8\u03da\u03dc\u03de\u03e0'
             u'\u03e2\u03e4\u03e6\u03e8\u03ea\u03ec\u03ee\u03f4\u03f7'
             u'\u03f9-\u03fa\u03fd-\u042f\u0460\u0462\u0464\u0466\u0468\u046a'
             u'\u046c\u046e\u0470\u0472\u0474\u0476\u0478\u047a\u047c\u047e'
             u'\u0480\u048a\u048c\u048e\u0490\u0492\u0494\u0496\u0498\u049a'
             u'\u049c\u049e\u04a0\u04a2\u04a4\u04a6\u04a8\u04aa\u04ac\u04ae'
             u'\u04b0\u04b2\u04b4\u04b6\u04b8\u04ba\u04bc\u04be\u04c0-\u04c1'
             u'\u04c3\u04c5\u04c7\u04c9\u04cb\u04cd\u04d0\u04d2\u04d4\u04d6'
             u'\u04d8\u04da\u04dc\u04de\u04e0\u04e2\u04e4\u04e6\u04e8\u04ea'
             u'\u04ec\u04ee\u04f0\u04f2\u04f4\u04f6\u04f8\u04fa\u04fc\u04fe'
             u'\u0500\u0502\u0504\u0506\u0508\u050a\u050c\u050e\u0510\u0512'
             u'\u0514\u0516\u0518\u051a\u051c\u051e\u0520\u0522\u0531-\u0556'
             u'\u10a0-\u10c5\u1e00\u1e02\u1e04\u1e06\u1e08\u1e0a\u1e0c\u1e0e'
             u'\u1e10\u1e12\u1e14\u1e16\u1e18\u1e1a\u1e1c\u1e1e\u1e20\u1e22'
             u'\u1e24\u1e26\u1e28\u1e2a\u1e2c\u1e2e\u1e30\u1e32\u1e34\u1e36'
             u'\u1e38\u1e3a\u1e3c\u1e3e\u1e40\u1e42\u1e44\u1e46\u1e48\u1e4a'
             u'\u1e4c\u1e4e\u1e50\u1e52\u1e54\u1e56\u1e58\u1e5a\u1e5c\u1e5e'
             u'\u1e60\u1e62\u1e64\u1e66\u1e68\u1e6a\u1e6c\u1e6e\u1e70\u1e72'
             u'\u1e74\u1e76\u1e78\u1e7a\u1e7c\u1e7e\u1e80\u1e82\u1e84\u1e86'
             u'\u1e88\u1e8a\u1e8c\u1e8e\u1e90\u1e92\u1e94\u1e9e\u1ea0\u1ea2'
             u'\u1ea4\u1ea6\u1ea8\u1eaa\u1eac\u1eae\u1eb0\u1eb2\u1eb4\u1eb6'
             u'\u1eb8\u1eba\u1ebc\u1ebe\u1ec0\u1ec2\u1ec4\u1ec6\u1ec8\u1eca'
             u'\u1ecc\u1ece\u1ed0\u1ed2\u1ed4\u1ed6\u1ed8\u1eda\u1edc\u1ede'
             u'\u1ee0\u1ee2\u1ee4\u1ee6\u1ee8\u1eea\u1eec\u1eee\u1ef0\u1ef2'
             u'\u1ef4\u1ef6\u1ef8\u1efa\u1efc\u1efe\u1f08-\u1f0f\u1f18-\u1f1d'
             u'\u1f28-\u1f2f\u1f38-\u1f3f\u1f48-\u1f4d\u1f59-\u1f5f'
             u'\u1f68-\u1f6f\u1fb8-\u1fbb\u1fc8-\u1fcb\u1fd8-\u1fdb'
             u'\u1fe8-\u1fec\u1ff8-\u1ffb\u2102\u2107\u210b-\u210d\u2110-\u2112'
             u'\u2115\u2119-\u211d\u2124\u2126\u2128\u212a-\u212d\u2130-\u2133'
             u'\u213e-\u213f\u2145\u2183\u2c00-\u2c2e\u2c60\u2c62-\u2c64\u2c67'
             u'\u2c69\u2c6b\u2c6d-\u2c6f\u2c72\u2c75\u2c80\u2c82\u2c84\u2c86'
             u'\u2c88\u2c8a\u2c8c\u2c8e\u2c90\u2c92\u2c94\u2c96\u2c98\u2c9a'
             u'\u2c9c\u2c9e\u2ca0\u2ca2\u2ca4\u2ca6\u2ca8\u2caa\u2cac\u2cae'
             u'\u2cb0\u2cb2\u2cb4\u2cb6\u2cb8\u2cba\u2cbc\u2cbe\u2cc0\u2cc2'
             u'\u2cc4\u2cc6\u2cc8\u2cca\u2ccc\u2cce\u2cd0\u2cd2\u2cd4\u2cd6'
             u'\u2cd8\u2cda\u2cdc\u2cde\u2ce0\u2ce2\ua640\ua642\ua644\ua646'
             u'\ua648\ua64a\ua64c\ua64e\ua650\ua652\ua654\ua656\ua658\ua65a'
             u'\ua65c\ua65e\ua662\ua664\ua666\ua668\ua66a\ua66c\ua680\ua682'
             u'\ua684\ua686\ua688\ua68a\ua68c\ua68e\ua690\ua692\ua694\ua696'
             u'\ua722\ua724\ua726\ua728\ua72a\ua72c\ua72e\ua732\ua734\ua736'
             u'\ua738\ua73a\ua73c\ua73e\ua740\ua742\ua744\ua746\ua748\ua74a'
             u'\ua74c\ua74e\ua750\ua752\ua754\ua756\ua758\ua75a\ua75c\ua75e'
             u'\ua760\ua762\ua764\ua766\ua768\ua76a\ua76c\ua76e\ua779\ua77b'
             u'\ua77d-\ua77e\ua780\ua782\ua784\ua786\ua78b\uff21-\uff3a]')

    idrest = ur'%s(?:%s|[0-9])*(?:(?<=_)%s)?' % (letter, letter, op)

    tokens = {
        'root': [
            # method names
            (r'(class|trait|object)(\s+)', bygroups(Keyword, Text), 'class'),
            (ur"'%s" % idrest, Text.Symbol),
            (r'[^\S\n]+', Text),
            (r'//.*?\n', Comment.Single),
            (r'/\*', Comment.Multiline, 'comment'),
            (ur'@%s' % idrest, Name.Decorator),
            (ur'(abstract|ca(?:se|tch)|d(?:ef|o)|e(?:lse|xtends)|'
             ur'f(?:inal(?:ly)?|or(?:Some)?)|i(?:f|mplicit)|'
             ur'lazy|match|new|override|pr(?:ivate|otected)'
             ur'|re(?:quires|turn)|s(?:ealed|uper)|'
             ur't(?:h(?:is|row)|ry)|va[lr]|w(?:hile|ith)|yield)\b|'
             u'(<[%:-]|=>|>:|[#=@_\u21D2\u2190])(\\b|(?=\\s)|$)', Keyword),
            (ur':(?!%s)' % op, Keyword, 'type'),
            (ur'%s%s\b' % (upper, idrest), Name.Class),
            (r'(true|false|null)\b', Keyword.Constant),
            (r'(import|package)(\s+)', bygroups(Keyword, Text), 'import'),
            (r'(type)(\s+)', bygroups(Keyword, Text), 'type'),
            (r'""".*?"""(?!")', String),
            (r'"(\\\\|\\"|[^"])*"', String),
            (r"'\\.'|'[^\\]'|'\\u[0-9a-fA-F]{4}'", String.Char),
#            (ur'(\.)(%s|%s|`[^`]+`)' % (idrest, op), bygroups(Operator,
#             Name.Attribute)),
            (idrest, Name),
            (r'`[^`]+`', Name),
            (r'\[', Operator, 'typeparam'),
            (r'[\(\)\{\};,.#]', Operator),
            (op, Operator),
            (r'([0-9][0-9]*\.[0-9]*|\.[0-9]+)([eE][+-]?[0-9]+)?[fFdD]?',
             Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+L?', Number.Integer),
            (r'\n', Text)
        ],
        'class': [
            (ur'(%s|%s|`[^`]+`)(\s*)(\[)' % (idrest, op),
             bygroups(Name.Class, Text, Operator), 'typeparam'),
            (r'\s+', Text),
            (r'{', Operator, '#pop'),
            (r'\(', Operator, '#pop'),
            (r'//.*?\n', Comment.Single, '#pop'),
            (ur'%s|%s|`[^`]+`' % (idrest, op), Name.Class, '#pop'),
        ],
        'type': [
            (r'\s+', Text),
            (u'<[%:]|>:|[#_\u21D2]|forSome|type', Keyword),
            (r'([,\);}]|=>|=)(\s*)', bygroups(Operator, Text), '#pop'),
            (r'[\(\{]', Operator, '#push'),
            (ur'((?:%s|%s|`[^`]+`)(?:\.(?:%s|%s|`[^`]+`))*)(\s*)(\[)' %
             (idrest, op, idrest, op),
             bygroups(Keyword.Type, Text, Operator), ('#pop', 'typeparam')),
            (ur'((?:%s|%s|`[^`]+`)(?:\.(?:%s|%s|`[^`]+`))*)(\s*)$' %
             (idrest, op, idrest, op),
             bygroups(Keyword.Type, Text), '#pop'),
            (r'//.*?\n', Comment.Single, '#pop'),
            (ur'\.|%s|%s|`[^`]+`' % (idrest, op), Keyword.Type)
        ],
        'typeparam': [
            (r'[\s,]+', Text),
            (u'<[%:]|=>|>:|[#_\u21D2]|forSome|type', Keyword),
            (r'([\]\)\}])', Operator, '#pop'),
            (r'[\(\[\{]', Operator, '#push'),
            (ur'\.|%s|%s|`[^`]+`' % (idrest, op), Keyword.Type)
        ],
        'comment': [
            (r'[^/\*]+', Comment.Multiline),
            (r'/\*', Comment.Multiline, '#push'),
            (r'\*/', Comment.Multiline, '#pop'),
            (r'[*/]', Comment.Multiline)
        ],
        'import': [
            (ur'(%s|\.)+' % idrest, Name.Namespace, '#pop')
        ],
    }


class GosuLexer(RegexLexer):
    """
    For Gosu source code.

    *New in Pygments 1.5.*
    """

    name = 'Gosu'
    aliases = ['gosu']
    filenames = ['*.gs', '*.gsx', '*.gsp', '*.vark']
    mimetypes = ['text/x-gosu']

    flags = re.MULTILINE | re.DOTALL

    tokens = {
        'root': [
            # method names
            (r'^(\s*(?:[a-zA-Z_][a-zA-Z0-9_\.\[\]]*\s+)+?)' # modifiers etc.
             r'([a-zA-Z_][a-zA-Z0-9_]*)'                    # method name
             r'(\s*)(\()',                                  # signature start
             bygroups(using(this), Name.Function, Text, Operator)),
            (r'[^\S\n]+', Text),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'@[a-zA-Z_][a-zA-Z0-9_\.]*', Name.Decorator),
            (r'(in|as|typeof|statictypeof|typeis|typeas|if|else|foreach|for|'
             r'index|while|do|continue|break|return|try|catch|finally|this|'
             r'throw|new|switch|case|default|eval|super|outer|classpath|'
             r'using)\b', Keyword),
            (r'(var|delegate|construct|function|private|internal|protected|'
             r'public|abstract|override|final|static|extends|transient|'
             r'implements|represents|readonly)\b', Keyword.Declaration),
            (r'(property\s+)(get|set)?', Keyword.Declaration),
            (r'(boolean|byte|char|double|float|int|long|short|void|block)\b',
             Keyword.Type),
            (r'(package)(\s+)', bygroups(Keyword.Namespace, Text)),
            (r'(true|false|null|NaN|Infinity)\b', Keyword.Constant),
            (r'(class|interface|enhancement|enum)(\s+)([a-zA-Z_][a-zA-Z0-9_]*)',
             bygroups(Keyword.Declaration, Text, Name.Class)),
            (r'(uses)(\s+)([a-zA-Z0-9_.]+\*?)',
             bygroups(Keyword.Namespace, Text, Name.Namespace)),
            (r'"', String, 'string'),
            (r'(\??[\.#])([a-zA-Z_][a-zA-Z0-9_]*)',
             bygroups(Operator, Name.Attribute)),
            (r'(:)([a-zA-Z_][a-zA-Z0-9_]*)',
             bygroups(Operator, Name.Attribute)),
            (r'[a-zA-Z_\$][a-zA-Z0-9_]*', Name),
            (r'and|or|not|[\\~\^\*!%&\[\]\(\)\{\}<>\|+=:;,./?-]', Operator),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'[0-9]+', Number.Integer),
            (r'\n', Text)
        ],
        'templateText': [
          (r'(\\<)|(\\\$)', String),
          (r'(<%@\s+)(extends|params)',
           bygroups(Operator, Name.Decorator), 'stringTemplate'),
          (r'<%!--.*?--%>', Comment.Multiline),
          (r'(<%)|(<%=)', Operator, 'stringTemplate'),
          (r'\$\{', Operator, 'stringTemplateShorthand'),
          (r'.', String)
        ],
        'string': [
          (r'"', String, '#pop'),
          include('templateText')
        ],
        'stringTemplate': [
          (r'"', String, 'string'),
          (r'%>', Operator, '#pop'),
          include('root')
        ],
        'stringTemplateShorthand': [
          (r'"', String, 'string'),
          (r'\{', Operator, 'stringTemplateShorthand'),
          (r'\}', Operator, '#pop'),
          include('root')
        ],
    }


class GosuTemplateLexer(Lexer):
    """
    For Gosu templates.

    *New in Pygments 1.5.*
    """

    name = 'Gosu Template'
    aliases = ['gst']
    filenames = ['*.gst']
    mimetypes = ['text/x-gosu-template']
    lexer = GosuLexer()

    def get_tokens_unprocessed(self, text):
        stack = ['templateText']
        for item in self.lexer.get_tokens_unprocessed(text, stack):
            yield item


class GroovyLexer(RegexLexer):
    """
    For `Groovy <http://groovy.codehaus.org/>`_ source code.

    *New in Pygments 1.5.*
    """

    name = 'Groovy'
    aliases = ['groovy']
    filenames = ['*.groovy']
    mimetypes = ['text/x-groovy']

    flags = re.MULTILINE | re.DOTALL

    tokens = {
        'root': [
            # method names
            (r'^(\s*(?:[a-zA-Z_][a-zA-Z0-9_\.\[\]]*\s+)+?)' # return arguments
             r'([a-zA-Z_][a-zA-Z0-9_]*)'                    # method name
             r'(\s*)(\()',                                  # signature start
             bygroups(using(this), Name.Function, Text, Operator)),
            (r'[^\S\n]+', Text),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'@[a-zA-Z_][a-zA-Z0-9_\.]*', Name.Decorator),
            (r'(assert|break|case|catch|continue|default|do|else|finally|for|'
             r'if|goto|instanceof|new|return|switch|this|throw|try|while|in|as)\b',
             Keyword),
            (r'(abstract|const|enum|extends|final|implements|native|private|'
             r'protected|public|static|strictfp|super|synchronized|throws|'
             r'transient|volatile)\b', Keyword.Declaration),
            (r'(def|boolean|byte|char|double|float|int|long|short|void)\b',
             Keyword.Type),
            (r'(package)(\s+)', bygroups(Keyword.Namespace, Text)),
            (r'(true|false|null)\b', Keyword.Constant),
            (r'(class|interface)(\s+)', bygroups(Keyword.Declaration, Text),
             'class'),
            (r'(import)(\s+)', bygroups(Keyword.Namespace, Text), 'import'),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
            (r'\$/((?!/\$).)*/\$', String),
            (r'/(\\\\|\\"|[^/])*/', String),
            (r"'\\.'|'[^\\]'|'\\u[0-9a-fA-F]{4}'", String.Char),
            (r'(\.)([a-zA-Z_][a-zA-Z0-9_]*)', bygroups(Operator, Name.Attribute)),
            (r'[a-zA-Z_][a-zA-Z0-9_]*:', Name.Label),
            (r'[a-zA-Z_\$][a-zA-Z0-9_]*', Name),
            (r'[~\^\*!%&\[\]\(\)\{\}<>\|+=:;,./?-]', Operator),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+L?', Number.Integer),
            (r'\n', Text)
        ],
        'class': [
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name.Class, '#pop')
        ],
        'import': [
            (r'[a-zA-Z0-9_.]+\*?', Name.Namespace, '#pop')
        ],
    }


class IokeLexer(RegexLexer):
    """
    For `Ioke <http://ioke.org/>`_ (a strongly typed, dynamic,
    prototype based programming language) source.

    *New in Pygments 1.4.*
    """
    name = 'Ioke'
    filenames = ['*.ik']
    aliases = ['ioke', 'ik']
    mimetypes = ['text/x-iokesrc']
    tokens = {
        'interpolatableText': [
            (r'(\\b|\\e|\\t|\\n|\\f|\\r|\\"|\\\\|\\#|\\\Z|\\u[0-9a-fA-F]{1,4}'
             r'|\\[0-3]?[0-7]?[0-7])', String.Escape),
            (r'#{', Punctuation, 'textInterpolationRoot')
            ],

        'text': [
            (r'(?<!\\)"', String, '#pop'),
            include('interpolatableText'),
            (r'[^"]', String)
            ],

        'documentation': [
            (r'(?<!\\)"', String.Doc, '#pop'),
            include('interpolatableText'),
            (r'[^"]', String.Doc)
            ],

        'textInterpolationRoot': [
            (r'}', Punctuation, '#pop'),
            include('root')
            ],

        'slashRegexp': [
            (r'(?<!\\)/[oxpniums]*', String.Regex, '#pop'),
            include('interpolatableText'),
            (r'\\/', String.Regex),
            (r'[^/]', String.Regex)
            ],

        'squareRegexp': [
            (r'(?<!\\)][oxpniums]*', String.Regex, '#pop'),
            include('interpolatableText'),
            (r'\\]', String.Regex),
            (r'[^\]]', String.Regex)
            ],

        'squareText': [
            (r'(?<!\\)]', String, '#pop'),
            include('interpolatableText'),
            (r'[^\]]', String)
            ],

        'root': [
            (r'\n', Text),
            (r'\s+', Text),

            # Comments
            (r';(.*?)\n', Comment),
            (r'\A#!(.*?)\n', Comment),

            #Regexps
            (r'#/', String.Regex, 'slashRegexp'),
            (r'#r\[', String.Regex, 'squareRegexp'),

            #Symbols
            (r':[a-zA-Z0-9_!:?]+', String.Symbol),
            (r'[a-zA-Z0-9_!:?]+:(?![a-zA-Z0-9_!?])', String.Other),
            (r':"(\\\\|\\"|[^"])*"', String.Symbol),

            #Documentation
            (r'((?<=fn\()|(?<=fnx\()|(?<=method\()|(?<=macro\()|(?<=lecro\()'
             r'|(?<=syntax\()|(?<=dmacro\()|(?<=dlecro\()|(?<=dlecrox\()'
             r'|(?<=dsyntax\())\s*"', String.Doc, 'documentation'),

            #Text
            (r'"', String, 'text'),
            (r'#\[', String, 'squareText'),

            #Mimic
            (r'[a-zA-Z0-9_][a-zA-Z0-9!?_:]+(?=\s*=.*mimic\s)', Name.Entity),

            #Assignment
            (r'[a-zA-Z_][a-zA-Z0-9_!:?]*(?=[\s]*[+*/-]?=[^=].*($|\.))',
             Name.Variable),

            # keywords
            (r'(break|cond|continue|do|ensure|for|for:dict|for:set|if|let|'
             r'loop|p:for|p:for:dict|p:for:set|return|unless|until|while|'
             r'with)(?![a-zA-Z0-9!:_?])', Keyword.Reserved),

            # Origin
            (r'(eval|mimic|print|println)(?![a-zA-Z0-9!:_?])', Keyword),

            # Base
            (r'(cell\?|cellNames|cellOwner\?|cellOwner|cells|cell|'
             r'documentation|hash|identity|mimic|removeCell\!|undefineCell\!)'
             r'(?![a-zA-Z0-9!:_?])', Keyword),

            # Ground
            (r'(stackTraceAsText)(?![a-zA-Z0-9!:_?])', Keyword),

            #DefaultBehaviour Literals
            (r'(dict|list|message|set)(?![a-zA-Z0-9!:_?])', Keyword.Reserved),

            #DefaultBehaviour Case
            (r'(case|case:and|case:else|case:nand|case:nor|case:not|case:or|'
             r'case:otherwise|case:xor)(?![a-zA-Z0-9!:_?])', Keyword.Reserved),

            #DefaultBehaviour Reflection
            (r'(asText|become\!|derive|freeze\!|frozen\?|in\?|is\?|kind\?|'
             r'mimic\!|mimics|mimics\?|prependMimic\!|removeAllMimics\!|'
             r'removeMimic\!|same\?|send|thaw\!|uniqueHexId)'
             r'(?![a-zA-Z0-9!:_?])', Keyword),

            #DefaultBehaviour Aspects
            (r'(after|around|before)(?![a-zA-Z0-9!:_?])', Keyword.Reserved),

            # DefaultBehaviour
            (r'(kind|cellDescriptionDict|cellSummary|genSym|inspect|notice)'
             r'(?![a-zA-Z0-9!:_?])', Keyword),
            (r'(use|destructuring)', Keyword.Reserved),

            #DefaultBehavior BaseBehavior
            (r'(cell\?|cellOwner\?|cellOwner|cellNames|cells|cell|'
             r'documentation|identity|removeCell!|undefineCell)'
             r'(?![a-zA-Z0-9!:_?])', Keyword),

            #DefaultBehavior Internal
            (r'(internal:compositeRegexp|internal:concatenateText|'
             r'internal:createDecimal|internal:createNumber|'
             r'internal:createRegexp|internal:createText)'
             r'(?![a-zA-Z0-9!:_?])', Keyword.Reserved),

            #DefaultBehaviour Conditions
            (r'(availableRestarts|bind|error\!|findRestart|handle|'
             r'invokeRestart|rescue|restart|signal\!|warn\!)'
             r'(?![a-zA-Z0-9!:_?])', Keyword.Reserved),

            # constants
            (r'(nil|false|true)(?![a-zA-Z0-9!:_?])', Name.Constant),

            # names
            (r'(Arity|Base|Call|Condition|DateTime|Aspects|Pointcut|'
             r'Assignment|BaseBehavior|Boolean|Case|AndCombiner|Else|'
             r'NAndCombiner|NOrCombiner|NotCombiner|OrCombiner|XOrCombiner|'
             r'Conditions|Definitions|FlowControl|Internal|Literals|'
             r'Reflection|DefaultMacro|DefaultMethod|DefaultSyntax|Dict|'
             r'FileSystem|Ground|Handler|Hook|IO|IokeGround|Struct|'
             r'LexicalBlock|LexicalMacro|List|Message|Method|Mixins|'
             r'NativeMethod|Number|Origin|Pair|Range|Reflector|Regexp Match|'
             r'Regexp|Rescue|Restart|Runtime|Sequence|Set|Symbol|'
             r'System|Text|Tuple)(?![a-zA-Z0-9!:_?])', Name.Builtin),

            # functions
            (ur'(generateMatchMethod|aliasMethod|\u03bb|\u028E|fnx|fn|method|'
             ur'dmacro|dlecro|syntax|macro|dlecrox|lecrox|lecro|syntax)'
             ur'(?![a-zA-Z0-9!:_?])', Name.Function),

            # Numbers
            (r'-?0[xX][0-9a-fA-F]+', Number.Hex),
            (r'-?(\d+\.?\d*|\d*\.\d+)([eE][+-]?[0-9]+)?', Number.Float),
            (r'-?\d+', Number.Integer),

            (r'#\(', Punctuation),

             # Operators
            (ur'(&&>>|\|\|>>|\*\*>>|:::|::|\.\.\.|===|\*\*>|\*\*=|&&>|&&=|'
             ur'\|\|>|\|\|=|\->>|\+>>|!>>|<>>>|<>>|&>>|%>>|#>>|@>>|/>>|\*>>|'
             ur'\?>>|\|>>|\^>>|~>>|\$>>|=>>|<<=|>>=|<=>|<\->|=~|!~|=>|\+\+|'
             ur'\-\-|<=|>=|==|!=|&&|\.\.|\+=|\-=|\*=|\/=|%=|&=|\^=|\|=|<\-|'
             ur'\+>|!>|<>|&>|%>|#>|\@>|\/>|\*>|\?>|\|>|\^>|~>|\$>|<\->|\->|'
             ur'<<|>>|\*\*|\?\||\?&|\|\||>|<|\*|\/|%|\+|\-|&|\^|\||=|\$|!|~|'
             ur'\?|#|\u2260|\u2218|\u2208|\u2209)', Operator),
            (r'(and|nand|or|xor|nor|return|import)(?![a-zA-Z0-9_!?])',
             Operator),

            # Punctuation
            (r'(\`\`|\`|\'\'|\'|\.|\,|@@|@|\[|\]|\(|\)|{|})', Punctuation),

            #kinds
            (r'[A-Z][a-zA-Z0-9_!:?]*', Name.Class),

            #default cellnames
            (r'[a-z_][a-zA-Z0-9_!:?]*', Name)
        ]
    }


class ClojureLexer(RegexLexer):
    """
    Lexer for `Clojure <http://clojure.org/>`_ source code.

    *New in Pygments 0.11.*
    """
    name = 'Clojure'
    aliases = ['clojure', 'clj']
    filenames = ['*.clj']
    mimetypes = ['text/x-clojure', 'application/x-clojure']

    special_forms = [
        '.', 'def', 'do', 'fn', 'if', 'let', 'new', 'quote', 'var', 'loop'
    ]

    # It's safe to consider 'ns' a declaration thing because it defines a new
    # namespace.
    declarations = [
        'def-', 'defn', 'defn-', 'defmacro', 'defmulti', 'defmethod',
        'defstruct', 'defonce', 'declare', 'definline', 'definterface',
        'defprotocol', 'defrecord', 'deftype', 'defproject', 'ns'
    ]

    builtins = [
        '*', '+', '-', '->', '/', '<', '<=', '=', '==', '>', '>=', '..',
        'accessor', 'agent', 'agent-errors', 'aget', 'alength', 'all-ns',
        'alter', 'and', 'append-child', 'apply', 'array-map', 'aset',
        'aset-boolean', 'aset-byte', 'aset-char', 'aset-double', 'aset-float',
        'aset-int', 'aset-long', 'aset-short', 'assert', 'assoc', 'await',
        'await-for', 'bean', 'binding', 'bit-and', 'bit-not', 'bit-or',
        'bit-shift-left', 'bit-shift-right', 'bit-xor', 'boolean', 'branch?',
        'butlast', 'byte', 'cast', 'char', 'children', 'class',
        'clear-agent-errors', 'comment', 'commute', 'comp', 'comparator',
        'complement', 'concat', 'conj', 'cons', 'constantly', 'cond', 'if-not',
        'construct-proxy', 'contains?', 'count', 'create-ns', 'create-struct',
        'cycle', 'dec',  'deref', 'difference', 'disj', 'dissoc', 'distinct',
        'doall', 'doc', 'dorun', 'doseq', 'dosync', 'dotimes', 'doto',
        'double', 'down', 'drop', 'drop-while', 'edit', 'end?', 'ensure',
        'eval', 'every?', 'false?', 'ffirst', 'file-seq', 'filter', 'find',
        'find-doc', 'find-ns', 'find-var', 'first', 'float', 'flush', 'for',
        'fnseq', 'frest', 'gensym', 'get-proxy-class', 'get',
        'hash-map', 'hash-set', 'identical?', 'identity', 'if-let', 'import',
        'in-ns', 'inc', 'index', 'insert-child', 'insert-left', 'insert-right',
        'inspect-table', 'inspect-tree', 'instance?', 'int', 'interleave',
        'intersection', 'into', 'into-array', 'iterate', 'join', 'key', 'keys',
        'keyword', 'keyword?', 'last', 'lazy-cat', 'lazy-cons', 'left',
        'lefts', 'line-seq', 'list*', 'list', 'load', 'load-file',
        'locking', 'long', 'loop', 'macroexpand', 'macroexpand-1',
        'make-array', 'make-node', 'map', 'map-invert', 'map?', 'mapcat',
        'max', 'max-key', 'memfn', 'merge', 'merge-with', 'meta', 'min',
        'min-key', 'name', 'namespace', 'neg?', 'new', 'newline', 'next',
        'nil?', 'node', 'not', 'not-any?', 'not-every?', 'not=', 'ns-imports',
        'ns-interns', 'ns-map', 'ns-name', 'ns-publics', 'ns-refers',
        'ns-resolve', 'ns-unmap', 'nth', 'nthrest', 'or', 'parse', 'partial',
        'path', 'peek', 'pop', 'pos?', 'pr', 'pr-str', 'print', 'print-str',
        'println', 'println-str', 'prn', 'prn-str', 'project', 'proxy',
        'proxy-mappings', 'quot', 'rand', 'rand-int', 'range', 're-find',
        're-groups', 're-matcher', 're-matches', 're-pattern', 're-seq',
        'read', 'read-line', 'reduce', 'ref', 'ref-set', 'refer', 'rem',
        'remove', 'remove-method', 'remove-ns', 'rename', 'rename-keys',
        'repeat', 'replace', 'replicate', 'resolve', 'rest', 'resultset-seq',
        'reverse', 'rfirst', 'right', 'rights', 'root', 'rrest', 'rseq',
        'second', 'select', 'select-keys', 'send', 'send-off', 'seq',
        'seq-zip', 'seq?', 'set', 'short', 'slurp', 'some', 'sort',
        'sort-by', 'sorted-map', 'sorted-map-by', 'sorted-set',
        'special-symbol?', 'split-at', 'split-with', 'str', 'string?',
        'struct', 'struct-map', 'subs', 'subvec', 'symbol', 'symbol?',
        'sync', 'take', 'take-nth', 'take-while', 'test', 'time', 'to-array',
        'to-array-2d', 'tree-seq', 'true?', 'union', 'up', 'update-proxy',
        'val', 'vals', 'var-get', 'var-set', 'var?', 'vector', 'vector-zip',
        'vector?', 'when', 'when-first', 'when-let', 'when-not',
        'with-local-vars', 'with-meta', 'with-open', 'with-out-str',
        'xml-seq', 'xml-zip', 'zero?', 'zipmap', 'zipper']

    # valid names for identifiers
    # well, names can only not consist fully of numbers
    # but this should be good enough for now

    # TODO / should divide keywords/symbols into namespace/rest
    # but that's hard, so just pretend / is part of the name
    valid_name = r'(?!#)[\w!$%*+<=>?/.#-]+'

    def _multi_escape(entries):
        return '(%s)' % ('|'.join(re.escape(entry) + ' ' for entry in entries))

    tokens = {
        'root': [
            # the comments - always starting with semicolon
            # and going to the end of the line
            (r';.*$', Comment.Single),

            # whitespaces - usually not relevant
            (r'[,\s]+', Text),

            # numbers
            (r'-?\d+\.\d+', Number.Float),
            (r'-?\d+', Number.Integer),
            (r'0x-?[abcdef\d]+', Number.Hex),

            # strings, symbols and characters
            (r'"(\\\\|\\"|[^"])*"', String),
            (r"'" + valid_name, String.Symbol),
            (r"\\(.|[a-z]+)", String.Char),

            # keywords
            (r'::?' + valid_name, String.Symbol),

            # special operators
            (r'~@|[`\'#^~&@]', Operator),

            # highlight the special forms
            (_multi_escape(special_forms), Keyword),

            # Technically, only the special forms are 'keywords'. The problem
            # is that only treating them as keywords means that things like
            # 'defn' and 'ns' need to be highlighted as builtins. This is ugly
            # and weird for most styles. So, as a compromise we're going to
            # highlight them as Keyword.Declarations.
            (_multi_escape(declarations), Keyword.Declaration),

            # highlight the builtins
            (_multi_escape(builtins), Name.Builtin),

            # the remaining functions
            (r'(?<=\()' + valid_name, Name.Function),

            # find the remaining variables
            (valid_name, Name.Variable),

            # Clojure accepts vector notation
            (r'(\[|\])', Punctuation),

            # Clojure accepts map notation
            (r'(\{|\})', Punctuation),

            # the famous parentheses!
            (r'(\(|\))', Punctuation),
        ],
    }


class TeaLangLexer(RegexLexer):
    """
    For `Tea <http://teatrove.org/>`_ source code. Only used within a
    TeaTemplateLexer.

    *New in Pygments 1.5.*
    """

    flags = re.MULTILINE | re.DOTALL

    tokens = {
        'root': [
            # method names
            (r'^(\s*(?:[a-zA-Z_][a-zA-Z0-9_\.\[\]]*\s+)+?)' # return arguments
             r'([a-zA-Z_][a-zA-Z0-9_]*)'                    # method name
             r'(\s*)(\()',                                  # signature start
             bygroups(using(this), Name.Function, Text, Operator)),
            (r'[^\S\n]+', Text),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'@[a-zA-Z_][a-zA-Z0-9_\.]*', Name.Decorator),
            (r'(and|break|else|foreach|if|in|not|or|reverse)\b',
             Keyword),
            (r'(as|call|define)\b', Keyword.Declaration),
            (r'(true|false|null)\b', Keyword.Constant),
            (r'(template)(\s+)', bygroups(Keyword.Declaration, Text), 'template'),
            (r'(import)(\s+)', bygroups(Keyword.Namespace, Text), 'import'),
            (r'"(\\\\|\\"|[^"])*"', String),
            (r'\'(\\\\|\\\'|[^\'])*\'', String),
            (r'(\.)([a-zA-Z_][a-zA-Z0-9_]*)', bygroups(Operator, Name.Attribute)),
            (r'[a-zA-Z_][a-zA-Z0-9_]*:', Name.Label),
            (r'[a-zA-Z_\$][a-zA-Z0-9_]*', Name),
            (r'(isa|[.]{3}|[.]{2}|[=#!<>+-/%&;,.\*\\\(\)\[\]\{\}])', Operator),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+L?', Number.Integer),
            (r'\n', Text)
        ],
        'template': [
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name.Class, '#pop')
        ],
        'import': [
            (r'[a-zA-Z0-9_.]+\*?', Name.Namespace, '#pop')
        ],
    }


class CeylonLexer(RegexLexer):
    """
    For `Ceylon <http://ceylon-lang.org/>`_ source code.

    *New in Pygments 1.6.*
    """

    name = 'Ceylon'
    aliases = ['ceylon']
    filenames = ['*.ceylon']
    mimetypes = ['text/x-ceylon']

    flags = re.MULTILINE | re.DOTALL

    #: optional Comment or Whitespace
    _ws = r'(?:\s|//.*?\n|/[*].*?[*]/)+'

    tokens = {
        'root': [
            # method names
            (r'^(\s*(?:[a-zA-Z_][a-zA-Z0-9_\.\[\]]*\s+)+?)' # return arguments
             r'([a-zA-Z_][a-zA-Z0-9_]*)'                    # method name
             r'(\s*)(\()',                                  # signature start
             bygroups(using(this), Name.Function, Text, Operator)),
            (r'[^\S\n]+', Text),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'(variable|shared|abstract|doc|by|formal|actual)',
             Name.Decorator),
            (r'(break|case|catch|continue|default|else|finally|for|in|'
             r'variable|if|return|switch|this|throw|try|while|is|exists|'
             r'nonempty|then|outer)\b', Keyword),
            (r'(abstracts|extends|satisfies|adapts|'
             r'super|given|of|out|assign|'
             r'transient|volatile)\b', Keyword.Declaration),
            (r'(function|value|void)\b',
             Keyword.Type),
            (r'(package)(\s+)', bygroups(Keyword.Namespace, Text)),
            (r'(true|false|null)\b', Keyword.Constant),
            (r'(class|interface|object)(\s+)',
             bygroups(Keyword.Declaration, Text), 'class'),
            (r'(import)(\s+)', bygroups(Keyword.Namespace, Text), 'import'),
            (r'"(\\\\|\\"|[^"])*"', String),
            (r"'\\.'|'[^\\]'|'\\u[0-9a-fA-F]{4}'", String.Quoted),
            (r"`\\.`|`[^\\]`|`\\u[0-9a-fA-F]{4}`", String.Char),
            (r'(\.)([a-zA-Z_][a-zA-Z0-9_]*)',
             bygroups(Operator, Name.Attribute)),
            (r'[a-zA-Z_][a-zA-Z0-9_]*:', Name.Label),
            (r'[a-zA-Z_\$][a-zA-Z0-9_]*', Name),
            (r'[~\^\*!%&\[\]\(\)\{\}<>\|+=:;,./?-]', Operator),
            (r'\d{1,3}(_\d{3})+\.\d{1,3}(_\d{3})+[kMGTPmunpf]?', Number.Float),
            (r'\d{1,3}(_\d{3})+\.[0-9]+([eE][+-]?[0-9]+)?[kMGTPmunpf]?',
             Number.Float),
            (r'[0-9][0-9]*\.\d{1,3}(_\d{3})+[kMGTPmunpf]?', Number.Float),
            (r'[0-9][0-9]*\.[0-9]+([eE][+-]?[0-9]+)?[kMGTPmunpf]?',
             Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'\d{1,3}(_\d{3})+[kMGTP]?', Number.Integer),
            (r'[0-9]+[kMGTP]?', Number.Integer),
            (r'\n', Text)
        ],
        'class': [
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name.Class, '#pop')
        ],
        'import': [
            (r'[a-zA-Z0-9_.]+\w+ \{([a-zA-Z,]+|\.\.\.)\}',
             Name.Namespace, '#pop')
        ],
    }


class KotlinLexer(RegexLexer):
    """
    For `Kotlin <http://confluence.jetbrains.net/display/Kotlin/>`_
    source code.

    Additional options accepted:

    `unicodelevel`
      Determines which Unicode characters this lexer allows for identifiers.
      The possible values are:

      * ``none`` -- only the ASCII letters and numbers are allowed. This
        is the fastest selection.
      * ``basic`` -- all Unicode characters from the specification except
        category ``Lo`` are allowed.
      * ``full`` -- all Unicode characters as specified in the C# specs
        are allowed.  Note that this means a considerable slowdown since the
        ``Lo`` category has more than 40,000 characters in it!

      The default value is ``basic``.

    *New in Pygments 1.5.*
    """

    name = 'Kotlin'
    aliases = ['kotlin']
    filenames = ['*.kt']
    mimetypes = ['text/x-kotlin'] # inferred

    flags = re.MULTILINE | re.DOTALL | re.UNICODE

    # for the range of allowed unicode characters in identifiers,
    # see http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-334.pdf

    levels = {
        'none': '@?[_a-zA-Z][a-zA-Z0-9_]*',
        'basic': ('@?[_' + uni.Lu + uni.Ll + uni.Lt + uni.Lm + uni.Nl + ']' +
                  '[' + uni.Lu + uni.Ll + uni.Lt + uni.Lm + uni.Nl +
                  uni.Nd + uni.Pc + uni.Cf + uni.Mn + uni.Mc + ']*'),
        'full': ('@?(?:_|[^' +
                 uni.allexcept('Lu', 'Ll', 'Lt', 'Lm', 'Lo', 'Nl') + '])'
                 + '[^' + uni.allexcept('Lu', 'Ll', 'Lt', 'Lm', 'Lo', 'Nl',
                                        'Nd', 'Pc', 'Cf', 'Mn', 'Mc') + ']*'),
    }

    tokens = {}
    token_variants = True

    for levelname, cs_ident in levels.items():
        tokens[levelname] = {
            'root': [
                # method names
                (r'^([ \t]*(?:' + cs_ident + r'(?:\[\])?\s+)+?)' # return type
                 r'(' + cs_ident + ')'                           # method name
                 r'(\s*)(\()',                               # signature start
                 bygroups(using(this), Name.Function, Text, Punctuation)),
                (r'^\s*\[.*?\]', Name.Attribute),
                (r'[^\S\n]+', Text),
                (r'\\\n', Text), # line continuation
                (r'//.*?\n', Comment.Single),
                (r'/[*](.|\n)*?[*]/', Comment.Multiline),
                (r'\n', Text),
                (r'[~!%^&*()+=|\[\]:;,.<>/?-]', Punctuation),
                (r'[{}]', Punctuation),
                (r'@"(""|[^"])*"', String),
                (r'"(\\\\|\\"|[^"\n])*["\n]', String),
                (r"'\\.'|'[^\\]'", String.Char),
                (r"[0-9](\.[0-9]*)?([eE][+-][0-9]+)?"
                 r"[flFLdD]?|0[xX][0-9a-fA-F]+[Ll]?", Number),
                (r'#[ \t]*(if|endif|else|elif|define|undef|'
                 r'line|error|warning|region|endregion|pragma)\b.*?\n',
                 Comment.Preproc),
                (r'\b(extern)(\s+)(alias)\b', bygroups(Keyword, Text,
                 Keyword)),
                (r'(abstract|as|break|catch|'
                 r'fun|continue|default|delegate|'
                 r'do|else|enum|extern|false|finally|'
                 r'fixed|for|goto|if|implicit|in|interface|'
                 r'internal|is|lock|null|'
                 r'out|override|private|protected|public|readonly|'
                 r'ref|return|sealed|sizeof|'
                 r'when|this|throw|true|try|typeof|'
                 r'unchecked|unsafe|virtual|void|while|'
                 r'get|set|new|partial|yield|val|var)\b', Keyword),
                (r'(global)(::)', bygroups(Keyword, Punctuation)),
                (r'(bool|byte|char|decimal|double|dynamic|float|int|long|'
                 r'short)\b\??', Keyword.Type),
                (r'(class|struct)(\s+)', bygroups(Keyword, Text), 'class'),
                (r'(package|using)(\s+)', bygroups(Keyword, Text), 'package'),
                (cs_ident, Name),
            ],
            'class': [
                (cs_ident, Name.Class, '#pop')
            ],
            'package': [
                (r'(?=\()', Text, '#pop'), # using (resource)
                ('(' + cs_ident + r'|\.)+', Name.Namespace, '#pop')
            ]
        }

    def __init__(self, **options):
        level = get_choice_opt(options, 'unicodelevel', self.tokens.keys(),
                               'basic')
        if level not in self._all_tokens:
            # compile the regexes now
            self._tokens = self.__class__.process_tokendef(level)
        else:
            self._tokens = self._all_tokens[level]

        RegexLexer.__init__(self, **options)


class XtendLexer(RegexLexer):
    """
    For `Xtend <http://xtend-lang.org/>`_ source code.

    *New in Pygments 1.6.*
    """

    name = 'Xtend'
    aliases = ['xtend']
    filenames = ['*.xtend']
    mimetypes = ['text/x-xtend']

    flags = re.MULTILINE | re.DOTALL

    tokens = {
        'root': [
            # method names
            (r'^(\s*(?:[a-zA-Z_][a-zA-Z0-9_\.\[\]]*\s+)+?)' # return arguments
             r'([a-zA-Z_$][a-zA-Z0-9_$]*)'                    # method name
             r'(\s*)(\()',                                  # signature start
             bygroups(using(this), Name.Function, Text, Operator)),
            (r'[^\S\n]+', Text),
            (r'//.*?\n', Comment.Single),
            (r'/\*.*?\*/', Comment.Multiline),
            (r'@[a-zA-Z_][a-zA-Z0-9_\.]*', Name.Decorator),
            (r'(assert|break|case|catch|continue|default|do|else|finally|for|'
             r'if|goto|instanceof|new|return|switch|this|throw|try|while|IF|'
             r'ELSE|ELSEIF|ENDIF|FOR|ENDFOR|SEPARATOR|BEFORE|AFTER)\b',
             Keyword),
            (r'(def|abstract|const|enum|extends|final|implements|native|private|'
             r'protected|public|static|strictfp|super|synchronized|throws|'
             r'transient|volatile)\b', Keyword.Declaration),
            (r'(boolean|byte|char|double|float|int|long|short|void)\b',
             Keyword.Type),
            (r'(package)(\s+)', bygroups(Keyword.Namespace, Text)),
            (r'(true|false|null)\b', Keyword.Constant),
            (r'(class|interface)(\s+)', bygroups(Keyword.Declaration, Text),
             'class'),
            (r'(import)(\s+)', bygroups(Keyword.Namespace, Text), 'import'),
            (r"(''')", String, 'template'),
            (ur"(\u00BB)", String, 'template'),
            (r'"(\\\\|\\"|[^"])*"', String),
            (r"'(\\\\|\\'|[^'])*'", String),
            (r'[a-zA-Z_][a-zA-Z0-9_]*:', Name.Label),
            (r'[a-zA-Z_\$][a-zA-Z0-9_]*', Name),
            (r'[~\^\*!%&\[\]\(\)\{\}<>\|+=:;,./?-]', Operator),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+L?', Number.Integer),
            (r'\n', Text)
        ],
        'class': [
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name.Class, '#pop')
        ],
        'import': [
            (r'[a-zA-Z0-9_.]+\*?', Name.Namespace, '#pop')
        ],
        'template': [
            (r"'''", String, '#pop'),
            (ur"\u00AB", String, '#pop'),
            (r'.', String)
        ],
    }
