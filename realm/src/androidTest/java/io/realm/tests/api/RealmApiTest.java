/*
 * Copyright 2014 Realm Inc.
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

package io.realm.tests.api;

import android.test.AndroidTestCase;

import java.io.IOException;
import java.util.Date;

import io.realm.Realm;
import io.realm.RealmChangeListener;
import io.realm.RealmList;
import io.realm.RealmObject;
import io.realm.internal.SharedGroup;
import io.realm.internal.Table;
import io.realm.tests.api.entities.AllColumns;
import io.realm.tests.api.entities.AllColumnsRealmProxy;
import io.realm.tests.api.entities.Dog;
import io.realm.tests.api.entities.NoAnnotationObject;


public class RealmApiTest extends AndroidTestCase {

    // Test setup methods:
    private void setupSharedGroup() {
        Realm.setDefaultDurability(SharedGroup.Durability.FULL);
    }

    private Realm getTestRealm() {
        setupSharedGroup();
        
        Realm testRealm = null;
        try {
            testRealm = new Realm(getContext().getFilesDir());
        } catch (IOException ex) {
            ex.printStackTrace();
            fail("Unexpected exception while initializing test case");
        }
        return testRealm;
    }

    private <E extends RealmObject> E getTestObject(Realm realm, Class<E> clazz) {
        setupSharedGroup();

        return realm.create(clazz);
    }

    final static int TEST_DATA_SIZE = 2;
    private void buildAllColumnsTestData(Realm realm) {
        realm.clear();
        realm.beginWrite();

        AllColumns allColumns = null;
        allColumns = getTestObject(realm, AllColumns.class);
        allColumns.setColumnBoolean(true);
        allColumns.setColumnBinary(new byte[]{1,2,3});
        allColumns.setColumnDate(new Date());
        allColumns.setColumnDouble(3.1415);
        allColumns.setColumnFloat(1.234567f);
        allColumns.setColumnString("test data");
        allColumns.setColumnLong(45);

        allColumns = null;
        allColumns = getTestObject(realm, AllColumns.class);
        allColumns.setColumnBoolean(false);
        allColumns.setColumnBinary(new byte[]{4,5,6});
        allColumns.setColumnDate(new Date());
        allColumns.setColumnDouble(9999.99);
        allColumns.setColumnFloat(0.1f);
        allColumns.setColumnString("more data");
        allColumns.setColumnLong(46);

        realm.commit();
    }

    //Test Realm.java API

    // setDefaultDurability(SharedGroup.Durability durability)
    public void testShouldSetDurabilityFull() {
        Realm.setDefaultDurability(SharedGroup.Durability.FULL);
    }

    // setDefaultDurability(SharedGroup.Durability durability)
    public void testShouldSetDurabilityFullByName() {
        Realm.setDefaultDurability(SharedGroup.Durability.valueOf("FULL"));
    }

    // setDefaultDurability(SharedGroup.Durability durability)
    public void testShouldSetDurabilityWithInvalidNameFail() {
        try {
            Realm.setDefaultDurability(SharedGroup.Durability.valueOf("INVALID"));
            fail("Expected IllegalArgumentException when providing illegal Durability value");
        } catch (IllegalArgumentException ioe) {
        }
    }

    // setDefaultDurability(SharedGroup.Durability durability)
    public void testShouldSetDurabilityMemOnly() {
        Realm.setDefaultDurability(SharedGroup.Durability.MEM_ONLY);
    }

    // Realm Constructors
    public void testShouldCreateRealm() {
        setupSharedGroup();

        try {
            Realm realm = new Realm(getContext().getFilesDir());
        } catch (Exception ex) {
            ex.printStackTrace();
            fail("Unexpected Exception: "+ex.getMessage());
        }
    }

    public void testShouldFailCreateRealmWithNullDir() {
        setupSharedGroup();

        try {
            Realm realm = new Realm(null);
            fail("Expected IOException");
        } catch (IOException ioe) {
        } catch (Exception ex) {
            ex.printStackTrace();
            fail("Unexpected exception");
        }
        //} catch (NoClassDefFoundError ncdf) {
    }

    public void testShouldFailWithNullFileName() {
        setupSharedGroup();

        try {
            Realm realm = new Realm(getContext().getFilesDir(), null);
            fail("Expected IOException");
        } catch (IOException ioe) {
        } catch (Exception ex) {
            ex.printStackTrace();
            fail("Unexpected exception: " + ex.getMessage());
        }
        //} catch (NullPointerException npe) {
    }

    // Table creation and manipulation

    //Table getTable(Class<?> clazz)
    public void testShouldGetTable() {
        Realm testRealm = getTestRealm();

        Table table = testRealm.getTable(AllColumns.class);
        assertNotNull("getTable is returning a null Table object", table);
    }

    //boolean contains(Class<?> clazz)
    public void testShouldContainTable() {
        Realm testRealm = getTestRealm();
        testRealm.getTable(AllColumns.class);

        boolean testIfContained = testRealm.contains(AllColumns.class);
        assertTrue("contains returns false for newly created table", testIfContained);
    }

    //boolean contains(Class<?> clazz)
    public void testShouldNotContainTable() {
        Realm testRealm = getTestRealm();

        boolean testIfContained = testRealm.contains(AllColumns.class);
        assertFalse("contains returns true for non-existing table", testIfContained);
    }

    //<E extends RealmObject> E create(Class<E> clazz)
    public void testShouldCreateObject() {
        Realm testRealm = getTestRealm();

        RealmObject allColumns = testRealm.create(AllColumns.class);
        boolean instanceMatch = allColumns instanceof AllColumnsRealmProxy;
        assertTrue("Realm.create is returning wrong object type", instanceMatch);
    }

    //<E extends RealmObject> E create(Class<E> clazz)
    public void testShouldNotCreateObject() {
        Realm testRealm = getTestRealm();

        RealmObject noAnnotationObject = testRealm.create(NoAnnotationObject.class);
        assertNull("Realm create expected to fail", noAnnotationObject);
    }

    // <E> void remove(Class<E> clazz, long objectIndex)
    public void testShouldRemoveRow() {
        Realm realm = getTestRealm();
        buildAllColumnsTestData(realm);

        realm.remove(AllColumns.class,0);
    }

    // <E extends RealmObject> E get(Class<E> clazz, long rowIndex)
    public void testShouldGetObject() {
        Realm realm = getTestRealm();
        buildAllColumnsTestData(realm);

        RealmObject allColumns = realm.get(AllColumns.class,0);
        boolean instanceMatch = allColumns instanceof AllColumns;
        assertTrue("Realm.get is returning wrong object type", instanceMatch);
    }

    // <E extends RealmObject> RealmQuery<E> where(Class<E> clazz)
    public void testShouldReturnResultSet() {
        Realm realm = getTestRealm();
        buildAllColumnsTestData(realm);

        RealmList<AllColumns> realmList = realm.where(AllColumns.class).findAll();
        boolean checkListSize = realmList.size() == TEST_DATA_SIZE;
        assertTrue("Realm.get is returning wrong object type",checkListSize);
    }

    // <E extends RealmObject> RealmTableOrViewList<E> allObjects(Class<E> clazz)
    public void testShouldReturnTableOrViewList() {
        Realm realm = getTestRealm();
        buildAllColumnsTestData(realm);

        RealmList<AllColumns> realmList = realm.allObjects(AllColumns.class);
        boolean checkListSize = realmList.size() == TEST_DATA_SIZE;
        assertTrue("Realm.get is returning wrong object type",checkListSize);
    }

    // void ensureRealmAtVersion(int version, RealmMigration migration)
    public void testShouldVerifyVersion() {
    }


    // Notifications

    //void addChangeListener(RealmChangeListener listener)
    public void testChangeNotify() {
        Realm realm = getTestRealm();
        int testCount = 0;

        realm.addChangeListener(new RealmChangeListener() {
            @Override
            public void onChange() {
                testCount++;
            }
        });

        try {
            realm.beginWrite();
            for (int i = 0; i < 5; i++) {

                Dog dog = realm.create(Dog.class);
                dog.setName("King "+Integer.toString(testCount) );
            }

            realm.commit();

        } catch (Throwable t) {
            t.printStackTrace();
        }
    }


        //void removeChangeListener(RealmChangeListener listener)

    //void removeAllChangeListeners()

    //boolean hasChanged()

    // Transactions

    //void refresh()


    //void beginWrite()

    //void commit()

    //void clear(Class<?> classSpec)

    //void clear()

    //int getVersion()

    //void setVersion(int version)

}