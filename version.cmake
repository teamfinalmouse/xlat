# Define the application version
set(APP_VERSION_MAJOR 1)
set(APP_VERSION_MINOR 5)
set(APP_VERSION_PATCH 99) # patch level 99 = work-in-progress for next release
set(APP_VERSION_STRING "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_PATCH}")

# Get the GIT SHA
execute_process(
        COMMAND git rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Define version-related compile definitions
add_definitions(
        -DAPP_VERSION_MAJOR=${APP_VERSION_MAJOR}
        -DAPP_VERSION_MINOR=${APP_VERSION_MINOR}
        -DAPP_VERSION_PATCH=${APP_VERSION_PATCH}
        -DAPP_VERSION="${APP_VERSION_STRING}"
        -DAPP_VERSION_FULL="${APP_VERSION_STRING}-${GIT_SHA}"
        -DGIT_SHA="${GIT_SHA}"
) 