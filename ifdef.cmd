/* ifdoc.cmd (REXX) for OS/2
 *   �{�t�@�C���� OS/2 �� REXX �X�N���v�g�ł��B
 *   �g���q�� CMD �ł����AWindows ����͎g�p�ł��܂���B
 */
printFlag = 1
do while lines() > 0
    line = linein()
    if left(line,3) = "---" then do
	if length(line) <= 4  then do
	    printFlag = 1
	end
	else do
	    printFlag = 0
	    do i=2 to words(line)
		do j=1 to arg()
		    if arg(j) = word(line,i) then
			printFlag = 1
		end
	    end
	end 
    end
    else if printFlag = 1 then
	say line
end
/* vim:set ft=rexx */
