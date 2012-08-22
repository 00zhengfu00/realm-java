//
// This example is used in the short introduction "Tightdb Java Interface"
// The @@ comments below are used for automatic extraction to the documentation
//

package com.tightdb.example;

import java.io.IOException;

import com.tightdb.Group;
import com.tightdb.generated.People;
import com.tightdb.generated.PeopleQuery;
import com.tightdb.generated.PeopleTable;
import com.tightdb.lib.Table;
import com.tightdb.lib.TightDB;

// @@Example: create_table @@
public class TutorialExample {
    @Table
    class people {
    	String name;
    	int age;
    	boolean hired;
    }
    
public static void main(String[] args) {
    PeopleTable peopletable = new PeopleTable();
// @@EndExample@@
		        
    /****************************** BASIC OPERATIONS *************************/
		
    // @@Example: insert_rows @@
    peopletable.add("John", 20, true);
    peopletable.add("Mary", 21, false);
    peopletable.add("Lars", 21, true);
    peopletable.add("Phil", 43, false);
    peopletable.add("Anni", 54, true);
    // @@EndExample@@
		
    / @@Example: insert_at_index @@
    peopletable.insert(2, "Frank", 34, true);
    // @@EndExample@@

    // @@Example: number_of_rows @@
    if (!peopletable.isEmpty()) {
        long s = peopletable.size(); // s => 6
    }
    // @@EndExample@@
		
    System.out.println("Size = " + peopletable.size() + "\n");
				
    /****************************** GETTERS AND SETTERS **********************/
		
    // @@Example: accessing_rows @@
    // 2 ways to get the value
    String name = peopletable.at(2).getName(); // name => "Mary"
    // or
    String name2 = peopletable.at(2).name.get();
		
    // 2 ways to set the value
    peopletable.at(2).name.set("NewName");
    // or
    peopletable.at(2).setName("NewName"); 
    // @@EndExample@@
		
    System.out.println("at(2).getName -> " + name + " or " + name2);
    System.out.println("at(2).setName('NewName') -> " + peopletable.at(2).getName());

    /****************************** DATA REMOVAL *****************************/
    // @@Example: deleting_row @@
    peopletable.remove(2);
    // @@EndExample@@

    System.out.println("\nRemoved row 2. Down to " + peopletable.size() + " rows.\n");

    /****************************** ITERATION OF ALL RECORDS *****************/

    // lazy iteration over the table
		
    // @@Example: iteration @@
    for (People person : peopletable) {
        System.out.println(person.getName() + " is " + person.getAge() + " years old.");
    }
    // @@EndExample@@
		
    /****************************** SIMPLE QUERY *****************************/

    // @@Example: simple_seach @@
    People p = peopletable.name.equal("John").findFirst();
    // @@EndExample@@

    System.out.println("\nFind 'John': " + p + "\n");

    /****************************** COMPLEX QUERY ****************************/

    // @@Example: advanced_search @@
    // Define the query
    PeopleQuery query = peopletable.name.contains("a")
        age.between(20, 30)
        .group()
            .hired.equal(false)
            .or()
            .name.endsWith("y")
        .endGroup();
    // Count matches
    System.out.println(query.count() + " person match query.");

    // Take the average age of the matches    
    System.out.println(query.average(??) + " years is the average.");

    // Perform query and use the result
    for (People person : query.findAll()) {
        // ... do something with matching 'person'
    }        
    // @@EndExample

    /****************************** SERIALIZE ********************************/

    // @@Example: serialisation @@
    // Create Table in Group
    Group group = new Group();
    PeopleTable people1 = new PeopleTable(group);

    people1.add("John", 20, true);
    people1.add("Mary", 21, false);

    // Write to disk
    try {
        group.writeToFile("people.tightdb");
    } catch (IOException e) {
        e.printStackTrace();
    }  

    // Load a group from disk (and print contents)
    Group fromDisk = new Group("people.tightdb");
    PeopleTable people2 = new PeopleTable(fromDisk);

    for (People person : people2) {
        System.out.println(person.getName() + " is " + person.getAge() + " years old");
    }

    // Write same group to memory buffer
    byte[] buffer = group.writeToMem();

    // Load a group from memory (and print contents)
    Group fromMem = new Group(buffer);
    PeopleTable people3 = new PeopleTable(fromMem);

    for (People person : people3) {
        System.out.println(person.getName() + " is " + person.getAge() + " years old");
    }
    // @@EndExample@@
}
}
