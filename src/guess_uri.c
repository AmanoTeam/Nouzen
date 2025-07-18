#include <stdlib.h>

#include <string.h>
#include <ctype.h>

#include "guess_uri.h"
#include "fs/exists.h"

int uri_guess_type(const char* const something) {
	
	int status = 0;
	
	const unsigned char a = something[0];
	const unsigned char b = something[1];
	
	if (strncmp(something, "https://", 8) == 0 || strncmp(something, "http://", 7) == 0) {
		return GUESS_URI_TYPE_URL;
	}
	
	status = (
		(a == '.' && (b == '/' || b == '\\')) || (isalpha(a) && b == ':') || (a == '/' || a == '\\')
	);
	
	if (!status) {
		return GUESS_URI_TYPE_SOMETHING_ELSE;
	}
	
	if (file_exists(something) == 1) {
		return GUESS_URI_TYPE_LOCAL_FILE;
	}
	
	if (directory_exists(something) == 1) {
		return GUESS_URI_TYPE_LOCAL_DIRECTORY;
	}
	
	return GUESS_URI_TYPE_SOMETHING_ELSE;
	
}
