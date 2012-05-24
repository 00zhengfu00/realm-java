package com.tightdb.lib;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import org.junit.Ignore;
import org.junit.Test;

import com.tightdb.generated.Employee;
import com.tightdb.generated.EmployeeView;

public class TableColumnsTest extends AbstractTableTest {

	@Test
	public void shouldFindFirstRecordByColumnValue() throws IllegalAccessException {
		Employee record1 = employees.firstName.findFirst(NAME1);
		assertEquals(1, record1.getPosition());

		Employee record2 = employees.salary.findFirst(10000);
		assertEquals(0, record2.getPosition());

		Employee record3 = employees.salary.findFirst(12345);
		assertNull(record3);
	}

	@Test
	public void shouldFindAllRecordsByColumnValue() throws IllegalAccessException {
		EmployeeView view1 = employees.firstName.findAll(NAME1);
		assertEquals(1, view1.size());

		EmployeeView view2 = employees.salary.findAll(10000);
		assertEquals(2, view2.size());

		EmployeeView view3 = employees.salary.findAll(12345);
		assertEquals(0, view3.size());
	}

	@Test
	public void shouldAggregateColumnValue() {
		assertEquals(10000, employees.salary.minimum());
		assertEquals(30000, employees.salary.maximum());
		assertEquals(50000, employees.salary.sum());
	}

}
