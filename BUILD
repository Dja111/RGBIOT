load("@AvrToolchain//:helpers.bzl", "cpu_frequency_flag", "default_embedded_binary", "mcu_avr_gcc_flag")

constraint_setting(
    name = "debug",
)

constraint_value(
    name = "debug_enabled",
    constraint_setting = "debug",
)

config_setting(
    name = "debug_config",
    constraint_values = [
        ":debug_enabled",
    ],
)

platform(
    name = "debug_platform",
    constraint_values = [
        ":debug_enabled",
    ],
    visibility = [
        "//visibility:public",
    ],
)

default_embedded_binary(
    name = "RGBIOT",
    srcs = [
    	"include/messageAdapter.h",
        "include/PinPortMapping.h",
        "include/Timer.h",
        "include/neo_pixel.h",
        "include/millis.h",
        "src/messageAdapter.c",
        "src/PinPortMapping.c",
        "src/Timer.c",
        "src/main.c",
        "src/neo_pixel.c",
        "src/millis.c",
        "src/Setup/HardwareSetup.h",
    ],
    copts = mcu_avr_gcc_flag() + cpu_frequency_flag() +
            select({
                ":debug_config": ["-DDEBUG=1"],
                "//conditions:default": ["-DDEBUG=0"],
            }),
    linkopts = mcu_avr_gcc_flag(),
    deps = [
        "@CommunicationModule//integration_tests:MotherboardSetup",
        "@EmbeddedUtilities//:PeriodicScheduler",
        "@RemoteResourceFramework//:R2FLibStaticResources",
    ],
)

