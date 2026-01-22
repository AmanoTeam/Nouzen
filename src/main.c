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
#include "format.h"
#include "wcurl.h"
#include "program_help.h"
#include "os/cpuinfo.h"
#include "os/osdetect.h"
#include "nouzen.h"
#include "term/keyboard.h"
#include "term/screen.h"

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

static const char KOPT_S[] = "s";
static const char KOPT_SEARCH[] = "search";

static const char KOPT_SHOW[] = "show";

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
#define ACTION_SEARCH (0x12)
#define ACTION_SHOW (0x13)

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
	
	status = (
		strcmp(arg->key, KOPT_SEARCH) == 0 ||
		strcmp(arg->key, KOPT_S) == 0
	);
	
	if (status) {
		return ACTION_SEARCH;
	}
	
	status = (
		strcmp(arg->key, KOPT_SHOW) == 0
	);
	
	if (status) {
		return ACTION_SHOW;
	}
	
	return ACTION_UNKNOWN;
	
}

static int repolist_perform_search(repolist_t* const repolist, const char* const query) {
	
	int err = APTERR_SUCCESS;
	int paginate = 1;
	
	ssize_t status = 0;
	size_t index = 0;
	
	repo_t* repo = NULL;
	
	cir_t cir = {0};
	const cir_key_t* key = NULL;
	
	pkgs_t pkgs = {0};
	pkgs_paging_t paging = {0};
	
	pkg_t* pkg = NULL;
	
	paging.maximum = 15;
	
	cir_init(&cir);
	
	hide_cursor();
	
	while (1) {
		status = repolist_search_pkg(repolist, query, paging, &pkgs);
		
		if (paging.position == 0) {
			if (status < 1) {
				err = APTERR_PACKAGE_SEARCH_NO_MATCHES;
				goto end;
			}
			
			paginate = (((size_t) status) == paging.maximum);
		}
		
		if (paginate) {
			erase_screen();
		}
		
		for (index = 0; index < pkgs.offset; index++) {
			pkg = pkgs.items[index];
			
			repo = repolist_get_pkg_repo(repolist, pkg);
			
			printf(
				"\r\n\033[92m%s\033[0m/%s %s %s\r\n  %s\r\n",
				pkg->name,
				repo->release,
				pkg->version,
				repoarch_unstringify(pkg->arch),
				pkg->description
			);
		}
		
		if (!paginate) {
			printf("\r\n");
			goto end;
		}
		
		printf("\r\nPress 'q' to quit, or navigate using the arrow keys (left, up, right, down).");
		fflush(stdout);
		
		input:;
		
		key = cir_get(&cir);
		
		switch (key->type) {
			case KEY_PAGE_UP:
			case KEY_ARROW_LEFT:
			case KEY_ARROW_UP:
			case KEY_HOME: {
				if (paging.position == 0) {
					goto input;
				}
				
				paging.position -= 1;
				
				break;
			}
			case KEY_PAGE_DOWN:
			case KEY_ARROW_RIGHT:
			case KEY_ARROW_DOWN:
			case KEY_END: {
				if (((size_t) status) < paging.maximum) {
					goto input;
				}
				
				paging.position += 1;
				
				break;
			}
			case KEY_CTRL_BACKSLASH:
			case KEY_CTRL_C:
			case KEY_CTRL_D:
			case KEY_Q: {
				printf("\r\n");
				goto end;
			}
			default: {
				goto input;
			}
		}
	}
	
	end:;
	
	show_cursor();
	cir_free(&cir);
	
	return err;
	
}

static int repolist_perform_show(repolist_t* const repolist, const char* const query) {
	
	int err = APTERR_SUCCESS;
	
	size_t index = 0;
	size_t subindex = 0;
	
	repo_t* repo = NULL;
	
	pkg_t* pkg = NULL;
	const pkg_t* subpkg = NULL;
	
	maintainer_t* maintainer = NULL;
	maintainers_t* maintainers = NULL;
	
	pkgs_t* pkgs = NULL;
	
	const char* key = NULL;
	
	char package_size[BTOS_MAX_SIZE];
	
	pkg = repolist_get_pkg(repolist, query);
	
	if (pkg == NULL) {
		err = APTERR_PACKAGE_SEARCH_NO_MATCHES;
		goto end;
	}
	
	err = repolist_resolve_deps(repolist, pkg);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	repo = repolist_get_pkg_repo(repolist, pkg);
	
	maintainers = pkg->maintainer;
	
	printf("\r\nPackage: %s\r\n", pkg->name);
	printf("Version: %s\r\n", pkg->version);
	
	if (pkg->maintainer != NULL) {
		printf("Maintainer: ");
		
		for (index = 0; index < maintainers->offset; index++) {
			maintainer = &maintainers->items[index];
			printf(
				"%s%s <%s>",
				((index == 0) ? "" : ", "),
				maintainer->name,
				((maintainer->email == NULL) ? "user@example.com" : maintainer->email)
			);
		}
		
		printf("\r\n");
	}
	
	btos(pkg->installed_size, package_size);
	printf("Installed-Size: %s\r\n", package_size);
	
	for (index = 0; index < 3; index++) {
		switch (index) {
			case 0: {
				key = "Depends";
				pkgs = pkg->depends;
				break;
			}
			case 1: {
				key = "Breaks";
				pkgs = pkg->breaks;
				break;
			}
			case 2: {
				key = "Replaces";
				pkgs = pkg->replaces;
				break;
			}
		}
		
		if (pkgs == NULL) {
			continue;
		}
		
		printf("%s: ", key);
		
		for (subindex = 0; subindex < pkgs->offset; subindex++) {
			subpkg = pkgs->items[subindex];
			printf("%s%s", ((subindex == 0) ? "" : ", "), subpkg->name);
		}
		
		printf("\r\n");
	}
	
	if (pkg->homepage != NULL) {
		printf("Homepage: %s\r\n", pkg->homepage);
	}
	
	btos(pkg->size, package_size);
	printf("Download-Size: %s\r\n", package_size);
	
	printf("APT-Manual-Installed: %s\r\n", ((pkg->autoinstall) ? "no": "yes"));
	
	key = repoarch_unstringify(pkg->arch);
	
	printf("APT-Sources: %s %s/%s %s Packages\r\n", repo->base_uri.value, repo->release, repo->resource, key);
	
	if (pkg->description != NULL) {
		printf("Description: %s\r\n", pkg->description);
	}
	
	printf("\r\n");
	
	end:;
	
	return err;
	
}


int main(int argc, argv_t* argv[]) {
	
	int err = 0;
	size_t index = 0;
	size_t size = 0;
	
	int action = 0;
	int operation = 0;
	ssize_t nproc = 0;
	
	const char* search_query = NULL;
	
	const char* operating_system = NULL;
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
	
	operating_system = osdetect_getplatform();
	
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
			case ACTION_LOGLEVEL:
			case ACTION_SEARCH:
			case ACTION_SHOW: {
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
				strsplit_init(&split, &part, arg->value, ";");
				
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
			case ACTION_SEARCH:
			case ACTION_SHOW: {
				search_query = arg->value;
				operation = action;
				break;
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
		case ACTION_SEARCH: {
			err = repolist_perform_search(&list, search_query);
			break;
		}
		case ACTION_SHOW: {
			err = repolist_perform_show(&list, search_query);
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
	
	repolist_free(&list);
	argparse_free(&argparse);
	
}
