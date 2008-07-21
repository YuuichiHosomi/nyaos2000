#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include "nndir.h"
#include "writer.h"
#include "reader.h"
#include "ntcons.h"

#ifdef NYACUS

int AnsiConsoleWriter::default_color=-1;

Writer &AnsiConsoleWriter::write( char c )
{
    enum{ EMPTY = -1 };

    if( mode == PRINTING ){
	if( c == '\x1B' ){
	    mode = STACKING;
	    param[ n = 0 ] = 0;
	}else if( c == '\b' ){
	    fflush( stdout );
	    fflush( stderr );
	    Console::backspace();
	}else{
	    fputc( c, fp() );
	}
    }else{
	if( c == '[' ){
	    flag |= BAREN ;
	}else if( c == '>' ){
	    flag |= GREATER ;
	}else if( isdigit(c) ){
	    if( param[ n ] == EMPTY ){
		param[ n ] = c - '0' ;
	    }else{
		param[ n ] = param[ n ] * 10 + (c-'0');
	    }
	}else if( c == ';' ){
	    if( n < numof(param)-1 )
		param[ ++n ] = EMPTY ;
	}else if( isalpha(c) ){
	    if( c == 'm' ){
		int lastcolor = Console::color();
		int forecolor =   lastcolor       & 7;
		int backcolor = ((lastcolor >> 4) & 7);
		int high      = ((lastcolor >> 3 )& 1);

		if( default_color == -1 )
		    default_color = lastcolor ;

		for(int i=0 ; i <= n ; ++i ){
		    /* ANSI-ESC : BGR �� * NT-CODE  : RGB �̕ϊ��e�[�u�� */
		    static int cnv[]={ 0 , 4 , 2 , 6 , 1 , 5 , 3 , 7 };

		    if( 30 <= param[i] && param[i] <= 37 ){
			forecolor = cnv[ param[i]-30 ];
		    }else if( 40 <= param[i] && param[i] <= 47 ){
			backcolor = cnv[ param[i]-40 ];
		    }else if( param[i] == 1 ){
			high = 1;
		    }else if( param[i] == 0 ){
			forecolor = 7;
			backcolor = 0;
			high      = 0;
		    }
		}
		Console::color( (backcolor<<4) | (high<<3) | forecolor );
	    }else if( c == 'J' ){
		Console::clear();
	    }else if( c == 'l' && (flag & GREATER) != 0 ){
		int x,y;
		Console::getLocate(x,y);
		Console::locate(x,y);
	    }
	    n = 0;
	    mode = PRINTING;
	}else{
	    n = 0;
	    mode = PRINTING;
	}
    }
    return *this;
}

Writer &AnsiConsoleWriter::write( const char *s )
{
    while( *s != '\0' )
	this->write( *s++ );
    fflush( fp() );
    return *this;
}

#endif

AnsiConsoleWriter conOut(stdout),conErr(stderr);

int Writer::isatty() const
{ 
    return 0;
}


Writer::~Writer(){}

Writer &Writer::operator << (int n)
{
    NnString tmp;
    tmp.addValueOf( n );
    return *this << tmp.chars();
#if 0
    if( n < 0 ){
        *this << '-';
        return *this << -n;
    }
    if( n >= 10 ){
        *this << (n/10) ;
        n %= 10;
    }
    return *this << "0123456789"[n];
#endif
}

Writer &Writer::write( const char *s )
{
    while( *s != '\0' )
        *this << *s++;
    return *this;
}

Writer &StreamWriter::write( char c )
{
    fputc( c , fp_ );
    return *this;
}
Writer &StreamWriter::write( const char *s )
{
    fputs( s , fp_ );
    return *this;
}

void PipeWriter::open( const NnString &cmds_ )
{
#ifdef NYADOS
    tempfn = NnDir::tempfn();
    FILE *fp = fopen( tempfn.chars() , "w" );
#elif defined(__BORLANDC__)
    FILE *fp = _popen( cmds_.chars() , "w" );
#else
    FILE *fp = popen( cmds_.chars() , "w" );
#endif
    if( fp == NULL )
        return;
    cmds   = cmds_;
    cmds.trim();
    setFp( fp );
}

PipeWriter::PipeWriter( const char *s )
{
    NnString cmds_(s);
    open( cmds_ );
}

PipeWriter::~PipeWriter()
{
    if( fp() == NULL )
        return;
    
#ifdef NYADOS
    fclose( fp() );

#if 1
    {
	Redirect redirect0(0);
	redirect0.switchTo( tempfn.chars() ,"r" );
	system( cmds.chars() );
    }
#else
    int quote=0;
    const char *p=cmds.chars();
    for(;;){
        if( *p == '"' )
            quote = !quote;
        if( *p == '\0' ){
            if( quote )
                exestr << '"';
            exestr << " < " << tempfn;
            break;
        }
        if( quote == 0  &&  *p=='|' ){
            exestr << " < " << tempfn << ' ' << p;
            break;
        }
        exestr += *p++;
    }
    system( exestr.chars() );
#endif
    remove( tempfn.chars() );
#elif defined(__BORLANDC__)
    _pclose( fp() );
#else
    pclose( fp() );
#endif
}

#ifdef NYADOS

FileWriter::FileWriter( const char *fn , const char *mode )
{
    fd_=NnDir::open(fn,mode);
    size = 0;
}
FileWriter::FileWriter( const NnString &fn , const char *mode )
{
    fd_=NnDir::open(fn.chars() , mode );
    size = 0;
}

FileWriter::~FileWriter()
{
    if( fd_ != -1 ){
        if( size > 0 ){
            NnDir::write(fd_,buffer,size);
        }
        NnDir::close(fd_);
    }
}
void FileWriter::put( char c )
{
    buffer[ size++ ] = c;
    if( size >= sizeof(buffer) ){
        NnDir::write(fd_,buffer,size);
        size = 0;
    }
}
Writer &FileWriter::write( const char *s )
{
    while( *s != '\0' ){
        if( *s == '\n' )
            this->put('\r');
        this->put(*s++);
    }
    return *this;
}
Writer &FileWriter::write( char c )
{
    if( c == '\n' )
        this->put('\r');
    this->put('\n');
    return *this;
}

int FileWriter::isatty() const
{ 
    return ::isatty(fd_);
}

#else /* NYADOS �ȊO�̏ꍇ�́A�R���X�g���N�^�̂� */

FileWriter::FileWriter( const char *fn , const char *mode )
    : StreamWriter( fopen(fn,mode) )
{
    /* Borland C++ �ł́A�ŏ��ɏ������ނ܂ŁA�t�@�C���|�C���^��
     * �ړ����Ȃ��̂ŁAdup2 ���g���ꍇ�́A
     * �����I�� lseek �ňړ������Ă��K�v������B
     * (stdio �� io �����݂����Ďg���͖̂{���̓_���Ȃ̂ŁA����͌����Ȃ��c)
     */
    if( *mode == 'a' )
	fseek( fp() , 0 , SEEK_END );
}

FileWriter::~FileWriter()
{
    if( ok() )
        fclose( this->fp() );
}


#endif

/* �W���o�́E���͂����_�C���N�g����
 *      x - �t�@�C���n���h��
 */
void Redirect::set(int x)
{
    if( original_fd == -1 )
        original_fd = dup(fd());
    dup2( x , fd() );
    isOpenFlag = 1;
}

void Redirect::close()
{
    if( original_fd != -1  &&  isOpenFlag ){ 
        ::close( fd() );
        isOpenFlag = 0;
    }
}

/* ���_�C���N�g�����ɖ߂� */
void Redirect::reset()
{
    if( original_fd != -1 ){
        dup2( original_fd , fd() );
        ::close( original_fd );
        original_fd = -1;
    }
}

/* �t�@�C�������w�肵�āA���_�C���N�g����B
 *      fname - �t�@�C����
 *      mode  - ���[�h
 * return
 *      0 - ����
 *      -1 - ���s
 */
int Redirect::switchTo( const NnString &fname , const char *mode  )
{
    if( *mode == 'r' ){
	FileReader fr(fname.chars());
	if( fr.eof() )
	    goto errpt;
	this->set( fr.fd() );
    }else{
	FileWriter fw( fname.chars() , mode );
	if( ! fw.ok() )
	    goto errpt;
	this->set( fw.fd() );

    }
    return 0;
  errpt:
    conErr << fname << ": can't open temporary file.\n";
    return -1;
}

int StreamWriter::isatty() const
{
    return ::isatty( fd() ); 
}



