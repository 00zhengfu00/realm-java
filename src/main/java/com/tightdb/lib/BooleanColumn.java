package com.tightdb.lib;

import com.tightdb.TableBase;

public class BooleanColumn<Cursor, Query> extends AbstractColumn<Boolean, Cursor, Query> {

	public BooleanColumn(TableBase table, AbstractCursor<Cursor> cursor, int index, String name) {
		super(table, cursor, index, name);
	}

	public BooleanColumn(TableBase table, int index, String name) {
		super(table, index, name);
	}

	@Override
	public Boolean get() {
		return table.getBoolean(columnIndex, (int) cursor.getPosition());
	}

	@Override
	public void set(Boolean value) {
		table.setBoolean(columnIndex, (int) cursor.getPosition(), value);
	}

}
