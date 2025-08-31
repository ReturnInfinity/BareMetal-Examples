## 03-hello-world-http

### Description:

A C example that implements a very basic TCP/IP stack (with optional DHCP client) and web server. Based on [minIP](https://github.com/IanSeyler/minIP)

### Prerequisites:

- `libBareMetal.c` and `libBareMetal.h`. The `build.sh` script will download them.

### Compile:
```sh
./build.sh
```

### Notes:

The binary for this application is just under 4KiB.

If DHCP isn't required you can remove the commented section `-DNO_DHCP` in `build.sh`. Make sure to set the correct IP values for `src_IP`, `src_SN`, and `src_GW` in `hello_http.c`.
