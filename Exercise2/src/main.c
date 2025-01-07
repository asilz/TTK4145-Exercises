#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <inttypes.h>
#include <netinet/ip.h>

struct SocketThread
{
    struct sockaddr address;
    pthread_t pid;
    int socket;
};

struct udp_header
{
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length; // including the header itself
    uint16_t checksum;
};

void *udp_receive(void *args)
{
    struct SocketThread *receiver = args;
    receiver->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (receiver->socket == -1)
    {
        return (void *)(intptr_t)errno;
    }

    int value = 1;
    if (setsockopt(receiver->socket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) == -1)
    {
        (void)close(receiver->socket);
        return (void *)(intptr_t)errno;
    }

    struct timeval time;
    time.tv_sec = 5;
    time.tv_usec = 0;
    if (setsockopt(receiver->socket, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time)) == -1)
    {
        (void)close(receiver->socket);
        return (void *)(intptr_t)errno;
    }

    if (bind(receiver->socket, &receiver->address, sizeof(receiver->address)) == -1)
    {
        (void)close(receiver->socket);
        return (void *)(intptr_t)errno;
    }

    while (1)
    {
        char buf[128];
        socklen_t socket_size = sizeof(struct sockaddr);
        ssize_t length = recvfrom(receiver->socket, buf, sizeof(buf), 0, &receiver->address, &socket_size);
        if (length == -1)
        {
            (void)close(receiver->socket);
            return (void *)(intptr_t)errno;
        }
        for (ssize_t i = 0; i < length; ++i)
        {
            int err = putchar(buf[i]);
            if (err < 0)
            {
                (void)close(receiver->socket);
                return (void *)(intptr_t)err;
            }
        }
        int err = putchar('\n');
        if (err < 0)
        {
            (void)close(receiver->socket);
            return (void *)(intptr_t)err;
        }
    }
    return (void *)(intptr_t)close(receiver->socket);
}

void *udp_transmit(void *args)
{
    struct SocketThread *transmitter = args;
    transmitter->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (transmitter->socket == -1)
    {
        return (void *)(intptr_t)errno;
    }

    int value = 1;
    if (setsockopt(transmitter->socket, SOL_SOCKET, SO_BROADCAST, &value, sizeof(value)) == -1)
    {
        (void)close(transmitter->socket);
        return (void *)(intptr_t)errno;
    }
    if (setsockopt(transmitter->socket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) == -1)
    {
        (void)close(transmitter->socket);
        return (void *)(intptr_t)errno;
    }

    if (bind(transmitter->socket, &transmitter->address, sizeof(transmitter->address)) == -1)
    {
        (void)close(transmitter->socket);
        return (void *)(intptr_t)errno;
    }

    while (1)
    {
        const char *buf = "UDP message";
        socklen_t socket_size = sizeof(struct sockaddr);
        ssize_t length = sendto(transmitter->socket, buf, strlen(buf), 0, &transmitter->address, socket_size);
        if (length == -1)
        {
            (void)close(transmitter->socket);
            return (void *)(intptr_t)errno;
        }
        sleep(1);
    }
    return (void *)(intptr_t)close(transmitter->socket);
}

int tcp_test(struct SocketThread *transmitter)
{
    transmitter->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (transmitter->socket == -1)
    {
        return errno;
    }

    int value = 1;
    if (setsockopt(transmitter->socket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) == -1)
    {
        (void)close(transmitter->socket);
        return errno;
    }

    if (connect(transmitter->socket, &transmitter->address, sizeof(transmitter->address)) == -1)
    {
        (void)close(transmitter->socket);
        return errno;
    }

    while (1)
    {
        char buf[1024] = "TCP message";
        socklen_t socket_size = sizeof(struct sockaddr);
        ssize_t length = sendto(transmitter->socket, buf, sizeof(buf), 0, &transmitter->address, socket_size);
        if (length == -1)
        {
            (void)close(transmitter->socket);
            return errno;
        }
        length = recvfrom(transmitter->socket, buf, sizeof(buf), 0, &transmitter->address, &socket_size);
        if (length == -1)
        {
            (void)close(transmitter->socket);
            return errno;
        }
        for (ssize_t i = 0; i < length; ++i)
        {
            int err = putchar(buf[i]);
            if (err < 0)
            {
                (void)close(transmitter->socket);
                return err;
            }
        }
        int err = putchar('\n');
        if (err < 0)
        {
            (void)close(transmitter->socket);
            return err;
        }
        sleep(1);
    }

    return close(transmitter->socket);
}

unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

void *raw_transmit(void *args)
{
    char buf[4096] = {0};
    struct iphdr *ip = (struct iphdr *)buf;
    struct udp_header *header = (struct udp_header *)(ip + 1);

    struct SocketThread *transmitter = args;
    transmitter->socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (transmitter->socket == -1)
    {
        return (void *)(intptr_t)errno;
    }

    int value = 1;

    if (setsockopt(transmitter->socket, IPPROTO_IP, IP_HDRINCL, &value, sizeof(value)) == -1)
    {
        return (void *)(intptr_t)errno;
    }

    ip->ihl = sizeof(ip);
    ip->version = 4;
    ip->tos = 16;
    ip->tot_len = sizeof(ip) + sizeof(header);
    ip->id = htons(54321);
    ip->ttl = 64;
    ip->protocol = 17;
    ip->saddr = htonl(INADDR_ANY);
    ip->daddr = htonl(INADDR_BROADCAST);

    header->src_port = ((struct sockaddr_in *)(&transmitter->address))->sin_port;
    header->dst_port = ((struct sockaddr_in *)(&transmitter->address))->sin_port;
    header->length = htons(sizeof(header));

    ip->check = csum((unsigned short *)buf, sizeof(ip) + sizeof(header));

    if (sendto(transmitter->socket, buf, ip->tot_len, 0, &transmitter->address, sizeof(transmitter->address)) == -1)
    {
        return (void *)(intptr_t)errno;
    }

    return NULL;
}

void *raw_receive(void *args)
{
    return NULL;
}

#define UDP_TRANSMIT_PORT 20000
#define UDP_RECEIVE_PORT 20001
#define TCP_FIXED_PORT 34933
#define TCP_NULL_PORT 33546
#define SERVER_ADDRESS 0x0A16EB2A

int main(void)
{
    struct SocketThread receiver;
    struct SocketThread transmitter;
    struct SocketThread tcp;

    struct sockaddr_in in = {
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_family = AF_INET,
        .sin_port = htons(UDP_RECEIVE_PORT),
    };

    struct sockaddr *address = (struct sockaddr *)(&in);

    receiver.address = *address;

    in.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    in.sin_port = htons(UDP_TRANSMIT_PORT);
    transmitter.address = *address;

    pthread_attr_t attr;
    int err = pthread_attr_init(&attr);
    if (err != 0)
    {
        return err;
    }
    err = pthread_create(&receiver.pid, &attr, udp_receive, &receiver);
    if (err != 0)
    {
        return err;
    }
    err = pthread_create(&transmitter.pid, &attr, udp_transmit, &transmitter);
    if (err != 0)
    {
        return err;
    }

    in.sin_addr.s_addr = htonl(SERVER_ADDRESS);
    in.sin_port = htons(TCP_FIXED_PORT);
    tcp.address = *address;
    tcp_test(&tcp);

    err = pthread_join(transmitter.pid, NULL);
    if (err != 0)
    {
        return err;
    }
    err = pthread_join(receiver.pid, NULL);
    if (err != 0)
    {
        return err;
    }

    return 0;
}