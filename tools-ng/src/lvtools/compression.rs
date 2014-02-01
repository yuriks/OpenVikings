use std::vec;

pub fn decompress_data<T: Reader>(src: &mut T, decoded_data_len: uint) -> ~[u8] {
	let mut scratch = [0u8, ..4*1024];
	let mut dst = vec::with_capacity(decoded_data_len);

	let mut scratch_i = 0;

	loop {
		let mut al = src.read_u8();

		for _ in range(0, 8) {
			if al & 1 != 0 {
				// Verbatim byte
				let symbol = src.read_u8();

				scratch[scratch_i] = symbol;
				scratch_i = (scratch_i + 1) % scratch.len();

				dst.push(symbol);
				if dst.len() == decoded_data_len {
					return dst;
				}
			} else {
				// Copy bytes from scratch area
				let copy_op = src.read_le_u16();

				let copy_len = ((copy_op >> 12) + 3) as uint;
				let mut copy_src = (copy_op & 0xFFF) as uint;

				for _ in range(0, copy_len) {
					let symbol = scratch[copy_src];
					copy_src = (copy_src + 1) % scratch.len();

					scratch[scratch_i] = symbol;
					scratch_i = (scratch_i + 1) % scratch.len();

					dst.push(symbol);
					if dst.len() == decoded_data_len {
						return dst;
					}
				}
			}

			al = al >> 1;
		}
	}
}
