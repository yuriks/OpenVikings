use std::io::{File, SeekSet};
use std::num::min;

fn seek_to_chunk(f: &mut File, chunk_id: u16) -> uint {
	f.seek(chunk_id as i64 * 4, SeekSet);
	let chunk_start = f.read_le_u32();
	let chunk_end = f.read_le_u32();
	f.seek(chunk_start as i64, SeekSet);
	return (chunk_end - chunk_start) as uint;
}

pub struct ChunkReader<'a> {
	data_file: &'a mut File,
	chunk_pos: i64,
	chunk_start: i64,
	chunk_end: i64,
}

impl<'a> ChunkReader<'a> {
	pub fn new(data_file: &'a mut File, chunk_id: u16) -> ChunkReader<'a> {
		data_file.seek(chunk_id as i64 * 4, SeekSet);
		let chunk_start = data_file.read_le_u32();
		let chunk_end = data_file.read_le_u32();
		data_file.seek(chunk_start as i64, SeekSet);

		ChunkReader {
			data_file: data_file,
			chunk_pos: chunk_start as i64,
			chunk_start: chunk_start as i64,
			chunk_end: chunk_end as i64,
		}
	}
}

impl<'a> Reader for ChunkReader<'a> {
	fn read(&mut self, buf: &mut [u8]) -> Option<uint> {
		if self.chunk_pos >= self.chunk_end {
			return None;
		}
		let max_read = min(buf.len(), (self.chunk_end - self.chunk_pos) as uint);

		match self.data_file.read(buf.mut_slice_to(max_read)) {
			Some(num_read) => {
				self.chunk_pos += num_read as i64;
				Some(num_read)
			},
			None => None,
		}
	}
}
