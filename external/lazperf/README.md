This is a verbatim copy of code:

Source: https://github.com/hobu/laz-perf (cpp/lazperf/ directory)
Commit: e0bd817 @ master (post 3.0.0 release)
License: Apache-2.0

Adds a tiny patch to un-deprecate eb_vlr::addField() and eb_vlr::eb_vlr(int ebCount)
so that the deprecation warnings does not break QGIS CI.
Upstream ticket: https://github.com/hobu/laz-perf/issues/122
