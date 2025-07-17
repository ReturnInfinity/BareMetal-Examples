// Rust on BareMetal - tested on rust 1.87.0
// rustc -O --crate-type lib -o rust.o --emit obj rust.rs
// ld -T app.ld -o rust.app rust.o

// do not include the standard library
#![no_std]
#![no_main]

use core::arch::asm;
use core::panic::PanicInfo;

// println macro that calls b_output
#[macro_export]
macro_rules! println {
	() => {
		let s = "\n";
		b_output(s.as_ptr(), s.len());
	};
	($fmt:expr) => {
		let s = concat!($fmt, "\n");
		b_output(s.as_ptr(), s.len());
	};
}

// Main entry
#[unsafe(no_mangle)]
pub extern "C" fn _start() {
	println!("Hello world from Rust!");
}

// BareMetal b_output wrapper
fn b_output(string_pointer: *const u8, string_length: usize) {
	unsafe {
		asm!(
			"mov rsi, {0}", // string address
			"mov rcx, {1}", // string length
			"call [0x00100018]", // b_output
			in(reg) {string_pointer},
			in(reg) {string_length}
		);
	}
}

// This function is called during panic
#[panic_handler]
fn panic(_info: &PanicInfo) -> !
{
	loop {}
}
