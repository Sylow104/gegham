#include "gegham.h"

int main(int argc, const char *argv[])
{
	xlsx_t *file = xlsx_open(argv[1]);
	if (!file) {
		exit(-1);
	}
	workbook_generate(file);
	// main procedures here

	// cleanup here
	return xlsx_close(file);
}