#!/bin/sh

if [ "$LD_LIBRARY_PATH" = "" ]; then
	LD_LIBRARY_PATH=/usr/lib/grass/lib:/usr/lib/grass64/lib
else
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/grass/lib:/usr/lib/grass64/lib
fi

export LD_LIBRARY_PATH

exec $0.bin "$@"
