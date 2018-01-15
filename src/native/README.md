README for qgis_native lib
==========================

This library is intended to offer abstraction to the host OS's underlying public
interfaces. This is useful for OSes that provide interfaces in languages other
than C/C++, or for grouping calls to OS-specific code so that it only needs to
be updated in one place in the source tree. It is advisable to leverage existing
functions provided by Qt, rather than rely upon OS-specific code, unless such
code extends the application to provide a better OS-specific user experience or
solve a problem.

Example
-------

As of Mac OS X 10.9 (Mavericks) many system public API calls to Carbon libraries
(based upon C) have been deprecated in favor of modern Cocoa libraries (written
in Objective-C), which can no longer be directly called from C++. Coalescing
and mixing these new calls in a library, using Objective-C++ allows not only
access to the Apple system Objective-C libraries and frameworks, but also those
from third-parties, like the auto-updating Sparkle.framework.

See also: http://el-tramo.be/blog/mixing-cocoa-and-qt/
          http://sparkle.andymatuschak.org/
