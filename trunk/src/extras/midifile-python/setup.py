#!/usr/bin/env python

import distutils.core

distutils.core.setup(name = "CMidiFile", ext_modules = [distutils.core.Extension("CMidiFile", sources = ["CMidiFile.c", "../midifile/midifile.c"], include_dirs = ["../midifile"])])

