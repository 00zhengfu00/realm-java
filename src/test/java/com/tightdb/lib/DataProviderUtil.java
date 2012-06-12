package com.tightdb.lib;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

public class DataProviderUtil {

	public static Iterator<Object[]> allCombinations(List<?>... lists) {
		Iterator<Object[]> iterator = new VariationsIterator<Object>(Arrays.asList(lists));
		return iterator;
	}

}
