#if !defined(ERRORS_H)
#define ERRORS_H

#define APTERR_SUCCESS 0 /* Success */

#define APTERR_ARCHIVE_UNCOMPRESS_FAILURE -1 /* Could not uncompress archive */

#define APTERR_ARGPARSE_ARGUMENT_EMPTY -2 /* Got an empty argument while parsing the command-line arguments */
#define APTERR_ARGPARSE_ARGUMENT_INVALID -3 /* This argument is invalid or was not recognized */
#define APTERR_ARGPARSE_ARGUMENT_VALUE_MISSING -4 /* This keyword argument requires a value to be supplied */
#define APTERR_ARGPARSE_INVALID_UINT -5 /* Could not parse this string as a decimal integer */
#define APTERR_ARGPARSE_TOO_MANY_PACKAGES -6 /* Too many packages specified in a single command call */
#define APTERR_ARGPARSE_VALUE_UNEXPECTED -7 /* Got an unexpected value while parsing the command-line arguments */

#define APTERR_CLI_USER_INTERRUPTED -8 /* User interrupted */

#define APTERR_EXPAND_FILENAME_FAILURE -9 /* Could not resolve filename */

#define APTERR_FSTREAM_LOCK_FAILURE -10 /* Could not lock file */
#define APTERR_FSTREAM_OPEN_FAILURE -11 /* Could not open file */
#define APTERR_FSTREAM_READ_EMPTY_FILE -12 /* Tried to read contents from an empty file */
#define APTERR_FSTREAM_READ_FAILURE -13 /* Could not read data from file */
#define APTERR_FSTREAM_SEEK_FAILURE -14 /* Could not seek file */
#define APTERR_FSTREAM_TELL_FAILURE -15 /* Could not get current file position */
#define APTERR_FSTREAM_WRITE_FAILURE -16 /* Could not write data to file */

#define APTERR_FS_CHMOD_FAILURE -17 /* Could not set file permissions */
#define APTERR_FS_GTMOD_FAILURE -18 /* Could not query file permissions */
#define APTERR_FS_MKDIR_FAILURE -19 /* Could not create directory at the specified location */
#define APTERR_FS_READLINK_FAILURE -20 /* Could not query symbolic link of file */
#define APTERR_FS_RM_FAILURE -21 /* Could not delete file/directory */
#define APTERR_FS_SYMLINK_FAILURE -22 /* Could not create symbolic link */
#define APTERR_FS_WALKDIR_FAILURE -23 /* Could not iterate over the files at the specified location */
#define APTERR_FS_CHDIR_FAILURE -24 /* Could not change the current working directory */

#define APTERR_GET_APP_DIRECTORY_FAILURE -25 /* Could not get application directory */

#define APTERR_LOAD_UNSUPPORTED_URI -26 /* Could not load repository from this URI; either this protocol is not supported or it was not recognized */

#define APTERR_MEM_ALLOC_FAILURE -27 /* Could not allocate memory */

#define APTERR_NO_TMPDIR -28 /* Could not find a suitable directory for storing temporary files */

#define APTERR_PACKAGE_DEPENDENCY_LOOP -29 /* Dependency loop */
#define APTERR_PACKAGE_MISSING_FILENAME -30 /* The metadata section of this package is missing the 'Filename' field */
#define APTERR_PACKAGE_MISSING_NAME -31 /* The metadata section of this package is missing the 'Package' field */
#define APTERR_PACKAGE_MISSING_VERSION -32 /* The metadata section of this package is missing the 'Version' field */
#define APTERR_PACKAGE_SECTION_INVALID -33 /* The metadata section of this package is invalid */
#define APTERR_PACKAGE_UNSATISFIED_DEPENDENCY -34 /* This package has an unsatisfiable dependency */
#define APTERR_PACKAGE_SEARCH_NO_MATCHES -35 /* No packages matched the query */
#define APTERR_PKG_DATA_FILE_MISSING -36 /* Could not find the 'data.tar' file inside the package archive */
#define APTERR_PKG_METADATA_WRITE_FAILURE -37 /* Could not write package metadata */
#define APTERR_PKG_RESOLVE_URI_FAILURE -38 /* Could not resolve URI to a valid resource */

#define APTERR_PATCHELF_INIT_FAILURE -39 /* Could not initialize the patchelf utility */

#define APTERR_PLATFORM_UNKNOWN -40 /* Cannot detect current platform */

#define APTERR_REPO_CONF_MISSING_FIELD -41 /* This configuration file is missing required fields */
#define APTERR_REPO_CONF_PARSE_FAILURE -42 /* Could not parse repository source list file */
#define APTERR_REPO_GET_CONFDIR_FAILURE -43 /* Could not get configuration directory */
#define APTERR_REPO_GET_PKGSDIR_FAILURE -44 /* Could not get packages directory */
#define APTERR_REPO_GET_SRCDIR_FAILURE -45 /* Could not get sources directory */
#define APTERR_REPO_LOAD_NO_SOURCES_AVAILABLE -46 /* No repository sources have been configured yet */
#define APTERR_REPO_LOAD_UNSUPPORTED_URI -47 /* Could not load repository index from this URI; either this protocol is not supported or it was not recognized */
#define APTERR_REPO_PKG_INDEX_TOO_LARGE -48 /* This package index exceeds the maximum allowed size */
#define APTERR_REPO_UNKNOWN_ARCHITECTURE -49 /* Unknown repository architecture */
#define APTERR_REPO_UNKNOWN_FORMAT -499 /* Unknown repository format */

#define APTERR_WCURLMLT_ADD_FAILURE -50 /* Could not add the cURL handler to cURL multi */
#define APTERR_WCURLMLT_INIT_FAILURE -51 /* Could not initialize the cURL multi interface */
#define APTERR_WCURLMLT_PERFORM_FAILURE -52 /* Could not perform on cURL multi */
#define APTERR_WCURLMLT_POLL_FAILURE -53 /* Could not poll on cURL multi */
#define APTERR_WCURLMLT_REMOVE_FAILURE -54 /* Could not remove the cURL handler from cURL multi */
#define APTERR_WCURLMLT_SETOPT_FAILURE -55 /* Could not set options on cURL multi */

#define APTERR_WCURL_GETINFO_FAILURE -56 /* Could not get info about HTTP transfer */
#define APTERR_WCURL_INIT_FAILURE -57 /* Could not initialize the HTTP client due to an unexpected error */
#define APTERR_WCURL_REQUEST_FAILURE -58 /* HTTP request failure */
#define APTERR_WCURL_SETOPT_FAILURE -59 /* Could not set options on HTTP client */
#define APTERR_WCURL_SLIST_FAILURE -60 /* Could not append item to list */

const char* apterr_getmessage(const int code);

#endif
