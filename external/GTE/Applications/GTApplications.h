// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

// Applications/Common
#include <Applications/Application.h>
#include <Applications/CameraRig.h>
#include <Applications/Command.h>
#include <Applications/Console.h>
#include <Applications/ConsoleApplication.h>
#include <Applications/Environment.h>
#include <Applications/LogReporter.h>
#include <Applications/OnIdleTimer.h>
#include <Applications/Timer.h>
#include <Applications/TrackBall.h>
#include <Applications/TrackCylinder.h>
#include <Applications/TrackObject.h>
#include <Applications/WICFileIO.h>
#include <Applications/WindowApplication.h>

#if defined(GTE_USE_MSWINDOWS)
// Applications/MSW
#include <Applications/MSW/Console.h>
#include <Applications/MSW/ConsoleSystem.h>
#include <Applications/MSW/LogToMessageBox.h>
#include <Applications/MSW/LogToOutputWindow.h>
#include <Applications/MSW/Window.h>
#include <Applications/MSW/WindowSystem.h>
#include <Applications/MSW/WICFileIO.h>
#endif

#if defined(GTE_USE_LINUX)
// Applications/GLX
#include <Applications/GLX/Window.h>
#include <Applications/GLX/WindowSystem.h>
#include <Applications/GLX/WICFileIO.h>
#endif
