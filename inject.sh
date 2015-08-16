#!/bin/bash
open=`ps -e | grep Game.exe | wc -l`
if [ $open -gt 0 ]; then
	pid=`ps -e | grep -m 1 Game.exe | sed -e 's/^ *//' -e 's/^\([0-9][0-9]*\) .*/\1/'`
	surgeon -i $pid /usr/local/lib/snoogans.so
else
	echo "err: couldn't find Game.exe"
fi

