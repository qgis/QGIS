# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

class HtmlContent:
        def __init__(self, data):
                self.data = data if not isinstance(data, HtmlContent) else data.data

        def toHtml(self):
                if isinstance(self.data, list) or isinstance(self.data, tuple):
                        html = u''
                        for item in self.data:
                                html += HtmlContent(item).toHtml()
                        return html

                if hasattr(self.data, 'toHtml' ):
                        return self.data.toHtml()

                html = unicode(self.data).replace("\n", "<br>")
                return html

        def hasContents(self):
                if isinstance(self.data, list) or isinstance(self.data, tuple):
                        empty = True
                        for item in self.data:
                                if item.hasContents():
                                        empty = False
                                        break
                        return not empty

                if hasattr(self.data, 'hasContents'):
                        return self.data.hasContents()

                return len(self.data) > 0

class HtmlElem:
        def __init__(self, tag, data, attrs=None):
                self.tag = tag
                self.data = data if isinstance(data, HtmlContent) else HtmlContent(data)
                self.attrs = attrs if attrs is not None else dict()
                if 'tag' in self.attrs:
                        self.setTag( self.attrs['tag'] )
                        del self.attrs['tag']

        def setTag(self, tag):
                self.tag = tag

        def getOriginalData(self):
                return self.data.data

        def setAttr(self, name, value):
                self.attrs[name] = value

        def getAttrsHtml(self):
                html = u''
                for k, v in self.attrs.iteritems():
                        html += u' %s="%s"' % ( k, v )
                return html

        def openTagHtml(self):
                return u"<%s%s>" % ( self.tag, self.getAttrsHtml() )

        def closeTagHtml(self):
                return u"</%s>" % self.tag

        def toHtml(self):
                return u"%s%s%s" % ( self.openTagHtml(), self.data.toHtml(), self.closeTagHtml() )

        def hasContents(self):
                return self.data.toHtml() != ""


class HtmlParagraph(HtmlElem):
        def __init__(self, data, attrs=None):
                HtmlElem.__init__(self, 'p', data, attrs)


class HtmlListItem(HtmlElem):
        def __init__(self, data, attrs=None):
                HtmlElem.__init__(self, 'li', data, attrs)

class HtmlList(HtmlElem):
        def __init__(self, items, attrs=None):
                # make sure to have HtmlListItem items
                items = list(items)
                for i, item in enumerate(items):
                        if not isinstance(item, HtmlListItem):
                                items[i] = HtmlListItem( item )
                HtmlElem.__init__(self, 'ul', items, attrs)


class HtmlTableCol(HtmlElem):
        def __init__(self, data, attrs=None):
                HtmlElem.__init__(self, 'td', data, attrs)

        def closeTagHtml(self):
                # FIX INVALID BEHAVIOR: an empty cell as last table's cell break margins
                return u"&nbsp;%s" % HtmlElem.closeTagHtml(self)

class HtmlTableRow(HtmlElem):
        def __init__(self, cols, attrs=None):
                # make sure to have HtmlTableCol items
                cols = list(cols)
                for i, c in enumerate(cols):
                        if not isinstance(c, HtmlTableCol):
                                cols[i] = HtmlTableCol( c )
                HtmlElem.__init__(self, 'tr', cols, attrs)

class HtmlTableHeader(HtmlTableRow):
        def __init__(self, cols, attrs=None):
                HtmlTableRow.__init__(self, cols, attrs)
                for c in self.getOriginalData():
                        c.setTag('th')

class HtmlTable(HtmlElem):
        def __init__(self, rows, attrs=None):
                # make sure to have HtmlTableRow items
                rows = list(rows)
                for i, r in enumerate(rows):
                        if not isinstance(r, HtmlTableRow):
                                rows[i] = HtmlTableRow( r )
                HtmlElem.__init__(self, 'table', rows, attrs)


class HtmlWarning(HtmlContent):
        def __init__(self, data):
                data = [ '<img src=":/icons/warning-20px.png">&nbsp;&nbsp; ', data ]
                HtmlContent.__init__(self, data)


class HtmlSection(HtmlContent):
        def __init__(self, title, content=None):
                data = [ '<div class="section"><h2>', title, '</h2>' ]
                if content is not None:
                        data.extend( [ '<div>', content, '</div>' ] )
                data.append( '</div>' )
                HtmlContent.__init__(self, data)
