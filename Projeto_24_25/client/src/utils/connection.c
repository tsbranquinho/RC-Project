#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void resolve_hostname(const char *hostname) {
    struct addrinfo hints;
    char ipstr[INET_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, NULL, &hints, &res) != 0) {
        fprintf(stderr, "Error: Unable to resolve hostname '%s'\n", hostname);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));

    GSIP = strdup(ipstr);
    freeaddrinfo(res);
}

void get_arguments(int argc, char *argv[]) {
    int opt, ip_set = 0, port_set = 0;
    while ((opt = getopt(argc, argv, "n:p:")) != -1) {
        switch (opt) {
            case 'n':
                if (ip_set) {
                    usage(argv[0]);
                }
                resolve_hostname(optarg);
                ip_set = 1;
                break;

            case 'p':
                if (port_set) {
                    usage(argv[0]);
                }
                if (!is_number(optarg)) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    exit(EXIT_FAILURE);
                }
                GSport = atoi(optarg);
                if (GSport < 0 || GSport > MAX_PORT) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    exit(EXIT_FAILURE);
                }
                port_set = 1;
                break;

            default:
                usage(argv[0]);
        }
    }

    if (!ip_set || !port_set) {
        usage(argv[0]);
    }
}

int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}