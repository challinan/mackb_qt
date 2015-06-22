#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <string.h>
#include <ctype.h>

#define USB_SERIAL "/dev/cu.usbserial-AI02BTGZ"
#define DEBUG 1

struct termios orig_termios;
int speed_knob_hex;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;
    
    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy((void *)&new_termios, (void *)&orig_termios, (size_t) sizeof(new_termios));
    
    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int serial_port_char_avail(int fd) {
    struct timeval tv = { 0L, 25000L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    return select(fd+1, &fds, NULL, NULL, &tv);
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = (int) read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

int send_char(int fd, int c) {
    int rc;
    rc = (int) write(fd, &c, 1);
    if ( rc == -1 ) {
        perror("send_char: Error writing to usb serial\n");
        return -1;
    }
    if ( rc != 1 ) {
        printf("send_char: Write error: invalid number of characters written %d\n", rc);
        return -1;
    }
#ifdef DEBUG
    printf("send_char: wrote %0x\r\n", c);
#endif
    return rc;
}

int send_open(int fd) {
    int i, rc;
    char cmd[] = {0x00, 0x02};
    
    for ( i=0; i<sizeof(cmd); i++) {
        rc = send_char(fd, cmd[i]);
        if ( rc == -1 ) return rc;
    }
    
    printf("send_open: Writing 2 bytes to FD[%d]\n\r", fd);
    return 1;
}

int send_close(int fd) {
    int i, rc;
    char cmd[] = {0x00, 0x03};
    
    for ( i=0; i<sizeof(cmd); i++) {
        rc = send_char(fd, cmd[i]);
        if ( rc == -1 ) return rc;
    }
    
#ifdef DEBUG
    printf("send_close: Writing 2 bytes to FD[%d]\n\r", fd);
#endif
    return 1;
}


int send_echo(int fd) {
    int i, rc;
    char cmd[] = {0x00, 0x04, 0x41};
    
    for ( i=0; i<sizeof(cmd); i++) {
        rc = send_char(fd, cmd[i]);
        if ( rc == -1 ) return rc;
    }
    
#ifdef DEBUG
    printf("send_echo: Writing %ld bytes to FD[%d]\n\r", sizeof(cmd), fd);
#endif
    return 1;
}

int send_vccread(int fd) {
    int i, rc;
    char cmd[] = {0x00, 21};
    
    for ( i=0; i<sizeof(cmd); i++) {
        rc = send_char(fd, cmd[i]);
        if ( rc == -1 ) return rc;
    }
    
#ifdef DEBUG
    printf("send_vccread: Writing %ld bytes to FD[%d]\n\r", sizeof(cmd), fd);
#endif
    return 1;
}

int process_status_byte(int c) {
    printf("Status byte %02x not implemented yet\n", c);
    return 0;
}

int main(int argc, char **argv) {
    
    char buf[512];
    int fd, rc, c;
    int start_timeout = 0, timeout = 0;
    speed_t baud = B1200; /* baud rate */
    struct termios settings;
    
    printf("Opening %s\n\r", USB_SERIAL);
    fd = open(USB_SERIAL, O_RDWR);
    if ( fd == -1 ) {
        perror("Error opening serial port\n");
        exit(1);
    }
    printf("USB Serial port opened successfully on %d\n\r", fd);
    
    /* set the other settings (in this case, baud, 1200/8N2) */
    tcgetattr(fd, &settings);
    settings.c_cflag = 0;
    settings.c_cflag = CSTOPB | CS8 | CREAD | CLOCAL;
    cfmakeraw(&settings);
    cfsetspeed(&settings, baud);
    
    /* Now enable the settings */
    rc = tcsetattr(fd, TCSANOW, &settings);
    if ( rc == -1 ) {
        close(fd);
        printf("main: tcsetattr returned %d\n", rc);
        perror("main: tcsetattr failed\n");
    }
    
    printf("Current input baud rate is %d\n", (int) cfgetispeed(&settings));
    printf("Current output baud rate is %d\n", (int) cfgetospeed(&settings));
    
    rc = send_open(fd);
    if ( rc == -1 ) exit(rc);
    
    set_conio_terminal_mode();
    
    while ( 1 ) {
        if ( serial_port_char_avail(fd) ) {
            rc = (int) read(fd, buf, 1);
            if ( rc == -1 ) {
                perror("Error reading usb-serial\n");
                exit(1);
            }
            c = buf[0];
            if ( (c & 0xc0) == 0x80) {
                /* This is a speed pot report */
                speed_knob_hex = c;
                continue;
            }
            if ( (c & 0xc0) == 0xc0 ) {
                /* 
                 * This is a status byte. (Host may or may not have asked for it.)
                 * process status change, note that it could be a pushbutton change.
                 */
                process_status_byte(c);
                continue;
            }
            
            printf("Received %x\r\n", buf[0]);
        }
        if ( kbhit() ) {
            c = getch();
            switch(c) {
                case 'Q':
                    start_timeout = 1;
                    send_close(fd);
                    printf("Initiating shutdown sequence\r\n");
                    break;
                case 'E':
                    send_echo(fd);
                    break;
                case 'V':
                    send_vccread(fd);
                default:
                    send_char(fd, toupper(c));
            }
        }
        if ( start_timeout )
            timeout++;
        if ( timeout > 1000000 ) break;
    }
    
    printf("USB Serial port closed\n\r");
    close(fd);
    
    return 0;
}