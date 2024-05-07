#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/types.h>

#include <archive.h>
#include <archive_entry.h>

#if defined(_WIN32) && defined(_UNICODE)
	#include "wio.h"
#endif

#include "uncompress.h"

static const size_t ARCHIVE_BLOCK_SIZE = 10240;

int entries_append(
	archive_entries_t* const entries,
	const char* const entry
) {
	
	size_t size = 0;
	
	char* item = NULL;
	char** items = NULL;
	
	item = malloc(strlen(entry) + 1);
	
	if (item == NULL) {
		return -1;
	}
	
	strcpy(item, entry);
	
	if (sizeof(*entries->items) * (entries->offset + 1) > entries->size) {
		size = entries->size + sizeof(*entries->items) * (entries->offset + 1);
		items = realloc(entries->items, size);
		
		if (items == NULL) {
			return -1;
		}
		
		entries->size = size;
		entries->items = items;
	}
	
	entries->items[entries->offset++] = item;
	
	return 0;
	
}

int uncompress(
	const char* const source,
	const size_t size,
	uncompress_callback_t callback,
	void* const callback_data,
	archive_entries_t* const entries
) {
	
	int code = 0;
	int err = 0;
	
	const char* pathname = NULL;
	
	struct archive* input_archive = archive_read_new();
	struct archive* output_archive = NULL;
	
	struct archive_entry* entry = NULL;
	
	const int flags = ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
	const void* chunk = NULL;
	
	size_t rsize = 0;
	
	#if ARCHIVE_VERSION_NUMBER >= 3000000
		int64_t offset = 0;
	#else
		off_t offset = 0;
	#endif
	
	/* Required for proper handling of Unicode characters in filenames */
	if (setlocale(LC_ALL, "") == NULL) {
		err = -1;
		goto end;
	}
	
	if (input_archive == NULL) {
		err = -1;
		goto end;
	}
	
	code = archive_read_support_filter_xz(input_archive);
	
	if (code != ARCHIVE_OK) {
		err = -1;
		goto end;
	}
	
	code = archive_read_support_filter_zstd(input_archive);
	
	if (!(code == ARCHIVE_OK || code == ARCHIVE_WARN)) {
		err = -1;
		goto end;
	}
	
	code = archive_read_support_filter_gzip(input_archive);
	
	if (code != ARCHIVE_OK) {
		err = -1;
		goto end;
	}
	
	code = archive_read_support_filter_bzip2(input_archive);
	
	if (code != ARCHIVE_OK) {
		err = -1;
		goto end;
	}
	
	code = archive_read_support_format_tar(input_archive);
	
	if (code != ARCHIVE_OK) {
		err = -1;
		goto end;
	}
	
	code = archive_read_support_format_ar(input_archive);
	
	if (code != ARCHIVE_OK) {
		err = -1;
		goto end;
	}
	
	code = archive_read_support_format_raw(input_archive);
	
	if (code != ARCHIVE_OK) {
		err = -1;
		goto end;
	}
	
	if (callback == NULL) {
		output_archive = archive_write_disk_new();
		
		if (output_archive == NULL) {
			err = -1;
			goto end;
		}
	}
	
	if (output_archive != NULL) {
		code = archive_write_disk_set_options(output_archive, flags);
		
		if (code != ARCHIVE_OK) {
			err = -1;
			goto end;
		}
	}
	
	if (size > 0) {
		code = archive_read_open_memory(input_archive, source, size);
	} else {
		code = archive_read_open_filename(input_archive, source, ARCHIVE_BLOCK_SIZE);
	}
	
	if (code != ARCHIVE_OK) {
		err = -1;
		goto end;
	}
	
	while (1) {
		code = archive_read_next_header(input_archive, &entry);
		
		if (code == ARCHIVE_EOF) {
			code = ARCHIVE_OK;
			break;
		}
		
		if (code != ARCHIVE_OK) {
			err = -1;
			goto end;
		}
		
		if (entries != NULL) {
			pathname = archive_entry_pathname(entry);
			err = entries_append(entries, pathname);
			
			if (err != 0) {
				err = -1;
				goto end;
			}
		}
		
		if (output_archive != NULL) {
			code = archive_write_header(output_archive, entry);
			
			if (code != ARCHIVE_OK) {
				continue;
			}
		}
		
		while (1) {
			code = archive_read_data_block(input_archive, &chunk, &rsize, &offset);
			
			if (code == ARCHIVE_EOF) {
				code = ARCHIVE_OK;
				break;
			}
			
			if (code != ARCHIVE_OK) {
				err = -1;
				goto end;
			}
			
			if (callback != NULL) {
				err = (*callback)((char*) chunk, rsize, sizeof(char), callback_data);
				
				if (err != (int) rsize) {
					err = -1;
					goto end;
				}
				
				err = 0;
			}
			
			if (output_archive != NULL) {
				code = archive_write_data_block(output_archive, chunk, rsize, offset);
				
				if (code != ARCHIVE_OK) {
					err = -1;
					goto end;
				}
			}
		}
	}
	
	end:;
	
	if (err != 0) {
		fprintf(stderr, "uncompress(): %s\n", archive_error_string(input_archive));
	}
	
	archive_read_close(input_archive);
	archive_read_free(input_archive);
	
	if (output_archive != NULL) {
		archive_write_close(output_archive);
		archive_write_free(output_archive);
	}
	
	return err;
	
}

void archive_entries_free(archive_entries_t* const entries) {
	
	size_t index = 0;
	char* value = NULL;
	
	for (index = 0; index < entries->offset; index++) {
		value = entries->items[index];
		free(value);
	}
	
	free(entries->items);
	entries->items = NULL;
	
	entries->size = 0;
	entries->offset = 0;
	
}
