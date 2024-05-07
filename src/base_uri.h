#if !defined(BASE_URI_H)
#define BASE_URI_H

#define BASE_URI_TYPE_TEXT 0x00
#define BASE_URI_TYPE_URL 0x01
#define BASE_URI_TYPE_LOCAL_FILE 0x02
#define BASE_URI_TYPE_LOCAL_DIRECTORY 0x03

#define BASEURI_ERR_SUCCESS 0 /* Success */

#define BASEURI_ERR_MEM_ALLOC_FAILURE -1 /* Could not allocate memory */

#define BASEURI_ERR_UNSUPPORTED_URI -89 /* Could not resolve this URI; either this protocol is not supported or it was not recognized */

#define BASEURI_ERR_URL_INIT_FAILURE -57 /* Could not initialize the cURL URL interface */
#define BASEURI_ERR_URL_GET_FAILURE -58 /* Could not get URL from this cURL URL interface */
#define BASEURI_ERR_URL_SET_FAILURE -59 /* Could not set URL for this cURL URL interface */

struct BaseURI {
	int type;
	char* value;
};

typedef struct BaseURI base_uri_t;

int uri_resolve_url(const char* const a, const char* const b, char** destination);
int uri_resolve_path(const char* const a, const char* const b, char** destination);
int uri_resolve_file(const char* const a, const char* const b, char** destination);
int uri_resolve_directory(const char* const a, const char* const b, char** destination);

int uri_resolve(
	base_uri_t* const base,
	const char* const source,
	char** destination
);

void uri_free(base_uri_t* const base);

#endif
