#include <stdlib.h>
#include <unistd.h>

#include <ncurses.h>

#define GRID_W 90
#define GRID_H 100

WINDOW * window;

typedef int col_t[ GRID_H ];
typedef col_t * grid_t;

int last_grid[ GRID_W ][ GRID_H ];
int grid[ GRID_W ][ GRID_H ];

static void draw_grid()
{
    for ( int x = 0; x < GRID_W; x++ ) {
        for ( int y = 0; y < GRID_H; y++ ) {
            int id = grid[ x ][ y ];

            if ( id == 1 ) {
                wattron( window, COLOR_PAIR( 1 ) );
            } else if ( id == 2 ) {
                wattron( window, COLOR_PAIR( 2 ) );
            } else if ( id == 3 ) {
                wattron( window, COLOR_PAIR( 3 ) );
            }

            mvwaddch( window, y + 1, x + 1, id ? '#' : ' ' );
        }
    }
}

static int in_bounds( int x, int y )
{
    return x >= 0 && y >= 0 && x < GRID_W && y < GRID_H;
}

static int is_vacant( int x, int y )
{
    if ( !in_bounds( x, y ) )
        return 0;

    return grid[ x ][ y ] == 0;
}

static int is_less_dense( int x, int y )
{
    if ( !in_bounds( x, y ) )
        return 0;

    if ( grid[ x ][ y ] == 0 )
        return 1;
    if ( grid[ x ][ y ] == 1 )
        return 1;

    return 0;
}

static int is_more_dense_than_gas( int x, int y )
{
    if ( !in_bounds( x, y ) )
        return 0;

    if ( grid[ x ][ y ] == 3 )
        return 0;

    return 1;
}

static int r = 0;

static void tick_gas( int x, int y )
{
    r = rand();

    int new_x = x;
    int new_y = y;

    if ( is_vacant( x, y - 1 ) ) {
        new_y--;
    } else if ( ( ( r % 2 ) == 0 ) && is_more_dense_than_gas( x + 1, y - 1 ) ) {
        new_x++;
        new_y--;
    } else if ( ( ( r % 2 ) == 1 ) && is_more_dense_than_gas( x - 1, y - 1 ) ) {
        new_x--;
        new_y--;
    } else if ( ( ( r % 2 ) == 0 ) && is_more_dense_than_gas( x + 1, y ) ) {
        new_x++;
    } else if ( ( ( r % 2 ) == 1 ) && is_more_dense_than_gas( x - 1, y ) ) {
        new_x--;
    }

    grid[ x ][ y ] = 0;
    grid[ new_x ][ new_y ] = 3;
}

static void tick_water( int x, int y )
{
    r = rand();

    int new_x = x;
    int new_y = y;

    if ( is_vacant( x, y + 1 ) ) {
        new_y++;
    } else if ( ( ( r % 2 ) == 0 ) && is_vacant( x + 1, y + 1 ) ) {
        new_x++;
        new_y++;
    } else if ( ( ( r % 2 ) == 1 ) && is_vacant( x - 1, y + 1 ) ) {
        new_x--;
        new_y++;
    } else if ( ( ( r % 2 ) == 0 ) && is_vacant( x + 1, y ) ) {
        new_x++;
    } else if ( ( ( r % 2 ) == 1 ) && is_vacant( x - 1, y ) ) {
        new_x--;
    }

    grid[ x ][ y ] = 0;
    grid[ new_x ][ new_y ] = 1;
}

static void tick_sand( int x, int y )
{
    r = rand();

    int new_x = x;
    int new_y = y;

    if ( is_less_dense( x, y + 1 ) ) {
        new_y++;
    } else if ( ( ( r % 2 ) == 0 ) && is_less_dense( x + 1, y + 1 ) ) {
        new_x++;
        new_y++;
    } else if ( ( ( r % 2 ) == 1 ) && is_less_dense( x - 1, y + 1 ) ) {
        new_x--;
        new_y++;
    }

    grid[ x ][ y ] = grid[ new_x ][ new_y ];

    if ( grid[ new_x ][ new_y ] == 1 ) {
        grid[ x ][ y ] = 3;
    }
    grid[ new_x ][ new_y ] = 2;
}

static void tick()
{
    for ( int x = 0; x < GRID_W; x++ ) {
        for ( int y = 0; y < GRID_H; y++ ) {
            last_grid[ x ][ y ] = grid[ x ][ y ];
        }
    }

    for ( int x = 0; x < GRID_W; x++ ) {
        for ( int y = 0; y < GRID_H; y++ ) {
            if ( last_grid[ x ][ y ] != grid[ x ][ y ] )
                continue;

            if ( last_grid[ x ][ y ] == 1 ) {
                tick_water( x, y );
            }
            if ( last_grid[ x ][ y ] == 2 ) {
                tick_sand( x, y );
            }
            if ( last_grid[ x ][ y ] == 3 ) {
                tick_gas( x, y );
            }
        }
    }
}

static void add_water()
{
    for ( int x = 30; x < 80; x++ ) {
        for ( int y = 10; y < 80; y++ ) {
            grid[ x ][ y ] = 1;
        }
    }
}

static void add_sand()
{
    for ( int x = 5; x < 10; x++ ) {
        for ( int y = 5; y < 70; y++ ) {
            grid[ x ][ y ] = 2;
        }
    }
}

int main()
{
    srand( 0 );
    add_water();
    add_sand();

    initscr();

    if ( has_colors() == FALSE )
        return 1;

    start_color();
    init_pair( 1, COLOR_BLUE, COLOR_BLACK );
    init_pair( 2, COLOR_YELLOW, COLOR_BLACK );
    init_pair( 3, COLOR_WHITE, COLOR_BLACK );

    const int x_offset = ( COLS - GRID_W ) / 2 - 1;
    const int y_offset = ( LINES - GRID_H ) / 2 - 1;
    window = newwin( GRID_H + 2, GRID_W + 2, y_offset, x_offset );

    refresh();

    box( window, 0, 0 );
    mvwaddstr( window, 0, 1, "SHIRO SAND SIM" );
    draw_grid();

    wrefresh( window );

    // while ( getch() != 'q' ) {
    for ( int i = 0; i < 1500; i++ ) {
        tick();
        draw_grid();
        wrefresh( window );
        refresh();

        usleep( 10000 );
    }

    // getch();

    endwin();

    return 0;
}
