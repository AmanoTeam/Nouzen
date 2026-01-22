#if !defined(PACKAGE_H)
#define PACKAGE_H

#include "query.h"
#include "strsplit.h"
#include "biggestint.h"

enum Architecture {
	ARCH_UNKNOWN,
	ARCH_AMD64,
	ARCH_i386,
	ARCH_ARMEL,
	ARCH_ARMHF,
	ARCH_MIPS64EL,
	ARCH_PPC64EL,
	ARCH_S390X,
	ARCH_MIPS,
	ARCH_MIPSEL,
	ARCH_ARM64,
	ARCH_IA64,
	ARCH_ALPHA,
	ARCH_S390,
	ARCH_SPARC,
	ARCH_HPPA,
	ARCH_POWERPC
};

typedef enum Architecture architecture_t;

struct Depends {
	size_t size;
	size_t offset;
	struct Package** items;
};

struct InstallStatus {
	hquery_t metadata;
	char* filename;
};

typedef struct InstallStatus installation_t;

struct Package {
	size_t index;
	char* name;
	char* version;
	char* description;
	void* depends;
	void* provides;
	void* recommends;
	void* suggests;
	void* breaks;
	void* replaces;
	void* maintainer;
	char* homepage;
	char* bugs;
	installation_t installation;
	biguint_t size;
	biguint_t installed_size;
	char* filename;
	int obsolete;
	int resolved;
	int installed;
	int upgradable;
	int removable;
	int autoinstall;
	size_t repo;
	architecture_t arch;
};

struct Packages {
	size_t size;
	size_t offset;
	struct Package** items;
};

struct PkgsIter {
	size_t index;
	struct Packages* packages;
};

struct Maintainer {
	char* name;
	char* email;
};

typedef struct Maintainer maintainer_t;

struct Maintainers {
	size_t size;
	size_t offset;
	maintainer_t* items;
};

typedef struct Maintainers maintainers_t;

typedef struct Package pkg_t;
typedef struct Packages pkgs_t;
typedef struct PkgsIter pkgs_iter_t;
typedef struct Depends depends_t;

int pkg_parse(
	hquery_t* const query,
	pkg_t* const pkg
);

void pkgs_free(
	pkgs_t * const pkgs,
	const int copy
);

int pkgs_append(
	pkgs_t * const pkgs,
	pkg_t* const pkg,
	const int copy
);

void pkgs_delete(
	pkgs_t* const pkgs,
	pkg_t* const pkg
);

pkg_t* pkg_get(
	pkgs_t * const pkgs,
	const char* const name
);

void pkgsiter_init(
	pkgs_iter_t* const iter,
	pkgs_t* const pkgs
);

pkg_t* pkgsiter_next(
	pkgs_iter_t* const iter
);

const strsplit_part_t* pkglist_split_next(
	strsplit_t* const strsplit,
	strsplit_part_t* const part
);

int pkgs_exists(
	const pkgs_t* const pkgs,
	const pkg_t* const pkg
);

void pkgs_free(
	pkgs_t * const pkgs,
	const int copy
);

int maintainers_parse(maintainers_t* const maintainers, const char* const value);
void maintainers_free(maintainers_t* const maintainers);

architecture_t get_architecture(const char* const name);
const char* repoarch_unstringify(const architecture_t value);

int pkg_key_matches(const char* const line);

#endif
