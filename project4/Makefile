LIB = myfs_lib.c myfs_lib_support.c vdisk.c
FORMAT = myfs_format
INSPECT = myfs_inspect
LIST = myfs_list
MKD = myfs_mkd
RMD = myfs_rmd
STATS = myfs_stats
TOUCH = myfs_touch
RM = myfs_rm
APPEND = myfs_append
MORE = myfs_more
CREATE = myfs_create
LINK = myfs_link
COPY = myfs_copy
MOVE = myfs_move
MKP = myfs_mkp
LC = myfs_lc
all:
	gcc $(FORMAT).c $(LIB) -o $(FORMAT)
	gcc $(INSPECT).c $(LIB) -o $(INSPECT)
	gcc $(LIST).c $(LIB) -o $(LIST)
	gcc $(MKD).c $(LIB) -o $(MKD)
	gcc $(RMD).c $(LIB) -o $(RMD)
	gcc $(STATS).c $(LIB) -o $(STATS)
	gcc $(TOUCH).c $(LIB) -o $(TOUCH)
	gcc $(RM).c $(LIB) -o $(RM)
	gcc $(APPEND).c $(LIB) -o $(APPEND)
	gcc $(MORE).c $(LIB) -o $(MORE)
	gcc $(CREATE).c $(LIB) -o $(CREATE)
	gcc $(LINK).c $(LIB) -o $(LINK)
	gcc $(COPY).c $(LIB) -o $(COPY)
	gcc $(MOVE).c $(LIB) -o $(MOVE)
	gcc $(MKP).c $(LIB) -o $(MKP)
	gcc $(LC).c $(LIB) -o $(LC)
clean:
	rm $(FORMAT)
	rm $(INSPECT)
	rm $(LIST)
	rm $(MKD)
	rm $(RMD)
	rm $(STATS)
	rm $(TOUCH)
	rm $(RM)
	rm $(APPEND)
	rm $(MORE)
	rm $(CREATE)
	rm $(LINK)
	rm $(COPY)
	rm $(MOVE)
	rm $(MKP)
	rm $(LC)
	rm project4.zip
zip:
	zip project4.zip *.c *.h Makefile README.txt