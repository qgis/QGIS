# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

nodistdir : `pwd`
noinst_PROGRAMS = testqgsapplication \
		testqgsapplication \
		testqgscolortable \
		testqgscontexthelp \
		testqgscustomsymbol \
		testqgsdatamanager \
		testqgsdatasource \
		testqgsfeature \
		testqgsfeatureattribute \
		testqgsfield \
		testqgsgeometry \
		testqgsgeometryvertexindex \
		testqgsline \
		testqgslinesymbol \
		testqgsmarkercatalogue \
		testqgsnumericsortlistviewitem \
		testqgspluginitem \
		testqgspluginregistry \
		testqgspoint \
		testqgspolygonsymbol \
		testqgsprovidercountcalcevent \
		testqgsproviderextentcalcevent \
		testqgsprovidermetadata \
		testqgsproviderregistry \
		testqgsrangerenderitem \
		testqgsrasterdataprovider \
		testqgsrect \
		testqgsrenderer \
		testqgsrenderitem \
		testqgsscalecalculator \
		testqgssearchstring \
		testqgssearchtreenode \
		testqgssymbol \
		testqgsvectordataprovider 
# testqgslabelattributes 
# testqgsdistancearea 
# testqgsbookmarkitem 

#
# Define some global variables that will be used for building each test
#

GLOBALLDADD = $(QT_LDADD) \
				$(PG_LIB) \
				$(GDAL_LDADD) \
				-lproj \
				../../../src/core/libqgis_core.la \
				../../../src/raster/libqgis_raster.la \
				../../../src/legend/libqgis_legend.la \
				../../../src/gui/libqgis_gui.la \
				../../../src/composer/libqgis_composer.la
GLOBALCXXFLAGS = $(CXXFLAGS) \
				$(EXTRA_CXXFLAGS) \
				$(GDAL_CFLAGS) \
				$(QT_CXXFLAGS) \
        $(GEOS_CFLAGS) \
				$(PG_INC) \
				-I../../../src/raster \
				-I../../../src/core \
				-I../../../src/gui

#
# Instruction for running the qt4 meta object compiler 
#

%.moc.cpp: %.cpp
	$(MOC) -o $@ $<

BUILT_SOURCES = $(testqgsapplication_MOC) \
		$(testqgsbookmarkitem_MOC) \
		$(testqgscolortable_MOC) \
		$(testqgscontexthelp_MOC) \
		$(testqgscustomsymbol_MOC) \
		$(testqgsdatamanager_MOC) \
		$(testqgsdatasource_MOC) \
		$(testqgsdistancearea_MOC) \
		$(testqgsfeatureattribute_MOC) \
		$(testqgsfeature_MOC) \
		$(testqgsfield_MOC) \
		$(testqgsgeometry_MOC) \
		$(testqgsgeometryvertexindex_MOC) \
		$(testqgslabelattributes_MOC) \
		$(testqgsline_MOC) \
		$(testqgslinesymbol_MOC) \
		$(testqgsmarkercatalogue_MOC) \
		$(testqgsnumericsortlistviewitem_MOC) \
		$(testqgspluginitem_MOC) \
		$(testqgspluginregistry_MOC) \
		$(testqgspoint_MOC) \
		$(testqgspolygonsymbol_MOC) \
		$(testqgsprovidercountcalcevent_MOC) \
		$(testqgsproviderextentcalcevent_MOC) \
		$(testqgsprovidermetadata_MOC) \
		$(testqgsproviderregistry_MOC) \
		$(testqgsrangerenderitem_MOC) \
		$(testqgsrasterdataprovider_MOC) \
		$(testqgsrect_MOC) \
		$(testqgsrenderer_MOC) \
		$(testqgsrenderitem_MOC) \
		$(testqgsscalecalculator_MOC) \
		$(testqgssearchstring_MOC) \
		$(testqgssearchtreenode_MOC) \
		$(testqgssymbol_MOC) \
		$(testqgsvectordataprovider_MOC) 

CLEANFILES = $(BUILT_SOURCES)

#
# Specify the compilation files for each unit test now
#

testqgsapplication_MOC = testqgsapplication.moc.cpp
testqgsapplication_SOURCES = testqgsapplication.cpp 
testqgsapplication_LDADD = $(GLOBALLDADD)
testqgsapplication_CXXFLAGS =  $(GLOBALCXXFLAGS)


testqgssymbol_MOC = testqgssymbol.moc.cpp
testqgssymbol_SOURCES = testqgssymbol.cpp 
testqgssymbol_LDADD = $(GLOBALLDADD)
testqgssymbol_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsfield_MOC = testqgsfield.moc.cpp
testqgsfield_SOURCES = testqgsfield.cpp
testqgsfield_LDADD = $(GLOBALLDADD)
testqgsfield_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsfeature_MOC = testqgsfeature.moc.cpp
testqgsfeature_SOURCES = testqgsfeature.cpp
testqgsfeature_LDADD = $(GLOBALLDADD)
testqgsfeature_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsgeometry_MOC = testqgsgeometry.moc.cpp
testqgsgeometry_SOURCES = testqgsgeometry.cpp
testqgsgeometry_LDADD = $(GLOBALLDADD)
testqgsgeometry_CXXFLAGS =  $(GLOBALCXXFLAGS)

#testqgsbookmarkitem_MOC = testqgsbookmarkitem.moc.cpp
#testqgsbookmarkitem_SOURCES = testqgsbookmarkitem.cpp
#testqgsbookmarkitem_LDADD = $(GLOBALLDADD)
#testqgsbookmarkitem_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgscolortable_MOC = testqgscolortable.moc.cpp
testqgscolortable_SOURCES = testqgscolortable.cpp
testqgscolortable_LDADD = $(GLOBALLDADD)
testqgscolortable_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgscontexthelp_MOC = testqgscontexthelp.moc.cpp
testqgscontexthelp_SOURCES = testqgscontexthelp.cpp
testqgscontexthelp_LDADD = $(GLOBALLDADD)
testqgscontexthelp_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgscustomsymbol_MOC = testqgscustomsymbol.moc.cpp
testqgscustomsymbol_SOURCES = testqgscustomsymbol.cpp
testqgscustomsymbol_LDADD = $(GLOBALLDADD)
testqgscustomsymbol_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsdatamanager_MOC = testqgsdatamanager.moc.cpp
testqgsdatamanager_SOURCES = testqgsdatamanager.cpp
testqgsdatamanager_LDADD = $(GLOBALLDADD)
testqgsdatamanager_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsdatasource_MOC = testqgsdatasource.moc.cpp
testqgsdatasource_SOURCES = testqgsdatasource.cpp
testqgsdatasource_LDADD = $(GLOBALLDADD)
testqgsdatasource_CXXFLAGS =  $(GLOBALCXXFLAGS)

#testqgsdistancearea_MOC = testqgsdistancearea.moc.cpp
#testqgsdistancearea_SOURCES = testqgsdistancearea.cpp
#testqgsdistancearea_LDADD = $(GLOBALLDADD)
#testqgsdistancearea_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsfeatureattribute_MOC = testqgsfeatureattribute.moc.cpp
testqgsfeatureattribute_SOURCES = testqgsfeatureattribute.cpp
testqgsfeatureattribute_LDADD = $(GLOBALLDADD)
testqgsfeatureattribute_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsgeometryvertexindex_MOC = testqgsgeometryvertexindex.moc.cpp
testqgsgeometryvertexindex_SOURCES = testqgsgeometryvertexindex.cpp
testqgsgeometryvertexindex_LDADD = $(GLOBALLDADD)
testqgsgeometryvertexindex_CXXFLAGS =  $(GLOBALCXXFLAGS)

#testqgslabelattributes_MOC = testqgslabelattributes.moc.cpp
#testqgslabelattributes_SOURCES = testqgslabelattributes.cpp
#testqgslabelattributes_LDADD = $(GLOBALLDADD)
#testqgslabelattributes_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsline_MOC = testqgsline.moc.cpp
testqgsline_SOURCES = testqgsline.cpp
testqgsline_LDADD = $(GLOBALLDADD)
testqgsline_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgslinesymbol_MOC = testqgslinesymbol.moc.cpp
testqgslinesymbol_SOURCES = testqgslinesymbol.cpp
testqgslinesymbol_LDADD = $(GLOBALLDADD)
testqgslinesymbol_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmarkercatalogue_MOC = testqgsmarkercatalogue.moc.cpp
testqgsmarkercatalogue_SOURCES = testqgsmarkercatalogue.cpp
testqgsmarkercatalogue_LDADD = $(GLOBALLDADD)
testqgsmarkercatalogue_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsnumericsortlistviewitem_MOC = testqgsnumericsortlistviewitem.moc.cpp
testqgsnumericsortlistviewitem_SOURCES = testqgsnumericsortlistviewitem.cpp
testqgsnumericsortlistviewitem_LDADD = $(GLOBALLDADD)
testqgsnumericsortlistviewitem_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgspluginitem_MOC = testqgspluginitem.moc.cpp
testqgspluginitem_SOURCES = testqgspluginitem.cpp
testqgspluginitem_LDADD = $(GLOBALLDADD)
testqgspluginitem_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgspluginregistry_MOC = testqgspluginregistry.moc.cpp
testqgspluginregistry_SOURCES = testqgspluginregistry.cpp
testqgspluginregistry_LDADD = $(GLOBALLDADD)
testqgspluginregistry_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgspoint_MOC = testqgspoint.moc.cpp
testqgspoint_SOURCES = testqgspoint.cpp
testqgspoint_LDADD = $(GLOBALLDADD)
testqgspoint_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgspolygonsymbol_MOC = testqgspolygonsymbol.moc.cpp
testqgspolygonsymbol_SOURCES = testqgspolygonsymbol.cpp
testqgspolygonsymbol_LDADD = $(GLOBALLDADD)
testqgspolygonsymbol_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsprovidercountcalcevent_MOC = testqgsprovidercountcalcevent.moc.cpp
testqgsprovidercountcalcevent_SOURCES = testqgsprovidercountcalcevent.cpp
testqgsprovidercountcalcevent_LDADD = $(GLOBALLDADD)
testqgsprovidercountcalcevent_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsproviderextentcalcevent_MOC = testqgsproviderextentcalcevent.moc.cpp
testqgsproviderextentcalcevent_SOURCES = testqgsproviderextentcalcevent.cpp
testqgsproviderextentcalcevent_LDADD = $(GLOBALLDADD)
testqgsproviderextentcalcevent_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsprovidermetadata_MOC = testqgsprovidermetadata.moc.cpp
testqgsprovidermetadata_SOURCES = testqgsprovidermetadata.cpp
testqgsprovidermetadata_LDADD = $(GLOBALLDADD)
testqgsprovidermetadata_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsproviderregistry_MOC = testqgsproviderregistry.moc.cpp
testqgsproviderregistry_SOURCES = testqgsproviderregistry.cpp
testqgsproviderregistry_LDADD = $(GLOBALLDADD)
testqgsproviderregistry_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsrangerenderitem_MOC = testqgsrangerenderitem.moc.cpp
testqgsrangerenderitem_SOURCES = testqgsrangerenderitem.cpp
testqgsrangerenderitem_LDADD = $(GLOBALLDADD)
testqgsrangerenderitem_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsrasterdataprovider_MOC = testqgsrasterdataprovider.moc.cpp
testqgsrasterdataprovider_SOURCES = testqgsrasterdataprovider.cpp
testqgsrasterdataprovider_LDADD = $(GLOBALLDADD)
testqgsrasterdataprovider_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsrect_MOC = testqgsrect.moc.cpp
testqgsrect_SOURCES = testqgsrect.cpp
testqgsrect_LDADD = $(GLOBALLDADD)
testqgsrect_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsrenderer_MOC = testqgsrenderer.moc.cpp
testqgsrenderer_SOURCES = testqgsrenderer.cpp
testqgsrenderer_LDADD = $(GLOBALLDADD)
testqgsrenderer_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsrenderitem_MOC = testqgsrenderitem.moc.cpp
testqgsrenderitem_SOURCES = testqgsrenderitem.cpp
testqgsrenderitem_LDADD = $(GLOBALLDADD)
testqgsrenderitem_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsscalecalculator_MOC = testqgsscalecalculator.moc.cpp
testqgsscalecalculator_SOURCES = testqgsscalecalculator.cpp
testqgsscalecalculator_LDADD = $(GLOBALLDADD)
testqgsscalecalculator_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgssearchstring_MOC = testqgssearchstring.moc.cpp
testqgssearchstring_SOURCES = testqgssearchstring.cpp
testqgssearchstring_LDADD = $(GLOBALLDADD)
testqgssearchstring_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgssearchtreenode_MOC = testqgssearchtreenode.moc.cpp
testqgssearchtreenode_SOURCES = testqgssearchtreenode.cpp
testqgssearchtreenode_LDADD = $(GLOBALLDADD)
testqgssearchtreenode_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsvectordataprovider_MOC = testqgsvectordataprovider.moc.cpp
testqgsvectordataprovider_SOURCES = testqgsvectordataprovider.cpp
testqgsvectordataprovider_LDADD = $(GLOBALLDADD)
testqgsvectordataprovider_CXXFLAGS =  $(GLOBALCXXFLAGS)
