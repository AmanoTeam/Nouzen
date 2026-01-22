/*
This file is auto-generated. Use the tool at ../tools/errors.h.py to regenerate.
*/

#include "errors.h"

const char* apterr_getmessage(const int code) {
	
	switch (code) {
		case APTERR_SUCCESS:
			return "Success";
		case APTERR_ARCHIVE_UNCOMPRESS_FAILURE:
			return "Could not uncompress archive";
		case APTERR_ARGPARSE_ARGUMENT_EMPTY:
			return "Got an empty argument while parsing the command-line arguments";
		case APTERR_ARGPARSE_ARGUMENT_INVALID:
			return "This argument is invalid or was not recognized";
		case APTERR_ARGPARSE_ARGUMENT_VALUE_MISSING:
			return "This keyword argument requires a value to be supplied";
		case APTERR_ARGPARSE_INVALID_UINT:
			return "Could not parse this string as a decimal integer";
		case APTERR_ARGPARSE_TOO_MANY_PACKAGES:
			return "Too many packages specified in a single command call";
		case APTERR_ARGPARSE_VALUE_UNEXPECTED:
			return "Got an unexpected value while parsing the command-line arguments";
		case APTERR_CLI_USER_INTERRUPTED:
			return "User interrupted";
		case APTERR_EXPAND_FILENAME_FAILURE:
			return "Could not resolve filename";
		case APTERR_FSTREAM_LOCK_FAILURE:
			return "Could not lock file";
		case APTERR_FSTREAM_OPEN_FAILURE:
			return "Could not open file";
		case APTERR_FSTREAM_READ_EMPTY_FILE:
			return "Tried to read contents from an empty file";
		case APTERR_FSTREAM_READ_FAILURE:
			return "Could not read data from file";
		case APTERR_FSTREAM_SEEK_FAILURE:
			return "Could not seek file";
		case APTERR_FSTREAM_TELL_FAILURE:
			return "Could not get current file position";
		case APTERR_FSTREAM_WRITE_FAILURE:
			return "Could not write data to file";
		case APTERR_FS_CHMOD_FAILURE:
			return "Could not set file permissions";
		case APTERR_FS_GTMOD_FAILURE:
			return "Could not query file permissions";
		case APTERR_FS_MKDIR_FAILURE:
			return "Could not create directory at the specified location";
		case APTERR_FS_READLINK_FAILURE:
			return "Could not query symbolic link of file";
		case APTERR_FS_RM_FAILURE:
			return "Could not delete file/directory";
		case APTERR_FS_SYMLINK_FAILURE:
			return "Could not create symbolic link";
		case APTERR_FS_WALKDIR_FAILURE:
			return "Could not iterate over the files at the specified location";
		case APTERR_GET_APP_DIRECTORY_FAILURE:
			return "Could not get application directory";
		case APTERR_LOAD_UNSUPPORTED_URI:
			return "Could not load repository from this URI; either this protocol is not supported or it was not recognized";
		case APTERR_MEM_ALLOC_FAILURE:
			return "Could not allocate memory";
		case APTERR_NO_TMPDIR:
			return "Could not find a suitable directory for storing temporary files";
		case APTERR_PACKAGE_DEPENDENCY_LOOP:
			return "Dependency loop";
		case APTERR_PACKAGE_MISSING_FILENAME:
			return "The metadata section of this package is missing the 'Filename' field";
		case APTERR_PACKAGE_MISSING_NAME:
			return "The metadata section of this package is missing the 'Package' field";
		case APTERR_PACKAGE_MISSING_VERSION:
			return "The metadata section of this package is missing the 'Version' field";
		case APTERR_PACKAGE_SECTION_INVALID:
			return "The metadata section of this package is invalid";
		case APTERR_PACKAGE_UNSATISFIED_DEPENDENCY:
			return "This package has an unsatisfiable dependency";
		case APTERR_PACKAGE_SEARCH_NO_MATCHES:
			return "No packages matched the query";
		case APTERR_PATCHELF_INIT_FAILURE:
			return "Could not initialize the patchelf utility";
		case APTERR_PKG_DATA_FILE_MISSING:
			return "Could not find the 'data.tar' file inside the package archive";
		case APTERR_PKG_METADATA_WRITE_FAILURE:
			return "Could not write package metadata";
		case APTERR_PKG_RESOLVE_URI_FAILURE:
			return "Could not resolve URI to a valid resource";
		case APTERR_PLATFORM_UNKNOWN:
			return "Cannot detect current platform";
		case APTERR_REPO_CONF_MISSING_FIELD:
			return "This configuration file is missing required fields";
		case APTERR_REPO_CONF_PARSE_FAILURE:
			return "Could not parse repository source list file";
		case APTERR_REPO_GET_CONFDIR_FAILURE:
			return "Could not get configuration directory";
		case APTERR_REPO_GET_PKGSDIR_FAILURE:
			return "Could not get packages directory";
		case APTERR_REPO_GET_SRCDIR_FAILURE:
			return "Could not get sources directory";
		case APTERR_REPO_LOAD_NO_SOURCES_AVAILABLE:
			return "No sources have been configured in /etc/nouzen";
		case APTERR_REPO_LOAD_UNSUPPORTED_URI:
			return "Could not load repository index from this URI; either this protocol is not supported or it was not recognized";
		case APTERR_REPO_PKG_INDEX_TOO_LARGE:
			return "This package index exceeds the maximum allowed size";
		case APTERR_REPO_UNKNOWN_ARCHITECTURE:
			return "Unknown repository architecture";
		case APTERR_WCURLMLT_ADD_FAILURE:
			return "Could not add the cURL handler to cURL multi";
		case APTERR_WCURLMLT_INIT_FAILURE:
			return "Could not initialize the cURL multi interface";
		case APTERR_WCURLMLT_PERFORM_FAILURE:
			return "Could not perform on cURL multi";
		case APTERR_WCURLMLT_POLL_FAILURE:
			return "Could not poll on cURL multi";
		case APTERR_WCURLMLT_REMOVE_FAILURE:
			return "Could not remove the cURL handler from cURL multi";
		case APTERR_WCURLMLT_SETOPT_FAILURE:
			return "Could not set options on cURL multi";
		case APTERR_WCURL_GETINFO_FAILURE:
			return "Could not get info about HTTP transfer";
		case APTERR_WCURL_INIT_FAILURE:
			return "Could not initialize the HTTP client due to an unexpected error";
		case APTERR_WCURL_REQUEST_FAILURE:
			return "HTTP request failure";
		case APTERR_WCURL_SETOPT_FAILURE:
			return "Could not set options on HTTP client";
		case APTERR_WCURL_SLIST_FAILURE:
			return "Could not append item to list";
	}
	
	return "Unknown error";
	
}
