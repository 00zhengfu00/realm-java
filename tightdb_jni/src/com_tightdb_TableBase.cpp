#include <jni.h>
#include <tightdb.hpp>
#include <tightdb/lang_bind_helper.hpp>

#include "util.h"
#include "mixedutil.h"
#include "com_tightdb_TableBase.h"
#include "ColumnTypeUtil.h"
#include "TableSpecUtil.h"
#include "java_lang_List_Util.h"
#include "mixedutil.h"

using namespace tightdb;

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeUpdateFromSpec(
	JNIEnv* env, jobject jTable, jlong nativePtr, jobject jTableSpec)
{
	Table* pTable = reinterpret_cast<Table*>(nativePtr);
	Spec& spec = pTable->get_spec();
	updateSpecFromJSpec(env, spec, jTableSpec);
	pTable->update_from_spec();
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeSize(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr)
{
	return reinterpret_cast<Table*>(nativeTablePtr)->
        size();
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeClear(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr)
{
	reinterpret_cast<Table*>(nativeTablePtr)->
        clear();
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeGetColumnCount(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr)
{
	return reinterpret_cast<Table*>(nativeTablePtr)->
        get_column_count();
}

JNIEXPORT jstring JNICALL Java_com_tightdb_TableBase_nativeGetColumnName(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex)
{
	return env->NewStringUTF(reinterpret_cast<Table*>(nativeTablePtr)->
        get_column_name(static_cast<size_t>(columnIndex)));
}


JNIEXPORT jobject JNICALL Java_com_tightdb_TableBase_nativeGetTableSpec(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr)
{
	static jmethodID jTableSpecConsId = GetTableSpecMethodID(env, "<init>", "()V");
	if (jTableSpecConsId) {
    	jobject jTableSpec = env->NewObject(GetClassTableSpec(env), jTableSpecConsId);
    	
        Table* pTable = reinterpret_cast<Table*>(nativeTablePtr);
	    const Spec& tableSpec = pTable->get_spec();
        UpdateJTableSpecFromSpec(env, tableSpec, jTableSpec);
	    
        return jTableSpec;
	}
    return NULL;
}

JNIEXPORT jint JNICALL Java_com_tightdb_TableBase_nativeGetColumnType(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex)
{
	return static_cast<int>(reinterpret_cast<Table*>(nativeTablePtr)->
        get_column_type(static_cast<size_t>(columnIndex)));
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeAddEmptyRow(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong rows)
{
	return static_cast<size_t>(reinterpret_cast<Table*>(nativeTablePtr)->
        add_empty_row(static_cast<size_t>(rows)));
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeRemove(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong rowIndex)
{
	reinterpret_cast<Table*>(nativeTablePtr)->
        remove(static_cast<size_t>(rowIndex));
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeRemoveLast(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr)
{
	reinterpret_cast<Table*>(nativeTablePtr)->
        remove_last();
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertLong(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jlong value)
{
	reinterpret_cast<Table*>(nativeTablePtr)->
        insert_int(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), value);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertBoolean(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jboolean value)
{
	reinterpret_cast<Table*>(nativeTablePtr)->
        insert_bool(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), value != 0 ? true : false);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertDate(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jlong dateTimeValue)
{
	reinterpret_cast<Table*>(nativeTablePtr)->
        insert_date(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), static_cast<time_t>(dateTimeValue));
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertString(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jstring value)
{
	const char* valueCharPtr = env->GetStringUTFChars(value, NULL);
    if (!valueCharPtr) 
        return;

	reinterpret_cast<Table*>(nativeTablePtr)->
        insert_string(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), valueCharPtr);
	env->ReleaseStringUTFChars(value, valueCharPtr);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertMixed(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jobject jMixedValue)
{
	Table* pTable = reinterpret_cast<Table*>(nativeTablePtr);
	ColumnType columnType = GetMixedObjectType(env, jMixedValue);
    TR("\nInsertMixed columnType %d\n", columnType);
	switch(columnType) {
	case COLUMN_TYPE_INT:
		{
			jlong longValue = GetMixedIntValue(env, jMixedValue);
			pTable->insert_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(longValue));
			return;
		}
	case COLUMN_TYPE_BOOL:
		{
			jboolean boolValue = GetMixedBooleanValue(env, jMixedValue);
			pTable->insert_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(boolValue != 0 ? true : false));
			return;
		}
	case COLUMN_TYPE_STRING:
		{
			jstring jStringValue = GetMixedStringValue(env, jMixedValue);
			const char* stringCharPtr = env->GetStringUTFChars(jStringValue, NULL);
            if (!stringCharPtr) 
                break;
			pTable->insert_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(stringCharPtr));
			env->ReleaseStringUTFChars(jStringValue, stringCharPtr);
			return;
		}
	case COLUMN_TYPE_BINARY:
		{
            jint mixedBinaryType = GetMixedBinaryType(env, jMixedValue);
			if (mixedBinaryType == 0) { // byte[]
                TR("\ninsertMixed(byte[])\n");
				jbyteArray dataArray = GetMixedByteArrayValue(env, jMixedValue);
                if (!dataArray) {
                    TR("\nCan't get MixedValue, ByteArray\n");
                    break;
                }
				BinaryData binaryData;
				binaryData.pointer = (const char*)(env->GetByteArrayElements(dataArray, NULL));
                if (!binaryData.pointer) {
                    TR("\nCan't get ByteArray\n");
                    break;
                }
                binaryData.len = static_cast<size_t>(env->GetArrayLength(dataArray));
				pTable->insert_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(binaryData));
				env->ReleaseByteArrayElements(dataArray, (jbyte*)(binaryData.pointer), 0);
                return;

			} else if (mixedBinaryType == 1) { // ByteBuffer
                TR("\ninsertMixed(ByteBuffer)\n");
                jobject jByteBuffer = GetMixedByteBufferValue(env, jMixedValue);
                if (!jByteBuffer) {
                    TR("\nCan't get ByteBuffer\n");
                    break;
                }
                BinaryData binaryData;
				binaryData.pointer = (const char*)(env->GetDirectBufferAddress(jByteBuffer));
                TR("SetMixed(Binary, data=%x, len=%d)", binaryData.pointer, binaryData.len);
				if (!binaryData.pointer) {
                    TR("\nCan't get BufferAddress\n");
                    break;
                }
                binaryData.len = static_cast<size_t>(env->GetDirectBufferCapacity(jByteBuffer));
				TR("SetMixed(Binary, data=%x, len=%d)", binaryData.pointer, binaryData.len);
                if (binaryData.len >= 0) {
                    pTable->insert_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(binaryData));
                    return;
                }
            } else {
				TR("\nError Mixed binary type invalid: %\n", mixedBinaryType);
			}
            break;
		}
	case COLUMN_TYPE_DATE:
		{
			jlong dateTimeValue = GetMixedDateTimeValue(env, jMixedValue);
			pTable->insert_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(tightdb::Date(static_cast<time_t>(dateTimeValue))));
			return;
		}
	case COLUMN_TYPE_TABLE:
		{
			pTable->insert_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(COLUMN_TYPE_TABLE));
		    return;
        }
	default:
		{
			TR("\nThis type of mixed is not supported yet: %s\n", __FUNCTION__);
		}
	}
    ThrowException(env, IllegalArgument, "nativeInsertMixed()");
}


JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetMixed(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jobject jMixedValue)
{	
	Table* pTable = reinterpret_cast<Table*>(nativeTablePtr);
	ColumnType columnType = GetMixedObjectType(env, jMixedValue);
	switch(columnType) {
	case COLUMN_TYPE_INT:
		{
			jlong longValue = GetMixedIntValue(env, jMixedValue);
			pTable->set_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(longValue));
			return;
		}
	case COLUMN_TYPE_BOOL:
		{
			jboolean boolValue = GetMixedBooleanValue(env, jMixedValue);
			pTable->set_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(boolValue != 0 ? true : false));
			return;
		}
	case COLUMN_TYPE_STRING:
		{
			jstring jStringValue = GetMixedStringValue(env, jMixedValue);
			const char* stringCharPtr = env->GetStringUTFChars(jStringValue, NULL);
            if (stringCharPtr) {
			    pTable->set_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(stringCharPtr));
			    env->ReleaseStringUTFChars(jStringValue, stringCharPtr);
            }
			return;
		}
	case COLUMN_TYPE_DATE:
		{
			jlong dateTimeValue = GetMixedDateTimeValue(env, jMixedValue);
			Date date(dateTimeValue);
			pTable->set_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(date));
			return;
		}
	case COLUMN_TYPE_BINARY:
		{
			jint mixedBinaryType = GetMixedBinaryType(env, jMixedValue);
			if (mixedBinaryType == 0) {
				jbyteArray dataArray = GetMixedByteArrayValue(env, jMixedValue);
                if (!dataArray)
                    break;
				BinaryData binaryData;
				binaryData.pointer = (const char*)(env->GetByteArrayElements(dataArray, NULL));
                if (!binaryData.pointer) 
                    break;
                binaryData.len = static_cast<size_t>(env->GetArrayLength(dataArray));
				pTable->set_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(binaryData));
				env->ReleaseByteArrayElements(dataArray, (jbyte*)(binaryData.pointer), 0);
                return;
			} else if (mixedBinaryType == 1) {
                jobject jByteBuffer = GetMixedByteBufferValue(env, jMixedValue);
                if (!jByteBuffer)
                    break;
				
                BinaryData binaryData;
				binaryData.pointer = (const char*)(env->GetDirectBufferAddress(jByteBuffer));
                TR("SetMixed(Binary, data=%x, len=%d)", binaryData.pointer, binaryData.len);
				if (!binaryData.pointer) 
                    break;
                binaryData.len = static_cast<size_t>(env->GetDirectBufferCapacity(jByteBuffer));
				TR("SetMixed(Binary, data=%x, len=%d)", binaryData.pointer, binaryData.len);
                if (binaryData.len >= 0)
                    pTable->set_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(binaryData));
                return;
			}
            break; // failed
		}
	case COLUMN_TYPE_TABLE:
		{
			pTable->set_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), Mixed(COLUMN_TYPE_TABLE));
		    return;
        }
    default:
		{
			TR("\nERROR: This type of mixed is not supported yet: %d.", columnType);
		}
	}
    TR("\nERROR: nativeSetMixed() failed.\n");
    ThrowException(env, IllegalArgument, "nativeSetMixed()");
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertSubTable(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	TR("nativeInsertSubTable(jTable:%x, nativeTablePtr: %x, colIdx: %lld, rowIdx: %lld)\n",
       jTable, nativeTablePtr,  columnIndex, rowIndex);
	reinterpret_cast<Table*>(nativeTablePtr)->
        insert_subtable(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex));
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertDone(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr)
{
	reinterpret_cast<Table*>(nativeTablePtr)->
        insert_done();
}


JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeGetLong(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	return reinterpret_cast<Table*>(nativeTablePtr)->
        get_int(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex));
}

JNIEXPORT jboolean JNICALL Java_com_tightdb_TableBase_nativeGetBoolean(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	return reinterpret_cast<Table*>(nativeTablePtr)->
        get_bool(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex));
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeGetDateTime(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	return reinterpret_cast<Table*>(nativeTablePtr)->
        get_date(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex));
}

JNIEXPORT jstring JNICALL Java_com_tightdb_TableBase_nativeGetString(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	const char* valueCharPtr = reinterpret_cast<Table*>(nativeTablePtr)->
        get_string(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex));
	return env->NewStringUTF(valueCharPtr);
}

JNIEXPORT jobject JNICALL Java_com_tightdb_TableBase_nativeGetBinary(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	BinaryData data = reinterpret_cast<Table*>(nativeTablePtr)->
        get_binary(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex));
	return env->NewDirectByteBuffer((void*)data.pointer, data.len);
}

JNIEXPORT jbyteArray JNICALL Java_com_tightdb_TableBase_nativeGetByteArray(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	BinaryData data = reinterpret_cast<Table*>(nativeTablePtr)->
        get_binary(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex));
	jbyteArray jresult = env->NewByteArray(data.len);       
	env->SetByteArrayRegion(jresult, 0, data.len, (const jbyte*)data.pointer);
	return jresult;
}

JNIEXPORT jint JNICALL Java_com_tightdb_TableBase_nativeGetMixedType(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	ColumnType mixedType = reinterpret_cast<Table*>(nativeTablePtr)->
        get_mixed_type(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex));
	return static_cast<jint>(mixedType);
}

JNIEXPORT jobject JNICALL Java_com_tightdb_TableBase_nativeGetMixed(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	Mixed value = reinterpret_cast<Table*>(nativeTablePtr)->
        get_mixed(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex));
	return CreateJMixedFromMixed(env, value);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeGetSubTable(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
    if (!IndexAndTypeValid(env, nativeTablePtr, columnIndex, rowIndex, COLUMN_TYPE_TABLE))
        return NULL;
	Table* pSubTable = const_cast<Table*>(LangBindHelper::get_subtable_ptr(reinterpret_cast<Table*>(nativeTablePtr), 
        static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex)));
    TR("nativeGetSubTable(jTableBase:%x, nativeTablePtr: %x, colIdx: %lld, rowIdx: %lld) : %x\n",
        jTableBase, nativeTablePtr, columnIndex, rowIndex, pSubTable);
    return (jlong)pSubTable;
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeGetSubTableSize(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
    Table* tbl = reinterpret_cast<Table*>(nativeTablePtr);
    if (IndexAndTypeValid(env, nativeTablePtr, columnIndex, rowIndex, COLUMN_TYPE_TABLE)) {
        return tbl->get_subtable_size(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex));
    }
    return -1;
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetString(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jstring value)
{
	const char* valueCharPtr = env->GetStringUTFChars(value, NULL);
    if (valueCharPtr) {
	    reinterpret_cast<Table*>(nativeTablePtr)->
            set_string(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), valueCharPtr);
	    env->ReleaseStringUTFChars(value, valueCharPtr);
    }
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetLong(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jlong value)
{
	return reinterpret_cast<Table*>(nativeTablePtr)->
        set_int(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), value);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetBoolean(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jboolean value)
{
	return reinterpret_cast<Table*>(nativeTablePtr)->
        set_bool(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), value == JNI_TRUE ? true : false);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetDate(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jlong dateTimeValue)
{
	reinterpret_cast<Table*>(nativeTablePtr)->
        set_date(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), dateTimeValue);
}


JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetBinary(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jobject byteBuffer)
{	
	const char *dataPtr = (const char*)(env->GetDirectBufferAddress(byteBuffer));
    if (!dataPtr) {
        ThrowException(env, IllegalArgument, "nativeSetBinary byteBuffer");
        return;
    }
    size_t dataLen = static_cast<size_t>(env->GetDirectBufferCapacity(byteBuffer));
    if (dataLen < 0) {
        ThrowException(env, IllegalArgument, "nativeSetBinary(byteBuffer) - can't get BufferCapacity.");
        return;
    }
    reinterpret_cast<Table*>(nativeTablePtr)->
        set_binary(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), dataPtr, dataLen);            
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertBinary__JJJLjava_nio_ByteBuffer_2(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jobject byteBuffer)
{
    const char *dataPtr = (const char*)(env->GetDirectBufferAddress(byteBuffer));
    if (!dataPtr) {
        TR("\nERROR: nativeInsertBinary( nativePtr %x, col %x, row %x, byteBuf %x) - can't get BufferAddress!\n",
            nativeTablePtr, columnIndex, rowIndex, byteBuffer);
        ThrowException(env, IllegalArgument, "nativeInsertBinary(byteBuffer) - can't get BufferAddress.");
        return;
    }
    size_t dataLen = static_cast<size_t>(env->GetDirectBufferCapacity(byteBuffer));
    if (dataLen < 0) {
        ThrowException(env, IllegalArgument, "nativeInsertBinary(byteBuffer) - can't get BufferCapacity.");
        return;
    }
    reinterpret_cast<Table*>(nativeTablePtr)->
        insert_binary(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), dataPtr, dataLen);
}


JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetByteArray(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex, jbyteArray dataArray)
{
	jbyte* bytePtr = env->GetByteArrayElements(dataArray, NULL);
    if (!bytePtr) {
        ThrowException(env, IllegalArgument, "nativeSetByteArray");
        return;
    }
    size_t dataLen = static_cast<size_t>(env->GetArrayLength(dataArray));
    reinterpret_cast<Table*>(nativeTablePtr)->
        set_binary(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), reinterpret_cast<const char*>(bytePtr), dataLen);
    
    env->ReleaseByteArrayElements(dataArray, bytePtr, 0);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeInsertBinary__JJJ_3B(
	JNIEnv* env, jobject jTableBase, jlong nativeTableBasePtr, jlong columnIndex, jlong rowIndex, jbyteArray dataArray)
{
	jbyte* bytePtr = env->GetByteArrayElements(dataArray, NULL);
    if (!bytePtr) {
        ThrowException(env, IllegalArgument, "nativeInsertBinary byte[]");
        return;
    }
    size_t dataLen = static_cast<size_t>(env->GetArrayLength(dataArray));
	reinterpret_cast<Table*>(nativeTableBasePtr)->
        insert_binary(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex), reinterpret_cast<const char*>(bytePtr), dataLen);
	
    env->ReleaseByteArrayElements(dataArray, bytePtr, 0);
}


JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeClearSubTable(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr, jlong columnIndex, jlong rowIndex)
{
	reinterpret_cast<Table*>(nativeTablePtr)->
        clear_subtable(static_cast<size_t>(columnIndex), static_cast<size_t>(rowIndex));
}

// Indexing methods:

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeSetIndex(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex)
{
	reinterpret_cast<Table*>(nativeTablePtr)->
        set_index(static_cast<size_t>(columnIndex));
}

JNIEXPORT jboolean JNICALL Java_com_tightdb_TableBase_nativeHasIndex(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex)
{
	return reinterpret_cast<Table*>(nativeTablePtr)->
        has_index(static_cast<size_t>(columnIndex));
}

// Aggregare methods:

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeSum(
	JNIEnv* env, jobject jTableBase, jlong nativePtr, jlong columnIndex)
{
	return reinterpret_cast<Table*>(nativePtr)->
        sum(static_cast<size_t>(columnIndex));
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeMaximum(
	JNIEnv* env, jobject jTableBase, jlong nativePtr, jlong columnIndex)
{	
	return reinterpret_cast<Table*>(nativePtr)->
        maximum(static_cast<size_t>(columnIndex));
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeMinimum(
	JNIEnv* env, jobject jTableBase, jlong nativePtr, jlong columnIndex)
{	
	return reinterpret_cast<Table*>(nativePtr)->
        minimum(static_cast<size_t>(columnIndex));
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeAverage(
	JNIEnv* env, jobject jTableBase, jlong nativePtr, jlong columnIndex)
{
	//return reinterpret_cast<Table*>(nativePtr)->
    //    average(static_cast<size_t>(columnIndex));
	return 0;
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindFirstInt(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong value)
{
	return reinterpret_cast<Table*>(nativeTablePtr)->
        find_first_int(static_cast<size_t>(columnIndex), value);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindFirstBoolean(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jboolean value)
{
	return reinterpret_cast<Table*>(nativeTablePtr)->
        find_first_bool(static_cast<size_t>(columnIndex), value != 0 ? true : false);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindFirstDate(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jlong dateTimeValue)
{
	return reinterpret_cast<Table*>(nativeTablePtr)->
        find_first_date(static_cast<size_t>(columnIndex), (time_t)dateTimeValue);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindFirstString(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr, jlong columnIndex, jstring value)
{
	const char* valueCharPtr = env->GetStringUTFChars(value, NULL);
    if (!valueCharPtr) 
        return -1;

	jlong result = reinterpret_cast<Table*>(nativeTablePtr)->
        find_first_string(static_cast<size_t>(columnIndex), valueCharPtr);
    env->ReleaseStringUTFChars(value, valueCharPtr);
	return result;
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindAllInt(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr, jlong columnIndex, jlong value)
{
	Table* pTable = reinterpret_cast<Table*>(nativeTablePtr);
	TableView* pTableView = new TableView( 
        pTable->find_all_int(static_cast<size_t>(columnIndex), value) );
	return reinterpret_cast<jlong>(pTableView);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindAllBool(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr, jlong columnIndex, jboolean value)
{
	Table* pTable = reinterpret_cast<Table*>(nativeTablePtr);
	TableView* pTableView = new TableView( 
        pTable->find_all_bool(static_cast<size_t>(columnIndex), value != 0 ? true : false) );
	return reinterpret_cast<jlong>(pTableView);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindAllDate(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr, jlong columnIndex, jlong dateTimeValue)
{
	Table* pTable = reinterpret_cast<Table*>(nativeTablePtr);
	TableView* pTableView = new TableView( 
        pTable->find_all_date(static_cast<size_t>(columnIndex), static_cast<time_t>(dateTimeValue)) );
	return reinterpret_cast<jlong>(pTableView);
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_nativeFindAllString(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr, jlong columnIndex, jstring value)
{
	Table* pTable = reinterpret_cast<Table*>(nativeTablePtr);
	const char* valueCharPtr = env->GetStringUTFChars(value, NULL);
    if (!valueCharPtr) 
        return -1;

	TableView* pTableView = new TableView(
        pTable->find_all_string(static_cast<size_t>(columnIndex), valueCharPtr) );
	return reinterpret_cast<jlong>(pTableView);
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeOptimize(
	JNIEnv* env, jobject jTableBase, jlong nativeTablePtr)
{
	reinterpret_cast<Table*>(nativeTablePtr)->optimize();
}

JNIEXPORT void JNICALL Java_com_tightdb_TableBase_nativeClose(
	JNIEnv* env, jobject jTable, jlong nativeTablePtr)
{
	TR("nativeClose(jTable: %x, nativeTablePtr: %x)\n", jTable, nativeTablePtr);
    LangBindHelper::unbind_table_ref(reinterpret_cast<Table*>(nativeTablePtr));
}

JNIEXPORT jlong JNICALL Java_com_tightdb_TableBase_createNative(JNIEnv* env, jobject jTable)
{
    TR("CreateNative(jTable: %x)\n", jTable);
    return reinterpret_cast<jlong>(new Table());
}
