from conan import ConanFile


class VoiceAVLearningConan(ConanFile):
    name = "voice_av_learning"
    version = "0.1.0"
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "ffmpeg_flavor": ["light", "full"],
    }
    default_options = {
        "ffmpeg_flavor": "light",
        "ffmpeg/*:shared": False,
        "ffmpeg/*:fPIC": True,
        "ffmpeg/*:avdevice": True,
        "ffmpeg/*:avcodec": True,
        "ffmpeg/*:avformat": True,
        "ffmpeg/*:swresample": True,
        "ffmpeg/*:swscale": True,
        "ffmpeg/*:postproc": True,
        "ffmpeg/*:avfilter": True,
        "ffmpeg/*:with_asm": True,
        "ffmpeg/*:with_zlib": True,
        "ffmpeg/*:with_bzip2": True,
        "ffmpeg/*:with_lzma": True,
        "ffmpeg/*:with_libiconv": True,
        "ffmpeg/*:with_freetype": True,
        "ffmpeg/*:with_libxml2": False,
        "ffmpeg/*:with_fontconfig": False,
        "ffmpeg/*:with_fribidi": False,
        "ffmpeg/*:with_harfbuzz": False,
        "ffmpeg/*:with_libjxl": False,
        "ffmpeg/*:with_openapv": False,
        "ffmpeg/*:with_openjpeg": True,
        "ffmpeg/*:with_openh264": True,
        "ffmpeg/*:with_opus": True,
        "ffmpeg/*:with_vorbis": True,
        "ffmpeg/*:with_zeromq": False,
        "ffmpeg/*:with_sdl": False,
        "ffmpeg/*:with_libx264": True,
        "ffmpeg/*:with_libx265": True,
        "ffmpeg/*:with_libvpx": True,
        "ffmpeg/*:with_libmp3lame": True,
        "ffmpeg/*:with_libfdk_aac": True,
        "ffmpeg/*:with_libwebp": True,
        "ffmpeg/*:with_ssl": "openssl",
        "ffmpeg/*:with_libalsa": True,
        "ffmpeg/*:with_pulse": True,
        "ffmpeg/*:with_vaapi": True,
        "ffmpeg/*:with_vdpau": True,
        "ffmpeg/*:with_vulkan": False,
        "ffmpeg/*:with_whisper": False,
        "ffmpeg/*:with_xcb": True,
        "ffmpeg/*:with_soxr": False,
        "ffmpeg/*:with_appkit": True,
        "ffmpeg/*:with_avfoundation": True,
        "ffmpeg/*:with_coreimage": True,
        "ffmpeg/*:with_audiotoolbox": True,
        "ffmpeg/*:with_videotoolbox": True,
        "ffmpeg/*:with_programs": True,
        "ffmpeg/*:with_libsvtav1": True,
        "ffmpeg/*:with_libaom": True,
        "ffmpeg/*:with_libdav1d": True,
        "ffmpeg/*:with_libdrm": False,
        "ffmpeg/*:with_jni": False,
        "ffmpeg/*:with_mediacodec": False,
        "ffmpeg/*:with_xlib": True,
        "ffmpeg/*:disable_everything": False,
        "ffmpeg/*:disable_all_encoders": False,
        "ffmpeg/*:disable_encoders": None,
        "ffmpeg/*:enable_encoders": None,
        "ffmpeg/*:disable_all_decoders": False,
        "ffmpeg/*:disable_decoders": None,
        "ffmpeg/*:enable_decoders": None,
        "ffmpeg/*:disable_all_hardware_accelerators": False,
        "ffmpeg/*:disable_hardware_accelerators": None,
        "ffmpeg/*:enable_hardware_accelerators": None,
        "ffmpeg/*:disable_all_muxers": False,
        "ffmpeg/*:disable_muxers": None,
        "ffmpeg/*:enable_muxers": None,
        "ffmpeg/*:disable_all_demuxers": False,
        "ffmpeg/*:disable_demuxers": None,
        "ffmpeg/*:enable_demuxers": None,
        "ffmpeg/*:disable_all_parsers": False,
        "ffmpeg/*:disable_parsers": None,
        "ffmpeg/*:enable_parsers": None,
        "ffmpeg/*:disable_all_bitstream_filters": False,
        "ffmpeg/*:disable_bitstream_filters": None,
        "ffmpeg/*:enable_bitstream_filters": None,
        "ffmpeg/*:disable_all_protocols": False,
        "ffmpeg/*:disable_protocols": None,
        "ffmpeg/*:enable_protocols": None,
        "ffmpeg/*:disable_all_devices": False,
        "ffmpeg/*:disable_all_input_devices": False,
        "ffmpeg/*:disable_input_devices": None,
        "ffmpeg/*:enable_input_devices": None,
        "ffmpeg/*:disable_all_output_devices": False,
        "ffmpeg/*:disable_output_devices": None,
        "ffmpeg/*:enable_output_devices": None,
        "ffmpeg/*:disable_all_filters": False,
        "ffmpeg/*:disable_filters": None,
        "ffmpeg/*:enable_filters": None,
    }
    generators = "CMakeDeps", "CMakeToolchain", "VirtualRunEnv"

    def requirements(self):
        # Conan Center does not publish an exact ffmpeg/7.1 recipe. 7.1.3 is
        # the latest available patch release in the 7.1 series.
        self.requires("ffmpeg/7.1.3")

    def layout(self):
        platform_name = str(self.settings.os).lower()
        self.folders.build = f"build/{platform_name}"
        self.folders.generators = f"build/{platform_name}/generators"

    def configure(self):
        ffmpeg = self.options["ffmpeg"]

        if self.options.ffmpeg_flavor == "light":
            self._configure_light_ffmpeg(ffmpeg)
        else:
            self._configure_full_ffmpeg(ffmpeg)

    def _configure_light_ffmpeg(self, ffmpeg):
        # Keep Conan Center's default ffmpeg/7.1.3 options. The dependency
        # scripts choose a compiler profile that matches available binaries.
        ffmpeg.shared = False

    def _configure_full_ffmpeg(self, ffmpeg):
        # Static FFmpeg libraries, matching --enable-static.
        ffmpeg.shared = False

        # Core FFmpeg libraries.
        ffmpeg.avdevice = True
        ffmpeg.avcodec = True
        ffmpeg.avformat = True
        ffmpeg.avfilter = True
        ffmpeg.swresample = True
        ffmpeg.swscale = True
        ffmpeg.postproc = True
        ffmpeg.with_programs = True

        # Conan Center options that map directly to requested configure flags.
        ffmpeg.with_bzip2 = True
        ffmpeg.with_fontconfig = True
        ffmpeg.with_freetype = True
        ffmpeg.with_fribidi = True
        ffmpeg.with_harfbuzz = True
        ffmpeg.with_libaom = True
        ffmpeg.with_libdav1d = False
        ffmpeg.with_libfdk_aac = False
        ffmpeg.with_libiconv = True
        ffmpeg.with_libmp3lame = True
        ffmpeg.with_libsvtav1 = False
        ffmpeg.with_libvpx = True
        ffmpeg.with_libwebp = True
        ffmpeg.with_libx264 = True
        ffmpeg.with_libx265 = True
        ffmpeg.with_libxml2 = True
        ffmpeg.with_lzma = True
        ffmpeg.with_openh264 = False
        ffmpeg.with_openjpeg = True
        ffmpeg.with_opus = True
        ffmpeg.with_sdl = True
        ffmpeg.with_vorbis = True
        # The Windows/MSVC recipe path has trouble detecting libzmq through
        # pkg-config, even when Conan has resolved the dependency.
        ffmpeg.with_zeromq = self.settings.os != "Windows"
        ffmpeg.with_zlib = True

        # The Conan Center recipe does not expose gnutls. It supports openssl
        # or securetransport; use openssl so HTTPS/TLS protocols are enabled.
        ffmpeg.with_ssl = "openssl"

        # Linux/FreeBSD-only hardware and desktop integrations. Only VAAPI is
        # requested; the rest are set false to avoid recipe defaults adding
        # unrelated desktop/audio integrations.
        if self.settings.os in ("Linux", "FreeBSD"):
            ffmpeg.with_libalsa = False
            ffmpeg.with_libdrm = False
            ffmpeg.with_pulse = False
            ffmpeg.with_vaapi = True
            ffmpeg.with_vdpau = False
            ffmpeg.with_vulkan = False
            ffmpeg.with_xcb = False
            ffmpeg.with_xlib = False

        # Apple-only integrations.
        if self.settings.os == "Macos":
            ffmpeg.with_appkit = False
            ffmpeg.with_avfoundation = False
            ffmpeg.with_audiotoolbox = False
            ffmpeg.with_coreimage = False
            ffmpeg.with_videotoolbox = False
