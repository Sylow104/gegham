#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#include "helgarahi.h"
#include <sqlite3.h>

typedef struct table table_t;

table_t *table_src_import(sheet_t *input);
int table_destroy(table_t *obj);
int table_select_column(table_t *to_mod, size_t index, bool is_pk);
int table_select_all_columns(table_t *to_mod, ssize_t pk_index);
int table_select_column_name(table_t *to_mod, const char *name);
int table_has_header(table_t *to_mod, bool option);
int table_build(table_t *to_mod, sqlite3 *db, const char *tbl_name);
int table_migrate(table_t *to_mod, sqlite3 *db, const char *tbl_name);

#ifdef __cplusplus
}
#endif