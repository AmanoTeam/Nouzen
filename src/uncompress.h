struct ArchiveEntries {
	size_t size;
	size_t offset;
	char** items;
};

typedef struct ArchiveEntries archive_entries_t;

typedef size_t (*uncompress_callback_t)(char*, size_t, size_t, void*);

int uncompress(
	const char* const source,
	const size_t size,
	uncompress_callback_t callback,
	void* const callback_data,
	archive_entries_t* const entries
);

void archive_entries_free(archive_entries_t* const entries);
