#!/usr/bin/env python3

import argparse
import os
import io
import json

parser = argparse.ArgumentParser(
	prog = "nz",
	description = "A command-line utility for downloading and installing packages from APT repositories.",
	allow_abbrev = False,
	add_help = False,
	epilog = "Note: options that take a value must use an equal sign (e.g. --install=PACKAGE)."
)

parser.add_argument(
	"-h",
	"--help",
	required = False,
	action = "store_true",
	help = "Display this help text and exit."
)

parser.add_argument(
	"-v",
	"--version",
	action = "store_true",
	help = "Print version information and exit."
)

parser.add_argument(
	"--update",
	required = False,
	action = "store_true",
	help = "Synchronize the local package index with remote repositories."
)

parser.add_argument(
	"-i",
	"--install",
	metavar = "PACKAGE",
	required = False,
	help = "Install one or more packages. Use a semicolon-separated list (e.g. 'pkg1;pkg2')."
)

parser.add_argument(
	"-u",
	"--uninstall",
	metavar = "PACKAGE",
	required = False,
	help = "Uninstall one or more packages. Use a semicolon-separated list (e.g. 'pkg1;pkg2')."
)

parser.add_argument(
	"-s",
	"--search",
	metavar = "PACKAGE",
	required = False,
	help = "Search available repositories for packages matching the given query."
)

parser.add_argument(
	"-c",
	"--concurrency",
	required = False,
	help = "Set the number of parallel downloads. Use '0' for automatic detection, or '1' to disable parallelism."
)

parser.add_argument(
	"-f",
	"--force-refresh",
	required = False,
	action = "store_true",
	help = "Force a complete rebuild of the local repository index."
)

parser.add_argument(
	"-p",
	"--prefix",
	required = False,
	help = "Specify an alternate installation root (prefix) for packages."
)

parser.add_argument(
	"-y",
	"--assume-yes",
	required = False,
	action = "store_true",
	help = "Automatically answer 'yes' to all prompts (non-interactive mode)."
)

parser.add_argument(
	"--loglevel",
	required = False,
	help = "Set output verbosity. Valid levels: 'quiet', 'standard', 'warning', 'error', 'verbose'."
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
