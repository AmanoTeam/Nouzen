#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
#endif

#if !defined(_WIN32)
	#include <sys/stat.h>
	#include <unistd.h>
	#include <limits.h>
#endif

#if defined(_WIN32)
	#include "fs/absoluteness.h"
	#include "fs/sep.h"
	#include "fs/exists.h"
#endif

int symlink_exists(const char* const path) {
	/*
	Checks if the path is a symbolic link.
	
	Returns (1) if the path is a symbolic link, (0) if it is not, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		DWORD attributes = 0;
		
		#if defined(_UNICODE)
			wchar_t* wpath = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(path) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wpaths = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
			
			if (wpaths == 0) {
				err = -1;
				goto end;
			}
			
			wpath = malloc((prefixs + (size_t) wpaths) * sizeof(*wpath));
			
			if (wpath == NULL) {
				err = -1;
				goto end;
			}
			
			if (prefixs > 0) {
				wcscpy(wpath, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath + prefixs, wpaths) == 0) {
				err = -1;
				goto end;
			}
			
			attributes = GetFileAttributesW(wpath);
		#else
			attributes = GetFileAttributesA(path);
		#endif
		
		if (attributes == INVALID_FILE_ATTRIBUTES) {
			err = -1;
			goto end;
		}
		
		err = (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
	#else
		struct stat st = {0};
		
		if (lstat(path, &st) == -1) {
			err = -1;
			goto end;
		}
		
		err = S_ISLNK(st.st_mode);
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wpath);
	#endif
	
	return err;
	
}

int create_symlink(const char* const source, const char* const destination) {
	/*
	Create a symbolic link.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		int flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
		
		if (directory_exists(source)) {
			flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
		}
		
		#if defined(_UNICODE)
			wchar_t* wsource = NULL;
			wchar_t* wdestination = NULL;
			
			int wfilenames = 0;
			
			/* This prefix is required to support long paths in Windows 10+ */
			size_t prefixs = isabsolute(source) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wfilenames = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
			
			if (wfilenames == 0) {
				err = -1;
				goto end;
			}
			
			wsource = malloc((prefixs + (size_t) wfilenames) * sizeof(*wsource));
			
			if (prefixs > 0) {
				wcscpy(wsource, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, source, -1, wsource + prefixs, wfilenames) == 0) {
				err = -1;
				goto end;
			}
			
			prefixs = isabsolute(destination) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wfilenames = MultiByteToWideChar(CP_UTF8, 0, destination, -1, NULL, 0);
			
			if (wfilenames == 0) {
				err = -1;
				goto end;
			}
			
			wdestination = malloc((prefixs + (size_t) wfilenames) * sizeof(*wdestination));
			
			if (prefixs > 0) {
				wcscpy(wdestination, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, destination, -1, wdestination + prefixs, wfilenames) == 0) {
				err = -1;
				goto end;
			}
			
			if (CreateSymbolicLinkW(wsource, wdestination, flags) == FALSE) {
				err = -1;
				goto end;
			}
		#else
			if (CreateSymbolicLinkA(source, destination, flags) == FALSE) {
				err = -1;
				goto end;
			}
		#endif
	#else
		if (symlink(source, destination) == -1) {
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wsource);
		free(wdestination);
	#endif
	
	return err;
	
}

char* get_symlink(const char* const path) {
	
	char* resolved = NULL;
	
	#if defined(_WIN32)
		return resolved;
	#else
		ssize_t size = 0;
		char tmp[PATH_MAX];
		
		size = readlink(path, tmp, sizeof(tmp));
		
		if (size == -1) {
			goto end;
		}
		
		tmp[(size_t) size] = '\0';
		
		resolved = malloc(strlen(tmp) + 1);
		
		if (resolved == NULL) {
			goto end;
		}
		
		strcpy(resolved, tmp);
	#endif
	
	end:;
	
	return resolved;
	
}