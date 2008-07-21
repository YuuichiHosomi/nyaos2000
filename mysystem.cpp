#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <io.h>
#include <dos.h>
#include <string.h>

#include "config.h"

#ifndef NYADOS
#  include <fcntl.h>
#endif

#include "nnstring.h"
#include "nnvector.h"
#include "nnhash.h"
#include "writer.h"
#include "nndir.h"
#include "shell.h"

enum{
    TOO_LONG_COMMANDLINE = -3  ,
    MAX_COMMANDLINE_SIZE = 127 ,
};


/* ���spawn�Bspawn�̃C���^�[�t�F�C�X�� NN���C�u�����ɓK�����`�Œ񋟂���B
 *      args - �p�����[�^
 *      wait - P_WAIT , P_NOWAIT
 * return
 *      spawn �̖߂�l
 */
static int mySpawn( const NnVector &args , int wait )
{
    const char **argv
        =(const char**)malloc( sizeof(const char *)*(args.size() + 1) );
    for(int i=0 ; i<args.size() ; i++)
        argv[i] = ((NnString*)args.const_at(i))->chars();
    argv[ args.size() ] = NULL;
    int result;
    result = spawnvp( wait , (char*)NnDir::long2short(argv[0]) , (char**)argv );
    free( argv );
    return result;
}

static void parseRedirect( const char *&cmdline , Redirect &redirect )
{
    const char *mode="w";
    if( *cmdline == '>' ){
	mode = "a";
	++cmdline;
    }
    NnString path;
    NyadosShell::readNextWord( cmdline , path );
    if( redirect.switchTo( path , mode ) != 0 ){
	conErr << path << ": can't open.\n";
    }
}

/* ���_�C���N�g�� > �̉E���ڍs�̏������s��.
 *	sp : > �̎��̈ʒu
 *	redirect : ���_�C���N�g�I�u�W�F�N�g
 */
static void after_lessthan( const char *&sp , Redirect &redirect )
{
    if( sp[0] == '&'  &&  isdigit(sp[1]) ){
	/* n>&m */
	redirect.set( sp[1]-'0' );
	sp += 2;
    }else if( sp[0] == '&' && sp[1] == '-' ){
	/* n>&- */
	redirect.switchTo( "nul", "w" );
    }else{
	/* n> �t�@�C���� */
	parseRedirect( sp , redirect );
    }
}

/* �R�}���h���C���� | �ŕ������āA�x�N�^�[�Ɋe�R�}���h��������
 *	cmdline - �R�}���h���C��
 *	vector - �e�v�f������z��
 */
static void devidePipes( const char *cmdline , NnVector &vector )
{
    NnString one,left(cmdline);
    while( left.splitTo(one,left,"|") , !one.empty() )
	vector.append( one.clone() );
}

/* NYADOS ��p�F���_�C���N�g�����t���P�R�}���h�������[�`��
 *	cmdline - �R�}���h
 *	wait - P_WAIT or P_NOWAIT
 * return
 *	�R�}���h�̎��s����
 */
static int do_one_command( const char *cmdline , int wait )
{
    Redirect redirect0(0);
    Redirect redirect1(1);
    Redirect redirect2(2);
    NnString execstr;

    int quote=0;
    while( *cmdline != '\0' ){
	if( *cmdline == '"' )
	    quote ^= 1;

        if( quote==0 && cmdline[0]=='<' ){
            ++cmdline;
            NnString path;
            NyadosShell::readNextWord( cmdline , path );
            if( redirect0.switchTo( path , "r" ) != 0 ){
                conErr << path << ": can't open.\n";
            }
        }else if( quote==0 && cmdline[0]=='>' ){
	    ++cmdline;
	    parseRedirect( cmdline , redirect1 );
	}else if( quote==0 && cmdline[0]=='1' && cmdline[1]=='>' ){
	    cmdline += 2;
	    after_lessthan( cmdline , redirect1 );
	}else if( quote==0 && cmdline[0]=='2' && cmdline[1]=='>' ){
	    cmdline += 2;
	    after_lessthan( cmdline , redirect2 );
	}else{
	    execstr << *cmdline++;
	}
    }
#ifdef NYADOS
    /* NYADOS �ȊO�̏ꍇ�́A�����ɗ��鎞�́A����
     * standalone ���[�h�ł��邱�Ƃ��m�肵�Ă���̂ŁA
     * ���f����K�v���Ȃ��Ȃ��B
     */
    if( properties.get("standalone") == NULL ){
	return system( execstr.chars() );
    }
#endif
    NnVector args;

    execstr.splitTo( args );
    return mySpawn( args , wait );
}

/* NYADOS ��p�Fsystem��֊֐��i�p�C�v���C�������j
 *	cmdline - �R�}���h������	
 * return
 *	�Ō�Ɏ��s�����R�}���h�̃G���[�R�[�h
 */
int mySystem( const char *cmdline )
{
    if( cmdline[0] == '\0' )
	return 0;
#ifdef NYADOS
    /* NYADOS �̏ꍇ�́A�W���G���[�o�́E�[���p�C�v�̎��O�����̂��߁A
     * ���������[���K�w����(do_one_command�֐�)�ŁAsystem �֐����ĂԁB
     *
     * NYACUS �ł�������Ȃ��̂́A95,98,Me �ł͖{���̃p�C�v�̓��삪
     * ���܂����s����ł��邽�߁B
     */
    NnString outempfn;
#else
    NnString *mode=(NnString*)properties.get("standalone");
    if( mode == NULL || mode->length() <= 0 ){
	errno = 0;
	int rc=system( cmdline );
	if( errno != 0 ){
	    perror( SHELL_NAME );
	    return -1;
	}
	return rc;
    }
    int pipefd0 = -1;
    int save0 = -1;
#endif
    NnVector pipeSet;
    int result=0;

    devidePipes( cmdline , pipeSet );
    for( int i=0 ; i < pipeSet.size() ; ++i ){
	errno = 0;
#ifdef NYADOS
	/* NYADOS �ł̓e���|�����t�@�C�����x�[�X�Ƃ����A
	 * �[���p�C�v���\�z����B
	 */
	NnString intempfn;
	{
	    Redirect redirect0(0);
	    Redirect redirect1(1);

	    if( ! outempfn.empty() ){
		intempfn = outempfn;
		redirect0.switchTo( intempfn , "r" );
	    }
	    if( i < pipeSet.size()-1 ){
		outempfn = NnDir::tempfn(); 
		redirect1.switchTo( outempfn , "w" );
	    }
	    result = do_one_command( ((NnString*)pipeSet.at(i))->chars() , P_WAIT );
	}
	if( ! intempfn.empty() )
	    ::remove( intempfn.chars() );
#else
	/* NYACUS,NYAOS2 �ł́A�{���̃p�C�v���R�}���h��A������ */
        if( pipefd0 != -1 ){
	    /* �p�C�v���C�������ɍ���Ă���ꍇ�A
	     * ���͑���W�����͂֒���K�v������
	     */
            if( save0 == -1  )
		save0 = dup( 0 );
            dup2( pipefd0 , 0 );
	    ::close( pipefd0 );
            pipefd0 = -1;
        }
        if( i < pipeSet.size() - 1 ){
	    /* �p�C�v���C���̖����łȂ��R�}���h�̏ꍇ�A
	     * �W���o�͂̐���A�p�C�v�̈���ɂ���K�v������
	     */
            int handles[2];

            int save1=dup(1);
#ifdef NYAOS2
            _pipe( handles );
#else
            _pipe( handles , 0 , O_BINARY | O_NOINHERIT );
#endif
            dup2( handles[1] , 1 );
	    ::close( handles[1] );
            pipefd0 = handles[0];

            do_one_command( ((NnString*)pipeSet.at(i))->chars(),P_NOWAIT );

            dup2( save1 , 1 );
            ::close( save1 );
        }else{
            result = do_one_command( ((NnString*)pipeSet.at(i))->chars(),P_WAIT );
        }
#endif
	if( result < 0 ){
#ifdef NYADOS
	    if( ! outempfn.empty() )
		::remove( outempfn.chars() );
#endif
	    if( ((NnString*)pipeSet.at(i))->length() > 110 ){
		conErr << "Too long command line,"
			    " or bad command or file name.\n";
	    }else{
		conErr << "Bad command or file name.\n";
	    }
            goto exit;
	}
    }
  exit:
#ifndef NYADOS
    if( pipefd0 != -1 ){
	::close( pipefd0 );
    }
    if( save0 != -1 ){
        dup2( save0 , 0 );
        ::close( save0 );
    }
#endif
    return result;
}
