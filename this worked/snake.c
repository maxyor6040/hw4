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
loff_t my_llseek(struct file *, loff_t, int);
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
        .llseek=    my_llseek,
        .flush=		NULL,
        .ioctl=		my_ioctl,
        .owner=		THIS_MODULE,
};

/*
typedef struct game_t {
    Matrix board;
    Semaphore globalLock;
    Semaphore whiteMoves;
    Semaphore blackMoves;
    int someoneLeft;
    int countOpens;
    int Winner;
} game;

typedef struct privateGameData_t {
    game* myGame;
    int color;
} privateGameData;
*/

game Games[256];

int init_module( void ) {
    my_major = register_chrdev( my_major, "snake", &my_fops );
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
        Games[i].Winner = EMPTY;
    }
    return 0;
}

void cleanup_module( void ) {
    unregister_chrdev( my_major, "snake" );
    return;
}

int my_open( struct inode *inode, struct file *filp ) {
    int i = MINOR( inode->i_rdev );
    down(&(Games[i].globalLock));
    Games[i].countOpens++;
    if(Games[i].countOpens == 1){//WHITE player
        privateGameData* PGD1=kmalloc(sizeof(*PGD1), GFP_KERNEL);
        PGD1->color=WHITE;
        PGD1->myGame = Games+i;
        filp->private_data = (void*)PGD1;
        Init(&(PGD1->myGame->board));
        up(&(Games[i].globalLock));
        down(&PGD1->myGame->whiteMoves);
        up(&PGD1->myGame->whiteMoves);
    }else if(Games[i].countOpens == 2){//BLACK player
        up(&(Games[i].globalLock));
        privateGameData* PGD2=kmalloc(sizeof(*PGD2), GFP_KERNEL);
        PGD2->color=BLACK;
        PGD2->myGame = Games+i;
        filp->private_data = (void*)PGD2;
        up(&PGD2->myGame->whiteMoves);
    }else{//third or above entrence to my_open
        up(&(Games[i].globalLock));
        return -EBUSY;
    }
    return 0;
}

int my_release( struct inode *inode, struct file *filp ) {
    privateGameData* PGD = (privateGameData*)filp->private_data;
    down(&PGD->myGame->globalLock);
    PGD->myGame->someoneLeft = TRUE;
    up(&PGD->myGame->whiteMoves);
    up(&PGD->myGame->blackMoves);
    up(&PGD->myGame->globalLock);
    kfree(filp->private_data);
    return 0;
}

loff_t my_llseek(struct file *filp, loff_t loff, int off){
    return -ENOSYS;
}

ssize_t my_read( struct file *filp, char *buf, size_t count, loff_t *f_pos ) {
    //printk("\n~1~\n");
    privateGameData* PGD = (privateGameData*)filp->private_data;
    //printk("\n~2~\n");
    down(&PGD->myGame->globalLock);
    //printk("\n~3~\n");
    if (PGD->myGame->someoneLeft == TRUE || PGD->myGame->Winner!=0){
        //printk("\n~4~\n");
        up(&PGD->myGame->whiteMoves);
        //printk("\n~5~\n");
        up(&PGD->myGame->blackMoves);
        //printk("\n~6~\n");
        up(&PGD->myGame->globalLock);
        //printk("\n~7~\n");
        return -10;
    }
    /* DEAN CHANGES */
    //printk("\n~8~\n");
    Print(&PGD->myGame->board,buf,count);
    //printk("\n~9~\n");
    up(&PGD->myGame->globalLock);
    //printk("\n~10~\n");
    /* END OF DEAN CHANGES */

    //read the data
    //copy the data to user
    //return the ammount of read data
    return 0;//TODO: remove this and think
}


ssize_t my_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {

    privateGameData* PGD = (privateGameData*)filp->private_data;
    if (PGD->color == WHITE){
        down(&PGD->myGame->whiteMoves);
    }
    else if (PGD->color == BLACK){
        down(&PGD->myGame->blackMoves);
    }
    down(&PGD->myGame->globalLock);
    if (PGD->myGame->someoneLeft == TRUE || PGD->myGame->Winner!=0){
        up(&PGD->myGame->whiteMoves);
        up(&PGD->myGame->blackMoves);
        up(&PGD->myGame->globalLock);
        return -10;
    }
    char move;
    copy_from_user((void*)&move, (void*)buf, 1);
    Direction dir = (int)(move - '0');
    if (dir != UP   && dir != DOWN && dir != LEFT && dir != RIGHT)
    {
        PGD->myGame->Winner = -(PGD->color);
        return -1;
    }
    Update(&PGD->myGame->board, PGD->color, PGD, move);

    if (PGD->color == WHITE){
        up(&PGD->myGame->blackMoves);
    }
    else if (PGD->color == BLACK){
        up(&PGD->myGame->whiteMoves);
    }
    up(&PGD->myGame->globalLock);

    return 0;//TODO: remove this and think
}


int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) {
    /* DEAN CHANGES */
    privateGameData* PGD = (privateGameData*)filp->private_data;
    down(&PGD->myGame->globalLock);
    /* END OF DEAN CHANGES */
    switch( cmd ) {
        case SNAKE_GET_WINNER:
            if (PGD->myGame->Winner == WHITE)
            {
                up(&PGD->myGame->globalLock);
                return 4;
            }
            if (PGD->myGame->Winner == BLACK)
            {
                // if we got here it means that black player lost becuase "1" is the white player and he's stuck
                up(&PGD->myGame->globalLock);
                return 2;
            }
            if (PGD->myGame->Winner == TIE)
            {
                up(&PGD->myGame->globalLock);
                return 5;
            }
            else {
                up(&PGD->myGame->globalLock);
                return -1;
            }
            /* END OF DEAN CHANGES */
            break;

        case SNAKE_GET_COLOR:
            //handle SNAKE_GET_COLOR;
            /* DEAN CHANGES */
            if (PGD->color == WHITE){
                up(&PGD->myGame->globalLock);
                return 4;
            } else if (PGD->color == BLACK){
                up(&PGD->myGame->globalLock);
                return 2;
            } else {
                up(&PGD->myGame->globalLock);
                return -ENOTTY;
            }
            /* END OF DEAN CHANGES */
            break;

        default:
            up(&PGD->myGame->globalLock);
            return -ENOTTY;
    }
    /* DEAN CHANGES */
    up(&PGD->myGame->globalLock); //TODO think about this
    /* END OF DEAN CHANGES */
    return 0;
}





