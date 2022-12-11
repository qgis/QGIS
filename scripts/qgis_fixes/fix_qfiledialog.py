# -*- coding: utf-8 -*-
"""
Migrate QFileDialog methods from PyQt4 to PyQt5
"""
# Author: Médéric Ribreux <mederic.ribreux@medspx.fr>
# Adapted from fix_pyqt
# and http://python3porting.com/fixers.html

# Local imports
from lib2to3.fixer_base import BaseFix


class FixQfiledialog(BaseFix):

    PATTERN = """
    power< 'QFileDialog' trailer< '.' filter=('getOpenFileNameAndFilter'|'getOpenFileNamesAndFilter'|'getSaveFileNameAndFilter') > any >
    |
    expr_stmt< filename=any '=' power< any trailer< '(' power< 'QFileDialog' trailer< '.' method=('getOpenFileName'|'getOpenFileNames'|'getSaveFileName') > any > ')' > > >
    |
    expr_stmt< filename=any '=' power< 'QFileDialog' trailer< '.' method=('getOpenFileName'|'getOpenFileNames'|'getSaveFileName') > any > >
    """

    def transform(self, node, results):
        # First case: getOpen/SaveFileName
        # We need to add __ variable because in PyQt5
        # getOpen/SaveFileName returns a tuple
        if 'filename' in results:
            node = results['filename']

            # count number of leaves (result variables)
            nbLeaves = sum(1 for i in node.leaves())

            # If we have less than two args,
            # we add __ special variable
            if nbLeaves < 3:
                fileName = node.value
                node.value = '{}, __'.format(fileName)
                node.changed()

        # Rename *AndFilter methods
        if 'filter' in results:
            method = results['filter'][0]
            method.value = method.value.replace('AndFilter', '')
            method.changed()
