LIB = myfs_lib.c myfs_lib_support.c vdisk.c
FORMAT = myfs_format
INSPECT = myfs_inspect
LIST = myfs_list
MKD = myfs_mkd
RMD = myfs_rmd
STATS = myfs_stats
FOPEN = myfs_fopen
FCLOSE = myfs_fclose
FWRITE = myfs_fwrite
FREAD = myfs_fread
DELETE = myfs_delete_file
MOVE = myfs_move
LINK = myfs_link
MKP = myfs_mkp
all:
	gcc $(FORMAT).c $(LIB) -o $(FORMAT)
	gcc $(INSPECT).c $(LIB) -o $(INSPECT)
	gcc $(LIST).c $(LIB) -o $(LIST)
	gcc $(MKD).c $(LIB) -o $(MKD)
	gcc $(RMD).c $(LIB) -o $(RMD)
	gcc $(STATS).c $(LIB) -o $(STATS)
	gcc $(FOPEN).c $(LIB) -o $(FOPEN)
	gcc $(FCLOSE).c $(LIB) -o $(FCLOSE)
	gcc $(FWRITE).c $(LIB) -o $(FWRITE)
	gcc $(FREAD).c $(LIB) -o $(FREAD)
	gcc $(DELETE).c $(LIB) -o $(MOVE)
	gcc $(LINK).c $(LIB) -o $(LINK)
	gcc $(MKP).c $(LIB) -o $(MKP)
clean:
	rm $(FORMAT)
	rm $(INSPECT)
	rm $(LIST)
	rm $(MKD)
	rm $(RMD)
	rm $(STATS)
	rm $(FOPEN)
	rm $(FCLOSE)
	rm $(FWRITE)
	rm $(FREAD)
	rm $(DELETE)
	rm $(LINK)
	rm $(MKP)
	rm project4.zip
zip:
	zip project4.zip *.c *.h Makefile README.txt