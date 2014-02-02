use std::num::{from_str_radix, FromStrRadix};

pub fn from_str_multiradix<T: FromStrRadix>(s: &str) -> Option<T> {
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
