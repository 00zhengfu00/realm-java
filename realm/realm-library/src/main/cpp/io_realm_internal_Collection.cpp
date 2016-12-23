/*
 * Copyright 2016 Realm Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <jni.h>
#include "io_realm_internal_Collection.h"

#include <vector>

#include <shared_realm.hpp>
#include <results.hpp>

#include "util.hpp"
#include "jni_util/method.hpp"

using namespace realm;
using namespace realm::jni_util;

// We need to control the life cycle of Results, weak ref of Java Collection object and the NotificationToken.
// Wrap all three together, so when the Java Collection object gets GCed, all three of them will be invalidated.
struct ResultsWrapper {
    jobject m_collection_weak_ref;
    NotificationToken m_notification_token;

    ResultsWrapper(Results&& results)
    : m_collection_weak_ref(nullptr), m_results(results), m_notification_token() {}

    ResultsWrapper(ResultsWrapper&&) = delete;
    ResultsWrapper& operator=(ResultsWrapper&&) = delete;

    ResultsWrapper(ResultsWrapper const&) = delete;
    ResultsWrapper& operator=(ResultsWrapper const&) = delete;

    ~ResultsWrapper()
    {
        if (m_collection_weak_ref) {
            JNIEnv *env;
            g_vm->AttachCurrentThread(&env, nullptr);
            env->DeleteWeakGlobalRef(m_collection_weak_ref);
        }
    }

    inline Results& get_original_results()
    {
        return m_results;
    }

    inline Results& get_results()
    {
        if (m_snapshot.get_mode() == Results::Mode::Empty) {
            return m_results;
        } else {
            return m_snapshot;
        }
    }

    inline void switch_to_snapshot()
    {
        if (m_snapshot.get_mode() == Results::Mode::Empty) {
            m_snapshot = m_results.snapshot();
        }
    }

    inline void switch_to_origin()
    {
        if (m_snapshot.get_mode() != Results::Mode::Empty) {
            m_snapshot = Results();
        }
    }

    // TODO: This is for deletion related APIs on the Collection. This is not efficient at all since it moves and
    //       and creates TableView. Expose the reference of snapshot's TableView from Results and use that instead.
    inline void refresh_snapshot()
    {
        m_snapshot = m_results.snapshot();
    }

    inline bool is_detached()
    {
        return m_snapshot.get_mode() != Results::Mode::Empty;
    }

private:
    Results m_results;
    Results m_snapshot;
};

static void finalize_results(jlong ptr);

static void finalize_results(jlong ptr)
{
    TR_ENTER_PTR(ptr);
    delete reinterpret_cast<ResultsWrapper*>(ptr);
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_Collection_nativeCreateResults(JNIEnv* env, jclass, jlong shared_realm_ptr, jlong query_ptr,
        jlong sort_desc_native_ptr, jlong distinct_desc_native_ptr)
{
    TR_ENTER()
    try {
        auto query = reinterpret_cast<Query*>(query_ptr);
        if (!QUERY_VALID(env, query)) {
            return reinterpret_cast<jlong>(nullptr);
        }

        auto shared_realm = *(reinterpret_cast<SharedRealm*>(shared_realm_ptr));
        auto sort_desc_ptr = reinterpret_cast<SortDescriptor*>(sort_desc_native_ptr);
        auto distinct_desc_ptr = reinterpret_cast<SortDescriptor*>(distinct_desc_native_ptr);
        Results results(shared_realm, *query,
                        sort_desc_ptr ? *sort_desc_ptr : SortDescriptor(),
                        distinct_desc_ptr ? *distinct_desc_ptr : SortDescriptor());
        auto wrapper = new ResultsWrapper(std::move(results));
        if (shared_realm->is_in_transaction()) {
            wrapper->switch_to_snapshot();
        }

        return reinterpret_cast<jlong>(wrapper);
    } CATCH_STD()
    return reinterpret_cast<jlong>(nullptr);
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_Collection_nativeCreateSnapshot(JNIEnv* env, jclass, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        auto snapshot = wrapper->get_original_results();
        return reinterpret_cast<jlong>(new Results(snapshot));
    } CATCH_STD()
    return reinterpret_cast<jlong>(nullptr);
}

JNIEXPORT jboolean JNICALL
Java_io_realm_internal_Collection_nativeContains(JNIEnv *env, jclass, jlong native_ptr, jlong native_row_ptr)
{
    TR_ENTER_PTR(native_ptr);
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        auto row = reinterpret_cast<Row*>(native_row_ptr);
        size_t index = wrapper->get_results().index_of(*row);
        return to_jbool(index != not_found);
    } CATCH_STD();
    return JNI_FALSE;
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_Collection_nativeGetRow(JNIEnv *env, jclass, jlong native_ptr, jint index)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        auto row = wrapper->get_results().get(static_cast<size_t>(index));
        return reinterpret_cast<jlong>(new Row(std::move(row)));
    } CATCH_STD()
    return reinterpret_cast<jlong>(nullptr);
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_Collection_nativeFirstRow(JNIEnv *env, jclass, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        auto optional_row = wrapper->get_results().first();
        if (optional_row) {
            return reinterpret_cast<jlong>(new Row(std::move(optional_row.value())));
        }
    } CATCH_STD()
    return reinterpret_cast<jlong>(nullptr);

}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_Collection_nativeLastRow(JNIEnv *env, jclass, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        auto optional_row = wrapper->get_results().last();
        if (optional_row) {
            return reinterpret_cast<jlong>(new Row(std::move(optional_row.value())));
        }
    } CATCH_STD()
    return reinterpret_cast<jlong>(nullptr);
}

JNIEXPORT void JNICALL
Java_io_realm_internal_Collection_nativeClear(JNIEnv *env, jclass, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        wrapper->get_results().clear();
        // Refresh snapshot
        wrapper->refresh_snapshot();
    } CATCH_STD()
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_Collection_nativeSize(JNIEnv *env, jclass, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        return static_cast<jlong>(wrapper->get_results().size());
    } CATCH_STD()
    return 0;
}

JNIEXPORT jobject JNICALL
Java_io_realm_internal_Collection_nativeAggregate(JNIEnv *env, jclass, jlong native_ptr, jlong column_index,
        jbyte agg_func)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);

        size_t index = S(column_index);
        Optional<Mixed> value;
        switch (agg_func) {
            case io_realm_internal_Collection_AGGREGATE_FUNCTION_MINIMUM:
                value = wrapper->get_results().min(index);
                break;
            case io_realm_internal_Collection_AGGREGATE_FUNCTION_MAXIMUM:
                value = wrapper->get_results().max(index);
                break;
            case io_realm_internal_Collection_AGGREGATE_FUNCTION_AVERAGE:
                value = wrapper->get_results().average(index);
                // TODO: Align the behavior with ObjectStore.
                if (!value) {
                    value = Optional<Mixed>(0.0);
                }
                break;
            case io_realm_internal_Collection_AGGREGATE_FUNCTION_SUM:
                value = wrapper->get_results().sum(index);
                break;
            default:
                REALM_UNREACHABLE();
        }

        if (!value) {
            return static_cast<jobject>(nullptr);
        }

        Mixed m = *value;
        switch (m.get_type()) {
            case type_Int:
                return NewLong(env, m.get_int());
            case type_Float:
                return NewFloat(env, m.get_float());
            case type_Double:
                return NewDouble(env, m.get_double());
            case type_Timestamp:
                return NewDate(env, m.get_timestamp());
            default:
                throw std::invalid_argument("Excepted numeric type");
        }
    } CATCH_STD()
    return static_cast<jobject>(nullptr);
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_Collection_nativeSort(JNIEnv *env, jclass, jlong native_ptr, jlong sort_desc_native_ptr)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        auto sort_descriptor = *reinterpret_cast<SortDescriptor*>(sort_desc_native_ptr);
        auto sorted_result = wrapper->get_results().sort(std::move(sort_descriptor));
        return reinterpret_cast<jlong>(new ResultsWrapper(std::move(sorted_result)));
    } CATCH_STD()
    return reinterpret_cast<jlong>(nullptr);
}

JNIEXPORT void JNICALL
Java_io_realm_internal_Collection_nativeStartListening(JNIEnv* env, jobject instance, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)

    static JniMethod notify_change_listeners(env, instance, "notifyChangeListeners", "(Z)V");

    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        if (wrapper->m_collection_weak_ref == nullptr) {
            wrapper->m_collection_weak_ref = env->NewWeakGlobalRef(instance);
        }

        auto cb = [=](realm::CollectionChangeSet const& changes,
                                   std::exception_ptr err) {
            // OS will call all notifiers' callback in one run, so check the Java exception first!!
            if (env->ExceptionCheck()) return;

            //if (!wrapper->is_detached()) {
                env->CallVoidMethod(wrapper->m_collection_weak_ref, notify_change_listeners, changes.empty());
            //}
        };

        wrapper->m_notification_token =  wrapper->get_original_results().add_notification_callback(cb);
    } CATCH_STD()
}

JNIEXPORT void JNICALL
Java_io_realm_internal_Collection_nativeStopListening(JNIEnv *env, jobject, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)

    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        wrapper->m_notification_token = {};
    } CATCH_STD()
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_Collection_nativeGetFinalizerPtr(JNIEnv *, jclass)
{
    TR_ENTER()
    return reinterpret_cast<jlong>(&finalize_results);
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_Collection_nativeWhere(JNIEnv *env, jclass, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);

        Query *query = new Query(wrapper->get_original_results().get_query());
        return reinterpret_cast<jlong>(query);
    } CATCH_STD()
    return 0;
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_Collection_nativeIndexOf(JNIEnv *env, jclass, jlong native_ptr, jlong row_native_ptr)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        auto row = reinterpret_cast<Row*>(row_native_ptr);

        return static_cast<jlong>(wrapper->get_results().index_of(*row));
    } CATCH_STD()
    return npos;
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_Collection_nativeIndexOfBySourceRowIndex(JNIEnv *env, jclass, jlong native_ptr,
                                                                jlong source_row_index)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        auto index = static_cast<size_t>(source_row_index);

        return static_cast<jlong>(wrapper->get_results().index_of(index));
    } CATCH_STD()
    return npos;

}

JNIEXPORT void JNICALL
Java_io_realm_internal_Collection_nativeEnableSnapshot(JNIEnv *env, jclass, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        wrapper->switch_to_snapshot();
    } CATCH_STD()
}

JNIEXPORT void JNICALL
Java_io_realm_internal_Collection_nativeDisableSnapshot(JNIEnv *env, jclass, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        wrapper->switch_to_origin();
    } CATCH_STD()
}

JNIEXPORT jboolean JNICALL
Java_io_realm_internal_Collection_nativeIsDetached(JNIEnv *env, jclass, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)
    auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
    return wrapper->is_detached();
}

JNIEXPORT jboolean JNICALL
Java_io_realm_internal_Collection_nativeDeleteLast(JNIEnv *env, jclass, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)
    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        if (wrapper->get_results().size() > 0) {
            wrapper->get_results().get_tableview().remove_last();
            // Refresh snapshot
            wrapper->refresh_snapshot();
            return JNI_TRUE;
        }
    } CATCH_STD()

    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_io_realm_internal_Collection_nativeDeleteFirst(JNIEnv *env, jclass, jlong native_ptr)
{
    TR_ENTER_PTR(native_ptr)

    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        if (wrapper->get_results().size() > 0) {
            wrapper->get_results().get_tableview().remove(0);
            // Refresh snapshot
            wrapper->refresh_snapshot();
            return JNI_TRUE;
        }
    } CATCH_STD()

    return JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_io_realm_internal_Collection_nativeDelete(JNIEnv *env, jclass, jlong native_ptr, jlong index)
{
    TR_ENTER_PTR(native_ptr)

    try {
        auto wrapper = reinterpret_cast<ResultsWrapper*>(native_ptr);
        auto view = wrapper->get_results().get_tableview();
        size_t size = view.size();
        if (index < 0 || index >= size) {
            throw Results::OutOfBoundsIndexException{static_cast<size_t>(index), size};
        }
        view.remove(static_cast<size_t>(index));
        // Refresh snapshot
        wrapper->refresh_snapshot();
    } CATCH_STD()
}

