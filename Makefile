INSTALLPATH = /usr/bin

all:
	make -C surgeon && make -C snoogans.0.5

clean:
	make -C surgeon clean && make -C snoogans.0.5 clean

install:
	make -C surgeon install && make -C snoogans.0.5 install && cp d2-inject $(INSTALLPATH) && cp d2-unload $(INSTALLPATH) && chmod +x $(INSTALLPATH)/d2-inject && chmod +x $(INSTALLPATH)/d2-unload

remove:
	make -C surgeon remove && make -C snoogans.0.5 remove && rm $(INSTALLPATH)/d2-inject && rm $(INSTALLPATH)/d2-unload
