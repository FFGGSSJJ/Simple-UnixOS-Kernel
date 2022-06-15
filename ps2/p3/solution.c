//#include <asm/spinlock.h>
// You may add up to 5 elements to this struct.
// The type of synchronization primitive you may use is SpinLock.
typedef struct zs_enter_exit_lock{
    spinlock_t *lock;    //Initialized to UNLOCK
    volatile unsigned int zombie_lab_num; // 0 to 10
    volatile unsigned int scientist_lab_num; // 0 to 4
    volatile unsigned int zombie_wait_num; // 0 to infinite
    volatile unsigned int scientist_wait_num; // 0 to infinite
}zs_lock;


/* 
 * zombie_enter
 *   DESCRIPTION: Check conditions and let a zombie enter the lab. 
 *   INPUTS: zs -- pointer to the zombie-scientist lock structure 
 *   OUTPUTS: none
 *   RETURN VALUE: success enter 0; fail enter -1
 *   SIDE EFFECTS: none.
 */
int zombie_enter(zs_lock* zs) {
    spin_lock_irq(zs->lock);
    zs->zombie_wait_num ++;
    spin_unlock_irq(zs->lock);
    //end until the lab has been successfully entered by the call
    while(1) {
        spin_lock_irq(zs->lock);
        // Only when no scientist waiting in line and in lab
        // and there is zombie waiting in line and less that
        // 10 zombies are in the lab can a zombie enters.
        // else, wait until the conditions are fitted
        if (zs->scientist_lab_num == 0 && zs->scientist_wait_num == 0 && 
            zs->zombie_wait_num > 0 && zs->zombie_lab_num < 10) {
            zs->zombie_lab_num ++;
            zs->zombie_wait_num --;
            spin_unlock_irq(zs->lock);
            return 0;
        } else {
            spin_unlock_irq(zs->lock);
        }
    }
    return -1;
}


/* 
 * zombie_exit
 *   DESCRIPTION: let a zombie leave the lab. Sanitize the lab if necessary
 *   INPUTS: zs -- pointer to the zombie-scientist lock structure 
 *   OUTPUTS: none
 *   RETURN VALUE: success enter 0; fail enter -1
 *   SIDE EFFECTS: none.
 */
int zombie_exit(zs_lock* zs) {
    spin_lock_irq(zs->lock);
    // The only condition is that there is a zombie in the lab
    if (zs->zombie_lab_num > 0) {
        zs->zombie_lab_num --;
        // If there is no zombie in lab and there is a scientist
        // waiting, then we sanitize the lab.
        // At this time the lab is empty as for the conditions
        if (zs->zombie_lab_num == 0 && zs->scientist_wait_num > 0) {
            sanitize_lab(zs);
            spin_unlock_irq(zs->lock);
            return 0;
        }
        spin_unlock_irq(zs->lock);
        return 0;
    } else {
        spin_unlock_irq(zs->lock);
        return -1;
    }
}


/* 
 * scientist_enter
 *   DESCRIPTION: Check conditions and let a scientist enter the lab. 
 *   INPUTS: zs -- pointer to the zombie-scientist lock structure 
 *   OUTPUTS: none
 *   RETURN VALUE: success enter 0; fail enter -1
 *   SIDE EFFECTS: none.
 */
int scientist_enter(zs_lock* zs) {
    spin_lock_irq(zs->lock);
    zs->scientist_wait_num ++;
    spin_unlock_irq(zs->lock);
    //end until the lab has been successfully entered by the call
    while(1) {
        spin_lock_irq(zs->lock);
        // Only when no zombies are in lab and there
        // are scientists waiting in line and less that
        // 4 scientists are in the lab can a scientist enters.
        // else, wait until the conditions are fitted
        if (zs->zombie_lab_num == 0 && zs->scientist_wait_num > 0 && zs->scientist_lab_num < 4) {
            zs->scientist_lab_num ++;
            zs->scientist_wait_num --;
            spin_unlock_irq(zs->lock);
            return 0;
        } else {
            spin_unlock_irq(zs->lock);
        }
    }
    return -1;
}


/* 
 * scientist_exit
 *   DESCRIPTION: let a scientist leave the lab.
 *   INPUTS: zs -- pointer to the zombie-scientist lock structure 
 *   OUTPUTS: none
 *   RETURN VALUE: success enter 0; fail enter -1
 *   SIDE EFFECTS: none.
 */
int scientist_exit(zs_lock* zs) {
    spin_lock_irq(zs->lock);
    // The only condition is that there is a scientist in the lab
    if (zs->scientist_lab_num > 0) {
        zs->scientist_lab_num --;
        spin_unlock_irq(zs->lock);
        return 0;
    } else {
        spin_unlock_irq(zs->lock);
        return -1;
    }
}


/* 
 * sanitize_lab
 *   DESCRIPTION: Sanitizes the lab, removing all zombie contaminants. 
 *   INPUTS: zs -- pointer to the zombie-scientist lock structure 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Must only be called when the lab is empty.
 */
void sanitize_lab(zs_lock* zs);

