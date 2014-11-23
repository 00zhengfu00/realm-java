package io.realm;

import android.test.AndroidTestCase;

import java.util.ConcurrentModificationException;
import java.util.Date;
import java.util.Iterator;
import java.util.ListIterator;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import io.realm.entities.AllTypes;
import io.realm.exceptions.RealmException;

public class RealmResultsIteratorTests extends AndroidTestCase {

    protected final static int TEST_DATA_SIZE = 2516;
    protected Realm testRealm;

    @Override
    protected void setUp() throws InterruptedException {
        boolean result = Realm.deleteRealmFile(getContext());
        assertTrue(result);

        testRealm = Realm.getInstance(getContext());

        testRealm.beginTransaction();
        testRealm.allObjects(AllTypes.class).clear();

        for (int i = 0; i < TEST_DATA_SIZE; ++i) {
            AllTypes allTypes = testRealm.createObject(AllTypes.class);
            allTypes.setColumnBoolean((i % 2) == 0);
            allTypes.setColumnBinary(new byte[]{1, 2, 3});
            allTypes.setColumnDate(new Date((long) i));
            allTypes.setColumnDouble(3.1415 + i);
            allTypes.setColumnFloat(1.234567f + i);
            allTypes.setColumnString("test data " + i);
            allTypes.setColumnLong(i);
        }
        testRealm.commitTransaction();
    }

    public void testListIterator_atBeginning() {
        ListIterator<AllTypes> it = testRealm.allObjects(AllTypes.class).listIterator();
        assertFalse(it.hasPrevious());
        assertEquals(-1, it.previousIndex());
        assertTrue(it.hasNext());
        assertEquals(1, it.nextIndex());
    }

    public void testListIterator_atEnd() {
        ListIterator<AllTypes> it = testRealm.allObjects(AllTypes.class).listIterator(TEST_DATA_SIZE - 1);
        assertTrue(it.hasPrevious());
        assertEquals(TEST_DATA_SIZE - 2, it.previousIndex());
        assertFalse(it.hasNext());
        assertEquals(TEST_DATA_SIZE, it.nextIndex());
    }

    private enum ListIteratorMethods { ADD, REMOVE, SET; }
    public void testListIterator_unsupportedMethods() {
        for (ListIteratorMethods method : ListIteratorMethods.values()) {
            ListIterator<AllTypes> it = testRealm.allObjects(AllTypes.class).listIterator();
            try {
                switch (method) {
                    case ADD: it.add(new AllTypes()); break;
                    case REMOVE: it.remove(); break;
                    case SET: it.set(new AllTypes()); break;
                }
            } catch(RealmException e) {
                assertTrue(true);
                continue;
            }

            fail(method + " should not be supported");
        }
    }


    public void testRemovingObjectsInsideLoop() {
        RealmResults<AllTypes> result = testRealm.allObjects(AllTypes.class);

        try {
            testRealm.beginTransaction();
            for (AllTypes obj : result) {
                obj.removeFromRealm();
            }
        } catch (ConcurrentModificationException e) {
            assertTrue(true);
            return;
        } finally {
            testRealm.cancelTransaction();
        }

        fail("Modifying Realm while iterating is not allowed");
    }

    // Query iterator should still be valid if we modify Realm after query but before iterator is
    // fetched.
    public void testIterator_validAfterAutoUpdate() {
        RealmResults<AllTypes> result = testRealm.allObjects(AllTypes.class);

        testRealm.beginTransaction();
        result.removeLast();
        testRealm.commitTransaction();

        long sum = 0;
        for (AllTypes obj : result) {
            sum += obj.getColumnLong();
        }

        assertEquals(sum(0, TEST_DATA_SIZE - 2), sum);
    }


    public void testIterator_standardBehavior() {
        RealmResults<AllTypes> result = testRealm.allObjects(AllTypes.class);

        long sum = 0;
        for (AllTypes obj : result) {
            sum += obj.getColumnLong();
        }

        assertEquals(sum(0, TEST_DATA_SIZE - 1), sum);
    }

    public void testIterator_invalidMethod() {
        RealmResults<AllTypes> result = testRealm.allObjects(AllTypes.class);
        Iterator<AllTypes> it = result.iterator();
        it.next();
        try {
            it.remove();
         } catch (RealmException e) {
            assertTrue(true);
            return;
        }

        fail("remove() not allowed");
    }

    // Using size() as heuristic for concurrent modifications is dangerous as we might skip
    // elements.
    public void testRemovingObjectsFromOtherThreadWhileIterating() throws InterruptedException, ExecutionException {

        // Prefill
        Realm realm = Realm.getInstance(getContext(), "test");
        realm.beginTransaction();
        realm.clear(AllTypes.class);
        AllTypes o1 = realm.createObject(AllTypes.class);
        o1.setColumnLong(1);
        AllTypes o2 = realm.createObject(AllTypes.class);
        o2.setColumnLong(2);
        realm.commitTransaction();

        // Iterate past 1st item
        RealmResults<AllTypes> result = realm.allObjects(AllTypes.class);
        Iterator<AllTypes> it = result.iterator();
        it.next();

        // Delete first item and insert new. Meaning Item 2 gets Item 1s place, and is skipped
        // in iterator when calling next().
        Callable<Boolean> backgroundWorker = new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                Realm backgroundRealm = Realm.getInstance(getContext(), "test", false);
                backgroundRealm.beginTransaction();
                RealmResults<AllTypes> backgroundResult = backgroundRealm.allObjects(AllTypes.class);
                if (backgroundResult.size() != 2) return false;
                backgroundResult.sort("columnLong", RealmResults.SORT_ORDER_ASCENDING).remove(0);
                AllTypes o3 = backgroundRealm.createObject(AllTypes.class);
                o3.setColumnLong(3);
                backgroundRealm.commitTransaction();
                if (backgroundResult.size() != 2) return false;
                return true;
            }
        };

        // Wait for background thread to finish
        ExecutorService executorService = Executors.newFixedThreadPool(1);
        Future<Boolean> backgroundResult = executorService.submit(backgroundWorker);
        assertTrue(backgroundResult.get());
        realm.refresh(); // This shouldn't be needed, but is currently.

        // Next item would now be o3.
        try {
            AllTypes o3 = it.next();
            assertEquals(3, o3.getColumnLong());
            fail("Failed to detect the list was modified, but retained it's size while iterating");
        } catch (ConcurrentModificationException e) {
            return;
        }
    }

    private long sum(int start, int end) {
        long sum = 0;
        for (int i = start; i <= end; i++) {
            sum += i;
        }
        return sum;
    }
}
