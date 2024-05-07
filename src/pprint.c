#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "package.h"
#include "terminal.h"
#include "pprint.h"

void pprint_packages(const pkgs_t* const indirect) {
	
	const pkg_t* pkg = NULL;
	
	size_t written = 0;
	size_t index = 0;
	size_t size = 0;
	size_t count = 0;
	
	const pkgs_t* pkgs = indirect;
	
	const size_t width = get_terminal_width() - 4;
	
	for (index = 0; index < pkgs->offset; index++) {
		pkg = pkgs->items[index];
		
		if (written == 0) {
			fprintf(stderr, "  ");
		}
		
		size = strlen(pkg->name) + 1;
		
		if ((written + size) > width && count > 0) {
			written = 0;
			count = 0;
			index--;
			
			fprintf(stderr, "  \n");
			
			continue;
		}
		
		fprintf(stderr, "%s ", pkg->name);
		
		count++;
		written += size;
	}
	
	fprintf(stderr, "\n");
	
}
