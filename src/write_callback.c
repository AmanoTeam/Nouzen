#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "repository.h"
#include "logging.h"
#include "buffer.h"
#include "fs/fstream.h"
#include "write_callback.h"

size_t write_string_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
	
	buffer_t* buffer = userdata;
	
	const size_t chunk_size = size * nmemb;
	
	if (buffer_append(buffer, ptr, chunk_size) != 0) {
		loggln(
			LOG_ERROR,
			"Exceeded max repository index size (%zu > %zu)\n",
			(buffer->offset + chunk_size),
			(buffer->size - 1)
		);
		
		return 0;
	}
	
	return chunk_size;
	
}

size_t write_file_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
	
	int status = FSTREAM_SUCCESS;
	
	fstream_t* stream = userdata;
	const size_t chunk_size = size * nmemb;
	
	status = fstream_write(stream, ptr, chunk_size);
	
	if (status != FSTREAM_SUCCESS) {
		return 0;
	}
	
	return chunk_size;
	
}