#include "gegham.h"

int main(int argc, const char *argv[])
{
	xlsx_t *file = xlsx_open(argv[1]);
	if (!file) {
		exit(-1);
	}
	// main procedures here
	table_t *to_op = table_src_import(&file->workbook.sheets[0]);
	table_select_all_columns(to_op, -1);
	table_build(to_op, 0x0, "fish");

	// cleanup here
	table_destroy(to_op);
	return xlsx_close(file);
}