# Copyright (C) 2004 Jens Oberender <j.obi at troja.net>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id$

i18ndir = ${pkgdatadir}/i18n

nodist_i18n_DATA = \
	qgis_cs_CZ.qm \
	qgis_de.qm \
	qgis_fr.qm \
	qgis_it.qm \
	qgis_nl.qm \
	qgis_pl_PL.qm \
	qgis_pt_BR.qm \
	qgis_ru.qm \
	qgis_sv.qm \
	qgis_pl_PL.ts \
	qgis_zh_CN.qm

i18n_SOURCES = \
	qgis_cs_CZ.ts \
	qgis_de.ts \
	qgis_fr.ts \
	qgis_it.ts \
	qgis_nl.ts \
	qgis_pl_PL.ts \
	qgis_pt_BR.ts \
	qgis_ru.ts \
	qgis_sv.ts \
	qgis_pl_PL.ts \
	qgis_zh_CN.ts

%.qm: %.ts
	cd ..; ./create_qm_files.sh

EXTRA_DIST = $(i18n_SOURCES)
