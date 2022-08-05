#include "gegham.h"

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
	char *name;
	sheet_t *src;
	header_cell_t *header_cells;
	size_t num_selected;
	header_cell_t *pk;
} table_t;


table_t *table_src_import(sheet_t *input, const char *tbl_name)
{
	table_t *to_ret;
	if (!input || !tbl_name) {
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

	to_ret->name = strdup(tbl_name);


	return to_ret;
}

int table_destroy(table_t *obj)
{
	if (obj) {
		free(obj->header_cells);
		free(obj->name);
		free(obj);
	}
	return 0;
}

header_type_t column_type(table_t *to_check, size_t index)
{
	header_type_t to_ret = TYPE_NONE;
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
	
	if (to_ret & TYPE_TEXT || !to_ret) {
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
		return 1;
	}

	return 0;
}

int table_select_all_columns(table_t *to_mod, ssize_t pk_index)
{
	int rc = 0;
	for (to_mod->num_selected = 0; to_mod->num_selected < 
			to_mod->src->num_cols; ) {
		if (pk_index == to_mod->num_selected) {
			rc = table_select_column(to_mod, 
				to_mod->num_selected, true);
		} else {
			rc = table_select_column(to_mod,
				to_mod->num_selected, false);
		}
		if (rc < 0) {
			break;
		}
	}

	return rc;
}

int table_select_column_name(table_t *to_mod, const char *name)
{
	size_t i;
	char *src_comp;
	for (i = 0; i < to_mod->src->num_cols; i++) {
		src_comp = to_mod->src->cells[i].content;
		if (!src_comp) {
			continue;
		}
		if (!strcmp(name, src_comp)) {
			return table_select_column(to_mod, i, false);
		}
	}
	return -1;
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

int column_build(cell_t *cell, header_type_t type, char *ext_buffer)
{
	char buffer[64];
	switch (type) {
		case TYPE_NONE:
		case TYPE_TEXT:
			snprintf(buffer, 64, "\'%s\' TEXT", cell->content);
			break;
		case TYPE_INT:
			snprintf(buffer, 64, "\'%s\' INTEGER", cell->content);
			break;
		case TYPE_REAL:
			snprintf(buffer, 64, "\'%s\' REAL", cell->content);
			break;
		default:
			return -1;
	}
	strcat(ext_buffer, buffer);
	return 0;
}

int table_build(table_t *to_mod, sqlite3 *db)
{
	char buffer[2048];
	sqlite3_stmt *stmt;
	header_cell_t *to_use;
	cell_t *cur;

	if (!to_mod || !db) {
		return -1;
	}

	if (!to_mod->num_selected) {
		return -2;
	}

	snprintf(buffer, 2048, "create table if not exists \'%s\' (", 
		to_mod->name);
	for (size_t i = 0; i < to_mod->num_selected; i++) {
		to_use = &to_mod->header_cells[i];
		cur = &to_mod->src->cells[to_use->index];
		if (column_build(cur, to_use->type, buffer)) {
			return -3;
		}
		if (i + 1 < to_mod->num_selected) {
			strcat(buffer, ", ");
		}
	}
	if (to_mod->pk) {
		strcat(buffer, ", primary key (");
		strcat(buffer, to_mod->src->cells[to_mod->pk->index].content);
		strcat(buffer, ")");
	}

	strcat(buffer, ");");
	printf("sql stmt: %s\n", buffer);

	return sqlite3_exec(db, buffer, 0x0, 0x0, 0x0);
}

int table_migrate(table_t *to_op, sqlite3 *db)
{
	size_t cur_row, cur_col;
	sqlite3_stmt *stmt;
	int rc = 0;
	char buffer[512];
	cell_t *cur_cell;
	cell_t *row_base;

	snprintf(buffer, 512, "insert into %s values (", to_op->name);
	for (cur_col = 0; cur_col < to_op->num_selected; cur_col++) {
		strcat(buffer, "?");
		if (cur_col + 1 < to_op->num_selected) {
			strcat(buffer, ", ");
		}
	}
	strcat(buffer, ");");
	if (sqlite3_prepare(db, buffer, -1, &stmt, 0x0)) {
		return -1;
	}

	for (cur_row = 1; cur_row < to_op->src->num_rows; cur_row++) {
		row_base = &to_op->src->cells[cur_row * to_op->src->num_cols];
		for (cur_col = 0; cur_col < to_op->num_selected; cur_col++) {
			size_t src_index = to_op->header_cells[cur_col].index;
			cur_cell = &row_base[src_index];
			rc = sqlite3_bind_text(stmt, cur_col + 1, 
				cur_cell->content, -1, 0x0);
			if (rc != SQLITE_OK) {
				goto exit;
			}
		}

		rc = sqlite3_step(stmt);
		if (rc == SQLITE_DONE) {
			rc = 0;
		} else {
			goto exit;
		}
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
	}

exit:
	sqlite3_finalize(stmt);
	return rc;
}