#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <curl/urlapi.h>

#include "wcurl.h"
#include "fs/absoluteness.h"
#include "fs/basename.h"
#include "fs/sep.h"
#include "base_uri.h"

void uri_free(base_uri_t* const base) {
	
	base->type = BASE_URI_TYPE_TEXT;
	free(base->value);
	base->value = NULL;
	
}

int uri_resolve_url(const char* const a, const char* const b, char** destination) {
	
	int err = BASEURI_ERR_SUCCESS;
	
	CURLU* cu = curl_url();
	
	if (cu == NULL) {
		err = BASEURI_ERR_URL_INIT_FAILURE;
		goto end;
	}
	
	if (curl_url_set(cu, CURLUPART_URL, a, 0) != CURLUE_OK) {
		err = BASEURI_ERR_URL_SET_FAILURE;
		goto end;
	}
	
	if (curl_url_set(cu, CURLUPART_URL, b, 0) != CURLUE_OK) {
		err = BASEURI_ERR_URL_SET_FAILURE;
		goto end;
	}
	
	if (curl_url_get(cu, CURLUPART_URL, destination, 0) != CURLUE_OK) {
		err = BASEURI_ERR_URL_GET_FAILURE;
		goto end;
	}
	
	end:;
	
	curl_url_cleanup(cu);
	
	return err;
	
}

int uri_resolve_path(const char* const a, const char* const b, char** destination) {
	
	char path[PATH_MAX];
	
	if (isabsolute(b)) {
		strcpy(path, b);
	} else {
		const char* const name = basename(a);
		const size_t size = ((size_t) (name - a)) - 1;
		
		path[0] = '\0';
		
		if ((size + strlen(PATHSEP_S) + strlen(b) + 1) > sizeof(path)) {
			return WCURL_ERR_BUFFER_OVERFLOW;
		}
		
		memcpy(path, a, size);
		path[size] = '\0';
		
		strcat(path, PATHSEP_S);
		strcat(path, b);
	}
	
	*destination = malloc(strlen(path) + 1);
	
	if (*destination == NULL) {
		return BASEURI_ERR_MEM_ALLOC_FAILURE;
	}
	
	strcpy(*destination, path);
	
	return BASEURI_ERR_SUCCESS;
	
}

int uri_resolve_file(const char* const a, const char* const b, char** destination) {
	
	return uri_resolve_path(a, b, destination);
	
}

int uri_resolve_directory(const char* const a, const char* const b, char** destination) {
	
	int err = BASEURI_ERR_SUCCESS;
	char* directory = NULL;
	
	const unsigned char ch = *(strchr(a, '\0') - 1);
	const int sep = (ch == PATHSEP);
	
	if (sep) {
		return uri_resolve_path(a, b, destination);
	}
	
	directory = malloc(strlen(a) + strlen(PATHSEP_S) + 1);
	
	if (directory == NULL) {
		return BASEURI_ERR_MEM_ALLOC_FAILURE;
	}
	
	strcpy(directory, a);
	strcat(directory, PATHSEP_S);
	
	err = uri_resolve_path(directory, b, destination);
	
	free(directory);
	
	return err;
	
}

int uri_resolve(
	base_uri_t* const base,
	const char* const source,
	char** destination
) {
	
	int err = BASEURI_ERR_SUCCESS;
	
	switch (base->type) {
		case BASE_URI_TYPE_URL:
			err = uri_resolve_url(base->value, source, destination);
			break;
		case BASE_URI_TYPE_LOCAL_FILE:
			err = uri_resolve_file(base->value, source, destination);
			break;
		case BASE_URI_TYPE_LOCAL_DIRECTORY:
			err = uri_resolve_directory(base->value, source, destination);
			break;
		default:
			err = BASEURI_ERR_UNSUPPORTED_URI;
			break;
	}
	
	return err;
	
}
