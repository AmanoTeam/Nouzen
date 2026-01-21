#include "fs/fstream.h"
#include "package.h"
#include "wcurl.h"

struct PkgDownload {
	wcurl_t wcurl;
	wcurl_error_t error;
	fstream_t* stream;
	size_t retries;
};

typedef struct PkgDownload pkgdl_t;

struct PkgDownloader {
	size_t offset;
	size_t size;
	pkgdl_t* items;
};

typedef struct PkgDownloader downloader_t;

typedef void (*progress_callback_t)(size_t, size_t);

struct DownloadOptions {
	size_t concurrency;
	size_t retry;
	char* temporary_directory;
	progress_callback_t progress_callback;
};

typedef struct DownloadOptions dlopts_t;

int downloader_add(
	downloader_t* const downloader,
	const dlopts_t* const options,
	pkg_t* const pkg
);

int downloader_wait(
	downloader_t* const downloader,
	const dlopts_t* const options
);

void downloader_free(downloader_t* const downloader);

void dlopts_free(dlopts_t* const options);
