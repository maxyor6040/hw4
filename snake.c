#include <linux/ctype.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/system.h>
#include <asm/uaccess.h>

#include "include/asm-i386/semaphore.h"
//TODO: find out how do we reach the semaphore.h file

#include "snake.h"
#include "hw3q1.c"

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Dean's beauties" );

int iNumDevices = 0;
int max_games = 0;
MODULE_PARM( max_games, "i" );
int num_of_games = 0; //current number of games. should be 0 <= current_games_num <= max_games
/* globals */
int my_major = 0; 			/* will hold the major # of my device driver */


struct file_operations my_fops = {
        .open=		my_open,
        .release=	my_release,
        .read=		my_read,
        .write=		my_write,
        .llseek=		NULL,
        .ioctl=		my_ioctl,
        .owner=		THIS_MODULE,
};

typedef struct semaphore Mutex;
typedef struct semaphore Semaphore;

typedef struct game_t {
    Matrix Game;
    Semaphore check_open_only_twice;
    Semaphore catch_turn;
    Semaphore someone_left;
    int did_game_start; //TODO: use this instead of NULL in game matrix or not?
} game;

typedef struct privateGameData_t {
    game* myGame;
    int color;
} privateGameData;


game Games[256];

int init_module( void ) {
    my_major = register_chrdev( my_major, "SNAKE GAME", &my_fops );
    if( my_major < 0 ) {
        printk( KERN_WARNING "can't get dynamic major\n" );
        return my_major;
    }

    if ( max_games <= 0 ) {
        printk( KERN_WARNING "wrong max_games/max_games was not initiallized\n" );
        return -1;//TODO: make sure we need this
    }

    for (int i=0; i< 256; ++i) {
        sema_init(&Games[i].check_open_only_twice, 1);
        sema_init(&Games[i].catch_turn, 0);
        sema_init(&Games[i].someone_left, 1);
        Games[i].did_game_start = FALSE;
    }

    return 0;
}


void cleanup_module( void ) {

    unregister_chrdev( my_major, "SNAKE GAME" );
    //TODO: remove all the private data. We will probably store there the minor, to know which game the user is playing in. + if he's black or white
    return;
}


int my_open( struct inode *inode, struct file *filp ) {

    int i = MINOR( inode->i_rdev );
    privateGameData* PGD;
    if ( Games[i].did_game_start == TRUE ) { //TODO: MAKE SURE WE ARE NOT IN A RACE CONDITION!!!
        if(down_trylock(Games[i].check_open_only_twice)==0){//black player
            PGD = kmalloc(sizeof(*privateGameData));
            PGD->myGame = Games+i;
            PGD->color = BLACK;

            filp->private_data = (void*)PGD;
            up(PGD->myGame->catch_turn);
            down(PGD->myGame->catch_turn);
        }
        else{//should not enter. already two players
            //TODO: return error
        }
    }
    else{
        //this is white player
        PGD = kmalloc(sizeof(*privateGameData));
        PGD->myGame = Games+i;
        PGD->color = WHITE;

        filp->private_data = (void*)PGD;
        down(PGD->myGame->catch_turn);
        /*region no need
        if( filp->f_mode & FMODE_READ )
            //handle read opening
        if( filp->f_mode & FMODE_WRITE )
            //handle write opening

        if( filp->f_flags & O_NONBLOCK ) {
            //example of additional flag
        }
        */
    }
    return 0;
}


int my_release( struct inode *inode, struct file *filp ) {
    if( filp->f_mode & FMODE_READ )
        //handle read close\ing
    if( filp->f_mode & FMODE_WRITE )
        //handle write closing

        MOD_DEC_USE_COUNT; /*no need in 2.4 or later*/

    return 0;
}


ssize_t my_read( struct file *filp, char *buf, size_t count, loff_t *f_pos ) {
    //read the data
    //copy the data to user
    //return the ammount of read data
}


ssize_t my_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
    //copy the data from user
    //write the data
    // return the ammount of written data
}


ssize_t my_read2( struct file *filp, char *buf, size_t count, loff_t *f_pos ) {
    //read the data
    //copy the data to user
    //return the ammount of read data
}


ssize_t my_write2(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
    //copy the data from user
    //write the data
    //return the ammount of written data
}




int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) {

    switch( cmd ) {
        case MY_OP1:
            //handle op1;
            break;

        case MY_OP2:
            //handle op2;
            break;

        default: return -ENOTTY;
    }
    return 0;
}
