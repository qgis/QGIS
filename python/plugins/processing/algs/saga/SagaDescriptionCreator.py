# -*- coding: utf-8 -*-

"""
***************************************************************************
    SagaDescriptionCreator.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import subprocess
import os


class SagaDescriptionCreator(object):

    def createLibraryFiles(self):
        with open('c:\\saga\\sagalibs.txt') as f:
            for lib in f:
                lib = lib.strip('\n')
                command = ['c:\\saga\\saga_cmd.exe', lib]
                with open('c:\\saga\\desc\\' + lib + '.sagalib', 'w') as f2:
                    subprocess.Popen(
                        command,
                        shell=True,
                        stdout=f2,
                        stdin=subprocess.DEVNULL,
                        stderr=subprocess.STDOUT,
                        universal_newlines=True,
                    )

    def createLibraryMap(self):
        self.map = {}
        for libFile in os.listdir('c:\\saga\\desc'):
            if libFile.endswith('sagalib'):
                # fix_print_with_import
                print(libFile)
                algs = []
                with open(os.path.join('c:\\saga\\desc', libFile)) as f:
                    for line in f:
                        line = line.strip('\n').strip(' ')
                        digit = line.split('\t')[0]
                        # fix_print_with_import
                        print(digit)
                        if digit.isdigit():
                            algs.append(digit)
                    self.map[libFile[:-8]] = algs

        # fix_print_with_import
        print(str(self.map))

    def createDescriptionFiles(self):
        for lib in list(self.map.keys()):
            algs = self.map[lib]
            for alg in algs:
                command = ['c:\\saga\\saga_cmd.exe', lib, alg]
                with open('c:\\saga\\desc\\' + lib + '_' + alg + '.txt', 'w') as f:
                    # fix_print_with_import
                    print(str(command))
                    subprocess.Popen(
                        command,
                        shell=True,
                        stdout=f,
                        stdin=subprocess.DEVNULL,
                        stderr=f,
                        universal_newlines=True,
                    )

    def create(self):
        self.createLibraryMap()
        self.createDescriptionFiles()


def main():

    SagaDescriptionCreator().create()


if __name__ == '__main__':
    main()
