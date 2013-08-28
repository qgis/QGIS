# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : July 2013
    Copyright            : (C) 2013 by Victor Olaya
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
__date__ = 'July 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


from exampleprovider.ExampleAlgorithm import ExampleAlgorithm

from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.core.ProcessingConfig import Setting, ProcessingConfig


class ExampleAlgorithmProvider(AlgorithmProvider):

    MY_DUMMY_SETTING = "MY_DUMMY_SETTING"

    def __init__(self):
        AlgorithmProvider.__init__(self)
        # deactivate provider by default
        self.activate = False
        # load algorithms
        self.alglist = [ExampleAlgorithm()]
        for alg in self.alglist:
            alg.provider = self

    def initializeSettings(self):
        '''In this method we add settings needed to configure our provider.
        Do not forget to call the parent method, since it takes care or
        automatically adding a setting for activating or deactivating the
        algorithms in the provider
        '''
        AlgorithmProvider.initializeSettings(self)
        ProcessingConfig.addSetting(Setting("Example algorithms", ExampleAlgorithmProvider.MY_DUMMY_SETTING, "Example setting", "Default value"))
        '''To get the parameter of a setting parameter, use
        ProcessingConfig.getSetting(name_of_parameter)
        '''

    def unload(self):
        '''Setting should be removed here, so they do not appear anymore
        when the plugin is unloaded'''
        AlgorithmProvider.unload(self)
        ProcessingConfig.removeSetting(ExampleAlgorithmProvider.MY_DUMMY_SETTING)

    def getName(self):
        '''This is the name that will appear on the toolbox group.
        It is also used to create the command line name of all the algorithms
        from this provider
        '''
        return "Example provider"

    def getDescription(self):
        '''This is the provired full name.
        '''
        return "Example algorithms"

    def getIcon(self):
        '''We return the default icon'''
        return AlgorithmProvider.getIcon(self)

    def _loadAlgorithms(self):
        '''Here we fill the list of algorithms in self.algs.
        This method is called whenever the list of algorithms should be updated.
        If the list of algorithms can change
        (for instance, if it contains algorithms from user-defined scripts and
        a new script might have been added), you should create the list again
        here.
        In this case, since the list is always the same, we assign from the pre-made list.
        This assignment has to be done in this method even if the list does not change,
        since the self.algs list is cleared before calling this method
        '''
        self.algs = self.alglist
