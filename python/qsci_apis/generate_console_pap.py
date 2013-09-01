# -*- coding:utf-8 -*-
"""
/***************************************************************************
Module to generate prepared APIs for calltips and auto-completion.
                             -------------------
begin                : 2013-08-29
copyright            : (C) 2013 Larry Shaffer
email                : larrys (at) dakotacarto (dot) com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
Portions of this file contain code from Eric4 APIsManager module.
"""

import sys
import os

from PyQt4.Qsci import QsciLexerPython, QsciAPIs
from PyQt4.QtGui import QApplication
from PyQt4.QtCore import QObject


class PrepareAPIs(QObject):
    def __init__(self, api_lexer, api_files, pap_file):
        QObject.__init__(self)
        self._api = None
        self._api_files = api_files
        self._api_lexer = api_lexer
        self._pap_file = pap_file

    def _clearLexer(self):
        self.qlexer = None

    def _stopPreparation(self):
        if self._api is not None:
            self._api.cancelPreparation()
        self._api = None
        sys.exit(1)

    def _preparationFinished(self):
        self._clearLexer()
        try:
            if os.path.exists(self._pap_file):
                os.remove(self._pap_file)
            prepd = self._api.savePrepared(unicode(self._pap_file))
            self._api = None
            sys.exit(0 if prepd else 1)
        except Exception, err:
            self._api = None
            sys.exit(1)

    def prepareAPI(self):
        try:
            self._api = QsciAPIs(self._api_lexer)
            self._api.apiPreparationFinished.connect(self._preparationFinished)
            for api_file in self._api_files:
                self._api.load(unicode(api_file))
            self._api.prepare()
        except Exception, err:
            self._api = None
            sys.exit(1)


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print 'Usage: python <script> <pap_file> <apis_src_dir> <api_bin_dir>'
        sys.exit(1)
    pap_file = sys.argv[1]
    api_src_dir = sys.argv[2]
    api_bin_dir = sys.argv[3]

    api_files = [
        os.path.join(api_bin_dir, 'PyQGIS.api'),
        os.path.join(api_src_dir, 'Python-2.7.api'),
        os.path.join(api_src_dir, 'PyQt4-4.7.4.api'),
        os.path.join(api_src_dir, 'GEOS-3.4.2.api'),
        os.path.join(api_src_dir, 'OSGeo_GDAL-OGR-1.9.1.api')
    ]
    # print api_files.__repr__()
    # print pap_file.__repr__()

    app = QApplication(sys.argv, False)  # just start a non-gui console app
    api_lexer = QsciLexerPython()
    prepap = PrepareAPIs(api_lexer, api_files, pap_file)
    prepap.prepareAPI()

    sys.exit(app.exec_())
