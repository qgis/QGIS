#!/bin/bash

echo "== [configure release-asan] =="
cmake --preset release-asan "$@"
echo ""

echo "== [configure release-msan] =="
cmake --preset release-msan "$@"
echo ""

echo "== [configure release-ubsan] =="
cmake --preset release-ubsan "$@"
echo ""
