#include "serial.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#define BAUD B19200


int serial_open(const char *device){
    int fd;
    
    fd = open(device, O_RDWR | O_NOCTTY);
    if(fd == -1) {
        printf( "failed to open port %s\n", device );
        return -1;
    }

    struct termios  config;
    if(!isatty(fd)) { printf("error: not a tty!"); return -1;}
    if(tcgetattr(fd, &config) < 0) { printf("error: couldn't get attr"); return -1;}
    //
    // Input flags - Turn off input processing
    // convert break to null byte, no CR to NL translation,
    // no NL to CR translation, don't mark parity errors or breaks
    // no input parity check, don't strip high bit off,
    // no XON/XOFF software flow control
    //
    //config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
    //                    INLCR | PARMRK | INPCK | ISTRIP | IXON);
    config.c_iflag = 0;
    //
    // Output flags - Turn off output processing
    // no CR to NL translation, no NL to CR-NL translation,
    // no NL to CR translation, no column 0 CR suppression,
    // no Ctrl-D suppression, no fill characters, no case mapping,
    // no local output processing
    //
    // config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
    //                     ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
    config.c_oflag = 0;
    //
    // No line processing:
    // echo off, echo newline off, canonical mode off, 
    // extended input processing off, signal chars off
    //
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    //
    // Turn off character processing
    // clear current char size mask, no parity checking,
    // no output processing, force 8 bit input
    //
    config.c_cflag &= ~(CSIZE | PARENB);
    config.c_cflag |= CS8;
    //
    // One input byte is enough to return from read()
    // Inter-character timer off
    //
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;
    //
    // Communication speed (simple version, using the predefined
    // constants)
    //
    if(cfsetispeed(&config, BAUD) < 0 || cfsetospeed(&config, BAUD) < 0) {
         printf("error: couldn't set baud!\n");
         return -1;
    }
    //
    // Finally, apply the configuration
    //
    if(tcsetattr(fd, TCSAFLUSH, &config) < 0) { printf("error: couldn't set serial attrs\n"); return -1; }

    printf("Serial opened!\n");
    fflush(stdout);
    //write(fd,"A",1);
    return fd;
}

void serial_sync(int fd){
    const int length=32;
    const int sync_byte=0;
    uint8_t sync[length];
    for (int i=0; i<length; i++)
        sync[i] = sync_byte;
    write(fd, sync, length);
}

void serial_send_packet(int fd, packet_buffer* packet){
    uint8_t len = sizeof(packet_buffer);
    write(fd, &len, 1);
    write(fd, packet, len);
    usleep(40000);
}

void serial_close(int fd){
    close(fd);
}
