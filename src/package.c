#include <ctype.h>
#include <string.h>

#include "strsplit.h"
#include "fstream.h"
#include "query.h"
#include "package.h"
#include "logging.h"
#include "errors.h"

int pkg_parse(
	hquery_t* const query,
	pkg_t* const pkg
) {
	
	int err = APTERR_SUCCESS;
	
	const char* value = NULL;
	
	memset(pkg, 0, sizeof(*pkg));
	
	/* Package */
	value = query_get_string(query, "Package");
	
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
	
	/* Filename */
	value = query_get_string(query, "Filename");
	
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
	
	/* Version */
	value = query_get_string(query, "Version");
	
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
	
	/* Provides */
	value = query_get_string(query, "Provides");
	
	if (value != NULL) {
		pkg->provides = malloc(strlen(value) + 1);
		
		if (pkg->provides == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->provides, value);
	}
	
	/* Suggests */
	value = query_get_string(query, "Suggests");
	
	if (value != NULL) {
		pkg->suggests = malloc(strlen(value) + 1);
		
		if (pkg->suggests == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->suggests, value);
	}
	
	/* Recommends */
	value = query_get_string(query, "Recommends");
	
	if (value != NULL) {
		pkg->recommends = malloc(strlen(value) + 1);
		
		if (pkg->recommends == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->recommends, value);
	}
	
	/* Depends */
	value = query_get_string(query, "Depends");
	
	if (value != NULL) {
		pkg->depends = malloc(strlen(value) + 1);
		
		if (pkg->depends == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->depends, value);
	}
	
	/* Breaks */
	value = query_get_string(query, "Breaks");
	
	if (value != NULL) {
		pkg->breaks = malloc(strlen(value) + 1);
		
		if (pkg->breaks == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->breaks, value);
	}
	
	/* Size */
	pkg->size = query_get_uint(query, "Size");
	
	/* Installed-Size */
	pkg->installed_size = query_get_uint(query, "Installed-Size");
	
	if (pkg->installed_size != 0) {
		pkg->installed_size = pkg->installed_size * 1000;
	}
	
	pkg->autoinstall = -1;
	pkg->removable = -1;
	
	end:;
	
	return err;
	
}

void pkg_free(pkg_t* const pkg) {
	
	pkg->index = 0;
	
	free(pkg->name);
	pkg->name = NULL;
	
	free(pkg->version);
	pkg->version = NULL;
	
	if (pkg->resolved) {
		pkgs_free(pkg->depends, 0);
		pkgs_free(pkg->breaks, 0);
		pkgs_free(pkg->recommends, 0);
		pkgs_free(pkg->suggests, 0);
	}
	
	free(pkg->depends);
	free(pkg->breaks);
	free(pkg->recommends);
	free(pkg->suggests);
	
	pkg->depends = NULL;
	pkg->breaks = NULL;
	pkg->recommends = NULL;
	pkg->suggests = NULL;
		
	free(pkg->installation.filename);
	pkg->installation.filename = NULL;
	
	query_free(&pkg->installation.metadata);
	
	free(pkg->provides);
	pkg->provides = NULL;
	
	pkg->size = 0;
	pkg->installed_size = 0;
	
	free(pkg->filename);
	pkg->filename = NULL;
	
	pkg->obsolete = 0;
	pkg->resolved = 0;
	pkg->installed = 0;
	pkg->upgradable = 0;
	pkg->removable = 0;
	pkg->autoinstall = 0;
	pkg->repo = 0;
	
	free(pkg);
	
}

void pkgs_free(
	pkgs_t * const pkgs,
	const int copy
) {
	
	if (pkgs == NULL) {
		return;
	}
	
	size_t index = 0;
	pkg_t* pkg = NULL;
	
	/* If the items within the list are not independent copies, we should not free them */
	for (index = 0; copy && index < pkgs->offset; index++) {
		pkg = pkgs->items[index];
		pkg_free(pkg);
	}
	
	free(pkgs->items);
	pkgs->items = NULL;
	
	pkgs->size = 0;
	pkgs->offset = 0;
	
}

int pkgs_exists(
	const pkgs_t* const pkgs,
	const pkg_t* const pkg
) {
	
	size_t index = 0;
	const pkg_t* subpkg = NULL;
	
	for (index = 0; index < pkgs->offset; index++) {
		subpkg = pkgs->items[index];
		
		if (subpkg == pkg) {
			return 1;
		}
	}
	
	return 0;
	
}

int pkgs_append(
	pkgs_t* const pkgs,
	pkg_t* const pkg,
	const int copy
) {
	
	size_t size = 0;
	
	pkg_t* item = pkg;
	pkg_t** items = NULL;
	
	if (copy) {
		item = malloc(sizeof(*item));
		
		if (item == NULL) {
			return APTERR_MEM_ALLOC_FAILURE;
		}
		
		memcpy(item, pkg, sizeof(*pkg));
	}
	
	if (sizeof(*pkgs->items) * (pkgs->offset + 1) > pkgs->size) {
		size = pkgs->size + sizeof(*pkgs->items) * (pkgs->offset + 1);
		items = realloc(pkgs->items, size);
		
		if (items == NULL) {
			return APTERR_MEM_ALLOC_FAILURE;
		}
		
		pkgs->size = size;
		pkgs->items = items;
	}
	
	pkgs->items[pkgs->offset++] = item;
	
	return APTERR_SUCCESS;
	
}

void pkgs_delete(
	pkgs_t* const pkgs,
	pkg_t* const pkg
) {
	
	size_t index = 0;
	pkg_t* subpkg = NULL;
	
	void* source = NULL;
	void* destination = NULL;
	size_t size = 0;
	
	for (index = 0; index < pkgs->offset; index++) {
		subpkg = pkgs->items[index];
		
		if (pkg != subpkg) {
			continue;
		}
		
		if ((index + 1) < pkgs->offset) {
			destination = &pkgs->items[index];
			source = &pkgs->items[index + 1];
			size = sizeof(*pkgs->items) * (pkgs->offset - index - 1);
			
			memmove(destination, source, size);
		}
		
		pkgs->items[pkgs->offset] = NULL;
		pkgs->offset--;
		
		break;
	}
	
}

const strsplit_part_t* pkglist_split_next(
	strsplit_t* const strsplit,
	strsplit_part_t* const part
) {
	
	const char* end = NULL;
	const char* match = NULL;
	
	while (1) {
		if (strsplit_next(strsplit, part) == NULL) {
			return NULL;
		}
		
		if (part->size != 0) {
			break;
		}
	}
	
	match = part->begin;
	end = (part->begin + part->size);
	
	while (match != end) {
		const unsigned char ch = *match;
		
		if (ch == ',') {
			match = NULL;
			break;
		}
		
		if (isspace(ch) || ch == ':') {
			match++;
			break;
		}
		
		match++;
	}
	
	//match = strstr(part->begin, "|");
	
	if (match != part->begin && match != NULL && match < end) {
		part->size -= (size_t) (end - match) + 1;
		end = (part->begin + part->size);
	}
	
	/*
	if (*(end - 1) == ')') {
		match = strstr(part->begin, "(");
		
		if (match != NULL && match < end) {
			part->size -= (size_t) (end - match) + 1;
			end = (part->begin + part->size);
		}
	}
	
	match = strstr(part->begin, ":");
	
	if (match != NULL && match < end) {
		part->size -= (size_t) (end - match);
		end = (part->begin + part->size);
	}
	*/
	return part;
	
}

void pkgsiter_init(
	pkgs_iter_t* const iter,
	pkgs_t* const pkgs
) {
	
	iter->index = 0;
	iter->packages = pkgs;
	
}

pkg_t* pkgsiter_next(
	pkgs_iter_t* const iter
) {
	
	if (iter->packages == NULL) {
		return NULL;
	}
	
	if (iter->packages->offset == 0) {
		return NULL;
	}
	
	if (iter->index > (iter->packages->offset - 1)) {
		return NULL;
	}
	
	return iter->packages->items[iter->index++];
	
}

architecture_t get_architecture(const char* const name) {
	
	if (strcmp(name, "amd64") == 0 || strcmp(name, "x86_64") == 0) {
		return ARCH_AMD64;
	}
	
	if (strcmp(name, "i386") == 0 || strcmp(name, "i686") == 0) {
		return ARCH_i386;
	}
	
	if (strcmp(name, "armel") == 0 || strcmp(name, "arm") == 0) {
		return ARCH_ARMEL;
	}
	
	if (strcmp(name, "armhf") == 0) {
		return ARCH_ARMHF;
	}
	
	if (strcmp(name, "mips64el") == 0) {
		return ARCH_MIPS64EL;
	}
	
	if (strcmp(name, "ppc64el") == 0) {
		return ARCH_PPC64EL;
	}
	
	if (strcmp(name, "s390x") == 0) {
		return ARCH_S390X;
	}
	
	if (strcmp(name, "mips") == 0) {
		return ARCH_MIPS;
	}
	
	if (strcmp(name, "mipsel") == 0) {
		return ARCH_MIPSEL;
	}
	
	if (strcmp(name, "arm64") == 0 || strcmp(name, "aarch64") == 0) {
		return ARCH_ARM64;
	}
	
	if (strcmp(name, "ia64") == 0) {
		return ARCH_IA64;
	}
	
	if (strcmp(name, "alpha") == 0) {
		return ARCH_ALPHA;
	}
	
	if (strcmp(name, "s390") == 0) {
		return ARCH_S390;
	}
	
	if (strcmp(name, "sparc") == 0) {
		return ARCH_SPARC;
	}
	
	if (strcmp(name, "hppa") == 0) {
		return ARCH_HPPA;
	}
	
	if (strcmp(name, "powerpc") == 0) {
		return ARCH_POWERPC;
	}
	
	return ARCH_UNKNOWN;
	
}
