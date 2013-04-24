#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
***************************************************************************
    generate_application_descriptors.py
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


import os, sys
import otbApplication
from qgis.core import QgsApplication

outputpath = unicode( QgsApplication.qgisSettingsDirPath() + "python/plugins/sextante/otb/description" )
endl = os.linesep

def convertendl(s):
    '''Convert a string for compatibility in txt dump'''
    return s.replace('\n', '\\n')

def get_app_list():
  blackList = ["TestApplication"]
  appNames = [app for app in otbApplication.Registry.GetAvailableApplications() if app not in blackList]
  return appNames


def generate_all_app_descriptors( ) :
  appliIdx = 0
  for appliname in get_app_list():
    appliIdx = appliIdx+1
    generate_app_descriptor( appliname )
    #generate_app_doc( appliname )

def generate_app_descriptor( appliname ) :
  print appliname

  appInstance = otbApplication.Registry.CreateApplication(appliname)
  appInstance.UpdateParameters() # TODO need this ?

  out = ""

  # the application key
  out += appliname + endl

  # the executable
  out += "otbcli_" + appliname + endl

  # long name
  out += convertendl(appInstance.GetDocName()) + endl

  # group
  out += get_group(appInstance) + endl

  for paramKey in appInstance.GetParametersKeys():
    pdesc =  generate_param_descriptor(appInstance, paramKey)
    if len(pdesc) > 0:
      out += pdesc + endl

  with open( os.path.join(outputpath, appliname + '.txt'), 'w' ) as outfile:
    outfile.write(out)

def generate_app_doc( appliname ) :
  appInstance = otbApplication.Registry.CreateApplication(appliname)
  appInstance.UpdateParameters() # TODO need this ?
  html = appInstance.GetHtmlExample()

  docpath= os.path.join(outputpath, 'doc')
  if not os.path.exists(docpath):
      os.makedirs(docpath)

  with open( os.path.join(docpath, appliname + '.html'), 'w' ) as outfile:
      outfile.write(html)

def get_group( appInstance ) :
    tags = appInstance.GetDocTags()
    sectionTags = ["Image Manipulation","Vector Data Manipulation", "Calibration","Geometry", "Image Filtering","Feature Extraction","Stereo","Learning","Segmentation"]
    for sectionTag in sectionTags:
        for tag in tags:
            if tag == sectionTag:
                return sectionTag
    return "Miscellaneous"

def generate_param_descriptor( appInstance, paramKey ):
  paramcreationfunction = {
      otbApplication.ParameterType_Empty : generate_parameter_Empty,
      otbApplication.ParameterType_Int : generate_parameter_Int,
      otbApplication.ParameterType_Float : generate_parameter_Float,
      otbApplication.ParameterType_String : generate_parameter_String,
      otbApplication.ParameterType_StringList : generate_parameter_NOTHANDLED,
      otbApplication.ParameterType_InputFilename : generate_parameter_InputFilename,
      otbApplication.ParameterType_OutputFilename : generate_parameter_OutputFilename,
      otbApplication.ParameterType_Directory : generate_parameter_Directory,
      otbApplication.ParameterType_Choice : generate_parameter_Choice,
      otbApplication.ParameterType_InputImage : generate_parameter_InputImage,
      otbApplication.ParameterType_InputImageList : generate_parameter_InputImageList,
      otbApplication.ParameterType_InputVectorData : generate_parameter_InputVectorData,
      otbApplication.ParameterType_InputVectorDataList : generate_parameter_InputVectorDataList,
      otbApplication.ParameterType_OutputImage : generate_parameter_OutputImage,
      otbApplication.ParameterType_OutputVectorData : generate_parameter_OutputVectorData,
      otbApplication.ParameterType_Radius : generate_parameter_Radius,
      otbApplication.ParameterType_Group : generate_parameter_NOTHANDLED,
      otbApplication.ParameterType_ListView : generate_parameter_NOTHANDLED,
      otbApplication.ParameterType_ComplexInputImage : generate_parameter_ComplexInputImage,
      otbApplication.ParameterType_ComplexOutputImage : generate_parameter_ComplexOutputImage,
      otbApplication.ParameterType_RAM : generate_parameter_RAM
      }
  return paramcreationfunction[ appInstance.GetParameterType(paramKey) ](appInstance, paramKey)

def generate_parameter_Empty( appInstance, paramKey ):
  out = "ParameterBoolean"
  out += "|"
  out += "-" + paramKey
  out += "|"
  out += convertendl(appInstance.GetParameterName(paramKey))
  out += "|"
  if appInstance.IsParameterEnabled(paramKey):
    out += "True"
  return out


def generate_parameter_Int( appInstance, paramKey ):
  out = "ParameterNumber"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))
  out += "|"

  out += "None"
  out += "|"
  out += "None"
  out += "|"

  defaultVal = "0"
  try:
    defaultVal = str(appInstance.GetParameterInt(paramKey))
  except:
    pass
  out += defaultVal
  return out


def generate_parameter_Float( appInstance, paramKey ):
  out = "ParameterNumber"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))
  out += "|"

  out += "None"
  out += "|"
  out += "None"
  out += "|"

  defaultVal = "0.0"
  try:
    defaultVal = str(appInstance.GetParameterFloat(paramKey))
  except:
    pass
  out += defaultVal
  return out

def generate_parameter_String( appInstance, paramKey ):
  out = "ParameterString"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))
  out += "|"

  defaultVal = ""
  try:
    defaultVal = str(appInstance.GetParameterString(paramKey))
  except:
    pass
  out += defaultVal
  return out


def generate_parameter_InputFilename( appInstance, paramKey ):
  out = "ParameterFile"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))
  out += "|"

  try:
    defaultVal = str(appInstance.GetParameterString(paramKey))
    out += "|" + defaultVal
  except:
    pass

  return out


def generate_parameter_OutputFilename( appInstance, paramKey ):
  out = "OutputFile"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))

  return out

def generate_parameter_Directory( appInstance, paramKey ):
  out = "ParameterFile"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))

  try:
    defaultVal = str(appInstance.GetParameterString(paramKey))
    out += "|" + defaultVal
  except:
    pass

  return out

def generate_parameter_Choice( appInstance, paramKey ):
  out = "ParameterSelection"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))
  out += "|"

  choices = ""
  for choice in appInstance.GetChoiceKeys(paramKey):
    choices += choice
    choices += ";"
  out += choices[:-1]

  out += "|"
  out += str(appInstance.GetParameterInt(paramKey))

  return out

def generate_parameter_InputImage( appInstance, paramKey ):
  out = "ParameterRaster"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))
  out += "|"

  out += str(not appInstance.IsMandatory(paramKey))

  return out


def generate_parameter_InputImageList( appInstance, paramKey ):
  out = "ParameterMultipleInput"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))
  out += "|"

  out += "3"
  out += "|"

  out += str(not appInstance.IsMandatory(paramKey))

  return out

def generate_parameter_InputVectorData( appInstance, paramKey ):
  out = "ParameterVector"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))
  out += "|"

  out += "-1"
  out += "|"

  out += str(not appInstance.IsMandatory(paramKey))

  return out


def generate_parameter_InputVectorDataList( appInstance, paramKey ):
  out = "ParameterMultipleInput"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))
  out += "|"

  out += "-1"
  out += "|"

  out += str(not appInstance.IsMandatory(paramKey))

  return out


def generate_parameter_OutputImage( appInstance, paramKey ):
  out = "OutputRaster"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))

  return out


def generate_parameter_NOTHANDLED( appInstance, paramKey ):
  return ""


def generate_parameter_OutputVectorData( appInstance, paramKey ):
  out = "OutputVector"
  out += "|"

  out += "-" + paramKey
  out += "|"

  out += convertendl(appInstance.GetParameterName(paramKey))

  return out


def generate_parameter_Radius( appInstance, paramKey ):
  return generate_parameter_Int(appInstance, paramKey)


def generate_parameter_ComplexInputImage( appInstance, paramKey ):
  return generate_parameter_InputImage(appInstance, paramKey)


def generate_parameter_ComplexOutputImage( appInstance, paramKey ):
  return generate_parameter_OutputImage(appInstance, paramKey)


def generate_parameter_RAM( appInstance, paramKey ):
  return generate_parameter_Int(appInstance, paramKey)




if __name__ == "__main__" :
  generate_all_app_descriptors()
