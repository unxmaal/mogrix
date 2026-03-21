//! SDL2 audio backend for cpal on IRIX.
//!
//! Uses SDL2's callback-based audio API, which is proven to work on IRIX.
//! Only output streams are supported (no input/recording).

use std::sync::{Arc, Mutex};
use std::time::Duration;

use crate::traits::{DeviceTrait, HostTrait, StreamTrait};
use crate::{
    BuildStreamError, Data, DefaultStreamConfigError, DeviceNameError, DevicesError,
    InputCallbackInfo, OutputCallbackInfo, PauseStreamError, PlayStreamError, SampleFormat,
    StreamConfig, StreamError, SupportedStreamConfig, SupportedStreamConfigRange,
    SupportedStreamConfigsError,
};

// --- SDL2 FFI bindings (minimal, just what we need for audio) ---

#[allow(non_camel_case_types)]
mod ffi {
    use std::os::raw::{c_char, c_int, c_void};

    pub const SDL_INIT_AUDIO: u32 = 0x00000010;

    pub const AUDIO_S16SYS: u16 = {
        // IRIX is big-endian MIPS
        #[cfg(target_endian = "big")]
        { 0x9010 } // AUDIO_S16MSB
        #[cfg(target_endian = "little")]
        { 0x8010 } // AUDIO_S16LSB
    };

    pub const AUDIO_F32SYS: u16 = {
        #[cfg(target_endian = "big")]
        { 0x9120 } // AUDIO_F32MSB
        #[cfg(target_endian = "little")]
        { 0x8120 } // AUDIO_F32LSB
    };

    pub type SDL_AudioCallback =
        Option<unsafe extern "C" fn(userdata: *mut c_void, stream: *mut u8, len: c_int)>;

    pub type SDL_AudioDeviceID = u32;

    #[repr(C)]
    pub struct SDL_AudioSpec {
        pub freq: c_int,
        pub format: u16,
        pub channels: u8,
        pub silence: u8,
        pub samples: u16,
        pub padding: u16,
        pub size: u32,
        pub callback: SDL_AudioCallback,
        pub userdata: *mut c_void,
    }

    unsafe impl Send for SDL_AudioSpec {}

    extern "C" {
        pub fn SDL_Init(flags: u32) -> c_int;
        pub fn SDL_InitSubSystem(flags: u32) -> c_int;
        pub fn SDL_QuitSubSystem(flags: u32);
        pub fn SDL_GetError() -> *const c_char;
        pub fn SDL_OpenAudioDevice(
            device: *const c_char,
            iscapture: c_int,
            desired: *const SDL_AudioSpec,
            obtained: *mut SDL_AudioSpec,
            allowed_changes: c_int,
        ) -> SDL_AudioDeviceID;
        pub fn SDL_CloseAudioDevice(dev: SDL_AudioDeviceID);
        pub fn SDL_PauseAudioDevice(dev: SDL_AudioDeviceID, pause_on: c_int);
    }

    pub unsafe fn sdl_error_string() -> String {
        let ptr = SDL_GetError();
        if ptr.is_null() {
            return String::from("unknown SDL error");
        }
        std::ffi::CStr::from_ptr(ptr)
            .to_string_lossy()
            .into_owned()
    }
}

// --- Stream callback state ---

struct CallbackState {
    data_callback: Box<dyn FnMut(&mut Data, &OutputCallbackInfo) + Send>,
    error_callback: Box<dyn FnMut(StreamError) + Send>,
    sample_format: SampleFormat,
}

// --- cpal types ---

#[derive(Default)]
pub struct Devices {
    yielded: bool,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Device;

pub struct Host;

pub struct Stream {
    device_id: ffi::SDL_AudioDeviceID,
    // Prevent callback state from being dropped while SDL is using it.
    // The Arc::into_raw pointer keeps SDL's userdata alive.
    _callback_state: Arc<Mutex<CallbackState>>,
}

// Stream must be Send + Sync for cpal
unsafe impl Send for Stream {}
unsafe impl Sync for Stream {}

pub struct SupportedInputConfigs;
pub struct SupportedOutputConfigs {
    yielded: bool,
}

impl Host {
    #[allow(dead_code)]
    pub fn new() -> Result<Self, crate::HostUnavailable> {
        let ret = unsafe { ffi::SDL_InitSubSystem(ffi::SDL_INIT_AUDIO) };
        if ret != 0 {
            // Try full init if subsystem init fails
            let ret = unsafe { ffi::SDL_Init(ffi::SDL_INIT_AUDIO) };
            if ret != 0 {
                return Err(crate::HostUnavailable);
            }
        }
        Ok(Host)
    }
}

impl Devices {
    pub fn new() -> Result<Self, DevicesError> {
        Ok(Devices { yielded: false })
    }
}

impl DeviceTrait for Device {
    type SupportedInputConfigs = SupportedInputConfigs;
    type SupportedOutputConfigs = SupportedOutputConfigs;
    type Stream = Stream;

    fn name(&self) -> Result<String, DeviceNameError> {
        Ok("SDL2 Audio".to_owned())
    }

    fn supported_input_configs(
        &self,
    ) -> Result<SupportedInputConfigs, SupportedStreamConfigsError> {
        Ok(SupportedInputConfigs)
    }

    fn supported_output_configs(
        &self,
    ) -> Result<SupportedOutputConfigs, SupportedStreamConfigsError> {
        Ok(SupportedOutputConfigs { yielded: false })
    }

    fn default_input_config(&self) -> Result<SupportedStreamConfig, DefaultStreamConfigError> {
        Err(DefaultStreamConfigError::StreamTypeNotSupported)
    }

    fn default_output_config(&self) -> Result<SupportedStreamConfig, DefaultStreamConfigError> {
        Ok(SupportedStreamConfig::new(
            2,                                          // stereo
            crate::SampleRate(44100),                   // 44.1 kHz
            crate::SupportedBufferSize::Range {
                min: 256,
                max: 8192,
            },
            SampleFormat::F32,
        ))
    }

    fn build_input_stream_raw<D, E>(
        &self,
        _config: &StreamConfig,
        _sample_format: SampleFormat,
        _data_callback: D,
        _error_callback: E,
        _timeout: Option<Duration>,
    ) -> Result<Self::Stream, BuildStreamError>
    where
        D: FnMut(&Data, &InputCallbackInfo) + Send + 'static,
        E: FnMut(StreamError) + Send + 'static,
    {
        Err(BuildStreamError::StreamConfigNotSupported)
    }

    fn build_output_stream_raw<D, E>(
        &self,
        config: &StreamConfig,
        sample_format: SampleFormat,
        data_callback: D,
        error_callback: E,
        _timeout: Option<Duration>,
    ) -> Result<Self::Stream, BuildStreamError>
    where
        D: FnMut(&mut Data, &OutputCallbackInfo) + Send + 'static,
        E: FnMut(StreamError) + Send + 'static,
    {
        let sdl_format = match sample_format {
            SampleFormat::I16 => ffi::AUDIO_S16SYS,
            SampleFormat::F32 => ffi::AUDIO_F32SYS,
            _ => return Err(BuildStreamError::StreamConfigNotSupported),
        };

        let channels = config.channels as u8;
        let sample_rate = config.sample_rate.0 as i32;

        // Buffer size: use requested or default to 1024
        let buffer_frames = match config.buffer_size {
            crate::BufferSize::Fixed(n) => n as u16,
            crate::BufferSize::Default => 1024,
        };

        let callback_state = Arc::new(Mutex::new(CallbackState {
            data_callback: Box::new(data_callback),
            error_callback: Box::new(error_callback),
            sample_format,
        }));

        let state_ptr = Arc::into_raw(callback_state.clone()) as *mut std::ffi::c_void;

        let desired = ffi::SDL_AudioSpec {
            freq: sample_rate,
            format: sdl_format,
            channels,
            silence: 0,
            samples: buffer_frames,
            padding: 0,
            size: 0,
            callback: Some(sdl_audio_callback),
            userdata: state_ptr,
        };

        let mut obtained = unsafe { std::mem::zeroed::<ffi::SDL_AudioSpec>() };

        let device_id = unsafe {
            ffi::SDL_OpenAudioDevice(
                std::ptr::null(), // default device
                0,                // output (not capture)
                &desired,
                &mut obtained,
                0, // no allowed changes
            )
        };

        if device_id == 0 {
            // Clean up the Arc we leaked
            unsafe { Arc::from_raw(state_ptr as *const Mutex<CallbackState>) };
            let err = unsafe { ffi::sdl_error_string() };
            return Err(BuildStreamError::BackendSpecific {
                err: crate::BackendSpecificError { description: err },
            });
        }

        Ok(Stream {
            device_id,
            _callback_state: callback_state,
        })
    }
}

unsafe extern "C" fn sdl_audio_callback(
    userdata: *mut std::ffi::c_void,
    stream: *mut u8,
    len: std::os::raw::c_int,
) {
    let state = &*(userdata as *const Mutex<CallbackState>);
    if let Ok(mut state) = state.lock() {
        let len = len as usize;
        let sample_size = state.sample_format.sample_size();
        let num_samples = len / sample_size;

        // Create a Data wrapper around the SDL buffer
        let data = std::slice::from_raw_parts_mut(stream, len);
        let mut cpal_data = Data::from_parts(data.as_mut_ptr() as *mut (), num_samples, state.sample_format);

        let zero = crate::StreamInstant::new(0, 0);
        let info = OutputCallbackInfo::new(crate::OutputStreamTimestamp {
            callback: zero,
            playback: zero,
        });

        (state.data_callback)(&mut cpal_data, &info);
    } else {
        // Lock poisoned — fill with silence
        std::ptr::write_bytes(stream, 0, len as usize);
    }
}

impl HostTrait for Host {
    type Devices = Devices;
    type Device = Device;

    fn is_available() -> bool {
        true
    }

    fn devices(&self) -> Result<Self::Devices, DevicesError> {
        Devices::new()
    }

    fn default_input_device(&self) -> Option<Device> {
        None
    }

    fn default_output_device(&self) -> Option<Device> {
        Some(Device)
    }
}

impl StreamTrait for Stream {
    fn play(&self) -> Result<(), PlayStreamError> {
        unsafe { ffi::SDL_PauseAudioDevice(self.device_id, 0) };
        Ok(())
    }

    fn pause(&self) -> Result<(), PauseStreamError> {
        unsafe { ffi::SDL_PauseAudioDevice(self.device_id, 1) };
        Ok(())
    }
}

impl Drop for Stream {
    fn drop(&mut self) {
        unsafe {
            // Pause first to stop callbacks, then close device
            ffi::SDL_PauseAudioDevice(self.device_id, 1);
            ffi::SDL_CloseAudioDevice(self.device_id);
        }
    }
}

impl Iterator for Devices {
    type Item = Device;

    fn next(&mut self) -> Option<Device> {
        if !self.yielded {
            self.yielded = true;
            Some(Device)
        } else {
            None
        }
    }
}

impl Iterator for SupportedInputConfigs {
    type Item = SupportedStreamConfigRange;

    fn next(&mut self) -> Option<SupportedStreamConfigRange> {
        None
    }
}

impl Iterator for SupportedOutputConfigs {
    type Item = SupportedStreamConfigRange;

    fn next(&mut self) -> Option<SupportedStreamConfigRange> {
        if !self.yielded {
            self.yielded = true;
            // Report support for stereo 44.1kHz F32
            Some(SupportedStreamConfigRange::new(
                2,
                crate::SampleRate(8000),
                crate::SampleRate(48000),
                crate::SupportedBufferSize::Range {
                    min: 256,
                    max: 8192,
                },
                SampleFormat::F32,
            ))
        } else {
            None
        }
    }
}
