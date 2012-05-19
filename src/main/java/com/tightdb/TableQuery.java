package com.tightdb;


public class TableQuery {
	public TableQuery(){
		this.nativePtr = createNativePtr();
	}
	
	// Query for integer values.
	public TableQuery equal(long columnIndex, long value){
		nativeEqual(nativePtr, columnIndex, value);
		return this;
	}
	
	protected native void nativeEqual(long nativeQueryPtr, long columnIndex, long value);
	
	public TableQuery notEqual(long columnIndex, long value){
		nativeNotEqual(nativePtr, columnIndex, value);
		return this;
	}
	
	protected native void nativeNotEqual(long nativeQueryPtr, long columnIndex, long value);
		
	public TableQuery greater(long columnIndex, long value){
		nativeGreater(nativePtr, columnIndex, value);
		return this;
	}
	protected native void nativeGreater(long nativeQueryPtr, long columnIndex, long value);
	
	public TableQuery greaterEqual(long columnIndex, long value){
		nativeGreaterEqual(nativePtr, columnIndex, value);
		return this;
	}
	protected native void nativeGreaterEqual(long nativeQueryPtr, long columnIndex, long value);

	public TableQuery less(long columnIndex, long value){
		nativeLess(nativePtr, columnIndex, value);
		return this;
	}
	protected native void nativeLess(long nativeQueryPtr, long columnIndex, long value);
	
	public TableQuery lessEqual(long columnIndex, long value){
		nativeLessEqual(nativePtr, columnIndex, value);
		return this;
	}
	protected native void nativeLessEqual(long nativeQueryPtr, long columnIndex, long value);
	
	public TableQuery between(long columnIndex, long value1, long value2){
		nativeBetween(nativePtr, columnIndex, value1, value2);
		return this;
	}
	protected native void nativeBetween(long nativeQueryPtr, long columnIndex, long value1, long value2);
	
	// Query for boolean values.
	public TableQuery equal(long columnIndex, boolean value){
		nativeEqual(nativePtr, columnIndex, value);
		return this;
	}
	protected native void nativeEqual(long nativeQueryPtr, long columnIndex, boolean value);
	
	// Query for String values.
	public TableQuery equal(long columnIndex, String value, boolean caseSensitive){
		nativeEqual(nativePtr, columnIndex, value, caseSensitive);
		return this;
	}
	protected native void nativeEqual(long nativeQueryPtr, long columnIndex, String value, boolean caseSensitive);
	
	public TableQuery equal(long columnIndex, String value){
		return equal(columnIndex, value, true);
	}
	
	public TableQuery beginsWith(long columnIndex, String value, boolean caseSensitive){
		nativeBeginsWith(nativePtr, columnIndex, value, caseSensitive);
		return this;
	}
	protected native void nativeBeginsWith(long nativeQueryPtr, long columnIndex, String value, boolean caseSensitive);
	
	
	public TableQuery beginsWith(long columnIndex, String value){
		return beginsWith(columnIndex, value, true);
	}
	
	public TableQuery endsWith(long columnIndex, String value, boolean caseSensitive){
		nativeEndsWith(nativePtr, columnIndex, value, caseSensitive);
		return this;
	}
	protected native void nativeEndsWith(long nativeQueryPtr, long columnIndex, String value, boolean caseSensitive);
	
	public TableQuery endsWith(long columnIndex, String value){
		return endsWith(columnIndex, value, true);
	}
	
	public TableQuery contains(long columnIndex, String value, boolean caseSensitive){
		nativeContains(nativePtr, columnIndex, value, caseSensitive);
		return this;
	}
	
	protected native void nativeContains(long nativeQueryPtr, long columnIndex, String value, boolean caseSensitive);
	
	public TableQuery contains(long columnIndex, String value){
		return contains(columnIndex, value, true);
	}

	public TableQuery notEqual(long columnIndex, String value, boolean caseSensitive){
		nativeNotEqual(nativePtr, columnIndex, value, caseSensitive);
		return this;
	}
	protected native void nativeNotEqual(long nativeQueryPtr, long columnIndex, String value, boolean caseSensitive);
	
	public TableQuery notEqual(long columnIndex, String value){
		return notEqual(columnIndex, value, true);
	}

	// Grouping
	public void group(){
		nativeGroup(nativePtr);
	}
	
	protected native void nativeGroup(long nativeQueryPtr);
	
	public void endGroup(){
		nativeEndGroup(nativePtr);
	}
	protected native void nativeEndGroup(long nativeQueryPtr);
	
	public void subTable(long columnIndex){
		nativeSubTable(nativePtr, columnIndex);
	}
	
	protected native void nativeSubTable(long nativeQueryPtr, long columnIndex);

	public void parent(){
		nativeParent(nativePtr);
	}
	
	public native void nativeParent(long nativeQueryPtr);

	public void or(){
		nativeOr(nativePtr);
	}
	
	protected native void nativeOr(long nativeQueryPtr);

	// Searching methods.
	public long findNext(TableBase table, long lastMatch){
		return nativeFindNext(nativePtr, table, lastMatch);
	}
	
	protected native long nativeFindNext(long nativeQueryPtr, TableBase table, long lastMatch);
	
	public long findNext(TableBase table){
		return findNext(table, -1L);
	}
	
	public TableViewBase findAll(TableBase tableBase, long start, long end, long limit){
		return new TableViewBase(tableBase, nativeFindAll(nativePtr, tableBase, start, end, limit));
	}
	
	protected native long nativeFindAll(long nativeQueryPtr, TableBase tableBase, long start, long end, long limit);
	
	public TableViewBase findAll(TableBase tableBase){
		return findAll(tableBase, 0L, -1L, -1L);
	}
	
	public long sum(TableBase table, long columnIndex, long start, long end, long limit){
		return nativeSum(nativePtr, table, columnIndex, start, end, limit);
	}
	
	protected native long nativeSum(long nativeQueryPtr, TableBase table, long columnIndex, long start, long end, long limit);
	
	public long sum(TableBase tableBase, long columnIndex){
		return sum(tableBase, columnIndex, 0L, -1L, -1L);
	}
	
	public long maximum(TableBase table, long columnIndex, long start, long end, long limit){
		return nativeMaximum(nativePtr, table, columnIndex, start, end, limit);
	}
	
	protected native long nativeMaximum(long nativeQueryPtr, TableBase table, long columnIndex, long start, long end, long limit);
	
	public long maximum(TableBase tableBase, long columnIndex){
		return maximum(tableBase, columnIndex, 0L, -1L, -1L);
	}
	
	public long minimum(TableBase table, long columnIndex, long start, long end, long limit){
		return nativeMinimum(nativePtr, table, columnIndex, start, end, limit);
	}
	
	protected native long nativeMinimum(long nativeQueryPtr, TableBase table, long columnIndex, long start, long end, long limit);
	
	public long minimum(TableBase tableBase, long columnIndex){
		return minimum(tableBase, columnIndex, 0L, -1L, -1L);
	}
	
	public double average(TableBase table, long columnIndex, long start, long end, long limit){
		return nativeAverage(nativePtr, table, columnIndex, start, end, limit);
	}
	
	protected native double nativeAverage(long nativeQueryPtr, TableBase table, long columnIndex, long start, long end, long limit);

	public double average(TableBase tableBase, long columnIndex){
		return average(tableBase, columnIndex, 0L, -1L, -1L);
	}
	
	public long count(TableBase table, long start, long end, long limit){
		return nativeCount(nativePtr, table, start, end, limit);
	}
	
	protected native long nativeCount(long nativeQueryPtr, TableBase table, long start, long end, long limit);

	public long count(TableBase tableBase){
		return count(tableBase, 0L, -1L, -1L);
	}
	
	// Deletion.
	public void remove(TableBase tableBase, long start, long end, long limit){
		nativeRemove(nativePtr, tableBase, start, end, limit);
	}
	
	public void remove(TableBase tableBase){
		remove(tableBase, 0L, -1L, -1L);
	}
	
	protected native void nativeRemove(long nativeQueryPtr, TableBase tableBase, long start, long end, long limit);
	
	public String getErrorCode(){
		return nativeGetErrorCode(nativePtr);
	}
	
	protected native String nativeGetErrorCode(long nativePtr);
	
	protected native long createNativePtr();
	
	protected long nativePtr;
}
