/*
===============================================================================

  FILE:  filestream.hpp

  CONTENTS:
    Stream abstractions

  PROGRAMMERS:

    uday.karan@gmail.com - Hobu, Inc.

  COPYRIGHT:

    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

===============================================================================
*/

#pragma once

#include <iostream>

#include "lazperf.hpp"

namespace lazperf
{

// Convenience class

struct OutFileStream
{
public:
    LAZPERF_EXPORT OutFileStream(std::ostream& out);

    LAZPERF_EXPORT void putBytes(const unsigned char *c, size_t len);
    LAZPERF_EXPORT OutputCb cb();

private:
    std::ostream& f_;
};

// Convenience class

struct InFileStream
{
    struct Private;

public:
    LAZPERF_EXPORT InFileStream(std::istream& in);
    LAZPERF_EXPORT ~InFileStream();

    // This will force a fill on the next fetch.
    LAZPERF_EXPORT void reset();
    LAZPERF_EXPORT InputCb cb();

private:
    std::unique_ptr<Private> p_;
};

} // namespace lazperf

