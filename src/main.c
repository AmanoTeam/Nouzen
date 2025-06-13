#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32) && defined(_UNICODE)
	#include "wio.h"
	#define main wmain
#endif

#include "argparse.h"
#include "errors.h"
#include "options.h"
#include "biggestint.h"
#include "repository.h"
#include "logging.h"
#include "sslcerts.h"
#include "nouzen.h"
#include "wcurl.h"
#include "program_help.h"
#include "os/cpu.h"
#include "distros.h"
#include "nouzen.h"

#define PKGS_QUEUE_MAX (128)

static const char KOPT_I[] = "i";
static const char KOPT_INSTALL[] = "install";

static const char KOPT_U[] = "u";
static const char KOPT_UNINSTALL[] = "uninstall";

static const char KOPT_R[] = "r";
static const char KOPT_REMOVE[] = "remove";

static const char KOPT_C[] = "c";
static const char KOPT_CONCURRENCY[] = "concurrency";
static const char KOPT_PARALLELISM[] = "parallelism";

static const char KOPT_F[] = "f";
static const char KOPT_FORCE_REFRESH[] = "force-refresh";
static const char KOPT_IGNORE_CACHED_INDEX[] = "ignore-cached-index";

static const char KOPT_P[] = "p";
static const char KOPT_PREFIX[] = "prefix";
static const char KOPT_INSTALL_PREFIX[] = "install-prefix";

static const char KOPT_Y[] = "y";
static const char KOPT_YES[] = "yes";
static const char KOPT_ASSUME_YES[] = "assume-yes";

static const char KOPT_LOGLEVEL[] = "loglevel";

static const char KOPT_UPDATE[] = "update";
static const char KOPT_REFRESH[] = "refresh";

static const char KOPT_DESTROY[] = "destroy";

static const char KOPT_H[] = "h";
static const char KOPT_HELP[] = "help";

static const char KOPT_V[] = "v";
static const char KOPT_VERSION[] = "version";

#define ACTION_UNKNOWN (0x00)
#define ACTION_INSTALL (0x01)
#define ACTION_UNINSTALL (0x02)
#define ACTION_PARALLELISM (0x03)
#define ACTION_FORCE_REFRESH (0x04)
#define ACTION_PREFIX (0x05)
#define ACTION_ASSUME_YES (0x06)
#define ACTION_LOGLEVEL (0x07)
#define ACTION_UPDATE (0x08)
#define ACTION_DESTROY (0x09)
#define ACTION_HELP (0x10)
#define ACTION_VERSION (0x11)

static int get_action(const arg_t* const arg) {
	
	int status = 0;
	
	status = (strcmp(arg->key, KOPT_I) == 0 || strcmp(arg->key, KOPT_INSTALL) == 0);
	
	if (status) {
		return ACTION_INSTALL;
	}
	
	status = (
		strcmp(arg->key, KOPT_U) == 0 || strcmp(arg->key, KOPT_UNINSTALL) == 0 ||
		strcmp(arg->key, KOPT_R) == 0 || strcmp(arg->key, KOPT_REMOVE) == 0
	);
	
	if (status) {
		return ACTION_UNINSTALL;
	}
	
	status = (
		strcmp(arg->key, KOPT_C) == 0 || strcmp(arg->key, KOPT_CONCURRENCY) == 0 ||
		strcmp(arg->key, KOPT_PARALLELISM) == 0
	);
	
	if (status) {
		return ACTION_PARALLELISM;
	}
	
	status = (
		strcmp(arg->key, KOPT_F) == 0 ||
		strcmp(arg->key, KOPT_FORCE_REFRESH) == 0 ||
		strcmp(arg->key, KOPT_IGNORE_CACHED_INDEX) == 0
	);
	
	if (status) {
		return ACTION_FORCE_REFRESH;
	}
	
	status = (
		strcmp(arg->key, KOPT_P) == 0 ||
		strcmp(arg->key, KOPT_PREFIX) == 0 ||
		strcmp(arg->key, KOPT_INSTALL_PREFIX) == 0
	);
	
	if (status) {
		return ACTION_PREFIX;
	}
	
	status = (
		strcmp(arg->key, KOPT_Y) == 0 ||
		strcmp(arg->key, KOPT_YES) == 0 ||
		strcmp(arg->key, KOPT_ASSUME_YES) == 0
	);
	
	if (status) {
		return ACTION_ASSUME_YES;
	}
	
	status = (strcmp(arg->key, KOPT_LOGLEVEL) == 0);
	
	if (status) {
		return ACTION_LOGLEVEL;
	}
	
	status = (
		strcmp(arg->key, KOPT_UPDATE) == 0 ||
		strcmp(arg->key, KOPT_REFRESH) == 0
	);
	
	if (status) {
		return ACTION_UPDATE;
	}
	
	status = (strcmp(arg->key, KOPT_DESTROY) == 0);
	
	if (status) {
		return ACTION_DESTROY;
	}
	
	status = (
		strcmp(arg->key, KOPT_H) == 0 ||
		strcmp(arg->key, KOPT_HELP) == 0
	);
	
	if (status) {
		return ACTION_HELP;
	}
	
	status = (
		strcmp(arg->key, KOPT_V) == 0 ||
		strcmp(arg->key, KOPT_VERSION) == 0
	);
	
	if (status) {
		return ACTION_VERSION;
	}
	
	return ACTION_UNKNOWN;
	
}

int main(int argc, argv_t* argv[]) {
	
	int err = 0;
	size_t index = 0;
	size_t size = 0;
	
	int action = 0;
	int operation = 0;
	ssize_t nproc = 0;
	
	char* operating_system = NULL;
	char* config_dir = NULL;
	
	char* value = NULL;
	char* packages[PKGS_QUEUE_MAX];
	
	repolist_t list = {0};
	
	argparse_t argparse = {0};
	const arg_t* arg = NULL;
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	options_t* options = NULL;
	
	logging_t loglevel = LOG_QUIET;
	
	wcurl_t* wcurl = NULL;
	wcurl_error_t* wcurl_error = NULL;
	
	#if defined(_WIN32) && defined(_UNICODE)
		wio_enable_unicode();
	#endif
	
	config_dir = repo_get_config_dir();
	
	if (config_dir == NULL) {
		err = APTERR_REPO_GET_CONFDIR_FAILURE;
		goto end;
	}
	
	err = options_load(config_dir);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	options = get_options();
	
	wcurl = wcurl_getglobal();
	
	if (wcurl == NULL) {
		err = APTERR_WCURL_INIT_FAILURE;
		goto end;
	}
	
	operating_system = get_platform();
	
	if (operating_system == NULL) {
		err = APTERR_PLATFORM_UNKNOWN;
		goto end;
	}
	
	wcurl_error = wcurl_geterr(wcurl);
	
	err = argparse_init(&argparse, argc, argv);
	
	if (err != ARGPARSE_ERR_SUCCESS) {
		switch (err) {
			case ARGPARSE_ERR_VALUE_UNEXPECTED:
				err = APTERR_ARGPARSE_VALUE_UNEXPECTED;
				break;
			case ARGPARSE_ERR_ARGUMENT_EMPTY:
				err = APTERR_ARGPARSE_ARGUMENT_EMPTY;
				break;
			case ARGPARSE_ERR_MEM_ALLOC_FAILURE:
				err = APTERR_MEM_ALLOC_FAILURE;
				break;
			default:
				break;
		}
		
		goto end;
	}
	
	while ((arg = argparse_getnext(&argparse)) != NULL) {
		action = get_action(arg);
		
		switch (action) {
			case ACTION_INSTALL:
			case ACTION_UNINSTALL:
			case ACTION_PARALLELISM:
			case ACTION_PREFIX:
			case ACTION_LOGLEVEL: {
				if (arg->value == NULL) {
					err = APTERR_ARGPARSE_ARGUMENT_VALUE_MISSING;
					goto end;
				}
				
				break;
			}
			default: {
				break;
			}
		}
		
		switch (action) {
			case ACTION_INSTALL:
			case ACTION_UNINSTALL: {
				strsplit_init(&split, arg->value, ";");
				
				while (strsplit_next(&split, &part) != NULL) {
					if ((index + 2) > PKGS_QUEUE_MAX) {
						err = APTERR_ARGPARSE_TOO_MANY_PACKAGES;
						goto end;
					}
					
					value = malloc(part.size + 1);
					
					if (value == NULL) {
						err = APTERR_MEM_ALLOC_FAILURE;
						goto end;
					}
					
					strncpy(value, part.begin, part.size);
					value[part.size] = '\0';
					
					packages[index++] = value;
				}
				
				operation = action;
				
				break;
			}
			case ACTION_PARALLELISM: {
				options->concurrency = strtobui(arg->value, NULL, 10);
				
				if (errno == ERANGE) {
					err = APTERR_ARGPARSE_INVALID_UINT;
					goto end;
				}
				
				if (options->concurrency == 0) {
					nproc = get_nproc();
					options->concurrency = ((nproc == -1) ? 1 : nproc);
				}
				
				break;
			}
			case ACTION_UPDATE:
			case ACTION_FORCE_REFRESH: {
				options->force_refresh = 1;
				break;
			}
			case ACTION_PREFIX: {
				free(options->prefix);
				
				options->prefix = malloc(strlen(arg->value) + 1);
				
				if (options->prefix == NULL) {
					err = APTERR_MEM_ALLOC_FAILURE;
					goto end;
				}
				
				strcpy(options->prefix, arg->value);
				
				break;
			}
			case ACTION_ASSUME_YES: {
				options->assume_yes = 1;
				break;
			}
			case ACTION_LOGLEVEL: {
				loglevel = loglevel_unstringify(arg->value);
				loglevel_set(loglevel);
				break;
			}
			case ACTION_DESTROY: {
				operation = action;
				break;
			}
			case ACTION_HELP: {
				printf("%s", PROGRAM_HELP);
				goto end;
			}
			case ACTION_VERSION: {
				printf("%s v%s (%s)\n", PROJECT_NAME, PROJECT_VERSION, operating_system);
				goto end;
			}
			case ACTION_UNKNOWN: {
				err = APTERR_ARGPARSE_ARGUMENT_INVALID;
				goto end;
			}
		}
	}
	
	packages[index++] = NULL;
	
	err = repolist_load(&list);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	switch (operation) {
		case ACTION_INSTALL: {
			err = repolist_install_package(&list, packages);
			break;
		}
		case ACTION_UNINSTALL: {
			err = repolist_remove_package(&list, packages);
			break;
		}
		case ACTION_DESTROY: {
			err = repolist_destroy(&list);
			break;
		}
		default: {
			break;
		}
	}
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	
	end:;
	
	if (err != APTERR_SUCCESS && err != APTERR_CLI_USER_INTERRUPTED) {
		fprintf(stderr, "fatal error: (%i) %s", -err, apterr_getmessage(err));
		
		switch (err) {
			case APTERR_WCURL_REQUEST_FAILURE:
			case APTERR_WCURL_SETOPT_FAILURE: {
				fprintf(stderr, ": %s", wcurl_error->msg);
				break;
			}
			case APTERR_ARGPARSE_ARGUMENT_VALUE_MISSING:
			case APTERR_ARGPARSE_ARGUMENT_INVALID: {
				fprintf(stderr, ": %.*s%s", 1 + (strlen(arg->key) > 1), "--", arg->key);
				break;
			}
			case APTERR_ARGPARSE_INVALID_UINT: {
				fprintf(stderr, ": %s", arg->value);
				break;
			}
		}
		
		fprintf(stderr, "\n");
	}
	
	size = index;
	
	for (index = 0; index < size; index++) {
		value = packages[index];
		free(value);
	}
	
	sslcerts_unload_certificates();
	
	free(config_dir);
	free(operating_system);
	
	repolist_free(&list);
	argparse_free(&argparse);
	
}
