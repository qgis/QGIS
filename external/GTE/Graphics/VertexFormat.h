// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DataFormat.h>

namespace gte
{
    // Enumerations for DX11.  TODO: Add a GLSL subsystem to allow
    // hooking up 'location' to the semantic.
    enum VASemantic
    {
        VA_NO_SEMANTIC,
        VA_POSITION,
        VA_BLENDWEIGHT,
        VA_BLENDINDICES,
        VA_NORMAL,
        VA_PSIZE,
        VA_TEXCOORD,
        VA_TANGENT,
        VA_BINORMAL,
        VA_TESSFACTOR,
        VA_POSITIONT,
        VA_COLOR,
        VA_FOG,
        VA_DEPTH,
        VA_SAMPLE,
        VA_NUM_SEMANTICS
    };

    enum VAConstant
    {
        // TODO:  Modify to the numbers for Shader Model 5 (DX11).

        // The maximum number of attributes for a vertex format.
        VA_MAX_ATTRIBUTES = 16,

        // The maximum number of texture coordinate units.
        VA_MAX_TCOORD_UNITS = 8,

        // The maximum number of color units.
        VA_MAX_COLOR_UNITS = 2
    };

    class VertexFormat
    {
    public:
        // Construction.
        VertexFormat();

        // Support for reusing a VertexFormat object within a scope.  This
        // call resets the object to the state produced by the default
        // constructor call.
        void Reset();

        // Create a packed vertex format, where all attributes are contiguous
        // in memory.  The order of the attributes is determined by the order
        // of Bind calls.
        void Bind(VASemantic semantic, DFType type, unsigned int unit);

        // Member access.  GetAttribute returns 'true' when the input i is
        // such that 0 <= i < GetNumAttributes(), in which case the returned
        // semantic, type, unit, and offset are valid.
        inline unsigned int GetVertexSize() const
        {
            return mVertexSize;
        }

        inline int GetNumAttributes() const
        {
            return mNumAttributes;
        }

        void GetAttribute(int i, VASemantic& semantic, DFType& type,
            unsigned int& unit, unsigned int& offset) const;

        // Determine whether a semantic/unit exists.  If so, return the
        // index i that can be used to obtain more information about the
        // attribute by the functions after this.  If not, return -1.
        int GetIndex(VASemantic semantic, unsigned int unit) const;
        DFType GetType(int i) const;
        unsigned int GetOffset(int i) const;

    private:
        class Attribute
        {
        public:
            Attribute()
                :
                semantic(VA_NO_SEMANTIC),
                type(DF_UNKNOWN),
                unit(0),
                offset(0)
            {
            }

            VASemantic semantic;
            DFType type;
            unsigned int unit;
            unsigned int offset;
        };

        int mNumAttributes;
        unsigned int mVertexSize;
        Attribute mAttributes[VA_MAX_ATTRIBUTES];
    };
}
