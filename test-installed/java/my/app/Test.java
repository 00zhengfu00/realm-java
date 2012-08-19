package my.app;

import com.tightdb.*;
import com.tightdb.lib.Table;

public class Test {

    @Table
    class hilbert {
        String fido;
    }

    @Table
    class banach {
        String type;
        String number;
    }

    public static void main(String[] args)
    {
        Group db = new Group();

        HilbertTable hilbert = new HilbertTable(db);
        hilbert.add("Odif");

        BanachTable banach = new BanachTable(db);
        banach.add("John", "Doe");
    }
}
