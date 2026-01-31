#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "ask.h"
#include "buffer.h"
#include "downloader.h"
#include "errors.h"
#include "format.h"
#include "fs/absrel.h"
#include "fs/basename.h"
#include "fs/chdir.h"
#include "fs/exists.h"
#include "fs/fstream.h"
#include "fs/getexec.h"
#include "fs/mkdir.h"
#include "fs/realpath.h"
#include "fs/rm.h"
#include "fs/sep.h"
#include "fs/splitext.h"
#include "fs/symlink.h"
#include "fs/walkdir.h"
#include "guess_file_format.h"
#include "guess_uri.h"
#include "logging.h"
#include "nouzen.h"
#include "options.h"
#include "os/envdir.h"
#include "os/osdetect.h"
#include "os/shell.h"
#include "package.h"
#include "pprint.h"
#include "progress_callback.h"
#include "query.h"
#include "repository.h"
#include "strsplit.h"
#include "term/keyboard.h"
#include "term/screen.h"
#include "uncompress.h"
#include "wcurl.h"
#include "write_callback.h"

static const char ETC_DIRECTORY[] = 
	PATHSEP_M
	"etc";

static const char SOURCES_LIST_DIRECTORY[] = 
	PATHSEP_M
	"sources.list";

static const char SOURCES_CACHE_DIRECTORY[] = 
	PATHSEP_M
	"sources.cache";

static const char PACKAGES_DIRECTORY[] = 
	PATHSEP_M
	"packages.installed";

static const char WCURL_USER_AGENT[] = 
	PROJECT_NAME
	PATHSEP_POSIX_M
	PROJECT_VERSION;
	
static const char BRACKETS_START[] = " (";
static const char BRACKETS_END[] = ")";

static const char CONF_FILE_EXT[] = ".conf";

static const char TAR_FILE_EXT[] = ".tar";
static const char XZ_FILE_EXT[] = ".xz";
static const char BZ2_FILE_EXT[] = ".bz2";
static const char ZST_FILE_EXT[] = ".zst";
static const char GZ_FILE_EXT[] = ".gz";

static const char KCONFIGURE[] = "configure";
static const char KUPGRADE[] = "upgrade";
static const char KINSTALL[] = "install";
static const char KDISTS[] = "dists";
static const char KBINARY[] = "binary-";
static const char KPACKAGES[] = "Packages";
static const char KAPKINDEX[] = "APKINDEX";
static const char KHYPHEN[] = "-";

static const char* const SYSTEM_LIBRARY_PATH[] = {
	"usr" PATHSEP_M "local" PATHSEP_M "lib64",
	"usr" PATHSEP_M "local" PATHSEP_M "lib",
	"lib64",
	"lib",
	"usr" PATHSEP_M "lib64",
	"usr" PATHSEP_M "lib"
};

static const char ELF_MAGIC_NUMBERS[] = {0x7f, 0x45, 0x4c, 0x46};

static const char* const PACKAGES_FILE_EXT[] = {
	XZ_FILE_EXT,
	BZ2_FILE_EXT,
	ZST_FILE_EXT,
	GZ_FILE_EXT,
	""
};

const char* get_loader(const architecture_t architecture) {
	
	switch (architecture) {
		case ARCH_AMD64:
			return "lib64" PATHSEP_M "ld-linux-x86-64.so.2";
		case ARCH_i386:
			return "lib" PATHSEP_M "ld-linux.so.2";
		case ARCH_ARMEL:
			return "lib" PATHSEP_M "ld-linux.so.3";
		case ARCH_ARMHF:
			return "lib" PATHSEP_M "ld-linux-armhf.so.3";
		case ARCH_MIPS64EL:
			return "lib64" PATHSEP_M "ld.so.1";
		case ARCH_PPC64EL:
			return "lib64" PATHSEP_M "ld64.so.2";
		case ARCH_S390X:
			return "lib" PATHSEP_M "ld64.so.1";
		case ARCH_MIPS:
			return "lib" PATHSEP_M "ld.so.1";
		case ARCH_MIPSEL:
			return "lib" PATHSEP_M "ld.so.1";
		case ARCH_ARM64:
			return "lib" PATHSEP_M "ld-linux-aarch64.so.1";
		case ARCH_IA64:
			return "lib" PATHSEP_M "ld-linux-ia64.so.2";
		case ARCH_ALPHA:
			return "lib" PATHSEP_M "ld-linux.so.2";
		case ARCH_S390:
			return "lib" PATHSEP_M "ld.so.1";
		case ARCH_SPARC:
			return "lib" PATHSEP_M "ld-linux.so.2";
		case ARCH_HPPA:
			return "lib" PATHSEP_M "ld.so.1";
		case ARCH_POWERPC:
			return "lib" PATHSEP_M "ld.so.1";
		default:
			break;
	}
	
	return NULL;
	
}

const char* get_triplet(const architecture_t architecture) {
	
	switch (architecture) {
		case ARCH_AMD64:
			return "x86_64-linux-gnu";
		case ARCH_i386:
			return "i386-linux-gnu";
		case ARCH_ARMEL:
			return "arm-linux-gnueabi";
		case ARCH_ARMHF:
			return "arm-linux-gnueabihf";
		case ARCH_MIPS64EL:
			return "mips64el-linux-gnuabi64";
		case ARCH_PPC64EL:
			return "powerpc64le-linux-gnu";
		case ARCH_S390X:
			return "s390x-linux-gnu";
		case ARCH_MIPS:
			return "mips-linux-gnu";
		case ARCH_MIPSEL:
			return "mipsel-linux-gnu";
		case ARCH_ARM64:
			return "aarch64-linux-gnu";
		case ARCH_IA64:
			return "ia64-linux-gnu";
		case ARCH_ALPHA:
			return "alpha-linux-gnu";
		case ARCH_S390:
			return "s390-linux-gnu";
		case ARCH_SPARC:
			return "sparc-linux-gnu";
		case ARCH_HPPA:
			return "hppa-linux-gnu";
		case ARCH_POWERPC:
			return "powerpc-linux-gnu";
		default:
			break;
	}
	
	return NULL;
	
}

char* get_local_temp_dir(void) {
	
	char* directory = NULL;
	char* temporary_directory = NULL;
	
	directory = get_temp_dir();
	
	if (directory == NULL) {
		return directory;
	}
	
	temporary_directory = malloc(
		strlen(directory) +
		strlen(PATHSEP_S) +
		strlen(PROJECT_NAME_LOWERCASE) + 1
	);
	
	if (temporary_directory == NULL) {
		free(directory);
		return NULL;
	}
	
	strcpy(temporary_directory, directory);
	strcat(temporary_directory, PATHSEP_S);
	strcat(temporary_directory, PROJECT_NAME_LOWERCASE);
	
	if (create_directory(temporary_directory) != 0) {
		free(directory);
		free(temporary_directory);
		return NULL;
	}
	
	return temporary_directory;
	
}

char* repo_get_config_dir(void) {
	
	char* app_directory = NULL;
	char* name = NULL;
	
	app_directory = get_app_directory();
	
	if (app_directory == NULL) {
		goto end;
	}
	
	name = malloc(
		strlen(app_directory) +
		strlen(ETC_DIRECTORY) +
		strlen(PATHSEP_S) +
		strlen(PROJECT_NAME_LOWERCASE) +
		1
	);
	
	if (name == NULL) {
		goto end;
	}
	
	strcpy(name, app_directory);
	strcat(name, ETC_DIRECTORY);
	strcat(name, PATHSEP_S);
	strcat(name, PROJECT_NAME_LOWERCASE);
	
	if (create_directory(name) != 0) {
		free(name);
		name = NULL;
	}
	
	end:;
	
	free(app_directory);
	
	return name;
	
}

char* repo_get_pkgs_dir(void) {
	
	char* config_dir = NULL;
	char* pkgs_dir = NULL;
	
	config_dir = repo_get_config_dir();
	
	if (config_dir == NULL) {
		goto end;
	}
	
	pkgs_dir = malloc(strlen(config_dir) + strlen(PACKAGES_DIRECTORY) + 1);
	
	if (pkgs_dir == NULL) {
		goto end;
	}
	
	strcpy(pkgs_dir, config_dir);
	strcat(pkgs_dir, PACKAGES_DIRECTORY);
	
	if (create_directory(pkgs_dir) == 0) {
		goto end;
	}
	
	free(pkgs_dir);
	pkgs_dir = NULL;
	
	end:;
	
	free(config_dir);
	
	return pkgs_dir;
	
}

char* repo_get_src_dir(void) {
	
	char* config_dir = NULL;
	char* sources_directory = NULL;
	
	config_dir = repo_get_config_dir();
	
	if (config_dir == NULL) {
		goto end;
	}
	
	sources_directory = malloc(strlen(config_dir) + strlen(SOURCES_LIST_DIRECTORY) + 1);
	
	if (sources_directory == NULL) {
		goto end;
	}
	
	strcpy(sources_directory, config_dir);
	strcat(sources_directory, SOURCES_LIST_DIRECTORY);
	
	if (create_directory(sources_directory) == 0) {
		goto end;
	}
	
	free(sources_directory);
	sources_directory = NULL;
	
	end:;
	
	free(config_dir);
	
	return sources_directory;
	
}

char* pkg_get_installation(const pkg_t* const pkg) {
	
	char* pkgs_dir = NULL;
	char* file = NULL;
	
	pkgs_dir = repo_get_pkgs_dir();
	
	if (pkgs_dir == NULL) {
		goto end;
	}
	
	file = malloc(
		strlen(pkgs_dir) +
		strlen(PATHSEP_S) +
		strlen(pkg->name) +
		1
	);
	
	if (file == NULL) {
		goto end;
	}
	
	strcpy(file, pkgs_dir);
	strcat(file, PATHSEP_S);
	strcat(file, pkg->name);
	
	end:;
	
	free(pkgs_dir);
	
	return file;
	
}

char* repo_get_cache_dir(void) {
	
	char* config_dir = NULL;
	char* cache_dir = NULL;
	
	config_dir = repo_get_config_dir();
	
	if (config_dir == NULL) {
		goto end;
	}
	
	cache_dir = malloc(strlen(config_dir) + strlen(SOURCES_CACHE_DIRECTORY) + 1);
	
	if (cache_dir == NULL) {
		goto end;
	}
	
	strcpy(cache_dir, config_dir);
	strcat(cache_dir, SOURCES_CACHE_DIRECTORY);
	
	if (create_directory(cache_dir) != 0) {
		free(cache_dir);
		cache_dir = NULL;
		goto end;
	}
	
	end:;
	
	free(config_dir);
	
	return cache_dir;
	
}

char* repo_get_cache_file(repo_t* const repo) {
	
	char* cache_dir = NULL;
	char* cache = NULL;
	
	cache_dir = repo_get_cache_dir();
	
	if (cache_dir == NULL) {
		goto end;
	}
	
	cache = malloc(
		strlen(cache_dir) +
		strlen(PATHSEP_S) +
		strlen(repo->name) +
		1
	);
	
	if (cache == NULL) {
		goto end;
	}
	
	strcpy(cache, cache_dir);
	strcat(cache, PATHSEP_S);
	strcat(cache, repo->name);
	
	end:;
	
	free(cache_dir);
	
	return cache;
	
}
	
char* repo_fetch_cache(repo_t* const repo) {
	
	char* cache = NULL;
	
	int exists = 0;
	
	loggln(LOG_VERBOSE, "Checking for cached repository index from '%s'", repo->name);
	
	cache = repo_get_cache_file(repo);
	
	if (cache == NULL) {
		goto end;
	}
	
	exists = file_exists(cache) == 1;
	
	end:;
	
	loggln(LOG_VERBOSE, "Cache status: %i (%s)", exists, exists ? "HIT" : "MISS");
	
	if (!exists) {
		free(cache);
		cache = NULL;
	}
	
	return cache;
	
}

int repo_store_cache(repo_t* const repo, const char* const data, const size_t size) {
	
	int err = APTERR_SUCCESS;
	
	char* cache = NULL;
	
	fstream_t* stream = NULL;
	
	loggln(LOG_VERBOSE, "Store cache for repository index '%s'", repo->name);
	
	cache = repo_get_cache_file(repo);
	
	if (cache == NULL) {
		goto end;
	}
	
	stream = fstream_open(cache, FSTREAM_WRITE);
	
	if (stream == NULL) {
		err = APTERR_FSTREAM_OPEN_FAILURE;
		goto end;
	}
	
	err = fstream_write(stream, data, size);
	
	if (err != 0) {
		err = APTERR_FSTREAM_WRITE_FAILURE;
		goto end;
	}
	
	loggln(LOG_VERBOSE, "Repository index cached at '%s'", cache);
	
	end:;
	
	fstream_close(stream);
	free(cache);
	
	return err;
	
}

base_uri_t* repo_get_uri(repo_t* const repo) {
	
	if (repo->base_uri.value == NULL) {
		return &repo->uri;
	}
	
	return &repo->base_uri;
	
}

static int repo_set_uri(
	repo_t* const repo,
	const int type,
	const char* const uri,
	const char* const base
) {
	/*
	Set the base URI of the APT repository.
	
	This can be a path to a local file or directory in the filesystem
	or an HTTP URL.
	*/
	
	int err = APTERR_SUCCESS;
	int subtype = 0;
	
	repo->uri.type = type;
	repo->uri.value = malloc(strlen(uri) + 1);
	
	if (repo->uri.value == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(repo->uri.value, uri);
	
	if (base == NULL) {
		goto end;
	}
	
	subtype = uri_guess_type(base);
	
	switch (subtype) {
		case GUESS_URI_TYPE_URL:
			repo->base_uri.type = BASE_URI_TYPE_URL;
			break;
		case GUESS_URI_TYPE_LOCAL_FILE:
			repo->base_uri.type = BASE_URI_TYPE_LOCAL_FILE;
			break;
		case GUESS_URI_TYPE_LOCAL_DIRECTORY:
			repo->base_uri.type = BASE_URI_TYPE_LOCAL_DIRECTORY;
			break;
		default:
			err = APTERR_REPO_LOAD_UNSUPPORTED_URI;
			goto end;
	}
	
	repo->base_uri.value = malloc(strlen(base) + 1);
	
	if (repo->base_uri.value == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(repo->base_uri.value, base);
	
	end:;
	
	return err;
	
}

static int repotype_unstringify(const char* const value) {
	
	if (strcmp(value, "apt") == 0) {
		return REPO_TYPE_APT;
	}
	
	if (strcmp(value, "apk") == 0) {
		return REPO_TYPE_APK;
	}
	
	return REPO_TYPE_UNKNOWN;
	
}

static const char* pkg_get_field_name(const int type, const int field) {
	
	switch (type) {
		case REPO_TYPE_APT: {
			switch (field) {
				case PKG_SECTION_FIELD_NAME:
					return "Package";
				case PKG_SECTION_FIELD_VERSION:
					return "Version";
				case PKG_SECTION_FIELD_DESCRIPTION:
					return "Description";
				case PKG_SECTION_FIELD_DEPENDS:
					return "Depends";
				case PKG_SECTION_FIELD_PROVIDES:
					return "Provides";
				case PKG_SECTION_FIELD_RECOMMENDS:
					return "Recommends";
				case PKG_SECTION_FIELD_SUGGESTS:
					return "Suggests";
				case PKG_SECTION_FIELD_BREAKS:
					return "Breaks";
				case PKG_SECTION_FIELD_REPLACES:
					return "Replaces";
				case PKG_SECTION_FIELD_MAINTAINER:
					return "Maintainer";
				case PKG_SECTION_FIELD_HOMEPAGE:
					return "Homepage";
				case PKG_SECTION_FIELD_BUGS:
					return "Bugs";
				case PKG_SECTION_FIELD_SIZE:
					return "Size";
				case PKG_SECTION_FIELD_INSTALLED_SIZE:
					return "Installed-Size";
				case PKG_SECTION_FIELD_FILENAME:
					return "Filename";
			}
			
			break;
		}
		case REPO_TYPE_APK: {
			switch (field) {
				case PKG_SECTION_FIELD_NAME:
					return "P";
				case PKG_SECTION_FIELD_VERSION:
					return "V";
				case PKG_SECTION_FIELD_DESCRIPTION:
					return "T";
				case PKG_SECTION_FIELD_DEPENDS:
					return "D";
				case PKG_SECTION_FIELD_PROVIDES:
					return "p";
				case PKG_SECTION_FIELD_MAINTAINER:
					return "m";
				case PKG_SECTION_FIELD_HOMEPAGE:
					return "U";
				case PKG_SECTION_FIELD_SIZE:
					return "S";
				case PKG_SECTION_FIELD_INSTALLED_SIZE:
					return "I";
			}
			
			break;
		}
	}
	
	return NULL;
	
}

static char* aptpkg_from_apkpkg(const char* const value) {
	/*
	Convert an APK-style (Alpine) package list into an APT-style package list.
	*/
	
	char* begin = NULL;
	const char* end = NULL;
	
	char* ptr = NULL;
	
	unsigned char a = 0;
	unsigned char b = 0;
	
	char sequence[3];
	
	char* result = NULL;
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	strsplit_t subsplit = {0};
	strsplit_part_t subpart = {0};
	
	size_t size = 0;
	
	int status = 0;
	
	result = malloc(strlen(value) * 3);
	
	if (result == NULL) {
		return result;
	}
	
	result[0] = '\0';
	
	strsplit_init(&split, &part, value, " ");
	
	while (strsplit_next(&split, &part) != NULL) {
		begin = part.begin;
		end = (part.begin + part.size);
		
		a = result[0];
		
		if (a != '\0') {
			strcat(result, ", ");
		}
		
		while (1) {
			a = *begin;
			
			if (a == ':') {
				if (strncmp("cmd:", part.begin, 4) == 0 || strncmp("so:", part.begin, 3) == 0) {
					part.size -= (++begin - part.begin);
					part.begin = begin;
					continue;
				}
				
				*begin = '-';
			}
			
			if (a == '>' || a == '<' || a == '=') {
				size = (begin - part.begin);
				
				strncat(result, part.begin, size);
				
				ptr = strchr(result, '\0');
				
				strcat(result, " (");
				
				/* Comparison operators */
				sequence[1] = '\0';
				sequence[2] = '\0';
				
				begin++;
				b = *begin;
				
				sequence[0] = a;
				
				if ((a == '>' || a == '<') && b == '=') {
					sequence[1] = b;
					begin++;
				}
				
				if (begin == end) {
					*ptr = '\0';
					break;
				}
				
				strcat(result, sequence);
				
				/* Version */
				strcat(result, " ");
				
				size = (end - begin);
				strncat(result, begin, size);
				
				strcat(result, ")");
				
				break;
			}
			
			if (begin == end) {
				strncat(result, part.begin, part.size);
				break;
			}
			
			begin++;
		}
	}
	
	/* Convert "musl (< 1.2.5-r21), musl (> 1.2.5-r21)" into "musl (= 1.2.5-r21)". */
	strsplit_init(&split, &part, result, ",");
	
	while (strsplit_next(&split, &part) != NULL) {
		if (*part.begin != '!') {
			continue;
		}
		
		begin = strsplit_findstr(&part, "(");
		
		if (begin == NULL) {
			continue;
		}
		
		begin++;
		a = begin[0];
		
		if (!((a == '<' || a == '>') && begin[1] == ' ')) {
			continue;
		}
		
		b = (a == '<' ? '>' : '<');
		
		size = (begin - part.begin);
		
		status = 0;
		
		strsplit_init(&subsplit, &subpart, result, ",");
		
		while (strsplit_next(&subsplit, &subpart) != NULL) {
			if (subpart.begin == part.begin) {
				continue;
			}
			
			if (subpart.size != part.size) {
				continue;
			}
			
			if (strncmp(part.begin, subpart.begin, size) != 0) {
				continue;
			}
			
			if (!(subpart.begin[size] == b && subpart.begin[size + 1] == ' ')) {
				continue;
			}
			
			if (strncmp(part.begin + (size + 1), subpart.begin + (size + 1), part.size - size - 1) != 0) {
				continue;
			}
			
			status = 1;
			
			part.begin[size] = '=';
			
			begin = subpart.begin;
			
			if (strsplit_next(&subsplit, &subpart) == NULL) {
				*begin = '\0';
				break;
			}
			
			size = strlen(subpart.begin) + 1;
			memmove(begin, subpart.begin, size);
			
			break;
		}
		
		if (!status) {
			continue;
		}
		
		begin = strchr(result, '\0');
		
		do {
			*begin = '\0';
			a = *(--begin);
		} while (a == ' ' || a == ',' || a == '\0');
		
		begin = part.begin + 1;
		size = strlen(begin) + 1;
		
		memmove(part.begin, begin, size);
		
		strsplit_init(&split, &part, result, ",");
	}
	
	return result;
	
}

int pkg_parse_section(
	repo_t* const repo,
	pkg_t* const pkg,
	hquery_t* const query
) {
	
	int err = APTERR_SUCCESS;
	
	const char* key = NULL;
	const char* value = NULL;
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	char* ptr = NULL;
	
	size_t size = 0;
	
	/* Package */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_NAME);
	value = query_get_string(query, key);
	
	if (value == NULL) {
		err = APTERR_PACKAGE_MISSING_NAME;
		goto end;
	}
	
	pkg->name = malloc(strlen(value) + 1);
	
	if (pkg->name == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(pkg->name, value);
	
	/* Description */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_DESCRIPTION);
	value = query_get_string(query, key);
	
	if (value != NULL) {
		pkg->description = malloc(strlen(value) + 1);
		
		if (pkg->description == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->description, value);
	}
	
	/* Homepage */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_HOMEPAGE);
	value = query_get_string(query, key);
	
	if (value != NULL) {
		pkg->homepage = malloc(strlen(value) + 1);
		
		if (pkg->homepage == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->homepage, value);
	}
	
	/* Bugs */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_BUGS);
	value = query_get_string(query, key);
	
	if (value != NULL) {
		pkg->bugs = malloc(strlen(value) + 1);
		
		if (pkg->bugs == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->bugs, value);
	}
	
	/* Maintainer */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_MAINTAINER);
	value = query_get_string(query, key);
	
	if (value != NULL) {
		pkg->maintainer = malloc(strlen(value) + 1);
		
		if (pkg->maintainer == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->maintainer, value);
	}
	
	/* Version */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_VERSION);
	value = query_get_string(query, key);
	
	if (value == NULL) {
		err = APTERR_PACKAGE_MISSING_VERSION;
		goto end;
	}
	
	pkg->version = malloc(strlen(value) + 1);
	
	if (pkg->version == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(pkg->version, value);
	
	/* Filename */
	if (repo->type == REPO_TYPE_APK) {
		pkg->filename = malloc(
			strlen(repo->release) + 
			strlen(PATHSEP_POSIX_S) +
			strlen(repo->resource) + 
			strlen(PATHSEP_POSIX_S) +
			strlen(repo->platform) + 
			strlen(PATHSEP_POSIX_S) +
			strlen(pkg->name) +
			1 /* - */ +
			strlen(pkg->version) +
			4 /* .apk */ +
			 1
		);
		
		if (pkg->filename == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->filename, repo->release);
		strcat(pkg->filename, PATHSEP_POSIX_S);
		strcat(pkg->filename, repo->resource);
		strcat(pkg->filename, PATHSEP_POSIX_S);
		strcat(pkg->filename, repo->platform);
		strcat(pkg->filename, PATHSEP_POSIX_S);
		strcat(pkg->filename, pkg->name);
		strcat(pkg->filename, "-");
		strcat(pkg->filename, pkg->version);
		strcat(pkg->filename, ".apk");
	} else {
		key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_FILENAME);
		value = query_get_string(query, key);
		
		if (value == NULL) {
			err = APTERR_PACKAGE_MISSING_FILENAME;
			goto end;
		}
		
		pkg->filename = malloc(strlen(value) + 1);
		
		if (pkg->filename == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->filename, value);
	}
	
	/* Provides */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_PROVIDES);
	value = query_get_string(query, key);
	
	if (value != NULL) {
		if (repo->type == REPO_TYPE_APK) {
			pkg->provides = aptpkg_from_apkpkg(value);
		} else {
			pkg->provides = malloc(strlen(value) + 1);
			
			if (pkg->provides == NULL) {
				err = APTERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strcpy(pkg->provides, value);
		}
	}
	
	/* Suggests */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_SUGGESTS);
	value = query_get_string(query, key);
	
	if (value != NULL) {
		pkg->suggests = malloc(strlen(value) + 1);
		
		if (pkg->suggests == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->suggests, value);
	}
	
	/* Recommends */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_RECOMMENDS);
	value = query_get_string(query, key);
	
	if (value != NULL) {
		pkg->recommends = malloc(strlen(value) + 1);
		
		if (pkg->recommends == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->recommends, value);
	}
	
	/* Depends */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_DEPENDS);
	value = query_get_string(query, key);
	
	if (value != NULL) {
		if (repo->type == REPO_TYPE_APK) {
			pkg->depends = aptpkg_from_apkpkg(value);
		} else {
			pkg->depends = malloc(strlen(value) + 1);
			
			if (pkg->depends == NULL) {
				err = APTERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strcpy(pkg->depends, value);
		}
	}
	
	/* Breaks */
	if (repo->type == REPO_TYPE_APK && pkg->depends != NULL && strstr(pkg->depends, "!") != NULL) {
		pkg->breaks = malloc(strlen(pkg->depends) + 1);
		
		if (pkg->breaks == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		((char*) pkg->breaks)[0] = '\0';
		
		strsplit_init(&split, &part, pkg->depends, ",");
		
		while (strsplit_next(&split, &part) != NULL) {
			if (*part.begin != '!') {
				continue;
			}
			
			if (((char*) pkg->breaks)[0] != '\0') {
				strcat(pkg->breaks, ", ");
			}
			
			strncat(pkg->breaks, part.begin + 1, part.size - 1);
			
			ptr = part.begin - (2 * (part.index != 0));
			
			if (strsplit_next(&split, &part) == NULL) {
				*ptr = '\0';
				break;
			}
			
			size = strlen(part.begin) + 1;
			memmove(ptr, part.begin, size);
			
			strsplit_init(&split, &part, pkg->depends, ",");
		}
		
		if (((char*) pkg->breaks)[0] == '\0') {
			free(pkg->breaks);
			pkg->breaks = NULL;
		}
	} else {
		key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_BREAKS);
		value = query_get_string(query, key);
		
		if (value != NULL) {
			pkg->breaks = malloc(strlen(value) + 1);
			
			if (pkg->breaks == NULL) {
				err = APTERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strcpy(pkg->breaks, value);
		}
	}
	
	/* Replaces */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_REPLACES);
	value = query_get_string(query, key);
	
	if (value != NULL) {
		pkg->replaces = malloc(strlen(value) + 1);
		
		if (pkg->replaces == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->replaces, value);
	}
	
	/* Size */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_SIZE);
	pkg->size = query_get_uint(query, key);
	
	/* Installed-Size */
	key = pkg_get_field_name(repo->type, PKG_SECTION_FIELD_INSTALLED_SIZE);
	pkg->installed_size = query_get_uint(query, key);
	
	if (pkg->installed_size != 0 && repo->type == REPO_TYPE_APT) {
		pkg->installed_size = pkg->installed_size * 1000;
	}
	
	pkg->autoinstall = -1;
	pkg->removable = -1;
	
	end:;
	
	return err;
	
}

int repo_load_string(
	repo_t* const repo,
	const char* const string,
	const size_t size,
	const int cache
) {
	
	int status = 0;
	int err = 0;
	
	long int file_size = 0;
	
	fstream_t* stream = NULL;
	
	size_t index = 0;
	ssize_t rsize = 0;
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	hquery_t query = {0};
	
	pkg_t  pkg = {0};
	
	char* section = NULL;
	
	char* index_file = NULL;
	
	char* directory = NULL;
	char* temporary_directory = NULL;
	
	const char* source = string;
	
	buffer_t buffer = {0};
	
	const int format = format_guess_string(string);
	
	temporary_directory = get_local_temp_dir();
	
	if (temporary_directory == NULL) {
		err = APTERR_NO_TMPDIR;
		goto end;
	}
	
	if (set_current_directory(temporary_directory) != 0) {
		err = APTERR_FS_CHDIR_FAILURE;
		goto end;
	}
	
	index_file = malloc(
		strlen(temporary_directory) +
		strlen(PATHSEP_S) +
		strlen(KAPKINDEX) +
		strlen(KPACKAGES) +
		1
	);
	
	if (index_file == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(index_file, temporary_directory);
	strcat(index_file, PATHSEP_S);
	
	switch (repo->type) {
		case REPO_TYPE_APT: {
			strcat(index_file, "data");
			break;
		}
		case REPO_TYPE_APK: {
			strcat(index_file, KAPKINDEX);
			break;
		}
		default: {
			err = APTERR_REPO_UNKNOWN_FORMAT;
			goto end;
		}
	}
	
	if (format != GUESS_FILE_FORMAT_SOMETHING_ELSE) {
		loggln(
			LOG_VERBOSE,
			"Package index file is a compressed archive; attempting to decompress"
		);
		
		err = uncompress(string, size, NULL, NULL, NULL);
		
		if (err != 0) {
			err = APTERR_ARCHIVE_UNCOMPRESS_FAILURE;
			goto end;
		}
		
		loggln(LOG_VERBOSE, "Attempting to read package index file from '%s'", index_file);
		
		stream = fstream_open(index_file, FSTREAM_READ);
		
		if (stream == NULL) {
			err = APTERR_FSTREAM_OPEN_FAILURE;
			goto end;
		}
		
		status = fstream_seek(stream, 0, FSTREAM_SEEK_END);
		
		if (status == -1) {
			err = APTERR_FSTREAM_SEEK_FAILURE;
			goto end;
		}
		
		file_size = fstream_tell(stream);
		
		if (file_size == -1) {
			err = APTERR_FSTREAM_TELL_FAILURE;
			goto end;
		}
		
		if (file_size == 0) {
			err = APTERR_FSTREAM_READ_EMPTY_FILE;
			goto end;
		}
		
		status = fstream_seek(stream, 0, FSTREAM_SEEK_BEGIN);
		
		if (status == -1) {
			err = APTERR_FSTREAM_SEEK_FAILURE;
			goto end;
		}
		
		err = buffer_init(&buffer, (size_t) file_size + 1);
		
		if (err != 0) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		rsize = fstream_read(stream, buffer.data, (size_t) file_size);
		
		if (rsize == -1) {
			err = APTERR_FSTREAM_READ_FAILURE;
			goto end;
		}
		
		buffer.data[rsize] = '\0';
		buffer.offset = rsize;
		
		source = buffer.data;
	}
	
	section = malloc(APT_MAX_PKG_SECTION_LEN);
	
	if (section == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	section[0] = '\0';
	
	strsplit_init(&split, &part, source, "\n");
	
	while (1) {
		if (strsplit_next(&split, &part) == NULL) {
			break;
		}
		
		if (part.size == 0) {
			if (section[0] == '\0') {
				continue;
			}
			
			query_free(&query);
			query_init(&query, '\n', ":");
			
			status = query_load_string(&query, section);
			
			if (status != 0) {
				err = APTERR_PACKAGE_SECTION_INVALID;
				goto end;
			}
			
			memset(&pkg, 0, sizeof(pkg));
			
			err = pkg_parse_section(repo, &pkg, &query);
			
			if (err != APTERR_SUCCESS) {
				goto end;
			}
			
			pkg.index = index++;
			
			pkg.repo = repo->index;
			pkg.arch = repo->architecture;
			
			err = pkgs_append(&repo->pkgs, &pkg, 1);
			
			if (err != APTERR_SUCCESS) {
				goto end;
			}
			
			section[0] = '\0';
			
			continue;
		}
		
		if (*section != '\0' && pkg_key_matches(repo->type, part.begin)) {
			strcat(section, "\n");
		}
		
		strncat(section, part.begin, part.size);
	}
	
	if (cache) {
		err = repo_store_cache(repo, source, strlen(source));
	}
	
	end:;
	
	remove_directory_contents(temporary_directory);
	
	query_free(&query);
	free(section);
	free(temporary_directory);
	buffer_free(&buffer);
	
	return err;
	
}

int repo_load_file(
	repo_t* const repo,
	const char* const filename,
	const char* const base,
	const int cache
) {
	
	int err = 0;
	int status = 0;
	
	long int file_size = 0;
	ssize_t rsize = 0;
	
	fstream_t* stream = NULL;
	
	char* absolute_path = NULL;
	
	buffer_t buffer = {0};
	
	absolute_path = expand_filename(filename);
	
	if (absolute_path == NULL) {
		err = APTERR_EXPAND_FILENAME_FAILURE;
		goto end;
	}
	
	loggln(LOG_VERBOSE, "Attempt to load repository index from local file %s", absolute_path);
	
	stream = fstream_open(filename, FSTREAM_READ);
	
	if (stream == NULL) {
		err = APTERR_FSTREAM_OPEN_FAILURE;
		goto end;
	}
	
	status = fstream_seek(stream, 0, FSTREAM_SEEK_END);
	
	if (status == -1) {
		err = APTERR_FSTREAM_SEEK_FAILURE;
		goto end;
	}
	
	file_size = fstream_tell(stream);
	
	if (file_size == -1) {
		err = APTERR_FSTREAM_TELL_FAILURE;
		goto end;
	}
	
	if (file_size == 0) {
		err = APTERR_FSTREAM_READ_EMPTY_FILE;
		goto end;
	}
	
	if (file_size > (APT_MAX_PKG_INDEX_LEN - 1)) {
		err = APTERR_REPO_PKG_INDEX_TOO_LARGE;
		goto end;
	}
	
	status = fstream_seek(stream, 0, FSTREAM_SEEK_BEGIN);
	
	if (status == -1) {
		err = APTERR_FSTREAM_SEEK_FAILURE;
		goto end;
	}
	
	err = buffer_init(&buffer, (size_t) file_size + 1);
	
	if (err != 0) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	rsize = fstream_read(stream, buffer.data, (size_t) file_size);
	
	if (rsize == -1) {
		err = APTERR_FSTREAM_READ_FAILURE;
		goto end;
	}
	
	buffer.data[rsize] = '\0';
	buffer.offset = rsize;
	
	err = repo_load_string(repo, buffer.data, buffer.offset, cache);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	err = repo_set_uri(repo, BASE_URI_TYPE_LOCAL_FILE, absolute_path, base);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	end:;
	
	fstream_close(stream);
	free(absolute_path);
	buffer_free(&buffer);
	
	return err;
	
}

int repo_load_url(
	repo_t* const repo,
	const char* const url,
	const char* const base,
	const int cache
) {
	
	int err = APTERR_SUCCESS;
	CURLcode code = CURLE_OK;
	
	char* effective_url = NULL;
	
	buffer_t buffer = {0};
	
	wcurl_t* wcurl = NULL;
	CURL* curl = NULL;
	
	loggln(LOG_VERBOSE, "Attempt to load repository index from URL %s", url);
	
	wcurl = wcurl_getglobal();
	
	if (wcurl == NULL) {
		err = APTERR_WCURL_INIT_FAILURE;
		goto end;
	}
	
	curl = wcurl_getcurl(wcurl);
	
	err = buffer_init(&buffer, APT_MAX_PKG_INDEX_LEN);
	
	if (err != 0) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_URL, url);
	
	if (code != CURLE_OK) {
		err = APTERR_WCURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_string_cb);
	
	if (code != CURLE_OK) {
		err = APTERR_WCURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
	
	if (code != CURLE_OK) {
		err = APTERR_WCURL_SETOPT_FAILURE;
		goto end;
	}
	
	err = wcurl_perform(wcurl);
	
	if (err != WCURL_ERR_SUCCESS) {
		err = APTERR_WCURL_REQUEST_FAILURE;
		goto end;
	}
	
	 err = repo_load_string(repo, buffer.data, buffer.offset, cache);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	code = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effective_url);
	
	if (code != CURLE_OK) {
		err = APTERR_WCURL_GETINFO_FAILURE;
		goto end;
	}
	
	err = repo_set_uri(repo, BASE_URI_TYPE_URL, url, base);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	end:;
	
	buffer_free(&buffer);
	
	code = curl_easy_setopt(curl, CURLOPT_URL, NULL);
	
	if (code != CURLE_OK) {
		err = APTERR_WCURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	
	if (code != CURLE_OK) {
		err = APTERR_WCURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
	
	if (code != CURLE_OK) {
		err = APTERR_WCURL_SETOPT_FAILURE;
		goto end;
	}
	
	return err;
	
}

int repo_load(
	repo_t* const repo,
	const char* const something,
	const char* const base,
	const int cache
) {
	
	int err = 0;
	
	const int type = uri_guess_type(something);
	
	switch (type) {
		case GUESS_URI_TYPE_URL:
			err = repo_load_url(repo, something, base, cache);
			break;
		case GUESS_URI_TYPE_LOCAL_FILE:
			err = repo_load_file(repo, something, base, cache);
			break;
		case GUESS_URI_TYPE_SOMETHING_ELSE:
			err = repo_load_string(repo, something, 0, cache);
			break;
		default:
			err = APTERR_LOAD_UNSUPPORTED_URI;
			break;
	}
	
	return err;
	
}

int repolist_append(
	repolist_t* const list,
	const repo_t* const repo
) {
	
	size_t size = 0;
	repo_t* items = NULL;
	
	if (sizeof(*list->items) * (list->offset + 1) > list->size) {
		size = list->size + sizeof(*list->items) * (list->offset + 1);
		items = realloc(list->items, size);
		
		if (items == NULL) {
			return APTERR_MEM_ALLOC_FAILURE;
		}
		
		list->size = size;
		list->items = items;
	}
	
	list->items[list->offset++] = *repo;
	
	return APTERR_SUCCESS;
	
}

int repolist_load(repolist_t* const list) {
	
	int err = APTERR_SUCCESS;
	size_t index = 0;
	
	size_t sources = 0;
	
	CURLcode code = CURLE_OK;
	
	char* config_dir = NULL;
	char* sources_directory = NULL;
	char* pkgs_directory = NULL;
	
	char* filename = NULL;
	
	char* url = NULL;
	char* match = NULL;
	
	char* user_agent = NULL;
	const char* operating_system = NULL;
	
	const char* repository = NULL;
	const char* release = NULL;
	const char* resources = NULL;
	const char* architecture = NULL;
	int type = 0;
	
	const char* file_extension = NULL;
	const char* value = NULL;
	
	options_t* options = NULL;
	pkg_t* pkg = NULL;
	
	hquery_t query = {0};
	
	repo_t repo = {0};
	size_t repo_index = 0;
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	wcurl_t* wcurl = NULL;
	CURL* curl = NULL;
	
	walkdir_t walkdir = {0};
	const walkdir_item_t* item = NULL;
	
	operating_system = osdetect_getplatform();
	
	if (operating_system == NULL) {
		err = APTERR_PLATFORM_UNKNOWN;
		goto end;
	}
	
	wcurl = wcurl_getglobal();
	
	if (wcurl == NULL) {
		err = APTERR_WCURL_INIT_FAILURE;
		goto end;
	}
	
	curl = wcurl_getcurl(wcurl);
	
	user_agent = malloc(strlen(WCURL_USER_AGENT) + strlen(BRACKETS_START) + strlen(operating_system) + strlen(BRACKETS_END) + 1);
	
	if (user_agent == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(user_agent, WCURL_USER_AGENT);
	strcat(user_agent, BRACKETS_START);
	strcat(user_agent, operating_system);
	strcat(user_agent, BRACKETS_END);
	
	code = curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	config_dir = repo_get_config_dir();
	
	if (config_dir == NULL) {
		err = APTERR_REPO_GET_CONFDIR_FAILURE;
		goto end;
	}
	
	err = options_load(config_dir);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	pkgs_directory = repo_get_pkgs_dir();
	
	if (pkgs_directory == NULL) {
		err = APTERR_REPO_GET_PKGSDIR_FAILURE;
		goto end;
	}
	
	options = get_options();
	
	sources_directory = repo_get_src_dir();
	
	if (sources_directory == NULL) {
		err = APTERR_REPO_GET_SRCDIR_FAILURE;
		goto end;
	}
	
	logg(LOG_STANDARD, "Reading package lists...");
	
	loggln(LOG_VERBOSE, "Looking for repositories in %s", sources_directory);
	
	if (create_directory(sources_directory) != 0) {
		err = APTERR_FS_MKDIR_FAILURE;
		goto end;
	}
	
	if (walkdir_init(&walkdir, sources_directory) == -1) {
		err = APTERR_FS_WALKDIR_FAILURE;
		goto end;
	}
	
	while ((item = walkdir_next(&walkdir)) != NULL) {
		if (strcmp(item->name, ".") == 0 || strcmp(item->name, "..") == 0) {
			continue;
		}
		
		if (item->type != WALKDIR_ITEM_FILE) {
			loggln(LOG_VERBOSE, "Ignoring '%s' as it is not a file", item->name);
			continue;
		}
		
		match = strchr(item->name, '\0') - strlen(CONF_FILE_EXT);
		
		if (strlen(item->name) <= strlen(CONF_FILE_EXT) || strcmp(match, CONF_FILE_EXT) != 0) {
			loggln(LOG_VERBOSE, "Ignoring '%s' as it lacks the required file extension (%s)", item->name, CONF_FILE_EXT);
			continue;
		}
		
		sources += 1;
		
		free(filename);
		
		filename = malloc(strlen(sources_directory) + strlen(PATHSEP_S) + strlen(item->name) + 1);
		
		if (filename == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(filename, sources_directory);
		strcat(filename, PATHSEP_S);
		strcat(filename, item->name);
		
		query_free(&query);
		query_init(&query, '\n', "=");
		
		loggln(LOG_VERBOSE, "Loading repository file from %s", filename);
		
		err = query_load_file(&query, filename);
		
		if (err != 0) {
			err = APTERR_REPO_CONF_PARSE_FAILURE;
			goto end;
		}
		
		/* Repository */
		repository = query_get_string(&query, "repository");
		
		if (repository == NULL) {
			err = APTERR_REPO_CONF_MISSING_FIELD;
			goto end;
		}
		
		loggln(LOG_VERBOSE, "Read repository property (repository = %s)", repository);
		
		/* Release */
		release = query_get_string(&query, "release");
		
		if (release == NULL) {
			err = APTERR_REPO_CONF_MISSING_FIELD;
			goto end;
		}
		
		loggln(LOG_VERBOSE, "Read repository property (release = %s)", release);
		
		/* Resource */
		resources = query_get_string(&query, "resource");
		
		if (resources == NULL) {
			err = APTERR_REPO_CONF_MISSING_FIELD;
			goto end;
		}
		
		loggln(LOG_VERBOSE, "Read repository property (resource = %s)", resources);
		
		/* Architecture */
		architecture = query_get_string(&query, "architecture");
		
		if (architecture == NULL) {
			err = APTERR_REPO_CONF_MISSING_FIELD;
			goto end;
		}
		
		loggln(LOG_VERBOSE, "Read repository property (architecture = %s)", architecture);
		
		/* Format */
		value = query_get_string(&query, "format");
		
		if (value == NULL) {
			err = APTERR_REPO_CONF_MISSING_FIELD;
			goto end;
		}
		
		loggln(LOG_VERBOSE, "Read repository property (type = %s)", value);
		
		type = repotype_unstringify(value);
		
		if (type == REPO_TYPE_UNKNOWN) {
			err = APTERR_REPO_UNKNOWN_FORMAT;
			goto end;
		}
		
		loggln(LOG_VERBOSE, "Repository format '%s' matched as value '%i'", value, type);
		
		strsplit_init(&split, &part, resources, " ");
		
		while (strsplit_next(&split, &part) != NULL) {
			if (part.size == 0) {
				continue;
			}
			
			memset(&repo, 0, sizeof(repo));
			
			repo.type = type;
			
			repo.architecture = get_architecture(architecture);
			
			if (repo.architecture == ARCH_UNKNOWN) {
				err = APTERR_REPO_UNKNOWN_ARCHITECTURE;
				goto end;
			}
			
			repo.name = malloc(
				strlen(release) + strlen(KHYPHEN) +
				part.size + strlen(KHYPHEN) +
				strlen(architecture) + 1
			);
			
			if (repo.name == NULL) {
				err = APTERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strcpy(repo.name, release);
			strcat(repo.name, KHYPHEN);
			strncat(repo.name, part.begin, part.size);
			strcat(repo.name, KHYPHEN);
			strcat(repo.name, architecture);
			
			repo.resource = malloc(part.size + 1);
			
			if (repo.resource == NULL) {
				err = APTERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strncpy(repo.resource, part.begin, part.size);
			repo.resource[part.size] = '\0';
			
			repo.release = malloc(strlen(release) + 1);
			
			if (repo.release == NULL) {
				err = APTERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strcpy(repo.release, release);
			
			repo.platform = malloc(strlen(architecture) + 1);
			
			if (repo.platform == NULL) {
				err = APTERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strcpy(repo.platform, architecture);
			
			free(url);
			
			if (!options->force_refresh) {
				url = repo_fetch_cache(&repo);
				
				if (url != NULL) {
					err = repo_load(&repo, url, repository, 0);
					
					if (err != APTERR_SUCCESS) {
						goto end;
					}
					
					err = repolist_append(list, &repo);
					
					if (err != APTERR_SUCCESS) {
						goto end;
					}
					
					continue;
				}
			}
			
			url = malloc(
				strlen(repository) +
				strlen(PATHSEP_POSIX_S) +
				strlen(KDISTS) +
				strlen(PATHSEP_POSIX_S) +
				strlen(release) +
				strlen(PATHSEP_POSIX_S) +
				part.size + 
				strlen(PATHSEP_POSIX_S) +
				strlen(KBINARY) + strlen(architecture) +
				strlen(PATHSEP_POSIX_S) +
				strlen(KPACKAGES) +
				strlen(TAR_FILE_EXT) +
				1 + 3 + 1
			);
			
			if (url == NULL) {
				err = APTERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strcpy(url, repository);
			strcat(url, PATHSEP_POSIX_S);
			
			if (repo.type == REPO_TYPE_APT) {
				strcat(url, KDISTS);
				strcat(url, PATHSEP_POSIX_S);
			}
			
			strcat(url, release);
			strcat(url, PATHSEP_POSIX_S);
			strncat(url, part.begin, part.size);
			strcat(url, PATHSEP_POSIX_S);
			
			if (repo.type == REPO_TYPE_APT) {
				strcat(url, KBINARY);
			}
			
			strcat(url, architecture);
			strcat(url, PATHSEP_POSIX_S);
			
			switch (repo.type) {
				case REPO_TYPE_APT: {
					strcat(url, KPACKAGES);
					break;
				}
				case REPO_TYPE_APK: {
					strcat(url, KAPKINDEX);
					strcat(url, TAR_FILE_EXT);
					break;
				}
				default: {
					err = APTERR_REPO_UNKNOWN_FORMAT;
					goto end;
				}
			}
			
			match = strchr(url, '\0');
			
			for (index = 0; index < sizeof(PACKAGES_FILE_EXT) / sizeof(*PACKAGES_FILE_EXT); index++) {
				file_extension = PACKAGES_FILE_EXT[index];
				strcpy(match, file_extension);
				
				repo.index = repo_index;
				err = repo_load(&repo, url, repository, options->cache);
				
				if (err == APTERR_WCURL_REQUEST_FAILURE) {
					continue;
				}
				
				if (err != APTERR_SUCCESS) {
					goto end;
				}
				
				break;
			}
			
			err = repolist_append(list, &repo);
			
			if (err != APTERR_SUCCESS) {
				goto end;
			}
			
			repo_index++;
		}
	}
	
	if (sources == 0) {
		erase_line();
		loggln(LOG_ERROR, "No repository sources configured in %s", sources_directory);
		
		err = APTERR_REPO_LOAD_NO_SOURCES_AVAILABLE;
		goto end;
	}
	
	loggln(LOG_INFO, "Loaded repository configuration from %zu source files", sources);
	
	walkdir_free(&walkdir);
	
	if (walkdir_init(&walkdir, pkgs_directory) == -1) {
		err = APTERR_FS_WALKDIR_FAILURE;
		goto end;
	}
	
	while ((item = walkdir_next(&walkdir)) != NULL) {
		if (strcmp(item->name, ".") == 0 || strcmp(item->name, "..") == 0) {
			continue;
		}
		
		pkg = repolist_get_pkg(list, item->name);
		
		if (pkg == NULL) {
			continue;
		}
		
		pkg->installed = 1;
		
		err = pkgs_append(&list->installed, pkg, 0);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	loggln(LOG_STANDARD, " Done");
	
	end:;
	
	query_free(&query);
	
	free(url);
	free(config_dir);
	free(filename);
	free(sources_directory);
	free(pkgs_directory);
	free(user_agent);
	
	walkdir_free(&walkdir);
	
	if (err != APTERR_SUCCESS) {
		repolist_free(list);
	}
	
	return err;
	
}

int repolist_destroy(repolist_t* const list) {
	
	int err = APTERR_SUCCESS;
	int answer = ASK_ANSWER_YES;
	
	options_t* options = NULL;
	
	char* pkgs_directory = NULL;
	
	pkgs_directory = repo_get_pkgs_dir();
	
	if (pkgs_directory == NULL) {
		err = APTERR_REPO_GET_PKGSDIR_FAILURE;
		goto end;
	}
	
	options = get_options();
	
	loggln(
		LOG_STANDARD,
		"All files under the following directories will be PERMANENTLY ERASED:\n  %s\n  %s",
		pkgs_directory,
		options->prefix
	);
	
	answer = ask();
	
	if (answer != ASK_ANSWER_YES) {
		err = APTERR_CLI_USER_INTERRUPTED;
		goto end;
	}
	
	remove_directory_contents(pkgs_directory);
	remove_directory_contents(options->prefix);
	
	pkgs_free(&list->installed, 0);
	
	end:;
	
	free(pkgs_directory);
	
	return err;
	
}

pkg_t* pkgs_get_virt_pkg(
	pkgs_t* const pkgs,
	const char* const name
) {
	
	size_t index = 0;
	size_t size = 0;
	
	pkg_t* pkg = NULL;
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	int matches = 0;
	
	const char* end = NULL;
	
	size = strlen(name);
	
	for (index = 0; index < pkgs->offset; index++) {
		pkg = pkgs->items[index];
		
		strsplit_init(&split, &part, pkg->provides, ",");
		
		while (pkglist_split_next(&split, &part) != NULL) {
			matches = (strncmp(name, part.begin, size) == 0 && size == part.size);
			
			if (!matches) {
				continue;
			}
			
			return pkg;
		}
		
		strsplit_init(&split, &part, pkg->replaces, ",");
		
		while (pkglist_split_next(&split, &part) != NULL) {
			matches = (strncmp(name, part.begin, size) == 0 && size == part.size);
			
			if (!matches) {
				continue;
			}
			
			return pkg;
		}
	}
	
	return NULL;
	
}
		

pkg_t* pkgs_get_pkg(
	pkgs_t* const pkgs,
	const char* const name
) {
	/*
	Get the package by name.

	This searches for it in the current package list.
	*/
	
	size_t index = 0;
	
	pkg_t* pkg = NULL;
	
	for (index = 0; index < pkgs->offset; index++) {
		pkg = pkgs->items[index];
		
		if (strcmp(pkg->name, name) != 0) {
			continue;
		}
		
		 return pkg;
	}
	
	pkg = pkgs_get_virt_pkg(pkgs, name);
	
	if (pkg != NULL) {
		loggln(
			LOG_VERBOSE,
			"Dependency on virtual package '%s' will be satisfied by '%s'",
			name,
			pkg->name
		);
	}
	
	return pkg;
	
}

pkg_t* repolist_get_pkg(
	const repolist_t* const list,
	const char* const name
) {
	/*
	Get the package by name.
	
	This searches for it in all the loaded repositories.
	*/
	
	size_t index = 0;
	
	repo_t* repo = NULL;
	pkg_t* pkg = NULL;
	
	for (index = 0; index < list->offset; index++) {
		repo = &list->items[index];
		
		pkg = pkgs_get_pkg(&repo->pkgs, name);
		
		if (pkg != NULL) {
			break;
		}
	}
	
	return pkg;
	
}

repo_t* repolist_get_pkg_repo(
	const repolist_t* const list,
	const pkg_t* const pkg
) {
	/*
	Get the repository to where this package belongs to.
	*/
	
	size_t index = 0;
	
	repo_t* repo = NULL;
	pkgs_t* pkgs = NULL;
	pkg_t* subpkg = NULL;
	
	for (index = 0; index < list->offset; index++) {
		repo = &list->items[index];
		pkgs = &repo->pkgs;
		
		if (pkg->index > (pkgs->offset - 1)) {
			continue;
		}
		
		subpkg = pkgs->items[pkg->index];
		
		if (pkg != subpkg) {
			continue;
		}
		
		return repo;
	}
	
	return NULL;
	
}

ssize_t repolist_search_pkg(
	const repolist_t* const list,
	const char* const query,
	const pkgs_paging_t paging,
	pkgs_t* const results
) {
	/*
	Search all repositories for packages matching the given query.
	
	Returns the number of matched results, or -1 on error.
	*/
	
	int matches = 0;
	
	size_t index = 0;
	size_t subindex = 0;
	
	ssize_t offset = 0;
	
	const size_t skip = (paging.maximum * paging.position);
	
	size_t current = 0;
	
	repo_t* repo = NULL;
	pkg_t* pkg = NULL;
	
	for (index = 0; index < list->offset; index++) {
		repo = &list->items[index];
		
		for (subindex = 0; subindex < repo->pkgs.offset; subindex++) {
			pkg = repo->pkgs.items[subindex];
			
			matches = (query == NULL);
			
			if (!matches) {
				matches = strstr(pkg->name, query) != NULL;
			}
			
			if (!matches) {
				matches = pkg->provides != NULL && strstr(pkg->provides, query) != NULL;
			}
			
			if (!matches) {
				matches = pkg->replaces != NULL && strstr(pkg->replaces, query) != NULL;
			}
			
			if (!matches) {
				continue;
			}
			
			current++;
			
			if (skip != 0 && current <= skip) {
				continue;
			}
			
			if (pkgs_append(results, pkg, 0) != APTERR_SUCCESS) {
				return -1;
			}
			
			offset++;
			
			if (((size_t) offset) >= paging.maximum) {
				return offset;
			}
		}
	}
	
	return offset;
	
}

int repolist_resolve_maintainers(
	const repolist_t* const list,
	pkg_t* const pkg
) {
	
	int err = APTERR_SUCCESS;
	
	maintainers_t maintainers = {0};
	
	(void) list;
	
	err = maintainers_parse(&maintainers, pkg->maintainer);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	pkg->maintainer = malloc(sizeof(maintainers));
	
	if (pkg->maintainer == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	memcpy(pkg->maintainer, &maintainers, sizeof(maintainers));
	
	end:;
	
	if (err != APTERR_SUCCESS) {
		maintainers_free(&maintainers);
	}
	
	return err;
	
}

int repolist_resolve_related(
	const repolist_t* const list,
	pkg_t* const pkg,
	const int action
) {
	
	int err = APTERR_SUCCESS;
	
	const int ignore_unsatisfied = (action != REPOLIST_RESOLVE_DEPENDS);
	
	char* name = NULL;
	char* value = NULL;
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	pkg_t* item = NULL;
	
	pkgs_t pkgs = {0};
	
	pkgs_t* items = NULL;
	
	void** destination = NULL;
	
	switch (action) {
		case REPOLIST_RESOLVE_DEPENDS:
			value = pkg->depends;
			destination = &pkg->depends;
			break;
		case REPOLIST_RESOLVE_BREAKS:
			value = pkg->breaks;
			destination = &pkg->breaks;
			break;
		case REPOLIST_RESOLVE_REPLACES:
			value = pkg->replaces;
			destination = &pkg->replaces;
			break;
		case REPOLIST_RESOLVE_SUGGESTS:
			value = pkg->suggests;
			destination = &pkg->suggests;
			break;
		case REPOLIST_RESOLVE_RECOMMENDS:
			value = pkg->recommends;
			destination = &pkg->recommends;
			break;
	}
	
	strsplit_init(&split, &part, value, ",");
	
	while (1) {
		if (pkglist_split_next(&split, &part) == NULL) {
			break;
		}
		
		free(name);
		
		name = malloc(part.size + 1);
		
		if (name == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strncpy(name, part.begin, part.size);
		name[part.size] = '\0';
		
		item = repolist_get_pkg(list, name);
		
		if (item == pkg) {
			if (action != REPOLIST_RESOLVE_DEPENDS) {
				continue;
			}
			
			err = APTERR_PACKAGE_DEPENDENCY_LOOP;
			goto end;
		}
		
		if (item == NULL) {
			if (ignore_unsatisfied) {
				continue;
			}
			
			loggln(
				LOG_ERROR,
				"Dependency on package '%s' cannot be satisfied; either it does not exist, is obsolete, or is no longer available",
				name
			);
			
			err = APTERR_PACKAGE_UNSATISFIED_DEPENDENCY;
			goto end;
		}
		
		if (pkgs_exists(&pkgs, item)) {
			continue;
		}
		
		err = pkgs_append(&pkgs, item, 0);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	free(*destination);
	*destination = NULL;
	
	if (pkgs.offset < 1) {
		goto end;
	}
	
	items = malloc(sizeof(pkgs));
	
	if (items == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	memcpy(items, &pkgs, sizeof(pkgs));
	
	*destination = items;
	
	end:;
	
	if (err != APTERR_SUCCESS) {
		pkgs_free(&pkgs, 0);
	}
	
	free(name);
	
	return err;
	
}

int repolist_fix_loops(
	const repolist_t* const list,
	pkg_t* const pkg
) {
	
	int err = APTERR_SUCCESS;
	
	pkg_t* dependency = NULL;
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	strsplit_t subsplit = {0};
	strsplit_part_t subpart = {0};
	
	char* start = NULL;
	size_t size = 0;
	
	char* name = NULL;
	
	if (pkg->resolved) {
		return 0;
	}
	
	if (pkg->depends == NULL) {
		return 0;
	}

	strsplit_init(&split, &part, pkg->depends, ",");
	
	while (1) {
		if (pkglist_split_next(&split, &part) == NULL) {
			break;
		}
		
		free(name);
		
		name = malloc(part.size + 1);
		
		if (name == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strncpy(name, part.begin, part.size);
		name[part.size] = '\0';
		
		dependency = repolist_get_pkg(list, name);
		
		if (dependency == NULL) {
			continue;
		}
		
		if (strcmp(dependency->name, pkg->name) == 0) {
			start = (char*) part.begin;
			err = APTERR_PACKAGE_DEPENDENCY_LOOP;
			
			if (pkglist_split_next(&split, &part) == NULL) {
				*start = '\0';
				
				if (start == pkg->depends) {
					free(pkg->depends);
					pkg->depends = NULL;
				}
				
				goto end;
			}
			
			size = strlen(part.begin) + 1;
			memmove(start, part.begin, size);
			
			goto end;
		}
		
		if (dependency->resolved || dependency->depends == NULL) {
			continue;
		}
		
		strsplit_init(&subsplit, &subpart, dependency->depends, ",");
		
		while (1) {
			if (pkglist_split_next(&subsplit, &subpart) == NULL) {
				break;
			}
			
			free(name);
			
			name = malloc(subpart.size + 1);
			
			if (name == NULL) {
				err = APTERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strncpy(name, subpart.begin, subpart.size);
			name[subpart.size] = '\0';
		
			if (strcmp(name, pkg->name) != 0) {
				continue;
			}
			
			err = APTERR_PACKAGE_DEPENDENCY_LOOP;
			
			start = (char*) subpart.begin;
			
			if (pkglist_split_next(&subsplit, &subpart) == NULL) {
				*start = '\0';
				
				if (start == dependency->depends) {
					free(dependency->depends);
					dependency->depends = NULL;
				}
				
				goto end;
			}
			
			size = strlen(subpart.begin) + 1;
			memmove(start, subpart.begin, size);
			
			goto end;
		}
	}
	
	end:;
	
	free(name);
	
	if (err == APTERR_PACKAGE_DEPENDENCY_LOOP) {
		loggln(
			LOG_VERBOSE,
			"Package '%s' depends on '%s', but '%s' also depends on '%s' (dependency loop)",
			pkg->name,
			dependency->name,
			dependency->name,
			pkg->name
		);
	}
				
	
	return err;
	
}

int repolist_resolve_deps(
	repolist_t* const list,
	pkg_t* const pkg
) {
	
	int err = APTERR_SUCCESS;
	pkgs_iter_t iter = {0};
	
	pkgs_t* dependencies = NULL;
	pkg_t* dependency = NULL;
	
	repo_t* repo = NULL;
	base_uri_t* base_uri = NULL;
	
	installation_t* installation = NULL;
	hquery_t* query = NULL;
	
	const char* value = NULL;
	char* uri = NULL;
	
	installation = &pkg->installation;
	query = &installation->metadata;
	
	loggln(
		LOG_INFO,
		"Resolving dependencies from package '%s'",
		pkg->name
	);
	
	if (pkg->resolved) {
		goto end;
	}
	
	err = APTERR_PACKAGE_DEPENDENCY_LOOP;
	
	while (err == APTERR_PACKAGE_DEPENDENCY_LOOP) {
		err = repolist_fix_loops(list, pkg);
	}
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	repo = repolist_get_pkg_repo(list, pkg);
	base_uri = repo_get_uri(repo);
	
	err = uri_resolve(base_uri, pkg->filename, &uri);
	
	if (err != BASEURI_ERR_SUCCESS) {
		err = APTERR_PKG_RESOLVE_URI_FAILURE;
		goto end;
	}
	
	free(pkg->filename);
	pkg->filename = uri;
	
	installation->filename = pkg_get_installation(pkg);
	
	if (installation->filename == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	pkg->installed = pkgs_exists(&list->installed, pkg);
	
	if (pkg->installed) {
		query_init(query, '\n', ":");
		
		err = query_load_file(query, installation->filename);
		
		if (err != 0) {
			err = APTERR_PACKAGE_SECTION_INVALID;
			goto end;
		}
		
		value = query_get_string(query, "Version");
		
		if (value == NULL) {
			err = APTERR_REPO_CONF_MISSING_FIELD;
			goto end;
		}
		
		pkg->upgradable = strcmp(pkg->version, value) != 0;
		pkg->autoinstall = query_get_bool(query, "Auto-Install");
	}
	
	loggln(
		LOG_VERBOSE,
		"State info for '%s': (installed = %i, upgradable = %i, autoinstall = %i, metadata = '%s')",
		pkg->name,
		pkg->installed,
		pkg->upgradable,
		pkg->autoinstall,
		installation->filename
	);
	
	if (pkg->upgradable) {
		loggln(
			LOG_VERBOSE,
			"Version info for '%s': (old = '%s', new = '%s')",
			pkg->name,
			value,
			pkg->version
		);
	}
	
	if (pkg->breaks != NULL) {
		err = repolist_resolve_related(list, pkg, REPOLIST_RESOLVE_BREAKS);
	
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	if (pkg->suggests != NULL) {
		err = repolist_resolve_related(list, pkg, REPOLIST_RESOLVE_SUGGESTS);
	
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	if (pkg->recommends != NULL) {
		err = repolist_resolve_related(list, pkg, REPOLIST_RESOLVE_RECOMMENDS);
	
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	if (pkg->replaces != NULL) {
		err = repolist_resolve_related(list, pkg, REPOLIST_RESOLVE_REPLACES);
	
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	if (pkg->maintainer != NULL) {
		err = repolist_resolve_maintainers(list, pkg);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	pkg->resolved = 1;
	
	if (pkg->depends == NULL) {
		goto end;
	}
	
	err = repolist_resolve_related(list, pkg, REPOLIST_RESOLVE_DEPENDS);
	
	if (err == APTERR_PACKAGE_UNSATISFIED_DEPENDENCY) {
		loggln(
			LOG_ERROR,
			"Package '%s' has unsatisfied dependencies; marking it obsolete",
			pkg->name
		);
		
		pkg->obsolete = 1;
		pkg->depends = NULL;
		
		goto end;
	}
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	dependencies = pkg->depends;
	
	pkgsiter_init(&iter, dependencies);
	
	while ((dependency = pkgsiter_next(&iter)) != NULL) {
		if (dependency->resolved) {
			continue;
		}
		
		err = repolist_resolve_deps(list, dependency);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
		
		loggln(
			LOG_VERBOSE,
			"Resolved dependency '%s' from package '%s'",
			dependency->name,
			pkg->name
		);
	}
	
	loggln(
		LOG_INFO,
		"Package '%s' resolved to %zu direct dependencies",
		pkg->name,
		dependencies->offset
	);
	
	end:;
	
	return err;
	
}

int pkgs_collect(
	pkgs_t* const pkgs,
	pkg_t* const pkg
) {
	
	int err = APTERR_SUCCESS;
	
	size_t index = 0;
	
	pkg_t* subpkg = NULL;
	const pkgs_t* const depends = pkg->depends;
	
	if (pkgs_exists(pkgs, pkg)) {
		goto end;
	}
	
	err = pkgs_append(pkgs, pkg, 0);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	if (depends == NULL) {
		goto end;
	}
	
	for (index = 0; index < depends->offset; index++) {
		subpkg = depends->items[index];
		
		err = pkgs_collect(pkgs, subpkg);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	end:;
	
	return err;
	
}

int repolist_get_dependants(
	repolist_t* const list,
	const pkg_t* const dependency,
	pkgs_t* const dependants
) {
	
	int err = 0;
	
	size_t index = 0;
	
	repo_t* repo = NULL;
	pkg_t* pkg = NULL;
	pkg_t* subpkg = NULL;
	
	int matches = 0;
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	pkgs_iter_t iter = {0};
	pkgs_iter_t subiter = {0};
	
	const size_t size = strlen(dependency->name) + 1;
	
	loggln(LOG_VERBOSE, "Fetching dependants on dependency '%s'", dependency->name);
	
	for (index = 0; index < list->offset; index++) {
		repo = &list->items[index];
		
		pkgsiter_init(&iter, &repo->pkgs);
	
		while ((pkg = pkgsiter_next(&iter)) != NULL) {
			if (pkg->depends == NULL) {
				continue;
			}
			
			if (!pkgs_exists(&list->installed, pkg)) {
				continue;
			}
			
			matches = 0;
			
			if (pkg->resolved) {
				pkgsiter_init(&subiter, pkg->depends);
				
				while ((subpkg = pkgsiter_next(&subiter)) != NULL) {
					matches = strcmp(subpkg->name, dependency->name) == 0;
					
					if (matches) {
						break;
					}
				}
			} else {
				strsplit_init(&split, &part, pkg->depends, ",");
				
				while (pkglist_split_next(&split, &part) != NULL) {
					matches = strncmp(dependency->name, part.begin, size) == 0;
					
					if (matches) {
						break;
					}
				}
			}
			
			if (!matches) {
				continue;
			}
			
			err = pkgs_append(dependants, pkg, 0);
			
			if (err != APTERR_SUCCESS) {
				goto end;
			}
		}
	}
	
	loggln(LOG_VERBOSE, "The package '%s' has %zu installed dependants", dependency->name, dependants->offset);
	
	end:;
	
	return err;
	
}

int repolist_fetch_packages(
	repolist_t* const list,
	char* const* const packages,
	pkgs_t * const direct,
	pkgs_t * const indirect
) {
	
	int err = 0;
	
	size_t package_index = 0;
	
	const char* name = NULL;
	
	pkg_t* pkg = NULL;
	pkgs_t pkgs = {0};
	
	pkgs_t queue = {0};
	
	while (1) {
		name = packages[package_index++];
		
		if (name == NULL) {
			break;
		}
		
		pkg = repolist_get_pkg(list, name);
		
		if (pkg == NULL) {
			loggln(
				LOG_WARN,
				"Package '%s' does not exist; ignoring",
				name
			);
			
			continue;
		}
		
		err = pkgs_append(direct, pkg, 0);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
		
		err = repolist_resolve_deps(list, pkg);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
		
		pkgs_free(&pkgs, 0);
		
		err = pkgs_collect(indirect, pkg);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	end:;
	
	pkgs_free(&queue, 0);
	pkgs_free(&pkgs, 0);
	
	return err;
	
}

int repolist_remove_package(
	repolist_t* const list,
	char* const* const packages
) {
	
	int err = 0;
	
	int answer = ASK_ANSWER_YES;
	
	biguint_t freed_disk_space = 0;
	
	pkgs_iter_t iter = {0};
	pkgs_iter_t subiter = {0};
	
	pkgs_t direct = {0};
	pkgs_t indirect = {0};
	pkgs_t dependants = {0};
	pkgs_t dependencies = {0};
	pkgs_t removables = {0};
	
	pkg_t* pkg = NULL;
	pkg_t* subpkg = NULL;
	
	options_t* options = NULL;
	
	char format[BTOS_MAX_SIZE];
	
	options = get_options();
	
	err = repolist_fetch_packages(list, packages, &direct, &indirect);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	pkgs_free(&indirect, 0);
	
	pkgsiter_init(&iter, &direct);
	
	while ((pkg = pkgsiter_next(&iter)) != NULL) {
		if (!pkg->installed) {
			loggln(LOG_WARN, "Package '%s' is not installed; ignoring", pkg->name);
			pkgs_delete(&direct, pkg);
			
			continue;
		}
		
		err = repolist_get_dependants(list, pkg, &direct);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	pkgsiter_init(&iter, &direct);

	while ((pkg = pkgsiter_next(&iter)) != NULL) {
		err = repolist_resolve_deps(list, pkg);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
		
		err = pkgs_collect(&indirect, pkg);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	pkgsiter_init(&iter, &indirect);
	
	while ((pkg = pkgsiter_next(&iter)) != NULL) {
		if (pkg->removable != -1) {
			continue;
		}
		
		pkgs_free(&dependants, 0);
		
		err = repolist_get_dependants(list, pkg, &dependants);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
		
		pkg->removable = 1;
		
		if (dependants.offset < 1) {
			continue;
		}
		
		pkg->removable = 1;
		
		pkgsiter_init(&subiter, &dependants);
		
		while ((subpkg = pkgsiter_next(&subiter)) != NULL) {
			pkg->removable = !subpkg->installed || pkgs_exists(&indirect, subpkg);
			
			loggln(LOG_VERBOSE, "Package '%s' (%savailable) depends on '%s'", subpkg->name, (pkg->removable) ? "not ": "", pkg->name);
			
			if (!pkg->removable) {
				break;
			}
		}
		
		if (pkg->removable) {
			continue;
		}
		
		err = repolist_resolve_deps(list, pkg);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
		
		err = pkgs_collect(&dependencies, pkg);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
		
		pkgsiter_init(&subiter, &dependencies);
		
		while ((subpkg = pkgsiter_next(&subiter)) != NULL) {
			if (!pkgs_exists(&indirect, subpkg)) {
				continue;
			}
			
			subpkg->removable = 0;
		}
	}
	
	pkgsiter_init(&iter, &indirect);
	
	while ((pkg = pkgsiter_next(&iter)) != NULL) {
		if (!pkg->removable) {
			continue;
		}
		
		err = pkgs_append(&removables, pkg, 0);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
		
		freed_disk_space += pkg->installed_size;
	}
	
	if (removables.offset == 0) {
		goto end;
	}
	
	loggln(LOG_STANDARD, "The following packages will be REMOVED:");
	pprint_packages(&removables);
	
	loggln(LOG_STANDARD, "%zu upgraded, %zu newly installed, %zu to remove and %zu not upgraded.", 0, 0, removables.offset, list->installed.offset - removables.offset);
	
	btos(freed_disk_space, format);
	loggln(LOG_STANDARD, "After this operation, %s disk space will be freed.", format);
	
	if (!options->assume_yes) {
		answer = ask();
	}
	
	if (answer != ASK_ANSWER_YES) {
		err = APTERR_CLI_USER_INTERRUPTED;
		goto end;
	}
	
	pkgsiter_init(&iter, &removables);
	
	while ((pkg = pkgsiter_next(&iter)) != NULL) {
		err = repolist_remove_single_package(list, pkg);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	end:;
	
	pkgs_free(&direct, 0);
	pkgs_free(&indirect, 0);
	pkgs_free(&dependants, 0);
	pkgs_free(&dependencies, 0);
	pkgs_free(&removables, 0);
	
	return err;
	
}

int repolist_install_package(
	repolist_t* const list,
	char* const* const packages
) {
	
	int err = 0;
	
	int answer = ASK_ANSWER_YES;
	int upgrade_or_install = 0;
	
	options_t* options = NULL;
	
	pkg_t* pkg = NULL;
	pkg_t* subpkg = NULL;
	
	pkgs_t direct = {0};
	pkgs_t indirect = {0};
	
	pkgs_t suggests = {0};
	pkgs_t recommends = {0};
	pkgs_t additional = {0};
	pkgs_t installs = {0};
	pkgs_t upgrades = {0};
	pkgs_t non_upgradable = {0};
	
	size_t install = 0;
	size_t upgrade = 0;
	
	biguint_t required_disk_space = 0;
	biguint_t download_size = 0;
	
	pkgs_iter_t iter = {0};
	pkgs_iter_t subiter = {0};
	
	downloader_t downloader = {0};
	dlopts_t dlopts = {0};
	
	char format[BTOS_MAX_SIZE];
	
	options = get_options();
	
	err = repolist_fetch_packages(list, packages, &direct, &indirect);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	pkgsiter_init(&iter, &indirect);
	
	while ((pkg = pkgsiter_next(&iter)) != NULL) {
		install += !pkg->installed;
		upgrade += pkg->upgradable;
		
		if (pkg->autoinstall == -1) {
			pkg->autoinstall = !pkgs_exists(&direct, pkg);
		}
		
		if (pkg->installed && !pkg->upgradable && pkgs_exists(&direct, pkg)) {
			loggln(LOG_STANDARD, "%s is already the newest version (%s).", pkg->name, pkg->version);
		}
		
		if (!pkgs_exists(&direct, pkg) && !pkg->installed) {
			err = pkgs_append(&additional, pkg, 0);
			
			if (err != APTERR_SUCCESS) {
				goto end;
			}
		}
		
		if (!pkg->installed) {
			err = pkgs_append(&installs, pkg, 0);
			
			if (err != APTERR_SUCCESS) {
				goto end;
			}
		}
		
		if (pkg->upgradable) {
			err = pkgs_append(&upgrades, pkg, 0);
			
			if (err != APTERR_SUCCESS) {
				goto end;
			}
		}
		
		if (!(pkg->upgradable || !pkg->installed)) {
			continue;
		}
		
		required_disk_space += pkg->installed_size;
		download_size += pkg->size;
	}
	
	upgrade_or_install = (upgrade || install);
	
	pkgsiter_init(&iter, &direct);
	
	while ((pkg = pkgsiter_next(&iter)) != NULL) {
		pkgsiter_init(&subiter, pkg->suggests);
		
		while ((subpkg = pkgsiter_next(&subiter)) != NULL) {
			if (pkgs_exists(&indirect, subpkg)) {
				continue;
			}
			
			err = pkgs_append(&suggests, subpkg, 0);
			
			if (err != APTERR_SUCCESS) {
				goto end;
			}
		}
		
		pkgsiter_init(&subiter, pkg->recommends);
		
		while ((subpkg = pkgsiter_next(&subiter)) != NULL) {
			if (pkgs_exists(&indirect, subpkg)) {
				continue;
			}
			
			err = pkgs_append(&recommends, subpkg, 0);
			
			if (err != APTERR_SUCCESS) {
				goto end;
			}
		}
	}
	
	pkgsiter_init(&iter, &list->installed);
	
	while ((pkg = pkgsiter_next(&iter)) != NULL) {
		if (pkg->upgradable) {
			continue;
		}
		
		err = pkgs_append(&non_upgradable, pkg, 0);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	if (additional.offset != 0) {
		loggln(LOG_STANDARD, "The following additional packages will be installed:");
		pprint_packages(&additional);
	}
	
	if (upgrade_or_install && suggests.offset != 0) {
		loggln(LOG_STANDARD, "Suggested packages:");
		pprint_packages(&suggests);
	}
		
	if (upgrade_or_install && recommends.offset != 0) {
		loggln(LOG_STANDARD, "Recommended packages:");
		pprint_packages(&recommends);
	}
	
	if (installs.offset != 0) {
		loggln(LOG_STANDARD, "The following NEW packages will be installed:");
		pprint_packages(&installs);
	}
	
	if (upgrades.offset != 0) {
		loggln(LOG_STANDARD, "The following packages will be upgraded:");
		pprint_packages(&upgrades);
	}
	
	loggln(LOG_STANDARD, "%zu upgraded, %zu newly installed, %zu to remove and %zu not upgraded.", upgrades.offset, installs.offset, 0, non_upgradable.offset);
	
	if (upgrade_or_install) {
		btos(download_size, format);
		loggln(LOG_STANDARD, "Need to get %s of archives.", format);
		
		btos(required_disk_space, format);
		loggln(LOG_STANDARD, "After this operation, %s of additional disk space will be used.", format);
	}
	
	if (!upgrade_or_install) {
		goto end;
	}
	
	if (!options->assume_yes) {
		answer = ask();
	}
	
	if (answer != ASK_ANSWER_YES) {
		err = APTERR_CLI_USER_INTERRUPTED;
		goto end;
	}
	
	dlopts.concurrency = options->concurrency;
	dlopts.temporary_directory = get_local_temp_dir();
	dlopts.progress_callback = download_progress_callback;
	dlopts.retry = 8;
	
	if (dlopts.temporary_directory == NULL) {
		err = APTERR_NO_TMPDIR;
		goto end;
	}
	
	pkgsiter_init(&iter, &indirect);
	
	while ((pkg = pkgsiter_next(&iter)) != NULL) {
		err = downloader_add(&downloader, &dlopts, pkg);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	err = downloader_wait(&downloader, &dlopts);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	pkgsiter_init(&iter, &indirect);
	
	while ((pkg = pkgsiter_next(&iter)) != NULL) {
		err = repolist_install_single_package(list, pkg);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	end:;
	
	pkgs_free(&direct, 0);
	pkgs_free(&indirect, 0);
	pkgs_free(&suggests, 0);
	pkgs_free(&recommends, 0);
	pkgs_free(&additional, 0);
	pkgs_free(&installs, 0);
	pkgs_free(&upgrades, 0);
	pkgs_free(&non_upgradable, 0);
	
	downloader_free(&downloader);
	dlopts_free(&dlopts);
	
	return err;
	
}

char* find_file(const char* const directory, const char* const name) {
	
	size_t index = 0;
	const char* file_extension = NULL;
	
	char* filename = NULL;
	char* match = NULL;
	
	filename = malloc(
		strlen(directory) +
		strlen(PATHSEP_S) +
		strlen(name) +
		strlen(TAR_FILE_EXT) +
		+ 16
	);
	
	if (filename == NULL) {
		return NULL;
	}
	
	strcpy(filename, directory);
	strcat(filename, PATHSEP_S);
	strcat(filename, name);
	strcat(filename, TAR_FILE_EXT);
	
	match = strchr(filename, '\0');
	
	for (index = 0; index < sizeof(PACKAGES_FILE_EXT) / sizeof(*PACKAGES_FILE_EXT); index++) {
		file_extension = PACKAGES_FILE_EXT[index];
		strcpy(match, file_extension);
		
		if (file_exists(filename) != 1) {
			continue;
		}
		
		return filename;
	}
	
	match -= strlen(TAR_FILE_EXT);
	
	*(match) = '\0';
	
	if (file_exists(filename) == 1) {
		return filename;
	}
	
	free(filename);
	
	return NULL;
	
}

int repolist_remove_single_package(
	repolist_t* const list,
	pkg_t* const pkg
) {
	
	int err = APTERR_SUCCESS;
	
	int status = 0;
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	installation_t* installation = NULL;
	hquery_t* query = NULL;
	options_t* options = NULL;
	
	const char* entries = NULL;
	
	size_t removed = 0;
	
	char* name = NULL;
	
	if (!pkg->installed) {
		goto end;
	}
	
	installation = &pkg->installation;
	query = &installation->metadata;
	
	loggln(LOG_STANDARD, "Removing %s (%s) ...", pkg->name, pkg->version);
	
	loggln(LOG_VERBOSE, "Removing package files from '%s'", pkg->name);
	
	options = get_options();
	
	entries = query_get_string(query, "Entries");
	
	if (entries == NULL) {
		err = APTERR_REPO_CONF_MISSING_FIELD;
		goto end;
	}
	
	strsplit_init(&split, &part, entries, ",");
	
	while (1) {
		if (strsplit_next(&split, &part) == NULL) {
			if (!removed) {
				break;
			}
			
			removed = 0;
			
			strsplit_init(&split, &part, entries, ",");
			
			continue;
		}
		
		name = malloc(strlen(options->prefix) + strlen(PATHSEP_S) + part.size + 1);
		
		if (name == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(name, options->prefix);
		strcat(name, PATHSEP_S);
		strncat(name, part.begin, part.size);
		
		status = -1;
		
		if (file_exists(name) == 1) {
			status = remove_file(name);
		} else if (directory_exists(name) == 1) {
			status = remove_empty_directory(name);
		}
		
		removed += (status == 0);
		
		if (status != 0) {
			continue;
		}
		
		loggln(LOG_VERBOSE, "Removed '%s'", name);
	}
	
	remove_file(installation->filename);
	
	loggln(LOG_VERBOSE, "Removed '%s'", installation->filename);
	
	loggln(LOG_VERBOSE, "Marking '%s' as not installed", pkg->name);
	
	pkg->installed = 0;
	pkgs_delete(&list->installed, pkg);
	
	end:;
	
	return err;
	
}


int repolist_install_single_package(
	repolist_t* const list,
	pkg_t* const pkg
) {
	
	int err = APTERR_SUCCESS;
	
	installation_t* installation = NULL;
	hquery_t* query = NULL;
	options_t* options = NULL;
	
	logging_t loglevel = 0;
	
	archive_entries_t entries = {0};
	
	size_t index = 0;
	size_t size = 0;
	
	ssize_t status = 0;
	
	repo_t* repo = NULL;
	
	char* command = NULL;
	
	const char* version = NULL;
	const char* entry = NULL;
	const char* file_extension = NULL;
	const char* loader = NULL;
	const char* triplet = NULL;
	
	const char* a = NULL;
	const char* b = NULL;
	
	char* directory = NULL;
	char* temporary_directory = NULL;
	
	char* filename = NULL;
	
	char* src = NULL;
	
	char* buffer = NULL;
	char* match = NULL;
	
	walkdir_t walkdir = {0};
	const walkdir_item_t* item = NULL;
	
	fstream_t* stream = NULL;
	
	char chunk[4];
	
	repo = repolist_get_pkg_repo(list, pkg);
	
	installation = &pkg->installation;
	query = &installation->metadata;
	
	if (!(pkg->upgradable || !pkg->installed)) {
		goto end;
	}
	
	options = get_options();
	
	directory = get_local_temp_dir();
	
	if (directory == NULL) {
		err = APTERR_NO_TMPDIR;
		goto end;
	}
	
	temporary_directory = malloc(
		strlen(directory) +
		strlen(PATHSEP_S) +
		strlen(pkg->name) +
		1
	);
	
	if (temporary_directory == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(temporary_directory, directory);
	strcat(temporary_directory, PATHSEP_S);
	strcat(temporary_directory, pkg->name);
	
	if (create_directory(temporary_directory) != 0) {
		err = APTERR_FS_MKDIR_FAILURE;
		goto end;
	}
	
	if (pkg->upgradable) {
		loglevel = loglevel_get();
		loglevel_set(LOG_QUIET);
		
		err = repolist_remove_single_package(list, pkg);
		
		loglevel_set(loglevel);
		
		if (err != APTERR_SUCCESS) {
			goto end;
		}
	}
	
	logg(LOG_STANDARD, "Unpacking %s (%s)", pkg->name, pkg->version);
	
	if (pkg->upgradable) {
		version = query_get_string(query, "Version");
		logg(LOG_STANDARD, " over (%s)", version);
	}
	
	loggln(LOG_STANDARD, " ...");
	
	if (repo->type == REPO_TYPE_APT) {
		if (set_current_directory(temporary_directory) != 0) {
			err = APTERR_FS_CHDIR_FAILURE;
			goto end;
		}
		
		loggln(LOG_VERBOSE, "Unpacking '%s' to '%s'", pkg->filename, temporary_directory);
		
		err = uncompress(pkg->filename, 0, NULL, NULL, NULL);
		
		if (err != 0) {
			err = APTERR_ARCHIVE_UNCOMPRESS_FAILURE;
			goto end;
		}
		
		err = remove_file(pkg->filename);
		
		if (err != 0) {
			err = APTERR_FS_RM_FAILURE;
			goto end;
		}
		
		free(filename);
		filename = find_file(temporary_directory, "control");
		
		if (filename == NULL) {
			err = APTERR_PKG_CONTROL_FILE_MISSING;
			goto end;
		}
		
		loggln(LOG_VERBOSE, "Found control file at '%s'", filename);
		
		err = uncompress(filename, 0, NULL, NULL, NULL);
		
		if (err != 0) {
			err = APTERR_ARCHIVE_UNCOMPRESS_FAILURE;
			goto end;
		}
	}
	
	if (set_current_directory(options->prefix) != 0) {
		err = APTERR_FS_CHDIR_FAILURE;
		goto end;
	}
	
	switch (repo->type) {
		case REPO_TYPE_APT: {
			free(filename);
			filename = find_file(temporary_directory, "preinst");
			
			if (options->maintainer_scripts && filename != NULL) {
				loggln(LOG_VERBOSE, "Found pre-install script at '%s'", filename);
				
				free(command);
				
				if (pkg->upgradable) {
					command = malloc(
						strlen(filename) + 1 +
						strlen(KUPGRADE) + 1 +
						strlen(version) + 1 +
						strlen(pkg->version) + 1
					);
					
					if (command == NULL) {
						err = APTERR_MEM_ALLOC_FAILURE;
						goto end;
					}
					
					strcpy(command, filename);
					strcat(command, " ");
					strcat(command, KUPGRADE);
					strcat(command, " ");
					strcat(command, version);
					strcat(command, " ");
					strcat(command, pkg->version);
				} else {
					command = malloc(
						strlen(filename) + 1 +
						strlen(KINSTALL) + 1
					);
					
					if (command == NULL) {
						err = APTERR_MEM_ALLOC_FAILURE;
						goto end;
					}
					
					strcpy(command, filename);
					strcat(command, " ");
					strcat(command, KINSTALL);
				}
				
				loggln(LOG_VERBOSE, "Execute subprocess: '%s'", command);
				
				execute_shell_command(command);
			}
			
			free(filename);
			filename = find_file(temporary_directory, "data");
			
			if (filename == NULL) {
				err = APTERR_PKG_DATA_FILE_MISSING;
				goto end;
			}
			
			loggln(LOG_VERBOSE, "Found data file at '%s'", filename);
			
			break;
		}
		case REPO_TYPE_APK: {
			filename = malloc(strlen(pkg->filename) + 1);
			
			if (filename == NULL) {
				err = APTERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strcpy(filename, pkg->filename);
			
			break;
		}
		default: {
			err = APTERR_REPO_UNKNOWN_FORMAT;
			goto end;
		}
	}
	
	loggln(LOG_VERBOSE, "Unpacking package files from '%s' to '%s'", filename, options->prefix);
	
	err = uncompress(filename, 0, NULL, NULL, &entries);
	
	if (err != 0) {
		err = APTERR_ARCHIVE_UNCOMPRESS_FAILURE;
		goto end;
	}
	
	loggln(LOG_STANDARD, "Setting up %s (%s) ...", pkg->name, pkg->version);
	
	switch (repo->type) {
		case REPO_TYPE_APT: {
			free(filename);
			filename = find_file(temporary_directory, "postinst");
			
			if (options->maintainer_scripts && filename != NULL) {
				loggln(LOG_VERBOSE, "Found post-install script at '%s'", filename);
				
				free(command);
				
				command = malloc(
					strlen(filename) + 1 +
					strlen(KCONFIGURE) + 1 +
					((version == NULL) ? 0 : strlen(version)) + 1
				);
				
				if (command == NULL) {
					err = APTERR_MEM_ALLOC_FAILURE;
					goto end;
				}
				
				strcpy(command, filename);
				strcat(command, " ");
				strcat(command, KCONFIGURE);
				
				if (version != NULL) {
					strcat(command, " ");
					strcat(command, version);
				}
				
				loggln(LOG_VERBOSE, "Execute subprocess: '%s'", command);
				
				execute_shell_command(command);
			}
			
			break;
		}
		case REPO_TYPE_APK: {
			break;
		}
		default: {
			err = APTERR_REPO_UNKNOWN_FORMAT;
			goto end;
		}
	}
	
	loggln(LOG_VERBOSE, "Deleting temporary files from '%s'", temporary_directory);
	
	err = remove_directory(temporary_directory);
	
	if (err != 0) {
		err = APTERR_FS_RM_FAILURE;
		goto end;
	}
	
	for (index = 0; index < entries.offset; index++) {
		entry = entries.items[index];
		size += strlen(entry) + 1;
	}
	
	size++;
	
	buffer = malloc(size);
	
	if (buffer == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	buffer[0] = '\0';
	
	for (index = 0; index < entries.offset; index++) {
		entry = entries.items[index];
		
		if (strncmp("./", entry, 2) == 0) {
			entry += 2;
		}
		
		if (*entry == '\0') {
			continue;
		}
		
		if (*buffer != '\0') {
			strcat(buffer, ",");
		}
		
		strcat(buffer, entry);
		
		match = strchr(buffer, '\0');
		
		if (match == buffer) {
			continue;
		}
		
		match--;
		
		if (*match != '/') {
			continue;
		}
		
		*match = '\0';
	}
	
	loader = get_loader(pkg->arch);
	triplet = get_triplet(pkg->arch);
	
	a = basename(loader);
	
	for (index = 0; index < entries.offset; index++) {
		entry = entries.items[index];
		
		if (strncmp("./", entry, 2) == 0) {
			entry += 2;
		}
		
		if (*entry == '\0') {
			continue;
		}
		
		if (symlink_exists(entry) != 1) {
			continue;
		}
		
		free(filename);
		filename = get_symlink(entry);
		
		if (filename == NULL) {
			err = APTERR_FS_READLINK_FAILURE;
			goto end;
		}
		
		if (!isabsolute(filename)) {
			continue;
		}
		
		loggln(LOG_VERBOSE, "Removing hardcoded symlink at '%s'", entry);
		
		if (remove_file(entry) != 0) {
			err = APTERR_FS_RM_FAILURE;
			goto end;
		}
		
		src = malloc(strlen(options->prefix) + strlen(filename) + 1);
		
		if (src == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(src, options->prefix);
		strcat(src, filename);
		
		free(filename);
		filename = src;
		
		loggln(LOG_VERBOSE, "Symlinking '%s' to '%s'", filename, entry);
		
		if (mklink(filename, entry) != 0) {
			err = APTERR_FS_SYMLINK_FAILURE;
			goto end;
		}
	}
	
	if (options->symlink_prefix != NULL && directory_exists(options->symlink_prefix) == 1) {
		if (walkdir_init(&walkdir, options->symlink_prefix) == -1) {
			err = APTERR_FS_WALKDIR_FAILURE;
			goto end;
		}
		
		while ((item = walkdir_next(&walkdir)) != NULL) {
			if (strcmp(item->name, ".") == 0 || strcmp(item->name, "..") == 0) {
				continue;
			}
			
			if (symlink_exists(item->name) == 1) {
				continue;
			}
			
			free(filename);
			filename = malloc(strlen(options->symlink_prefix) + strlen(PATHSEP_S) + strlen(item->name) + 1);
			
			if (filename == NULL) {
				err = APTERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strcpy(filename, options->symlink_prefix);
			strcat(filename, PATHSEP_S);
			strcat(filename, item->name);
			
			loggln(LOG_VERBOSE, "Symlinking '%s' to '%s'", filename, item->name);
			
			if (mklink(filename, item->name) != 0) {
				err = APTERR_FS_SYMLINK_FAILURE;
				goto end;
			}
		}
	}
	
	query_init(query, '\n', ": ");
	
	err = query_add_string(query, "Package", pkg->name);
	
	if (err != 0) {
		err = APTERR_PKG_METADATA_WRITE_FAILURE;
		goto end;
	}
	
	err = query_add_string(query, "Version", pkg->version);
	
	if (err != 0) {
		err = APTERR_PKG_METADATA_WRITE_FAILURE;
		goto end;
	}
	
	err = query_add_uint(query, "Auto-Install", pkg->autoinstall);
	
	if (err != 0) {
		err = APTERR_PKG_METADATA_WRITE_FAILURE;
		goto end;
	}
	
	err = query_add_string(query, "Entries", buffer);
	
	if (err != 0) {
		err = APTERR_PKG_METADATA_WRITE_FAILURE;
		goto end;
	}
	
	loggln(LOG_VERBOSE, "Exporting package metadata to '%s'", installation->filename);
	
	err = query_dump_file(query, installation->filename);
	
	if (err != 0) {
		err = APTERR_PKG_METADATA_WRITE_FAILURE;
		goto end;
	}
	
	loggln(LOG_VERBOSE, "Marking '%s' as installed", pkg->name);
	
	pkg->upgradable = 0;
	pkg->installed = 1;
	pkg->removable = -1;
	
	err = pkgs_append(&list->installed, pkg, 0);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	end:;
	
	free(temporary_directory);
	free(directory);
	free(filename);
	free(buffer);
	free(command);
	
	archive_entries_free(&entries);
	
	fstream_close(stream);
	
	walkdir_free(&walkdir);
	
	return err;
	
}

void repo_free(repo_t* const repo) {
	
	repo->index = 0;
	
	free(repo->name);
	repo->name = NULL;
	
	free(repo->resource);
	repo->resource = NULL;
	
	free(repo->release);
	repo->release = NULL;
	
	free(repo->platform);
	repo->platform = NULL;
	
	pkgs_free(&repo->pkgs, 1);
	
	uri_free(&repo->uri);
	uri_free(&repo->base_uri);
	
}

void repolist_free(repolist_t* const list) {
	
	size_t index = 0;
	repo_t* repo = NULL;
	
	pkgs_free(&list->installed, 0);
	
	for (index = 0; index < list->offset; index++) {
		repo = &list->items[index];
		repo_free(repo);
	}
	
	free(list->items);
	list->items = NULL;
	
	list->size = 0;
	list->offset = 0;
	
	options_free();
	
}
