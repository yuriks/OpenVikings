extern mod extra;
extern mod lvtools;

use extra::getopts::groups::{optopt, optflag, getopts, OptGroup};
use getopts = extra::getopts::groups;
use lvtools::compression;
use lvtools::util::from_str_multiradix;
use std::io::{println, stdout, stdin};

fn print_usage(program: &str, opts: &[OptGroup]) {
	println!("Usage: {} [options..] [--size <length>]", program);
	println(getopts::usage("Foobar", opts));
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
