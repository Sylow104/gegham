#include "gegham.h"


table_t *table_src_import(sheet_t *input)
{
	table_t *to_ret;
	if (input) {
		return 0x0;
	}
	
	to_ret = (table_t *) malloc(sizeof(table_t));
	if (!to_ret) {
		return to_ret;
	}
	to_ret->header_cells = calloc(input->num_cols, sizeof(header_cell_t));
	if (!to_ret->header_cells) {
		free(to_ret);
		return 0x0;
	}

	to_ret->src = input;

	to_ret->num_selected = 0;
	to_ret->pk = 0x0;

	return to_ret;
}

header_type_t column_type(table_t *to_check, size_t index)
{
	header_type_t to_ret = TYPE_UNK;
	cell_t *cur;
	size_t *n_cols = &to_check->src->num_cols;
	size_t *n_rows = &to_check->src->num_rows;
	cell_t *start = to_check->src->cells;
	double dbl_result;
	uint64_t uint_result;
	char *tail;
	if (!to_check) {
		return to_ret;
	}
	
	cur = &start[*n_cols + index];
	for (size_t i_row = 1; i_row < *n_rows; i_row++, cur += *n_cols) {
		if (!cur->content) {
			to_ret |= TYPE_NONE;
			continue;
		}
		uint_result = strtol(cur->content, &tail, 0);
		if (!*tail) {
			to_ret |= TYPE_INT;
			continue;
		}

		dbl_result = strtold(cur->content, &tail);
		if (!*tail) {
			to_ret |= TYPE_REAL;
			continue;
		}
		to_ret |= TYPE_TEXT;
	}
	
	if (to_ret & TYPE_TEXT && to_ret & 0b111) {
		to_ret = TYPE_TEXT;
	}

	return to_ret;
}

int table_select_column(table_t *to_mod, size_t index, bool is_pk)
{
	if (index >= to_mod->src->num_cols || index < 0) {
		return -1;
	}
	if (to_mod->num_selected >= to_mod->src->num_cols || 
			to_mod->num_selected < 0) {
		return -2;
	}
	header_cell_t *target = &to_mod->header_cells[to_mod->num_selected++];

	target->index = index;
	target->type = column_type(to_mod, index);
	if (is_pk && !to_mod->pk) {
		to_mod->pk = target;
	} else {
		return -10;
	}

	return 0;
}

int table_has_header(table_t *to_mod, bool option)
{
	if (!to_mod) {
		return -1;
	}
	// TODO implement header and headerless tables, 
	// indexed by numbers instead.
	//to_mod->has_header = option;
	return 0;
}

int table_build(table_t *to_mod, sqlite3 *db, const char *tbl_name)
{
	char buffer[2048];
	sqlite3_stmt *stmt;
	if (!to_mod || !db) {
		return -1;
	}

	if (!to_mod->num_selected) {
		return -2;
	}

	snprintf(buffer, 2048, "create table if not exists %s (", tbl_name);

	return sqlite3_exec(db, buffer, 0x0, 0x0, 0x0);
}