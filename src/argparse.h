#if !defined(ARGPARSE_H)
#define ARGPARSE_H

#include <stdlib.h>

#define ARGPARSE_ERR_SUCCESS 0 /* Success */
#define ARGPARSE_ERR_MEM_ALLOC_FAILURE -1 /* Could not allocate memory */
#define ARGPARSE_ERR_VALUE_UNEXPECTED -2 /* Got an unexpected value while parsing the command-line arguments */
#define ARGPARSE_ERR_ARGUMENT_EMPTY -3 /* Got an empty argument while parsing the command-line arguments */

#if defined(_WIN32) && defined(_UNICODE)
	#define argv_t wchar_t
#else
	#define argv_t char
#endif

enum ArgumentType {
	ARGTYPE_NON_VALUE,
	ARGTYPE_VALUE,
	ARGTYPE_VALUE_ONLY
};

typedef enum ArgumentType argtype_t;

struct Argument {
	argtype_t type;
	char* key;
	char* value;
};

typedef struct Argument arg_t;

struct Arguments {
	size_t offset;
	size_t size;
	arg_t* items;
};

typedef struct Arguments args_t;

struct ArgumentParser {
	size_t index;
	size_t argc;
	argv_t** argv;
	args_t arguments;
};

typedef struct ArgumentParser argparse_t;

int argparse_init(
	argparse_t* const argparser,
	const int argc,
	argv_t** const argv
);

const arg_t* argparse_getnext(argparse_t* const argparser);

void argparse_free(argparse_t* const argparser);

#endif
