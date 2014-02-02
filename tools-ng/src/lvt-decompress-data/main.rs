extern mod extra;
extern mod lvtools;

use extra::getopts::groups::{optopt, optflag, getopts, OptGroup};
use getopts = extra::getopts::groups;
use lvtools::compression;
use std::io::{println, stdout, stdin};
use std::num::{from_str_radix, FromStrRadix};

fn print_usage(program: &str, opts: &[OptGroup]) {
	println!("Usage: {} [options..] [--size <length>]", program);
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
		optopt("s", "size", "manually specify output size instead of reading from header", "SIZE"),
		];
	let matches = match getopts(args.tail(), opts) {
		Ok(m) => m,
		Err(f) => fail!(f.to_err_msg()),
	};

	if matches.opt_present("h") {
		print_usage(args[0], opts);
		return;
	}

	let mut input = stdin();

	let data_size = match matches.opt_str("s") {
		Some(size_str) => from_str_multiradix(size_str).expect("Invalid size"),
		None => input.read_le_u16() as uint + 1,
	};
	let data = compression::decompress_data(&mut input, data_size);

	stdout().write(data);
}
