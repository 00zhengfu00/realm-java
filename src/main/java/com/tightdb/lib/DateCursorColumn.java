package com.tightdb.lib;

import java.text.DateFormat;
import java.util.Date;

import com.tightdb.TableBase;

public class DateCursorColumn<Cursor, Query> extends AbstractColumn<Date, Cursor, Query> {

	private static final DateFormat FORMATTER = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT);
	
	public DateCursorColumn(EntityTypes<?, ?, Cursor, Query> types, TableBase table, AbstractCursor<Cursor> cursor, int index, String name) {
		super(types, table, cursor, index, name);
	}

	@Override
	public Date get() {
		return new Date(table.getLong(columnIndex, (int) cursor.getPosition()));
	}

	@Override
	public void set(Date value) {
		table.setLong(columnIndex, (int) cursor.getPosition(), value.getTime());
	}

	@Override
	public String getReadable() {
		return FORMATTER.format(get());
	}
	
}
