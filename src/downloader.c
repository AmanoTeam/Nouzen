#include <stddef.h>
#include <string.h>

#include <curl/curl.h>

#include "downloader.h"
#include "terminal.h"
#include "fstream.h"
#include "wcurl.h"
#include "errors.h"
#include "fs/sep.h"
#include "write_callback.h"

static const char DEB_FILE_EXT[] = ".deb";

int downloader_append(
	downloader_t* const downloader,
	pkgdl_t* download
) {
	
	size_t size = 0;
	
	pkgdl_t* item = download;
	pkgdl_t* items = NULL;
	
	if (sizeof(*downloader->items) * (downloader->offset + 1) > downloader->size) {
		size = downloader->size + sizeof(*downloader->items) * (downloader->offset + 1);
		items = realloc(downloader->items, size);
		
		if (items == NULL) {
			return APTERR_MEM_ALLOC_FAILURE;
		}
		
		downloader->size = size;
		downloader->items = items;
	}
	
	downloader->items[downloader->offset++] = *item;
	
	return APTERR_SUCCESS;
	
}

int downloader_add(
	downloader_t* const downloader,
	const dlopts_t* const options,
	pkg_t* const pkg
) {
	
	char* value = NULL; 
	
	int err = APTERR_SUCCESS;
	
	CURLcode code = CURLE_OK;
	
	pkgdl_t download = {0};
	
	fstream_t* stream = NULL;
	
	wcurl_t* wcurl_global = NULL;
	wcurl_t* wcurl = &download.wcurl;
	wcurl_error_t* error = &download.error;
	
	CURL* curl = NULL;
	
	if (!(pkg->upgradable || !pkg->installed)) {
		goto end;
	}
	
	wcurl_global = wcurl_getglobal();
	
	if (wcurl_global == NULL) {
		err = APTERR_WCURL_INIT_FAILURE;
		goto end;
	}
	
	err = wcurl_duplicate(wcurl_global, wcurl);
	
	if (err != WCURL_ERR_SUCCESS) {
		err = APTERR_WCURL_INIT_FAILURE;
		goto end;
	}
	
	curl = wcurl_getcurl(wcurl);
	
	code = curl_easy_setopt(curl, CURLOPT_URL, pkg->filename);
	
	if (code != CURLE_OK) {
		err = APTERR_WCURL_SETOPT_FAILURE;
		goto end;
	}
	
	free(pkg->filename);
	
	pkg->filename = malloc(
		strlen(options->temporary_directory) +
		strlen(PATHSEP_S) +
		strlen(pkg->name) +
		strlen(DEB_FILE_EXT) +
		1
	);
	
	if (pkg->filename == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(pkg->filename, options->temporary_directory);
	strcat(pkg->filename, PATHSEP_S);
	strcat(pkg->filename, pkg->name);
	strcat(pkg->filename, DEB_FILE_EXT);
	
	stream = fstream_open(pkg->filename, FSTREAM_WRITE);
	
	if (stream == NULL) {
		err = APTERR_FSTREAM_OPEN_FAILURE;
		goto end;
	}
	
	download.stream = stream;
	
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file_cb);
	
	if (code != CURLE_OK) {
		err = APTERR_WCURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);
	
	if (code != CURLE_OK) {
		err = APTERR_WCURL_SETOPT_FAILURE;
		goto end;
	}
	
	error->msg = malloc(CURL_ERROR_SIZE);
	
	if (error->msg == NULL) {
		err = APTERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error->msg);
	
	if (code != CURLE_OK) {
		err = APTERR_WCURL_SETOPT_FAILURE;
		goto end;
	}
	
	wcurl->retry = options->retry;
	
	err = downloader_append(downloader, &download);
	
	if (err != APTERR_SUCCESS) {
		goto end;
	}
	
	end:;
	
	free(value);
	
	if (err != APTERR_SUCCESS) {
		fstream_close(stream);
	}
	
	return err;
	
}

int downloader_wait(
	downloader_t* const downloader,
	const dlopts_t* const options
) {
	
	int err = APTERR_SUCCESS;
	
	CURLMcode code = CURLM_OK;
	
	size_t index = 0;
	size_t current = 0;
	size_t total = 0;
	
	int running = 1;
	CURLMsg* msg = NULL;
	int left = 0;
	
	int status = 0;
	
	pkgdl_t* download = NULL;
	
	wcurl_multi_t* wcurl_multi = NULL;
	wcurl_error_t* wcurl_error = NULL;
	
	CURL* curl = NULL;
	CURLM* curl_multi = NULL;
	
	if (downloader->offset < 1) {
		goto end;
	}
	
	wcurl_multi = wcurlmlt_getglobal(options->concurrency);
	
	if (wcurl_multi == NULL) {
		err = APTERR_WCURLMLT_INIT_FAILURE;
		goto end;
	}
	
	curl_multi = wcurlmlt_getcurl(wcurl_multi);
	
	curl = wcurl_getglobal();
	
	if (curl == NULL) {
		err = APTERR_WCURL_INIT_FAILURE;
		goto end;
	}
	
	wcurl_error = wcurl_geterr(curl);
	
	for (index = 0; index < downloader->offset; index++) {
		download = &downloader->items[index];
		curl = wcurl_getcurl(&download->wcurl);
		
		code = curl_multi_add_handle(curl_multi, curl);
		
		if (code != CURLM_OK) {
			err = APTERR_WCURLMLT_ADD_FAILURE;
			goto end;
		}
		
		total += 1;
	}
	
	if (options->progress_callback != NULL) {
		(*options->progress_callback)(total, current);
	}
	
	while (running) {
		code = curl_multi_perform(curl_multi, &running);
		
		if (code != CURLM_OK) {
			err = APTERR_WCURLMLT_PERFORM_FAILURE;
			goto end;
		}
		
		if (running) {
			code = curl_multi_poll(curl_multi, NULL, 0, 0, NULL);
		}
		
		if (code != CURLM_OK) {
			err = APTERR_WCURLMLT_POLL_FAILURE;
			goto end;
		}
		
		while ((msg = curl_multi_info_read(curl_multi, &left)) != NULL) {
			if (msg->msg != CURLMSG_DONE) {
				continue;
			}
			
			download = NULL;
			
			for (index = 0; index < downloader->offset; index++) {
				download = &downloader->items[index];
				curl = wcurl_getcurl(&download->wcurl);
				
				if (curl == msg->easy_handle) {
					break;
				}
				
				download = NULL;
			}
			
			code = curl_multi_remove_handle(curl_multi, msg->easy_handle);
			
			if (code != CURLM_OK) {
				err = APTERR_WCURLMLT_REMOVE_FAILURE;
				goto end;
			}
			
			if (msg->data.result != CURLE_OK) {
				status = wcurl_retryable(msg->easy_handle, msg->data.result);
				
				if (download->retries++ > options->retry || !status) {
					/* Propagate the error to the global HTTP client so that we can retrieve it later */
					strcpy(wcurl_error->msg, download->error.msg);
					wcurl_error->code = msg->data.result;
					
					if (wcurl_error->msg[0] == '\0') {
						strcpy(wcurl_error->msg, curl_easy_strerror(wcurl_error->code));
					}
					
					err = APTERR_WCURL_REQUEST_FAILURE;
					goto end;
				}
				
				if (download->stream != NULL) {
					/*
					The download failed, but it is still retryable. Let's try again after
					discarding any partially downloaded data.
					*/
					status = fstream_seek(download->stream, 0, FSTREAM_SEEK_BEGIN);
					
					if (status != FSTREAM_SUCCESS) {
						err = APTERR_FSTREAM_SEEK_FAILURE;
						goto end;
					}
				}
				
				code = curl_multi_add_handle(curl_multi, msg->easy_handle);
				
				if (code != CURLM_OK) {
					err = APTERR_WCURLMLT_ADD_FAILURE;
					goto end;
				}
			} else {
				current++;
				
				if (options->progress_callback != NULL) {
					(*options->progress_callback)(total, current);
				}
				
				wcurl_free(&download->wcurl);
				
				fstream_close(download->stream);
				download->stream = NULL;
			}
		}
	}
	
	end:;
	
	erase_line();
	
	return err;
	
}

void downloader_free(downloader_t* const downloader) {
	
	size_t index = 0;
	pkgdl_t* download = NULL;
	
	for (index = 0; index < downloader->offset; index++) {
		download = &downloader->items[index];
		
		wcurl_free(&download->wcurl);
		wcurlerr_free(&download->error);
		fstream_close(download->stream);
		download->stream = NULL;
		download->retries = 0;
	}
	
	free(downloader->items);
	downloader->items = NULL;
	downloader->offset = 0;
	downloader->size = 0;
	
}

void dlopts_free(dlopts_t* const options) {
	
	free(options->temporary_directory);
	
	options->temporary_directory = NULL;
	options->concurrency = 0;
	options->retry = 0;
	options->progress_callback = NULL;
	
}