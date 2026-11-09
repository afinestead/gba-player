idf_component_register(
	SRCS "src/nvs_driver.cpp" "src/spi_driver.cc"
	INCLUDE_DIRS "include"
	REQUIRES nvs_flash esp_driver_spi
)
