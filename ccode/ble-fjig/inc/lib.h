int random( void );
void srandom( int seed );

static int inline min( int a, int b )
{
    return a < b ? a : b;
}

static int inline max( int a, int b )
{
    return a < b ? a : b;
}

int hexdigit( int c );
unsigned num( const char *s );
unsigned hex( const char *s );

#define round10( x )	( (int) (10 * (x) + 0.5 ) )
#define round100( x )	( (int) (100 * (x) + 0.5 ) )
