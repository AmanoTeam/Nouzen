/*
This file is auto-generated. Use the tool at ../tools/program_help.h.py to regenerate.
*/

#if !defined(PROGRAM_HELP_H)
#define PROGRAM_HELP_H

#define PROGRAM_HELP \
	"usage: nz [-h] [-v] [--update] [-i PACKAGE] [-u PACKAGE] [-s PACKAGE] [-c CONCURRENCY] [-f] [-p PREFIX] [-y] [--loglevel LOGLEVEL]\n"\
	"\n"\
	"A command-line utility for downloading and installing packages from APT repositories.\n"\
	"\n"\
	"options:\n"\
	"  -h, --help            Display this help text and exit.\n"\
	"  -v, --version         Print version information and exit.\n"\
	"  --update              Update the local package index from remote repositories.\n"\
	"  -i PACKAGE, --install PACKAGE\n"\
	"                        Install one or more packages. Use a semicolon-separated list (e.g. 'pkg1;pkg2').\n"\
	"  -u PACKAGE, --uninstall PACKAGE\n"\
	"                        Uninstall one or more packages. Use a semicolon-separated list (e.g. 'pkg1;pkg2').\n"\
	"  -s PACKAGE, --search PACKAGE\n"\
	"                        Search available repositories for packages matching the given query.\n"\
	"  -c CONCURRENCY, --concurrency CONCURRENCY\n"\
	"                        Set the number of parallel downloads. Use '0' for automatic detection, or '1' to disable parallelism.\n"\
	"  -f, --force-refresh   Force a complete rebuild of the local repository index.\n"\
	"  -p PREFIX, --prefix PREFIX\n"\
	"                        Specify an alternate installation root (prefix) for packages.\n"\
	"  -y, --assume-yes      Automatically answer 'yes' to all prompts (non-interactive mode).\n"\
	"  --loglevel LOGLEVEL   Set output verbosity. Valid levels: 'quiet', 'standard', 'warning', 'error', 'verbose'.\n"\
	"\n"\
	"Note: options that take a value must use an equal sign (e.g. --install=PACKAGE).\n"\

#endif
