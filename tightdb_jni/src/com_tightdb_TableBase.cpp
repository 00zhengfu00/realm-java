#include "util.h"
#include "mixedutil.h"
#include "com_tightdb_TableBase.h"
#include "columntypeutil.h"
#include "TableSpecUtil.h"
#include "java_lang_List_Util.h"
#include "mixedutil.h"
#include "tablebase_tpl.hpp"

#include <sstream>

using namespace tightdb;


JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeUpdateFromSpec(
	JNIEnv* env, jobject, jlong nativeTablePtr, jobject jTableSpec)
{
    if (!TABLE_VALID(env, TBL(nativeTablePtr))) return;

	Table* pTable = TBL(nativeTablePtr);
    TR((env, "nativeUpdateFromSpec(tblPtr %x, spec %x)\n", pTable, jTableSpec));
    Spec& spec = pTable->get_spec();
	updateSpecFromJSpec(env, spec, jTableSpec);
	pTable->update_from_spec();
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeSize(
	JNIEnv* env, jobject, jlong nativeTablePtr)
{
    if (!TABLE_VALID(env, TBL(nativeTablePtr))) return 0;
    return TBL(nativeTablePtr)->size();
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeClear(
	JNIEnv* env, jobject, jlong nativeTablePtr)
{
    if (!TABLE_VALID(env, TBL(nativeTablePtr))) return;
	TBL(nativeTablePtr)->clear();
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeGetColumnCount(
	JNIEnv* env, jobject, jlong nativeTablePtr)
{
    if (!TABLE_VALID(env, TBL(nativeTablePtr))) return 0;
	return TBL(nativeTablePtr)->get_column_count();
}

JNIEXPORT jstring JNICALL Java_com_tightdb_TableBase_nativeGetColumnName(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex)
{
    if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return NULL;
	return env->NewStringUTF( TBL(nativeTablePtr)->get_column_name( S(columnIndex)) );
}

JNIEXPORT jobject JNICALL Java_com_tightdb_TableBase_nativeGetTableSpec(
	JNIEnv* env, jobject, jlong nativeTablePtr)
{
    if (!TABLE_VALID(env, TBL(nativeTablePtr))) return 0;

    TR((env, "nativeGetTableSpec(table %x)\n", nativeTablePtr));
	static jmethodID jTableSpecConsId = GetTableSpecMethodID(env, "<init>", "()V");
	if (jTableSpecConsId) {
        // Create a new TableSpec object in Java
        const Table* pTable = TBL(nativeTablePtr);
    	const Spec& tableSpec = pTable->get_spec();
        jobject jTableSpec = env->NewObject(GetClassTableSpec(env), jTableSpecConsId);	
	    if (jTableSpec) {
            // copy the c++ spec to the new java TableSpec
            UpdateJTableSpecFromSpec(env, tableSpec, jTableSpec);
            return jTableSpec;
        }
	}
    return NULL;
}

JNIEXPORT jint JNICALL Java_com_tightdb_TableBase_nativeGetColumnType(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex)
{
    if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return 0;

    return static_cast<int>( TBL(nativeTablePtr)->get_column_type( S(columnIndex)) );
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeAddEmptyRow(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong rows)
{
    if (!TABLE_VALID(env, TBL(nativeTablePtr))) return 0;

    return static_cast<jlong>( TBL(nativeTablePtr)->add_empty_row( S(rows)) );
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeRemove(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong rowIndex)
{
    if (!ROW_INDEX_VALID(env, TBL(nativeTablePtr), rowIndex)) return;

    TBL(nativeTablePtr)->remove(S(rowIndex));
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeRemoveLast(
	JNIEnv* env, jobject, jlong nativeTablePtr)
{
    if (!TABLE_VALID(env, TBL(nativeTablePtr))) return;

    TBL(nativeTablePtr)->remove_last();
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertLong(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jlong value)
{
	if (!INDEX_INSERT_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;
//TODO??? check type. Also in set*()

    TBL(nativeTablePtr)->insert_int( S(columnIndex), S(rowIndex), value);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertBoolean(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jboolean value)
{
	if (!INDEX_INSERT_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;
    
    TBL(nativeTablePtr)->insert_bool( S(columnIndex), S(rowIndex), value != 0 ? true : false);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertDate(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jlong dateTimeValue)
{
	if (!INDEX_INSERT_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

    TBL(nativeTablePtr)->insert_date( S(columnIndex), S(rowIndex), static_cast<time_t>(dateTimeValue));
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertString(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jstring value)
{
	if (!INDEX_INSERT_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

    const char* valueCharPtr = env->GetStringUTFChars(value, NULL);
    if (!valueCharPtr) 
        return;
	TBL(nativeTablePtr)->insert_string( S(columnIndex), S(rowIndex), valueCharPtr);
	env->ReleaseStringUTFChars(value, valueCharPtr);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertMixed(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jobject jMixedValue)
{ 
    if (!INDEX_INSERT_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

    tbl_nativeDoMixed(&Table::insert_mixed, TBL(nativeTablePtr), env, columnIndex, rowIndex, jMixedValue);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetMixed(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jobject jMixedValue)
{ 
    if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

    tbl_nativeDoMixed(&Table::set_mixed, TBL(nativeTablePtr), env, columnIndex, rowIndex, jMixedValue);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertSubTable(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	if (!INDEX_INSERT_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

    TR((env, "nativeInsertSubTable(jTable:%x, nativeTablePtr: %x, colIdx: %lld, rowIdx: %lld)\n",
       jTable, nativeTablePtr,  columnIndex, rowIndex));
	TBL(nativeTablePtr)->insert_subtable( S(columnIndex), S(rowIndex));
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertDone(
	JNIEnv* env, jobject, jlong nativeTablePtr)
{
    if (!TABLE_VALID(env, TBL(nativeTablePtr))) return;
    
    TBL(nativeTablePtr)->insert_done();
}


JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeGetLong(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return 0;

    return TBL(nativeTablePtr)->get_int( S(columnIndex), S(rowIndex));
}

JNIEXPORT jboolean JNICALL Java_com_tightdb_TableBase_nativeGetBoolean(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return false;

    return TBL(nativeTablePtr)->get_bool( S(columnIndex), S(rowIndex));
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeGetDateTime(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return 0;

    return TBL(nativeTablePtr)->get_date( S(columnIndex), S(rowIndex));
}

JNIEXPORT jstring JNICALL Java_com_tightdb_TableBase_nativeGetString(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return NULL;

	const char* valueCharPtr = TBL(nativeTablePtr)->get_string( S(columnIndex), S(rowIndex));
	return env->NewStringUTF(valueCharPtr);
}

JNIEXPORT jobject JNICALL Java_com_tightdb_TableBase_nativeGetByteBuffer(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return NULL;
    
    BinaryData data = TBL(nativeTablePtr)->get_binary( S(columnIndex), S(rowIndex));
	return env->NewDirectByteBuffer((void*)data.pointer, data.len);
}

JNIEXPORT jbyteArray JNICALL Java_com_tightdb_TableBase_nativeGetByteArray(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return NULL;

    return tbl_GetByteArray<Table>(env, nativeTablePtr, columnIndex, rowIndex);
}

JNIEXPORT jint JNICALL Java_com_tightdb_TableBase_nativeGetMixedType(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return 0;

	ColumnType mixedType = TBL(nativeTablePtr)->get_mixed_type( S(columnIndex), S(rowIndex));
	return static_cast<jint>(mixedType);
}

JNIEXPORT jobject JNICALL Java_com_tightdb_TableBase_nativeGetMixed(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
    if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return NULL;

	Mixed value = TBL(nativeTablePtr)->get_mixed( S(columnIndex), S(rowIndex));
	return CreateJMixedFromMixed(env, value);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeGetSubTable(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
    if (!INDEX_AND_TYPE_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex, COLUMN_TYPE_TABLE)) return 0;

	Table* pSubTable = static_cast<Table*>(LangBindHelper::get_subtable_ptr(TBL(nativeTablePtr), 
        S(columnIndex), S(rowIndex)));
    TR((env, "nativeGetSubTable(jTableBase:%x, nativeTablePtr: %x, colIdx: %lld, rowIdx: %lld) : %x\n",
        jTableBase, nativeTablePtr, columnIndex, rowIndex, pSubTable));
    return (jlong)pSubTable;
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeGetSubTableDuringInsert(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
    if (!INDEX_AND_TYPE_INSERT_VALID(env, TBL(nativeTablePtr), columnIndex, 
         rowIndex, COLUMN_TYPE_TABLE)) return 0;

	Table* pSubTable = static_cast<Table*>(LangBindHelper::get_subtable_ptr_during_insert(
        TBL(nativeTablePtr), S(columnIndex), S(rowIndex)));
    TR((env, "nativeGetSubTableDuringInsert(jTableBase:%x, nativeTablePtr: %x, colIdx: %lld, rowIdx: %lld) : %x\n",
        jTableBase, nativeTablePtr, columnIndex, rowIndex, pSubTable));
    return (jlong)pSubTable;
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeGetSubTableSize(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
    if (!INDEX_AND_TYPE_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex, COLUMN_TYPE_TABLE)) return 0;

    return TBL(nativeTablePtr)->get_subtable_size( S(columnIndex), S(rowIndex));
}


JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetString(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jstring value)
{
    if (!INDEX_AND_TYPE_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex, COLUMN_TYPE_STRING)) return;

    const char* valueCharPtr = env->GetStringUTFChars(value, NULL);
    if (valueCharPtr) {
	    TBL(nativeTablePtr)->set_string( S(columnIndex), S(rowIndex), valueCharPtr);
	    env->ReleaseStringUTFChars(value, valueCharPtr);
    }
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetLong(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jlong value)
{
    if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

	return TBL(nativeTablePtr)->set_int( S(columnIndex), S(rowIndex), value);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetBoolean(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jboolean value)
{
   	if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

    return TBL(nativeTablePtr)->set_bool( S(columnIndex), S(rowIndex), value == JNI_TRUE ? true : false);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetDate(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jlong dateTimeValue)
{
   	if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

    TBL(nativeTablePtr)->set_date( S(columnIndex), S(rowIndex), dateTimeValue);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetByteBuffer(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jobject byteBuffer)
{
    if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

    tbl_nativeDoBinary(&Table::set_binary, TBL(nativeTablePtr), env, columnIndex, rowIndex, byteBuffer);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertByteBuffer(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jobject byteBuffer)
{
    if (!INDEX_INSERT_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

    tbl_nativeDoBinary(&Table::insert_binary, TBL(nativeTablePtr), env, columnIndex, rowIndex, byteBuffer);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetByteArray(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jbyteArray dataArray)
{
    if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

    tbl_nativeDoByteArray(&Table::set_binary, TBL(nativeTablePtr), env, columnIndex, rowIndex, dataArray);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertByteArray(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jbyteArray dataArray)
{
    if (!INDEX_INSERT_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

    tbl_nativeDoByteArray(&Table::insert_binary, TBL(nativeTablePtr), env, columnIndex, rowIndex, dataArray);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeAddInt(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong value)
{	
	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return;

    TBL(nativeTablePtr)->add_int( S(columnIndex), value);
}


JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeClearSubTable(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
   	if (!INDEX_VALID(env, TBL(nativeTablePtr), columnIndex, rowIndex)) return;

    TBL(nativeTablePtr)->clear_subtable( S(columnIndex), S(rowIndex));
}

// Indexing methods:

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetIndex(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex)
{
    Table* pTable = TBL(nativeTablePtr);
   	if (!COL_INDEX_VALID(env, pTable, columnIndex)) 
        return;
    if (pTable->get_column_type(columnIndex) != COLUMN_TYPE_STRING) {
        ThrowException(env, IllegalArgument, "Invalid columntype - only string columns are supported.");
        return;
    }
    pTable->set_index( S(columnIndex));
}

JNIEXPORT jboolean JNICALL Java_com_tightdb_TableBase_nativeHasIndex(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex)
{
   	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return false;

    return TBL(nativeTablePtr)->has_index( S(columnIndex));
}

// Aggregare methods:

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeSum(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex)
{
   	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return 0;

    return TBL(nativeTablePtr)->sum( S(columnIndex));
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeMaximum(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex)
{	
   	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return 0;

    return TBL(nativeTablePtr)->maximum( S(columnIndex));
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeMinimum(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex)
{	
   	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return 0;

    return TBL(nativeTablePtr)->minimum( S(columnIndex));
}

JNIEXPORT jdouble JNICALL Java_com_tightdb_TableBase_nativeAverage(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex)
{
   	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return 0;
    return TBL(nativeTablePtr)->average( S(columnIndex));
}


JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeWhere(
    JNIEnv *, jobject, jlong nativeTablePtr)
{
    Query query = TBL(nativeTablePtr)->where();
    Query* queryPtr = new Query(query);
    return reinterpret_cast<jlong>(queryPtr);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindFirstInt(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong value)
{
   	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return 0;

	return TBL(nativeTablePtr)->find_first_int( S(columnIndex), value);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindFirstBool(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jboolean value)
{
   	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return false;

    return TBL(nativeTablePtr)->find_first_bool( S(columnIndex), value != 0 ? true : false);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindFirstDate(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong dateTimeValue)
{
   	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return 0;

	return TBL(nativeTablePtr)->find_first_date( S(columnIndex), (time_t)dateTimeValue);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindFirstString(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jstring value)
{
  	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return 0;

	const char* valueCharPtr = env->GetStringUTFChars(value, NULL);
    if (!valueCharPtr) 
        return 0;

	jlong result = TBL(nativeTablePtr)->find_first_string( S(columnIndex), valueCharPtr);
    env->ReleaseStringUTFChars(value, valueCharPtr);
	return result;
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindAllInt(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong value)
{
  	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return 0;

	TableView* pTableView = new TableView( TBL(nativeTablePtr)->find_all_int( S(columnIndex), value) );
	return reinterpret_cast<jlong>(pTableView);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindAllBool(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jboolean value)
{
  	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return 0;

	TableView* pTableView = new TableView( TBL(nativeTablePtr)->find_all_bool( S(columnIndex), 
                                           value != 0 ? true : false) );
	return reinterpret_cast<jlong>(pTableView);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindAllDate(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jlong dateTimeValue)
{
  	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return 0;

	TableView* pTableView = new TableView( TBL(nativeTablePtr)->find_all_date( S(columnIndex), 
                                           static_cast<time_t>(dateTimeValue)) );
	return reinterpret_cast<jlong>(pTableView);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindAllString(
	JNIEnv* env, jobject, jlong nativeTablePtr, jlong columnIndex, jstring value)
{
  	if (!COL_INDEX_VALID(env, TBL(nativeTablePtr), columnIndex)) return 0;

    Table* pTable = TBL(nativeTablePtr);
	const char* valueCharPtr = env->GetStringUTFChars(value, NULL);
    if (!valueCharPtr) 
        return 0;

	TableView* pTableView = new TableView( pTable->find_all_string( S(columnIndex), valueCharPtr) );
	return reinterpret_cast<jlong>(pTableView);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeDistinct(
    JNIEnv *env, jobject, jlong nativeTablePtr, jlong columnIndex)
{
    Table* pTable = TBL(nativeTablePtr);
    if (!COL_INDEX_VALID(env, pTable, columnIndex)) 
        return 0;
    if (!pTable->has_index(columnIndex)) {
        ThrowException(env, UnsupportedOperation, "The column must be indexed before distinct() can be used.");
        return 0;
    }
    if (pTable->get_column_type(columnIndex) != COLUMN_TYPE_STRING) {
        ThrowException(env, IllegalArgument, "Invalid columntype - only string columns are supported.");
        return 0;
    }
	TableView* pTableView = new TableView( pTable->distinct(S(columnIndex)) );
	return reinterpret_cast<jlong>(pTableView);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeOptimize(
	JNIEnv* env, jobject, jlong nativeTablePtr)
{
    if (!TABLE_VALID(env, TBL(nativeTablePtr))) return;
    
    TBL(nativeTablePtr)->optimize();
}

JNIEXPORT jstring JNICALL Java_com_tightdb_TableBase_nativeToJson(
    JNIEnv *env, jobject, jlong nativeTablePtr)
{
   Table* table = TBL(nativeTablePtr);
   if (!TABLE_VALID(env, table)) return NULL;

   // Write table to string in JSON format
   std::ostringstream ss;
   ss.sync_with_stdio(false); // for performance
   table->to_json(ss);
   const std::string str = ss.str();
   return env->NewStringUTF(str.c_str());
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeClose(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr)
{
    if (!TABLE_VALID(env, TBL(nativeTablePtr))) return;
    
    TR((env, "nativeClose(jTable: %x, nativeTablePtr: %x)\n", jTable, nativeTablePtr));
    LangBindHelper::unbind_table_ref(TBL(nativeTablePtr));
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_createNative(JNIEnv* env, jobject jTable)
{
    TR((env, "CreateNative(jTable: %x)\n", jTable));
    return reinterpret_cast<jlong>(LangBindHelper::new_table());
}

JNIEXPORT jboolean JNICALL Java_com_tightdb_TableBase_nativeIsValid(
    JNIEnv*, jobject, jlong nativeTablePtr)
{
    return TBL(nativeTablePtr)->is_valid();
}
