.PHONY: test

test:
	cmake -S libs/tests -B build/udp-server-tests
	cmake --build build/udp-server-tests
	ctest --test-dir build/udp-server-tests --output-on-failure