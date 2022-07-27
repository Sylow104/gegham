#include <helgarahi/xlsx.hpp>

int main(int argc, const char *argv[])
{
	xlsx file;
	sqlite3 *db;
	file.open(argv[1]);
	file.analyze();
	if (sqlite3_open("raw_db.db", &db)) {
		exit(-1);
	}
	
	table *delivery_info = file.get_table("delivery_info", 1);
	table *package_info = file.get_table("package_info", 2);

	delivery_info->add_all();
	delivery_info->import_schema(db);
	delivery_info->import_data(db);

	package_info->add_all();
	package_info->import_schema(db);
	package_info->import_data(db);

	return sqlite3_close(db);
}