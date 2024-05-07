#!/usr/bin/env python3

import argparse
import os
import io
import json

parser = argparse.ArgumentParser(
	prog = "nz",
	description = "A command-line utility to download and install packages from APT repositories.",
	allow_abbrev = False,
	add_help = False,
	epilog = "Note, options that take an argument require a equal sign. E.g. --install=PACKAGE"
)

parser.add_argument(
	"-h",
	"--help",
	required = False,
	action = "store_true",
	help = "Show this help message and exit."
)

parser.add_argument(
	"-v",
	"--version",
	action = "store_true",
	help = "Display the Nouzen version and exit."
)

parser.add_argument(
	"--update",
	required = False,
	action = "store_true",
	help = "Update the repository package index."
)

parser.add_argument(
	"-i",
	"--install",
	metavar = "PACKAGE",
	required = False,
	help = "Specify a semicolon-separated (;) list of packages to install."
)

parser.add_argument(
	"-u",
	"--uninstall",
	metavar = "PACKAGE",
	required = False,
	help = "Specify a semicolon-separated (;) list of packages to uninstall."
)

parser.add_argument(
	"-c",
	"--concurrency",
	required = False,
	help = "Specify how many packages should be downloaded simultaneously. Pass (0) to use the number of available CPU cores or (1) to disable parallelism."
)

parser.add_argument(
	"-f",
	"--force-refresh",
	required = False,
	action = "store_true",
	help = "Invalidate the cached repository index and force a refresh of the package list. The main difference between this and --update is that you can use it in combination with -i/--install."
)

parser.add_argument(
	"-p",
	"--prefix",
	required = False,
	help = "Specify the install location for packages."
)

parser.add_argument(
	"-y",
	"--assume-yes",
	required = False,
	action = "store_true",
	help = "Assume 'yes' for all prompts and run in non-interactive mode."
)

parser.add_argument(
	"--loglevel",
	required = False,
	help = "Set the logging level to show more or fewer logs. Options are 'quiet', 'standard', 'warning', 'error', or 'verbose'."
)

os.environ["LINES"] = "1000"
os.environ["COLUMNS"] = "1000"

file = io.StringIO()
parser.print_help(file = file)
file.seek(0, io.SEEK_SET)

text = file.read()

header = """/*
This file is auto-generated. Use the tool at ../tools/program_help.h.py to regenerate.
*/

#if !defined(PROGRAM_HELP_H)
#define PROGRAM_HELP_H

#define PROGRAM_HELP \\\n\
"""

for line in text.splitlines():
	line = json.dumps(obj = line + "\n")
	header += '\t%s\\\n' % line

header += "\n#endif\n"

destination = os.path.join(
	os.path.dirname(
		p = os.path.dirname(
			p = os.path.realpath(
				filename = __file__
			)
		)
	),
	"src/program_help.h"
)
	
print("Saving to '%s'" % (destination))

with open(file = destination, mode = "w") as file:
	file.write(header)
