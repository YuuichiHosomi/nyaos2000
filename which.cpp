#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "nndir.h"
#include "nnstring.h"
#include "shell.h"

/* ���s�t�@�C���̃p�X��T��
 *      nm      < ���s�t�@�C���̖��O
 *      which   > ���������ꏊ
 * return
 *      0 - ����
 *      -1 - �����炸
 */
int which( const char *nm, NnString &which )
{
    NnString rest(".");
    const char *env=getEnv("PATH",NULL);
    if( env != NULL )
	rest << ";" << env ;
    
    while( ! rest.empty() ){
	NnString path;
	rest.splitTo( path , rest , ";" );
	if( path.empty() )
	    continue;
        path << '\\' << nm;
        if( NnDir::access(path.chars())==0 ){
            which = path;
            return 0;
        }
    }
    return -1;
}
