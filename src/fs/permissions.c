#if defined(_WIN32)
	#include <stdlib.h>
	#include <stddef.h>
	#include <string.h>
	#include <windows.h>
	#include <fileapi.h>
#endif

#if !defined(_WIN32)
	#include <sys/stat.h>
#endif

#if defined(_WIN32)
	#include "fs/absoluteness.h"
	#include "fs/sep.h"
#endif

#include "fs/permissions.h"

int get_file_permissions(const char* const path) {
	
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
		
		err = (
			FMODE_USER_EXEC | FMODE_USER_READ | FMODE_GROUP_EXEC |
			FMODE_GROUP_READ | FMODE_OTHERS_EXEC | FMODE_OTHERS_READ
		);
		
		if ((attributes & FILE_ATTRIBUTE_READONLY) == 0) {
			err |= FMODE_USER_WRITE | FMODE_GROUP_WRITE | FMODE_OTHERS_WRITE;
		}
	#else
		struct stat st = {0};
		
		if (lstat(path, &st) == -1) {
			err = -1;
			goto end;
		}
		
		if ((st.st_mode & S_IRUSR) != 0) {
			err |= FMODE_USER_READ;
		}
		
		if ((st.st_mode & S_IWUSR) != 0) {
			err |= FMODE_USER_WRITE;
		}
		
		if ((st.st_mode & S_IXUSR) != 0) {
			err |= FMODE_USER_EXEC;
		}
		
		if ((st.st_mode & S_IRGRP) != 0) {
			err |= FMODE_GROUP_READ;
		}
		
		if ((st.st_mode & S_IWGRP) != 0) {
			err |= FMODE_GROUP_WRITE;
		}
		
		if ((st.st_mode & S_IXGRP) != 0) {
			err |= FMODE_GROUP_EXEC;
		}
		
		if ((st.st_mode & S_IROTH) != 0) {
			err |= FMODE_OTHERS_READ;
		}
		
		if ((st.st_mode & S_IWOTH) != 0) {
			err |= FMODE_OTHERS_WRITE;
		}
		
		if ((st.st_mode & S_IXOTH) != 0) {
			err |= FMODE_OTHERS_EXEC;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wpath);
	#endif
	
	return err;
	
}

int set_file_writable(const char* const path) {
	
	int err = 0;
	
	#if defined(_WIN32)
		BOOL status = FALSE;
		
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
			
			status = SetFileAttributesW(wpath, FILE_ATTRIBUTE_NORMAL);
		#else
			status = SetFileAttributesA(path, FILE_ATTRIBUTE_NORMAL);
		#endif
		
		if (!status) {
			err = -1;
			goto end;
		}
	#else
		struct stat st = {0};
		
		if (lstat(path, &st) == -1) {
			err = -1;
			goto end;
		}
		
		st.st_mode |= S_IWUSR;
		
		if (chmod(path, st.st_mode) == -1) {
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wpath);
	#endif
	
	return err;
	
}