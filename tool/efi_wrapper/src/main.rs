#![no_main]
#![no_std]
#![feature(const_option)]
#![feature(const_int_from_str)]

use core::slice;
use uefi::boot::{AllocateType, MemoryType};
use uefi::prelude::*;

const image_bytes: &[u8] = include_bytes!(env!("LOADER_FILE"));
const load_addr: u64 = match u64::from_str_radix(
    const_str::unwrap!(const_str::strip_prefix!(env!("LOAD_ADDRESS"), "0x")),
    16,
) {
    Ok(x) => x,
    Err(_) => panic!("Invalid hex string was provided for load address"),
};

#[entry]
fn main(_image_handle: Handle, system_table: SystemTable<Boot>) -> Status {
    uefi::helpers::init().unwrap();

    /* Allocate the region that we will load the image into */
    // TODO: What if this conflicts with where UEFI has loaded us (or something else that UEFI is using?)
    // We should:
    //      1. Allocate a random region of memory with size that is big enough to store the image
    //      2. Load the image there
    //      3. Have an assembly routine at the start of the image that relocates it to where it is meant to be executing prior to jumping to main.
    let image_buf_raw = system_table
        .boot_services()
        .allocate_pages(
            AllocateType::Address(load_addr),
            MemoryType::LOADER_DATA,
            (image_bytes.len() / 4096) + 1,
        )
        .expect("Failed to allocate load region");

    let image_slice = unsafe { slice::from_raw_parts_mut(load_addr as *mut u8, image_bytes.len()) };

    /* Exit boot services  */
    unsafe { system_table.exit_boot_services(MemoryType::LOADER_DATA) };

    image_slice.copy_from_slice(&image_bytes);

    /* The image is loaded, so we now jump to it */
    let kernel_start: unsafe extern "C" fn() = unsafe { core::mem::transmute(load_addr) };
    unsafe { (kernel_start)() };

    return Status::SUCCESS;
}
