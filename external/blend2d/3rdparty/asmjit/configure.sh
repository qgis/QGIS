#!/bin/bash

echo "== [configure debug] =="
cmake --preset debug "$@"
echo ""

echo "== [configure release] =="
cmake --preset release "$@"
echo ""
