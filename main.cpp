#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <algorithm>
#include <filesystem>
#include <span>

#include <fcntl.h>
#include <sys/types.h>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

constexpr unsigned c_rcvsize = 16 * 1024 * 1024; // 16 MiB recovery file

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::printf("Usage: rrtar [ARCHIVE.rr.tar]\n");
		return 1;
	}

	int fd = open(argv[1], O_RDWR);
	if (fd <= 0) {
		perror("Failed to open archive");
		return 1;
	}

	std::size_t sz = std::filesystem::file_size(argv[1]);
	if (sz <= c_rcvsize) {
		perror("Archive is too small");
		return 1;
	}
	auto raw_ptr = mmap(0, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!raw_ptr) {
		perror("Failed to mmap archive");
		return 1;
	} else {
		auto data = std::span<std::byte>(reinterpret_cast<std::byte*>(raw_ptr), sz - c_rcvsize);
		auto rcv = std::span<std::byte>(reinterpret_cast<std::byte*>(raw_ptr) + sz - c_rcvsize, c_rcvsize);
		for (std::size_t i = 0; i < rcv.size(); i++) {
			if (rcv[i] != std::byte(0)) {
				std::fprintf(stderr, "Recovery data is not empty, invalid archive");
				return 1;
			}
		}
		for (std::size_t i = 0; i < data.size(); i++) {
			// Any linear damage of the size up to c_rcvsize should be mathematically recoverable
			rcv[(i - sz) % c_rcvsize] ^= data[i];
		}
	}

	munmap(raw_ptr, sz);
	fsync(fd);
	close(fd);
	return 0;
}
