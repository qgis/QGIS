# FindSIP.py
#
# Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

import sys
import sipconfig

sipcfg = sipconfig.Configuration()
print("sip_version:%06.0x" % sipcfg.sip_version)
print("sip_version_num:%d" % sipcfg.sip_version)
print("sip_version_str:%s" % sipcfg.sip_version_str)
print("sip_bin:%s" % sipcfg.sip_bin)
print("default_sip_dir:%s" % sipcfg.default_sip_dir)
print("sip_inc_dir:%s" % sipcfg.sip_inc_dir)
print("sip_mod_dir:%s" % sipcfg.sip_mod_dir)
