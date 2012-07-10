package com.tightdb.lib;

import java.util.Date;

import org.testng.annotations.Test;

import com.tightdb.example.generated.Employee;
import com.tightdb.example.generated.EmployeeQuery;
import com.tightdb.example.generated.EmployeeTable;
import com.tightdb.example.generated.EmployeeView;

@Test
public class TableNavigationTest extends AbstractNavigationTest {

	private EmployeeTable employees;

	public TableNavigationTest() {
		employees = new EmployeeTable();

		employees.add("John", "Doe", 10000, true, new byte[] { 1, 2, 3 }, new Date(), "extra");
		employees.add("Johny", "B. Good", 20000, true, new byte[] { 1, 2, 3 }, new Date(), true);
		employees.insert(1, "Nikolche", "Mihajlovski", 30000, false, new byte[] { 4, 5 }, new Date(), 1234);
	}

	@Override
	protected AbstractRowset<Employee, EmployeeView, EmployeeQuery> getTableOrView() {
		return employees;
	}

}
