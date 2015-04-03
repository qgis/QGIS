# -*- coding: utf-8 -*-

"""
***************************************************************************
    parsing.py
    ---------------------
    Copyright            : (C) 2013 by CS Systemes d'information (CS SI)
    Email                : otb at c-s dot fr (CS SI)
    Contributors         : Julien Malik (CS SI)
                           Oscar Picas (CS SI)
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
__author__ = 'Julien Malik, Oscar Picas'
__copyright__ = '(C) 2013, CS Systemes d\'information  (CS SI)'

from collections import namedtuple
import re

def merge_pairs(list, should_merge, merge):
    """
    Merges adjacent elements of list using the function merge
    if they satisfy the predicate should_merge.
    """
    ret = []
    i = 0
    while i < len(list) - 1:
        a = list[i]
        b = list[i + 1]
        if should_merge(a, b):
            ret.append(merge(a, b))
            i += 2
        else:
            ret.append(a)
            i += 1
    if i == len(list) - 1:
        ret.append(list[i])
    return ret

QuotedString = namedtuple('QuotedString', 'contents comments')
_Arg = namedtuple('Arg', 'contents comments')
_Command = namedtuple('Command', 'name body comment')
BlankLine = namedtuple('BlankLine', '')

class File(list):
    def __repr__(self):
        return 'File(' + repr(list(self)) + ')'

class Comment(str):
    def __repr__(self):
        return 'Comment(' + str(self) + ')'

def Arg(contents, comments=None):
    return _Arg(contents, comments or [])

def Command(name, body, comment=None):
    return _Command(name, body, comment)

class CMakeParseError(Exception):
    pass

def prettify(s):
    """
    Returns the pretty-print of the contents of a CMakeLists file.
    """
    return str(parse(s))

def parse(s):
    '''
    Parses a string s in CMakeLists format whose
    contents are assumed to have come from the
    file at the given path.
    '''
    nums_toks = tokenize(s)
    nums_items = list(parse_file(nums_toks))
    nums_items = attach_comments_to_commands(nums_items)
    items = [item for _, item in nums_items]
    return File(items)

def parse_file(toks):
    '''
    Yields line number ranges and top-level elements of the syntax tree for
    a CMakeLists file, given a generator of tokens from the file.

    toks must really be a generator, not a list, for this to work.
    '''
    prev_type = 'newline'
    for line_num, (typ, tok_contents) in toks:
        if typ == 'comment':
            yield ([line_num], Comment(tok_contents))
        elif typ == 'newline' and prev_type == 'newline':
            yield ([line_num], BlankLine())
        elif typ == 'word':
            line_nums, cmd = parse_command(line_num, tok_contents, toks)
            yield (line_nums, cmd)
        prev_type = typ

def attach_comments_to_commands(nodes):
    return merge_pairs(nodes, command_then_comment, attach_comment_to_command)

def command_then_comment(a, b):
    line_nums_a, thing_a = a
    line_nums_b, thing_b = b
    return (isinstance(thing_a, _Command) and
            isinstance(thing_b, Comment) and
            set(line_nums_a).intersection(line_nums_b))

def attach_comment_to_command(lnums_command, lnums_comment):
    command_lines, command = lnums_command
    _, comment = lnums_comment
    return command_lines, Command(command.name, command.body[:], comment)

def parse_command(start_line_num, command_name, toks):
    cmd = Command(name=command_name, body=[], comment=None)
    expect('left paren', toks)
    for line_num, (typ, tok_contents) in toks:
        if typ == 'right paren':
            line_nums = range(start_line_num, line_num + 1)
            return line_nums, cmd
        elif typ == 'left paren':
            raise ValueError('Unexpected left paren at line %s' % line_num)
        elif typ in ('word', 'string'):
            cmd.body.append(Arg(tok_contents, []))
        elif typ == 'comment':
            c = tok_contents
            if cmd.body:
                cmd.body[-1].comments.append(c)
            else:
                cmd.comments.append(c)
    msg = 'File ended while processing command "%s" started at line %s' % (
        command_name, start_line_num)
    raise CMakeParseError(msg)

def expect(expected_type, toks):
    line_num, (typ, tok_contents) = toks.next()
    if typ != expected_type:
        msg = 'Expected a %s, but got "%s" at line %s' % (
            expected_type, tok_contents, line_num)
        raise CMakeParseError(msg)

# http://stackoverflow.com/questions/691148/pythonic-way-to-implement-a-tokenizer
scanner = re.Scanner([
    (r'#.*',                lambda scanner, token: ("comment", token)),
    (r'"[^"]*"',            lambda scanner, token: ("string", token)),
    (r"\(",                 lambda scanner, token: ("left paren", token)),
    (r"\)",                 lambda scanner, token: ("right paren", token)),
    (r'[^ \t\r\n()#"]+',    lambda scanner, token: ("word", token)),
    (r'\n',                 lambda scanner, token: ("newline", token)),
    (r"\s+",                None), # skip other whitespace
])

def tokenize(s):
    """
    Yields pairs of the form (line_num, (token_type, token_contents))
    given a string containing the contents of a CMakeLists file.
    """
    toks, remainder = scanner.scan(s)
    line_num = 1
    if remainder != '':
        msg = 'Unrecognized tokens at line %s: %s' % (line_num, remainder)
        raise ValueError(msg)
    for tok_type, tok_contents in toks:
        yield line_num, (tok_type, tok_contents.strip())
        line_num += tok_contents.count('\n')
