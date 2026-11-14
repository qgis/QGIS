#!/bin/bash

echo "== [configure debug-demos] =="
cmake --preset debug-demos "$@"
echo ""

echo "== [configure release-demos] =="
cmake --preset release-demos "$@"
echo ""
