#!/bin/sh

if test ! -d target
then
	mkdir target
	cd target
	env CMAKE_PREFIX_PATH=/usr/local/Cellar/qt\@5/5.15.2/ cmake ..
fi

