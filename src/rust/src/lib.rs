use futures::StreamExt;
use idevice::{
    IdeviceService,
    lockdown::LockdownClient,
    usbmuxd::{Connection, UsbmuxdAddr, UsbmuxdConnection, UsbmuxdListenEvent},
};
use std::collections::HashMap;
use std::ffi::CString;
use std::os::raw::c_char;
use std::thread;
use tokio::runtime::Builder;

#[repr(C)]
pub struct IdeviceEvent {
    pub kind: i32,
    pub udid: *mut c_char,
}

pub type IdeviceEventCallback = extern "C" fn(event: *const IdeviceEvent);

fn with_event(kind: i32, udid: &str, cb: IdeviceEventCallback) {
    let c_udid = CString::new(udid).unwrap_or_default();
    let mut ev = IdeviceEvent {
        kind,
        udid: c_udid.into_raw(),
    };

    cb(&ev);

    unsafe {
        let _ = CString::from_raw(ev.udid);
    }
}

fn make_event(kind: i32, udid: &str) -> IdeviceEvent {
    let c_udid = CString::new(udid).unwrap_or_default();
    IdeviceEvent {
        kind,
        udid: c_udid.into_raw(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn idevice_event_subscribe(cb: IdeviceEventCallback) {
    thread::spawn(move || {
        let rt = Builder::new_current_thread().enable_all().build().unwrap();
        rt.block_on(async move {
            let mut device_map: HashMap<u32, String> = HashMap::new();

            loop {
                match UsbmuxdConnection::default().await {
                    Ok(mut uc) => match uc.listen().await {
                        Ok(mut stream) => {
                            while let Some(evt) = stream.next().await {
                                match evt {
                                    Ok(UsbmuxdListenEvent::Connected(d)) => {
                                        // ignore non-USB connections
                                        if d.connection_type != Connection::Usb {
                                            continue;
                                        }

                                        let udid = d.udid.clone();
                                        let device_id = d.device_id;
                                        device_map.insert(device_id, udid.clone());

                                        let cb_clone = cb;
                                        tokio::spawn(async move {
                                            let already_paired = {
                                                let mut u2 =
                                                    match UsbmuxdConnection::default().await {
                                                        Ok(u) => u,
                                                        Err(_) => return,
                                                    };

                                                match u2.get_pair_record(&udid).await {
                                                    Ok(_) => true,
                                                    Err(_) => false,
                                                }
                                            };

                                            if already_paired {
                                                with_event(1, &udid, cb_clone);
                                                return;
                                            }

                                            with_event(3, &udid, cb_clone);

                                            let ev_fail_kind = 4;
                                            with_event(ev_fail_kind, &udid, cb_clone);

                                            with_event(1, &udid, cb_clone);

                                            let mut uc2 = match UsbmuxdConnection::default().await {
                                                Ok(u) => u,
                                                Err(_) => {
                                                    let ev_fail = make_event(4, &udid);
                                                    cb_clone(&ev_fail);
                                                    return;
                                                }
                                            };

                                            let dev = match uc2.get_device(&udid).await {
                                                Ok(d) => d,
                                                Err(_) => {
                                                    let ev_fail = make_event(4, &udid);
                                                    cb_clone(&ev_fail);
                                                    return;
                                                }
                                            };
                                            let provider = dev
                                                .to_provider(UsbmuxdAddr::default(), "iDescriptor");

                                            let mut lc =
                                                match LockdownClient::connect(&provider).await {
                                                    Ok(l) => l,
                                                    Err(_) => {
                                                        let ev_fail = make_event(4, &udid);
                                                        cb_clone(&ev_fail);
                                                        return;
                                                    }
                                                };

                                            let mut buid = match uc2.get_buid().await {
                                                Ok(b) => b,
                                                Err(_) => {
                                                    let ev_fail = make_event(4, &udid);
                                                    cb_clone(&ev_fail);
                                                    return;
                                                }
                                            };

                                            if let Some(first) = buid.chars().next() {
                                                let mut chars: Vec<char> = buid.chars().collect();
                                                // https://github.com/jkcoxson/idevice_pair/blob/63b524a5/src/main.rs
                                                // Modify it slightly so iOS doesn't invalidate the one connected right now.
                                                chars[0] = if first == 'F' { 'A' } else { 'F' };
                                                buid = chars.into_iter().collect();
                                            }

                                            let host_id =
                                                uuid::Uuid::new_v4().to_string().to_uppercase();

                                            let mut pf = match lc.pair(host_id, buid, None).await {
                                                Ok(p) => p,
                                                Err(_) => {
                                                    let ev_fail = make_event(4, &udid);
                                                    cb_clone(&ev_fail);
                                                    return;
                                                }
                                            };
                                            pf.udid = Some(udid.clone());

                                            let bytes = match pf.serialize() {
                                                Ok(b) => b,
                                                Err(_) => {
                                                    let ev_fail = make_event(4, &udid);
                                                    cb_clone(&ev_fail);
                                                    return;
                                                }
                                            };

                                            if let Err(_) = uc2.save_pair_record(&udid, bytes).await
                                            {
                                                let ev_fail = make_event(4, &udid);
                                                cb_clone(&ev_fail);
                                                return;
                                            }

                                            let ev_ok = make_event(1, &udid);
                                            cb_clone(&ev_ok);
                                        });
                                    }
                                    Ok(UsbmuxdListenEvent::Disconnected(device_id)) => {
                                        if let Some(udid) = device_map.remove(&device_id) {
                                            let ev = make_event(2, &udid);
                                            cb(&ev);
                                        }
                                    }
                                    Err(e) => {
                                        eprintln!("usbmuxd listen error: {e:?}");
                                        break;
                                    }
                                }
                            }
                        }
                        Err(e) => eprintln!("Failed to start usbmuxd listen: {e:?}"),
                    },
                    // Safe to ignore as it likely means usbmuxd isn't running
                    // usbmuxd is killed when the last device disconnects
                    Err(_) => {}
                }

                tokio::time::sleep(std::time::Duration::from_millis(2000)).await;
            }
        });
    });
}
