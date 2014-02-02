extern mod extra;
extern mod lvtools;

use extra::getopts::groups::{optflag, getopts, OptGroup};
use getopts = extra::getopts::groups;
use lvtools::chunk;
use lvtools::util::from_str_multiradix;
use std::io::{println, File, stdout};

fn print_usage(program: &str, opts: &[OptGroup]) {
	println!("Usage: {} [options..] <data.dat> <chunk_id>", program);
	println(getopts::usage("Foobar", opts));
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
	std::io::util::copy(&mut chunk_reader, &mut stdout());
}
