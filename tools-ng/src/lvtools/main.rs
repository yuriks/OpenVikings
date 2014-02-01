use std::io::{File, Truncate, Write};

mod chunk;
mod compression;

fn main() {
	let mut file = File::open(&Path::new("data.dat"))
		.expect("Failed to open data.dat!");

	let mut chunk_reader = chunk::ChunkReader::new(&mut file, 0x0C6);

	let data_size = chunk_reader.read_le_u16() as uint + 1;
	let data = compression::decompress_data(&mut chunk_reader, data_size);

	let mut outfile = File::open_mode(&Path::new("out_chunk.bin"), Truncate, Write)
		.expect("Failed to open out file!");
	outfile.write(data);
}
