#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "options.h"
#include "errors.h"
#include "fs/fstream.h"
#include "query.h"
#include "logging.h"
#include "fs/sep.h"
#include "fs/getexec.h"
#include "fs/mkdir.h"
#include "fs/normpath.h"
#include "fs/sep.h"
#include "fs/exists.h"
#include "os/cpuinfo.h"

static const char KOPT_CACHE[] = "cache";
static const char KOPT_PARALLELISM[] = "parallelism";
static const char KOPT_FORCE_REFRESH[] = "force-refresh";
static const char KOPT_PREFIX[] = "prefix";
static const char KOPT_LOGLEVEL[] = "loglevel";
static const char KOPT_SKIP_MAINTAINER_SCRIPTS[] = "skip-maintainer-scripts";
static const char KOPT_SYMLINK_PREFIX[] = "symlink-prefix";

static const char VPREFIX[] = "$ORIGIN" PATHSEP_M "sysroot";
static const char VLOGLEVEL[] = "standard";
static const char VSYMLINK_PREFIX[] = "none";
static const biguint_t VCACHE = 1;
static const biguint_t VFORCE_REFRESH = 0;
static const biguint_t VPARALLELISM = 0;
static const biguint_t VSKIP_MAINTAINER_SCRIPTS = 1;

static const char OPTIONS_FILE[] = "options.conf";
static const char DOLLAR_SIGN = '$';

static options_t options = {0};

int options_load(
	const char* const directory
) {
	
	int err = APTERR_SUCCESS;
	
	int status = 0;
	size_t size = 0;
	
	hquery_t query = {0};
	
	const char* start = NULL;
	const char* end = NULL;
	
	const char* vend = NULL;
	
	unsigned char ch = 0;
	
	char* key = NULL;
	char* prefix = NULL;
	
	logging_t loglevel = LOG_QUIET;
	biguint_t concurrency = 0;
	ssize_t nproc = 0;
	
	const char* var = NULL;
	const char* value = NULL;
	
	char* app_directory = NULL;
	
	char* filename = NULL;
	
	if (options.prefix != NULL) {
		goto end;
	}
	
	filename = malloc(strlen(directory) + strlen(PATHSEP_S) + strlen(OPTIONS_FILE) + 1);
	
	if (filename == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(filename, directory);
	strcat(filename, PATHSEP_S);
	strcat(filename, OPTIONS_FILE);
	
	query_init(&query, '\n', "=");
	
	if (file_exists(filename) == 1) {
		err = query_load_file(&query, filename);
	} else {
		query_init(&query, '\n', " = ");
		
		err = query_add_string(&query, KOPT_PREFIX, VPREFIX);
		
		if (err != 0) {
			err = APTERR_PKG_METADATA_WRITE_FAILURE;
			goto end;
		}
		
		err = query_add_uint(&query, KOPT_FORCE_REFRESH, VFORCE_REFRESH);
		
		if (err != 0) {
			err = APTERR_PKG_METADATA_WRITE_FAILURE;
			goto end;
		}
		
		err = query_add_uint(&query, KOPT_CACHE, VCACHE);
		
		if (err != 0) {
			err = APTERR_PKG_METADATA_WRITE_FAILURE;
			goto end;
		}
		
		err = query_add_uint(&query, KOPT_PARALLELISM, VPARALLELISM);
		
		if (err != 0) {
			err = APTERR_PKG_METADATA_WRITE_FAILURE;
			goto end;
		}
		
		err = query_add_uint(&query, KOPT_SKIP_MAINTAINER_SCRIPTS, VSKIP_MAINTAINER_SCRIPTS);
		
		if (err != 0) {
			err = APTERR_PKG_METADATA_WRITE_FAILURE;
			goto end;
		}
		
		err = query_add_string(&query, KOPT_LOGLEVEL, VLOGLEVEL);
		
		if (err != 0) {
			err = APTERR_PKG_METADATA_WRITE_FAILURE;
			goto end;
		}
		
		err = query_add_string(&query, KOPT_SYMLINK_PREFIX, VSYMLINK_PREFIX);
		
		if (err != 0) {
			err = APTERR_PKG_METADATA_WRITE_FAILURE;
			goto end;
		}
		
		err = query_dump_file(&query, filename);
		
		if (err != 0) {
			err = APTERR_PKG_METADATA_WRITE_FAILURE;
			goto end;
		}
	}
	
	if (err != 0) {
		err = APTERR_REPO_CONF_PARSE_FAILURE;
		goto end;
	}
	
	/* Prefix */
	value = query_get_string(&query, KOPT_PREFIX);
	
	if (value == NULL) {
		err = APTERR_REPO_CONF_MISSING_FIELD;
		goto end;
	}
	
	if (value[0] == DOLLAR_SIGN) {
		vend = strchr(value, '\0');
		start = value + 1;
		end = start;
		
		while (end != vend) {
			ch = *end;
			
			if (!(isalpha(ch) || isdigit(ch))) {
				break;
			}
			
			end++;
		}
		
		size = (size_t) (end - start);
		
		if (size > 0) {
			key = malloc(size + 1);
			
			if (key == NULL) {
				err = APTERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strncpy(key, start, size);
			key[size] = '\0';
			
			if (strcmp(key, "ORIGIN") == 0) {
				app_directory = get_app_directory();
				
				if (app_directory == NULL) {
					err = APTERR_GET_APP_DIRECTORY_FAILURE;
					goto end;
				}
				
				var = app_directory;
			} else {
				var = getenv(key);
			}
		}
		
		value = end;
	}
	
	if (*value == '\0') {
		err = APTERR_REPO_CONF_MISSING_FIELD;
		goto end;
	}
	
	prefix = malloc(((var == NULL) ? 0 : strlen(var)) + strlen(value) + 1);
	
	if (prefix == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	prefix[0] = '\0';
	
	if (var != NULL) {
		strcat(prefix, var);
	}
	
	strcat(prefix, value);
	
	normpath(prefix, 0);
	
	if (create_directory(prefix) != 0) {
		err = APTERR_FS_MKDIR_FAILURE;
		goto end;
	}
	
	options.prefix = prefix;
	
	options.force_refresh = VFORCE_REFRESH;
	
	/* Force-refresh */
	status = query_get_bool(&query, KOPT_FORCE_REFRESH);
	
	if (status != -1) {
		options.force_refresh = status;
	}
	
	options.cache = VCACHE;
	
	/* Cache */
	status = query_get_bool(&query, KOPT_CACHE);
	
	if (status != -1) {
		options.cache = status;
	}
	
	options.maintainer_scripts = !VSKIP_MAINTAINER_SCRIPTS;
	
	/* Skip maintainer scripts */
	status = query_get_bool(&query, KOPT_SKIP_MAINTAINER_SCRIPTS);
	
	if (status != -1) {
		options.maintainer_scripts = !status;
	}
	
	options.concurrency = VPARALLELISM;
	
	/* Parallel-Downloads */
	concurrency = query_get_uint(&query, KOPT_PARALLELISM);
	
	if (concurrency != BIGUINT_MAX) {
		options.concurrency = concurrency;
	}
	
	if (options.concurrency == VPARALLELISM) {
		nproc = get_nproc();
		options.concurrency = ((nproc == -1) ? 1 : nproc);
	}
	
	loglevel = LOG_STANDARD;
	
	/* Log level */
	value = query_get_string(&query, KOPT_LOGLEVEL);
	
	if (value != NULL) {
		loglevel = loglevel_unstringify(value);
	}
	
	loglevel_set(loglevel);
	
	prefix = NULL;
	
	/* Symlink prefix */
	value = query_get_string(&query, KOPT_SYMLINK_PREFIX);
	
	if (!(value == NULL || strcmp(value, VSYMLINK_PREFIX) == 0)) {
		prefix = malloc(strlen(value) + 1);
		
		if (prefix == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(prefix, value);
	}
	
	options.symlink_prefix = prefix;
	
	end:;
	
	free(key);
	free(filename);
	free(app_directory);
	
	query_free(&query);
	
	return err;
	
}

options_t* get_options(void) {
	return &options;
}

void options_free(void) {
	
	free(options.prefix);
	options.prefix = NULL;
	
	free(options.symlink_prefix);
	options.symlink_prefix = NULL;
	
	options.force_refresh = 0;
	options.cache = 0;
	options.concurrency = 0;
	
}