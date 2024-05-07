#if !defined(REPOSITORY_H)
#define REPOSITORY_H

#include "package.h"
#include "base_uri.h"

#define APT_MAX_PKG_INDEX_LEN ((1024 * 1024 * 100) + 1) /* 100 MiB */
#define APT_MAX_PKG_SECTION_LEN ((1024 * 1024 * 1) + 1) /* 1 MiB */

#define REPOLIST_RESOLVE_DEPENDS (0x00000001)
#define REPOLIST_RESOLVE_BREAKS (0x00000002)
#define REPOLIST_RESOLVE_SUGGESTS (0x00000004)
#define REPOLIST_RESOLVE_RECOMMENDS (0x00000010)

struct Repository {
	size_t index;
	char* name;
	architecture_t architecture;
	pkgs_t pkgs;
	base_uri_t uri;
	base_uri_t base_uri;
};

typedef struct Repository repo_t;

struct RepoList {
	size_t size;
	size_t offset;
	repo_t* items;
	pkgs_t installed;
};

typedef struct RepoList repolist_t;

int repolist_resolve_deps(
	repolist_t* const list,
	pkg_t* const pkg
);

void repo_free(repo_t* const repo);
void repolist_free(repolist_t* const list);

int repolist_load(repolist_t* const list);

int repolist_install_package(
	repolist_t* const list,
	char* const* const packages
);

int repolist_install_single_package(
	repolist_t* const list,
	pkg_t* const pkg
);

int repolist_remove_package(
	repolist_t* const list,
	char* const* const packages
);

int repolist_remove_single_package(
	repolist_t* const list,
	pkg_t* const pkg
);

int repolist_destroy(repolist_t* const list);

pkg_t* repolist_get_pkg(
	const repolist_t* const list,
	const char* const name
);

char* repo_get_config_dir(void);

#endif
