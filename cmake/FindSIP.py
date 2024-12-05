#
#   Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
#    All rights reserved.
#
#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions are met:
#        * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#        * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#        * Neither the name of the  Simon Edwards <simon@simonzone.com> nor the
#        names of its contributors may be used to endorse or promote products
#        derived from this software without specific prior written permission.
#
#    THIS SOFTWARE IS PROVIDED BY Simon Edwards <simon@simonzone.com> ''AS IS'' AND ANY
#    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#    DISCLAIMED. IN NO EVENT SHALL Simon Edwards <simon@simonzone.com> BE LIABLE FOR ANY
#    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# FindSIP.py
# Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

try:
    import sipbuild

    print("sip_version:%06.0x" % sipbuild.version.SIP_VERSION)
    print("sip_version_num:%d" % sipbuild.version.SIP_VERSION)
    print("sip_version_str:%s" % sipbuild.version.SIP_VERSION_STR)

    import sysconfig

    if "deb_system" in sysconfig.get_scheme_names():
        python_modules_dir = sysconfig.get_path("purelib", "deb_system")
    else:
        python_modules_dir = sysconfig.get_path("purelib")
    print("default_sip_dir:%s" % python_modules_dir)
except ImportError:  # Code for SIP v4
    import sipconfig

    sipcfg = sipconfig.Configuration()
    print("sip_version:%06.0x" % sipcfg.sip_version)
    print("sip_version_num:%d" % sipcfg.sip_version)
    print("sip_version_str:%s" % sipcfg.sip_version_str)
    print("sip_bin:%s" % sipcfg.sip_bin)
    print("default_sip_dir:%s" % sipcfg.default_sip_dir)
    print("sip_inc_dir:%s" % sipcfg.sip_inc_dir)
    # SIP 4.19.10+ has new sipcfg.sip_module_dir
    if hasattr(sipcfg, "sip_module_dir"):
        print("sip_module_dir:%s" % sipcfg.sip_module_dir)
    else:
        print("sip_module_dir:%s" % sipcfg.sip_mod_dir)
