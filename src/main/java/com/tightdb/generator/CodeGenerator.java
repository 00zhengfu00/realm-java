package com.tightdb.generator;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.jannocessor.collection.Power;
import org.jannocessor.collection.api.PowerList;
import org.jannocessor.collection.api.PowerMap;
import org.jannocessor.collection.api.PowerSet;
import org.jannocessor.model.executable.JavaMethod;
import org.jannocessor.model.structure.AbstractJavaClass;
import org.jannocessor.model.structure.JavaClass;
import org.jannocessor.model.type.JavaType;
import org.jannocessor.model.util.Methods;
import org.jannocessor.model.util.New;
import org.jannocessor.model.variable.JavaField;
import org.jannocessor.model.variable.JavaParameter;
import org.jannocessor.processor.api.CodeProcessor;
import org.jannocessor.processor.api.ProcessingContext;

public class CodeGenerator implements CodeProcessor<AbstractJavaClass> {

	private static final boolean DEBUG_MODE = false;

	private final static PowerSet<String> NUM_TYPES = Power.set("long", "int", "byte", "short", "Long", "Integer", "Byte", "Short");
	private final static PowerSet<String> OTHER_TYPES = Power.set("String", "Date");

	private Map<String, AbstractJavaClass> tables = new HashMap<String, AbstractJavaClass>();
	private Map<String, AbstractJavaClass> subtables = new HashMap<String, AbstractJavaClass>();

	@Override
	public void process(PowerList<AbstractJavaClass> classes, ProcessingContext context) {
		context.getLogger().info("Processing {} classes", classes.size());
		prepareTables(classes, context);

		for (AbstractJavaClass model : classes) {
			PowerList<JavaField> fields = model.getFields();
			context.getLogger().info("Generating code for entity \"{}\", with {} columns...", model.getName(), fields.size());

			// clone the class, so the original model isn't touched
			String entity = model.getName().getCapitalized();

			/*********** Prepare the attributes for the templates ****************/

			/* Construct the list of columns */

			int index = 0;
			final PowerList<JavaField> columns = Power.list();
			for (JavaField field : fields) {
				String columnType = getColumnType(field);
				JavaType type = getFieldType(field);
				JavaField f = New.field(type, field.getName().getText());

				boolean isSubtable = isSubtable(type);
				String subtype = isSubtable ? type.getSimpleName().getCapitalized() : null;
				PowerMap<String, ? extends Object> fieldAttrs = Power.map("index", index++, "columnType", columnType, "isSubtable", isSubtable).set(
						"subtype", subtype);
				f.getCode().setAttributes(fieldAttrs);
				columns.add(f);
			}

			/* Set the attributes */

			boolean isNested = isSubtable(model.getType());
			Map<String, ? extends Object> attributes = Power.map("entity", New.name(model.getName().getCapitalized()), "columns", columns,
					"isNested", isNested);

			/*********** Construct the table class ****************/

			JavaClass table = New.classs(entity + "Table");
			table.getCode().setMacroName("table");

			table.getCode().setAttributes(attributes);

			/* Construct the "add" method in the table class */

			List<JavaParameter> addParams = Power.list();

			for (JavaField field : fields) {
				if (!isSubtable(field.getType())) {
					addParams.add(New.parameter(field.getType(), field.getName().getText()));
				}
			}

			JavaMethod add = New.method(Methods.PUBLIC, New.type(entity), "add", addParams);
			add.getBody().setMacroName("table_add");
			table.getMethods().add(add);

			/* Construct the "insert" method in the table class */

			List<JavaParameter> insertParams = Power.list();
			insertParams.add(New.parameter(long.class, "position"));

			for (JavaField field : fields) {
				if (!isSubtable(field.getType())) {
					insertParams.add(New.parameter(field.getType(), field.getName().getText()));
				}
			}

			JavaMethod insert = New.method(Methods.PUBLIC, New.type(entity), "insert", insertParams);
			insert.getBody().setMacroName("table_insert");
			table.getMethods().add(insert);

			/*********** Construct the cursor class ****************/

			JavaClass cursor = New.classs(entity);
			cursor.getCode().setMacroName("cursor");
			cursor.getCode().setAttributes(attributes);

			/*********** Construct the view class ****************/

			JavaClass view = New.classs(entity + "View");
			view.getCode().setMacroName("view");
			view.getCode().setAttributes(attributes);

			/*********** Construct the query class ****************/

			JavaClass query = New.classs(entity + "Query");
			query.getCode().setMacroName("query");
			query.getCode().setAttributes(attributes);

			/*********** Generate the source code ****************/

			New.packagee("com.tightdb.generated").getClasses().addAll(table, cursor, view, query);

			context.generateCode(table, DEBUG_MODE);
			context.generateCode(cursor, DEBUG_MODE);
			context.generateCode(view, DEBUG_MODE);
			context.generateCode(query, DEBUG_MODE);
		}

	}

	private void prepareTables(PowerList<AbstractJavaClass> classes, ProcessingContext context) {
		for (AbstractJavaClass model : classes) {
			String name = model.getQualifiedName().getText();

			if (isReferencedBy(model, classes)) {
				context.getLogger().info("- detected subtable: {}", name);
				subtables.put(name, model);
			} else {
				context.getLogger().info("- detected top-level table: {}", name);
				tables.put(name, model);
			}
		}
	}

	private boolean isReferencedBy(AbstractJavaClass model, PowerList<AbstractJavaClass> classes) {
		String modelType = model.getType().getCanonicalName();
		
		for (AbstractJavaClass cls : classes) {
			for (JavaField field : cls.getFields()) {
				String fieldType = field.getType().getCanonicalName();
				if (modelType.equals(fieldType)) {
					return true;
				}
			}
		}
		
		return false;
	}

	private String getColumnType(JavaField field) {
		String type = field.getType().getSimpleName().getText();

		if (NUM_TYPES.contains(type)) {
			type = "Long";
		} else if ("boolean".equalsIgnoreCase(type)) {
			type = "Boolean";
		} else if ("byte[]".equalsIgnoreCase(type) || "ByteBuffer".equalsIgnoreCase(type)) {
			type = "Binary";
		} else if (isSubtable(field.getType())) {
			type = "Table";
		} else if (!OTHER_TYPES.contains(type)) {
			type = "Mixed";
		}

		return type;
	}

	private boolean isSubtable(JavaType type) {
		return subtables.containsKey(type.getCanonicalName());
	}

	private JavaType getFieldType(JavaField field) {
		JavaType type = field.getType().copy();
		String simpleName = type.getSimpleName().getText();

		if (NUM_TYPES.contains(simpleName)) {
			type = New.type("long");
		} else if (simpleName.equals("byte[]")) {
			type = New.type("java.nio.ByteBuffer");
		} else if (simpleName.equals("Object")) {
			type = New.type("com.tightdb.Mixed");
		}

		return type;
	}

}
