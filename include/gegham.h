#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#include "helgarahi.h"
#include <sqlite3.h>

typedef enum header_type
{
	TYPE_NONE,
	TYPE_TEXT = 0b1,
	TYPE_INT = 0b10,
	TYPE_REAL = 0b100,
} header_type_t;

typedef struct header_cell
{
	size_t index;
	header_type_t type;
} header_cell_t;


typedef struct table
{
	sheet_t *src;
	header_cell_t *header_cells;
	size_t num_selected;
	header_cell_t *pk;
	//bool has_header;
	size_t cursor;
} table_t;
table_t *table_src_import(sheet_t *input);
int table_destroy(table_t *obj);
int table_select_column(table_t *to_mod, size_t index, bool is_pk);
int table_select_all_columns(table_t *to_mod, ssize_t pk_index);
int table_has_header(table_t *to_mod, bool option);
int table_build(table_t *to_mod, sqlite3 *db, const char *tbl_name);
int table_migrate(table_t *to_mod, sqlite3 *db, const char *tbl_name);

#ifdef __cplusplus
}
#endif