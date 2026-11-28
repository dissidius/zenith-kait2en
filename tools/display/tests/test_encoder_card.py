"""Which GPU will encode — the question the VDD placement depends on.

Zenith's encoder list is ``{nvenc, vaapi, software}`` and the first that
validates wins (``src/video.cpp``). So on a hybrid laptop the dGPU takes the
session even though the iGPU composites the desktop, and the virtual display
belongs on the dGPU's spare port rather than the iGPU's lowest-numbered one.
Get this wrong and capture round-trips every frame through system memory.
"""

from zenith_display.detect import (
    VENDOR_AMD,
    VENDOR_INTEL,
    VENDOR_NVIDIA,
    Connector,
    configured_v4l2_card,
    encoder_card,
)


def _c(name, card, driver, vendor):
    return Connector(sysfs=f"/sys/class/drm/{card}-{name}", name=name,
                     status="disconnected", enabled=False, driver=driver,
                     card=card, vendor=vendor)


def test_nvidia_takes_the_session_on_a_hybrid_laptop():
    """The measured case: RTX 4050 on card0, Radeon 680M on card1, KDE
    composited by AMD. NVENC still encodes, so card0 is the card that matters."""
    connectors = [
        _c("DP-6", "card0", "nvidia", VENDOR_NVIDIA),
        _c("DP-1", "card1", "amdgpu", VENDOR_AMD),
    ]
    assert encoder_card(connectors, nv_version="580.65") == "card0"


def test_amd_encodes_when_no_nvidia_driver_is_loaded():
    connectors = [_c("DP-1", "card1", "amdgpu", VENDOR_AMD)]
    assert encoder_card(connectors, nv_version="") == "card1"


def test_a_too_old_nvidia_driver_does_not_get_the_session():
    """Below the bundled nvenc's minimum the NVIDIA encoder refuses and Zenith
    falls through to vaapi — so the AMD card is the one that will encode, and
    the VDD belongs there instead."""
    connectors = [
        _c("DP-6", "card0", "nvidia", VENDOR_NVIDIA),
        _c("DP-1", "card1", "amdgpu", VENDOR_AMD),
    ]
    assert encoder_card(connectors, nv_version="470.10") == "card1"


def test_intel_igpu_alone_encodes_via_vaapi():
    connectors = [_c("DP-1", "card0", "i915", VENDOR_INTEL)]
    assert encoder_card(connectors, nv_version="") == "card0"


def test_no_hardware_encoder_means_no_card_to_prefer():
    """A virtual GPU: software encode, and nothing to rank connectors by."""
    connectors = [_c("Virtual-1", "card0", "virtio_gpu", "0x1af4")]
    assert encoder_card(connectors, nv_version="") == ""


def test_nvidia_identified_by_vendor_id_not_only_driver_name():
    """The DRM driver link reads `nvidia` on some kernels and `nvidia-drm` on
    others; the PCI vendor id is the same either way."""
    connectors = [_c("DP-6", "card0", "nvidia", VENDOR_NVIDIA)]
    assert encoder_card(connectors, nv_version="580.65") == "card0"



def test_v4l2_uses_configured_render_device_not_card_order(tmp_path):
    config = tmp_path / "config" / "sunshine"
    config.mkdir(parents=True)
    (config / "zenith.conf").write_text(
        "encoder = v4l2m2m\nadapter_name = /dev/dri/renderD128\n")
    drm = tmp_path / "drm"
    intel = tmp_path / "intel"
    amd = tmp_path / "amd"
    intel.mkdir()
    amd.mkdir()
    for node, device in (("card0", amd), ("card1", intel), ("renderD128", intel)):
        (drm / node).mkdir(parents=True)
        (drm / node / "device").symlink_to(device)
    connectors = [_c("DP-4", "card0", "amdgpu", VENDOR_AMD),
                  _c("HDMI-A-1", "card1", "i915", VENDOR_INTEL)]
    assert configured_v4l2_card(connectors, {"XDG_CONFIG_HOME": str(config.parent)},
                                str(drm)) == "card1"


def test_v4l2_card_does_not_override_other_encoders(tmp_path):
    config = tmp_path / "sunshine"
    config.mkdir()
    (config / "zenith.conf").write_text("encoder = vaapi\n")
    assert configured_v4l2_card([], {"XDG_CONFIG_HOME": str(tmp_path)}) == ""
