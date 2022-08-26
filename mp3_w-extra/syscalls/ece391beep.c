#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int32_t frequency[] = {587, 294, 440};
int32_t duration[] = {54, 54, 214};

/* 
 *  beep
 *  DESCRIPTION: mute the PC speaker
 *  INPUTS:     frequency - voice freq array
 *              duration - voice duration array
 *  OUTPUTS:    none
 *  RETURN VALUE: none
 *  SIDE EFFECT: mute speaker and change PIT status
 *  REFERENCE: https://wiki.osdev.org/PC_Speaker
 */
void beep(uint32_t frequency, uint32_t duration) {
    int i;
    int rtc_fd;
    int ret_val;
    int garbage;

    // set the rtc
    rtc_fd = ece391_open((uint8_t*)"rtc");
    ret_val = 32;
    ret_val = ece391_write(rtc_fd, &ret_val, 4);
    // check if successfully write frequency
    if (ret_val == -1) {
        ece391_fdputs(1, (uint8_t*)"Error: fail to set RTC in beep.\n");
        return;
    }

    // play sound
    ece391_fdputs (1, (uint8_t*)"A sound frame is playing\n");
    ece391_sound(frequency);
    for (i = 0; i < duration; i++){
        // Wait for RTC tick
		ece391_read(rtc_fd, &garbage, 4);
    }
    ece391_nosound();

    // close rtc
    ece391_close(rtc_fd);
    return;
}


/* 
 *  main
 *  DESCRIPTION: use the beep function
 *  INPUTS:     none
 *  OUTPUTS:    none
 *  RETURN VALUE: 0
 *  SIDE EFFECT: play sounds
 *  REFERENCE: https://wiki.osdev.org/PC_Speaker
 */
int32_t main()
{

    int i;
    ece391_fdputs (1, (uint8_t*)"Start playing sound...\n");
    for (i=0; i<3; i++) {
        beep(frequency[i], duration[i] / 32);
    }
    ece391_fdputs (1, (uint8_t*)"Sound playing end.\n");

    return 0;
}