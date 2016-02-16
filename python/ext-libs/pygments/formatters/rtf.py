# -*- coding: utf-8 -*-
"""
    pygments.formatters.rtf
    ~~~~~~~~~~~~~~~~~~~~~~~

    A formatter that generates RTF files.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

from pygments.formatter import Formatter


__all__ = ['RtfFormatter']


class RtfFormatter(Formatter):
    """
    Format tokens as RTF markup. This formatter automatically outputs full RTF
    documents with color information and other useful stuff. Perfect for Copy and
    Paste into Microsoft® Word® documents.

    *New in Pygments 0.6.*

    Additional options accepted:

    `style`
        The style to use, can be a string or a Style subclass (default:
        ``'default'``).

    `fontface`
        The used font famliy, for example ``Bitstream Vera Sans``. Defaults to
        some generic font which is supposed to have fixed width.
    """
    name = 'RTF'
    aliases = ['rtf']
    filenames = ['*.rtf']

    unicodeoutput = False

    def __init__(self, **options):
        """
        Additional options accepted:

        ``fontface``
            Name of the font used. Could for example be ``'Courier New'``
            to further specify the default which is ``'\fmodern'``. The RTF
            specification claims that ``\fmodern`` are "Fixed-pitch serif
            and sans serif fonts". Hope every RTF implementation thinks
            the same about modern...
        """
        Formatter.__init__(self, **options)
        self.fontface = options.get('fontface') or ''

    def _escape(self, text):
        return text.replace('\\', '\\\\') \
                   .replace('{', '\\{') \
                   .replace('}', '\\}')

    def _escape_text(self, text):
        # empty strings, should give a small performance improvment
        if not text:
            return ''

        # escape text
        text = self._escape(text)
        if self.encoding in ('utf-8', 'utf-16', 'utf-32'):
            encoding = 'iso-8859-15'
        else:
            encoding = self.encoding or 'iso-8859-15'

        buf = []
        for c in text:
            if ord(c) > 128:
                ansic = c.encode(encoding, 'ignore') or '?'
                if ord(ansic) > 128:
                    ansic = '\\\'%x' % ord(ansic)
                else:
                    ansic = c
                buf.append(r'\ud{\u%d%s}' % (ord(c), ansic))
            else:
                buf.append(str(c))

        return ''.join(buf).replace('\n', '\\par\n')

    def format_unencoded(self, tokensource, outfile):
        # rtf 1.8 header
        outfile.write(r'{\rtf1\ansi\deff0'
                      r'{\fonttbl{\f0\fmodern\fprq1\fcharset0%s;}}'
                      r'{\colortbl;' % (self.fontface and
                                        ' ' + self._escape(self.fontface) or
                                        ''))

        # convert colors and save them in a mapping to access them later.
        color_mapping = {}
        offset = 1
        for _, style in self.style:
            for color in style['color'], style['bgcolor'], style['border']:
                if color and color not in color_mapping:
                    color_mapping[color] = offset
                    outfile.write(r'\red%d\green%d\blue%d;' % (
                        int(color[0:2], 16),
                        int(color[2:4], 16),
                        int(color[4:6], 16)
                    ))
                    offset += 1
        outfile.write(r'}\f0')

        # highlight stream
        for ttype, value in tokensource:
            while not self.style.styles_token(ttype) and ttype.parent:
                ttype = ttype.parent
            style = self.style.style_for_token(ttype)
            buf = []
            if style['bgcolor']:
                buf.append(r'\cb%d' % color_mapping[style['bgcolor']])
            if style['color']:
                buf.append(r'\cf%d' % color_mapping[style['color']])
            if style['bold']:
                buf.append(r'\b')
            if style['italic']:
                buf.append(r'\i')
            if style['underline']:
                buf.append(r'\ul')
            if style['border']:
                buf.append(r'\chbrdr\chcfpat%d' %
                           color_mapping[style['border']])
            start = ''.join(buf)
            if start:
                outfile.write('{%s ' % start)
            outfile.write(self._escape_text(value))
            if start:
                outfile.write('}')

        outfile.write('}')
