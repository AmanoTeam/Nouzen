#include <stdio.h>

#include "term/keyboard.h"
#include "ask.h"

int ask(void) {
	
	int answer = ASK_ANSWER_YES;
	
	cir_t cir = {0};
	
	const cir_key_t* key = NULL;
	
	printf("Do you want to continue? [Y/n] ");
	fflush(stdout);
	
	cir_init(&cir);
	
	while (1) {
		key = cir_get(&cir);
		
		switch (key->type) {
			case KEY_CTRL_C:
			case KEY_CTRL_D:
			case KEY_CTRL_BACKSLASH: {
				answer = ASK_ANSWER_INTERRUPTED;
				goto end;
			}
			case KEY_Y: {
				printf("%s", key->name);
				goto end;
			}
			case KEY_N: {
				printf("%s", key->name);
				answer = ASK_ANSWER_NO;
				goto end;
			}
			case KEY_ENTER: {
				printf("%s", "y");
				goto end;
			}
			default: {
				break;
			}
		}
	}
	
	end:;
	
	fflush(stdout);
	printf("%s", "\r\n");
	
	cir_free(&cir);
	
	if (answer != ASK_ANSWER_YES) {
		fprintf(stderr, "Abort.\n");
	}
	
	return answer;
	
}
