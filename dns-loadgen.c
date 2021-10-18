#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libnet.h>
#include <unistd.h>

#define MAXPAYLOADLEN 1500

void die(char *msg, char *errbuf) {
    printf("Err: %s: %s\n", msg, errbuf);
    exit(-1);
}

// Global Variables for build_msg()
FILE *bmfp;
int bmisclosed = 1;
char *bmln;
size_t bmlen;
ssize_t bmrdlen;
unsigned short bmtid = 0;
char *f5com = "f5.com.\n";

int build_msg(char *p, int maxlen) {
    int len=0;
    char *token;
    char tokenlen;
    const char delim[2] = ".";
    int i = 12; // Skip 12 bytes to jump directly to the doman query.

    memset(p, 0, maxlen);

    if (bmisclosed) {
        bmfp = fopen("domain.txt", "r");
        if (bmfp == NULL) die("fopen", "");
        bmisclosed = 0;
    }

    // Transaction ID
    if (++bmtid == 0) bmtid = 1;
    p[0] = (bmtid >> 8) & 0xff;
    p[1] = bmtid & 0xff;

    // Flags
    p[2] = 1; p[3] = 0x20;

    // Questions = 1
    p[5] = 1;

    // Answer RRs: 0; Authority RRs : 0; Additional RRs: 1;

    bmrdlen = getline(&bmln, &bmlen, bmfp);
    if (bmrdlen == -1) {
        fclose(bmfp);
        bmfp = fopen("domain.txt", "r");
        if (bmfp == NULL) die("fopen", "");
        bmrdlen = getline(&bmln, &bmlen, bmfp);
        if (bmrdlen == -1) die("getline 2nd time", "");
    }
    if (bmrdlen == 2) {
        bmln = f5com;
        bmrdlen = strlen(f5com);
    }

    //printf("bmrdlen: %li - bmln: %s\n", bmrdlen, bmln);
    token = strtok(bmln, delim);
    while (token != NULL) {
        tokenlen = (char) strlen(token);
        //printf("%i - %s ; ", tokenlen, token);
        if (tokenlen > 1) {
            p[i] = tokenlen;
            i++;
            memcpy(&p[i], token, tokenlen);
            i += tokenlen;
            if (i >= maxlen) {
                memset(p, 0, maxlen);
                return(-1);
            }
        }
        token = strtok(NULL, delim);
    }
    i += 2;
    p[i] = 1;
    i += 2;
    p[i] = 1;
    len = i+1;

    return(len);
}

// Global Variables for send_packet()
unsigned char enet_cli[6] = {0x00, 0x0c, 0x29, 0x15, 0x4e, 0x43};
unsigned char enet_srv[6] = {0x00, 0x0a, 0x49, 0xa8, 0x78, 0x09};
// 165.21.83.88
unsigned int dst_ip = 0xc80aa8c0;
unsigned int src_ip = 0x0100800a;
unsigned short ip_seqnum = 0;
unsigned short sport = 32000;
unsigned short dport = 53;

void send_packet(libnet_t *l, char *payload, unsigned short payloadlen) {
    libnet_ptag_t t;
    int c;
    unsigned int ipc, ipd;

    if (++sport == 0) sport = 32001;

    t = libnet_build_udp(
        sport,
        dport,
        LIBNET_UDP_H + payloadlen,
        0,
        (uint8_t*)payload,
        payloadlen,
        l,
        0
    );
    if (t==-1) die("libnet_build_udp", "");

    if (++ip_seqnum == 0) ip_seqnum=1;
    ipd = ((src_ip >> 24) + 1) & 0xff;
    if (ipd != 0) {
        src_ip = (src_ip & 0xffffff) + ((ipd << 24) & 0xff000000);
    } else {
        ipd = 1;
        ipc = ((src_ip >> 16) + 1) & 0xff;
        if (ipc == 0) {
            ipc = 1;
        }
        src_ip = (src_ip & 0xffff) + ((ipc << 16) & 0xff0000) + ((ipd << 24) & 0xff000000);
    }

    t = libnet_build_ipv4(
        LIBNET_IPV4_H + LIBNET_UDP_H + payloadlen,
        0,
        ip_seqnum,
        0,
        64,
        IPPROTO_UDP,
        0,
        src_ip,
        dst_ip,
        NULL,
        0,
        l,
        0
    );
    if (t==-1) die("libnet_build_ipv4", "");

    t = libnet_build_ethernet(
        enet_srv,
        enet_cli,
        ETHERTYPE_IP,
        NULL,
        0,
        l,
        0
    );
    if (t==-1) die("libnet_build_ethernet", "");

    c = libnet_write(l);
    libnet_clear_packet(l);

}

int main(int argc, char **argv) {
    char errbuf[1024];
    const char *inf = "ens192";
    libnet_t *l;
    unsigned short payloadlen;
    char payload[MAXPAYLOADLEN];
    int i;
    int j=0;

    l = libnet_init(LIBNET_LINK, inf, errbuf);
    if (l==NULL) die("libnet_init", errbuf);

    //for (i=0; i<1; i++) {
    while(1) {
        payloadlen = build_msg(payload, MAXPAYLOADLEN);
        if (payloadlen != -1) {
            send_packet(l, payload, payloadlen);
            j++;
            if (j>=10) {
                j = 0;
                usleep(100);
            }
        }
    }

    return(0);
}

