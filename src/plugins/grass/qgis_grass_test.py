#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -----------------------------------------------------------
#
# Copyright (C) 2012  Radim Blazek
# EMAIL: radim.blazek (at) gmail.com
#
# -----------------------------------------------------------
#
# licensed under the terms of GNU GPL 2
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# ---------------------------------------------------------------------

"""
***************************************************************************
 GRASS Direct test - to be run in GRASS shell

 - collects list of raster layers
 - exports GRASS layers on low resolution to temporary GeoTIFFs
 - runs GRASS modules in standard and direct mode and compares results
 - writes out report
***************************************************************************
"""
__author__ = 'Radim Blazek'
__date__ = 'December 2012'
__copyright__ = '(C) 2012, Radim Blazek'
__revision__ = '$Format:%H$'

import os
import sys
import subprocess
import time
import re


class Test:

    def __init__(self):
        if "GISBASE" not in os.environ or "GISRC" not in os.environ:
            print "This script must be run in GRASS shell."
            sys.exit(1)

        if "QGIS_PREFIX_PATH" not in os.environ:
            print "QGIS_PREFIX_PATH environment variable not set."
            sys.exit(1)

        self.size = 10
        self.reportStr = ""
        pass

    # add message to HTML report
    def report(self, msg):
        self.reportStr += msg + "\n"

    def writeReport(self):
        print self.reportStr

    def test(self):
        print "GRASS Direct test"

        tmp_dir = os.path.abspath("qgis-grass-test-%s" % time.strftime('%y%m%d-%H%M%S'))
        tmp_dir = os.path.abspath("qgis-grass-test-debug") # debug
        print "Output will be written to %s" % tmp_dir

        files_dir = "%s/tif" % tmp_dir
        #os.makedirs( files_dir )

        # get list of raster layers
        print "Getting list of rasters ..."
        rasters = self.srun(["g.mlist", "type=rast"]).splitlines()
        max_rasters = 1
        print "%s rasters found, using first %s" % (len(rasters), max_rasters)
        rasters = rasters[0:1]

        print "Exporting rasters"
        for raster in rasters:
            print raster
            output = "%s/%s.tif" % (files_dir, raster)
            self.srun(["g.region", "rast=%s" % raster, "cols=%s" % self.size, "rows=%s" % self.size])
            self.srun(["r.out.gdal", "input=%s" % raster, "output=%s" % output])

        # run modules
        for module in self.modules():
            for raster in rasters:
                module = re.sub("  *", " ", module)
                module_name = module.split(" ")[0]
                # --- native ---
                self.srun(["g.region", "rast=%s" % raster, "cols=%s" % self.size, "rows=%s" % self.size])
                output = "qgistest1"
                # clean old
                self.srun(["g.remove", "-f", "rast=%s" % output])
                # substitute rasters
                native_args = module.replace("R1", raster).replace("RO1", output).split(" ")
                (code, out, err) = self.run(native_args)
                if code != 0:
                    self.report("Native failed: %s" % " ".join(native_args))
                # export
                native_output_file = "%s/%s-%s-native.tif" % (files_dir, module_name, raster)
                self.srun(["r.out.gdal", "input=%s" % output, "output=%s" % native_output_file])
                self.srun(["g.remove", "-f", "rast=%s" % output])

                # --- direct ---
                direct_input_file = "%s/%s.tif" % (files_dir, raster)
                direct_output_file = "%s/%s-%s-direct.tif" % (files_dir, module_name, raster)

                # substitute rasters
                direct_args = module.replace("R1", direct_input_file).replace("RO1", direct_output_file).split(" ")
                env = os.environ

                # CRS
                proj = self.srun(["g.proj", "-j"])
                longlat = True if proj.find("+proj=longlat") != -1 else False
                proj = proj.splitlines()
                proj = " ".join(proj)
                print proj
                env['QGIS_GRASS_CRS'] = proj

                # set GRASS region as environment variable
                reg = self.srun(["g.region", "-g"])
                reg_dict = dict(item.split("=") for item in reg.splitlines())
                reg_var = {'n': 'north', 's': 'south', 'e': 'east', 'w': 'west', 'nsres': 'n-s resol', 'ewres': 'e-w resol'}
                if longlat:
                    region = "proj:3;zone:-1" # longlat
                else:
                    region = "proj:99;zone:-1" # other projection
                for k, v in reg_dict.iteritems():
                    if k == 'cells':
                        continue
                    kn = k
                    if k in reg_var:
                        kn = reg_var[k]
                    region += ";%s:%s" % (kn, v)
                print region
                env['GRASS_REGION'] = region

                # add path to fake GRASS gis library
                env['LD_LIBRARY_PATH'] = "%s/lib/qgis/plugins/:%s" % (env['QGIS_PREFIX_PATH'], env['LD_LIBRARY_PATH'])
                (code, out, err) = self.run(direct_args, env)
                print "code = %s" % code
                if code != 0:
                    self.report("Direct failed: %s\n%s\n%s" % (" ".join(direct_args), out, err))
                # TODO: compare native x direct output

    def run(self, args, env=None, input=None, exit_on_error=False):
        cmd = " ".join(args)
        print cmd
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE, env=env)
        com = p.communicate(input)
        p.wait()

        if p.returncode != 0 and exit_on_error:
            msg = "Failed:\n" + str(com[0]) + "\n" + str(com[1])
            raise Exception(msg)

        return (p.returncode, com[0], com[1]) # return stdout

    # simple run
    def srun(self, args):
        return self.run(args, None, None, True)[1]

    def modules(self):
        # R1 - input raster 1
        # RO1 - output raster 1
        modules = [
            "r.slope.aspect elevation=R1 aspect=RO1"
        ]
        return modules

if __name__ == '__main__':
    test = Test()
    test.test()
    test.writeReport()
