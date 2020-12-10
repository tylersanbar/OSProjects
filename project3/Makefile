LIB = myfs_lib.c myfs_lib_support.c vdisk.c
FORMAT = myfs_format
INSPECT = myfs_inspect
LIST = myfs_list
MKD = myfs_mkd
RMD = myfs_rmd
STATS = myfs_stats
all:
	gcc $(FORMAT).c $(LIB) -o $(FORMAT)
	gcc $(INSPECT).c $(LIB) -o $(INSPECT)
	gcc $(LIST).c $(LIB) -o $(LIST)
	gcc $(MKD).c $(LIB) -o $(MKD)
	gcc $(RMD).c $(LIB) -o $(RMD)
	gcc $(STATS).c $(LIB) -o $(STATS)
clean:
	rm $(FORMAT)
	rm $(INSPECT)
	rm $(LIST)
	rm $(MKD)
	rm $(RMD)
	rm $(STATS)
	rm project3.zip
zip:
	zip project3.zip *.c *.h Makefile README.txt