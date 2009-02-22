#ifndef WRITER_H
#define WRITER_H
#include "nnstring.h"
#include <stdio.h>

class Writer : public NnObject {
public:
    virtual ~Writer();
    virtual Writer &write( char c )=0;
    virtual Writer &write( const char *s)=0;
    virtual int isatty() const;

    Writer &operator << ( char c ){ return write(c); }
    Writer &operator << ( const char *s ){ return write(s); }
    Writer &operator << ( int n );
    Writer &operator << (const NnString &s){ return *this << s.chars(); }
};

/* <stdio.h> �� FILE* ��ʂ��ďo�͂��� Writer�n�N���X.
 * �t�@�C���̃I�[�v���N���[�Y�Ȃǂ͂��Ȃ��B
 * �قƂ�ǁAstdout,stderr��p
 */
class StreamWriter : public Writer {
    FILE *fp_;
protected:
    FILE *fp(){ return fp_; }
    void setFp( FILE *fp ){ fp_ = fp; }
public:
    ~StreamWriter(){}
    StreamWriter() : fp_(NULL) {}
    StreamWriter( FILE *fp ) : fp_(fp){}
    Writer &write( char c );
    Writer &write( const char *s );
    int ok() const { return fp_ != NULL ; }
    int fd() const { return fileno(fp_); }
    int isatty() const;
};

/* �e�L�X�g�t�@�C���ɏo�͂��� Writer�n�N���X�B
 * �I�����Ƀt�@�C�����N���[�Y����B
 */
#ifdef NYADOS
    /* NYADOS �ł�,VFAT-API & RAW I/O API ���g�����������s�� */
    class FileWriter : public Writer {
        unsigned short fd_,size;
        char buffer[128];
        void put(char c);
    public:
        ~FileWriter();
        FileWriter( const char *fn     , const char *mode );
        FileWriter( const NnString &fn , const char *mode );
        Writer &write( char c );
        Writer &write( const char *s );
        int ok(){ return fd_ != -1; }
        int fd(){ return fd_; }
	int (isatty)() const ;
    };
#else
    /* NYADOS�ȊO�ł́A���ʂ� FILE* ���g�����������g�� */
    class FileWriter : public StreamWriter {
    public:
        ~FileWriter();
        FileWriter( const char *fn , const char *mode );
    };
#endif

class PipeWriter : public StreamWriter {
#ifdef NYADOS
    NnString tempfn;
#endif
    NnString cmds;
    void open( const NnString &cmds );
public:
    ~PipeWriter();
    PipeWriter( const char *cmds );
    PipeWriter( const NnString &cmds ){ open(cmds); }
};

#ifdef NYACUS

/* �G�X�P�[�v�V�[�P���X�����߂��āA��ʂ̃A�g���r���[�g��
 * �R���g���[���܂ōs���o�̓N���X.
 */
class AnsiConsoleWriter : public StreamWriter {
    static int default_color;
    int flag;
    int param[20];
    size_t n;
    enum { PRINTING , STACKING } mode ;
    enum { BAREN = 1 , GREATER = 2 } ;
public:
    AnsiConsoleWriter( FILE *fp ) 
	: StreamWriter(fp) , flag(0) , n(0) , mode(PRINTING) {}

    Writer &write( char c );
    Writer &write( const char *p );
};

#else

#define AnsiConsoleWriter StreamWriter

#endif

extern AnsiConsoleWriter conOut,conErr;

/* Writer �̃|�C���^�ϐ��ɑ΂��ă��_�C���N�g���� Writer �N���X. */
class WriterClone : public Writer {
    Writer **rep;
public:
    WriterClone( Writer **rep_ ) : rep(rep_) {}
    ~WriterClone(){}
    Writer &write( char c ){ return (*rep)->write( c ); }
    Writer &write( const char *s ){ return (*rep)->write( s ); }
};

/* �o�͓��e��S�Ď̂ĂĂ��܂� Writer */
class NullWriter : public Writer {
public:
    NullWriter(){}
    ~NullWriter(){}
    Writer &write( char c ){ return *this; }
    Writer &write( const char *s ){ return *this; }
};



/* �W���o�́E���͂����_�C���N�g������A���ɖ߂����肷��N���X */
class Redirect {
    int original_fd;
    int fd_;
    int isOpenFlag;
public:
    Redirect(int x) : original_fd(-1) , fd_(x) {}
    ~Redirect(){ reset(); }
    void close();
    void reset();
    void set(int x);
    int  fd() const { return fd_; }

    int switchTo( const NnString &fn , const char *mode );
};


#endif
