/*
===============================================================================

  FILE:  eb_vlr.hpp

  CONTENTS:
    LAZ io

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    uday.karan@gmail.com - Hobu, Inc.

  COPYRIGHT:

    (c) 2007-2014, martin isenburg, rapidlasso - tools to catch reality
    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

===============================================================================
*/

#pragma once

#include "portable_endian.hpp"
#include "vlr.hpp"

namespace lazperf
{
namespace io
{

struct eb_vlr : public vlr
{
#pragma pack(push, 1)
    struct ebfield
    {
        uint8_t reserved[2];
        uint8_t data_type;
        uint8_t options;
        char name[32];
        uint8_t unused[4];
        double no_data[3];
        double minval[3];
        double maxval[3];
        double scale[3];
        double offset[3];
        char description[32];

        ebfield() : reserved{}, data_type{ htole32(1) }, options{}, name{}, unused{},
            no_data{}, minval{}, maxval{}, scale{}, offset{}, description{}
        {}
    };
#pragma pack(pop)

    std::vector<ebfield> items;

    eb_vlr(size_t bytes)
    {
        for (size_t i = 0; i < bytes; ++i)
            addField();
    }

    void addField()
    {
        ebfield field;

        std::string name = "FIELD_" + std::to_string(items.size());
        strncpy(field.name, name.data(), 32);

        items.push_back(field);
    }

    size_t size() const
    {
        return 192 * items.size();
    }

    // Since all we fill in is a single byte field and a string field, we don't
    // need to worry about byte ordering.
    std::vector<uint8_t> data() const
    {
        const uint8_t *start = reinterpret_cast<const uint8_t *>(items.data());
        return std::vector<uint8_t>(start, start + size());
    }

    virtual vlr::vlr_header header()
    {
        return vlr_header { 0, "LASF_Spec", 4, (uint16_t)size(), ""  };
    }
};

} // namesapce io
} // namesapce lazperf

