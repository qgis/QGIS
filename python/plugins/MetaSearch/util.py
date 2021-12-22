# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright (C) 2010 NextGIS (http://nextgis.org),
#                    Alexander Bruy (alexander.bruy@gmail.com),
#                    Maxim Dubinin (sim@gis-lab.info)
#
# Copyright (C) 2014 Tom Kralidis (tomkralidis@gmail.com)
# Copyright (C) 2014 Angelos Tzotsos (tzotsos@gmail.com)
#
# This source is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This code is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
###############################################################################

from gettext import gettext, ngettext
import json
import logging
import warnings
import os
import webbrowser
from xml.dom.minidom import parseString
import xml.etree.ElementTree as etree

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    from jinja2 import Environment, FileSystemLoader

from pygments import highlight
from pygments.lexers import JsonLexer, XmlLexer
from pygments.formatters import HtmlFormatter
from qgis.PyQt.QtCore import QUrl, QUrlQuery
from qgis.PyQt.QtWidgets import QMessageBox
from qgis.PyQt.uic import loadUiType

from qgis.core import Qgis, QgsSettings

LOGGER = logging.getLogger('MetaSearch')


class StaticContext(object):
    """base configuration / scaffolding"""

    def __init__(self):
        """init"""
        self.ppath = os.path.dirname(os.path.abspath(__file__))


def get_ui_class(ui_file):
    """return class object of a uifile"""
    ui_file_full = '%s%sui%s%s' % (os.path.dirname(os.path.abspath(__file__)),
                                   os.sep, os.sep, ui_file)
    return loadUiType(ui_file_full)[0]


def render_template(language, context, data, template):
    """Renders HTML display of raw API request/response/content"""

    env = Environment(extensions=['jinja2.ext.i18n'],
                      loader=FileSystemLoader(context.ppath))
    env.install_gettext_callables(gettext, ngettext, newstyle=True)

    template_file = 'resources/templates/%s' % template
    template = env.get_template(template_file)
    return template.render(language=language, obj=data)


def get_connections_from_file(parent, filename):
    """load connections from connection file"""

    error = 0
    try:
        doc = etree.parse(filename).getroot()
        if doc.tag != 'qgsCSWConnections':
            error = 1
            msg = parent.tr('Invalid Catalog connections XML.')
    except etree.ParseError as err:
        error = 1
        msg = parent.tr('Cannot parse XML file: {0}').format(err)
    except IOError as err:
        error = 1
        msg = parent.tr('Cannot open file: {0}').format(err)

    if error == 1:
        QMessageBox.information(parent, parent.tr('Loading Connections'), msg)
        return
    return doc


def prettify_xml(xml):
    """convenience function to prettify XML"""

    if isinstance(xml, bytes):
        xml = xml.decode('utf-8')

    if xml.count('\n') > 20:  # likely already pretty printed
        return xml

    # check if it's a GET request
    if xml.startswith('http'):
        return xml
    else:
        return parseString(xml).toprettyxml()


def highlight_content(context, content, mimetype):
    """render content as highlighted HTML"""

    hformat = HtmlFormatter()
    css = hformat.get_style_defs('.highlight')
    if mimetype == 'json':
        body = highlight(json.dumps(content, indent=4), JsonLexer(), hformat)
    elif mimetype == 'xml':
        body = highlight(prettify_xml(content), XmlLexer(), hformat)

    env = Environment(loader=FileSystemLoader(context.ppath))

    template_file = 'resources/templates/api_highlight.html'
    template = env.get_template(template_file)
    return template.render(css=css, body=body)


def get_help_url():
    """return QGIS MetaSearch help documentation link"""

    locale_name = QgsSettings().value('locale/userLocale')[0:2]
    major, minor = Qgis.QGIS_VERSION.split('.')[:2]

    if minor == '99':  # master
        version = 'testing'
    else:
        version = '.'.join([major, minor])

    path = '%s/%s/docs/user_manual/plugins/core_plugins/plugins_metasearch.html' % (version, locale_name)  # noqa

    return '/'.join(['https://docs.qgis.org', path])


def open_url(url):
    """open URL in web browser"""

    webbrowser.open(url)


def normalize_text(text):
    """tidy up string"""

    return text.replace('\n', '')


def serialize_string(input_string):
    """apply a serial counter to a string"""

    s = input_string.strip().split()

    last_token = s[-1]
    all_other_tokens_as_string = input_string.replace(last_token, '')

    if last_token.isdigit():
        value = '%s%s' % (all_other_tokens_as_string, int(last_token) + 1)
    else:
        value = '%s 1' % input_string

    return value


def clean_ows_url(url):
    """clean an OWS URL of added basic service parameters"""

    url = QUrl(url)
    query_string = url.query()

    if query_string:
        query_string = QUrlQuery(query_string)
        query_string.removeQueryItem('service')
        query_string.removeQueryItem('SERVICE')
        query_string.removeQueryItem('request')
        query_string.removeQueryItem('REQUEST')
        url.setQuery(query_string)

    return url.toString()
