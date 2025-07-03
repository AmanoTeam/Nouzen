#include "biggestint.h"

struct Options {
	char* prefix;
	char* symlink_prefix;
	int force_refresh;
	int cache;
	int assume_yes;
	int maintainer_scripts;
	int patchelf;
	biguint_t concurrency;
};

typedef struct Options options_t;

int options_load(const char* const directory);
options_t* get_options(void);
void options_free(void);
