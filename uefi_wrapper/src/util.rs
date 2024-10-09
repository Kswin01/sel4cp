macro_rules! const_cstr {
    ($l:expr) => {
        unsafe { ::core::ffi::CStr::from_bytes_with_nul_unchecked(concat!($l, "\0").as_bytes()) }
    };
}

macro_rules! unwrap_or_continue {
    ($opt: expr) => {
        match $opt {
            Ok(v) => v,
            Err(_) => {
                continue;
            }
        }
    };
}