#include <stdbool.h>
#include <stdio.h>

extern bool application_init();

int main() {
	printf("Initializing engine\n");

	application_init();

	printf("Exiting engine\n");

	return 0;
}
