import java.util.Random;
import java.util.HashMap;
import java.util.Vector;

class bench_hashmap
{
	static long[] get_random(int count)
	{
		long[] v = new long[count];
		Random r = new Random();
	    for (int i = 0; i < count; i++) {
	    	v[i] = r.nextLong();
	    }
	    return v;
	}

	static void print_timings(String name, long t1, long t2, long t3, long count)
	{
		System.out.println(String.format("|%-30s|%8s|%12d|%8.1f|",
			String.format("%s::insert", name), "random", count,
			(t2-t1)*1000000.0/count));
		System.out.println(String.format("|%-30s|%8s|%12d|%8.1f|",
			String.format("%s::lookup", name), "random", count,
			(t3-t2)*1000000.0/count));
	}

	static void bench_hashmap(int count, boolean do_print) throws Throwable
	{
		HashMap<Long,Long> hm = new HashMap<Long,Long>();

		long[] data = get_random(count * 2);
		long t1 = System.currentTimeMillis();
		for (int i = 0; i < count; i++) {
			long key = data[i*2], val = data[i*2+1];
	    	hm.put(key, val);
		}
		long t2 = System.currentTimeMillis();
		for (int i = 0; i < count; i++) {
			long key = data[i*2], val = data[i*2+1];
			if (hm.get(key) != val) throw new Throwable();
		}
		long t3 = System.currentTimeMillis();
		if (do_print)
			print_timings("java.util.HashMap", t1, t2, t3, count);
	}

	public static void main(String args[]) throws Throwable
	{
		bench_hashmap(1000000, false); // warm up
		bench_hashmap(1000000, true); // do it
	}
}