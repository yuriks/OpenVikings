extern mod extra;
use extra::getopts::groups::{optflag, getopts, OptGroup};
use getopts = extra::getopts::groups;
use lvtools::{chunk, compression};
use std::io::{println, File, stdout};
use std::num::{from_str_radix, FromStrRadix};

mod lvtools;

fn print_usage(program: &str, opts: &[OptGroup]) {
	println!("Usage: {} [options..] <data.dat> <chunk_id>", program);
	println(getopts::usage("Foobar", opts));
}

fn from_str_multiradix<T: FromStrRadix>(s: &str) -> Option<T> {
	if s.starts_with("0x") {
		from_str_radix(s.slice_from(2), 16)
	} else if s.starts_with("0o") {
		from_str_radix(s.slice_from(2), 8)
	} else if s.starts_with("0b") {
		from_str_radix(s.slice_from(2), 2)
	} else {
		from_str_radix(s, 10)
	}
}

fn main() {
	let args = std::os::args();

	let opts = ~[
		optflag("h", "help", "show usage"),
		];
	let matches = match getopts(args.tail(), opts) {
		Ok(m) => m,
		Err(f) => fail!(f.to_err_msg()),
	};

	if matches.opt_present("h") {
		print_usage(args[0], opts);
		return;
	}

	if matches.free.len() != 2 {
		print_usage(args[0], opts);
		fail!("Incorrect number of positional arguments.");
	}

	let chunk_id = from_str_multiradix(matches.free[1])
		.expect("Invalid chunk id");

	let mut file = File::open(&Path::new(matches.free[0]))
		.expect("Failed to open data file!");

	let mut chunk_reader = chunk::ChunkReader::new(&mut file, chunk_id);
	let data_size = chunk_reader.read_le_u16() as uint + 1;
	let data = compression::decompress_data(&mut chunk_reader, data_size);

	stdout().write(data);
}
