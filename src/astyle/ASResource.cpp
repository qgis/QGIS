/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   ASResource.cpp
 *
 *   This file is a part of "Artistic Style" - an indentation and
 *   reformatting tool for C, C++, C# and Java source files.
 *   http://astyle.sourceforge.net
 *
 *   The "Artistic Style" project, including all files needed to
 *   compile it, is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this project; if not, write to the
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA  02110-1301, USA.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#include "astyle.h"


namespace astyle
{
const string ASResource::AS_IF = string("if");
const string ASResource::AS_ELSE = string("else");
const string ASResource::AS_FOR = string("for");
const string ASResource::AS_DO = string("do");
const string ASResource::AS_WHILE = string("while");
const string ASResource::AS_SWITCH = string("switch");
const string ASResource::AS_CASE = string("case");
const string ASResource::AS_DEFAULT = string("default");
const string ASResource::AS_CLASS = string("class");
const string ASResource::AS_STRUCT = string("struct");
const string ASResource::AS_UNION = string("union");
const string ASResource::AS_INTERFACE = string("interface");
const string ASResource::AS_NAMESPACE = string("namespace");
const string ASResource::AS_EXTERN = string("extern");
const string ASResource::AS_PUBLIC = string("public");
const string ASResource::AS_PROTECTED = string("protected");
const string ASResource::AS_PRIVATE = string("private");
const string ASResource::AS_STATIC = string("static");
const string ASResource::AS_SYNCHRONIZED = string("synchronized");
const string ASResource::AS_OPERATOR = string("operator");
const string ASResource::AS_TEMPLATE = string("template");
const string ASResource::AS_TRY = string("try");
const string ASResource::AS_CATCH = string("catch");
const string ASResource::AS_FINALLY = string("finally");
const string ASResource::AS_THROWS = string("throws");
const string ASResource::AS_CONST = string("const");
const string ASResource::AS_OVERRIDE = string("override");

const string ASResource::AS_ASM = string("asm");

const string ASResource::AS_BAR_DEFINE = string("#define");
const string ASResource::AS_BAR_INCLUDE = string("#include");
const string ASResource::AS_BAR_IF = string("#if");
const string ASResource::AS_BAR_EL = string("#el");
const string ASResource::AS_BAR_ENDIF = string("#endif");

const string ASResource::AS_OPEN_BRACKET = string("{");
const string ASResource::AS_CLOSE_BRACKET = string("}");
const string ASResource::AS_OPEN_LINE_COMMENT = string("//");
const string ASResource::AS_OPEN_COMMENT = string("/*");
const string ASResource::AS_CLOSE_COMMENT = string("*/");

const string ASResource::AS_ASSIGN = string("=");
const string ASResource::AS_PLUS_ASSIGN = string("+=");
const string ASResource::AS_MINUS_ASSIGN = string("-=");
const string ASResource::AS_MULT_ASSIGN = string("*=");
const string ASResource::AS_DIV_ASSIGN = string("/=");
const string ASResource::AS_MOD_ASSIGN = string("%=");
const string ASResource::AS_OR_ASSIGN = string("|=");
const string ASResource::AS_AND_ASSIGN = string("&=");
const string ASResource::AS_XOR_ASSIGN = string("^=");
const string ASResource::AS_GR_GR_ASSIGN = string(">>=");
const string ASResource::AS_LS_LS_ASSIGN = string("<<=");
const string ASResource::AS_GR_GR_GR_ASSIGN = string(">>>=");
const string ASResource::AS_LS_LS_LS_ASSIGN = string("<<<=");
const string ASResource::AS_RETURN = string("return");
const string ASResource::AS_CIN = string("cin");
const string ASResource::AS_COUT = string("cout");
const string ASResource::AS_CERR = string("cerr");

const string ASResource::AS_EQUAL = string("==");
const string ASResource::AS_PLUS_PLUS = string("++");
const string ASResource::AS_MINUS_MINUS = string("--");
const string ASResource::AS_NOT_EQUAL = string("!=");
const string ASResource::AS_GR_EQUAL = string(">=");
const string ASResource::AS_GR_GR = string(">>");
const string ASResource::AS_GR_GR_GR = string(">>>");
const string ASResource::AS_LS_EQUAL = string("<=");
const string ASResource::AS_LS_LS = string("<<");
const string ASResource::AS_LS_LS_LS = string("<<<");
const string ASResource::AS_ARROW = string("->");
const string ASResource::AS_AND = string("&&");
const string ASResource::AS_OR = string("||");
const string ASResource::AS_COLON_COLON = string("::");
const string ASResource::AS_PAREN_PAREN = string("()");
const string ASResource::AS_BLPAREN_BLPAREN = string("[]");

const string ASResource::AS_PLUS = string("+");
const string ASResource::AS_MINUS = string("-");
const string ASResource::AS_MULT = string("*");
const string ASResource::AS_DIV = string("/");
const string ASResource::AS_MOD = string("%");
const string ASResource::AS_GR = string(">");
const string ASResource::AS_LS = string("<");
const string ASResource::AS_NOT = string("!");
const string ASResource::AS_BIT_OR = string("|");
const string ASResource::AS_BIT_AND = string("&");
const string ASResource::AS_BIT_NOT = string("~");
const string ASResource::AS_BIT_XOR = string("^");
const string ASResource::AS_QUESTION = string("?");
const string ASResource::AS_COLON = string(":");
const string ASResource::AS_COMMA = string(",");
const string ASResource::AS_SEMICOLON = string(";");

const string ASResource::AS_FOREACH = string("foreach");
const string ASResource::AS_Q_FOREACH = string("Q_FOREACH");
const string ASResource::AS_LOCK = string("lock");
const string ASResource::AS_UNSAFE = string("unsafe");
const string ASResource::AS_FIXED = string("fixed");
const string ASResource::AS_GET = string("get");
const string ASResource::AS_SET = string("set");
const string ASResource::AS_ADD = string("add");
const string ASResource::AS_REMOVE = string("remove");

const string ASResource::AS_CONST_CAST = string("const_cast");
const string ASResource::AS_DYNAMIC_CAST = string("dynamic_cast");
const string ASResource::AS_REINTERPRET_CAST = string("reinterpret_cast");
const string ASResource::AS_STATIC_CAST = string("static_cast");


/**
 * Build the vector of assignment operators.
 * Used by BOTH ASFormatter.cpp and ASBeautifier.cpp
 *
 * @param assignmentOperators   a reference to the vector to be built.
 */
void ASResource::buildAssignmentOperators(vector<const string*> &assignmentOperators)
{
	assignmentOperators.push_back(&AS_ASSIGN);
	assignmentOperators.push_back(&AS_PLUS_ASSIGN);
	assignmentOperators.push_back(&AS_MINUS_ASSIGN);
	assignmentOperators.push_back(&AS_MULT_ASSIGN);
	assignmentOperators.push_back(&AS_DIV_ASSIGN);
	assignmentOperators.push_back(&AS_MOD_ASSIGN);
	assignmentOperators.push_back(&AS_OR_ASSIGN);
	assignmentOperators.push_back(&AS_AND_ASSIGN);
	assignmentOperators.push_back(&AS_XOR_ASSIGN);

	// Java
	assignmentOperators.push_back(&AS_GR_GR_GR_ASSIGN);
	assignmentOperators.push_back(&AS_GR_GR_ASSIGN);
	assignmentOperators.push_back(&AS_LS_LS_ASSIGN);

	// Unknown
	assignmentOperators.push_back(&AS_LS_LS_LS_ASSIGN);

	assignmentOperators.push_back(&AS_RETURN);
	assignmentOperators.push_back(&AS_CIN);
	assignmentOperators.push_back(&AS_COUT);
	assignmentOperators.push_back(&AS_CERR);
}

/**
 * Build the vector of C++ cast operators.
 * Used by ONLY ASFormatter.cpp
 *
 * @param castOperators     a reference to the vector to be built.
 */
void ASResource::buildCastOperators(vector<const string*> &castOperators)
{
	castOperators.push_back(&AS_CONST_CAST);
	castOperators.push_back(&AS_DYNAMIC_CAST);
	castOperators.push_back(&AS_REINTERPRET_CAST);
	castOperators.push_back(&AS_STATIC_CAST);
}

/**
 * Build the vector of header words.
 * Used by BOTH ASFormatter.cpp and ASBeautifier.cpp
 *
 * @param headers       a reference to the vector to be built.
 */
void ASResource::buildHeaders(vector<const string*> &headers, int fileType, bool beautifier)
{
	headers.push_back(&AS_IF);
	headers.push_back(&AS_ELSE);
	headers.push_back(&AS_FOR);
	headers.push_back(&AS_WHILE);
	headers.push_back(&AS_DO);
	headers.push_back(&AS_SWITCH);
	headers.push_back(&AS_TRY);
	headers.push_back(&AS_CATCH);

	if (beautifier)
	{
		headers.push_back(&AS_CASE);
		headers.push_back(&AS_DEFAULT);
		headers.push_back(&AS_CONST);
		headers.push_back(&AS_STATIC);
		headers.push_back(&AS_EXTERN);
		headers.push_back(&AS_TEMPLATE);
		headers.push_back(&AS_OVERRIDE);
	}

	if (fileType == JAVA_TYPE)
	{
		headers.push_back(&AS_FINALLY);
		headers.push_back(&AS_SYNCHRONIZED);
	}

	if (fileType == SHARP_TYPE)
	{
		headers.push_back(&AS_FINALLY);
		headers.push_back(&AS_FOREACH);
		headers.push_back(&AS_LOCK);
		headers.push_back(&AS_UNSAFE);
		headers.push_back(&AS_FIXED);
		headers.push_back(&AS_GET);
		headers.push_back(&AS_SET);
		headers.push_back(&AS_ADD);
		headers.push_back(&AS_REMOVE);
	}

	if ( fileType == C_TYPE )
	{
		headers.push_back(&AS_FOREACH);   // Qt macro
		headers.push_back(&AS_Q_FOREACH); // Qt macro
	}
}

/**
 * Build the vector of non-assignment operators.
 * Used by ONLY ASBeautifier.cpp
 *
 * @param nonAssignmentOperators       a reference to the vector to be built.
 */
void ASResource::buildNonAssignmentOperators(vector<const string*> &nonAssignmentOperators)
{
	nonAssignmentOperators.push_back(&AS_EQUAL);
	nonAssignmentOperators.push_back(&AS_PLUS_PLUS);
	nonAssignmentOperators.push_back(&AS_MINUS_MINUS);
	nonAssignmentOperators.push_back(&AS_NOT_EQUAL);
	nonAssignmentOperators.push_back(&AS_GR_EQUAL);
	nonAssignmentOperators.push_back(&AS_GR_GR_GR);
	nonAssignmentOperators.push_back(&AS_GR_GR);
	nonAssignmentOperators.push_back(&AS_LS_EQUAL);
	nonAssignmentOperators.push_back(&AS_LS_LS_LS);
	nonAssignmentOperators.push_back(&AS_LS_LS);
	nonAssignmentOperators.push_back(&AS_ARROW);
	nonAssignmentOperators.push_back(&AS_AND);
	nonAssignmentOperators.push_back(&AS_OR);
}

/**
 * Build the vector of header non-paren headers.
 * Used by BOTH ASFormatter.cpp and ASBeautifier.cpp
 *
 * @param nonParenHeaders       a reference to the vector to be built.
 */
void ASResource::buildNonParenHeaders(vector<const string*> &nonParenHeaders, int fileType, bool beautifier)
{
	nonParenHeaders.push_back(&AS_ELSE);
	nonParenHeaders.push_back(&AS_DO);
	nonParenHeaders.push_back(&AS_TRY);

	if (beautifier)
	{
		nonParenHeaders.push_back(&AS_CASE);
		nonParenHeaders.push_back(&AS_DEFAULT);
		nonParenHeaders.push_back(&AS_CONST);
		nonParenHeaders.push_back(&AS_STATIC);
		nonParenHeaders.push_back(&AS_EXTERN);
		nonParenHeaders.push_back(&AS_TEMPLATE);
		nonParenHeaders.push_back(&AS_OVERRIDE);
	}

	if (fileType == JAVA_TYPE)
	{
		nonParenHeaders.push_back(&AS_FINALLY);
	}

	if (fileType == SHARP_TYPE)
	{
		nonParenHeaders.push_back(&AS_CATCH);		// can be a paren or non-paren header
		nonParenHeaders.push_back(&AS_FINALLY);
		nonParenHeaders.push_back(&AS_UNSAFE);
		nonParenHeaders.push_back(&AS_GET);
		nonParenHeaders.push_back(&AS_SET);
		nonParenHeaders.push_back(&AS_ADD);
		nonParenHeaders.push_back(&AS_REMOVE);
	}
}

/**
 * Build the vector of operators.
 * Used by ONLY ASFormatter.cpp
 *
 * @param operators             a reference to the vector to be built.
 */
void ASResource::buildOperators(vector<const string*> &operators)
{
	operators.push_back(&AS_PLUS_ASSIGN);
	operators.push_back(&AS_MINUS_ASSIGN);
	operators.push_back(&AS_MULT_ASSIGN);
	operators.push_back(&AS_DIV_ASSIGN);
	operators.push_back(&AS_MOD_ASSIGN);
	operators.push_back(&AS_OR_ASSIGN);
	operators.push_back(&AS_AND_ASSIGN);
	operators.push_back(&AS_XOR_ASSIGN);
	operators.push_back(&AS_EQUAL);
	operators.push_back(&AS_PLUS_PLUS);
	operators.push_back(&AS_MINUS_MINUS);
	operators.push_back(&AS_NOT_EQUAL);
	operators.push_back(&AS_GR_EQUAL);
	operators.push_back(&AS_GR_GR_GR_ASSIGN);
	operators.push_back(&AS_GR_GR_ASSIGN);
	operators.push_back(&AS_GR_GR_GR);
	operators.push_back(&AS_GR_GR);
	operators.push_back(&AS_LS_EQUAL);
	operators.push_back(&AS_LS_LS_LS_ASSIGN);
	operators.push_back(&AS_LS_LS_ASSIGN);
	operators.push_back(&AS_LS_LS_LS);
	operators.push_back(&AS_LS_LS);
	operators.push_back(&AS_ARROW);
	operators.push_back(&AS_AND);
	operators.push_back(&AS_OR);
	operators.push_back(&AS_COLON_COLON);
	operators.push_back(&AS_PLUS);
	operators.push_back(&AS_MINUS);
	operators.push_back(&AS_MULT);
	operators.push_back(&AS_DIV);
	operators.push_back(&AS_MOD);
	operators.push_back(&AS_QUESTION);
	operators.push_back(&AS_COLON);
	operators.push_back(&AS_ASSIGN);
	operators.push_back(&AS_LS);
	operators.push_back(&AS_GR);
	operators.push_back(&AS_NOT);
	operators.push_back(&AS_BIT_OR);
	operators.push_back(&AS_BIT_AND);
	operators.push_back(&AS_BIT_NOT);
	operators.push_back(&AS_BIT_XOR);
	operators.push_back(&AS_OPERATOR);
	operators.push_back(&AS_COMMA);
	operators.push_back(&AS_RETURN);
}

/**
 * Build the vector of pre-block statements.
 * Used by ONLY ASBeautifier.cpp
 *
 * @param preBlockStatements        a reference to the vector to be built.
 */
void ASResource::buildPreBlockStatements(vector<const string*> &preBlockStatements)
{
	preBlockStatements.push_back(&AS_CLASS);
	preBlockStatements.push_back(&AS_STRUCT);
	preBlockStatements.push_back(&AS_UNION);
	preBlockStatements.push_back(&AS_INTERFACE);
	preBlockStatements.push_back(&AS_NAMESPACE);
	preBlockStatements.push_back(&AS_THROWS);
	preBlockStatements.push_back(&AS_EXTERN);
}

/**
 * Build the vector of pre-command headers.
 * Used by ONLY ASFormatter.cpp
 *
 * @param preCommandHeaders     a reference to the vector to be built.
 */
void ASResource::buildPreCommandHeaders(vector<const string*> &preCommandHeaders)
{
	preCommandHeaders.push_back(&AS_EXTERN);
	preCommandHeaders.push_back(&AS_THROWS);
	preCommandHeaders.push_back(&AS_CONST);
}

/**
 * Build the vector of pre-definition headers.
 * Used by ONLY ASFormatter.cpp
 *
 * @param preDefinitionHeaders      a reference to the vector to be built.
 */
void ASResource::buildPreDefinitionHeaders(vector<const string*> &preDefinitionHeaders)
{
	preDefinitionHeaders.push_back(&AS_CLASS);
	preDefinitionHeaders.push_back(&AS_INTERFACE);
	preDefinitionHeaders.push_back(&AS_NAMESPACE);
	preDefinitionHeaders.push_back(&AS_STRUCT);
}


}   // end namespace astyle
