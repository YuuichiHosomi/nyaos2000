# =============================================================
#   Makefile for NYADOS/NYACUS/NYAOS-II
#   executable by GNU Make
# =============================================================
#
# make 
#	���s�t�@�C���̍쐬
# make clean
#	*.obj,*.exe ���폜
# make cleanobj
#	*.obj ���폜
# This Makefile is for GNU Make.
#
# make package
#	*.zip �̍쐬
# make install
#	$(INSTALLDIR) �֎��s�t�@�C���̃R�s�[

# CCC=-DNDEBUG
CCC=-DNDEBUG
--- NYACUS
# Borland Make
CC=bcc32
NAME=NYACUS
CFLAGS=-O -d -Z $(CCC)
--- DIGITALMARS
CC=sc
NAME=NYADOS
CFLAGS=-P -ml -o $(CCC)
# -P ... Pascal linkcage
--- NYAOS2
# GNU Make
CC=gcc
NAME=NYAOS2
CFLAGS=-O2 -Zomf -Zsys -DOS2EMX $(CCC)
.SUFFIXES : .cpp .obj .exe .h
--- NYAOS2S
CC=sc
NAME=NYAOS2
CFLAGS=-mf -o $(CCC)
--- VC
CC=cl
NAME=NYACUS
CFLAGS=/Za /I vc $(CCC)
---
TARGET=$(NAME).EXE

.cpp.obj :
	$(CC) $(CFLAGS) -c $<

# make install �������Ɏ��s�t�@�C�����R�s�[����f�B���N�g��
INSTALLDIR=\usr\bin\.

# ------------------------------------------------------------
OBJ1=nyados.obj nnstring.obj nndir.obj twinbuf.obj mysystem.obj keyfunc.obj
OBJ2=getline.obj getline2.obj keybound.obj dosshell.obj nnhash.obj
OBJ3=writer.obj history.obj ishell.obj scrshell.obj wildcard.obj cmdchdir.obj
OBJ4=shell.obj shell4.obj foreach.obj which.obj reader.obj nnvector.obj
OBJ5=ntcons.obj shellstr.obj cmds1.obj cmds2.obj xscript.obj shortcut.obj
OBJ6=strfork.obj lsf.obj open.obj
OBJS=$(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6)
RESPONCE=nyados.tmp

$(TARGET) : $(OBJS)
--- NYAOS2
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lstdcpp
--- NYADOS NYACUS DIGITALMARS NYAOS2S
	echo $(OBJ1)  > $(RESPONCE)
	echo $(OBJ2) >> $(RESPONCE)
	echo $(OBJ3) >> $(RESPONCE)
	echo $(OBJ4) >> $(RESPONCE)
	echo $(OBJ5) >> $(RESPONCE)
	echo $(OBJ6) >> $(RESPONCE)
--- NYADOS 
	$(CC) $(CFLAGS) -o$(TARGET) @$(RESPONCE)
--- NYACUS
	$(CC) $(CFLAGS) -e$(TARGET) @$(RESPONCE)
	brc32 nyacus.rc nyacus.exe
--- DIGITALMARS 
	$(CC) $(CFLAGS) -o$(TARGET) @$(RESPONCE)
--- NYAOS2S
	$(CC) /L $(CFLAGS) -o$(TARGET) @$(RESPONCE)
--- VC
	echo /link /OPT:NOWIN98 >> $(RESPONCE)
	$(CC) USER32.lib $(CFLAGS) -o $(TARGET) @$(RESPONCE)
--- NYADOS NYACUS DIGITALMARS
	del $(RESPONCE)
---

# ���C�����[�`��
nyados.obj   : nyados.cpp   nnstring.h getline.h

# ��s����
twinbuf.obj  : twinbuf.cpp  getline.h
getline.obj  : getline.cpp  getline.h nnstring.h
getline2.obj : getline2.cpp getline.h
keybound.obj : keybound.cpp getline.h
keyfunc.obj : keyfunc.cpp getline.h
dosshell.obj : dosshell.cpp getline.h
xscript.obj : xscript.cpp

# �C���^�v���^����
shell.obj    : shell.cpp    shell.h 
shell4.obj   : shell4.cpp   shell.h nnstring.h
scrshell.obj : scrshell.cpp shell.h
ishell.obj   : ishell.cpp   shell.h ishell.h 
mysystem.obj : mysystem.cpp nnstring.h
shellstr.obj : shellstr.cpp

# �ʃR�}���h����
cmds1.obj : cmds1.cpp shell.h nnstring.h
cmds2.obj : cmds2.cpp shell.h nnstring.h
cmdchdir.obj  : cmdchdir.cpp  shell.h nnstring.h nndir.h
foreach.obj   : foreach.cpp   shell.h
lsf.obj : lsf.cpp

# ���ʃ��C�u����
nnstring.obj  : nnstring.cpp  nnstring.h  nnobject.h
nnvector.obj  : nnvector.cpp  nnvector.h  nnobject.h
nnhash.obj : nnhash.cpp nnhash.h nnobject.h
strfork.obj : strfork.cpp

# ���ˑ����C�u����
writer.obj    : writer.cpp    writer.h
reader.obj    : reader.cpp    reader.h
nndir.obj       : nndir.cpp nndir.h
wildcard.obj  : wildcard.cpp  nnstring.h nnvector.h nndir.h
ntcons.obj : ntcons.cpp
open.obj  : open.cpp

install :
	copy $(TARGET) $(INSTALLDIR)

package : $(NAME).txt $(TARGET)
	del *.obj
	zip $(NAME).zip $(NAME).txt $(TARGET) _$(NAME)
	zip nyasrc.zip Makefile* *.cpp *.h *.tmpl

clean : cleanobj
	if exist $(TARGET) del $(TARGET)

cleanobj : 
	del *.obj
	if exist $(RESPONCE) del $(RESPONCE)

todo :
	ruby todo.rb todo.txt > todo.html

--- NYADOS NYACUS DIGITALMARS
lib :
	del nncore16.lib
--- NYADOS DIGITALMARS
	lib nncore16.lib /c +nnstring.obj+nnvector.obj+nnhash.obj+nndir.obj,nncore16.lst,nncore16.lib
--- NYACUS
	tlib nncore32.lib /C +nnstring.obj+nnvector.obj+nnhash.obj+nndir.obj nncore32.lst
