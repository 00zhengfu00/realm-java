package com.tightdb.lib;

import com.tightdb.TableQuery;

public class BooleanQueryColumn<Cursor, View, Query> extends AbstractColumn<Boolean, Cursor, View, Query> {

	public BooleanQueryColumn(EntityTypes<?, View, Cursor, Query> types, IRowsetBase rowset, TableQuery query, int index, String name) {
		super(types, rowset, query, index, name);
	}

	public Query equal(boolean value) {
		return query(getQuery().equal(columnIndex, value));
	}
	public Query eq(boolean value) {
		return query(getQuery().equal(columnIndex, value));
	}

	public Query notEqual(boolean value) {
		return query(getQuery().equal(columnIndex, !value));
	}
	public Query neq(boolean value) {
		return query(getQuery().equal(columnIndex, !value));
	}
}
