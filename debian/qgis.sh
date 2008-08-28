#!/bin/sh

LD_LIBRARY_PATH=/usr/lib/grass/lib
export LD_LIBRARY_PATH

if [ "$LD_LIBRARY_PATH" = "" ]; then
	LD_LIBRARY_PATH=/usr/lib/grass/lib
else
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/grass/lib
fi

exec $0.bin "$@"
