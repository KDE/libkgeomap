
add_definitions(-DKGEOMAP_MARBLE_ADD_LAYER)
set(backend_map_marble_LIB_SRCS
    backend_map_marble.cpp
    backend_map_marble_layer.cpp
)

set(backend_map_googlemaps_LIB_SRCS
    backend_map_googlemaps.cpp
)

set(backend_map_osm_LIB_SRCS
    #backend_map_osm.cpp
)

add_library(mapbackends STATIC
    backend_map.cpp
    html_widget.cpp
    ${backend_map_marble_LIB_SRCS}
    ${backend_map_googlemaps_LIB_SRCS}
    ${backend_map_osm_LIB_SRCS}
)

target_link_libraries(mapbackends
    PRIVATE
        KF5::ConfigCore

        Qt5::Core
        Qt5::Gui # TBD: Remove Qt5::Gui after Marble is fixed to export it as public link library
        Qt5::Widgets
        Qt5::WebKitWidgets
)
