#include <ctype.h>
#include <string.h>

#include "strsplit.h"
#include "fs/fstream.h"
#include "query.h"
#include "package.h"
#include "logging.h"
#include "errors.h"

static const char* PKG_SECTION_KEYS[] = {
	"Architecture",
	"Breaks",
	"Bugs",
	"Build-Essential",
	"Build-Ids",
	"Built-Using",
	"Cnf-Extra-Commands",
	"Cnf-Ignore-Commands",
	"Cnf-Visible-Pkgname",
	"Conflicts",
	"Depends",
	"Description",
	"Description-md5",
	"Efi-Vendor",
	"Enhances",
	"Essential",
	"Filename",
	"Gstreamer-Decoders",
	"Gstreamer-Elements",
	"Gstreamer-Encoders",
	"Gstreamer-Uri-Sinks",
	"Gstreamer-Uri-Sources",
	"Gstreamer-Version",
	"Homepage",
	"Important",
	"Installed-Size",
	"Lua-Versions",
	"Maintainer",
	"MD5sum",
	"Modaliases",
	"Multi-Arch",
	"Origin",
	"Original-Maintainer",
	"Original-Vcs-Browser",
	"Original-Vcs-Git",
	"Package",
	"Postgresql-Catversion",
	"Pre-Depends",
	"Priority",
	"Protected",
	"Provides",
	"Recommends",
	"Replaces",
	"Ruby-Versions",
	"Section",
	"SHA1",
	"SHA256",
	"SHA512",
	"Size",
	"Source",
	"Suggests",
	"Tag",
	"Task",
	"Ubuntu-Oem-Kernel-Flavour",
	"Version",
	"X-Cargo-Built-Using"
};

int pkg_key_matches(const char* const line) {
	
	size_t index = 0;
	size_t size = 0;
	
	int matches = 0;
	
	const char* key = NULL;
	
	for (index = 0; index < sizeof(PKG_SECTION_KEYS) / sizeof(*PKG_SECTION_KEYS); index++) {
		key = PKG_SECTION_KEYS[index];
		size = strlen(key);
		
		matches = (strncmp(key, line, size) == 0);
		
		if (!matches) {
			continue;
		}
		
		return matches;
	}
	
	return matches;
	
}

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
	
	/* Description */
	value = query_get_string(query, "Description");
	
	if (value != NULL) {
		pkg->description = malloc(strlen(value) + 1);
		
		if (pkg->description == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->description, value);
	}
	
	/* Homepage */
	value = query_get_string(query, "Homepage");
	
	if (value != NULL) {
		pkg->homepage = malloc(strlen(value) + 1);
		
		if (pkg->homepage == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->homepage, value);
	}
	
	/* Bugs */
	value = query_get_string(query, "Bugs");
	
	if (value != NULL) {
		pkg->bugs = malloc(strlen(value) + 1);
		
		if (pkg->bugs == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->bugs, value);
	}
	
	/* Maintainer */
	value = query_get_string(query, "Maintainer");
	
	if (value != NULL) {
		pkg->maintainer = malloc(strlen(value) + 1);
		
		if (pkg->maintainer == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->maintainer, value);
	}
	
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
	
	/* Replaces */
	value = query_get_string(query, "Replaces");
	
	if (value != NULL) {
		pkg->replaces = malloc(strlen(value) + 1);
		
		if (pkg->replaces == NULL) {
			err = APTERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(pkg->replaces, value);
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
		pkgs_free(pkg->replaces, 0);
		maintainers_free(pkg->maintainer);
	}
	
	free(pkg->depends);
	free(pkg->breaks);
	free(pkg->recommends);
	free(pkg->suggests);
	free(pkg->replaces);
	free(pkg->homepage);
	free(pkg->bugs);
	free(pkg->maintainer);
	
	pkg->depends = NULL;
	pkg->breaks = NULL;
	pkg->recommends = NULL;
	pkg->suggests = NULL;
	pkg->replaces = NULL;
	pkg->homepage = NULL;
	pkg->bugs = NULL;
	
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
	
	size_t index = 0;
	pkg_t* pkg = NULL;
	
	if (pkgs == NULL) {
		return;
	}
	
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

void maintainer_free(maintainer_t* const maintainer) {
	
	free(maintainer->name);
	free(maintainer->email);
	
	maintainer->name = NULL;
	maintainer->email = NULL;
	
}

void maintainers_free(maintainers_t* const maintainers) {
	
	size_t index = 0;
	maintainer_t* maintainer = NULL;
	
	for (index = 0; index < maintainers->offset; index++) {
		maintainer = &maintainers->items[index];
		maintainer_free(maintainer);
	}
	
	free(maintainers->items);
	maintainers->items = NULL;
	
	maintainers->size = 0;
	maintainers->offset = 0;
	
}

int maintainers_append(
	maintainers_t* const maintainers,
	maintainer_t* const maintainer
) {
	
	size_t size = 0;
	
	maintainer_t* source = maintainer;
	maintainer_t* destination = NULL;
	
	maintainer_t* items = NULL;
	
	if (sizeof(*maintainers->items) * (maintainers->offset + 1) > maintainers->size) {
		size = maintainers->size + sizeof(*maintainers->items) * (maintainers->offset + 1);
		items = realloc(maintainers->items, size);
		
		if (items == NULL) {
			return APTERR_MEM_ALLOC_FAILURE;
		}
		
		maintainers->size = size;
		maintainers->items = items;
	}
	
	destination = &maintainers->items[maintainers->offset++];
	
	memcpy(destination, source, sizeof(*destination));
	
	return APTERR_SUCCESS;
	
}

int maintainers_parse(maintainers_t* const maintainers, const char* const value) {
	
	int err = APTERR_SUCCESS;
	
	maintainer_t maintainer = {0};
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	size_t size = 0;
	
	const char* begin = NULL;
	const char* end = NULL;
	
	unsigned char ch = 0;
	
	strsplit_init(&split, &part, value, ",");
	
	while (strsplit_next(&split, &part) != NULL) {
		begin = part.begin;
		end = (part.begin + part.size) - 1;
		
		maintainer.name = NULL;
		maintainer.email = NULL;
		
		while (1) {
			ch = *begin;
			
			if (begin == end && maintainer.name == NULL) {
				if (part.size == 0) {
					err = APTERR_PACKAGE_SECTION_INVALID;
					goto end;
				}
				
				maintainer.name = malloc(part.size + 1);
				
				if (maintainer.name == NULL) {
					err = APTERR_MEM_ALLOC_FAILURE;
					goto end;
				}
				
				strncpy(maintainer.name, part.begin, part.size);
				maintainer.name[part.size] = '\0';
				
				break;
			}
			
			if (ch == '<') {
				if (maintainer.name != NULL) {
					err = APTERR_PACKAGE_SECTION_INVALID;
					goto end;
				}
				
				size = (begin - part.begin);
				
				if (size == 0) {
					err = APTERR_PACKAGE_SECTION_INVALID;
					goto end;
				}
				
				ch = *(begin - 1);
				
				if (!isspace(ch)) {
					err = APTERR_PACKAGE_SECTION_INVALID;
					goto end;
				}
				
				size--;
				
				maintainer.name = malloc(size + 1);
				
				if (maintainer.name == NULL) {
					err = APTERR_MEM_ALLOC_FAILURE;
					goto end;
				}
				
				strncpy(maintainer.name, part.begin, size);
				maintainer.name[size] = '\0';
				
				ch = *end;
				
				if (ch != '>') {
					err = APTERR_PACKAGE_SECTION_INVALID;
					goto end;
				}
				
				begin++;
				
				size = (end - begin);
				
				if (size == 0) {
					err = APTERR_PACKAGE_SECTION_INVALID;
					goto end;
				}
				
				maintainer.email = malloc(size + 1);
				
				if (maintainer.email == NULL) {
					err = APTERR_MEM_ALLOC_FAILURE;
					goto end;
				}
				
				strncpy(maintainer.email, begin, size);
				maintainer.email[size] = '\0';
				
				break;
			}
			
			if (begin == end) {
				break;
			}
			
			begin++;
		}
		
		if (maintainer.name == NULL) {
			err = APTERR_PACKAGE_SECTION_INVALID;
			goto end;
		}
		
		err = maintainers_append(maintainers, &maintainer);
		
		if (err != APTERR_SUCCESS) {
			maintainer_free(&maintainer);
			goto end;
		}
	}
	
	end:;
	
	return err;
	
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
	
	if (match != part->begin && match != NULL && match < end) {
		part->size -= (size_t) ((end - match) + 1);
		end = (part->begin + part->size);
	}
	
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

const char* repoarch_unstringify(const architecture_t value) {
	
	switch (value) {
		case ARCH_AMD64:
			return "amd64";
		case ARCH_i386:
			return "i386";
		case ARCH_ARMEL:
			return "armel";
		case ARCH_ARMHF:
			return "armhf";
		case ARCH_MIPS64EL:
			return "mips64el";
		case ARCH_PPC64EL:
			return "ppc64el";
		case ARCH_S390X:
			return "s390x";
		case ARCH_MIPS:
			return "mips";
		case ARCH_MIPSEL:
			return "mipsel";
		case ARCH_ARM64:
			return "arm64";
		case ARCH_IA64:
			return "ia64";
		case ARCH_ALPHA:
			return "alpha";
		case ARCH_S390:
			return "s390";
		case ARCH_SPARC:
			return "sparc";
		case ARCH_HPPA:
			return "hppa";
		case ARCH_POWERPC:
			return "powerpc";
		default:
			return "unknown";
	}
	
}
