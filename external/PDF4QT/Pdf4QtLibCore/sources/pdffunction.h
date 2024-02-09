//    Copyright (C) 2019-2021 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.


#ifndef PDFFUNCTION_H
#define PDFFUNCTION_H

#include "pdfglobal.h"

#include <memory>

namespace pdf
{
class PDFObject;
class PDFFunction;
class PDFDocument;
class PDFParsingContext;

enum class FunctionType
{
    Identity = -1,
    Sampled = 0,
    Exponential = 2,
    Stitching = 3,
    PostScript = 4
};

using PDFFunctionPtr = std::shared_ptr<PDFFunction>;

/// Represents PDF function, as defined in Adobe PDF Reference 1.7, chapter 3.9.
/// Generally, function is m to n relation, f(x_1, ... , x_m) = (y_1, ..., y_n).
/// Function has domain and range, values outside of domain and range are clamped
/// to the nearest values. This class is fully thread safe (if constant functions
/// are called).
class PDF4QTLIBCORESHARED_EXPORT PDFFunction
{
public:

    /// Construct new function.
    /// \param m Number of input variables
    /// \param n Number of output variables
    /// \param domain Array of 2 x m variables of input range - [x1 min, x1 max, x2 min, x2 max, ... ]
    /// \param range Array of 2 x n variables of output range - [y1 min, y1 max, y2 min, y2 max, ... ]
    explicit PDFFunction(uint32_t m, uint32_t n, std::vector<PDFReal>&& domain, std::vector<PDFReal>&& range);
    virtual ~PDFFunction() = default;

    struct FunctionResult
    {
        inline FunctionResult(bool value) : evaluated(value) { }
        inline FunctionResult(const QString& message) : evaluated(false), errorMessage(message) { }

        /// Conversion operator (enables using this in boolean expressions and if)
        explicit operator bool() const { return evaluated; }

        bool evaluated;
        QString errorMessage;
    };

    /// Returns number of input variables
    inline uint32_t getInputVariableCount() const { return m_m; }

    /// Returns number of output variables
    inline uint32_t getOutputVariableCount() const { return m_n; }

    using iterator = PDFReal*;
    using const_iterator = const PDFReal*;

    /// Transforms input values to the output values.
    /// \param x_1 Iterator to the first input value
    /// \param x_n Iterator to the end of the input values (one item after last value)
    /// \param y_1 Iterator to the first output value
    /// \param y_n Iterator to the end of the output values (one item after last value)
    virtual FunctionResult apply(const_iterator x_1, const_iterator x_m, iterator y_1, iterator y_n) const = 0;

    /// Creates function from the object. If error occurs, exception is thrown.
    /// \param document Document, owning the pdf object
    /// \param object Object defining the function
    static PDFFunctionPtr createFunction(const PDFDocument* document, const PDFObject& object);

protected:
    static constexpr const size_t DEFAULT_OPERAND_COUNT = 32;

    /// Creates function from the object. If error occurs, exception is thrown.
    /// \param document Document, owning the pdf object
    /// \param object Object defining the function
    /// \param context Parsing context (to avoid circural references)
    static PDFFunctionPtr createFunctionImpl(const PDFDocument* document, const PDFObject& object, PDFParsingContext* context);

    /// Clamps input value to the domain range.
    /// \param index Index of the input variable, in range [0, m - 1]
    /// \param value Value to be clamped
    inline PDFReal clampInput(size_t index, PDFReal value) const { return qBound<PDFReal>(m_domain[2 * index], value, m_domain[2 * index + 1]); }

    /// Clamps output value to the domain range.
    /// \param index Index of the output variable, in range [0, n - 1]
    /// \param value Value to be clamped
    inline PDFReal clampOutput(size_t index, PDFReal value) const { return qBound<PDFReal>(m_range[2 * index], value, m_range[2 * index + 1]); }

    /// Performs linear interpolation between c0 and c1 using x (in range [0.0, 1.0]). If x is not of this range,
    /// then the function succeeds, and returns value outside of interval [c0, c1].
    /// \param x Value to be interpolated
    /// \param c0 Value for x == 0.0
    /// \param c1 Value for x == 1.0
    static inline constexpr PDFReal mix(PDFReal x, PDFReal c0, PDFReal c1)
    {
        return c0 * (1.0 - x) + c1 * x;
    }

    /// Returns true, if function has defined range
    inline bool hasRange() const { return !m_range.empty(); }

    uint32_t m_m;
    uint32_t m_n;

    std::vector<PDFReal> m_domain;
    std::vector<PDFReal> m_range;
};

/// Identity function
class PDF4QTLIBCORESHARED_EXPORT PDFIdentityFunction : public PDFFunction
{
public:
    explicit PDFIdentityFunction();
    virtual ~PDFIdentityFunction() = default;

    /// Transforms input values to the output values.
    /// \param x_1 Iterator to the first input value
    /// \param x_n Iterator to the end of the input values (one item after last value)
    /// \param y_1 Iterator to the first output value
    /// \param y_n Iterator to the end of the output values (one item after last value)
    virtual FunctionResult apply(const_iterator x_1, const_iterator x_m, iterator y_1, iterator y_n) const override;
};

/// Sampled function (Type 0 function).
/// \note Order is ignored, linear interpolation is always performed. No cubic spline
/// interpolation occurs.
class PDF4QTLIBCORESHARED_EXPORT PDFSampledFunction : public PDFFunction
{
public:

    /// Construct new sampled function.
    /// \param m Number of input variables
    /// \param n Number of output variables
    /// \param domain Array of 2 x m variables of input range - [x1 min, x1 max, x2 min, x2 max, ... ]
    /// \param range Array of 2 x n variables of output range - [y1 min, y1 max, y2 min, y2 max, ... ]
    /// \param size Number of samples for each variable (so array size is m)
    /// \param samples Array of samples (size is size[0] * size[1] * ... * size[m - 1] * n
    /// \param encoder Array of 2 x m variables of encoding range - [x1 min, x1 max, x2 min, x2 max, ... ]
    /// \param decoder Array of 2 x n variables of decoding range - [y1 min, y1 max, y2 min, y2 max, ... ]
    /// \param sampleMaximalValue Maximal value of the sample
    /// \param order Interpolation order
    explicit PDFSampledFunction(uint32_t m,
                                uint32_t n,
                                std::vector<PDFReal>&& domain,
                                std::vector<PDFReal>&& range,
                                std::vector<uint32_t>&& size,
                                std::vector<PDFReal>&& samples,
                                std::vector<PDFReal>&& encoder,
                                std::vector<PDFReal>&& decoder,
                                PDFReal sampleMaximalValue,
                                PDFInteger order);
    virtual ~PDFSampledFunction() = default;

    /// Transforms input values to the output values.
    /// \param x_1 Iterator to the first input value
    /// \param x_n Iterator to the end of the input values (one item after last value)
    /// \param y_1 Iterator to the first output value
    /// \param y_n Iterator to the end of the output values (one item after last value)
    virtual FunctionResult apply(const_iterator x_1, const_iterator x_m, iterator y_1, iterator y_n) const override;

    PDFInteger getOrder() const { return m_order; }

private:
    /// Number of nodes in m-dimensional hypercube (it is 2^m).
    uint32_t m_hypercubeNodeCount;

    /// Number of samples for each input variable
    std::vector<uint32_t> m_size;

    /// Samples (sample values), stored as reals in range [0, 1]
    std::vector<PDFReal> m_samples;

    /// Encoder, maps input values
    std::vector<PDFReal> m_encoder;

    /// Decoder, maps output values
    std::vector<PDFReal> m_decoder;

    /// Hypercube node offsets. This vector has size \p m_hypercubeNodeCount, and
    /// points to the node offsets of the other nodes, if we know the offset to the
    /// node (0, ..., 0).
    std::vector<uint32_t> m_hypercubeNodeOffsets;

    /// Maximal value of the sample (determined by number of the bits of the sample)
    PDFReal m_sampleMaximalValue;

    /// Interpolation order (1 = linear, 2 = quadratic, 3 = cubic)
    PDFInteger m_order;
};

/// Exponential function (Type 2 function)
/// This type of function has always exactly one input. Transformation of this function
/// is defined as f(x) = c0 + x^exponent * (c1 - c0). If exponent is 1.0, then linear interpolation
/// is performed as f(x) = c0 * (1 - x) + x * c1. To be more precise, if exponent is nearly 1.0,
/// then linear interpolation is used instead.
class PDF4QTLIBCORESHARED_EXPORT PDFExponentialFunction : public PDFFunction
{
public:
    /// Construct new exponential function.
    /// \param m Number of input variables (must be always 1!)
    /// \param n Number of output variables
    /// \param domain Array of 2 variables of input range - [x1 min, x1 max ]
    /// \param range Array of 2 x n variables of output range - [y1 min, y1 max, y2 min, y2 max, ... ]
    /// \param c0 Array of n variables defining output, when x == 0.0
    /// \param c1 Array of n variables defining output, when x == 1.0
    /// \param exponent Exponent of the exponential function.
    explicit PDFExponentialFunction(uint32_t m,
                                    uint32_t n,
                                    std::vector<PDFReal>&& domain,
                                    std::vector<PDFReal>&& range,
                                    std::vector<PDFReal>&& c0,
                                    std::vector<PDFReal>&& c1,
                                    PDFReal exponent);
    virtual ~PDFExponentialFunction() = default;

    /// Transforms input values to the output values.
    /// \param x_1 Iterator to the first input value
    /// \param x_n Iterator to the end of the input values (one item after last value)
    /// \param y_1 Iterator to the first output value
    /// \param y_n Iterator to the end of the output values (one item after last value)
    virtual FunctionResult apply(const_iterator x_1, const_iterator x_m, iterator y_1, iterator y_n) const override;

private:
    std::vector<PDFReal> m_c0;
    std::vector<PDFReal> m_c1;
    PDFReal m_exponent;
    bool m_isLinear;
};

/// Stitching function (Type 3 function)
/// This type of function has always exactly one input. Transformation of this function
/// is defined via k subfunctions which are used in defined intervals of the input value.
class PDF4QTLIBCORESHARED_EXPORT PDFStitchingFunction : public PDFFunction
{
public:
    struct PartialFunction
    {
        explicit inline PartialFunction() :
            bound0(0.0),
            bound1(0.0),
            encode0(0.0),
            encode1(0.0)
        {

        }

        explicit inline PartialFunction(PDFFunctionPtr function,
                                        PDFReal bound0,
                                        PDFReal bound1,
                                        PDFReal encode0,
                                        PDFReal encode1) :
            function(std::move(function)),
            bound0(bound0),
            bound1(bound1),
            encode0(encode0),
            encode1(encode1)
        {

        }

        PDFFunctionPtr function;
        PDFReal bound0;
        PDFReal bound1;
        PDFReal encode0;
        PDFReal encode1;
    };

    /// Construct new stitching function.
    /// \param m Number of input variables (must be always 1!)
    /// \param n Number of output variables
    explicit PDFStitchingFunction(uint32_t m,
                                  uint32_t n,
                                  std::vector<PDFReal>&& domain,
                                  std::vector<PDFReal>&& range,
                                  std::vector<PartialFunction>&& partialFunctions);
    virtual ~PDFStitchingFunction() override;

    /// Transforms input values to the output values.
    /// \param x_1 Iterator to the first input value
    /// \param x_n Iterator to the end of the input values (one item after last value)
    /// \param y_1 Iterator to the first output value
    /// \param y_n Iterator to the end of the output values (one item after last value)
    virtual FunctionResult apply(const_iterator x_1, const_iterator x_m, iterator y_1, iterator y_n) const override;

private:
    /// Partial function definitions
    std::vector<PartialFunction> m_partialFunctions;
};

/// Postscript function (Type 4 function)
/// Implements subset of postscript language
class PDF4QTLIBCORESHARED_EXPORT PDFPostScriptFunction : public PDFFunction
{
public:

    class PDFPostScriptFunctionException : public std::exception
    {
    public:
        inline explicit PDFPostScriptFunctionException(const QString& message) :
            m_message(message)
        {

        }

        /// Returns error message
        const QString& getMessage() const { return m_message; }

    private:
        QString m_message;
    };

    using InstructionPointer = size_t;

    enum class OperandType
    {
        Real,               ///< Real number
        Integer,            ///< Integer number
        Boolean,            ///< Boolean
        InstructionPointer  ///< Instruction pointer
    };

    enum class Code
    {
        // B.1 Arithmetic operators
        Add,
        Sub,
        Mul,
        Div,
        Idiv,
        Mod,
        Neg,
        Abs,
        Ceiling,
        Floor,
        Round,
        Truncate,
        Sqrt,
        Sin,
        Cos,
        Atan,
        Exp,
        Ln,
        Log,
        Cvi,
        Cvr,

        // B.2 Relational, Boolean and Bitwise operators
        Eq,
        Ne,
        Gt,
        Ge,
        Lt,
        Le,
        And,
        Or,
        Xor,
        Not,
        Bitshift,
        True,
        False,

        // B.3 Conditional operators
        If,
        IfElse,

        // B.4 Stack operators
        Pop,
        Exch,
        Dup,
        Copy,
        Index,
        Roll,

        // Special codes not present in PDF reference, but needed to implement
        // blocks (call and return function).
        Call,
        Return,
        Push,
        Execute
    };

    /// Gets the code from the byte array. If byte array contains invalid data,
    /// then exception is thrown.
    /// \param byteArray Byte array to be converted to the code
    static Code getCode(const QByteArray& byteArray);

    struct OperandObject
    {
        explicit inline constexpr OperandObject() :
            type(OperandType::Real),
            realNumber(0.0)
        {

        }

        static inline OperandObject createReal(PDFReal value) { OperandObject object; object.type = OperandType::Real; object.realNumber = value; return object; }
        static inline OperandObject createInteger(PDFInteger value) { OperandObject object; object.type = OperandType::Integer; object.integerNumber = value; return object; }
        static inline OperandObject createBoolean(bool value) { OperandObject object; object.type = OperandType::Boolean; object.boolean = value; return object; }
        static inline OperandObject createInstructionPointer(InstructionPointer value) { OperandObject object; object.type = OperandType::InstructionPointer; object.instructionPointer = value; return object; }

        OperandType type;

        union
        {
            PDFReal realNumber;
            PDFInteger integerNumber;
            bool boolean;
            InstructionPointer instructionPointer;
        };
    };

    static constexpr const InstructionPointer INVALID_INSTRUCTION_POINTER = std::numeric_limits<InstructionPointer>::max();

    struct CodeObject
    {
        explicit inline CodeObject() : code(Code::Return), next(INVALID_INSTRUCTION_POINTER), operand() { }
        explicit inline CodeObject(OperandObject operand, InstructionPointer next) : code(Code::Push), next(next), operand(std::move(operand)) { }
        explicit inline CodeObject(Code code, InstructionPointer next) : code(code), next(next), operand() { }

        Code code;
        InstructionPointer next;
        OperandObject operand;
    };

    using Program = std::vector<CodeObject>;

    /// Construct new postscript function.
    /// \param m Number of input variables
    /// \param n Number of output variables
    /// \param domain Array of 2 x m variables of input range - [x1 min, x1 max, x2 min, x2 max, ... ]
    /// \param range Array of 2 x n variables of output range - [y1 min, y1 max, y2 min, y2 max, ... ]
    explicit PDFPostScriptFunction(uint32_t m, uint32_t n, std::vector<PDFReal>&& domain, std::vector<PDFReal>&& range, Program&& program);
    virtual ~PDFPostScriptFunction() override;

    /// Create a PostScript program from the byte array
    static Program parseProgram(const QByteArray& byteArray);

    /// Transforms input values to the output values.
    /// \param x_1 Iterator to the first input value
    /// \param x_n Iterator to the end of the input values (one item after last value)
    /// \param y_1 Iterator to the first output value
    /// \param y_n Iterator to the end of the output values (one item after last value)
    virtual FunctionResult apply(const_iterator x_1, const_iterator x_m, iterator y_1, iterator y_n) const override;

private:
    Program m_program;

    friend class PDFPostScriptFunctionStack;
    friend class PDFPostScriptFunctionExecutor;
};

}   // namespace pdf

#endif // PDFFUNCTION_H
