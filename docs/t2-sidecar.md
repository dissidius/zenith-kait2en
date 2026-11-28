# Experimental Apple T2 Sidecar encoder port

This branch ports the V4L2 M2M implementation from [deqrocks/Sunshine](https://github.com/deqrocks/Sunshine/tree/t2-sidecar) (source snapshot `0737eece2ec42d23d956b7900ca0f46cd6507572`, based on Sunshine PR #5717) into Zenith. Zenith's virtual-display management remains separate from the encoder.

## Build

Linux builds with `SUNSHINE_ENABLE_V4L2=ON` select the source fork's pinned LizardByte/build-deps release `v2026.1017.131201` because the V4L2 zero-copy wrapper requires patched FFmpeg private headers and symbols. Use a clean build directory. A custom `FFMPEG_PREPARED_BINARIES` must contain compatible patched FFmpeg libraries. Configure with `-DSUNSHINE_ENABLE_V4L2=OFF` to use Zenith's original FFmpeg dependencies.

## Hardware smoke test on KAIT2EN

1. Confirm that `v4l2-ctl --list-devices` reports the T2 Sidecar encoder and that the account running Zenith can access it.
2. Stop other Sunshine-compatible hosts before starting Zenith.
3. Select the V4L2 encoder in the Web UI or set the following in Zenith's configuration:
   ```ini
   encoder = v4l2m2m
   hevc_mode = 1
   av1_mode = 1
   ```
4. Start with H.264 at 1920x1080, 60 FPS. KAIT2EN documentation recommends H.264 to avoid FFmpeg choosing the Apple AVE encoder for HEVC instead of Sidecar.
5. Test a normal GNOME capture first; then test Zenith's existing virtual-display creation, correct display selection, resolution matching, disconnect/reconnect, and restoration.
6. Inspect the host logs to confirm V4L2 hardware encoding and the device actually selected. Compare latency, temperature and frame pacing with VA-API.

This is an experimental source port, not a verified headless/T2 combination. Actual hardware tests and build/CI validation are required. The T2 encoder alone does not create a virtual display or remove GNOME session/portal constraints.

Reference: https://kait2en.org/documentation.html


## KAIT2EN capture and recovery test build

The initial hardware test confirmed Intel UHD 630 EGL conversion plus T2
H.264 encoding using Portal capture. Disabling the system tray avoided
the observed launch hang. The tested working configuration was:

```ini
capture = portal
encoder = v4l2m2m
adapter_name = /dev/dri/by-path/pci-0000:00:02.0-render
hevc_mode = 1
av1_mode = 1
system_tray = disabled
```

Verify that this PCI path resolves to the Intel render node on your host.
The next experimental KMS test changes only `capture = kms`; test the
ordinary desktop first, then Extend, then Headless. Keep a local console or
SSH recovery route available. Portal capture cannot automatically authorize
a new virtual monitor; the new strict target checks reject a mismatched
portal stream rather than silently mirroring the physical display.

The VDD launch path now validates capture after preparation and attempts
desktop restoration on launch timeout, last-client disconnect, and native
service stop/restart. A failed or interrupted VDD session must be launched
again instead of resumed. The native service also attempts to recover saved
Zenith display state after an abnormal exit. These changes still require
hardware validation and are not a guarantee against a kernel/compositor hang.

Emergency recovery from a GNOME terminal: `zenith-display restore`.
From a logged-in text console on GNOME/Wayland:

```sh
systemctl --user stop app-io.github.jacksonpate.Zenith.service
XDG_SESSION_TYPE=wayland XDG_CURRENT_DESKTOP=GNOME WAYLAND_DISPLAY=wayland-0 zenith-display --verbose restore
sudo chvt 2
```

Use the actual graphical VT if different. Do not use headless again until
normal KMS capture and Extend have been verified. IDR request timing and
sustained frame pacing remain open issues.
