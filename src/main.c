#include "helgarahi.h"

int main(int argc, const char *argv[])
{
	xlsx_t *file = xlsx_open(argv[1]);
	if (!file) {
		exit(-1);
	}

	return xlsx_close(file);
}