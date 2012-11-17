package com.tightdb;

import java.util.ArrayList;
import java.util.List;

public class TableSpec {

	public static class ColumnInfo {
		
		protected final ColumnType type;
		protected final String name;
		protected final TableSpec tableSpec;

		public ColumnInfo(ColumnType type, String name) {
			this.name = name;
			this.type = type;
			this.tableSpec = (type == ColumnType.ColumnTypeTable) ? new TableSpec() : null;
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((name == null) ? 0 : name.hashCode());
			result = prime * result + ((tableSpec == null) ? 0 : tableSpec.hashCode());
			result = prime * result + ((type == null) ? 0 : type.hashCode());
			return result;
		}

		@Override
		public boolean equals(Object obj) {
			if (this == obj)
				return true;
			if (obj == null)
				return false;
			if (getClass() != obj.getClass())
				return false;
			ColumnInfo other = (ColumnInfo) obj;
			if (name == null) {
				if (other.name != null)
					return false;
			} else if (!name.equals(other.name))
				return false;
			if (tableSpec == null) {
				if (other.tableSpec != null)
					return false;
			} else if (!tableSpec.equals(other.tableSpec))
				return false;
			if (type != other.type)
				return false;
			return true;
		}
		
	}

	private List<ColumnInfo> columnInfos;

	public TableSpec() {
		columnInfos = new ArrayList<ColumnInfo>();
	}

	public void addColumn(ColumnType type, String name) {
		columnInfos.add(new ColumnInfo(type, name));
	}

	protected void addColumn(int colTypeIndex, String name) {
		ColumnType[] colTypes = ColumnType.values();
		addColumn(colTypes[colTypeIndex], name);
	}

	public TableSpec addSubtableColumn(String name) {
		ColumnInfo columnInfo = new ColumnInfo(ColumnType.ColumnTypeTable, name);
		columnInfos.add(columnInfo);
		return columnInfo.tableSpec;
	}

	public TableSpec getSubtableSpec(long columnIndex) {
		return columnInfos.get((int) columnIndex).tableSpec;
	}

	public long getColumnCount() {
		return columnInfos.size();
	}

	public ColumnType getColumnType(long columnIndex) {
		return columnInfos.get((int) columnIndex).type;
	}

	public String getColumnName(long columnIndex) {
		return columnInfos.get((int) columnIndex).name;
	}

	public long getColumnIndex(String name) {
		for (int i = 0; i < columnInfos.size(); i++) {
			ColumnInfo columnInfo = columnInfos.get(i);
			if (columnInfo.name.equals(name)) {
				return i;
			}
		}
		return -1;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((columnInfos == null) ? 0 : columnInfos.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		TableSpec other = (TableSpec) obj;
		if (columnInfos == null) {
			if (other.columnInfos != null)
				return false;
		} else if (!columnInfos.equals(other.columnInfos))
			return false;
		return true;
	}

}
