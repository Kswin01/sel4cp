#![no_main]
#![no_std]

#[macro_use]
mod util;

use core::{ffi::CStr, str};
use core::mem::MaybeUninit;
use log::info;
use uefi::{boot::{open_protocol_exclusive, AllocateType, MemoryType, ScopedProtocol}, mem::memory_map::MemoryMap, prelude::*, system, table::boot};
use uefi::{prelude::*, Guid};
use uefi::proto::loaded_image::LoadedImage;
use linked_list_allocator::LockedHeap;
use core::{arch::asm, slice};
use goblin::pe::{
    section_table::{IMAGE_SCN_MEM_EXECUTE, IMAGE_SCN_MEM_READ},
    PE,
};

const load_addr_str: &str = env!("LOAD_ADDRESS");

#[global_allocator]
static ALLOCATOR: LockedHeap = LockedHeap::empty();

#[entry]
fn main(_image_handle: Handle, system_table: SystemTable<Boot>) -> Status {
    uefi::helpers::init().unwrap();
    info!("This is the load address env var: {}", load_addr_str);
    /* Get the load address that the loader image expected to be copied into. */
    let load_addr: u64 = u64::from_str_radix(load_addr_str.strip_prefix("0x").unwrap(), 16).expect("Load address was not a valid hex string");

    /* Allocating our own heap due to the absecnce of a heap allocator in no_std. This is required by the Goblin crate. */
    let heap_res = system_table.boot_services().allocate_pages(AllocateType::AnyPages, MemoryType::LOADER_DATA, 10).expect("Failed to alloc");

    unsafe {
        ALLOCATOR
            .lock()
            .init(heap_res as *mut u8, 10 * 4096);
    }

    /* Open the protocol */
    let proto: ScopedProtocol<LoadedImage> = open_protocol_exclusive(_image_handle).unwrap();
    /* Get the image info. This will include name, virtual address, size, etc... */
    let image_info: (*const core::ffi::c_void, u64) = proto.info();

    let sl = unsafe { slice::from_raw_parts(image_info.0 as *mut u8, image_info.1 as usize) };

    let pe_parsed: PE<'_> = PE::parse(sl).unwrap();

    for section in pe_parsed.sections {
        let name = unsafe{ CStr::from_ptr(section.name.as_ptr() as *const i8) };
        if name.to_str().unwrap() == ".mloader" {
            info!("Found the mloader section!");
            /* Allocate the region that we will load the image into */
            // TODO: What if this conflicts with where UEFI has loaded us (or something else that UEFI is using?)
            // We should:
            //      1. Allocate a random region of memory with size that is big enough to store the image
            //      2. Load the image there
            //      3. Have an assembly routine at the start of the image that relocates it to where it is meant to be executing prior to jumping to main.
            let allocate_res = system_table.boot_services().allocate_pages(AllocateType::Address(load_addr), MemoryType::LOADER_DATA, (section.virtual_size as usize / 4096) + 1).expect("Failed to alloc");
            /* Define the region as a slice so that we can access it */
            let load_region: &mut [u8] = unsafe {slice::from_raw_parts_mut(load_addr as *mut u8, section.virtual_size as usize)};
            /* Define the mloader section as a slice as well. */
            let section_data: &mut [u8] = unsafe {slice::from_raw_parts_mut((image_info.0 as u64 + section.virtual_address as u64) as *mut u8, section.virtual_size as usize)};
            /* Copy the image into the correct memory region */
            load_region.copy_from_slice(&section_data);
            break;
        }
    }

    /* Exit boot services  */
    unsafe{system_table.exit_boot_services(MemoryType::LOADER_DATA)};

    /* Jump to the loader  */
    let kernel_start: unsafe extern "C" fn() = unsafe { core::mem::transmute(load_addr)};
    unsafe { (kernel_start)() };

    return Status::SUCCESS
}
