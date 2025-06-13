/*
This file is auto-generated. Use the tool at ../tools/program_help.h.py to regenerate.
*/

#if !defined(PROGRAM_HELP_H)
#define PROGRAM_HELP_H

#define PROGRAM_HELP \
	"usage: nz [-h] [-v] [--update] [-i PACKAGE] [-u PACKAGE] [-c CONCURRENCY] [-f] [-p PREFIX] [-y] [--loglevel LOGLEVEL]\n"\
	"\n"\
	"A command-line utility to download and install packages from APT repositories.\n"\
	"\n"\
	"options:\n"\
	"  -h, --help            Show this help message and exit.\n"\
	"  -v, --version         Display the Nouzen version and exit.\n"\
	"  --update              Update the repository package index.\n"\
	"  -i PACKAGE, --install PACKAGE\n"\
	"                        Specify a semicolon-separated (;) list of packages to install.\n"\
	"  -u PACKAGE, --uninstall PACKAGE\n"\
	"                        Specify a semicolon-separated (;) list of packages to uninstall.\n"\
	"  -c CONCURRENCY, --concurrency CONCURRENCY\n"\
	"                        Specify how many packages should be downloaded simultaneously. Pass (0) to use the number of available CPU cores or (1) to disable parallelism.\n"\
	"  -f, --force-refresh   Invalidate the cached repository index and force a refresh of the package list.\n"\
	"  -p PREFIX, --prefix PREFIX\n"\
	"                        Specify the install location for packages.\n"\
	"  -y, --assume-yes      Assume 'yes' for all prompts and run in non-interactive mode.\n"\
	"  --loglevel LOGLEVEL   Set the logging level to show more or fewer logs. Options are 'quiet', 'standard', 'warning', 'error', or 'verbose'.\n"\
	"\n"\
	"Note, options that take an argument require a equal sign. E.g. --install=PACKAGE\n"\

#endif
