#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void get_arguments(int argc, char *argv[]) {

    int opt;
    settings.GSport = DEFAULT_PORT;
    settings.verbose_mode = 0;

    while ((opt = getopt(argc, argv, "p:v")) != -1) {
        switch (opt) {
            case 'p':
                settings.GSport = atoi(optarg);
                if (settings.GSport < 0 || settings.GSport > MAX_PORT) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'v':
                settings.verbose_mode = 1;
                printf("Verbose mode enabled\n");
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

void setup_server() {
    struct stat st;

    if (stat("GAMES", &st) == -1) {
        if (mkdir("GAMES", 0777) == -1) {
            perror("Error creating GAMES directory");
            exit(EXIT_FAILURE);
        }
    }
    if (stat("SCORES", &st) == -1) {
        if (mkdir("SCORES", 0777) == -1) {
            perror("Error creating SCORES directory");
            exit(EXIT_FAILURE);
        }
    }

    task_queue_init(&task_queue);

    create_udp_socket();
    create_tcp_socket();

    if (settings.verbose_mode) {
        printf("Starting Game Server on port: %d\n", settings.GSport);
        printf("Game Server is running.\n");
    }

    thread_configuration();

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

    settings.timeout = (struct timeval){.tv_sec = 10, .tv_usec = 0};
    FD_ZERO(&settings.read_fds);
    FD_SET(settings.udp_socket, &settings.read_fds);
    FD_SET(settings.tcp_socket, &settings.read_fds);

}

void create_tcp_socket() {
    int test=1;
    settings.tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(settings.GSport)
    };
    
    if (settings.tcp_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(settings.tcp_socket, SOL_SOCKET, SO_REUSEADDR, &test, sizeof(test)) < 0) {
        perror("Error setting socket timeout");

        pthread_mutex_lock(&fd_mutex);
        close(settings.tcp_socket);
        pthread_mutex_unlock(&fd_mutex);
        exit(EXIT_FAILURE);
    }

    if (bind(settings.tcp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(settings.tcp_socket, 5) < 0) {
        perror("TCP listen failed");
        exit(EXIT_FAILURE);
    }
}

void create_udp_socket() {
    settings.udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(settings.GSport)
    };

    if (settings.udp_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (bind(settings.udp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }
}

void thread_configuration() {
    num_threads = sysconf(_SC_NPROCESSORS_ONLN) * 2;
    if (num_threads <= 0) num_threads = 1;

    threads = malloc(sizeof(pthread_t) * num_threads);
    if (threads == NULL) {
        perror("Thread pool allocation failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }
}