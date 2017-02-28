# script parameters
##Dissolve with Stats=name
##Vector=group
##Input_layer=vector
##Dissolve_field=field Input_layer
##Statistics=string
##Output_layer=output vector

from processing.core.parameters import Parameter
from PyQt4 import QtCore
from qgis.core import *
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.tools.vector import VectorWriter
import os.path
import processing
import sys
import time
import math

# run dissolve processing algorithm
dissolve = processing.runalg("qgis:dissolve", Input_layer, "false", Dissolve_field, Output_layer)
# get output of dissolve algorithm
dissolveLayer = dissolve['OUTPUT']
dissolveLayer = processing.getObject(dissolveLayer)
# get list of statistics to compute for each field
listStats = Statistics.split(';')
# get object for input layer
inputLayer = processing.getObject(Input_layer)

def validation():
    # verifies that number of statistics = number of fields in input layer
    nbStats = len(listStats)
    provider = inputLayer.dataProvider()
    fields = provider.fields()
    nbFields = len(fields)
    if nbStats != nbFields:
        raise GeoAlgorithmExecutionException('Number of statistics is not equal to number of fields in input layer ; please check again.')

# once the dissolve output layer is created, calculates its new attributes values
def calculateFields(listStats, output):
    # iterates over input layer features to get attributes as a list of lists
    # uses the processing method so as to get only selected features if this option is set in the processing options
    iter = processing.features(inputLayer)
    attrs = [feature.attributes() for feature in iter]
    # get index of dissolve field
    provider = inputLayer.dataProvider()
    fields = provider.fields()
    listFieldNames = [field.name() for field in fields]
    indexDissolveField = listFieldNames.index(Dissolve_field)
    # get all values of the dissolve field (before processing : with duplicate values)
    valuesDissolveField = [feature[indexDissolveField] for feature in attrs]
    # get unique values for dissolve field, from output (seems more secure than to get it from valuesDissolveField ?)
    outputLayer = QgsVectorLayer(output, "name", "ogr")
    provider = dissolveLayer.dataProvider()
    fields = provider.fields()
    listFieldNames = [field.name() for field in fields]
    iter = outputLayer.getFeatures()
    uniqueValuesDissolveField = [feature.attributes()[indexDissolveField] for feature in iter]
    # initializes list of lists which will contain results (it will have one element per kept field)
    listRes = []
    # for each kept field
    for i in range(len(listFieldNames)):
        if listStats[i]  != 'no':
            # creates list which will contain attribute values for current field, one empty element per unique dissolve field value
            listAttrs = [[] for val in range(len(uniqueValuesDissolveField))]
            # fill this list with all the current field values corresponding to each dissolve field value
            valuesField = [feature[i] for feature in attrs]
            for (x,y) in zip(valuesDissolveField, valuesField):
                listAttrs[uniqueValuesDissolveField.index(x)].append(y)
            # removes any NULL values
            listAttrs = [[x for x in l if x] for l in listAttrs]
            # for each list in listAttrs, calculates one value according to the chosen stat
            # if list is empty (can happen if it contained originally only NULL values), return NULL as a result
            if listStats[i] == "mean":
                listAttrs = [sum(y) / len(y) if y else NULL for y in listAttrs]
            elif listStats[i] == "sum":
                listAttrs = [sum(y) if y else NULL for y in listAttrs]
            elif listStats[i] == "min":
                listAttrs = [min(y) if y else NULL for y in listAttrs]
            elif listStats[i] == "max":
                listAttrs = [max(y) if y else NULL for y in listAttrs]
            elif listStats[i] == "count":
                listAttrs = [len(y) if y else NULL for y in listAttrs]
            elif listStats[i] == "first":
                listAttrs = [y[0] if y else NULL for y in listAttrs]
            elif listStats[i] == "last":
                listAttrs = [y[-1] if y else NULL for y in listAttrs]
            elif listStats[i] == "median":
                listAttrs = [self.median(y) if y else NULL for y in listAttrs]
            elif listStats[i] == "sd":
                listAttrs = [self.standard_dev(y) if y else NULL for y in listAttrs]
            elif listStats[i] == "concat":
                listAttrs = [", ".join(y) if y else NULL for y in listAttrs]
            elif listStats[i] == "unique":
                listAttrs = [", ".join(set(y)) if y else NULL for y in listAttrs]
            # append each field result to listRes
            listRes.append(listAttrs)
    return listRes

# removes fields from the output which mustn't be kept, and set the other field values            
def setAttributes(listRes, output):
    # get indexes of fields to be deleted
    listIndexesDel = [i[0] for i in list(enumerate(listStats)) if i[1] == 'no']
    # get layer, provider and provider capabilities
    outputLayer = QgsVectorLayer(output, "name", "ogr")
    provider = outputLayer.dataProvider()
    caps = provider.capabilities()
    # delete fields to be deleted
    if caps & QgsVectorDataProvider.DeleteAttributes:
        res = provider.deleteAttributes(listIndexesDel)
        outputLayer.updateFields()
    # changes other fields attribute values
    fields = provider.fields()
    nb_fields = len(fields)
    outputLayer.startEditing()
    for fieldIndex in  range(nb_fields):
        for fid in range(len(listRes[0])):
            outputLayer.changeAttributeValue(fid, fieldIndex, listRes[fieldIndex][fid])
    outputLayer.commitChanges()

# gets the median from a list of numbers
def median(self, l):
    # sorts list, get list length
    l.sort()
    z = len(l)
    # if the list has an uneven number of elements
    if z%2:
        return l[z/2]
    # if the list has an even number of elements
    else:
        return (l[(z/2)-1] + l[z/2]) / 2.0
            
# gets standard deviation from a list of numbers
def standard_dev(self, l):
    mean = sum(l) / len(l)
    dev = [(x - mean)*(x - mean) for x in l]
    return math.sqrt(sum(dev) / len(l))

validation()
listRes = calculateFields(listStats, Output_layer)
setAttributes(listRes, Output_layer)
    

