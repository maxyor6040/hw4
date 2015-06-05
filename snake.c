//#include <linux/ctype.h>
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
#include <linux/sched.h>

#include <asm-i386/semaphore.h>
//TODO: find out how do we reach the semaphore.h file

#include "snake.h"
#include "hw3q1.c"

MODULE_LICENSE( "GPL" );
//MODULE_AUTHOR( "Dean's beauties" );

int max_games = 0;
MODULE_PARM( max_games, "i" );
int num_of_games = 0; //current number of games. should be 0 <= current_games_num <= max_games
/* globals */
int my_major = 0; 			/* will hold the major # of my device driver */


int init_module( void );
void cleanup_module( void );
int my_open( struct inode *inode, struct file *filp );
int my_release( struct inode *inode, struct file *filp );
ssize_t my_read( struct file *filp, char *buf, size_t count, loff_t *f_pos );
ssize_t my_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
ssize_t my_read2( struct file *filp, char *buf, size_t count, loff_t *f_pos );
ssize_t my_write2(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

struct file_operations my_fops = {
        .open=		my_open,
        .release=	my_release,
        .read=		my_read,
        .write=		my_write,
        .llseek=		NULL,
        .ioctl=		my_ioctl,
        .owner=		THIS_MODULE,
};

typedef struct semaphore Semaphore;

typedef struct game_t {
    Matrix board;
    Semaphore globalLock;
    Semaphore whiteMoves;
    Semaphore blackMoves;
    int someoneLeft;
    int countOpens;
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
    int i;
    for (i=0; i< 256; ++i) {
        sema_init(&Games[i].whiteMoves, 0);
        sema_init(&Games[i].blackMoves, 0);
        sema_init(&Games[i].globalLock, 1);
        Games[i].someoneLeft = FALSE;
        Games[i].countOpens = 0;
    }
    return 0;
}


void cleanup_module( void ) {
    unregister_chrdev( my_major, "SNAKE GAME" );
    return;
}


int my_open( struct inode *inode, struct file *filp ) {
    int i = MINOR( inode->i_rdev );
    Games[i].countOpens++;
    privateGameData* PGD;
    if(Games[i].countOpens == 1){//WHITE player
        privateGameData* PGD=kmalloc(sizeof(*PGD), GPF_KERNEL);
        PGD->color=WHITE;
        PGD->myGame = Games+i;
        filp->private_data = (void*)PGD;
        Init(PGD->myGame->board, i);
        down(&PGD->myGame->whiteMoves);
        up(&PGD->myGame->whiteMoves);
    }else if(Games[i].countOpens == 2){//BLACK player
        privateGameData* PGD=kmalloc(sizeof(*PGD), GPF_KERNEL);
        PGD->color=BLACK;
        PGD->myGame = Games+i;
        filp->private_data = (void*)PGD;
        up(&PGD->myGame->whiteMoves);
    }else{//third or above entrence to my_open
        //TODO: send error
    }
    return 0;
}


int my_release( struct inode *inode, struct file *filp ) {
    privateGameData* PGD = (privateGameData*)filp->private_data;
    if(PGD==NULL){
        //TODO: return error - already released this player
        return 0;
    }
    down(&PGD->myGame->globalLock);
    if(PGD==NULL){
        //TODO: return error - already released this player
        return 0;
    }
    PGD->myGame->someoneLeft = TRUE;
    up(&PGD->myGame->whiteMoves);
    up(&PGD->myGame->blackMoves);

    filp->private_data = NULL;
    
    up(&PGD->myGame->globalLock);
    kfree(filp->private_data);
    return 0;
//TODO: remove all the private data. We will probably store there the minor, to know which game the user is playing in. + if he's black or white
    return 0;
}


ssize_t my_read( struct file *filp, char *buf, size_t count, loff_t *f_pos ) {
    //read the data
    //copy the data to user
    //return the ammount of read data
    return 0;//TODO: remove this and think
}


ssize_t my_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
    //copy the data from user
    //write the data
    // return the ammount of written data
    return 0;//TODO: remove this and think
}


int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) {

    switch( cmd ) {
        case SNAKE_GET_WINNER:
            //handle SNAKE_GET_WINNER;
            break;

        case SNAKE_GET_COLOR:
            //handle SNAKE_GET_COLOR;
            break;

        default: return -ENOTTY;
    }
    return 0;
}

