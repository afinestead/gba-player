idf_component_register(
	SRCS "src/udp_server.cpp"
	INCLUDE_DIRS "include" "${CMAKE_CURRENT_LIST_DIR}/../.."
	REQUIRES freertos esp_common
)
