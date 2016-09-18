
add_definitions(-DKGEOMAP_MARBLE_ADD_LAYER)
set(backend_map_marble_LIB_SRCS
    backendmarble.cpp
    backendmarblelayer.cpp
)

set(backend_map_googlemaps_LIB_SRCS
    backendgooglemaps.cpp
)

set(backend_map_osm_LIB_SRCS
    #backend_map_osm.cpp
)

add_library(mapbackends STATIC
    mapbackend.cpp
    htmlwidget.cpp
    ${backend_map_marble_LIB_SRCS}
    ${backend_map_googlemaps_LIB_SRCS}
    ${backend_map_osm_LIB_SRCS}
)

target_link_libraries(mapbackends
    PUBLIC
        Marble
    PRIVATE
        KF5::ConfigCore
        KF5::I18n

        Qt5::Widgets
        Qt5::WebKitWidgets
)
