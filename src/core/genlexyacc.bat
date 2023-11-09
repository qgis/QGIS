@echo off
IF EXIST qgssearchstringlexer.cc del qgssearchstringlexer.cc
IF EXIST qgssearchstringparser.h del qgssearchstringparser.h
IF EXIST qgssearchstringparser.cc del qgssearchstringparser.cc
flex qgssearchstringlexer.ll
ren lex.yy.c qgssearchstringlexer.cc
bison -y -d qgssearchstringparser.yy
ren y.tab.c qgssearchstringparser.cc
ren y.tab.h qgssearchstringparser.h


