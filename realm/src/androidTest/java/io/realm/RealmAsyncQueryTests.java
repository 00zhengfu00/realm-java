/*
 * Copyright 2015 Realm Inc.
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

package io.realm;

import android.os.Looper;
import android.test.InstrumentationTestCase;

import junit.framework.AssertionFailedError;

import java.lang.reflect.Field;
import java.util.Date;
import java.util.Random;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import io.realm.entities.AllTypes;
import io.realm.entities.NonLatinFieldNames;
import io.realm.internal.async.RetryPolicy;
import io.realm.internal.async.UnreachableVersionException;

public class RealmAsyncQueryTests extends InstrumentationTestCase {
    private final static int NO_RETRY = 0;
    private static final int RETRY_ONCE = 1;
    private static final int RETRY_TWICE = 2;
    private static final int RETRY_NUMBER_NOT_APPLICABLE = Integer.MAX_VALUE;

    private final static int NO_ADVANCED_READ = 0;
    private static final int ADVANCE_ONE_READ = 1;
    private static final int ADVANCE_THREE_READ = 3;
    private static final int ADVANCE_HUNDRED_READ = 100;

    // Async query without any conflicts strategy
    public void testFindAll() throws Throwable {
        setDebugModeForAsyncRealmQuery(NO_ADVANCED_READ, RetryPolicy.MODE_NO_RETRY, NO_RETRY);
        // We need to control precisely which Looper/Thread our Realm
        // will operate on. This is unfortunately not possible when using the
        // current Instrumentation#InstrumentationThread, because InstrumentationTestRunner#onStart
        // Call Looper.prepare() for us and surprisingly doesn't call Looper#loop(), this is problematic
        // as the async query callback will not run (because the Handler is sending Runnables to a Looper
        // that didn't loop.
        //
        // On the other hand, using a dedicated 'ExecutorService' will allow us to fine grain control the
        // desired behaviour
        final CountDownLatch signalCallbackFinished = new CountDownLatch(1);
        final Looper[] looper = new Looper[1];
        final Throwable[] threadAssertionError = new Throwable[1];
        ExecutorService executorService = Executors.newSingleThreadExecutor();
        executorService.submit(new Runnable() {
            @Override
            public void run() {
                Looper.prepare();
                looper[0] = Looper.myLooper();
                Realm realm = null;
                try {
                    realm = openRealmInstance("test_find_all");
                    populateTestRealm(realm, 10);

                    // async query (will run on different thread)
                    realm.where(AllTypes.class)
                            .between("columnLong", 0, 9)
                            .findAll(new Realm.QueryCallback<AllTypes>() {
                                @Override
                                public void onSuccess(RealmResults<AllTypes> results) {
                                    try {
                                        assertEquals(10, results.size());

                                        // make sure access to RealmObject will not throw an Exception
                                        for (int i = 0, size = results.size(); i < size; i++) {
                                            assertEquals(i, results.get(i).getColumnLong());
                                        }

                                    } catch (AssertionFailedError e) {
                                        threadAssertionError[0] = e;

                                    } finally {
                                        // whatever happened, make sure to notify the waiting TestCase Thread
                                        signalCallbackFinished.countDown();
                                    }
                                }

                                @Override
                                public void onError(Exception t) {
                                    try {
                                        threadAssertionError[0] = t;
                                        t.printStackTrace();
                                    } finally {
                                        signalCallbackFinished.countDown();
                                    }
                                }
                            });

                    Looper.loop();// ready to receive callback

                } catch (Exception e) {
                    e.printStackTrace();
                    threadAssertionError[0] = e;
                } finally {
                    if (realm != null) {
                        realm.close();
                    }
                }
            }
        });

        // wait until the callback of our async query proceed
        signalCallbackFinished.await();
        looper[0].quit();
        executorService.shutdownNow();
        if (null != threadAssertionError[0]) {
            // throw any assertion errors happened in the background thread
            throw threadAssertionError[0];
        }
    }

    // Async query that should fail, because the caller thread has advanced
    // the version of the Realm, which is different from the one used by
    // the background thread. Since no retry policy was defined, we should fail.
    public void testMismatchedRealmVersion_should_crash_no_retry() throws Throwable {
        // simulate one advanced read just after the worker thread has finished querying the Realm
        // without any retry we should crash, because the background Realm used to perform the query
        // return a TableView using a different version of the caller Realm (now more advanced)
        setDebugModeForAsyncRealmQuery(1, RetryPolicy.MODE_NO_RETRY, NO_RETRY);

        final CountDownLatch signalCallbackFinishedLatch = new CountDownLatch(1);
        final Looper[] looper = new Looper[1];
        final Throwable[] threadAssertionError = new Throwable[1];
        ExecutorService executorService = Executors.newSingleThreadExecutor();
        executorService.submit(new Runnable() {
            @Override
            public void run() {
                Looper.prepare();
                looper[0] = Looper.myLooper();
                Realm realm = null;
                try {
                    realm = openRealmInstance("test_should_crash_no_retry");
                    populateTestRealm(realm, 10);

                    realm.where(AllTypes.class)
                            .between("columnLong", 0, 9)
                            .findAll(new Realm.DebugQueryCallback<AllTypes>() {
                                @Override
                                public void onSuccess(RealmResults<AllTypes> results) {
                                    signalCallbackFinishedLatch.countDown();
                                }

                                @Override
                                public void onError(Exception t) {
                                    try {
                                        threadAssertionError[0] = t;
                                    } finally {
                                        signalCallbackFinishedLatch.countDown();
                                    }
                                }

                                @Override
                                public void onBackgroundQueryCompleted(Realm realm) {
                                    // triggered on the background thread to alter the caller's Realm state
                                    realm.executeTransaction(new Realm.Transaction() {
                                        @Override
                                        public void execute(Realm realm) {
                                            realm.clear(AllTypes.class);
                                        }
                                    });
                                }
                            });

                    Looper.loop();

                } catch (Exception e) {
                    e.printStackTrace();
                    threadAssertionError[0] = e;
                } finally {
                    if (signalCallbackFinishedLatch.getCount() > 0) {
                        // opening Realm crashed, not even callbacks get the chance to be called
                        signalCallbackFinishedLatch.countDown();
                    }
                    if (realm != null) {
                        realm.close();
                    }
                }
            }
        });

        signalCallbackFinishedLatch.await();
        looper[0].quit();
        executorService.shutdownNow();
        if (null == threadAssertionError[0] || !(threadAssertionError[0] instanceof UnreachableVersionException)) {
            fail("Expecting RuntimeException: Unspecified exception." +
                    " Handover failed due to version mismatch in io_realm_internal_TableQuery.cpp");
        }
    }

    // async query that converge after 1 retry
    public void testMismatchedRealmVersion_should_converge_after_1_retry() throws Throwable {
        // simulate one advanced read just after the worker thread has finished querying the Realm
        setDebugModeForAsyncRealmQuery(ADVANCE_ONE_READ, RetryPolicy.MODE_MAX_RETRY, RETRY_ONCE);

        final CountDownLatch signalCallbackFinishedLatch = new CountDownLatch(1);
        final Looper[] looper = new Looper[1];
        final Throwable[] threadAssertionError = new Throwable[1];
        ExecutorService executorService = Executors.newSingleThreadExecutor();
        executorService.submit(new Runnable() {
            @Override
            public void run() {
                Looper.prepare();
                looper[0] = Looper.myLooper();
                Realm realm = null;
                try {
                    realm = openRealmInstance("should_converge_after_1_retry");
                    populateTestRealm(realm, 10);
                    realm.where(AllTypes.class)
                            .between("columnLong", 0, 9)
                            .findAll(new Realm.DebugQueryCallback<AllTypes>() {
                                @Override
                                public void onSuccess(RealmResults<AllTypes> results) {
                                    try {
                                        assertEquals(3, results.size());

                                        for (int i = 0, size = results.size(); i < size; i++) {
                                            assertEquals(i, results.get(i).getColumnLong());
                                        }

                                    } catch (AssertionFailedError e) {
                                        threadAssertionError[0] = e;

                                    } finally {
                                        signalCallbackFinishedLatch.countDown();
                                    }
                                }

                                @Override
                                public void onError(Exception t) {
                                    try {
                                        threadAssertionError[0] = t;
                                    } finally {
                                        signalCallbackFinishedLatch.countDown();
                                    }
                                }

                                @Override
                                public void onBackgroundQueryCompleted(Realm realm) {
                                    populateTestRealm(realm, 3);// this is already inside a transaction
                                }
                            });

                    Looper.loop();//ready to receive callback

                } catch (Exception e) {
                    e.printStackTrace();
                    threadAssertionError[0] = e;
                } finally {
                    if (signalCallbackFinishedLatch.getCount() > 0) {
                        // opening Realm crashed, not even callbacks get the chance to be called
                        signalCallbackFinishedLatch.countDown();
                    }
                    if (realm != null) {
                        realm.close();
                    }
                }
            }
        });

        signalCallbackFinishedLatch.await();
        looper[0].quit();
        executorService.shutdownNow();
        if (null != threadAssertionError[0]) {
            throw threadAssertionError[0];
        }
    }

    // This should crash because the number of retries is less than the number of modifications
    public void testMismatchedRealmVersion_should_crash_after_2_retries() throws Throwable {
        // simulate 3 modification to the caller's Realm for each result coming from the background thread
        setDebugModeForAsyncRealmQuery(ADVANCE_THREE_READ, RetryPolicy.MODE_MAX_RETRY, RETRY_TWICE);

        final CountDownLatch signalCallbackFinishedLatch = new CountDownLatch(1);
        final Looper[] looper = new Looper[1];
        final Throwable[] threadAssertionError = new Throwable[1];
        ExecutorService executorService = Executors.newSingleThreadExecutor();
        executorService.submit(new Runnable() {
            @Override
            public void run() {
                Looper.prepare();
                looper[0] = Looper.myLooper();
                Realm realm = null;
                try {
                    realm = openRealmInstance("test_should_crash_after_2_retries");
                    populateTestRealm(realm, 10);
                    realm.where(AllTypes.class)
                            .between("columnLong", 0, 9)
                            .findAll(new Realm.DebugQueryCallback<AllTypes>() {
                                @Override
                                public void onSuccess(RealmResults<AllTypes> results) {
                                    try {
                                        signalCallbackFinishedLatch.countDown();

                                    } catch (AssertionFailedError e) {
                                        threadAssertionError[0] = e;

                                    } finally {
                                        // whatever happened, make sure to notify the waiting TestCase Thread
                                        signalCallbackFinishedLatch.countDown();
                                    }
                                }

                                @Override
                                public void onError(Exception t) {
                                    try {
                                        threadAssertionError[0] = t;
                                    } finally {
                                        signalCallbackFinishedLatch.countDown();
                                    }
                                }

                                @Override
                                public void onBackgroundQueryCompleted(Realm realm) {
                                    populateTestRealm(realm, 3);
                                }
                            });

                    Looper.loop();

                } catch (Exception e) {
                    e.printStackTrace();
                    threadAssertionError[0] = e;
                } finally {
                    if (signalCallbackFinishedLatch.getCount() > 0) {
                        // opening Realm crashed, not even callbacks get the chance to be called
                        signalCallbackFinishedLatch.countDown();
                    }
                    if (realm != null) {
                        realm.close();
                    }
                }
            }
        });

        signalCallbackFinishedLatch.await();
        looper[0].quit();
        executorService.shutdownNow();
        if (null == threadAssertionError[0] || !(threadAssertionError[0] instanceof UnreachableVersionException)) {
            fail("Expecting RuntimeException: Unspecified exception." +
                    " Handover failed due to version mismatch in io_realm_internal_TableQuery.cpp");
        }
    }

    // Keep retrying until the caller thread & the background results converge
    public void testMismatchedRealmVersion_should_converge_after_100_retry() throws Throwable {
        setDebugModeForAsyncRealmQuery(ADVANCE_HUNDRED_READ, RetryPolicy.MODE_INDEFINITELY, RETRY_NUMBER_NOT_APPLICABLE);

        final CountDownLatch signalCallbackFinishedLatch = new CountDownLatch(1);
        final Looper[] looper = new Looper[1];
        final AtomicInteger numberOfEntries = new AtomicInteger(10);
        final Random random = new Random(System.currentTimeMillis());
        final Throwable[] threadAssertionError = new Throwable[1];
        ExecutorService executorService = Executors.newSingleThreadExecutor();
        executorService.submit(new Runnable() {
            @Override
            public void run() {
                Looper.prepare();
                looper[0] = Looper.myLooper();
                Realm realm = null;
                try {
                    realm = openRealmInstance("test_should_converge_after_100_retry");
                    populateTestRealm(realm, numberOfEntries.get());
                    realm.where(AllTypes.class)
                            .between("columnLong", 0, 9)
                            .findAll(new Realm.DebugQueryCallback<AllTypes>() {
                                @Override
                                public void onSuccess(RealmResults<AllTypes> results) {
                                    try {
                                        if (numberOfEntries.get() > 10) {
                                            assertEquals(10, results.size());
                                        } else {
                                            assertEquals(numberOfEntries.get(), results.size());
                                        }

                                        for (int i = 0, size = results.size(); i < size; i++) {
                                            assertEquals(i, results.get(i).getColumnLong());
                                        }

                                    } catch (AssertionFailedError e) {
                                        threadAssertionError[0] = e;

                                    } finally {
                                        signalCallbackFinishedLatch.countDown();
                                    }
                                }

                                @Override
                                public void onError(Exception t) {
                                    try {
                                        threadAssertionError[0] = t;
                                    } finally {
                                        signalCallbackFinishedLatch.countDown();
                                    }
                                }

                                @Override
                                public void onBackgroundQueryCompleted(Realm realm) {
                                    numberOfEntries.set(random.nextInt(100));
                                    populateTestRealm(realm, numberOfEntries.get());
                                }

                            });

                    Looper.loop();

                } catch (Exception e) {
                    e.printStackTrace();
                    threadAssertionError[0] = e;
                } finally {
                    if (signalCallbackFinishedLatch.getCount() > 0) {
                        signalCallbackFinishedLatch.countDown();
                    }
                    if (realm != null) {
                        realm.close();
                    }
                }
            }
        });

        signalCallbackFinishedLatch.await();
        looper[0].quit();
        executorService.shutdownNow();
        if (null != threadAssertionError[0]) {
            throw threadAssertionError[0];
        }
    }

    public void testCancelQuery() throws Throwable {
        setDebugModeForAsyncRealmQuery(ADVANCE_THREE_READ, RetryPolicy.MODE_INDEFINITELY, RETRY_NUMBER_NOT_APPLICABLE);

        final AtomicInteger retryNumber = new AtomicInteger(0);
        final CountDownLatch signalQueryRunning = new CountDownLatch(1);
        final Looper[] looper = new Looper[1];
        final Throwable[] threadAssertionError = new Throwable[1];
        ExecutorService executorService = Executors.newSingleThreadExecutor();
        final RealmQuery.Request[] asyncRequest = new RealmQuery.Request[1];
        executorService.submit(new Runnable() {
            @Override
            public void run() {
                Looper.prepare();
                looper[0] = Looper.myLooper();
                Realm realm = null;
                try {
                    realm = openRealmInstance("test_new_find_all");
                    populateTestRealm(realm, 10);

                    asyncRequest[0] = realm.where(AllTypes.class)
                            .between("columnLong", 0, 9)
                            .findAll(new Realm.DebugQueryCallback<AllTypes>() {
                                @Override
                                public void onSuccess(RealmResults<AllTypes> results) {
                                    threadAssertionError[0] = new AssertionFailedError("onSuccess called on a cancelled query");
                                }

                                @Override
                                public void onError(Exception t) {
                                    threadAssertionError[0] = new AssertionFailedError("onError called on a cancelled query");
                                }

                                @Override
                                public void onBackgroundQueryCompleted(Realm realm) {
                                    populateTestRealm(realm, 1);
                                    // after 2 retries we cancel the query
                                    if (retryNumber.incrementAndGet() == 2) {
                                        asyncRequest[0].cancel();
                                        signalQueryRunning.countDown();
                                    }
                                }
                            });

                    Looper.loop();

                } catch (Exception e) {
                    e.printStackTrace();
                    threadAssertionError[0] = e;

                } finally {
                    if (realm != null) {
                        realm.close();
                    }
                }
            }
        });

        signalQueryRunning.await();
        executorService.shutdown();
        executorService.awaitTermination(10, TimeUnit.SECONDS);

        assertTrue(asyncRequest[0].isCancelled());

        looper[0].quit();
        executorService.shutdownNow();

        if (null != threadAssertionError[0]) {
            throw threadAssertionError[0];
        }
    }

    // *** Helper methods ***

    // This could be done from #setUp but then we can't control
    // which Looper we want to associate this Realm instance with
    private Realm openRealmInstance(String name) {
        RealmConfiguration realmConfiguration = new RealmConfiguration.Builder(getInstrumentation().getTargetContext())
                .name(name)
                .deleteRealmIfMigrationNeeded()
                .build();
        Realm.deleteRealm(realmConfiguration);
        return Realm.getInstance(realmConfiguration);
    }

    private void populateTestRealm(final Realm testRealm, int objects) {
        testRealm.beginTransaction();
        testRealm.allObjects(AllTypes.class).clear();
        testRealm.allObjects(NonLatinFieldNames.class).clear();
        for (int i = 0; i < objects; ++i) {
            AllTypes allTypes = testRealm.createObject(AllTypes.class);
            allTypes.setColumnBoolean((i % 3) == 0);
            allTypes.setColumnBinary(new byte[]{1, 2, 3});
            allTypes.setColumnDate(new Date());
            allTypes.setColumnDouble(3.1415);
            allTypes.setColumnFloat(1.234567f + i);
            allTypes.setColumnString("test data " + i);
            allTypes.setColumnLong(i);
            NonLatinFieldNames nonLatinFieldNames = testRealm.createObject(NonLatinFieldNames.class);
            nonLatinFieldNames.set델타(i);
            nonLatinFieldNames.setΔέλτα(i);
            nonLatinFieldNames.set베타(1.234567f + i);
            nonLatinFieldNames.setΒήτα(1.234567f + i);
        }
        testRealm.commitTransaction();
        testRealm.refresh();
    }

    // we could have avoided using reflection to inject this behaviour if we were using a DI
    private void setDebugModeForAsyncRealmQuery(int numberOfAdvancedReadSimulation, int retryPolicyMode, int maxNumberOfRetries) {
        try {
            Field debugFlagField = RealmQuery.class.getDeclaredField("IS_DEBUG");
            Field nbAdvancedReadSimulationField = RealmQuery.class.getDeclaredField("NB_ADVANCE_READ_SIMULATION");
            Field nbNumberRetriesPolicyField = RealmQuery.class.getDeclaredField("MAX_NUMBER_RETRIES_POLICY");
            Field retryPolicyModeField = RealmQuery.class.getDeclaredField("RETRY_POLICY_MODE");

            debugFlagField.setAccessible(true);
            nbAdvancedReadSimulationField.setAccessible(true);
            nbNumberRetriesPolicyField.setAccessible(true);
            retryPolicyModeField.setAccessible(true);

            if (numberOfAdvancedReadSimulation > 0) {
                debugFlagField.set(null, true);
                nbAdvancedReadSimulationField.set(null, numberOfAdvancedReadSimulation);
                nbNumberRetriesPolicyField.set(null, maxNumberOfRetries);
                retryPolicyModeField.set(null, retryPolicyMode);

            } else {// reset to defaults
                debugFlagField.set(null, false);
                nbAdvancedReadSimulationField.set(null, 0);
                nbNumberRetriesPolicyField.set(null, 0);
                retryPolicyModeField.set(null, RetryPolicy.MODE_INDEFINITELY);
            }

        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
    }
}
