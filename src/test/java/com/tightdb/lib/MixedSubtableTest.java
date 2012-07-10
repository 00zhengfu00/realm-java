package com.tightdb.lib;

import static org.testng.AssertJUnit.*;

import org.testng.annotations.Test;

import com.tightdb.example.generated.Employee;
import com.tightdb.example.generated.PeopleTable;
import com.tightdb.example.generated.PhoneTable;

public class MixedSubtableTest extends AbstractTableTest {

	@Test
	public void shouldStoreSubtableInMixedTypeColumn() {
		Employee employee = employees.at(0);
		PhoneTable phones = employee.extra.createSubtable(PhoneTable.class);

		phones.add("mobile", "123");
		assertEquals(1, phones.size());

		PhoneTable phones2 = employee.extra.getSubtable(PhoneTable.class);
		assertEquals(1, phones2.size());
	}

	@Test(expectedExceptions = IllegalArgumentException.class)
	public void shouldFailOnOnWrongSubtableRetrievalFromMixedTypeColumn() {
		Employee employee = employees.at(0);
		PhoneTable phones = employee.extra.createSubtable(PhoneTable.class);

		phones.add("mobile", "123");
		assertEquals(1, phones.size());

		// should fail - since we try to get the wrong subtable class
		employee.extra.getSubtable(PeopleTable.class);
	}
	
	@Test(expectedExceptions = IllegalArgumentException.class)
	public void shouldFailOnOnSubtableRetrtievalFromIncorrectType() {
		Employee employee = employees.at(0);
		employee.extra.set(123);

		// should fail
		employee.extra.getSubtable(PhoneTable.class);
	}
	
}
