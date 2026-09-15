/*
 * Copyright 2026 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * wcnss_filter for Samsung Galaxy A3 2017 (a3y17lte, QCA9377, Exynos 7870)
 *
 * Bridges the CAF libbt-vendor (libbt-vendor in this repo) and the QCA9377
 * combo chip on the high-speed UART:
 *
 *   - libbt-vendor powers the chip and downloads the rampatch/NV firmware
 *     itself (bt_powerup() -> rfkill, rome_soc_init() -> /dev/ttySAC0),
 *     leaving the controller running at 3 Mbaud with hardware flow control;
 *   - it then starts this service via the vendor.wc_transport.start_hci
 *     property trigger and waits for
 *     vendor.wc_transport.hci_filter_status == "1";
 *   - after that, the HCI traffic of the Bluetooth stack flows through the
 *     abstract local sockets this daemon serves:
 *       "bt_sock"          full-duplex bridge socket <-> UART
 *       "wcnssfilter_ctrl" control socket; byte 0xDD requests shutdown
 *   - init stops the service again when start_hci goes false.
 *
 * Unlike Qualcomm MSM platforms there is no WCNSS_CTRL/PIL handover here:
 * the Exynos 'bt_power' platform driver handles power/LPM in-kernel (its
 * wake peer is hooked to UART0), so this daemon only owns the UART and the
 * bridge sockets.
 */

#define LOG_TAG "wcnss_filter"

#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/* --- protocol constants (must match libbt-vendor src/bt_vendor_qcom.c) --- */
#define BT_SOCK_NAME        "bt_sock"
#define CTRL_SOCK_NAME      "wcnssfilter_ctrl"
#define STOP_WCNSS_FILTER   0xDD /* stop request sent on the ctrl socket */

#define PROP_FILTER_STATUS  "vendor.wc_transport.hci_filter_status"

/* --- hardware specifics --- */
#define BT_UART_DEV   "/dev/ttySAC0" /* Exynos 7870 UART0 <-> QCA9377 */
#define BT_UART_SPEED B3000000       /* speed after rome_soc_init()   */

#define TRANSPORT_OPEN_RETRIES  15
#define RETRY_DELAY_MS          200

#define RING_SIZE (32 * 1024)

typedef struct {
    uint8_t data[RING_SIZE];
    size_t  head; /* read position  */
    size_t  tail; /* write position */
} ring_t;

static volatile sig_atomic_t stop_requested = 0;

static void on_signal(int signo) {
    (void)signo;
    stop_requested = 1;
}

static size_t ring_used(const ring_t *r) {
    return (r->tail >= r->head) ? (r->tail - r->head)
                                : RING_SIZE - r->head + r->tail;
}

static size_t ring_free(const ring_t *r) {
    return RING_SIZE - 1 - ring_used(r);
}

/* Push as much of buf as fits; returns bytes accepted. */
static size_t ring_push(ring_t *r, const uint8_t *buf, size_t len) {
    size_t n = 0;
    while (n < len && ring_free(r) > 0) {
        r->data[r->tail] = buf[n++];
        r->tail = (r->tail + 1) % RING_SIZE;
    }
    return n;
}

/* Copy up to len bytes out of the ring without consuming. */
static size_t ring_peek(const ring_t *r, uint8_t *buf, size_t len) {
    size_t n = 0, pos = r->head, avail = ring_used(r);
    if (len > avail)
        len = avail;
    while (n < len) {
        buf[n++] = r->data[pos];
        pos = (pos + 1) % RING_SIZE;
    }
    return n;
}

static void ring_consume(ring_t *r, size_t n) {
    size_t avail = ring_used(r);
    if (n > avail)
        n = avail;
    r->head = (r->head + n) % RING_SIZE;
}

static void ring_reset(ring_t *r) {
    r->head = r->tail = 0;
}

/* --- UART ---------------------------------------------------------------- */

static int transport_open(void) {
    struct termios term;
    int fd = -1, i;

    for (i = 0; i < TRANSPORT_OPEN_RETRIES; i++) {
        fd = open(BT_UART_DEV, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (fd >= 0)
            break;
        ALOGW("%s: open %s failed (%s), retry %d/%d", __func__, BT_UART_DEV,
              strerror(errno), i + 1, TRANSPORT_OPEN_RETRIES);
        usleep(RETRY_DELAY_MS * 1000);
    }
    if (fd < 0) {
        ALOGE("%s: giving up on %s: %s", __func__, BT_UART_DEV, strerror(errno));
        return -1;
    }

    if (tcflush(fd, TCIOFLUSH) < 0) {
        ALOGE("%s: tcflush failed: %s", __func__, strerror(errno));
        close(fd);
        return -1;
    }

    if (tcgetattr(fd, &term) < 0) {
        ALOGE("%s: tcgetattr failed: %s", __func__, strerror(errno));
        close(fd);
        return -1;
    }

    cfmakeraw(&term);
    term.c_cflag |= CLOCAL | CREAD;
    term.c_cflag |= CRTSCTS; /* QCA9377 requires HW flow control */

    cfsetospeed(&term, BT_UART_SPEED);
    cfsetispeed(&term, BT_UART_SPEED);

    if (tcsetattr(fd, TCSANOW, &term) < 0) {
        ALOGE("%s: tcsetattr failed: %s", __func__, strerror(errno));
        close(fd);
        return -1;
    }

    if (tcflush(fd, TCIOFLUSH) < 0) {
        ALOGW("%s: tcflush after tcsetattr failed: %s", __func__, strerror(errno));
    }

    ALOGI("%s: %s opened at %u baud, RTS/CTS enabled", __func__, BT_UART_DEV,
          (unsigned)BT_UART_SPEED);
    return fd;
}

/* --- sockets ------------------------------------------------------------- */

static int listen_abstract(const char *name) {
    int fd = -1, i;

    for (i = 0; i < TRANSPORT_OPEN_RETRIES; i++) {
        fd = socket_local_server(name, ANDROID_SOCKET_NAMESPACE_ABSTRACT,
                                 SOCK_STREAM);
        if (fd >= 0)
            break;
        ALOGW("%s: binding '%s' failed (%s), retry %d/%d", __func__, name,
              strerror(errno), i + 1, TRANSPORT_OPEN_RETRIES);
        usleep(RETRY_DELAY_MS * 1000);
    }
    if (fd < 0)
        ALOGE("%s: giving up on '%s': %s", __func__, name, strerror(errno));

    return fd;
}

/* --- main ---------------------------------------------------------------- */

int main(void) {
    int fd_uart = -1, fd_bt_srv = -1, fd_ctrl_srv = -1;
    int fd_bt_cli = -1, fd_ctrl_cli = -1;
    int ret = EXIT_FAILURE;
    ring_t to_uart, to_client;
    bool uart_closed_logged = false;
    static uint8_t io_buf[8192];

    ring_reset(&to_uart);
    ring_reset(&to_client);

    signal(SIGTERM, on_signal);
    signal(SIGINT, on_signal);
    signal(SIGHUP, on_signal);
    signal(SIGPIPE, SIG_IGN);

    ALOGI("wcnss_filter starting (QCA9377 @ %s)", BT_UART_DEV);

    fd_uart = transport_open();
    if (fd_uart < 0)
        goto out;

    fd_bt_srv = listen_abstract(BT_SOCK_NAME);
    if (fd_bt_srv < 0)
        goto out;

    fd_ctrl_srv = listen_abstract(CTRL_SOCK_NAME);
    if (fd_ctrl_srv < 0)
        goto out;

    /* libbt-vendor is polling this property from start_hci_filter(); only
     * signal readiness once every resource it relies on is up. */
    if (property_set(PROP_FILTER_STATUS, "1") < 0) {
        ALOGE("failed to set %s=1: %s", PROP_FILTER_STATUS, strerror(errno));
        goto out;
    }
    ALOGI("bridge up: %s=1, waiting for clients on %s / %s", PROP_FILTER_STATUS,
          BT_SOCK_NAME, CTRL_SOCK_NAME);

    while (!stop_requested) {
        struct pollfd fds[5];
        int nfd = 0, idx_ctrl_srv, idx_ctrl_cli = -1, idx_bt_srv, idx_uart,
                  idx_bt_cli = -1;
        int nready;

        idx_ctrl_srv = nfd;
        fds[nfd++] = (struct pollfd){ .fd = fd_ctrl_srv, .events = POLLIN };

        if (fd_ctrl_cli >= 0) {
            idx_ctrl_cli = nfd;
            fds[nfd++] =
                (struct pollfd){ .fd = fd_ctrl_cli, .events = POLLIN };
        }

        idx_bt_srv = nfd;
        fds[nfd++] = (struct pollfd){ .fd = fd_bt_srv, .events = POLLIN };

        idx_uart = nfd;
        fds[nfd] = (struct pollfd){ .fd = fd_uart, .events = POLLIN };
        if (ring_used(&to_uart) > 0)
            fds[nfd].events |= POLLOUT;
        nfd++;

        if (fd_bt_cli >= 0) {
            idx_bt_cli = nfd;
            fds[nfd] = (struct pollfd){ .fd = fd_bt_cli, .events = POLLIN };
            if (ring_used(&to_client) > 0)
                fds[nfd].events |= POLLOUT;
            nfd++;
        }

        nready = poll(fds, nfd, -1);
        if (nready < 0) {
            if (errno == EINTR)
                continue;
            ALOGE("poll failed: %s", strerror(errno));
            break;
        }

        /* ---- control channel ------------------------------------------ */
        if (fds[idx_ctrl_srv].revents & POLLIN) {
            int n = accept(fd_ctrl_srv, NULL, NULL);
            if (n >= 0) {
                if (fd_ctrl_cli >= 0) {
                    ALOGW("replacing active ctrl connection");
                    close(fd_ctrl_cli);
                }
                fd_ctrl_cli = n;
                ALOGV("ctrl client connected (fd %d)", fd_ctrl_cli);
            }
        }
        if (idx_ctrl_cli >= 0 && (fds[idx_ctrl_cli].revents & POLLIN)) {
            uint8_t cmd;
            ssize_t rd = read(fd_ctrl_cli, &cmd, 1);
            if (rd <= 0) {
                ALOGV("ctrl client disconnected");
                close(fd_ctrl_cli);
                fd_ctrl_cli = -1;
            } else if (cmd == STOP_WCNSS_FILTER) {
                ALOGI("stop requested on ctrl socket (0x%02x)", cmd);
                stop_requested = 1;
            } else {
                ALOGW("unknown ctrl byte: 0x%02x", cmd);
            }
        }

        /* ---- bluetooth bridge: server side ---------------------------- */
        if (fds[idx_bt_srv].revents & POLLIN) {
            int n = accept(fd_bt_srv, NULL, NULL);
            if (n >= 0) {
                if (fd_bt_cli >= 0) {
                    ALOGW("new bt client replaces old one (fd %d)", fd_bt_cli);
                    close(fd_bt_cli);
                }
                fd_bt_cli = n;
                ring_reset(&to_uart);
                ring_reset(&to_client);
                ALOGI("bt client connected (fd %d)", fd_bt_cli);
                uart_closed_logged = false;
            }
        }

        /* ---- bluetooth client <-> UART ------------------------------- */
        if (idx_bt_cli >= 0 &&
            (fds[idx_bt_cli].revents & (POLLERR | POLLHUP | POLLNVAL))) {
            ALOGW("bt client error/hangup, closing (fd %d)", fd_bt_cli);
            close(fd_bt_cli);
            fd_bt_cli = -1;
        } else if (idx_bt_cli >= 0) {
            if (fds[idx_bt_cli].revents & POLLIN) {
                ssize_t rd = recv(fd_bt_cli, io_buf, sizeof(io_buf), 0);
                if (rd <= 0) {
                    ALOGI("bt client disconnected");
                    close(fd_bt_cli);
                    fd_bt_cli = -1;
                } else {
                    size_t pushed = ring_push(&to_uart, io_buf, (size_t)rd);
                    if (pushed < (size_t)rd)
                        ALOGW("to_uart ring full, dropped %zd byte(s)",
                              (ssize_t)rd - (ssize_t)pushed);
                }
            }
            if (idx_bt_cli >= 0 && fd_bt_cli >= 0 &&
                (fds[idx_bt_cli].revents & POLLOUT)) {
                size_t avail = ring_peek(&to_client, io_buf, sizeof(io_buf));
                if (avail > 0) {
                    ssize_t wr = send(fd_bt_cli, io_buf, avail, MSG_NOSIGNAL);
                    if (wr > 0) {
                        ring_consume(&to_client, (size_t)wr);
                    } else if (wr < 0 && errno != EAGAIN &&
                               errno != EWOULDBLOCK) {
                        ALOGW("send to bt client failed: %s", strerror(errno));
                        close(fd_bt_cli);
                        fd_bt_cli = -1;
                    }
                }
            }
        }

        /* ---- UART ------------------------------------------------------ */
        if (fds[idx_uart].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            ALOGE("UART error, shutting down");
            break;
        }
        if (fds[idx_uart].revents & POLLIN) {
            size_t space = (fd_bt_cli >= 0) ? ring_free(&to_client)
                                            : sizeof(io_buf);
            if (space > sizeof(io_buf))
                space = sizeof(io_buf);
            if (space > 0) {
                ssize_t rd = read(fd_uart, io_buf, space);
                if (rd > 0) {
                    if (fd_bt_cli >= 0) {
                        ring_push(&to_client, io_buf, (size_t)rd);
                    } else if (!uart_closed_logged) {
                        ALOGI("dropping %zd UART byte(s) (no bt client)", rd);
                        uart_closed_logged = true;
                    }
                } else if (rd < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    ALOGE("UART read failed: %s", strerror(errno));
                }
            }
        }
        if (fds[idx_uart].revents & POLLOUT) {
            size_t avail = ring_peek(&to_uart, io_buf, sizeof(io_buf));
            if (avail > 0) {
                ssize_t wr = write(fd_uart, io_buf, avail);
                if (wr > 0) {
                    ring_consume(&to_uart, (size_t)wr);
                } else if (wr < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    ALOGE("UART write failed: %s", strerror(errno));
                }
            }
        }
    }

    ALOGI("shutting down");
    ret = EXIT_SUCCESS;

out:
    property_set(PROP_FILTER_STATUS, "0");
    if (fd_ctrl_cli >= 0)
        close(fd_ctrl_cli);
    if (fd_bt_cli >= 0)
        close(fd_bt_cli);
    if (fd_ctrl_srv >= 0)
        close(fd_ctrl_srv);
    if (fd_bt_srv >= 0)
        close(fd_bt_srv);
    if (fd_uart >= 0)
        close(fd_uart);

    return ret;
}
