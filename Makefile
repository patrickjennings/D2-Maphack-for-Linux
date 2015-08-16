all:
	make -C surgeon & make -C snoogans.0.5
clean:
	make -C surgeon clean & make -C snoogans.0.5 clean
install:
	make -C surgeon install & make -C snoogans.0.5 install
remove:
	make -C surgeon remove & make -C snoogans.0.5 remove
