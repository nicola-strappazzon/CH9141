#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define ATT_CID                  0x04
#define ATT_OP_ERROR_RSP         0x01
#define ATT_OP_WRITE_REQ         0x12
#define ATT_OP_WRITE_CMD         0x52
#define ATT_OP_HANDLE_VALUE_NOTI 0x1B
#define ATT_DEFAULT_MTU          23
#define ATT_MAX_PAYLOAD          (ATT_DEFAULT_MTU - 3)
#define MAX_LINE 512

static int ble_connect(const char *mac, int *sock) {
    struct sockaddr_l2 srcaddr, dstaddr;
    struct bt_security btsec;
    bdaddr_t src_addr;
    bdaddr_t dst_addr;
    int id;
    uint8_t dst_type = BDADDR_LE_PUBLIC;

    if (str2ba(mac, &dst_addr) < 0) {
        return -1;
    }

    id = hci_get_route(NULL);
    if (id < 0) {
        return -1;
    }

    if (hci_devba(id, &src_addr) < 0) {
        return -1;
    }

    *sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (*sock < 0) {
        return -1;
    }

    memset(&srcaddr, 0, sizeof(srcaddr));
    srcaddr.l2_family      = AF_BLUETOOTH;
    srcaddr.l2_cid         = htobs(ATT_CID);
    srcaddr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
    bacpy(&srcaddr.l2_bdaddr, &src_addr);

    if (bind(*sock, (struct sockaddr *)&srcaddr, sizeof(srcaddr)) < 0) {
        close(*sock);
        return -1;
    }

    memset(&btsec, 0, sizeof(btsec));
    btsec.level = BT_SECURITY_LOW;
    if (setsockopt(*sock, SOL_BLUETOOTH, BT_SECURITY, &btsec, sizeof(btsec)) != 0) {
        close(*sock);
        return -1;
    }

    memset(&dstaddr, 0, sizeof(dstaddr));
    dstaddr.l2_family      = AF_BLUETOOTH;
    dstaddr.l2_cid         = htobs(ATT_CID);
    dstaddr.l2_bdaddr_type = dst_type;
    bacpy(&dstaddr.l2_bdaddr, &dst_addr);

    if (connect(*sock, (struct sockaddr *)&dstaddr, sizeof(dstaddr)) < 0) {
        close(*sock);
        return -1;
    }

    return 0;
}

static int ble_att_write_req(int sock, uint16_t handle, const uint8_t *data, size_t len) {
    if (len > MAX_LINE) {
        return -1;
    }

    uint8_t buf[3 + MAX_LINE];
    buf[0] = ATT_OP_WRITE_REQ;
    buf[1] = handle & 0xFF;
    buf[2] = (handle >> 8) & 0xFF;
    memcpy(&buf[3], data, len);

    ssize_t n = write(sock, buf, 3 + len);
    if (n < 0) {
        return -1;
    }

    return 0;
}

static int ble_att_write_cmd(int sock, uint16_t handle, const uint8_t *data, size_t len) {
    if (len > MAX_LINE) {
        return -1;
    }

    uint8_t buf[3 + MAX_LINE];
    buf[0] = ATT_OP_WRITE_CMD;
    buf[1] = handle & 0xFF;
    buf[2] = (handle >> 8) & 0xFF;
    memcpy(&buf[3], data, len);

    ssize_t n = write(sock, buf, 3 + len);
    if (n < 0) {
        return -1;
    }

    return 0;
}

static int ble_send_to_tx(int sock, uint16_t handle_tx, const uint8_t *data, size_t len) {
    size_t offset = 0;
    while (offset < len) {
        size_t chunk = len - offset;
        if (chunk > ATT_MAX_PAYLOAD)
            chunk = ATT_MAX_PAYLOAD;

        if (ble_att_write_cmd(sock, handle_tx, data + offset, chunk) < 0)
            return -1;

        offset += chunk;
    }
    return 0;
}

int ble_readline(int sock) {
    uint8_t buf[MAX_LINE];

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int ret = select(sock + 1, &readfds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            return -1;
        }

        if (FD_ISSET(sock, &readfds)) {
            ssize_t n = read(sock, buf, sizeof(buf));
            if (n <= 0) {
                return -1;
            }

            uint8_t op = buf[0];

            if (op == ATT_OP_HANDLE_VALUE_NOTI && n > 3) {
                size_t data_len = n - 3;
    
                for (size_t i = 0; i < data_len; i++) {
                    unsigned char c = buf[3 + i];

                    if ((char)c == '\n')
                        return 0;

                    if (c >= 32 && c <= 126) {
                        putchar(c);
                    }
                }
            }
        }
    }
}


void show_help(const char *progname) {
    printf("Usage: %s [OPTIONS]\n", progname);
    printf("\nOptions:\n");
    printf("  --mac <string>        Target Bluetooth MAC address (required)\n");
    printf("  --message <string>    Send an initial message to the device\n");
    printf("  --wait                Wait for an incoming message\n");
    printf("  -?, -h, --help        Display this help message and exit\n");
    printf("\nExample:\n");
    printf("  %s --mac D1:5E:28:7A:4E:E0 --message \"Hello from CLI on Linux\" --wait\n\n", progname);
}

int main(int argc, char **argv) {
    int opt;
    int sock;
    int err;
    int arg_wait = 0;
    char *arg_mac = NULL;
    char *arg_message = NULL;
    char *message_crlf = NULL;
    char line[MAX_LINE];
    uint16_t h_ccc = 0x002e;
    uint16_t h_write = 0x0031;
    uint8_t ccc_val[2] = {0x01, 0x00};

    struct option long_options[] = {
        {"mac",     required_argument, 0, 1},
        {"message", required_argument, 0, 2},
        {"wait",    no_argument,       0, 3},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };


    while ((opt = getopt_long(argc, argv, "h?", long_options, NULL)) != -1) {
        switch (opt) {
            case 1: // --mac
                arg_mac = optarg;
                break;
            case 2: // --message
                arg_message = optarg;
                break;
            case 3: // --wait
                arg_wait = 1;
                break;
            case '?':
                show_help(argv[0]);
                return 0;
            case 'h':
                show_help(argv[0]);
                return 0;
            default:
                show_help(argv[0]);
                return 1;
        }
    }

    if (!arg_mac) {
        show_help(argv[0]);
        return 1;
    }

    if (!arg_message && !arg_wait) {
        fprintf(stderr, "Error: either --message or --wait must be specified.\n");
        fprintf(stderr, "You must provide at least one of these options.\n\n");
        show_help(argv[0]);
        return 1;
    }

    err = ble_connect(arg_mac, &sock);
    if (err < 0) {
        return err;
    }

    err = ble_att_write_req(sock, h_ccc, ccc_val, sizeof(ccc_val));
    if (err < 0) {
        close(sock);
        return err;
    }

    if (arg_message) {
        char message_crlf[1024];
        snprintf(message_crlf, sizeof(message_crlf), "%s\r\n", arg_message);

        err = ble_send_to_tx(sock, h_write, (const uint8_t *)message_crlf, strlen(message_crlf));
        if (err < 0) {
            close(sock);
            return err;
        }
    }

    if (arg_wait) {
        err = ble_readline(sock);
        if (err < 0) {
            close(sock);
            return err;
        }

        printf("\r\n");
    }

    close(sock);
    return 0;
}
