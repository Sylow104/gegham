#include "gegham.h"

// args: 
// 1 - xlsx file
// 2 - sqlite3 db file
// 3 - table name to inject

int main(int argc, const char *argv[])
{
	sqlite3 *db;
	xlsx_t *file;
	if (argc != 4) {
		exit(1);
	}
	
	file = xlsx_open(argv[1]);
	if (!file) {
		exit(-1);
	}
	if (sqlite3_open(argv[2], &db)) {
		exit(-2);
	}
	// main procedures here
	table_t *to_op = table_src_import(&file->workbook.sheets[0], argv[3]);
	table_select_all_columns(to_op, -1);
	table_build(to_op, db);
	int rc = table_migrate(to_op, db);
	if (rc) {
		printf("Sqlite Err: %s\n", sqlite3_errmsg(db));
	}

	// cleanup here
	table_destroy(to_op);
	sqlite3_close(db);
	return xlsx_close(file);
}