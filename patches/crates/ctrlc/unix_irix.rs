// IRIX replacement for ctrlc unix module.
// Uses raw libc calls instead of nix (which doesn't support IRIX).

use crate::error::Error as CtrlcError;
use std::io;

#[allow(static_mut_refs)]
mod implementation {
    static mut SEMAPHORE: libc::sem_t = unsafe { std::mem::zeroed() };

    pub unsafe fn sem_init() {
        libc::sem_init(&mut SEMAPHORE as *mut _, 0, 0);
    }

    pub unsafe fn sem_post() {
        let _ = libc::sem_post(&mut SEMAPHORE as *mut _);
    }

    pub unsafe fn sem_wait_forever() {
        while libc::sem_wait(&mut SEMAPHORE as *mut _) == -1 {}
    }
}

/// Platform specific error type
#[derive(Debug, PartialEq)]
pub struct Error(i32);

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "OS error {}", self.0)
    }
}

impl std::error::Error for Error {}

impl Error {
    pub const EEXIST: Self = Self(libc::EEXIST);
}

/// Platform specific signal type (unused in practice for IRIX)
#[derive(Debug, Clone, Copy)]
pub struct Signal(libc::c_int);

extern "C" fn os_handler(_: libc::c_int) {
    unsafe {
        implementation::sem_post();
    }
}

fn last_os_error() -> i32 {
    io::Error::last_os_error().raw_os_error().unwrap_or(0)
}

#[inline]
pub unsafe fn init_os_handler(overwrite: bool) -> Result<(), Error> {
    implementation::sem_init();

    let mut new_action: libc::sigaction = std::mem::zeroed();
    // IRIX sigaction uses sa_sigaction field (union with sa_handler)
    new_action.sa_sigaction = os_handler as libc::sighandler_t;
    new_action.sa_flags = libc::SA_RESTART;
    libc::sigemptyset(&mut new_action.sa_mask);

    // Install SIGINT handler
    let mut old_action: libc::sigaction = std::mem::zeroed();
    if libc::sigaction(libc::SIGINT, &new_action, &mut old_action) != 0 {
        return Err(Error(last_os_error()));
    }
    if !overwrite && old_action.sa_sigaction != libc::SIG_DFL {
        libc::sigaction(libc::SIGINT, &old_action, std::ptr::null_mut());
        return Err(Error::EEXIST);
    }

    // Install SIGTERM handler
    let mut old_term: libc::sigaction = std::mem::zeroed();
    if libc::sigaction(libc::SIGTERM, &new_action, &mut old_term) != 0 {
        libc::sigaction(libc::SIGINT, &old_action, std::ptr::null_mut());
        return Err(Error(last_os_error()));
    }
    if !overwrite && old_term.sa_sigaction != libc::SIG_DFL {
        libc::sigaction(libc::SIGINT, &old_action, std::ptr::null_mut());
        libc::sigaction(libc::SIGTERM, &old_term, std::ptr::null_mut());
        return Err(Error::EEXIST);
    }

    // Install SIGHUP handler
    let mut old_hup: libc::sigaction = std::mem::zeroed();
    if libc::sigaction(libc::SIGHUP, &new_action, &mut old_hup) != 0 {
        libc::sigaction(libc::SIGINT, &old_action, std::ptr::null_mut());
        libc::sigaction(libc::SIGTERM, &old_term, std::ptr::null_mut());
        return Err(Error(last_os_error()));
    }
    if !overwrite && old_hup.sa_sigaction != libc::SIG_DFL {
        libc::sigaction(libc::SIGINT, &old_action, std::ptr::null_mut());
        libc::sigaction(libc::SIGTERM, &old_term, std::ptr::null_mut());
        libc::sigaction(libc::SIGHUP, &old_hup, std::ptr::null_mut());
        return Err(Error::EEXIST);
    }

    Ok(())
}

#[inline]
pub unsafe fn block_ctrl_c() -> Result<(), CtrlcError> {
    implementation::sem_wait_forever();
    Ok(())
}
